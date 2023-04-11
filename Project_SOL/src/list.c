
#include <list.h>

int init_list(List * l,long n, long msec){
	l->cond_semi_term=TRUE;
	l->cond_term=TRUE;
	l->msec=msec;
	l->len=0;
	l->max_len=(size_t)n;
	l->tailer=NULL;
	l->header=NULL;
	errno=0;
	if( pthread_mutex_init(&(l->list_mutex),NULL)!=0){
    	errno=EINVAL;
        return EXIT_FAILURE;
    }
	if(pthread_cond_init(&(l->list_cond),NULL)!=0){
    	errno=EINVAL;
        return EXIT_FAILURE;
    }
	if(pthread_cond_init(&(l->list_full_cond),NULL)!=0){
    	errno=EINVAL;
        return EXIT_FAILURE;
    }
	return EXIT_SUCCESS;
}

list * malloc_nodo(list * nodo){
	nodo = _malloc_(sizeof(list));
	memset(nodo->path,'\0',PATHLEN-1);
	nodo->next=NULL;
	nodo->prec=NULL;
	return nodo;
}

//inserisce elementi in testa alla lista
//se maxlen==-1 inserisce in una lista contentente tutti i path dei file 
//altrimenti inserisce nella lista concorrente 
int head_insert (List * l, char n[]){
	list * new=NULL;
	if(l->len>=l->max_len)
		return l->len;
	new= malloc_nodo(new);
	strncpy(new->path,n,PATHLEN-1);
	if(l->len==0)
		l->tailer=new;
	else{
		l->header->prec=new;
		new->next=l->header;
	}
	l->header=new;
	l->len++;
	return l->len;
	//}
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
void delete_last(List * l, char ** ret){
	if(l->len==0)
		return;
	list * tmp=l->tailer;
	if(l->len==1){
		l->tailer=NULL;
		l->header=NULL;
	}
	else{
		l->tailer->prec->next=NULL;
		l->tailer=l->tailer->prec;
		tmp->prec=NULL;
	}
	l->len--;
	strncpy(*ret , tmp->path , PATHLEN-1);
	free(tmp);
	return;
}

void free_List(List * l){
	if(l->len==0)
		return;
	list * tmp=l->tailer;
	if(l->len==1){
		l->tailer=NULL;
		l->header=NULL;
	}
	else{
		l->tailer->prec->next=NULL;
		l->tailer=l->tailer->prec;
		tmp->prec=NULL;
	}
	l->len--;
	free(tmp);
	free_List(l);
	return;
}
