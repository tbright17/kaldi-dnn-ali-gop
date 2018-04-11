#!/bin/sh

# Copyright 2017   Author: Ming Tu
# Arguments:
# audio-dir: where audio files are stored
# data-dir: where extracted features are stored
# result-dir: where results are stored                               

set -e
#set -x

dnn=false
nj=1 # number of parallel jobs. Set it depending on number of CPU cores
split_per_speaker=true # split by speaker (true) or sentence (false)

# Enviroment preparation
. ./cmd.sh
. ./path.sh
[ -h steps ] || ln -s $KALDI_ROOT/egs/wsj/s5/steps
[ -h utils ] || ln -s $KALDI_ROOT/egs/wsj/s5/utils
. parse_options.sh || exit 1;

if [ $# != 3 ]; then
   echo "usage: run.sh <audio-dir> <data-dir> <result-dir>"
   echo "main options (for others, see top of script file)"
   echo "  --dnn false                           # whether to use DNN model"
   echo "  --nj                                  # number of jobs"
   echo "  --split_per_speaker                   # split by speaker (true) or sentence (false)"
   exit 1;
fi

audio_dir=$1
data_dir=$2
result_dir=$3

# data preparation
local/data_preparation.sh --nj $nj --dnn $dnn $audio_dir $data_dir

# Calculation
if [[ "$dnn" = false ]]; then
  echo "Using GMM model!"
  local/compute-gmm-gop.sh --nj "$nj" --cmd "$decode_cmd" --split_per_speaker "$split_per_speaker" $data_dir data/lang exp/tri1 $result_dir   ### gmm model
else
  echo "Using DNN model!"
  local/compute-dnn-gop.sh --nj "$nj" --cmd "$decode_cmd" --split_per_speaker "$split_per_speaker" $data_dir $data_dir/ivectors_hires \
            data_dnn/lang exp_dnn/nnet3 $result_dir    ### dnn model
fi
