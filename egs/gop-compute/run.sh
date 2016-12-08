#!/bin/sh

# Copyright 2016   Author: Junbo Zhang  <dr.jimbozhang@gmail.com>

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
decode_nj=1

# Decode
steps/align_si.sh --nj "$decode_nj" --cmd "$decode_cmd" data/test_20 data/lang exp/tri1 output/test_20_ali
local/compute-gmm-gop.sh --nj "$decode_nj" --cmd "$decode_cmd" data/test_20 data/lang exp/tri1 output/test_20_ali output/test_20_gop
