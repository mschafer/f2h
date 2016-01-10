FC = gfortran
CC = clang

FFLAGS = -O -g

LLVM_DWARFDUMP ?= /Users/mschafer/install/bin/llvm-dwarfdump

FORTRAN_SRC = \
  arrays.f \
  functions.f \
  strings.f


FORTRAN_OBJ = $(FORTRAN_SRC:.f=.o)
FORTRAN_SO = libtest.so
DUMP_FILE = $(FORTRAN_SO:.so=.dump)

default: $(FORTRAN_OBJ) $(FORTRAN_SO) $(DUMP_FILE) hand_test

# if we build the library from objects, then no debug symbols are present
# so build it from sources instead
$(FORTRAN_SO) : $(FORTRAN_SRC)
	$(FC) $(FFLAGS) -shared $(FORTRAN_SRC) -o $(FORTRAN_SO)

$(DUMP_FILE) : $(FORTRAN_SO)
	-$(LLVM_DWARFDUMP) $(FORTRAN_SO).dSYM/Contents/Resources/DWARF/$(FORTRAN_SO) > $@

hand_test : $(FORTRAN_OBJ) main.c by_hand.h
	$(CC) -g -o $@ main.c $(FORTRAN_OBJ) -L /usr/local/gfortran/lib -lgfortran

clean: 
	rm -f *.o $(FORTRAN_SO)

%.o : %.f90
	$(FC) $(FFLAGS) $< -c -o $@
