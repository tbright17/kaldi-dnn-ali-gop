#!/bin/sh

# Copyright 2016-2017   Author: Junbo Zhang  <dr.jimbozhang@gmail.com>

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

# Download and prepare the example data
[ -f swbd_model_tri1.tar.gz ] || wget https://github.com/jimbozhang/kaldi-gop/files/888135/swbd_model_tri1.tar.gz
[ -d exp ] || tar zxf swbd_model_tri1.tar.gz
sed -i '3,30275s/[a-z]/\u&/g' data/lang/words.txt

# Decode
local/compute-gmm-gop.sh --nj "$decode_nj" --cmd "$decode_cmd" data/eval data/lang exp/tri1 exp/eval_gop
