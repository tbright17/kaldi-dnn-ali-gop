// gop/gop-gmm.cc

// Copyright 2006  Junbo Zhang                    

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
#include "fstext/fstext-utils.h"
#include "decoder/decoder-wrappers.h"
#include "decoder/training-graph-compiler.h"
#include "gmm/decodable-am-diag-gmm.h"
#include "lat/kaldi-lattice.h"
#include "gop/gmm-gop.h"

namespace kaldi {

void GmmGop::Compute(const AmDiagGmm &am,
               const TransitionModel &tm,
               const Matrix<BaseFloat> &feats,
               const std::vector<int32> &align) {


}

void GmmGop::Write(std::ostream &out_stream, bool binary) const {

}

}  // End namespace kaldi
