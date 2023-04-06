
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
/*
int  thread_create(){
    if(Nthread<=0 || Qlen <=0){
        errno=EINVAL;
        return EXIT_FAILURE;
    }
    if( pthread_mutex_init(&mutex3,NULL)!=0||pthread_mutex_init(&mutex,NULL)!=0|| pthread_cond_init(&cond,NULL)!=0 || 
        pthread_cond_init(&cond2,NULL)!=0 || pthread_mutex_init(&mutex2,NULL)!=0){
        errno=EINVAL;
        return EXIT_FAILURE;
    }
    long i=Nthread;
    int err;
    nthread=Nthread;
    if( (pthread_create(&arrth[i] , NULL , &thread_delivery, (void*)i)!=0)){
        errno=EFAULT;
        return EXIT_FAILURE;
    }
    for(i=0; i<Nthread; i++){
        if((err = pthread_create(&arrth[i] , NULL , &thread_work , (void *)i)) !=0 ){
            errno=EFAULT;
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
*/
/*
void * thread_work(void * arg){
    char *buffer=_malloc_(sizeof(char)*PATHLEN);
    char *ret=_malloc_(sizeof(char)*MSGSCK);
    int err=0;
    long result=0;
    int i;
    long x=0;
    msg * messaggio=_malloc_(sizeof(msg));
    messaggio->path=_malloc_(sizeof(char)*PATHLEN);
    FILE * fp;
    while(condterm){
        LOCK(&mutex);
        while(Coda.len==0 && condterm){
            WAIT(&cond,&mutex);
        }
        if(condterm){
            _strcpy_(&buffer,delete_last(&Coda,&ret),PATHLEN);
            UNLOCK(&mutex);
            if ((fp=fopen(buffer,"rb"))==NULL){
			    fprintf(stderr, "ERRORE:Fopen %s\n",buffer);
			    pthread_exit((void*) EXIT_FAILURE);
		    }
            i=0;
            result=0;
            while((err=fscanf(fp,"%ld",&x))==1){
                result=result +( x*i);
                ++i;
            }
            if(err==EOF){// sono arrivato in fondo al file
                //fprintf(stdout,"Thread %ld il risultato del file %s e' %ld\n",(long)arg,buffer,result);
                LOCK(&mutex2);
                _strcpy_(&(messaggio->path),buffer,PATHLEN);
                messaggio->res=result;
                head_insert_(&Listforsend, messaggio->path , messaggio->res);
                SIGNAL(&cond2);
                UNLOCK(&mutex2);
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
            UNLOCK(&mutex);
        } 
    }
    LOCK(&mutex3);
    nthread--;
    UNLOCK(&mutex3);
    free(messaggio->path);
    free(messaggio);
    free(buffer);
    free(ret);
    return (void *)NULL;
}
*/