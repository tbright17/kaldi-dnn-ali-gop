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

typedef typename fst::StdArc Arc;
typedef typename Arc::StateId StateId;
typedef typename Arc::Weight Weight;

void GmmGop::Init(std::string &tree_in_filename,
            std::string &model_in_filename,
            std::string &lex_in_filename) {
  bool binary;
  Input ki(model_in_filename, &binary);
  tm_.Read(ki.Stream(), binary);
  am_.Read(ki.Stream(), binary);
  ReadKaldiObject(tree_in_filename, &ctx_dep_);

  fst::VectorFst<fst::StdArc> *lex_fst = fst::ReadFstKaldi(lex_in_filename);
  std::vector<int32> disambig_syms;  
  TrainingGraphCompilerOptions gopts;
  gc_ = new TrainingGraphCompiler(tm_, ctx_dep_, lex_fst, disambig_syms, gopts);

  for (size_t i = 0; i < tm_.NumTransitionIds(); i++) {
    pdfid_to_tid[tm_.TransitionIdToPdf(i)] = i;
  }

  decode_opts_.beam = 200;
}

void GmmGop::MakePhoneLoopAcceptor(std::vector<int32> &labels,
                                   fst::VectorFst<fst::StdArc> *ofst) {
  // TODO: make acceptor according phone contexts
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

  return likelihood;
}

BaseFloat GmmGop::ComputeGopNumeraViterbi(DecodableAmDiagGmmScaled &decodable,
                                          std::vector<int32> &align_in_phone) {
  fst::VectorFst<fst::StdArc> fst;
  StateId cur_state = fst.AddState();
  fst.SetStart(cur_state);
  for (size_t i = 0; i < align_in_phone.size(); i++) {
    StateId next_state = fst.AddState();
    Arc arc(align_in_phone[i], 0, Weight::One(), next_state);
    fst.AddArc(cur_state, arc);
    cur_state = next_state;
  }
  fst.SetFinal(cur_state, Weight::One());

  return Decode(fst, decodable);
}

BaseFloat GmmGop::ComputeGopDenomin(DecodableAmDiagGmmScaled &decodable,
                                    int32 phone_l, int32 phone_r) {
  KALDI_ASSERT(ctx_dep_.ContextWidth() == 3);
  KALDI_ASSERT(ctx_dep_.CentralPosition() == 1);
  std::vector<int32> phoneseq(3);
  phoneseq[0] = phone_l;
  phoneseq[2] = phone_r;

  fst::VectorFst<fst::StdArc> fst;
  StateId start_state = fst.AddState();
  fst.SetStart(start_state);

  const std::vector<int32> &phone_syms = tm_.GetPhones();
  for (size_t i = 0; i < phone_syms.size(); i++) {
    int32 phone = phone_syms[i];
    phoneseq[1] = phone;
    const int pdfclass_num = tm_.GetTopo().NumPdfClasses(phone);
    StateId cur_state = start_state;
    for (size_t c = 0; c < pdfclass_num; c++) {
      int32 pdf_id;
      KALDI_ASSERT(ctx_dep_.Compute(phoneseq, c, &pdf_id));
      int32 tid = pdfid_to_tid[pdf_id];

      StateId next_state = fst.AddState();
      Arc arc(tid, 0, Weight::One(), next_state);
      fst.AddArc(cur_state, arc);
      cur_state = next_state;

      Arc arc_selfloop(tid, 0, Weight::One(), cur_state);
      fst.AddArc(cur_state, arc_selfloop);
    }
    Arc arc(0, 0, Weight::One(), start_state);
    fst.AddArc(cur_state, arc);
  }
  fst.SetFinal(start_state, Weight::One());

  return Decode(fst, decodable);
}

void GmmGop::GetContextFromSplit(std::vector<std::vector<int32> > split,
                                 int32 index, int32 &phone_l, int32 &phone_r) {
  KALDI_ASSERT(index < split.size());
  phone_l = (index > 0) ? tm_.TransitionIdToPhone(split[index-1][0]) : 1;
  phone_r = (index < split.size() - 1) ? tm_.TransitionIdToPhone(split[index+1][0]): 1;
}

void GmmGop::Compute(const Matrix<BaseFloat> &feats,
                     const std::vector<int32> &transcript) {
  // Align
  fst::VectorFst<fst::StdArc> ali_fst;
  gc_->CompileGraphFromText(transcript, &ali_fst);
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

    int32 phone, phone_l, phone_r;
    GetContextFromSplit(split, i, phone_l, phone_r);
    BaseFloat gop_denominator = ComputeGopDenomin(gmm_decodable, phone_l, phone_r);
    gop_result_(i) = (gop_numerator - gop_denominator) / split[i].size();

    frame_start_idx += split[i].size();
  }
}

Vector<BaseFloat>& GmmGop::Result() {
  return gop_result_;
}

}  // End namespace kaldi
