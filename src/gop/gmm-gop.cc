// gop/gop-gmm.cc

// Copyright 2016-2017  Junbo Zhang

// This project based on Kaldi (https://github.com/kaldi-asr/kaldi).
// All the Kaldi's codes in this project are under the Apache License, Version 2.0.
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
// 
// However, this file has not been merged into Kaldi's master branch,
// and the codes in this file are NOT UNDER THE SAME LICENSE of Kaldi's.
// The codes in this file are NOT ALLOWED TO USE, COPY, DISTRIBUTE, OR MODIFY
// unless being permitted by the author.

#include <algorithm>
#include <limits>
#include <string>
#include <vector>
#include "base/kaldi-common.h"
#include "util/common-utils.h"
#include "gmm/am-diag-gmm.h"
#include "hmm/transition-model.h"
#include "fstext/fstext-utils.h"
#include "decoder/decoder-wrappers.h"
#include "gmm/decodable-am-diag-gmm.h"
#include "lat/kaldi-lattice.h"
#include "hmm/hmm-utils.h"
#include "gop/gmm-gop.h"

namespace kaldi {

void GmmGop::Init(std::string &tree_in_filename,
            std::string &model_in_filename,
            std::string &lex_in_filename) {
  bool binary;
  Input ki(model_in_filename, &binary);
  tm_.Read(ki.Stream(), binary);
  am_.Read(ki.Stream(), binary);
  lex_fst_ = fst::ReadFstKaldi(lex_in_filename);
  ReadKaldiObject(tree_in_filename, &ctx_dep_);

  decode_opts_.beam = 200;
}

bool GmmGop::CompileGraph(const fst::VectorFst<fst::StdArc> &phone2word_fst,
                          fst::VectorFst<fst::StdArc> *out_fst) {
  using namespace fst;

  KALDI_ASSERT(phone2word_fst.Start() != kNoStateId);

  ContextFst<StdArc> *cfst = NULL;
  {  // make cfst [ it's expanded on the fly ]
    const std::vector<int32> &phone_syms = tm_.GetPhones();  // needed to create context fst.
    int32 subseq_symbol = phone_syms.back() + 1;
    if (!disambig_syms_.empty() && subseq_symbol <= disambig_syms_.back())
      subseq_symbol = 1 + disambig_syms_.back();

    cfst = new ContextFst<StdArc>(subseq_symbol,
                                  phone_syms,
                                  disambig_syms_,
                                  ctx_dep_.ContextWidth(),
                                  ctx_dep_.CentralPosition());
  }

  VectorFst<StdArc> ctx2word_fst;
  ComposeContextFst(*cfst, phone2word_fst, &ctx2word_fst);
  // ComposeContextFst is like Compose but faster for this particular Fst type.
  // [and doesn't expand too many arcs in the ContextFst.]

  KALDI_ASSERT(ctx2word_fst.Start() != kNoStateId);

  HTransducerConfig h_cfg;
  h_cfg.transition_scale = gopts_.transition_scale;

  std::vector<int32> disambig_syms_h; // disambiguation symbols on
  // input side of H.
  VectorFst<StdArc> *H = GetHTransducer(cfst->ILabelInfo(),
                                        ctx_dep_,
                                        tm_,
                                        h_cfg,
                                        &disambig_syms_h);

  VectorFst<StdArc> &trans2word_fst = *out_fst;  // transition-id to word.
  TableCompose(*H, ctx2word_fst, &trans2word_fst);

  KALDI_ASSERT(trans2word_fst.Start() != kNoStateId);

  // Epsilon-removal and determinization combined. This will fail if not determinizable.
  DeterminizeStarInLog(&trans2word_fst);

  if (!disambig_syms_h.empty()) {
    RemoveSomeInputSymbols(disambig_syms_h, &trans2word_fst);
    // we elect not to remove epsilons after this phase, as it is
    // a little slow.
    if (gopts_.rm_eps)
      RemoveEpsLocal(&trans2word_fst);
  }

  // Encoded minimization.
  MinimizeEncoded(&trans2word_fst);

  std::vector<int32> disambig;
  AddSelfLoops(tm_,
               disambig,
               gopts_.self_loop_scale,
               gopts_.reorder,
               &trans2word_fst);

  delete H;
  delete cfst;
  return true;
}

void GmmGop::MakePhoneLoopAcceptor(std::vector<int32> &labels,
                                   fst::VectorFst<fst::StdArc> *ofst) {
  // TODO: make acceptor according phone contexts
  typedef typename fst::StdArc Arc;
  typedef typename Arc::StateId StateId;
  typedef typename Arc::Weight Weight;

  ofst->DeleteStates();
  StateId start_state = ofst->AddState();
  ofst->SetStart(start_state);
  const std::vector<int32> &phone_syms = tm_.GetPhones();
  for (size_t i = 0; i < phone_syms.size(); i++) {
    StateId next_state = ofst->AddState();
    Arc arc_phone(phone_syms[i], phone_syms[i], Weight::One(), next_state);
    ofst->AddArc(start_state, arc_phone);
  }
  ofst->SetFinal(start_state, Weight::One());
}

BaseFloat GmmGop::Decode(fst::VectorFst<fst::StdArc> &fst,
                         DecodableAmDiagGmmScaled &decodable,
                         std::vector<int32> *align) {
  FasterDecoderOptions decode_opts;
  FasterDecoder decoder(fst, decode_opts);
  decoder.Decode(&decodable);
  if (! decoder.ReachedFinal()) {
    KALDI_WARN << "Did not successfully decode.";
  }
  fst::VectorFst<LatticeArc> decoded;
  decoder.GetBestPath(&decoded);
  std::vector<int32> osymbols;
  LatticeWeight weight;
  GetLinearSymbolSequence(decoded, align, &osymbols, &weight);
  BaseFloat likelihood = -(weight.Value1()+weight.Value2());

  return likelihood;
}

BaseFloat GmmGop::ComputeGopNumera(DecodableAmDiagGmmScaled &decodable,
                                   std::vector<int32> &align,
                                   MatrixIndexT start_frame,
                                   int32 size) {
  KALDI_ASSERT(start_frame + size <= align.size());
  BaseFloat likelihood = 0;
  for (MatrixIndexT frame = start_frame; frame < start_frame + size; frame++) {
    likelihood += decodable.LogLikelihood(frame, align[frame]);
  }

  return likelihood / size;
}

BaseFloat GmmGop::ComputeGopNumeraViterbi(DecodableAmDiagGmmScaled &decodable,
                                          std::vector<int32> &align_in_phone) {
  fst::VectorFst<fst::StdArc> phonelinear_fst;
  MakeLinearAcceptor(align_in_phone, &phonelinear_fst);
  fst::VectorFst<fst::StdArc> fst;
  CompileGraph(phonelinear_fst, &fst);
  BaseFloat likelihood = Decode(fst, decodable);

  return likelihood / align_in_phone.size();
}

BaseFloat GmmGop::ComputeGopDenomin(DecodableAmDiagGmmScaled &decodable,
                                    std::vector<int32> &align_in_phone) {
  fst::VectorFst<fst::StdArc> phoneloop_fst;
  MakePhoneLoopAcceptor(align_in_phone, &phoneloop_fst);
  fst::VectorFst<fst::StdArc> fst;
  CompileGraph(phoneloop_fst, &fst);
  BaseFloat likelihood = Decode(fst, decodable);

  return likelihood / align_in_phone.size();
}

void GmmGop::Compute(const Matrix<BaseFloat> &feats,
                     const std::vector<int32> &transcript) {
  // Align
  fst::VectorFst<fst::StdArc> ali_fst;
  TrainingGraphCompiler gc(tm_, ctx_dep_, lex_fst_, disambig_syms_, gopts_);
  gc.CompileGraphFromText(transcript, &ali_fst);
  DecodableAmDiagGmmScaled ali_decodable(am_, tm_, feats, 1.0);
  std::vector<int32> align;
  Decode(ali_fst, ali_decodable, &align);
  KALDI_ASSERT(feats.NumRows() == align.size());

  // GOP
  std::vector<std::vector<int32> > split;
  SplitToPhones(tm_, align, &split);
  gop_result_.Resize(split.size());
  int32 frame_start_idx = 0;
  for (MatrixIndexT i = 0; i < split.size(); i++) {
    SubMatrix<BaseFloat> feats_in_phone = feats.Range(frame_start_idx, split[i].size(),
                                                      0, feats.NumCols());
    const Matrix<BaseFloat> features(feats_in_phone);
    DecodableAmDiagGmmScaled gmm_decodable(am_, tm_, features, 1.0);

    bool use_viterbi_numera = true;
    BaseFloat gop_numerator = use_viterbi_numera ?
                                ComputeGopNumeraViterbi(gmm_decodable, split[i]):
                                ComputeGopNumera(ali_decodable, align,
                                                 frame_start_idx, split[i].size());
    BaseFloat gop_denominator = ComputeGopDenomin(gmm_decodable, split[i]);
    gop_result_(i) = gop_numerator - gop_denominator;

    frame_start_idx += split[i].size();
  }
}

Vector<BaseFloat>& GmmGop::Result() {
  return gop_result_;
}

}  // End namespace kaldi
