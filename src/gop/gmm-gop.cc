// gop/gop-gmm.cc

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

typedef typename fst::StdArc Arc;
typedef typename Arc::StateId StateId;
typedef typename Arc::Weight Weight;

void GmmGop::Init(std::string &tree_in_filename,
            std::string &model_in_filename,
            std::string &lex_in_filename) {
  bool binary;
  Input ki(model_in_filename, &binary);
  tm_.Read(ki.Stream(), binary);
  am_.Read(ki.Stream(), binary);
  ReadKaldiObject(tree_in_filename, &ctx_dep_);

  fst::VectorFst<fst::StdArc> *lex_fst = fst::ReadFstKaldi(lex_in_filename);
  std::vector<int32> disambig_syms;  
  TrainingGraphCompilerOptions gopts;
  gc_ = new TrainingGraphCompiler(tm_, ctx_dep_, lex_fst, disambig_syms, gopts);

  for (size_t i = 0; i < tm_.NumTransitionIds(); i++) {
    pdfid_to_tid[tm_.TransitionIdToPdf(i)] = i;
  }
}

BaseFloat GmmGop::Decode(fst::VectorFst<fst::StdArc> &fst,
                         DecodableAmDiagGmmScaled &decodable,
                         std::vector<int32> *align) {
  FasterDecoderOptions decode_opts;
  decode_opts.beam = 2000; // number of beams for decoding. Larger, slower and more successful alignments. Smaller, more unsuccessful alignments.
  FasterDecoder decoder(fst, decode_opts);
  decoder.Decode(&decodable);
  if (! decoder.ReachedFinal()) {
    KALDI_WARN << "Did not successfully decode.";
  }
  fst::VectorFst<LatticeArc> decoded;
  decoder.GetBestPath(&decoded);
  std::vector<int32> osymbols;
  LatticeWeight weight;
  GetLinearSymbolSequence(decoded, align, &osymbols, &weight);
  BaseFloat likelihood = -(weight.Value1()+weight.Value2());

  return likelihood;
}

BaseFloat GmmGop::ComputeGopNumera(DecodableAmDiagGmmScaled &decodable,
                                   std::vector<int32> &align,
                                   MatrixIndexT start_frame,
                                   int32 size) {
  KALDI_ASSERT(start_frame + size <= align.size());
  BaseFloat likelihood = 0;
  for (MatrixIndexT frame = start_frame; frame < start_frame + size; frame++) {
    likelihood += decodable.LogLikelihood(frame, align[frame]);
  }

  return likelihood;
}

BaseFloat GmmGop::ComputeGopNumeraViterbi(DecodableAmDiagGmmScaled &decodable,
                                          int32 phone_l, int32 phone, int32 phone_r) {
  KALDI_ASSERT(ctx_dep_.ContextWidth() == 3);
  KALDI_ASSERT(ctx_dep_.CentralPosition() == 1);
  std::vector<int32> phoneseq(3);
  phoneseq[0] = phone_l;
  phoneseq[1] = phone;
  phoneseq[2] = phone_r;

  fst::VectorFst<fst::StdArc> fst;
  StateId cur_state = fst.AddState();
  fst.SetStart(cur_state);
  for (size_t c = 0; c < tm_.GetTopo().NumPdfClasses(phone); c++) {
    int32 pdf_id;
    //KALDI_ASSERT(ctx_dep_.Compute(phoneseq, c, &pdf_id));
    if (!ctx_dep_.Compute(phoneseq, c, &pdf_id)) {
      KALDI_ERR << "Failed to obtain pdf_id";
    }
    int32 tid = pdfid_to_tid[pdf_id];

    StateId next_state = fst.AddState();
    Arc arc(tid, 0, Weight::One(), next_state);
    fst.AddArc(cur_state, arc);
    cur_state = next_state;

    Arc arc_selfloop(tid, 0, Weight::One(), cur_state);
    fst.AddArc(cur_state, arc_selfloop);
  }
  fst.SetFinal(cur_state, Weight::One());

  return Decode(fst, decodable);
}

BaseFloat GmmGop::ComputeGopDenominViterbi(DecodableAmDiagGmmScaled &decodable,
                                    int32 phone_l, int32 phone_r) {
  KALDI_ASSERT(ctx_dep_.ContextWidth() == 3);
  KALDI_ASSERT(ctx_dep_.CentralPosition() == 1);
  std::vector<int32> phoneseq(3);
  phoneseq[0] = phone_l;
  phoneseq[2] = phone_r;

  fst::VectorFst<fst::StdArc> fst;
  StateId start_state = fst.AddState();
  fst.SetStart(start_state);

  const std::vector<int32> &phone_syms = tm_.GetPhones();
  for (size_t i = 0; i < phone_syms.size(); i++) {
    int32 phone = phone_syms[i];
    phoneseq[1] = phone;
    const int pdfclass_num = tm_.GetTopo().NumPdfClasses(phone);
    StateId cur_state = start_state;
    for (size_t c = 0; c < pdfclass_num; c++) {
      int32 pdf_id;
      //KALDI_ASSERT(ctx_dep_.Compute(phoneseq, c, &pdf_id));
      if (!ctx_dep_.Compute(phoneseq, c, &pdf_id)) {
        KALDI_ERR << "Failed to obtain pdf_id";
      }
      int32 tid = pdfid_to_tid[pdf_id];

      StateId next_state = fst.AddState();
      Arc arc(tid, 0, Weight::One(), next_state);
      fst.AddArc(cur_state, arc);
      cur_state = next_state;

      Arc arc_selfloop(tid, 0, Weight::One(), cur_state);
      fst.AddArc(cur_state, arc_selfloop);
    }
    Arc arc(0, 0, Weight::One(), start_state);
    fst.AddArc(cur_state, arc);
  }
  fst.SetFinal(start_state, Weight::One());

  return Decode(fst, decodable);
}

