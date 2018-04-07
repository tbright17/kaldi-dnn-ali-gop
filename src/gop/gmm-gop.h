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
#include "fstext/fstext-utils.h"

namespace kaldi {

class GmmGop {
public:
  GmmGop() {}
  void Init(std::string &tree_in_filename,
            std::string &model_in_filename,
            std::string &lex_in_filename);
  void Compute(const Matrix<BaseFloat> &feats, const std::vector<int32> &transcript,
              BaseFloatMatrixWriter *phn_conf_writer,
              BaseFloatMatrixWriter *phn_frame_conf_writer);
  Vector<BaseFloat>& Result();
  std::vector<int32>& get_alignment();
  std::vector<int32>& Phonemes();
  Matrix<BaseFloat>& PhonemesConf();
  Matrix<BaseFloat>& PhonemesFrameConf();
  Vector<BaseFloat>& get_phn_ll();

protected:
  AmDiagGmm am_;
  TransitionModel tm_;
  ContextDependency ctx_dep_;
  TrainingGraphCompiler *gc_;
  std::map<int32, int32> pdfid_to_tid;
  Vector<BaseFloat> gop_result_;
  std::vector<int32> alignment_;
  std::vector<int32> phones_;
  Matrix<BaseFloat> phonemes_conf_;
  Matrix<BaseFloat> phonemes_frame_conf_;
  Vector<BaseFloat> phones_loglikelihood_; // phoneme log likelihood

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
  Vector<BaseFloat> ComputePhonemesConf(DecodableAmDiagGmmScaled &decodable,
                                    int32 phone_l, int32 phone_r,
                                    MatrixIndexT start_frame,
                                   int32 size);
  void ComputeFramePhonemesConf(DecodableAmDiagGmmScaled &decodable,
                                    int32 phone_l, int32 phone_r,
                                    MatrixIndexT start_frame,
                                   int32 size);
  void GetContextFromSplit(std::vector<std::vector<int32> > split,
                           int32 index, int32 &phone_l, int32 &phone, int32 &phone_r);
};

}  // End namespace kaldi

#endif  // KALDI_GOP_GMM_H_
