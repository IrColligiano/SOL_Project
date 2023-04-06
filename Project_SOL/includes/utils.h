#if !defined (UTILS_H)
#define UTILS_H

#define UNLOCK(l)                                               \
    if (pthread_mutex_unlock(l)!=0){                            \
        fprintf(stderr, "ERRORE: Unlock mutex\n");		        \
        pthread_exit((void*)EXIT_FAILURE);				        \
    }

#define LOCK(l)                                                 \
    if (pthread_mutex_lock(l)!=0){                              \
        fprintf(stderr, "ERRORE: Lock mutex\n");		        \
        pthread_exit((void*)EXIT_FAILURE);				        \
    }

#define WAIT(c,l)                                               \
    if (pthread_cond_wait(c,l)!=0){                             \
        fprintf(stderr, "ERRORE: Wait\n");		                \
        pthread_exit((void*)EXIT_FAILURE);				        \
    }

#define TWAIT(c,l,t) {							                \
    int r=0;								                    \
    if ((r=pthread_cond_timedwait(c,l,t))!=0 && r!=ETIMEDOUT){	\
            fprintf(stderr, "ERRORE: Timed wait\n");			\
            pthread_exit((void*)EXIT_FAILURE);					\
    }									                        \
}

#define SIGNAL(c)                                               \
    if (pthread_cond_signal(c)!=0){	                            \
        fprintf(stderr, "ERRORE: Signal\n");			        \
        pthread_exit((void*)EXIT_FAILURE);					    \
    }

#define BCAST(c)                                                \
    if (pthread_cond_broadcast(c)!=0){		                    \
        fprintf(stderr, "ERRORE: Broadcast\n");			        \
        pthread_exit((void*)EXIT_FAILURE);						\
    }

#define DMUTEX(l)                                               \
    if (pthread_mutex_destroy(l)!=0){                           \
        fprintf(stderr, "ERRORE: Mutex destroy\n");             \
        return 0;                                               \
    }

#define IMUTEX(l,n)                                             \
    if (pthread_mutex_init(l,n)!=0){                            \
        fprintf(stderr, "ERRORE: Mutex init\n");                \
        return 0;                                               \
    }

#define DCOND(c)                                                \
    if (pthread_cond_destroy(c)!=0){                            \
        fprintf(stderr, "ERRORE: Cond destroy\n");              \
        return 0;                                               \
    }

#define ICOND(c,n)                                              \
    if (pthread_cond_init(c,n)!=0){                             \
        fprintf(stderr, "ERRORE: Cond init\n");                 \
        return 0;                                               \
    }

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stddef.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <data_struct.h>

//esegue una malloc e setta errno
void* _malloc_(size_t size);

#endif