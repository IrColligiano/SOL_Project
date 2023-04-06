CC = gcc

CFLAGS = -Wall -pedantic -std=c99 -pthread -I ./includes/

INCLUDE = -I ./includes

EXE = farm

#SCK = farm.sck

#OBJ = obj/master_utils.o  obj/main.o obj/list.o obj/utils.o obj/thread.o obj/masterworker.o obj/tree.o obj/collector.o
OBJ = obj/main.o obj/utils.o obj/list.o obj/thread.o
.PHONY : clean clean_sck
 
$(EXE) : $(OBJ)
	$(CC) $(CFLAGS) $^ -o $(EXE)
	 
obj/utils.o : src/utils.c includes/utils.h includes/data_struct.h
	$(CC) $(CFLAGS) -c $< -o obj/utils.o

obj/list.o : src/list.c includes/list.h includes/utils.h
	$(CC) $(CFLAGS) -c $< -o obj/list.o

#obj/tree.o : src/tree.c includes/tree.h includes/utils.h
#	$(CC) $(CFLAGS) -c $< -o obj/tree.o

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
