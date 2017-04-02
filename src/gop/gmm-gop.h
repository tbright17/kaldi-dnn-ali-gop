// gop/gop-gmm.h

// Copyright 2006  Junbo Zhang                    

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

#include "gmm/am-diag-gmm.h"
#include "hmm/transition-model.h"

namespace kaldi {

class GmmGop {
 public:
  GmmGop(AmDiagGmm &am, TransitionModel &tm) : am_(am), tm_(tm) {}
  void Compute(const Matrix<BaseFloat> &feats, const std::vector<int32> &align);
  Vector<BaseFloat>& Result();

 protected:
  AmDiagGmm &am_;
  TransitionModel &tm_;

  Vector<BaseFloat> gop_result_;

  BaseFloat ComputeGopNumera(SubMatrix<BaseFloat> &feats_in_phone,
                             std::vector<int32> &align_in_phone);
  BaseFloat ComputeGopDenomin(SubMatrix<BaseFloat> &feats_in_phone,
                              std::vector<int32> &align_in_phone);
};

}  // End namespace kaldi

#endif  // KALDI_GOP_GMM_H_
