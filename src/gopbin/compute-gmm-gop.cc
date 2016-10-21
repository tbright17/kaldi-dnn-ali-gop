// gopbin/compute-gop-gmm.cc

// Copyright 2006  Junbo Zhang                    

// This project based on Kaldi (https://github.com/kaldi-asr/kaldi).
// All the Kaldi's codes in this project are under the Apache License, Version 2.0.
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
// 
// However, This file has not been merged into Kaldi's master branch,
// and the codes in this file are NOT UNDER THE SAME LICENSE of Kaldi.
// The codes in this file are NOT ALLOWED TO USE, COPY, DISTRIBUTE, OR MODIFY
// unless being permitted by the author.

#include "base/kaldi-common.h"
#include "util/common-utils.h"
#include "gop/gmm-gop.h"


int main(int argc, char *argv[]) {
  try {
    using namespace kaldi;
    typedef kaldi::int32 int32;
    
    const char *usage =
        "Compute GOP given [GMM-based] models.\n"
        "Usage:   compute-gmm-gop [options] tree-in model-in\n"
        "e.g.: \n"
        " compute-gmm-gop tree 1.mdl\n";
    ParseOptions po(usage);

    if (po.NumArgs() != 2) {
      po.PrintUsage();
      exit(1);
    }

    GmmGop gop;
    gop.Foo();
    gop.Bar();

    KALDI_LOG << "Done.\n";
    return 0;
  } catch(const std::exception &e) {
    std::cerr << e.what();
    return -1;
  }
}


