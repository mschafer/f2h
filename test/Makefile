FC = gfortran
CC = clang

F_FLAGS = -O -g -fno-underscoring
C_FLAGS = -O -g

timestwo.o: timestwo.f
	$(FC) -c $(F_FLAGS) $<

test.o: test.c
	$(CC) -c $(C_FLAGS) $<

test: test.o timestwo.o
	$(CC) -o test test.o timestwo.o

clean: 
	rm timestwo.o test.o test
