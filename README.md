
# kaldi-gop
Computes GOP (Goodness of Pronunciation) and do forced alignment bases on Kaldi with nnet3 support.

## How to build
1. Download [Kaldi](http://www.kaldi-asr.org).
1. Put the folders under src into kaldi/src.
1. Compile the code as compiling kaldi (kaldi/src/INSTALL):

## Run the example
```
cd egs/gop-compute
./run.sh
```
### Notes on data preparation
To use this tool, audio files and corresponding transcript needs to be prepared and stored in following format:
    ```bash
    .
    ├── ...
    ├── data_dir                   
    │   ├── speaker1 # indicate speaker id          
    │   ├── speaker2         
    │   └── speaker3
    |       ├── utt1.wav # indicate utterance id
    |       ├── utt1.lab 
    └── ...
    ```

Do not use space in speaker folder name or utterance file name, using underscore instead.

## To-do
- [ ] Add GPU support
- [x] Convert alignment results to readable format (textgrid)
- [ ] Add comparison between GMM and DNN (nnet3)
- [x] Add feature extraction script
