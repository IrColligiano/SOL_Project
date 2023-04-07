#define _GNU_SOURCE

#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <getopt.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <thread.h>

//#include <masterworker.h>
//#include <collector.h>

int msleep(long msec);
long arg_int(const char* n);
int isNumber(const char* s, long* tmp);
void list_files_recursively(char *basePath,List * l);
char * arg_char_dir(char * n);
int is_regular_file(const char *path);
int is_directory(const char *path);

//int fd_skt, fd_c; // sck
//struct sockaddr_un sa;
long Nthread= NTHREADDEF;    // variabile del numero di thread
long Qlen= QLENDEF;          //variabile per la lunghezza della coda
long Tdelay= TDELAYDEF;     //variabile per i ms
char *DirectoryName=NULL;
char *Filename=NULL;
List Queue;
t_pool * th_pool=NULL;
//-------------------------MAIN--------------------------------

int main(int argc,char *argv[]){
    // controllo che ci sia almeno un argomento in argc 
    if(argc<2){
        fprintf(stderr,"ERRORE: Troppi pochi argomenti in ingresso\n");
        return EXIT_FAILURE;
    }
    pid_t pid;//pid del processo figlio
    int opt; // variabile per la gestione dello switch da parte di getopt
    long tmp; // variabile temporanea
    int opt_n=0;
    int opt_q=0;
    int opt_t=0;
    int opt_d=0;
    //int i=0; // indice
    char*a=NULL; //puntatore char di appoggio per getopt ed altre funzioni del main
    
    //whille per il parser getopt
    while((opt = getopt(argc, argv, "n:q:d:t:")) != -1){
        switch (opt){
//-n <nthread> specifica il numero di thread Worker del processo MasterWorker (valore di default 4)
        case 'n': 
            opt_n++;
            if(opt_n>1){
                fprintf(stderr,"ERRORE: Argomento '-n' puo' essere passato solo una volta\n");
                return EXIT_FAILURE;
            }
            tmp=arg_int(optarg);
            if(tmp<=-1){
                fprintf(stderr,"ERRORE: Argomento '-n' non valido, numero di threads impostato a: %ld\n",Nthread);
            } 
            else{
                Nthread=tmp;
                printf("Numero di threads impostato a: %ld\n",Nthread);
            }     
            break;
//-q <qlen> specifica la lunghezza della coda concorrente tra il thread Master ed i thread Worker (valore di default 8)
        case 'q':
            opt_q++;
            if(opt_q>1){
                fprintf(stderr,"ERRORE: Argomento '-q' puo' essere passato solo una volta\n");
                return EXIT_FAILURE;
            } 
            tmp=arg_int(optarg);
            if(tmp<=-1){
                fprintf(stderr,"ERRORE: Argomento '-q' non valido, lunghezza coda concorrente impostato a: %ld\n",Qlen);
            }
            else{
                Qlen=tmp;
                printf("Lunghezza coda concorrente impostato a: %ld\n",Qlen);                    
            }     
            break;
//-d <directory-name> specifica una directory in cui sono contenuti file binari ed eventualmente altre directory
//   contenente file binari, i file binari dovranno essere utilizzati come file di input per il calcolo;
        case 'd':
            opt_d++;
            if(opt_d>1){
                fprintf(stderr,"ERRORE: Argomento '-d' puo' essere passato solo una volta\n");
                return EXIT_FAILURE;
            }
            a=arg_char_dir(optarg);
            if(a==NULL){
                fprintf(stderr,"ERRORE: La directory specificata non e' valida o non esiste\n");
            }
            else{
                DirectoryName=_malloc_(sizeof(char)*PATHLEN);
                strncpy(DirectoryName,a,PATHLEN-1);
                fprintf(stdout,"Directory %s\n",DirectoryName);
            }
            break;
//-t <delay> specifica un tempo in millisecondi che intercorre tra l’invio di due richieste 
//   successive ai thread Worker da parte del thread Master (valore di default 0)
        case 't':
            opt_t++;
            if(opt_t>1){
                fprintf(stderr,"ERRORE: Argomento '-t' puo' essere passato solo una volta\n");
                return EXIT_FAILURE;
            }
            tmp=arg_int(optarg);
            if(tmp<=-1){
                fprintf(stderr,"ERRORE: Argomento '-t' non valido, Delay impostato a: %ldms\n",Tdelay);
            }
            else{
                Tdelay=tmp;
                
                printf("Delay impostato a: %ldms\n",Tdelay);                
            }   
            break;
        default:;
        }
    }
    /*
    int exitwhile=TRUE;
    while( exitwhile ){
        if(condterm==FALSE)
            exitwhile=FALSE;

        if(Listoffile.len==0){
            condsemiterm=FALSE;
            if(Coda.len==0)
                condterm=FALSE;
        }

        if(exitwhile && condsemiterm && condterm){
            ts.tv_sec = Tdelay / 1000;
            ts.tv_nsec = (Tdelay % 1000)*1000000;
            nanosleep(&ts, &ts);
            buffer=delete_last(&Listoffile,&buffer);
            LOCK(&mutex);
            head_insert(&Coda,buffer);
            SIGNAL(&cond);
            UNLOCK(&mutex);
        }
    }
    */
    if(init_list(&Queue,Qlen,Tdelay)!=0){
        fprintf(stderr,"ERRORE: Inizializzazione coda concorrente\n");
        return EXIT_FAILURE;
    }
    printf("inizio \n");
    if(thread_create(Nthread)!=0){
        fprintf(stderr,"ERRORE: Creazione thread\n");
        return EXIT_FAILURE;
    }
    int j=argc-1;
    int exit_while=TRUE;
    while(j>0 && is_regular_file(argv[j])!=0){
        LOCK(&(Queue.list_mutex));
        msleep(Queue.msec);
        while(Queue.len>=Queue.max_len)
            WAIT(&(Queue.list_full_cond),&(Queue.list_mutex));
        head_insert(&Queue,argv[j]);
        SIGNAL(&(Queue.list_cond));
        UNLOCK(&(Queue.list_mutex));
        j--;
        //printf("%s\n",argv[j]);
    }
    if(DirectoryName!=NULL){
        list_files_recursively(DirectoryName,&Queue);
        free(DirectoryName);
    }
    Queue.cond_term=FALSE;
    LOCK(&(th_pool->change_mutex));
    while(th_pool->n_th_on_work > 0){
        WAIT(&(th_pool->join_cond),&(th_pool->change_mutex));
        printf("mi sono svegliato\n");   
    }
    UNLOCK(&(th_pool->change_mutex));
    for(j=0 ; j<th_pool->n_th ; j++){
        if(JOIN((th_pool->arr_th[j]))==EXIT_FAILURE){
            return EXIT_FAILURE;
        }
    }
    free(th_pool->arr_th);
    free(th_pool);
    /*
    if((pid=fork()) == -1) {
    	fprintf(stderr,"ERRORE: Fork\n");
        return EXIT_FAILURE;
    }
    if(pid==0) {// avvio processo msterworker che rappresentera' il client
        init_list(&Listoffile,-1);
        list_files_recursively(DirectoryName,&Listoffile);
        //controllo che ci sia almeno un file di input
        if(Listoffile.len==0 && DirectoryName==NULL){
            fprintf(stderr,"ERRORE: Nessun file in input\n");
            return EXIT_FAILURE;
        }

        free(DirectoryName);
        init_list(&Coda,Qlen);
        fprintf(stdout,"PROCESSO MASTERWOKER: AVVIO\n");
        if(master_worker()!=0){
            fprintf(stderr,"ERRORE: MasterWorker\n");
            return EXIT_FAILURE;
        }
        printf("PROCESSO MASTERWORKER : TERMINO\n");
    }

    if(pid!=0){ // avvio processo collector che rappresentera' il server
        free(DirectoryName);
        printf("PROCESSO COLLECTOR: AVVIO\n");
        if(collector()!=0){
            fprintf(stderr,"ERRORE: Collector\n");
            return EXIT_FAILURE;
        }
        printf("PROCESSO COLLECTOR: TERMINO\n");
    }
    */
    return EXIT_SUCCESS;
}

