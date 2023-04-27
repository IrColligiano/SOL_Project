#ifndef __THREAD_H__
#define __THREAD_H__

#include <list.h>

t_pool *th_pool;

void* signal_handler_thread_work(void* arg);

int signal_handler_master();

void term_signal_handler();

int free_resource();

int JOIN(pthread_t th );

long result_from_path(char* pathname);

int write_on_socket_print_request();

int write_on_socket_term_request();

int write_on_socket(char* path, long res, size_t len);

int  thread_create(long number_th);

void * thread_work(void * arg);



#endif