// gopbin/compute-gop-gmm.cc

// Copyright 2016-2017  Junbo Zhang

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
    
    const char *usage =
        "Compute GOP with GMM-based models.\n"
        "Usage:   compute-gmm-gop [options] tree-in model-in lexicon-fst-in feature-rspecifier transcriptions-rspecifier gop-wspecifier\n"
        "e.g.: \n"
        " compute-gmm-gop tree 1.mdl lex.fst scp:train.scp ark:train.tra ark,t:1.gop\n";

    ParseOptions po(usage);

    po.Read(argc, argv);
    if (po.NumArgs() != 6) {
      po.PrintUsage();
      exit(1);
    }

    std::string tree_in_filename = po.GetArg(1);
    std::string model_in_filename = po.GetArg(2);
    std::string lex_in_filename = po.GetArg(3);
    std::string feature_rspecifier = po.GetArg(4);
    std::string transcript_rspecifier = po.GetArg(5);
    std::string gop_wspecifier = po.GetArg(6);

    SequentialBaseFloatMatrixReader feature_reader(feature_rspecifier);
    RandomAccessInt32VectorReader transcript_reader(transcript_rspecifier);
    BaseFloatVectorWriter gop_writer(gop_wspecifier);

    GmmGop gop;
    gop.Init(tree_in_filename, model_in_filename, lex_in_filename);

    // Compute for each utterance
    for (; !feature_reader.Done(); feature_reader.Next()) {
      std::string utt = feature_reader.Key();
      if (! transcript_reader.HasKey(utt)) {
        KALDI_WARN << "Can not find alignment for utterance " << utt;        
        continue;
      }
      const Matrix<BaseFloat> &features = feature_reader.Value();
      const std::vector<int32> &transcript = transcript_reader.Value(utt);

      gop.Compute(features, transcript);
      gop_writer.Write(utt, gop.Result());
    }
    KALDI_LOG << "Done.";
    return 0;
  } catch(const std::exception &e) {
    std::cerr << e.what();
    return -1;
  }
}
