#define _GNU_SOURCE
#define UNIX_MAX_PATH 108

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
#include <sys/wait.h>
#include <thread.h>
#include <collector.h>

int msleep(long msec);
long arg_int(const char* n);
int isNumber(const char* s, long* tmp);
void list_files_recursively(char *basePath);
char * arg_char_dir( char * n);
int is_regular_file(const char *path);
int is_directory(const char *path);

int fd_skt;
struct sockaddr_un sa;
volatile sig_atomic_t term_for_sig = TRUE;
long Nthread= NTHREADDEF;    // variabile del numero di thread
long Qlen= QLENDEF;          //variabile per la lunghezza della coda
long Tdelay= TDELAYDEF;     //variabile per i ms
char *DirectoryName=NULL;


//-------------------------MAIN--------------------------------

int main(int argc,char *argv[]){
    // controllo che ci sia almeno un argomento in argc 
    if(argc < 2){
        fprintf(stderr,"ERRORE: Troppi pochi argomenti in ingresso\n");
        return EXIT_FAILURE;
    }
    pid_t pid;//pid del processo figlio
    if((pid = fork()) == -1) {
    	fprintf(stderr,"ERRORE: Fork\n");
        return EXIT_FAILURE;
    }
    if(pid == 0){ // figlio che rappresenta il collector 
        if(collector_main() != EXIT_SUCCESS){
            fprintf(stderr,"ERRORE: COLLECTOR return\n");
            exit(EXIT_FAILURE);
        }
    }
    if(pid != 0){ // processo master che rappresenta il main
        int opt; // variabile per la gestione dello switch da parte di getopt
        long tmp; // variabile temporanea
        int opt_n=0;
        int opt_q=0;
        int opt_t=0;
        int opt_d=0;
        char*a=NULL; //puntatore char di appoggio per getopt ed altre funzioni del main
        //while per il parser getopt
        //List * Queue=NULL;
        //t_pool * th_pool=NULL;
        while((opt = getopt(argc, argv, "n:q:d:t:")) != -1){
            switch (opt){
//-n <nthread> specifica il numero di thread Worker del processo MasterWorker (valore di default 4)
            case 'n': 
                opt_n++;
                if(opt_n > 1){
                    fprintf(stderr,"ERRORE: Argomento '-n' puo' essere passato solo una volta\n");
                    return EXIT_FAILURE;
                }
                tmp = arg_int(optarg);
                if(tmp <= -1){
                    fprintf(stderr,"ERRORE: Argomento '-n' non valido, numero di threads impostato a: %ld\n",Nthread);
                } 
                else{
                    Nthread = tmp;
                    fprintf(stdout,"Numero di threads impostato a: %ld\n",Nthread);
                }     
                break;
//-q <qlen> specifica la lunghezza della coda concorrente tra il thread Master ed i thread Worker (valore di default 8)
            case 'q':
                opt_q++;
                if(opt_q > 1){
                    fprintf(stderr,"ERRORE: Argomento '-q' puo' essere passato solo una volta\n");
                    return EXIT_FAILURE;
                } 
                tmp=arg_int(optarg);
                if(tmp <= -1){
                    fprintf(stderr,"ERRORE: Argomento '-q' non valido, lunghezza coda concorrente impostato a: %ld\n",Qlen);
                }
                else{
                    Qlen = tmp;
                    fprintf(stdout,"Lunghezza coda concorrente impostato a: %ld\n",Qlen);                    
                }     
                break;
//-d <directory-name> specifica una directory in cui sono contenuti file binari ed eventualmente altre directory
//   contenente file binari, i file binari dovranno essere utilizzati come file di input per il calcolo;
            case 'd':
                opt_d++;
                if(opt_d > 1){
                    fprintf(stderr,"ERRORE: Argomento '-d' puo' essere passato solo una volta\n");
                    return EXIT_FAILURE;
                }
                a = arg_char_dir(optarg);
                if(a == NULL){
                    fprintf(stderr,"ERRORE: La directory specificata non e' valida o non esiste\n");
                }
                else{
                    DirectoryName = _malloc_(sizeof(char)*PATHLEN);
                    strncpy(DirectoryName,a,PATHLEN-1);
                    fprintf(stdout,"Directory %s\n",DirectoryName);
                }
                break;
//-t <delay> specifica un tempo in millisecondi che intercorre tra l’invio di due richieste 
//   successive ai thread Worker da parte del thread Master (valore di default 0)
            case 't':
                opt_t++;
                if(opt_t > 1){
                    fprintf(stderr,"ERRORE: Argomento '-t' puo' essere passato solo una volta\n");
                    return EXIT_FAILURE;
                }
                tmp = arg_int(optarg);
                if(tmp<=-1){
                    fprintf(stderr,"ERRORE: Argomento '-t' non valido, Delay impostato a: %ldms\n",Tdelay);
                }
                else{
                    Tdelay = tmp;
                    fprintf(stdout,"Delay impostato a: %ldms\n",Tdelay);                
                }   
                break;
            default:;
            }
        }
        memset(&sa, 0, sizeof(sa));
        sa.sun_family = AF_UNIX;
        strncpy(sa.sun_path, SCKNAME ,SCKLEN-1);
        if((fd_skt = socket(AF_UNIX, SOCK_STREAM, 0)) ==-1){
            fprintf(stderr,"ERRORE: Sck Masterworker\n");
            errno = EINVAL;
            return EXIT_FAILURE;
        }
        fprintf(stdout,"MASTER: socket\n");
        int err;
        sleep(0.1);
        while ( (err = connect( fd_skt, (struct sockaddr*)&sa , sizeof(sa) ) ) == -1 ){
            if ( errno == ENOENT ) 
                sleep(1);
            else{
                fprintf(stderr,"ERRORE: Connect %d\n", errno);
                return EXIT_FAILURE;
            }
        }
        fprintf(stdout,"MASTER: connect\n");
        if(signal_handler_master() !=0 ){
            free(DirectoryName);
            return EXIT_FAILURE;
        }
        if(init_list_mod(Qlen,Tdelay) != 0){
            free(DirectoryName);
            fprintf(stderr,"ERRORE: Inizializzazione coda concorrente\n");
            return EXIT_FAILURE;
        }
        /*
        if(init_list(&Queue,Qlen,Tdelay) != 0){
            free(DirectoryName);
            fprintf(stderr,"ERRORE: Inizializzazione coda concorrente\n");
            return EXIT_FAILURE;
        }
        */
        if(thread_create(Nthread) != 0){
            free(DirectoryName);
            fprintf(stderr,"ERRORE: Creazione thread\n");
            return EXIT_FAILURE;
        }
        int j = argc-1;
        while(j > 0 && term_for_sig && (is_regular_file(argv[j]) !=0)){
            LOCK(&(Queue->list_mutex));
            while(Queue->len >= Queue->max_len && term_for_sig)
                WAIT(&(Queue->list_full_cond),&(Queue->list_mutex));
            if(term_for_sig){
                msleep(Queue->msec);
                head_insert_mod(argv[j]);
                //head_insert(&Queue,argv[j]);
                SIGNAL(&(Queue->list_cond));
            }
            UNLOCK(&(Queue->list_mutex));
            j--;
        }
        if(DirectoryName != NULL){
            if(term_for_sig)
                list_files_recursively(DirectoryName);
            free(DirectoryName);
            DirectoryName=NULL;
        }
        /*
        pid_t pid2;
        pid2=getpid();
        kill(pid2,SIGUSR1);
        */
        Queue->cond_term=FALSE;
        LOCK(&(th_pool->join_mutex));
        while(th_pool->n_th_on_work > 0){
            WAIT(&(th_pool->join_cond),&(th_pool->join_mutex));   
        }
        UNLOCK(&(th_pool->join_mutex));
        //fprintf(stdout,"MASTER: sveglo\n");
        for(j = 0 ; j < th_pool->n_th ; j++){
            if(JOIN((th_pool->arr_th[j])) == EXIT_FAILURE){
                return EXIT_FAILURE;
            }
        }
        /*
        DMUTEX(&(th_pool->sck_mutex));
        DMUTEX(&(th_pool->join_mutex));
        DMUTEX(&(Queue->list_mutex));
        DCOND(&(th_pool->join_cond));
        DCOND(&(Queue->list_cond));
        DCOND(&(Queue->list_full_cond));
        free_List_mod();
        free(Queue);
        free(th_pool->arr_th);
        free(th_pool);
        */
        free_resource();
        term_signal_handler();
        close(fd_skt);   
        waitpid(-1, NULL, 0);
        fprintf(stdout,"MASTER: close\n");
    }
    Queue=NULL;
    th_pool=NULL;
    return EXIT_SUCCESS;
}

