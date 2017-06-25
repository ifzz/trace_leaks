# Define programs and commands.
SH     = sh
# Archive-maintaining program;
AR     = ar
# Program for doing assembly;
AS     = as
# Program for compiling C programs;
CC     = ${CROSS_COMPILE}gcc
# Program for compiling C++ programs;
CXX    = ${CROSS_COMPILE}g++
# Program for running the C preprocessor, with results to standard output;
CPP    = $(CC) -E
# Command to remove a file;
RM     = rm -f
# Command make a file;
MAKE   = make
# Install program
INSTALL= install

# Target executable file name
TARGET = trace_demo

# Target library file name
OUTLIB  = memtrace

# Project
PRJDIR  = .
PRJINC  = $(PRJDIR)/include
PRJSRC  = $(PRJDIR)/src
PRJOBJ  = $(PRJDIR)/obj
PRJLIB  = $(PRJDIR)/lib
PRJBIN  = $(PRJDIR)/bin

# Additional project with separate makefile
PRJSUB =

EXTINC = $(PRJINC)

# Additional libraries "-lcommon"
EXTLIB =

# Place -I options here
INCLUDES = -I. $(addprefix -I,$(EXTINC))

# List C and CPP source files here. (C dependencies are automatically generated.)
SOURCES = \
	debugnew.cpp debuginfo.cpp debugmalloc.cpp

# Add source directory prefix
ALLSOURCES = $(addprefix $(PRJSRC)/, $(SOURCES))

# Prepare "C" files list
CFILES   = $(filter %.c,   $(ALLSOURCES))
# Prepare "C++" files list
CPPFILES = $(filter %.cpp, $(ALLSOURCES))

# Define "C" object files.
COBJS	= $(patsubst $(PRJSRC)/%,$(PRJOBJ)/%,$(CFILES:%.c=%.o))
# Define "C++" object files.
CPPOBJS	= $(patsubst $(PRJSRC)/%,$(PRJOBJ)/%,$(CPPFILES:%.cpp=%.o))
# Define all object files.
OBJS	= $(COBJS) $(CPPOBJS)
# Define all dependencies
DEPS    = $(OBJS:%.o=%.d)

# Define output executable target
OUTBIN = $(PRJBIN)/$(TARGET)

# Define dynamic library file name
OUTDLIB = $(PRJLIB)/lib$(OUTLIB).so

# Define static library file name
OUTSLIB = $(PRJLIB)/lib$(OUTLIB).a

# Place -D or -U options here
DEF = -DNDEBUG

# Define CPU flags "-march=cpu-type"
CPU =

# Define specific MPU flags "-mno-fp-ret-in-387"
MPU =

# Define "C" standart
STD =

# Global flags
OPT  = $(MPU) $(MPU) $(STD) -fPIC -g -ggdb -O0

# Generate dependencies
DEP  = -MMD

WRN  = -Wall -Wclobbered -Wempty-body -Wignored-qualifiers -Wmissing-field-initializers -Wsign-compare -Wtype-limits -Wuninitialized -Wno-deprecated-declarations -fpermissive

# Define only valid "C" optimization flags
COPT = $(OPT) -fomit-frame-pointer -fno-stack-check
# Define only valid "C" warnings flags
CWRN = $(WRN) \
	-Wmissing-parameter-type -Wold-style-declaration -Wimplicit-int -Wimplicit-function-declaration -Wimplicit -Wignored-qualifiers \
	-Wformat-nonliteral -Wcast-align -Wpointer-arith -Wbad-function-cast -Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations \
	-Wnested-externs -Wshadow -Wwrite-strings -Wfloat-equal -Woverride-init

# Define only valid "C++" optimization flags
#CXXOPT  = ${OPT} -std=c++0x -frtti
CXXOPT  = ${OPT} -std=gnu++11
# Define only valid "C" warnings flags
CXXWRN  = $(WRN) \
	-Woverloaded-virtual \
	-Wignored-qualifiers \
	-Wformat-nonliteral -Wcast-align -Wpointer-arith -Wmissing-declarations \
	-Wcast-qual -Wwrite-strings -Wfloat-equal