Vector<BaseFloat> GmmGop::ComputePhonemesConf(DecodableAmDiagGmmScaled &decodable,
                                    int32 phone_l, int32 phone_r,
                                    MatrixIndexT start_frame,
                                   int32 size) {
//   KALDI_ASSERT(ctx_dep_.ContextWidth() == 3);
//   KALDI_ASSERT(ctx_dep_.CentralPosition() == 1);
//   std::vector<int32> phoneseq(3);
//   phoneseq[0] = phone_l;
//   phoneseq[2] = phone_r;

//   const std::vector<int32> &phone_syms = tm_.GetPhones();

//   Vector<BaseFloat> likelihood(phone_syms.size());

//   for (size_t i = 0; i < phone_syms.size(); i++) {
//     int32 phone = phone_syms[i];
//     phoneseq[1] = phone;
//     const int pdfclass_num = tm_.GetTopo().NumPdfClasses(phone);
    
//     BaseFloat phn_likelihood = 0.0;
//     for (MatrixIndexT frame = start_frame; frame < start_frame + size; frame++) {
//       Vector<BaseFloat> temp_likelihood(pdfclass_num);
//       for (size_t c = 0; c < pdfclass_num; c++) {
//         int32 pdf_id;
//         KALDI_ASSERT(ctx_dep_.Compute(phoneseq, c, &pdf_id));
//         int32 tid = pdfid_to_tid[pdf_id];

//         temp_likelihood(c) = decodable.LogLikelihood(frame, tid+1); 
//       }
//       phn_likelihood += temp_likelihood.LogSumExp(5);
//     }

//     likelihood(i) = phn_likelihood / size;
//   }

//   return likelihood;

  KALDI_ASSERT(ctx_dep_.ContextWidth() == 3);
  KALDI_ASSERT(ctx_dep_.CentralPosition() == 1);
  std::vector<int32> phoneseq(3);
  phoneseq[0] = phone_l;
  phoneseq[2] = phone_r;

  const std::vector<int32> &phone_syms = tm_.GetPhones();
  Vector<BaseFloat> likelihood(phone_syms.size());
  for (size_t i = 0; i < phone_syms.size(); i++) {

    fst::VectorFst<fst::StdArc> fst;
    StateId start_state = fst.AddState();
    fst.SetStart(start_state);

    int32 phone = phone_syms[i];
    phoneseq[1] = phone;
    const int pdfclass_num = tm_.GetTopo().NumPdfClasses(phone);
    StateId cur_state = start_state;
    for (size_t c = 0; c < pdfclass_num; c++) {
      int32 pdf_id;
      //KALDI_ASSERT(ctx_dep_.Compute(phoneseq, c, &pdf_id));
      if (!ctx_dep_.Compute(phoneseq, c, &pdf_id)) {
        KALDI_ERR << "Failed to obtain pdf_id";
      }
      int32 tid = pdfid_to_tid[pdf_id];

      StateId next_state = fst.AddState();
      Arc arc(tid, 0, Weight::One(), next_state);
      fst.AddArc(cur_state, arc);
      cur_state = next_state;

      Arc arc_selfloop(tid, 0, Weight::One(), cur_state);
      fst.AddArc(cur_state, arc_selfloop);
    }
    fst.SetFinal(cur_state, Weight::One());
    likelihood(i) = Decode(fst, decodable)/size;
  }
  return likelihood;
}

