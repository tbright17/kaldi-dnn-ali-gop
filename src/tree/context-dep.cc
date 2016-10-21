// tree/context-dep.cc

// Copyright 2009-2011  Microsoft Corporation

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

#include "tree/context-dep.h"
#include "base/kaldi-math.h"

namespace kaldi {

bool ContextDependency::Compute(const std::vector<int32> &phoneseq,
                                 int32 pdf_class,
                                 int32 *pdf_id) const {
  KALDI_ASSERT(static_cast<int32>(phoneseq.size()) == N_);
  EventType  event_vec;
  event_vec.reserve(N_+1);
  event_vec.push_back(std::make_pair
                      (static_cast<EventKeyType>(kPdfClass),  // -1
                       static_cast<EventValueType>(pdf_class)));
  KALDI_COMPILE_TIME_ASSERT(kPdfClass < 0);  // or it would not be sorted.
  for (int32 i = 0;i < N_;i++) {
    event_vec.push_back(std::make_pair
                        (static_cast<EventKeyType>(i),
                         static_cast<EventValueType>(phoneseq[i])));
    KALDI_ASSERT(static_cast<EventAnswerType>(phoneseq[i]) >= 0);
  }
  KALDI_ASSERT(pdf_id != NULL);
  return to_pdf_->Map(event_vec, pdf_id);
}


void ContextDependency::Write (std::ostream &os, bool binary) const {
  WriteToken(os, binary, "ContextDependency");
  WriteBasicType(os, binary, N_);
  WriteBasicType(os, binary, P_);
  WriteToken(os, binary, "ToPdf");
  to_pdf_->Write(os, binary);
  WriteToken(os, binary, "EndContextDependency");
}


void ContextDependency::Read (std::istream &is, bool binary) {
  if (to_pdf_) {
    delete to_pdf_;
    to_pdf_ = NULL;
  }
  ExpectToken(is, binary, "ContextDependency");
  ReadBasicType(is, binary, &N_);
  ReadBasicType(is, binary, &P_);
  EventMap *to_pdf = NULL;
  std::string token;
  ReadToken(is, binary, &token);
  if (token == "ToLength") {  // back-compat.
    EventMap *to_num_pdf_classes = EventMap::Read(is, binary);
    delete to_num_pdf_classes;
    ReadToken(is, binary, &token);
  }
  if (token == "ToPdf") {
    to_pdf = EventMap::Read(is , binary);
  } else {
    KALDI_ERR << "Got unexpected token " << token
              << " reading context-dependency object.";
  }
  ExpectToken(is, binary, "EndContextDependency");
  to_pdf_ = to_pdf;
}

void ContextDependency::GetPdfInfo(const std::vector<int32> &phones,
                                   const std::vector<int32> &num_pdf_classes,  // indexed by phone,
                                   std::vector<std::vector<std::pair<int32, int32> > > *pdf_info) const {

  EventType vec;
  KALDI_ASSERT(pdf_info != NULL);
  pdf_info->resize(NumPdfs());
  for (size_t i = 0 ; i < phones.size(); i++) {
    int32 phone = phones[i];
    vec.clear();
    vec.push_back(std::make_pair(static_cast<EventKeyType>(P_),
                                 static_cast<EventValueType>(phone)));
    // Now get length.
    KALDI_ASSERT(static_cast<size_t>(phone) < num_pdf_classes.size());
    EventAnswerType len = num_pdf_classes[phone];

    for (int32 pos = 0; pos < len; pos++) {
      vec.resize(2);
      vec[0] = std::make_pair(static_cast<EventKeyType>(P_),
                              static_cast<EventValueType>(phone));
      vec[1] = std::make_pair(kPdfClass, static_cast<EventValueType>(pos));
      std::sort(vec.begin(), vec.end());
      std::vector<EventAnswerType> pdfs;  // pdfs that can be at this pos as this phone.
      to_pdf_->MultiMap(vec, &pdfs);
      SortAndUniq(&pdfs);
      if (pdfs.empty()) {
        KALDI_WARN << "ContextDependency::GetPdfInfo, no pdfs returned for position "<< pos << " of phone " << phone << ".   Continuing but this is a serious error.";
      }
      for (size_t j = 0; j < pdfs.size(); j++) {
        KALDI_ASSERT(static_cast<size_t>(pdfs[j]) < pdf_info->size());
        (*pdf_info)[pdfs[j]].push_back(std::make_pair(phone, pos));
      }
    }
  }
  for (size_t i = 0; i < pdf_info->size(); i++) {
    std::sort( ((*pdf_info)[i]).begin(),  ((*pdf_info)[i]).end());
    KALDI_ASSERT(IsSortedAndUniq( ((*pdf_info)[i])));  // should have no dups.
  }
}


} // end namespace kaldi.
