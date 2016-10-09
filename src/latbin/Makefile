
all:

EXTRA_CXXFLAGS = -Wno-sign-compare
include ../kaldi.mk

BINFILES = lattice-push lattice-1best lattice-scale \
           lattice-align-phones lattice-to-ctm-conf \
           lattice-align-words-lexicon lattice-to-nbest \
           lattice-align-words lattice-to-phone-lattice \
           lattice-confidence lattice-to-post \
           lattice-copy-backoff lattice-word-align \
           lattice-copy nbest-to-ctm \
           lattice-prune nbest-to-lattice.cc

OBJFILES =



TESTFILES =

ADDLIBS = ../lat/kaldi-lat.a ../lm/kaldi-lm.a ../fstext/kaldi-fstext.a \
          ../hmm/kaldi-hmm.a ../tree/kaldi-tree.a ../util/kaldi-util.a \
          ../thread/kaldi-thread.a ../matrix/kaldi-matrix.a \
          ../base/kaldi-base.a 

include ../makefiles/default_rules.mk

