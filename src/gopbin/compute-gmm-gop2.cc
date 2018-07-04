// gopbin/compute-gop-gmm2.cc

// Copyright 2018  Ming Tu

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

#include "base/kaldi-common.h"
#include "util/common-utils.h"
#include "gop/gmm-gop2.h"

int main(int argc, char *argv[]) {
  try {
    using namespace kaldi;
    
    const char *usage =
        "Compute GOP normalized by first language info with GMM-based models.\n"
        "Usage:   compute-gmm-gop2 [options] tree-in model-in lexicon-fst-in l1-model-in l1-graph-in feature-rspecifier \n"
                  "transcriptions-rspecifier gop-wspecifier l1-gop-wspecifier phnseq-wspecifier\n";

    ParseOptions po(usage);

    po.Read(argc, argv);
    if (po.NumArgs() != 10) {
      po.PrintUsage();
      exit(1);
    }

    std::string tree_in_filename = po.GetArg(1);
    std::string model_in_filename = po.GetArg(2);
    std::string lex_in_filename = po.GetArg(3);
    std::string l1_model_in_filename = po.GetArg(4);
    std::string l1_graph_in_filename = po.GetArg(5);
    std::string feature_rspecifier = po.GetArg(6);
    std::string transcript_rspecifier = po.GetArg(7);
    std::string gop_wspecifier = po.GetArg(8);
    std::string l1_gop_wspecifier = po.GetArg(9);
    std::string phnseq_wspecifier = po.GetArg(10);

    SequentialBaseFloatMatrixReader feature_reader(feature_rspecifier);
    RandomAccessInt32VectorReader transcript_reader(transcript_rspecifier);
    BaseFloatVectorWriter gop_writer(gop_wspecifier);
    BaseFloatVectorWriter l1_gop_writer(l1_gop_wspecifier);
    Int32VectorWriter phnseq_writer(phnseq_wspecifier);

    GmmGop2 gop;
    gop.Init(tree_in_filename, model_in_filename, lex_in_filename, l1_model_in_filename);

    // Compute for each utterance
    for (; !feature_reader.Done(); feature_reader.Next()) {
      std::string utt = feature_reader.Key();
      if (! transcript_reader.HasKey(utt)) {
        KALDI_WARN << "Can not find alignment for utterance " << utt;        
        continue;
      }
      const Matrix<BaseFloat> &features = feature_reader.Value();
      const std::vector<int32> &transcript = transcript_reader.Value(utt);

      gop.Compute(features, transcript, l1_graph_in_filename);
      gop_writer.Write(utt, gop.Result());
      l1_gop_writer.Write(utt, gop.l1_Result());
      phnseq_writer.Write(utt, gop.phn_seq());
    }
    KALDI_LOG << "Done.";
    return 0;
  } catch(const std::exception &e) {
    std::cerr << e.what();
    return -1;
  }
}
