CC = gcc

INCLUDE = -I ./includes

CFLAGS = -Wall -pedantic -std=c99 -pthread $(INCLUDE)

EXE = farm

SCK = farm.sck

Q = -q 8

N = -n 4

T = -t 10

D = -d testdir

FILE = file1.dat file2.dat file3.dat file4.dat file5.dat file10.dat file12.dat file13.dat file14.dat file15.dat file16.dat file17.dat file18.dat file20.dat file100.dat file116.dat file117.dat

OBJ = obj/main.o obj/utils.o obj/list.o obj/thread.o obj/tree.o obj/collector.o

.PHONY : clean clean_sck exec valg
 
$(EXE) : $(OBJ)
	$(CC) $(CFLAGS) $^ -o $(EXE)
	 
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

clean:
	-rm $(OBJ) ./$(EXE)

clean_sck:
	rm $(SCK)

exec: 
	./$(EXE) $(D) $(FILE) $(T) $(N) $(Q)

valg:
	valgrind --error-exitcode=1 --leak-check=full ./$(EXE) $(D) $(FILE) $(T) $(N) $(Q)