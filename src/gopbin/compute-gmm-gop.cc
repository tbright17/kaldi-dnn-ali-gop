// gopbin/compute-gop-gmm.cc

// Copyright 2006  Junbo Zhang                    

// This project based on Kaldi (https://github.com/kaldi-asr/kaldi).
// All the Kaldi's codes in this project are under the Apache License, Version 2.0.
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
// 
// However, this file has not been merged into Kaldi's master branch,
// and the codes in this file are NOT UNDER THE SAME LICENSE of Kaldi's.
// The codes in this file are NOT ALLOWED TO USE, COPY, DISTRIBUTE, OR MODIFY
// unless being permitted by the author.

#include "base/kaldi-common.h"
#include "util/common-utils.h"
#include "gop/gmm-gop.h"

int main(int argc, char *argv[]) {
  try {
    using namespace kaldi;
    typedef kaldi::int32 int32;
    using fst::SymbolTable;
    using fst::VectorFst;
    using fst::StdArc;
    
    const char *usage =
        "Compute GOP given [GMM-based] models.\n"
        "Usage:   compute-gmm-gop [options] model-in feature-rspecifier alignments-rspecifier gop-wspecifier\n"
        "e.g.: \n"
        " compute-gmm-gop 1.mdl scp:test.scp ark:1.ali ark,t:1.gop\n";

    ParseOptions po(usage);

    po.Read(argc, argv);
    if (po.NumArgs() != 4) {
      po.PrintUsage();
      exit(1);
    }

    std::string model_in_filename = po.GetArg(1);
    std::string feature_rspecifier = po.GetArg(2);
    std::string alignment_rspecifier = po.GetArg(3);
    std::string gop_wspecifier = po.GetArg(4);

    // Load resources
    TransitionModel trans_model;
    AmDiagGmm am_gmm;
    {
      bool binary;
      Input ki(model_in_filename, &binary);
      trans_model.Read(ki.Stream(), binary);
      am_gmm.Read(ki.Stream(), binary);
    }
    SequentialBaseFloatMatrixReader feature_reader(feature_rspecifier);
    RandomAccessInt32VectorReader alignment_reader(alignment_rspecifier);
    BaseFloatVectorWriter gop_writer(gop_wspecifier);

    // Compute for each utterance
    int32 num_done = 0, num_err = 0, num_retry = 0;
    for (; !feature_reader.Done(); feature_reader.Next()) {
      std::string utt = feature_reader.Key();
      const Matrix<BaseFloat> &features = feature_reader.Value();
      if (! alignment_reader.HasKey(utt)) {
        KALDI_WARN << "Can not find alignment for utterance " << utt;        
        num_err++;
        continue;
      }
      const std::vector<int32> &alignment = alignment_reader.Value(utt);

      GmmGop gop(am_gmm, trans_model);
      gop.Compute(features, alignment);
      gop_writer.Write(utt, gop.Result());
    }
  

    KALDI_LOG << "Retried " << num_retry << " out of "
              << (num_done + num_err) << " utterances.";
    KALDI_LOG << "Done " << num_done << ", errors on " << num_err;
    return 0;
  } catch(const std::exception &e) {
    std::cerr << e.what();
    return -1;
  }
}
