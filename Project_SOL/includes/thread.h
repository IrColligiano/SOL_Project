#ifndef __THREAD_H__
#define __THREAD_H__

#include <list.h>

static inline int TRYLOCK(pthread_mutex_t* l) {
  int r=0;		
  if ((r=pthread_mutex_trylock(l))!=0 && r!=EBUSY) {		    
    fprintf(stderr, "ERRORE FATALE unlock\n");		    
    pthread_exit((void*)EXIT_FAILURE);			    
  }								    
  return r;	
}

//void convert(msg * messaggio , char ** buf);

//int  thread_create();

//void * thread_work(void * arg);

//void * thread_delivery(void * arg);


#endif