
# include the machine dependent configuration
-include ../../Makefile.conf

# if machine dependent configuration has been found:
ifdef GCCDEP

# source for plumed wrapper
SRC_WRAPPER=PlumedStatic.cpp

# source for plumed dynamic wrapper
SRC_DYNAMIC_WRAPPER=Plumed.c

# all sources
ALL_SRC=$(SRC_WRAPPER) $(SRC_DYNAMIC_WRAPPER)

# corresponding objects:
OBJ_WRAPPER=PlumedStatic.o
OBJ_DYNAMIC_WRAPPER=Plumed.o

# dependency files for all c++ sources
CPP_SRC=$(wildcard *.cpp)
ALL_DEP=$(addprefix deps/,$(CPP_SRC:.cpp=.d))

# default target
.PHONY: all
all: $(OBJ_WRAPPER) $(OBJ_DYNAMIC_WRAPPER)

links:
	@echo Nothing to do in $(CURDIR)

# there is a single C file here, and this is its dependency:
Plumed.o: Plumed.c Plumed.h
PlumedStatic.o: PlumedStatic.cpp Plumed.c Plumed.h

# Individual dependencies

# if machine dependent configuration has not been found:
else

.PHONY: error
error:
	@echo No configuration available
	@echo First run ./configure.sh in the root directory
endif

# this target is available anyway

.PHONY: clean
clean:
	rm -fr deps 
	rm -fr $(CLEANLIST)

# generic makefile rules
include ../maketools/make.rules

