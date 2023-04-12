CC = gcc

INCLUDE = -I ./includes

CFLAGS = -Wall -pedantic -std=c99 -pthread $(INCLUDE)

EXE = farm

DIR = testdir

#SCK = farm.sck

FILE = file1.dat file2.dat file3.dat file4.dat file5.dat file10.dat file12.dat file13.dat file14.dat file15.dat file16.dat file17.dat file18.dat file20.dat file100.dat file116.dat file117.dat
#OBJ = obj/master_utils.o  obj/main.o obj/list.o obj/utils.o obj/thread.o obj/masterworker.o obj/tree.o obj/collector.o
OBJ = obj/main.o obj/utils.o obj/list.o obj/thread.o

.PHONY : clean clean_sck exec
 
$(EXE) : $(OBJ)
	$(CC) $(CFLAGS) $^ -o $(EXE)
	 
obj/utils.o : src/utils.c includes/utils.h includes/data_struct.h
	$(CC) $(CFLAGS) -c $< -o obj/utils.o

obj/list.o : src/list.c includes/list.h includes/utils.h
	$(CC) $(CFLAGS) -c $< -o obj/list.o

obj/tree.o : src/tree.c includes/tree.h includes/utils.h
	$(CC) $(CFLAGS) -c $< -o obj/tree.o

#obj/master_utils.o : src/master_utils.c includes/master_utils.h includes/list.h
#	$(CC) $(CFLAGS) -c $< -o obj/master_utils.o
	
obj/thread.o : src/thread.c includes/thread.h includes/list.h
	$(CC) $(CFLAGS) -c $< -o obj/thread.o

#obj/masterworker.o : src/masterworker.c includes/masterworker.h includes/thread.h
#	$(CC) $(CFLAGS) -c $< -o obj/masterworker.o

#obj/collector.o : src/collector.c includes/collector.h includes/tree.h
#	$(CC) $(CFLAGS) -c $< -o obj/collector.o

obj/main.o : src/main.c includes/thread.h#includes/masterworker.h includes/collector.h
	$(CC) $(CFLAGS) -c $< -o obj/main.o

clean:
	-rm $(OBJ) $(EXE) *~ core

clean_sck:
	rm $(SCK)

exec: 
	./$(EXE) -d $(DIR) -q 8 $(FILE) -t 1000 -n 8
