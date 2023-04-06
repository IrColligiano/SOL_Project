#if !defined(LIST_H)
#define LIST_H

#include <utils.h>

/*
extern int condsemiterm;
extern int condterm;       		//variabile di terminazione dei thread
extern long Nthread;			// variabile numero di thread
extern long Qlen;        		//variabile per la lunghezza della coda
extern long Tdelay;     		//variabile per i ms
extern char *DirectoryName;	 	// stringa per il nome della directory
extern List Coda;           	// coda concorrente
extern List Listoffile;
extern list_ *Listforsend;
extern pthread_t * arrth;	    //array di thread
extern pthread_t * thdelivery; 	// thread
extern pthread_cond_t cond; 	// variabile di condizione
extern pthread_cond_t cond2;
extern pthread_mutex_t mutex;  	//variabile per il mutex
extern pthread_mutex_t mutex2;
extern pthread_mutex_t mutex3;
*/

//malloc nuovo nodo della lista
list *  malloc_nodo(list * nodo);

//inizializza i valori della lista
void init_list(List * l,long n,long msec);

//inserisce elementi in testa alla lista
//se maxlen==-1 inserisce in una lista contentente tutti i path dei file 
//altrimenti inserisce nella lista concorrente 
int head_insert (List * l, char n[]);

//stampa la lista
void print_list(List * l);

//elimina l ultimo elemnto della lista e restituisce:
//il path dell elemento eliminato,'\0'se non e stato elimanto 
char * delete_last(List * l, char * ret);

//elimina la lista;
void free_List(List *l);

#endif