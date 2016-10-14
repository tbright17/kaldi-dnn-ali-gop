all:


include ../kaldi.mk

OBJFILES = hmm-topology.o transition-model.o hmm-utils.o

LIBNAME = kaldi-hmm
ADDLIBS = ../tree/kaldi-tree.a ../util/kaldi-util.a ../thread/kaldi-thread.a \
          ../matrix/kaldi-matrix.a ../base/kaldi-base.a 

include ../makefiles/default_rules.mk

