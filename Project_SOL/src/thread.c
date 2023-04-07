
#include <thread.h>
/*
void convert(msg * messaggio , char ** buf){
    char tmp[MSGSCK-PATHLEN-2];
    sprintf(tmp,"|%ld",messaggio->res);
    strncpy(*buf,messaggio->path,PATHLEN-1);
    strncat(*buf,tmp,MSGSCK-PATHLEN-1);
    return;
}
*/
/*
void * thread_delivery(void * arg){
    int err;
    msg * messaggio=_malloc_(sizeof(msg));
    messaggio->path=_malloc_(sizeof(char)*PATHLEN);
    char *buf=_malloc_(sizeof(char)*MSGSCK);
    memset(buf,'\0',MSGSCK);
    int exit=TRUE;
    char notifica [9]={'\0'};
    size_t sizenotifica=9;
    memset(&sa, 0, sizeof(sa));
    sa.sun_family=AF_UNIX;
    strncpy(sa.sun_path, SCKNAME ,SCKLEN);
    if((fd_skt=socket(AF_UNIX,SOCK_STREAM,0))==-1){
        fprintf(stderr,"ERRORE: Sck Masterworker\n");
        errno = EINVAL;
        pthread_exit((void*)NULL);
    }
    //printf("CLIENT: socket fatta\n");
    while ( (err=connect( fd_skt , (struct sockaddr*)&sa , sizeof(sa) ) ) == -1 ){
        if ( errno == ENOENT ) 
            sleep(1);
        else{
            if(err==0)
                break;
            else{
                fprintf(stderr,"ERRORE: masterworker connect\n");
                pthread_exit((void*)NULL);
            }
        }
    }
    //printf("CLIENT: connect fatta\n");
    while(exit==TRUE){
        LOCK(&mutex2);
        if(Listforsend== NULL && nthread==0)
            exit=FALSE;
        while(Listforsend == NULL && exit==TRUE){
            WAIT(&cond2,&mutex2);
        }
        if(exit==TRUE){
            messaggio=delete_head_(&Listforsend, messaggio);
            //printf("CLIENT: Invio %s , %ld\n",messaggio->path,messaggio->res);
            convert(messaggio,&buf);
            if(write(fd_skt ,buf,MSGSCK)==-1)
                pthread_exit((void*)NULL);
            if(read(fd_skt ,notifica,sizenotifica)==-1)
                pthread_exit((void*)NULL);
            UNLOCK(&mutex2); 
        }
        else{
            UNLOCK(&mutex2);
        }
        //printf("CLIENT: Finito un ciclo\n");
    }
    //printf("CLIENT: esco dal ciclo\n");
    messaggio->res=-1;
    _strcpy_(&(messaggio->path),"stop",PATHLEN);
    convert(messaggio,&buf);
    if(write(fd_skt ,buf,MSGSCK)==-1)
        pthread_exit((void*)NULL);
    if(read(fd_skt ,notifica,sizenotifica)==-1)
        pthread_exit((void*)NULL);
    //printf("CLIENT: STOP!\n");
    close(fd_skt);
    printf("CLIENT: Mi disconnetto\n");
    free(messaggio->path);
    free(messaggio);
    free(buf);
    //printf("SERVER : Mi disconnetto\n");
    pthread_exit((void*)NULL);
}
*/

int  thread_create(long number_th){
    th_pool=(t_pool*)_malloc_(sizeof(t_pool));
    th_pool->n_th=(size_t)number_th;
    th_pool->arr_th=(pthread_t*)_malloc_(sizeof(pthread_t)*(th_pool->n_th));
    th_pool->n_th_on_work=(size_t)number_th;
    errno=0;
    if(th_pool->n_th<=0 || Queue.max_len <=0){
        errno=EINVAL;
        return EXIT_FAILURE;
    }
    if( pthread_mutex_init(&(th_pool->change_mutex),NULL)!=0){
        fprintf(stderr,"ERRORE: Inizializzazione variabile di mutex\n");
        errno=EINVAL;
        return EXIT_FAILURE;
    }
    if( pthread_mutex_init(&(th_pool->sck_mutex),NULL)!=0){
        fprintf(stderr,"ERRORE: Inizializzazione variabile di mutex\n");
        errno=EINVAL;
        return EXIT_FAILURE;
    }
    if( pthread_cond_init(&(th_pool->join_cond),NULL)!=0){
        fprintf(stderr,"ERRORE: Inizializzazione variabile di condizione\n");
        errno=EINVAL;
        return EXIT_FAILURE;
    }
    for(int i=0; i < th_pool->n_th; i++){
        errno=0;
        int err;
        if((err = pthread_create(&(th_pool->arr_th[i]) , NULL , &thread_work , NULL)!=0) ){
            fprintf(stderr,"ERRORE: Creazione thread n : %d\n",i);
            errno=EFAULT;
            return EXIT_FAILURE;

        }
    }
    return EXIT_SUCCESS;
}

int exit_ = TRUE;

void * thread_work(void * arg){
    while(TRUE){
        LOCK(&(Queue.list_mutex));
        while(Queue.len==0 && exit_){
            WAIT(&(Queue.list_cond),&(Queue.list_mutex));
        }
        if(exit_){
            char buffer[PATHLEN];
            memset(buffer,'\0',PATHLEN-1);
            char *ret=_malloc_(sizeof(char)*PATHLEN);
            memset(ret,'\0',PATHLEN-1);
            FILE * fp=NULL;
            delete_last(&Queue,&ret);
            if(!Queue.cond_term && Queue.len==0){
                exit_=FALSE;
                printf("ciao2\n");
            }
            strncpy(buffer,ret,PATHLEN-1);
            SIGNAL(&(Queue.list_full_cond));
            UNLOCK(&(Queue.list_mutex));
            free(ret);
            if ((fp=fopen(buffer,"rb"))==NULL){
			    fprintf(stderr, "ERRORE:Fopen %s\n",buffer);
			    pthread_exit((void*) EXIT_FAILURE);
		    }
            int err=0;
            int i=0;
            long result=0;
            long x=0;
            while((err=fscanf(fp,"%ld",&x))==1){
                result=result +( x*i);
                ++i;
            }
            if(err==EOF){// sono arrivato in fondo al file
                LOCK(&(th_pool->sck_mutex));
                fprintf(stdout,"Il risultato del file %s e' %ld\n",buffer,result);
                ///qui dovrei scrivere sul canale;
                UNLOCK(&(th_pool->sck_mutex));
            }
            if(err==0){ // ho letto caratteri nel file, che non voglio
                //fprintf(stdout,"Thread %ld il file %s non corrisponde alle specifiche\n",(long)arg,buffer);
            }
            fclose(fp);
            if(errno==EBADF){
                fprintf(stderr, "ERRORE: Fclose %s\n",buffer);
			    pthread_exit((void*)EXIT_FAILURE); 
		    }
        }
        else{
            printf("ciao\n");
            BCAST(&(Queue.list_cond));
            UNLOCK(&(Queue.list_mutex));
            break;
        }
    }
    LOCK(&(th_pool->change_mutex));
    th_pool->n_th_on_work--;
    printf("vado in vacanza %ld\n",th_pool->n_th_on_work);
    if(th_pool->n_th_on_work<=0){
        SIGNAL(&(th_pool->join_cond));
    }
    UNLOCK(&(th_pool->change_mutex));
    return (void *)NULL;
}

int JOIN(pthread_t th ){
    errno=0;
    if(pthread_join(th,NULL)!=0){
        if(errno==EDEADLK || errno == EINVAL || errno==ESRCH){
            fprintf(stderr, "ERRORE: Join\n");
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
