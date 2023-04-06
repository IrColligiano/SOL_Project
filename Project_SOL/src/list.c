
#include <list.h>

void init_list(List * l,long n, long msec){
	l->msec=msec;
	l->len=0;
	l->max_len=(size_t)n;
	l->tailer=NULL;
	l->header=NULL;
}

list * malloc_nodo(list * nodo){
	nodo = _malloc_(sizeof(list));
	memset(nodo->path,'\0',PATHLEN);
	nodo->next=NULL;
	nodo->prec=NULL;
	return nodo;
}

//inserisce elementi in testa alla lista
//se maxlen==-1 inserisce in una lista contentente tutti i path dei file 
//altrimenti inserisce nella lista concorrente 
int head_insert (List * l, char n[]){
	list * new=NULL;
    /*
	if(l->max_len==-1){
		new= malloc_nodo(new);
		strncpy(new->path,n,PATHLEN);
		if(l->len==0)
			l->tailer=new;
		else{
			l->header->prec=new;
			new->next=l->header;
		}
		l->header=new;
		l->len++;
		return l->len;
		}

	else{*/
	if(l->len>=l->max_len)
		return l->len;
	new= malloc_nodo(new);
	strncpy(new->path,n,PATHLEN);
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
char* delete_last(List * l, char * ret){
	if(l->len==0)
		return (char*)'\0';
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
	strncpy(ret , tmp->path , PATHLEN);
	free(tmp);
	return ret;
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
	free(tmp->path);
	free(tmp);
	free_List(l);
	return;
}
