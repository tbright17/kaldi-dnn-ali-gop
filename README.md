
# kaldi-dnn-ali-gop
Computes forced-alignment and GOP (Goodness of Pronunciation) bases on Kaldi with nnet3 support. Can optionally output the phoneme confusion matrix on frame or phoneme segment level. The acoustic model is trained using librispeech database (960 hours data) with the scripts under kaldi/egs/librispeech.

## Requirements
1. sox (http://sox.sourceforge.net/)
2. textgrid (https://github.com/kylebgorman/textgrid)
3. numpy (optional, only if you want the phoneme confusion measurements)

## How to build
1. Download [Kaldi](https://github.com/kaldi-asr/kaldi). Don't compile.
2. Put the folders under src into kaldi/src (replace Makefile).
3. Compile the code as compiling kaldi (kaldi/src/INSTALL).
4. Change KALDI_ROOT in egs/gop-compute/path.sh to your own KALDI_ROOT

## Run the example
```
cd egs/gop-compute
./run.sh --nj 1 --split_per_speaker true/false --dnn true/false audio_dir data_dir result_dir
```

--nj: number of jobs to do parallel computing, should be smaller than your number of CPU cores

--split_per_speaker: for parallelization, whether to split by speaker (true) or by utterance (false)

--dnn: use DNN acoustic model or GMM acoustic model

audio_dir: the folder where you put your audio files

data_dir: intermediate folder where acoustic features related files are stored

result_dir: where results are stored (aligned_textgrid: alignments in textgrid format; gop.txt: gop values for every phoneme of every utterance)

### Notes on data preparation
To use this tool, audio files (.wav) and corresponding transcript (.lab) needs to be prepared and stored in following format:

```
.
├── ...
├── audio_dir                   
│   ├── speaker1 # indicate speaker ID          
│   ├── speaker2         
│   └── speaker3
|       ├── utt1.wav # indicate utterance ID
|       ├── utt1.lab 
└── ...
```

Do not use space in speaker folder name or utterance file name, using underscore instead. Make sure different speakers have different folder names (speaker ID) and different audio files have different file name (utt ID). Please refer to Kaldi's documentation on [data preparation](http://kaldi-asr.org/doc/data_prep.html).

## To-do
- [ ] Add GPU support
- [x] Convert alignment results to readable format (textgrid)
- [ ] Add comparison between GMM and DNN (nnet3)
- [x] Add feature extraction script

### Citation

Please kindly cite the following paper if you find this repo useful in your research:

M. Tu, A. Grabek, J. Liss and V. Berisha, "Investigating the role of L1 in automatic pronunciation evaluation of L2 speech", to appear in proceedings of Interspeech 2018
