# kaldi-gop
This project computes GOP (Goodness of Pronunciation) bases on Kaldi.

This project has not been ready by far.

## How to build
1. Download the code of [Kaldi](http://www.kaldi-asr.org), and build it.
    Note that do not use "--shared" option when building kaldi.
1. go to src/
1. Edit CMakeLists.txt, set the KALDI_ROOT as your kaldi's path.
1. mkdir build && cd build
1. cmake .. && make
