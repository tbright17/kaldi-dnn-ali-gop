# kaldi-gop
This project computes GOP (Goodness of Pronunciation) bases on Kaldi.

## How to build
* Download the code of [Kaldi](http://www.kaldi-asr.org), and build it.
    Note that do not use "--shared" option when building kaldi.
* Compile the binary
```
cd src/
mkdir build && cd build
cmake .. && make
```
* Run the example
```
cd egs/gop-compute
./run.sh
