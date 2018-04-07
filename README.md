
# kaldi-gop
Computes GOP (Goodness of Pronunciation) and do forced alignment bases on Kaldi with nnet3 support. The acoustic model is trained using librispeech database (960 hours data) with the scripts under kaldi/egs/librispeech.

## How to build
1. Download [Kaldi](https://github.com/kaldi-asr/kaldi). Don't compile.
2. Put the folders under src into kaldi/src (replace Makefile).
3. Compile the code as compiling kaldi (kaldi/src/INSTALL).
4. Change KALDI_ROOT in egs/gop-compute/path.sh to your own KALDI_ROOT

## Run the example
```
cd egs/gop-compute
./run.sh --dnn true/false audio_dir data_dir result_dir
```
See meaning of arguments in run.sh

### Notes on data preparation
To use this tool, audio files (.wav) and corresponding transcript (.lab) needs to be prepared and stored in following format:

```
.
├── ...
├── data_dir                   
│   ├── speaker1 # indicate speaker ID          
│   ├── speaker2         
│   └── speaker3
|       ├── utt1.wav # indicate utterance ID
|       ├── utt1.lab 
└── ...
```

Do not use space in speaker folder name or utterance file name, using underscore instead. Make sure different speakers have different folder names (speaker ID) and different audio files have different file name (utt ID).

## To-do
- [ ] Add GPU support
- [x] Convert alignment results to readable format (textgrid)
- [ ] Add comparison between GMM and DNN (nnet3)
- [x] Add feature extraction script
