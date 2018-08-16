#!/bin/sh

# Copyright 2016-2017  Author: Ming Tu

# Begin configuration section.

nj=1
cmd=run.pl
dnn=false
# Begin configuration.
# End configuration options.

echo "$0 $@"  # Print the command line for logging

[ -f path.sh ] && . ./path.sh # source the path.
. parse_options.sh || exit 1;

if [ $# != 2 ]; then
   echo "usage: local/data_preparation.sh <audio-dir> <data-dir>"
   echo "e.g.:  local/data_preparation.sh /home/xxx/datasets/timit data/eval"
   echo "main options (for others, see top of script file)"
   echo "  --nj <nj>                                             # number of parallel jobs"
   echo "  --cmd (utils/run.pl|utils/queue.pl <queue opts>)      # how to run jobs."
   echo "  --dnn <true|false>                                    # use dnn model"
   exit 1;
fi

data=$1
dir=$2
mkdir -p $dir

# directory preparation
for d in $data/*/; do
    #echo $d
    for f in $d*.wav; do
        filename="$(basename $f)"
        filepath="$(dirname $f)"
        spkname="$(basename $filepath)"
        echo "${filename%.*} sox $f -e signed-integer -b 16 -r 16000 -t wav - |" >> $dir/wav.scp # Prepare wav.scp
        #echo "${filename%.*} sox $f -r 16000 -t amr-wb -C 8 - | sox - -t wav - |" >> $dir/wav.scp # Prepare wav.scp
        echo "$spkname ${filename%.*}" >> $dir/spk2utt # Prepare spk2utt
        echo "${filename%.*} $spkname" >> $dir/utt2spk # Prepare utt2spk
        (
            printf "${filename%.*} "
            cat "$data/$spkname/${filename%.*}.lab" | tr [a-z] [A-Z]
            printf "\n"
        ) >> $dir/text # Prepare transcript
    done
done

utils/fix_data_dir.sh $dir

wav-to-duration --read-entire-file=true p,scp:$dir/wav.scp ark,t:$dir/utt2dur || exit 1; # Prepare utt2dur

# feature extraction
if [[ "$dnn" = false ]]; then
    echo "Extracting feature for GMM model!"
    steps/make_mfcc.sh --cmd "$cmd" --nj "$nj" $dir $dir/mfcc/log $dir/mfcc || exit 1;
    steps/compute_cmvn_stats.sh $dir $dir/mfcc/log $dir/mfcc || exit 1;
    utils/fix_data_dir.sh $dir
else
    echo "Extracting feature for DNN model!"
    steps/make_mfcc.sh --nj "$nj" --mfcc-config conf/mfcc_hires.conf \
      --cmd "$cmd" $dir $dir/mfcc_hires/log $dir/mfcc_hires || exit 1;
    steps/compute_cmvn_stats.sh $dir $dir/mfcc_hires/log $dir/mfcc_hires || exit 1;
    utils/fix_data_dir.sh $dir

    steps/online/nnet2/extract_ivectors_online.sh --cmd "$cmd" --nj "$nj" \
      $dir exp_dnn/extractor \
      $dir/ivectors_hires || exit 1;
fi

echo "Finish data preparation and feature extraction!"