

all:

OPENFST_CXXFLAGS = 
OPENFST_LDLIBS =

include ../kaldi.mk

OBJFILES = kaldi-matrix.o kaldi-vector.o packed-matrix.o sp-matrix.o tp-matrix.o \
           matrix-functions.o qr.o compressed-matrix.o sparse-matrix.o

LIBNAME = kaldi-matrix

ADDLIBS = ../base/kaldi-base.a 

include ../makefiles/default_rules.mk

