// tree/context-dep.h

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

#ifndef KALDI_TREE_CONTEXT_DEP_H_
#define KALDI_TREE_CONTEXT_DEP_H_

#include "itf/context-dep-itf.h"
#include "tree/event-map.h"
#include "matrix/matrix-lib.h"
#include "tree/cluster-utils.h"

/*
  This header provides the declarations for the class ContextDependency, which inherits
  from the interface class "ContextDependencyInterface" in itf/context-dep-itf.h.
  This is basically a wrapper around an EventMap.  The EventMap
  (tree/event-map.h) declares most of the internals of the class, and the building routines are
  in build-tree.h which uses build-tree-utils.h, which uses cluster-utils.h . */


namespace kaldi {

static const EventKeyType kPdfClass = -1;  // The "name" to which we assign the
// pdf-class (generally corresponds ot position in the HMM, zero-based);
// must not be used for any other event.  I.e. the value corresponding to
// this key is the pdf-class (see hmm-topology.h for explanation of what this is).


/* ContextDependency is quite a generic decision tree.

   It does not actually do very much-- all the magic is in the EventMap object.
   All this class does is to encode the phone context as a sequence of events, and
   pass this to the EventMap object to turn into what it will interpret as a
   vector of pdfs.

   Different versions of the ContextDependency class that are written in the future may
   have slightly different interfaces and pass more stuff in as events, to the
   EventMap object.

   In order to separate the process of training decision trees from the process
   of actually using them, we do not put any training code into the ContextDependency class.
 */
class ContextDependency: public ContextDependencyInterface {
 public:
  virtual int32 ContextWidth() const { return N_; }
  virtual int32 CentralPosition() const { return P_; }


  /// returns success or failure; outputs pdf to pdf_id For positions that were
  /// outside the sequence (due to end effects), put zero.  Naturally
  /// phoneseq[CentralPosition()] must be nonzero.
  virtual bool Compute(const std::vector<int32> &phoneseq,
                       int32 pdf_class, int32 *pdf_id) const;

  virtual int32 NumPdfs() const {
    // this routine could be simplified to return to_pdf_->MaxResult()+1.  we're a
    // bit more paranoid than that.
    if (!to_pdf_) return 0;
    EventAnswerType max_result = to_pdf_->MaxResult();
    if (max_result < 0 ) return 0;
    else return (int32) max_result+1;
  }
  virtual ContextDependencyInterface *Copy() const {
    return new ContextDependency(N_, P_, to_pdf_->Copy());
  }

  /// Read context-dependency object from disk; throws on error
  void Read (std::istream &is, bool binary);

  // Constructor with no arguments; will normally be called
  // prior to Read()
  ContextDependency(): N_(0), P_(0), to_pdf_(NULL) { }

  // Constructor takes ownership of pointers.
  ContextDependency(int32 N, int32 P,
                    EventMap *to_pdf):
      N_(N), P_(P), to_pdf_(to_pdf) { }
  void Write (std::ostream &os, bool binary) const;

  ~ContextDependency() { delete to_pdf_; }

  const EventMap &ToPdfMap() const { return *to_pdf_; }

  /// GetPdfInfo returns a vector indexed by pdf-id, saying for each pdf which
  /// pairs of (phone, pdf-class) it can correspond to.  (Usually just one).
  /// c.f. hmm/hmm-topology.h for meaning of pdf-class.
  virtual void GetPdfInfo(const std::vector<int32> &phones,  // list of phones
                  const std::vector<int32> &num_pdf_classes,  // indexed by phone,
                  std::vector<std::vector<std::pair<int32, int32> > > *pdf_info)
      const;

 private:
  int32 N_;  //
  int32 P_;
  EventMap *to_pdf_;  // owned here.

  KALDI_DISALLOW_COPY_AND_ASSIGN(ContextDependency);
};


// Important note:
// Statistics for training decision trees will be of type:
// std::vector<std::pair<EventType, Clusterable*> >
// We don't make this a typedef as it doesn't add clarity.
// they will be sorted and unique on the EventType member, which
// itself is sorted and unique on the name (see event-map.h).

// See build-tree.h for functions relating to actually building the decision trees.




}  // namespace Kaldi


#endif
