#ifndef __LIST_H__
#define __LIST_H__

#include <utils.h>

list * Queue;

//inizializza i valori della lista
int init_list(long n,long msec);

//inserisce elementi in testa alla lista
//se maxlen==-1 inserisce in una lista contentente tutti i path dei file 
//altrimenti inserisce nella lista concorrente 
int head_insertion(char n[], size_t len);

//stampa la lista
void print_list();

//elimina l ultimo elemento della lista e modifica la stringa ret
void delete_last(char ** ret);

//elimina la lista;
void free_list();

#endif