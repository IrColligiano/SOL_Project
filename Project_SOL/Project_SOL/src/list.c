
#include <list.h>

List * Queue=NULL;

int init_list_mod(long n, long msec){
	Queue = (List*) _malloc_(sizeof(List));
	Queue->cond_term=TRUE;
	Queue->msec=msec;
	Queue->len=0;
	Queue->max_len=(size_t)n;
	Queue->tailer=NULL;
	Queue->header=NULL;
	errno=0;
	if( pthread_mutex_init(&(Queue->list_mutex),NULL)!=0){
    	errno=EINVAL;
        return EXIT_FAILURE;
    }
	if(pthread_cond_init(&(Queue->list_cond),NULL)!=0){
    	errno=EINVAL;
        return EXIT_FAILURE;
    }
	if(pthread_cond_init(&(Queue->list_full_cond),NULL)!=0){
    	errno=EINVAL;
        return EXIT_FAILURE;
    }
	return EXIT_SUCCESS;
}

int init_list(List ** l,long n, long msec){
	*l = (List*) _malloc_(sizeof(List));
	(*l)->cond_term=TRUE;
	(*l)->msec=msec;
	(*l)->len=0;
	(*l)->max_len=(size_t)n;
	(*l)->tailer=NULL;
	(*l)->header=NULL;
	errno=0;
	if( pthread_mutex_init(&((*l)->list_mutex),NULL)!=0){
    	errno=EINVAL;
        return EXIT_FAILURE;
    }
	if(pthread_cond_init(&((*l)->list_cond),NULL)!=0){
    	errno=EINVAL;
        return EXIT_FAILURE;
    }
	if(pthread_cond_init(&((*l)->list_full_cond),NULL)!=0){
    	errno=EINVAL;
        return EXIT_FAILURE;
    }
	return EXIT_SUCCESS;
}

list * malloc_nodo(list * nodo){
	nodo =_malloc_(sizeof(list));
	nodo = _malloc_(sizeof(char)*PATHLEN);
	memset(nodo->path,'\0',PATHLEN);
	nodo->next=NULL;
	nodo->prec=NULL;
	return nodo;
}

//inserisce elementi in testa alla lista
//se maxlen==-1 inserisce in una lista contentente tutti i path dei file 
//altrimenti inserisce nella lista concorrente 
int head_insert_mod (char n[]){
	if(Queue->len >= Queue->max_len)
		return Queue->len;
	list * new= _malloc_(sizeof(list));
	new->path= _malloc_(sizeof(char)*PATHLEN);
	memset(new->path,'\0',PATHLEN);
	new->next=NULL;
	new->prec=NULL;
	//new=(list*)malloc_nodo(new);
	strncpy(new->path,n,PATHLEN);
	if(Queue->len==0)
		Queue->tailer=new;
	else{
		Queue->header->prec=new;
		new->next=Queue->header;
	}
	Queue->header=new;
	Queue->len++;
	return Queue->len;
}

int head_insert (List ** l, char n[]){
	if((*l)->len>=(*l)->max_len)
		return (*l)->len;
	list * new=NULL;
	new=(list*)malloc_nodo(new);
	strncpy(new->path,n,PATHLEN);
	if((*l)->len==0)
		(*l)->tailer=new;
	else{
		(*l)->header->prec=new;
		new->next=(*l)->header;
	}
	(*l)->header=new;
	(*l)->len++;
	return (*l)->len;
	//}
}

void print_list_mod(){
	if(Queue->len==0){
		printf("lista vuota\n");
		return;
	}
	list * tmp=Queue->header;
	while(tmp->next!=NULL){
		printf("%s -> ",tmp->path);
		tmp=tmp->next;
	}
	printf("%s\n",tmp->path);
	return;
}

//stampa la lista
void print_list(List * l){
	if(l->len==0){
		printf("lista vuota\n");
		return;
	}
	list * tmp=l->header;
	while(tmp->next!=NULL){
		printf("%s -> ",tmp->path);
		tmp=tmp->next;
	}
	printf("%s\n",tmp->path);
	return;
}

//elimina l ultimo elemento della lista e restituisce:
//il path dell elemento eliminato,'\0'se non e stato elimanto 
void delete_last_mod(char ** ret){
	if(Queue->len == 0)
		return;
	list * tmp=Queue->tailer;
	if(Queue->len == 1){
		Queue->tailer = NULL;
		Queue->header = NULL;
	}
	else{
		Queue->tailer->prec->next = NULL;
		Queue->tailer = Queue->tailer->prec;
		tmp->prec = NULL;
	}
	Queue->len--;
	strncpy(*ret , tmp->path , PATHLEN);
	free(tmp->path);
	tmp->path=NULL;
	free(tmp);
	tmp=NULL;
	return;
}
void delete_last(List ** l, char ** ret){
	if((*l)->len == 0)
		return;
	list * tmp=(*l)->tailer;
	if((*l)->len == 1){
		(*l)->tailer = NULL;
		(*l)->header = NULL;
	}
	else{
		(*l)->tailer->prec->next = NULL;
		(*l)->tailer = (*l)->tailer->prec;
		tmp->prec = NULL;
	}
	(*l)->len--;
	strncpy(*ret , tmp->path , PATHLEN);
	free(tmp);
	tmp=NULL;
	return;
}
void free_List_mod(){
	if(Queue->len==0)
		return;
	list * tmp=Queue->tailer;
	if(Queue->len==1){
		Queue->tailer=NULL;
		Queue->header=NULL;
	}
	else{
		Queue->tailer->prec->next=NULL;
		Queue->tailer=Queue->tailer->prec;
		tmp->prec=NULL;
	}
	Queue->len--;
	free(tmp->path);
	tmp->path=NULL;
	free(tmp);
	tmp=NULL;
	free_List_mod(Queue);
	return;
}

void while_free_List(){
	if(Queue->len == 0)
		return;
	while(Queue->len != 0){
		list * tmp=Queue->tailer;
		if(Queue->len==1){
			Queue->tailer=NULL;
			Queue->header=NULL;
			Queue->len--;
			free(tmp->path);
			tmp->path=NULL;
			free(tmp);
			tmp=NULL;
		}
		else{
			Queue->tailer->prec->next=NULL;
			Queue->tailer=Queue->tailer->prec;
			tmp->prec=NULL;
			Queue->len--;
			free(tmp->path);
			tmp->path=NULL;
			free(tmp);
			tmp=NULL;
		}
		return;
	}


	return;
}

void free_List(List ** l){
	if((*l)->len==0)
		return;
	list * tmp=(*l)->tailer;
	if((*l)->len==1){
		(*l)->tailer=NULL;
		(*l)->header=NULL;
	}
	else{
		(*l)->tailer->prec->next=NULL;
		(*l)->tailer=(*l)->tailer->prec;
		tmp->prec=NULL;
	}
	(*l)->len--;
	free(tmp);
	tmp=NULL;
	free_List(l);
	return;
}
