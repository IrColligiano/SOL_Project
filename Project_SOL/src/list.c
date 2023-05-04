
#include <list.h>

List * Queue = NULL;

int init_list(long n, long msec){
	Queue =            (List*)_malloc_(sizeof(List));
	Queue->cond_term = TRUE;
	Queue->msec =      msec;
	Queue->len =       0;
	Queue->max_len =   (size_t)n;
	Queue->tailer =    NULL;
	Queue->header =    NULL;
	errno =            0;
	if( pthread_mutex_init(&(Queue->list_mutex),NULL) !=0){
    	errno = EINVAL;
        return EXIT_FAILURE;
    }
	if(pthread_cond_init(&(Queue->list_cond),NULL) !=0){
    	errno = EINVAL;
        return EXIT_FAILURE;
    }
	if(pthread_cond_init(&(Queue->list_full_cond),NULL) !=0){
    	errno = EINVAL;
        return EXIT_FAILURE;
    }
	return EXIT_SUCCESS;
}

//inserisce elementi in testa alla lista
//se maxlen==-1 inserisce in una lista contentente tutti i path dei file 
//altrimenti inserisce nella lista concorrente 
int head_insert(char n[], size_t len){
	if(Queue->len >= Queue->max_len)
		return -1;
	list * new = _malloc_(sizeof(list));
	new->path =  _malloc_(sizeof(char)*(len+1));
	memset(new->path,'\0',len+1);
	new->next =  NULL;
	new->prec =  NULL;
	strncpy(new->path , n , len);
	if(Queue->len == 0)
		Queue->tailer = new;
	else{
		Queue->header->prec = new;
		new->next = Queue->header;
	}
	Queue->header = new;
	Queue->len ++;
	return Queue->len;
}

void print_list(){
    if(Queue->len == 0){
		printf("lista vuota\n");
		return;
	}
	list * tmp = Queue->header;
	while(tmp->next != NULL){
		printf("%s -> ",tmp->path);
		tmp = tmp->next;
	}
	printf("%s\n",tmp->path);
	return;
}

//elimina l ultimo elemento della lista e modifica la stringa passata
void delete_last(char ** ret){
	if(Queue->len == 0)
		return;
	list * tmp = Queue->tailer;
	if(Queue->len == 1){
		Queue->tailer = NULL;
		Queue->header = NULL;
	}
	else{
		Queue->tailer->prec->next = NULL;
		Queue->tailer = Queue->tailer->prec;
		tmp->prec = NULL;
	}
	Queue->len --;
	strncpy(*ret , tmp->path , PATHLEN);
	free(tmp->path);
	tmp->path = NULL;
	free(tmp);
	tmp = NULL;
	return;
}

void free_list(){
	if(Queue->len == 0)
		return;
	list * tmp = Queue->tailer;
	if(Queue->len == 1){
		Queue->tailer = NULL;
		Queue->header = NULL;
	}
	else{
		Queue->tailer->prec->next = NULL;
		Queue->tailer = Queue->tailer->prec;
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