# Extra flags to give to the C compiler.
CFLAGS = $(MCU) $(DEF) $(DEP) $(COPT) $(CWRN)

# Extra flags to give to the C++ compiler.
CXXFLAGS = $(MCU) $(DEF) $(DEP) $(CXXOPT) $(CXXWRN)

# Extra flags to give to the C compiler.
override CFLAGS += $(INCLUDES)

# Extra flags to give to the C++ compiler.
override CXXFLAGS += $(INCLUDES)

# Define global linker flags
#EXFLAGS = $(EXTLIB) -L $(PRJLIB)
EXFLAGS = $(EXTLIB) -ldl

# Linker flags to give to the linker.
override LDFLAGS = $(EXFLAGS) -Wl,--no-as-needed -Wl,-rpath,/lib:/usr/lib

# Special linker flags to build shared library
override SHFLAGS = $(EXFLAGS) -shared -s

# Executable not defined
ifeq ($(TARGET),)
 # Output library defined
 ifneq ($(OUTLIB),)
  all: $(OUTDLIB) $(OUTSLIB)
 else
  all:
	@echo "WARNING: Neither output, neither library defined!"
 endif
# Executable defined
else
 # Executable defined, but output library not defined
 ifeq ($(OUTLIB),)
  all: $(OUTBIN)
 # Executable and output library defined
 else
  all: $(OUTDLIB) $(OUTSLIB) $(OUTBIN)
 endif
endif
  # Sub-project defined
  ifneq ($(PRJSUB),)
	$(MAKE) -C $(PRJSUB) all
  endif

# Executable defined
ifneq ($(TARGET),)
 # Output library not defined
 ifeq ($(OUTLIB),)
  $(OUTBIN): $(OBJS)
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) src/$(TARGET).cpp -o $(OUTBIN) $(LDFLAGS) $(OBJS)
 # Executable and output library defined
 else
  $(OUTBIN): $(OUTSLIB) $(OUTDLIB)
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) src/$(TARGET).cpp -o $(OUTBIN) $(LDFLAGS) $(OUTDLIB)
 endif
endif

ifneq ($(OUTLIB),)
 $(OUTDLIB): $(OBJS)
	mkdir -p $(@D)
	$(CXX) -o $@ $^ $(SHFLAGS)

 $(OUTSLIB): $(OBJS)
	mkdir -p $(@D)
	$(AR) rcs -o $@ $^
endif

$(COBJS): $(PRJOBJ)/%.o: $(PRJSRC)/%.c
	mkdir -p $(@D)
	$(CC) -c -o $@ $< $(DEP) $(CFLAGS)

$(CPPOBJS): $(PRJOBJ)/%.o: $(PRJSRC)/%.cpp
	mkdir -p $(@D)
	$(CXX) -c -o $@ $< $(DEP) $(CXXFLAGS)

$(PRJBIN):
	mkdir -p $@

$(PRJOBJ):
	mkdir -p $@

$(PRJLIB):
	mkdir -p $@

install: ${OUTDLIB} ${OUTSLIB} ${OUTBIN}
ifneq ($(BINDIR),)
	$(INSTALL) -m 755 ${OUTBIN} $(BINDIR)
endif

ifneq ($(LIBDIR),)
	$(INSTALL) -m 755 ${OUTDLIB} $(LIBDIR)
	$(INSTALL) -m 755 ${OUTSLIB} $(LIBDIR)
endif

# Target: clean project.
clean:
ifneq ($(PRJSUB),)
	$(MAKE) -C $(PRJSUB) clean
endif
	@echo Cleaning objects
	$(RM) -rf $(PRJOBJ)
	$(RM) $(PRJBIN)/$(TARGET).d

# Target: clean all.
distclean:
ifneq ($(PRJSUB),)
	$(MAKE) -C $(PRJSUB) distclean
endif
	@echo Cleaning all objects and executable
	$(RM) $(OBJS)
	$(RM) $(DEPS)

	$(RM) -rf $(PRJLIB)
	$(RM) -rf $(PRJBIN)
	$(RM) -rf $(PRJOBJ)

# Listing of phony targets.
.PHONY : all clean $(OUTDLIB) $(OUTSLIB) $(OUTBIN)

-include subsys_config.mk
-include $(DEPS)
