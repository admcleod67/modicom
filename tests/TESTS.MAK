# Modicom tests - Borland C++ 3.1 makefile
#
# Build and run from the repository root inside DOS (for example,
# DOSBox):
#
#       MAKE -f TESTS\TESTS.MAK rebuild
#       BUILD\TESTS\TSTRINGS
#
# This makefile is intentionally separate from the production
# MAKEFILE. It writes only to BUILD\TESTS and does not contribute
# any object files to BUILD\MODICOM.EXE. Adjust BORLANDC below if
# Borland C++ is not installed in C:\BORLANDC.

.AUTODEPEND

BORLANDC = C:\BORLANDC

CC    = BCC +MODICOM.CFG
TLINK = TLINK

INCLUDEPATH = $(BORLANDC)\INCLUDE;SRC;TESTS
LIBPATH     = $(BORLANDC)\LIB

CFLAGS = -I$(INCLUDEPATH) -L$(LIBPATH) -nBUILD\TESTS
LFLAGS = /v /x /c /P- /L$(LIBPATH)

all: BUILD\TESTS\TSTRINGS.EXE

BUILD\TESTS\TSTRINGS.EXE: BUILD\TESTS\TSTRINGS.OBJ BUILD\TESTS\STRINGS.OBJ
	$(TLINK) $(LFLAGS) @&&|
C0S.OBJ+
BUILD\TESTS\TSTRINGS.OBJ+
BUILD\TESTS\STRINGS.OBJ
BUILD\TESTS\TSTRINGS
		# no map file
CS.LIB
|

BUILD\TESTS\TSTRINGS.OBJ: TESTS\TSTRINGS.C TESTS\TEST.H SRC\STRINGS.H MODICOM.CFG
	@if not exist BUILD\NUL md BUILD
	@if not exist BUILD\TESTS\NUL md BUILD\TESTS
	$(CC) $(CFLAGS) -c TESTS\TSTRINGS.C

BUILD\TESTS\STRINGS.OBJ: SRC\STRINGS.C SRC\STRINGS.H MODICOM.CFG
	@if not exist BUILD\NUL md BUILD
	@if not exist BUILD\TESTS\NUL md BUILD\TESTS
	$(CC) $(CFLAGS) -c SRC\STRINGS.C

clean:
	-del BUILD\TESTS\TSTRINGS.EXE
	-del BUILD\TESTS\*.OBJ

rebuild: clean all
