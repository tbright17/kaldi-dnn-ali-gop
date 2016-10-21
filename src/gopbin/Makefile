
all:

EXTRA_CXXFLAGS = -Wno-sign-compare
include ../kaldi.mk

BINFILES = compute-gmm-gop

OBJFILES =



TESTFILES =

ADDLIBS = ../decoder/kaldi-decoder.a \
          ../fstext/kaldi-fstext.a ../hmm/kaldi-hmm.a ../gmm/kaldi-gmm.a \
          ../tree/kaldi-tree.a ../util/kaldi-util.a \
          ../matrix/kaldi-matrix.a ../base/kaldi-base.a ../gop/kaldi-gop.a


include ../makefiles/default_rules.mk
