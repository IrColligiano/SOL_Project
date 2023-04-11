
#include <thread.h>
#include <unistd.h>
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

long result_from_path(char* pathname) {
    if(pathname == NULL) {
        errno = EINVAL;
        return -1;
    }
    size_t n_byte=0;
    size_t n_byte_for_time=1;
    long res = 0;
    long i = 0;
    long x=0;
    FILE* fp=NULL;
    if((fp=fopen(pathname, "rb")) == NULL) {
        fprintf(stderr,"ERRORE: fopen %s\n",pathname);
        return -1;
    }
    while((n_byte=fread(&x , sizeof(long), n_byte_for_time , fp))== 1) {
        res = res +(x * i);
        i++;
    }
    if(feof(fp)){
        fclose(fp);
        return res;
    }
    else{
        fclose(fp);
    return -1;
    }
}


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
    int exit_while = TRUE;
    while(exit_while){
        LOCK(&(Queue.list_mutex));
        if(!(Queue.cond_term) && Queue.len==0){
            exit_=FALSE;
            BCAST(&(Queue.list_cond));
            exit_while=FALSE;
            }
        while(Queue.len==0 && exit_ && exit_while){
            WAIT(&(Queue.list_cond),&(Queue.list_mutex));
            //printf("Thread sveglo\n");
        }
        if(exit_ && exit_while){
            long result=0;
            int err=0;
            //FILE * fp=NULL;
            char buffer[PATHLEN];
            memset(buffer,'\0',PATHLEN-1);
            char *ret=_malloc_(sizeof(char)*PATHLEN);
            memset(ret,'\0',PATHLEN-1);
            delete_last(&Queue,&ret);
            SIGNAL(&(Queue.list_full_cond));
            UNLOCK(&(Queue.list_mutex));
            strncpy(buffer,ret,PATHLEN-1);
            free(ret);
            if((result=result_from_path(buffer))==-1){
                fprintf(stderr,"ERRORE: Lettura file\n");
            }
            else{
                LOCK(&(th_pool->sck_mutex));
                fprintf(stdout,"Il risultato del file %s e' %ld\n",buffer,result);
                ///qui dovrei scrivere sul canale;
                UNLOCK(&(th_pool->sck_mutex));
            }
        }
        else{
            BCAST(&(Queue.list_cond));
            exit_while=FALSE;
            UNLOCK(&(Queue.list_mutex));
        }
    }
    LOCK(&(th_pool->change_mutex));
    th_pool->n_th_on_work--;
    //printf("Vado in vacanza %ld\n",th_pool->n_th_on_work);
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


