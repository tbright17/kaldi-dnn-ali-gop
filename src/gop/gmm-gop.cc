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
#include "hmm/hmm-utils.h"
#include "gop/gmm-gop.h"

namespace kaldi {

BaseFloat GmmGop::ComputeGopNumera(SubMatrix<BaseFloat> &feats_in_phone,
                                   std::vector<int32> &align_in_phone) {
  int32 num_repeats = align_in_phone.size();
  KALDI_ASSERT(feats_in_phone.NumRows() == num_repeats);

  return 0;
}

BaseFloat GmmGop::ComputeGopDenomin(SubMatrix<BaseFloat> &feats_in_phone,
                                    std::vector<int32> &align_in_phone) {
  // int32 phone = tm_.TransitionIdToPhone(align_in_phone[0]);
  int32 num_repeats = align_in_phone.size();
  KALDI_ASSERT(feats_in_phone.NumRows() == num_repeats);

  return 0;
}

void GmmGop::Compute(const Matrix<BaseFloat> &feats,
                     const std::vector<int32> &align) {
  KALDI_ASSERT(feats.NumRows() == align.size());

  std::vector<std::vector<int32> > split;
  SplitToPhones(tm_, align, &split);
  int32 frame_start_idx = 0;
  for (size_t i = 0; i < split.size(); i++) {
    SubMatrix<BaseFloat> feats_in_phone = feats.Range(frame_start_idx, split[i].size(),
                                                      0, feats.NumCols());
    BaseFloat gop_numerator = ComputeGopNumera(feats_in_phone, split[i]);
    BaseFloat gop_denominator = ComputeGopDenomin(feats_in_phone, split[i]);
    gop_result_.push_back(gop_numerator - gop_denominator);

    frame_start_idx += split[i].size();
  }
}

void GmmGop::Write(std::ostream &out_stream, bool binary) const {

}

}  // End namespace kaldi
