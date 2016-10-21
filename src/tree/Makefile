
all:

include ../kaldi.mk

# note, build-tree-utils-test also tests build-tree-questions.cc


OBJFILES = event-map.o context-dep.o clusterable-classes.o cluster-utils.o

LIBNAME = kaldi-tree
ADDLIBS = ../util/kaldi-util.a ../matrix/kaldi-matrix.a ../base/kaldi-base.a 


include ../makefiles/default_rules.mk
