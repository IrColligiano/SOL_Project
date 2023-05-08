#ifndef __UTILS_H__
#define __UTILS_H__

#define UNLOCK(l)                                               \
    if (pthread_mutex_unlock(l) !=0){                           \
        perror("ERRORE: mutex_unlock mutex error");		        \
        pthread_exit((void*)EXIT_FAILURE);				        \
    }

#define LOCK(l)                                                 \
    if (pthread_mutex_lock(l) !=0){                             \
        perror("ERRORE: mutex_lock mutex error");		        \
        pthread_exit((void*)EXIT_FAILURE);				        \
    }

#define WAIT(c,l)                                               \
    if (pthread_cond_wait(c,l) !=0){                            \
        perror("ERRORE: cond_wait cond error");		            \
        pthread_exit((void*)EXIT_FAILURE);				        \
    }

#define SIGNAL(c)                                               \
    if (pthread_cond_signal(c) !=0){	                        \
        perror("ERRORE: cond_signal error");			        \
        pthread_exit((void*)EXIT_FAILURE);					    \
    }

#define BCAST(c)                                                \
    if (pthread_cond_broadcast(c) !=0){		                    \
        perror("ERRORE: cond_broadcast error");			        \
        pthread_exit((void*)EXIT_FAILURE);						\
    }

#define DMUTEX(l)                                               \
    if (pthread_mutex_destroy(l) !=0){                          \
        perror("ERRORE: mutex_destroy error");                  \
        return 0;                                               \
    }

#define DCOND(c)                                                \
    if (pthread_cond_destroy(c) !=0){                           \
        perror("ERRORE: cond_destroy error");                   \
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

//esegue una malloc e restituisce il puntatore void *
void * _malloc_(size_t size);

int write_n(int fd, void *buf, size_t size);

int read_n(int fd, void *buf, size_t size);

#endif