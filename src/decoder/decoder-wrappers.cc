// decoder/decoder-wrappers.cc

// Copyright 2014  Johns Hopkins University (author: Daniel Povey)

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

#include "decoder/decoder-wrappers.h"
#include "decoder/faster-decoder.h"

namespace kaldi {

// see comment in header.
void ModifyGraphForCarefulAlignment(
    fst::VectorFst<fst::StdArc> *fst) {
  typedef fst::StdArc Arc;
  typedef Arc::StateId StateId;
  typedef Arc::Label Label;
  typedef Arc::Weight Weight;
  StateId num_states = fst->NumStates();
  if (num_states == 0) {
    KALDI_WARN << "Empty FST input.";
    return;
  }
  Weight zero = Weight::Zero();
  // fst_rhs will be the right hand side of the Concat operation.
  fst::VectorFst<fst::StdArc> fst_rhs(*fst);
  // first remove the final-probs from fst_rhs.
  for (StateId state = 0; state < num_states; state++)
    fst_rhs.SetFinal(state, zero);
  StateId pre_initial = fst_rhs.AddState();
  Arc to_initial(0, 0, Weight::One(), fst_rhs.Start());
  fst_rhs.AddArc(pre_initial, to_initial);
  fst_rhs.SetStart(pre_initial);
  // make the pre_initial state final with probability one;
  // this is equivalent to keeping the final-probs of the first
  // FST when we do concat (otherwise they would get deleted).
  fst_rhs.SetFinal(pre_initial, Weight::One());
  fst::VectorFst<fst::StdArc> fst_concat;
  fst::Concat(fst, fst_rhs);
}

    
void AlignUtteranceWrapper(
    const AlignConfig &config,
    const std::string &utt,
    BaseFloat acoustic_scale,  // affects scores written to scores_writer, if
                               // present
    fst::VectorFst<fst::StdArc> *fst,  // non-const in case config.careful == 
                                       // true.
    DecodableInterface *decodable,  // not const but is really an input.
    Int32VectorWriter *alignment_writer,
    BaseFloatWriter *scores_writer,
    int32 *num_done,
    int32 *num_error,
    int32 *num_retried,
    double *tot_like,
    int64 *frame_count) {

  if ((config.retry_beam != 0 && config.retry_beam <= config.beam) ||
      config.beam <= 0.0) {
    KALDI_ERR << "Beams do not make sense: beam " << config.beam
              << ", retry-beam " << config.retry_beam;
  }

  if (fst->Start() == fst::kNoStateId) {
    KALDI_WARN << "Empty decoding graph for " << utt;
    if (num_error != NULL) (*num_error)++;
    return;
  }


  fst::StdArc::Label special_symbol = 0;
  if (config.careful)
    ModifyGraphForCarefulAlignment(fst);

  FasterDecoderOptions decode_opts;
  decode_opts.beam = config.beam;

  FasterDecoder decoder(*fst, decode_opts);
  decoder.Decode(decodable);

  bool ans = decoder.ReachedFinal();  // consider only final states.
  
  if (!ans && config.retry_beam != 0.0) {
    if (num_retried != NULL) (*num_retried)++;
    KALDI_WARN << "Retrying utterance " << utt << " with beam "
               << config.retry_beam;
    decode_opts.beam = config.retry_beam;
    decoder.SetOptions(decode_opts);
    decoder.Decode(decodable);
    ans = decoder.ReachedFinal();
  }

  if (!ans) {  // Still did not reach final state.
    KALDI_WARN << "Did not successfully decode file " << utt << ", len = "
               << decodable->NumFramesReady();
    if (num_error != NULL) (*num_error)++;
    return;
  }
  
  fst::VectorFst<LatticeArc> decoded;  // linear FST.
  decoder.GetBestPath(&decoded);
  if (decoded.NumStates() == 0) {
    KALDI_WARN << "Error getting best path from decoder (likely a bug)";
    if (num_error != NULL) (*num_error)++;
    return;
  }
    
  std::vector<int32> alignment;
  std::vector<int32> words;
  LatticeWeight weight;

  GetLinearSymbolSequence(decoded, &alignment, &words, &weight);
  BaseFloat like = -(weight.Value1()+weight.Value2()) / acoustic_scale;

  if (num_done != NULL) (*num_done)++;
  if (tot_like != NULL) (*tot_like) += like;
  if (frame_count != NULL) (*frame_count) += decodable->NumFramesReady();

  if (alignment_writer != NULL && alignment_writer->IsOpen())
    alignment_writer->Write(utt, alignment);
  
  if (scores_writer != NULL && scores_writer->IsOpen())
    scores_writer->Write(utt, -(weight.Value1()+weight.Value2()));
}


} // end namespace kaldi.
