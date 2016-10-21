all:

OPENFST_CXXFLAGS =
OPENFST_LDLIBS =
include ../kaldi.mk

OBJFILES = gmm-gop.o

LIBNAME = kaldi-gop

ADDLIBS = ../decoder/kaldi-decoder.a \
          ../fstext/kaldi-fstext.a ../hmm/kaldi-hmm.a ../gmm/kaldi-gmm.a \
          ../tree/kaldi-tree.a ../util/kaldi-util.a \
          ../matrix/kaldi-matrix.a ../base/kaldi-base.a


include ../makefiles/default_rules.mk
