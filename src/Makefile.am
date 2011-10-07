COMPILER= g++-4.5
LIBS= -l boost_program_options -l pthread -l z -ltinyxml
BOOST_INCLUDE= /usr/local/include
BOOST_LIBRARY= /usr/local/lib
STD= -std=c++0x
ARCH= -march=native
OPTIM_FLAGS= -O4 -ffast-math -fno-strict-aliasing -fomit-frame-pointer -funroll-loops -fmodulo-sched -foptimize-register-move -freciprocal-math -ffinite-math-only -fbranch-target-load-optimize
DEBUG_FLAGS= -g -ggdb -DNDEBUG -fno-inline #-p

# for normal fast execution.
#AM_CXXFLAGS= -W -Wall $(STD) $(ARCH) $(OPTIM_FLAGS) -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) #-ltcmalloc
# for debug purposes
AM_CXXFLAGS= -W -Wall $(STD) $(ARCH) $(OPTIM_FLAGS) $(DEBUG_FLAGS) -D_FILE_OFFSET_BITS=64 -I $(BOOST_INCLUDE) #-ltcmalloc

AM_LDFLAGS= $(LIBS) -DUSE_RESTRICT

bin_PROGRAMS= denetX active_interactor
denetX_SOURCES= lda.cc parser.cc sparse_dense.cc cache.cc gd.cc parse_regressor.cc unique_sort.cc loss_functions.cc cg.cc global_data.cc multisource.cc parse_args.cc utils.cc sender.cc  hash.cc message_relay.cc network.cc parse_example.cc parse_arfheader.cc simple_label.cc delay_ring.cc io.cc noop.cc parse_primitives.cc float_cmp.cc vw.cc main.cc  multiclass/infogain.cc multiclass/arraycopy.cc multiclass/htnb.cc multiclass/statistics.cc multiclass/mutils.cc multiclass/evaluation.cc multiclass/dvec.cc multiclass/parse_nbmodel.cc multiclass/nb.cc timsort.h multiclass/ht.h 

active_interactor_SOURCES= active_interactor.cc utils.cc

METASOURCES= AUTO
clean-local:
	-rm -f *~

.PHONY: move

move:
	-mv $(bin_PROGRAMS) ../build
	-mv *.o ../build
