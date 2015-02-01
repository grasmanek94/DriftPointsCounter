# This file demonstrates how to compile the DriftPointsCounter project on Linux.
#
# To compile DriftPointsCounter do:
#
# make DriftPointsCounter 
#

GPP = g++
GCC = gcc
DriftPointsCounter_OUTFILE = "./DriftPointsCounter.so"

COMPILE_FLAGS = -O3 -m32 -fPIC -c -I ./ -I ./amx -w -D LINUX -D SAMPGDK_STATIC -D SAMPGDK_AMALGAMATION

DriftPointsCounter = -D DriftPointsCounter $(COMPILE_FLAGS)

all: DriftPointsCounter

clean:
	-rm -f *~ *.o *.so

DriftPointsCounter: clean
	$(GPP) $(DriftPointsCounter) *.cpp
	$(GCC) $(DriftPointsCounter) *.c
	$(GPP) -m32 -fshort-wchar -shared -o $(DriftPointsCounter_OUTFILE) *.o

