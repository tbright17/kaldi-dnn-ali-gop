# This is the top-level Makefile for Kaldi.
# Also see kaldi.mk which supplies options and some rules
# used by the Makefiles in the subdirectories.

SHELL := /bin/bash

SUBDIRS = base decoder feat fstext gmm gmmbin hmm lat latbin lm matrix sgmm thread transform tree util

CUDAMEMTESTDIR = cudamatrix

SUBDIRS_LIB = $(filter-out %bin, $(SUBDIRS))

KALDI_SONAME ?= libkaldi.so

# Optional subdirectories
EXT_SUBDIRS = 
EXT_SUBDIRS_LIB = $(filter-out %bin, $(EXT_SUBDIRS))

include kaldi.mk

# Reset the default goal, so that the all target will become default
.DEFAULT_GOAL :=
all: checkversion test_dependencies kaldi.mk mklibdir $(SUBDIRS)
	-echo Done

mklibdir:
	test -d $(KALDILIBDIR) || mkdir $(KALDILIBDIR)

#I don't want to call rm -rf
rmlibdir:
ifneq ($(KALDILIBDIR), )
	-rm $(KALDILIBDIR)/*{.so,.a,.o}
	-rmdir $(KALDILIBDIR)
else
	@true
endif

.PHONY: checkversion
checkversion:
ifeq ($(shell ./configure --version),$(CONFIGURE_VERSION))
	@echo "The version of configure script matches kaldi.mk version. Good."
else
	@echo "The kaldi.mk file was generated using a different version of configure script. Please rerun the configure again"
	@test -f ./kaldi.mk && echo  "Hint: Previous configure command line: " && head -n 2 ./kaldi.mk | grep configure | sed 's/^# *//g'
	@false
endif

biglib: $(SUBDIRS_LIB)
ifeq ($(KALDI_FLAVOR), dynamic)
ifeq ($(shell uname), Darwin)
	$(CXX) -dynamiclib -o $(KALDILIBDIR)/libkaldi.dylib -install_name @rpath/libkaldi.dylib -framework Accelerate $(LDFLAGS) $(SUBDIRS_LIB:=/*.dylib)
else
ifeq ($(shell uname), Linux)
	#$(warning the following command will probably fail, in that case add -fPIC to your CXXFLAGS and remake all)
	$(CXX) -shared -o $(KALDILIBDIR)/$(KALDI_SONAME) -Wl,-soname=$(KALDI_SONAME),--whole-archive  $(SUBDIRS_LIB:=/kaldi-*.a) $(LDLIBS) -Wl,--no-whole-archive
else
	$(error Dynamic libraries not supported on this platform. Run configure with --static flag. )
endif
endif
endif

biglibext: $(EXT_SUBDIRS_LIB)
ifeq ($(KALDI_FLAVOR), dynamic)
ifeq ($(shell uname), Darwin)
	$(CXX) -dynamiclib -o $(KALDILIBDIR)/libkaldi_ext.dylib -install_name @rpath/libkaldi_ext.dylib -framework Accelerate $(LDFLAGS) $(EXT_SUBDIRS_LIB:=/*.dylib)
else
ifeq ($(shell uname), Linux)
	#$(warning The following command will probably fail, in that case add -fPIC to your CXXFLAGS and remake all.)
	$(CXX) -shared -o $(KALDILIBDIR)/libkaldi_ext.so -Wl,-soname=libkaldi_ext.so,--whole-archive  $(EXT_SUBDIRS_LIB:=/kaldi-*.a) -Wl,--no-whole-archive
else
	$(error Dynamic libraries not supported on this platform. Run configure with --static flag. )
endif
endif
endif

kaldi.mk:
	@[ -f kaldi.mk ] || { echo "kaldi.mk does not exist; you have to run ./configure"; exit 1; }

# Compile optional stuff
ext: test_dependencies ext_depend $(SUBDIRS) $(EXT_SUBDIRS)
	-echo Done

ifndef OPENFST_VER
$(error Please rerun configure: OPENFST_VER is not defined, likely kaldi.mk was produced by older configure script.)
endif
# Note: OPENFST_VER is determined by configure and added to kaldi.mk
OPENFST_VER_NUM := $(shell echo $(OPENFST_VER) | sed 's/\./ /g' | xargs printf "%d%02d%02d")
test_dependencies:
ifeq ("$(shell expr $(OPENFST_VER_NUM) \< 10302)","1")
	$(error OpenFst $(OPENFST_VER) is not supported. You now need OpenFst >= 1.3.2.)
endif

check_portaudio:
	@[ -d ../tools/portaudio ] || ( cd ../tools;  ./install_portaudio.sh )


clean: rmlibdir
	-for x in $(SUBDIRS) $(EXT_SUBDIRS); do $(MAKE) -C $$x clean; done

test: $(addsuffix /test, $(SUBDIRS))

ext_test: $(addsuffix /test, $(EXT_SUBDIRS))

# Define an implicit rule, expands to e.g.:
#  base/test: base
#     $(MAKE) -C base test
%/test: % mklibdir
	$(MAKE) -C $< test

cudavalgrind:
	-for x in $(CUDAMEMTESTDIR); do $(MAKE) -C $$x valgrind || { echo "valgrind on $$x failed"; exit 1; }; done

valgrind:
	-for x in $(MEMTESTDIRS); do $(MAKE) -C $$x valgrind || { echo "valgrind on $$x failed"; exit 1; }; done


depend: $(addsuffix /depend, $(SUBDIRS))

%/depend:
	$(MAKE) -C $(dir $@) depend


ext_depend: check_portaudio
	-for x in $(EXT_SUBDIRS); do $(MAKE) -C $$x depend; done


.PHONY: $(SUBDIRS)
$(SUBDIRS) : mklibdir
	$(MAKE) -C $@

.PHONY: $(EXT_SUBDIRS)
$(EXT_SUBDIRS) : mklibdir
	$(MAKE) -C $@


### Dependency list ###
# this is necessary for correct parallel compilation
#1)The tools depend on all the libraries

gmmbin latbin: \
 base matrix util feat tree thread gmm transform fstext hmm lm decoder lat

#2)The libraries have inter-dependencies
base:
matrix: base
thread: base
util: base matrix thread
feat: base matrix util gmm transform tree thread
tree: base util thread matrix
gmm: base util matrix tree thread
transform: base util matrix gmm tree thread
sgmm: base util matrix gmm tree transform thread hmm
fstext: base util thread matrix tree
hmm: base tree matrix util thread
lm: base util thread matrix fstext
decoder: base util thread matrix gmm sgmm hmm tree transform lat
lat: base util thread hmm tree matrix