int msleep(long msec){
    struct timespec ts;
    int res;
    errno=0;
    if (msec < 0){
        errno = EINVAL;
        return -1;
    }
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    //do{
    res = nanosleep(&ts, &ts);
    //} 
    //while (res && errno == EINTR);
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

void list_files_recursively(char *basePath){
    char path[PATHLEN];
    char fullpath[PATHLEN];
    memset(path,'\0',PATHLEN);
    memset(fullpath,'\0',PATHLEN);
    size_t pathlen=0;
    size_t basepathlen=strnlen(basePath,PATHLEN);
    size_t namelen=0;
    struct dirent *dp = NULL;
    char slash[2]={'/','\0'};
    DIR *dir = opendir(basePath);
    if(dir==NULL){
        return;
    }
    while ((dp = readdir(dir)) != NULL && term_for_sig){
        if(errno==EBADF){
            closedir(dir);
            return;
        }
        namelen=strnlen(dp->d_name,PATHLEN-1);
        if (strncmp(dp->d_name,".",namelen) != 0 && strncmp(dp->d_name, "..",namelen) != 0){
            pathlen=strnlen(path,PATHLEN-1);
            if ( !(pathlen + basepathlen + namelen >= PATHLEN ) ){
                if( dp->d_type == DT_REG && !(basepathlen + namelen  >= PATHLEN ) ){
                    strncpy(fullpath,basePath,PATHLEN);
                    strncat(fullpath,slash,1);
                    strncat(fullpath,dp->d_name,namelen);
                    if(is_regular_file(fullpath)!=0 && term_for_sig){
                        LOCK(&(Queue->list_mutex));
                        while(Queue->len >= Queue->max_len && term_for_sig){
                             WAIT(&(Queue->list_full_cond),&(Queue->list_mutex));
                        }
                        if(term_for_sig){
                            if( msleep(Queue->msec) == -1)
                                fprintf(stderr,"ERRORE: nanosleep\n");
                            //head_insert(&Queue,fullpath);
                            head_insert_mod(fullpath);
                            SIGNAL(&(Queue->list_cond));
                        }
                        UNLOCK(&(Queue->list_mutex));
                    }
                    memset(fullpath,'\0',PATHLEN);
                }
                strncpy(path, basePath,PATHLEN);
                strncat(path, slash,1);
                strncat(path, dp->d_name,namelen);
                list_files_recursively(path);
            }
        }
    }
    if(closedir(dir) == -1)
        fprintf(stderr,"ERRORE: closedir\n");
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
    errno=0;
    DIR *dir=NULL;
    dir = opendir(n);
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
