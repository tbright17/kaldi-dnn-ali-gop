// gop/gop-gmm.h

// Copyright 2016-2017  Junbo Zhang
//                      Ming Tu

// This program based on Kaldi (https://github.com/kaldi-asr/kaldi).
// However, this program is NOT UNDER THE SAME LICENSE of Kaldi's.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// version 2 as published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#ifndef KALDI_GOP_GMM_H_
#define KALDI_GOP_GMM_H_ 1

#include "base/kaldi-common.h"
#include "util/common-utils.h"
#include "gmm/am-diag-gmm.h"
#include "decoder/training-graph-compiler.h"
#include "gmm/decodable-am-diag-gmm.h"
#include "hmm/transition-model.h"
#include "decoder/faster-decoder.h"
#include "decoder/lattice-faster-decoder.h"
#include "fstext/fstext-utils.h"

namespace kaldi {

class GmmGop2 {
public:
  GmmGop2() {}
  void Init(std::string &tree_in_filename,
            std::string &model_in_filename,
            std::string &lex_in_filename,
            std::string &l1_model_in_filename);
  void Compute(const Matrix<BaseFloat> &feats, const std::vector<int32> &transcript, const std::string fst_in_str);
  Vector<BaseFloat>& Result();
  Vector<BaseFloat>& l1_Result();
  std::vector<int32>& phn_seq();

protected:
  AmDiagGmm am_, l1_am_;
  TransitionModel tm_, l1_tm_;
  ContextDependency ctx_dep_;
  TrainingGraphCompiler *gc_;
  std::map<int32, int32> pdfid_to_tid;
  std::vector<int32> l1_phnseq_;
  Vector<BaseFloat> gop_result_, l1_gop_result_;

  BaseFloat Decode(fst::VectorFst<fst::StdArc> &fst,
                   DecodableAmDiagGmmScaled &decodable,
                   std::vector<int32> *align = NULL);
  BaseFloat ComputeGopNumera(DecodableAmDiagGmmScaled &decodable,
                             std::vector<int32> &align,
                             MatrixIndexT start_frame,
                             int32 size);
  BaseFloat ComputeGopNumeraViterbi(DecodableAmDiagGmmScaled &decodable,
                                    int32 phone_l, int32 phone, int32 phone_r);
  BaseFloat ComputeGopDenominViterbi(DecodableAmDiagGmmScaled &decodable,
                              int32 phone_l, int32 phone_r);
  BaseFloat ComputeL1Gop(LatticeFasterDecoder &decoder, DecodableAmDiagGmmScaled &decodable, int32 index);
  void GetContextFromSplit(std::vector<std::vector<int32> > split,
                           int32 index, int32 &phone_l, int32 &phone, int32 &phone_r);
};

}  // End namespace kaldi

#endif  // KALDI_GOP_GMM_H_
