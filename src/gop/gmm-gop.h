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
  GmmGop() { }

  void Compute(const AmDiagGmm &am,
               const TransitionModel &tm,
               const Matrix<BaseFloat> &feats,
               const std::vector<int32> &align);

  void Write(std::ostream &os, bool binary) const;
  void Read(std::istream &in, bool binary);
};

}  // End namespace kaldi

#endif  // KALDI_GOP_GMM_H_
