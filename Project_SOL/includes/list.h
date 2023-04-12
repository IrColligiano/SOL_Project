#if !defined(LIST_H)
#define LIST_H

#include <utils.h>

List * Queue;

//malloc nuovo nodo della lista
list *  malloc_nodo(list * nodo);

//inizializza i valori della lista
int init_list(List ** l,long n,long msec);

//inserisce elementi in testa alla lista
//se maxlen==-1 inserisce in una lista contentente tutti i path dei file 
//altrimenti inserisce nella lista concorrente 
int head_insert (List ** l, char n[]);

//stampa la lista
void print_list(List * l);

//elimina l ultimo elemnto della lista e restituisce:
//il path dell elemento eliminato,'\0'se non e stato elimanto 
void delete_last(List ** l,char ** ret);

//elimina la lista;
void free_List(List **l);

#endif