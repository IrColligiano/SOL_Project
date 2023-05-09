CC = gcc

INCLUDE = -I ./includes

CFLAGS = -Wall -pedantic -std=c99 -pthread $(INCLUDE)

EXE = farm

SCK = farm.sck

Q = -q 8

N = -n 4

T = -t 1000

D = -d testdir

DIR = testdir

#SHELL = /bin/bash

FILE = file1.dat file2.dat file3.dat file4.dat file5.dat file10.dat file12.dat file13.dat file14.dat file15.dat file16.dat file17.dat file18.dat file20.dat file100.dat file116.dat file117.dat

OBJ = obj/main.o obj/utils.o obj/list.o obj/thread.o obj/tree.o obj/collector.o

.PHONY : clean exec valg cleanall test generafile mytest
 
$(EXE) : $(OBJ)
	$(CC) $(CFLAGS) $^ -o $(EXE)

generafile: generafile.o
	$(CC) generafile.o -o generafile
	
generafile.o: generafile.c
	$(CC) generafile.c -c generafile.c
	 
obj/utils.o : src/utils.c includes/utils.h includes/data_struct.h
	$(CC) $(CFLAGS) -c $< -o obj/utils.o

obj/list.o : src/list.c includes/list.h includes/utils.h
	$(CC) $(CFLAGS) -c $< -o obj/list.o

obj/tree.o : src/tree.c includes/tree.h includes/utils.h
	$(CC) $(CFLAGS) -c $< -o obj/tree.o

obj/thread.o : src/thread.c includes/thread.h includes/list.h
	$(CC) $(CFLAGS) -c $< -o obj/thread.o

obj/collector.o : src/collector.c includes/collector.h includes/tree.h
	$(CC) $(CFLAGS) -c $< -o obj/collector.o

obj/main.o : src/main.c includes/thread.h includes/collector.h
	$(CC) $(CFLAGS) -c $< -o obj/main.o

clean :
	-rm -f $(EXE) generafile $(OBJ) generafile.o $(EXE) *~ ./farm.sck expected.txt core

cleanall :
	-rm -f $(EXE) generafile $(OBJ) generafile.o $(EXE) *~ *.dat testdir ./farm.sck expected.txt core \
	rm -r $(DIR)

test: farm
	@chmod +x ./test.sh
	./test.sh

mytest: farm
	@chmod +x ./mytest.sh
	./mytest.sh

exec: 
	./$(EXE) $(D) $(T) $(N) $(Q) $(FILE)

valg:
	valgrind --error-exitcode=1 --leak-check=full ./$(EXE) $(D) $(T) $(N) $(Q) $(FILE)