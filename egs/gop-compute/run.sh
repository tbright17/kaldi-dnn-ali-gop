#!/bin/sh

# Copyright 2017   Author: Ming Tu                               

set -e
#set -x

if [ "$1" = "--help" ]; then
  echo "Usage: "
  echo "  $0 <model-dir>"
  echo "e.g.:"
  echo "  $0 exp/tri1"
  exit 1;
fi

# Enviroment preparation
. ./cmd.sh
. ./path.sh
[ -h steps ] || ln -s $KALDI_ROOT/egs/wsj/s5/steps
[ -h utils ] || ln -s $KALDI_ROOT/egs/wsj/s5/utils

decode_nj=1

# data preparation
local/data_preparation.sh --dnn true accent_test_audio accent_test_data_dnn

# Decode
#local/compute-gmm-gop.sh --nj "$decode_nj" --cmd "$decode_cmd" accent_test_data data/lang exp/tri1 accent_exp/eval_gop   ### gmm model
local/compute-dnn-gop.sh --nj "$decode_nj" --cmd "$decode_cmd" accent_test_data_dnn accent_test_data_dnn/ivectors_hires \
           data_dnn/lang exp_dnn/nnet3 accent_exp_dnn/eval_gop    ### dnn model
