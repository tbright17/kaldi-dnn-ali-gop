// decoder/decoder-wrappers.h

// Copyright   2014  Johns Hopkins University (author: Daniel Povey)

// See ../../COPYING for clarification regarding multiple authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
// WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache 2 License for the specific language governing permissions and
// limitations under the License.

#ifndef KALDI_DECODER_DECODER_WRAPPERS_H_
#define KALDI_DECODER_DECODER_WRAPPERS_H_

#include "itf/options-itf.h"
#include "itf/decodable-itf.h"
#include "util/common-utils.h"
#include "fst/fstlib.h"

// This header contains declarations from various convenience functions that are called
// from binary-level programs such as gmm-decode-faster.cc, gmm-align-compiled.cc, and
// so on.

namespace kaldi {


struct AlignConfig {
  BaseFloat beam;
  BaseFloat retry_beam;
  bool careful;

  AlignConfig(): beam(200.0), retry_beam(0.0), careful(false) { }

  void Register(OptionsItf *opts) {
    opts->Register("beam", &beam, "Decoding beam used in alignment");
    opts->Register("retry-beam", &retry_beam,
                   "Decoding beam for second try at alignment");
    opts->Register("careful", &careful,
                   "If true, do 'careful' alignment, which is better at detecting "
                   "alignment failure (involves loop to start of decoding graph).");
  }
};


/// AlignUtteranceWapper is a wrapper for alignment code used in training, that
/// is called from many different binaries, e.g. gmm-align, gmm-align-compiled,
/// sgmm-align, etc.  The writers for alignments and words will only be written
/// to if they are open.  The num_done, num_error, num_retried, tot_like and
/// frame_count pointers will (if non-NULL) be incremented or added to, not set,
/// by this function.
void AlignUtteranceWrapper(
    const AlignConfig &config,
    const std::string &utt,
    BaseFloat acoustic_scale,  // affects scores written to scores_writer, if
                               // present
    fst::VectorFst<fst::StdArc> *fst,  // non-const in case config.careful ==
                                       // true, we add loop.
    DecodableInterface *decodable,  // not const but is really an input.
    Int32VectorWriter *alignment_writer,
    BaseFloatWriter *scores_writer,
    int32 *num_done,
    int32 *num_error,
    int32 *num_retried,
    double *tot_like,
    int64 *frame_count);

} // end namespace kaldi.


#endif
