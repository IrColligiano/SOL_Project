#ifndef __DATA_STRUCT__
#define __DATA_STRUCT__

#define PATHLEN      256 // lunghezza path
#define NTHREADDEF   4   //valore default numero thread
#define QLENDEF      8   //valore default lunghezza lista
#define TDELAYDEF    0   //valore default tempo ms
#define TRUE         1   //TRUE
#define FALSE        0	 //FALSE
#define SCKNAME      "./farm.sck" // nome socket
#define SCKLEN       11 // lunghezza nome socket

//struct che descrive la threadpool
typedef struct pool_t{
	pthread_t     	*arr_th;
	size_t 			n_th;
	size_t			n_th_on_work;
	pthread_mutex_t sck_mutex;
	pthread_mutex_t join_mutex;
} pool;

//struct che descrive un nodo della coda concorrente
typedef struct node_t{
	size_t          lenght;
    char 			*path;
    struct node_t 	*prec;
    struct node_t 	*next;
} node;

//struct che descrive la coda concorrente
typedef struct list_t{
	pthread_mutex_t	list_mutex;
	pthread_cond_t	list_cond;
	pthread_cond_t  list_full_cond;
	long 			msec;
	size_t		 	len;
	size_t 			max_len;
	node		 	*head;
	node 			*tail;
	int 			cond_term;
} list;

//struct che descrive l abr
typedef struct abr_t{
	size_t			len;
	long 			res;
	char 			*path;
	struct abr_t 	*left;
	struct abr_t 	*right;
} abr;

#endif