int msleep(long msec){
    struct timespec ts;
    int res;
    if (msec < 0){
        errno = EINVAL;
        return -1;
    }
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    do{
        res = nanosleep(&ts, &ts);
    } 
    while (res && errno == EINTR);
    return res;
}

int is_regular_file(const char *path){
    struct stat pathstat;
    if(stat(path, &pathstat) != 0)
        return 0;
    return S_ISREG(pathstat.st_mode);
}


int is_directory(const char *path){
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

void list_files_recursively(char *basePath,List * l){
    char *path=_malloc_(sizeof(char)*PATHLEN);
    char *fullpath=_malloc_(sizeof(char)*PATHLEN);
    memset(path,'\0',1);
    memset(fullpath,'\0',1);
    size_t pathlen=0;
    size_t basepathlen=strnlen(basePath,PATHLEN);
    size_t namelen=0;
    struct dirent *dp=NULL;
    char slash[2]={'/','\0'};
    DIR *dir = opendir(basePath);
    if(dir==NULL){
        free(path);
        free(fullpath);
        return;
    }
    while ((dp = readdir(dir)) != NULL){
        errno=0;
        if(errno==EBADF){
            closedir(dir);
            free(fullpath);
            free(path);
        return;
        }
        namelen=strnlen(dp->d_name,PATHLEN);
        if (strncmp(dp->d_name,".",namelen) != 0 && strncmp(dp->d_name, "..",namelen) != 0){
            pathlen=strnlen(path,PATHLEN);
            if ( !(pathlen + basepathlen + namelen >= PATHLEN ) ){
                if( dp->d_type == DT_REG && !(basepathlen + namelen  >= PATHLEN ) ){
                    strncpy(fullpath,basePath,PATHLEN);
                    strncat(fullpath,slash,1);
                    strncat(fullpath,dp->d_name,namelen);
                    if(is_regular_file(fullpath)!=0){
                        LOCK(&(Queue.list_mutex));
                        msleep(l->msec);
                        while(l->len>=l->max_len){
                             WAIT(&(Queue.list_full_cond),&(Queue.list_mutex));
                        }
                        head_insert(l,fullpath);
                        SIGNAL(&(Queue.list_cond));
                        UNLOCK(&(Queue.list_mutex));
                    }
                    memset(fullpath,'\0',1);
                }
                strncpy(path, basePath,PATHLEN);
                strncat(path, slash,1);
                strncat(path, dp->d_name,namelen);
                list_files_recursively(path,l);
            }
        }
    }
    closedir(dir);
    free(fullpath);
    free(path);
    return;
}



int isNumber(const char* s, long* tmp){
    if (s==NULL) 
        return 1;
    if (strnlen(s,PATHLEN)==0) 
        return 1; 
    char* e = NULL;
    errno=0;
    long val = strtol(s, &e, 10); //strtol converte stringhe in long integer
    if (errno == ERANGE)
        return 2; // overflow
    if (e != NULL && *e == (char)0) {
        *tmp = val;
        return 0; // e' un numero
    }
    return 1; // non e' un numero
}

long arg_int(const char* n){
    long tmp=0; //variabile temporanea che mi raccoglie il long gestito da isNumber
    if (isNumber(n , &tmp) != 0 && tmp<0)
        return -1;
    else
        return tmp; 
}

char* arg_char_dir(char * n){
    DIR* dir = opendir(n);
    if (dir){ // la directory esiste
        closedir(dir);
        return n;
    }
    if (ENOENT == errno){ //la directory non esiste
        return NULL;
    }
    else{ // opendir() ha fallito per altri motivi
        return NULL;
    }
}
