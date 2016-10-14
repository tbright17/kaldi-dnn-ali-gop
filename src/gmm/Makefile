all:

OPENFST_CXXFLAGS =
OPENFST_LDLIBS =
include ../kaldi.mk

OBJFILES = diag-gmm.o diag-gmm-normal.o am-diag-gmm.o \
           model-common.o decodable-am-diag-gmm.o

LIBNAME = kaldi-gmm

ADDLIBS = ../tree/kaldi-tree.a ../util/kaldi-util.a ../thread/kaldi-thread.a \
          ../matrix/kaldi-matrix.a ../base/kaldi-base.a 



include ../makefiles/default_rules.mk
