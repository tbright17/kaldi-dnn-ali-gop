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
  fst::VectorFst<fst::StdArc> *lex_fst_ = fst::ReadFstKaldi(lex_in_filename);
  ReadKaldiObject(tree_in_filename, &ctx_dep_);
  TrainingGraphCompilerOptions gopts;
  std::vector<int32> disambig_syms;
  gc_ = new TrainingGraphCompiler(tm_, ctx_dep_, lex_fst_, disambig_syms, gopts);
  decode_opts_.beam = 200;
}

BaseFloat GmmGop::ComputeGopNumera(DecodableAmDiagGmmScaled &decodable,
                                   std::vector<int32> &align_in_phone) {
  int32 num_repeats = align_in_phone.size();
  BaseFloat likelihood = 0;
  for (MatrixIndexT frame=0; frame<num_repeats; frame++) {
    likelihood += decodable.LogLikelihood(frame, align_in_phone[frame]);
  }
  likelihood /= num_repeats;

  return likelihood;
}

BaseFloat GmmGop::ComputeGopDenomin(DecodableAmDiagGmmScaled &decodable,
                                    std::vector<int32> &align_in_phone) {
  using fst::SymbolTable;
  using fst::VectorFst;
  using fst::StdArc;

  int32 num_repeats = align_in_phone.size();

  // int32 phone = tm_.TransitionIdToPhone(align_in_phone[0]);
  return 1.0;

  VectorFst<StdArc> fst;
  FasterDecoderOptions decode_opts;
  FasterDecoder decoder(fst, decode_opts);
  decoder.Decode(&decodable);
  if (! decoder.ReachedFinal()) {
    KALDI_ERR << "Did not successfully decode " << ", len = " << decodable.NumFramesReady();
  }
  fst::VectorFst<LatticeArc> decoded;
  decoder.GetBestPath(&decoded);
  std::vector<int32> alignment;
  std::vector<int32> words;
  LatticeWeight weight;
  GetLinearSymbolSequence(decoded, &alignment, &words, &weight);
  BaseFloat like = -(weight.Value1()+weight.Value2());

  return like;
}

void GmmGop::AlignUtterance(fst::VectorFst<fst::StdArc> *fst,
                            DecodableInterface *decodable,
                            std::vector<int32> *align) {
  FasterDecoder decoder(*fst, decode_opts_);
  decoder.Decode(decodable);
  bool ans = decoder.ReachedFinal();
  if (!ans) {
    KALDI_WARN << "Did not successfully decode file ";
  }
  fst::VectorFst<LatticeArc> decoded;
  decoder.GetBestPath(&decoded);
  std::vector<int32> words;
  LatticeWeight weight;
  GetLinearSymbolSequence(decoded, align, &words, &weight);
}

void GmmGop::Compute(const Matrix<BaseFloat> &feats,
                     const std::vector<int32> &transcript) {
  // Align
  fst::VectorFst<fst::StdArc> ali_fst;
  if (! gc_->CompileGraphFromText(transcript, &ali_fst)) {
    KALDI_ERR << "Problem creating decoding graph for utterance ";
  }
  DecodableAmDiagGmmScaled ali_decodable(am_, tm_, feats, 1.0);
  std::vector<int32> align;
  AlignUtterance(&ali_fst, &ali_decodable, &align);
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

    BaseFloat gop_numerator = ComputeGopNumera(gmm_decodable, split[i]);
    BaseFloat gop_denominator = ComputeGopDenomin(gmm_decodable, split[i]);
    gop_result_(i) = gop_numerator - gop_denominator;

    frame_start_idx += split[i].size();
  }
}
Vector<BaseFloat>& GmmGop::Result() {
  return gop_result_;
}

}  // End namespace kaldi
