#ifndef __DATA_STRUCT__
#define __DATA_STRUCT__

#define PATHLEN      257 // lunghezza path
#define NTHREADDEF   4   //valore default numero thread
#define QLENDEF      8   //valore default lunghezza lista
#define TDELAYDEF    0   //valore default tempo ms
#define TRUE         1   //TRUE
#define FALSE        0	 //FALSE
#define SCKNAME      "./farm.sck"
#define SCKLEN       12

typedef struct pool_t{
	pthread_t     	*arr_th;
	size_t 			n_th;
	size_t			n_th_on_work;
	int 			skt;
	pthread_mutex_t sck_mutex;
	pthread_mutex_t join_mutex;
	pthread_cond_t  join_cond;
} pool;

typedef struct node{
    char 			*path;
    struct node 	*prec;
    struct node 	*next;
} list;

typedef struct list_t{
	pthread_mutex_t	list_mutex;
	pthread_cond_t	list_cond;
	pthread_cond_t  list_full_cond;
	long 			msec;
	size_t		 	len;
	size_t 			max_len;
	list		 	*header;
	list 			*tailer;
	int 			cond_term;
} List;

typedef struct abr_t{
	size_t			len;
	long 			res;
	char 			*path;
	struct abr_t 	*left;
	struct abr_t 	*right;
} abr;

#endif