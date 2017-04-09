// gop/gop-gmm.h

// Copyright 2016-2017  Junbo Zhang

// This project based on Kaldi (https://github.com/kaldi-asr/kaldi).
// All the Kaldi's codes in this project are under the Apache License, Version 2.0.
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
// 
// However, this file has not been merged into Kaldi's master branch,
// and the codes in this file are NOT UNDER THE SAME LICENSE of Kaldi's.
// The codes in this file are NOT ALLOWED TO USE, COPY, DISTRIBUTE, OR MODIFY
// unless being permitted by the author.

#ifndef KALDI_GOP_GMM_H_
#define KALDI_GOP_GMM_H_ 1

#include "base/kaldi-common.h"
#include "util/common-utils.h"
#include "gmm/am-diag-gmm.h"
#include "decoder/training-graph-compiler.h"
#include "gmm/decodable-am-diag-gmm.h"
#include "hmm/transition-model.h"
#include "decoder/faster-decoder.h"
#include "fstext/fstext-utils.h"

namespace kaldi {

class GmmGop {
public:
  GmmGop() {}
  void Init(std::string &tree_in_filename,
            std::string &model_in_filename,
            std::string &lex_in_filename);
  void Compute(const Matrix<BaseFloat> &feats, const std::vector<int32> &transcript);
  Vector<BaseFloat>& Result();

protected:
  AmDiagGmm am_;
  TransitionModel tm_;
  ContextDependency ctx_dep_;
  TrainingGraphCompiler *gc_;
  std::map<int32, int32> pdfid_to_tid;
  Vector<BaseFloat> gop_result_;

  void MakePhoneLoopAcceptor(std::vector<int32> &labels,
                             fst::VectorFst<fst::StdArc> *ofst);
  BaseFloat Decode(fst::VectorFst<fst::StdArc> &fst,
                   DecodableAmDiagGmmScaled &decodable,
                   std::vector<int32> *align = NULL);
  BaseFloat ComputeGopNumera(DecodableAmDiagGmmScaled &decodable,
                             std::vector<int32> &align,
                             MatrixIndexT start_frame,
                             int32 size);
  BaseFloat ComputeGopNumeraViterbi(DecodableAmDiagGmmScaled &decodable,
                                    int32 phone_l, int32 phone, int32 phone_r);
  BaseFloat ComputeGopDenomin(DecodableAmDiagGmmScaled &decodable,
                              int32 phone_l, int32 phone_r);
  void GetContextFromSplit(std::vector<std::vector<int32> > split,
                           int32 index, int32 &phone_l, int32 &phone, int32 &phone_r);
};

}  // End namespace kaldi

#endif  // KALDI_GOP_GMM_H_
