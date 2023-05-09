#ifndef __THREAD_H__
#define __THREAD_H__

#include <list.h>

pool *th_pool; // threadpool
pthread_t sig_handler_thread; // thread signal handler
volatile sig_atomic_t is_connected;// variabile che segnala qunado la connect e stata effettuata
volatile sig_atomic_t term_for_sig;// variabile che modifica il signal handler per terminare in caso di segnale

// funzione che esegue il thread signalhandler
void* signal_handler_thread_work(void* arg);
//setta una maschera con i seganli da gestire e avvia il thread signalhandler 
int signal_handler_master();
// libera le risorse usate dalla threadpool e dalla coda concorrente
int free_resource();
//esegue join e controlla errno
int JOIN(pthread_t th );
// esegue il calcolo del risultato letto 8 byte alla volta dal file passato
long result_from_file(char* pathname);
//acquisisce la lock e scrive -1 sulla socket per far stampare al collector
int write_on_socket_sigusr1();
// acquisice la lock e scrive -2 sulla socket per far terminare il collector
int write_on_socket_finish();
// scriver sulla socket nome file risultato e kunghezza della stringa nome file
int write_on_socket(char* path, long res, size_t len);
// inizializza la threadpool e lancia i thread worker
int  thread_create(long number_th);
// funzione eseguita dai thread worker
void * thread_work(void * arg);



#endif