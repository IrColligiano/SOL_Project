#ifndef __THREAD_H__
#define __THREAD_H__

#include <list.h>

int JOIN(pthread_t th );

//void convert(msg * messaggio , char ** buf);

int  thread_create(long number_th);

void * thread_work(void * arg);

//void * thread_delivery(void * arg);


#endif