
# kaldi-gop
This project computes GOP (Goodness of Pronunciation) bases on Kaldi.

## How to build
1. Download and complile [Kaldi](http://www.kaldi-asr.org). Note that you need to check out the branch 5.1 instead of master, and do not use the "--shared" option.
1. Edit src/CMakeLists.txt to set the variable $KALDI_ROOT.
1. Compile the binary:
```
cd src/
mkdir build && cd build
cmake .. && make
```
## Run the example
```
cd egs/gop-compute
./run.sh
