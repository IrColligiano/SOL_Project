#ifndef __D_STRUCT__
#define __D_STRUCT__

#define PATHLEN      257 // lunghezza path
#define NTHREADDEF   4   //valore default numero thread
#define QLENDEF      8   //valore default lunghezza lista
#define TDELAYDEF    0   //valore default tempo ms
#define TRUE         1   //TRUE
#define FALSE        0	 //FALSE

typedef struct pool{
	pthread_t     	*arr_th;
	size_t 			n_th;
	size_t			n_th_on_work;
	pthread_mutex_t sck_mutex;
	pthread_mutex_t change_mutex;
	pthread_cond_t  join_cond;
} t_pool;

typedef struct node{
    char 			path[PATHLEN];
    struct node 	*prec;
    struct node 	*next;
}list;

typedef struct list_t{
	pthread_mutex_t	list_mutex;
	pthread_cond_t	list_cond;
	pthread_cond_t  list_full_cond;
	long 			msec;
	size_t		 	len;
	size_t 			max_len;
	struct node 	*header;
	struct node 	*tailer;
	int 			cond_term;
	int				cond_semi_term;
	int 			stop;
}List;

typedef struct abr_{
	long 			res;
	char 			path[PATHLEN];
	struct abr_ 	*left;
	struct abr_ 	*right;
}abr;

#endif