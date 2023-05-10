
#include <list.h>

list * Queue = NULL;

int init_list(long n, long msec){
	Queue =            (list*)_malloc_(sizeof(list));
	Queue->cond_term = TRUE;
	Queue->msec =      msec;
	Queue->len =       0;
	Queue->max_len =   (size_t)n;
	Queue->tail =    NULL;
	Queue->head =    NULL;
	errno =            0;
	if( pthread_mutex_init(&(Queue->list_mutex),NULL) !=0){
		errno = EINVAL;
		HANDLE_ERROR("pthread_mutex_init()");
        return EXIT_FAILURE;
    }
	if(pthread_cond_init(&(Queue->list_cond),NULL) !=0){
		errno = EINVAL;
		HANDLE_ERROR("pthread_cond_init()");
        return EXIT_FAILURE;
    }
	if(pthread_cond_init(&(Queue->list_full_cond),NULL) !=0){
		errno = EINVAL;
		HANDLE_ERROR("pthread_cond_init()");
        return EXIT_FAILURE;
    }
	return EXIT_SUCCESS;
}

int head_insertion(char n[], size_t len){
	if(Queue->len >= Queue->max_len)
		return -1;
	node * new = _malloc_(sizeof(node));
	new->path =  _malloc_(sizeof(char)*(len+1));
	new->lenght = len;
	memset(new->path,'\0',len+1);
	new->next =  NULL;
	new->prec =  NULL;
	strncpy(new->path , n , len);
	if(Queue->len == 0)
		Queue->tail = new;
	else{
		Queue->head->prec = new;
		new->next = Queue->head;
	}
	Queue->head = new;
	Queue->len ++;
	return Queue->len;
}

void print_list(){
    if(Queue->len == 0){
		printf("lista vuota\n");
		return;
	}
	node * tmp = Queue->head;
	while(tmp->next != NULL){
		printf("%s -> ",tmp->path);
		tmp = tmp->next;
	}
	printf("%s\n",tmp->path);
	return;
}

void delete_last(char ** ret){
	if(Queue->len == 0)
		return;
	node * tmp = Queue->tail;
	if(Queue->len == 1){
		Queue->tail = NULL;
		Queue->head = NULL;
	}
	else{
		Queue->tail->prec->next = NULL;
		Queue->tail = Queue->tail->prec;
		tmp->prec = NULL;
	}
	Queue->len --;
	strncpy(*ret , tmp->path , tmp->lenght);
	free(tmp->path);
	tmp->path = NULL;
	free(tmp);
	tmp = NULL;
	return;
}

void free_list(){
	if(Queue->len == 0)
		return;
	node * tmp = Queue->tail;
	if(Queue->len == 1){
		Queue->tail = NULL;
		Queue->head = NULL;
	}
	else{
		Queue->tail->prec->next = NULL;
		Queue->tail = Queue->tail->prec;
		tmp->prec = NULL;
	}
	Queue->len --;
	free(tmp->path);
	tmp->path = NULL;
	free(tmp);
	tmp = NULL;
	free_list(Queue);
	return;
}
