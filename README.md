
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
## To-do
- [ ] Add GPU support
- [x] Convert alignment results to readable format (textgrid)
- [ ] Add comparison between GMM and DNN (nnet3)
- [ ] Add feature extraction script