void GmmGop::ComputeFramePhonemesConf(DecodableAmDiagGmmScaled &decodable,
                                    int32 phone_l, int32 phone_r,
                                    MatrixIndexT start_frame,
                                   int32 size) {
  KALDI_ASSERT(ctx_dep_.ContextWidth() == 3);
  KALDI_ASSERT(ctx_dep_.CentralPosition() == 1);
  std::vector<int32> phoneseq(3);
  phoneseq[0] = phone_l;
  phoneseq[2] = phone_r;

  const std::vector<int32> &phone_syms = tm_.GetPhones();

  for (MatrixIndexT frame = start_frame; frame < start_frame + size; frame++) {
    Vector<BaseFloat> likelihood(phone_syms.size());
    for (size_t i = 0; i < phone_syms.size(); i++) {
      int32 phone = phone_syms[i];
      phoneseq[1] = phone;
      const int pdfclass_num = tm_.GetTopo().NumPdfClasses(phone);

      Vector<BaseFloat> temp_likelihood(pdfclass_num);
      for (size_t c = 0; c < pdfclass_num; c++) {
        int32 pdf_id;
        //KALDI_ASSERT(ctx_dep_.Compute(phoneseq, c, &pdf_id));
        if (!ctx_dep_.Compute(phoneseq, c, &pdf_id)) {
          KALDI_ERR << "Failed to obtain pdf_id";
        }
        int32 tid = pdfid_to_tid[pdf_id];

        temp_likelihood(c) = decodable.LogLikelihood(frame, tid+1); 
      }
      likelihood(i) = temp_likelihood.LogSumExp(5);
    }
    phonemes_frame_conf_.CopyRowFromVec(likelihood, frame);
  }

}

void GmmGop::GetContextFromSplit(std::vector<std::vector<int32> > split,
                                 int32 index, int32 &phone_l, int32 &phone, int32 &phone_r) {
  KALDI_ASSERT(index < split.size());
  phone_l = (index > 0) ? tm_.TransitionIdToPhone(split[index-1][0]) : 1;
  phone = tm_.TransitionIdToPhone(split[index][0]);
  phone_r = (index < split.size() - 1) ? tm_.TransitionIdToPhone(split[index+1][0]): 1;
}

void GmmGop::Compute(const Matrix<BaseFloat> &feats,
                     const std::vector<int32> &transcript,
                     BaseFloatMatrixWriter *phn_conf_writer,
                     BaseFloatMatrixWriter *phn_frame_conf_writer) {
  // Align
  fst::VectorFst<fst::StdArc> ali_fst;
  gc_->CompileGraphFromText(transcript, &ali_fst);
  DecodableAmDiagGmmScaled ali_decodable(am_, tm_, feats, 1.0); // 1.0 is the acoustic scale
  //std::vector<int32> align;
  Decode(ali_fst, ali_decodable, &alignment_);
  KALDI_ASSERT(feats.NumRows() == alignment_.size());

  // GOP
  const std::vector<int32> &phone_syms = tm_.GetPhones();

  std::vector<std::vector<int32> > split;
  SplitToPhones(tm_, alignment_, &split);
  gop_result_.Resize(split.size());
  phones_.resize(split.size());
  phones_loglikelihood_.Resize(split.size());
  phonemes_conf_.Resize(split.size(),phone_syms.size());
  phonemes_frame_conf_.Resize(feats.NumRows(), phone_syms.size());
  int32 frame_start_idx = 0;
  for (MatrixIndexT i = 0; i < split.size(); i++) {
    SubMatrix<BaseFloat> feats_in_phone = feats.Range(frame_start_idx, split[i].size(),
                                                      0, feats.NumCols());
    const Matrix<BaseFloat> features(feats_in_phone);
    DecodableAmDiagGmmScaled split_decodable(am_, tm_, features, 1.0); // 1.0 is the acoustic scale

    int32 phone, phone_l, phone_r;
    GetContextFromSplit(split, i, phone_l, phone, phone_r);

    bool use_viterbi_numera = true;
    BaseFloat gop_numerator = use_viterbi_numera ?
                                ComputeGopNumeraViterbi(split_decodable, phone_l, phone, phone_r):
                                ComputeGopNumera(ali_decodable, alignment_,
                                                 frame_start_idx, split[i].size());
    BaseFloat gop_denominator = ComputeGopDenominViterbi(split_decodable, phone_l, phone_r);
    gop_result_(i) = (gop_numerator - gop_denominator) / split[i].size();
    phones_[i] = phone;
    phones_loglikelihood_(i) = gop_numerator;
    if (phn_conf_writer != NULL && phn_conf_writer->IsOpen()) {
      phonemes_conf_.CopyRowFromVec(ComputePhonemesConf(split_decodable,phone_l, phone_r,frame_start_idx, split[i].size()), i);
    }
    if (phn_frame_conf_writer != NULL && phn_frame_conf_writer->IsOpen()) {
      ComputeFramePhonemesConf(ali_decodable, phone_l, phone_r,frame_start_idx, split[i].size());
    }

    frame_start_idx += split[i].size();
  }
}

Vector<BaseFloat>& GmmGop::Result() {
  return gop_result_;
}

Vector<BaseFloat>& GmmGop::get_phn_ll() {
  return phones_loglikelihood_;
}

std::vector<int32>& GmmGop::get_alignment() {
  return alignment_;
}

std::vector<int32>& GmmGop::Phonemes() {
  return phones_;
}

Matrix<BaseFloat>& GmmGop::PhonemesConf() {
  return phonemes_conf_;
}

Matrix<BaseFloat>& GmmGop::PhonemesFrameConf() {
  return phonemes_frame_conf_;
}

}  // End namespace kaldi
