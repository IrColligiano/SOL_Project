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
#include <sys/wait.h>

#include <thread.h>
#include <collector.h>

// libera le risorse utilizzate dal master.
int free_resource();
// calcola quanti millisecondi + il resto in nanosecondi e usa nanosleep.
int my_nanosleep(long msec);
// funzione per il controllo sull inpunt di interi long.
long arg_int(const char* n);
// verifica che un numero sia intero.
int isNumber(const char* s, long* tmp);
// visita la directory di partenza e naviga le sottodirectory inserendo file regolari 
// nella coda concorrente ogni qual volta ne trova uno.
void list_files_recursively(char *basePath);
// funzione per il controllo dell input -d 
char * arg_char_dir(char * n);
// verifica che il pathname sia un file regolare
int is_regular_file(const char *path);
// verifica che il pathname sia una directory
int is_directory(const char *path);

int fd_skt;                                      // fd della socket
struct sockaddr_un sck_addr;                    
long Nthread =                       NTHREADDEF; // variabile del numero di thread
long Qlen =                          QLENDEF;    //variabile per la lunghezza della coda
long Tdelay =                        TDELAYDEF;  //variabile per i ms
char *DirectoryName =                NULL;       //puntatore ca char che conterra il nome della directory

//____________________MAIN____________________//

int main(int argc,char *argv[]){
    // controllo che ci sia almeno un argomento in argc 
    if(argc < 2){
        fprintf(stderr,"ERRORE: pochi argomenti in input\n");
        return EXIT_FAILURE;
    }
//_________________fork___________________//
    pid_t pid;//pid 
    if((pid = fork()) == -1) {
    	perror("ERRORE: fork error");
        return EXIT_FAILURE;
    }
//____________________collector____________________//

    if(pid == 0){ // figlio che rappresenta il collector ed il server
        if(collector_main() != EXIT_SUCCESS){
            perror("ERRORE: return collector error");
            return EXIT_FAILURE;
        }
    }
//____________________master worker____________________//

    if(pid != 0){ // processo master che rappresenta il main ed il client
        if(signal_handler_master() !=0 ){
            perror("ERRORE: signal_handler_master error");
            return EXIT_FAILURE;
        }
        int opt; // variabile per la gestione dello switch da parte di getopt
        long tmp; // variabile temporanea
        int opt_n=0;
        int opt_q=0;
        int opt_t=0;
        int opt_d=0;
        char*a=NULL; //puntatore char di appoggio per getopt ed altre funzioni del main
        while((opt = getopt(argc, argv, "n:q:d:t:")) != -1){
            switch (opt){

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
                    //fprintf(stdout,"Numero di threads impostato a: %ld\n",Nthread);
                }     
                break;

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
                    //fprintf(stdout,"Lunghezza coda concorrente impostato a: %ld\n",Qlen);                    
                }     
                break;

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
                    //fprintf(stdout,"Directory %s\n",DirectoryName);
                }
                break;

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
                    //fprintf(stdout,"Delay impostato a: %ldms\n",Tdelay);                
                }   
                break;
            default:;
            }
        }
        memset(&sck_addr, 0, sizeof(sck_addr));
        sck_addr.sun_family = AF_UNIX;
        strncpy(sck_addr.sun_path, SCKNAME ,SCKLEN);
        if((fd_skt = socket(AF_UNIX, SOCK_STREAM, 0)) ==-1){
            perror("ERRORE: socket master error");
            return EXIT_FAILURE;
        }
        int err;
        my_nanosleep(1);
        while ( (err = connect( fd_skt, (struct sockaddr*)&sck_addr , sizeof(sck_addr) ) ) == -1 ){
            if ( errno == ENOENT ) 
                my_nanosleep(1000);
            else{
                perror("ERRORE: connect master error");
                return EXIT_FAILURE;
            }
        }
        if(init_list(Qlen,Tdelay) != 0){
            fprintf(stderr,"ERRORE: init_list\n");
            free(DirectoryName);
            return EXIT_FAILURE;
        }
        
        if(thread_create(Nthread) != 0){
            free(DirectoryName);
            fprintf(stderr,"ERRORE: thread_create\n");
            return EXIT_FAILURE;
        }
//__________________inizio inserimento file in coda________________________//
        int j = argc-1;
        while(j > 0 && term_for_sig){
            if(is_regular_file(argv[j]) !=0){
                LOCK(&(Queue->list_mutex));
                while(Queue->len >= Queue->max_len && term_for_sig)
                    WAIT(&(Queue->list_full_cond),&(Queue->list_mutex));
                size_t len = strlen(argv[j]);
                if(term_for_sig && len < PATHLEN-1){
                    my_nanosleep(Queue->msec);
                    head_insertion(argv[j],len);
                    SIGNAL(&(Queue->list_cond));
                }
                UNLOCK(&(Queue->list_mutex));
            }
            j--;
        }
//______________________inizio inserimento dir in coda________________//
        if(DirectoryName != NULL){
            if(term_for_sig)
                list_files_recursively(DirectoryName);
            free(DirectoryName);
            DirectoryName=NULL;
        }
//______________________aspetto thread che terminino__________________//
        Queue->cond_term=FALSE;
        for(j = 0 ; j < th_pool->n_th ; j++){
            if( JOIN((th_pool->arr_th[j])) == EXIT_FAILURE)
                return EXIT_FAILURE;
        }
//___________termino sig_handler chiudo socket e libero risorse____________//
        free_resource();
        if(pthread_kill(sig_handler_thread,SIGUSR2) != 0){
            perror("ERRORE: pthread_kill error");
            return EXIT_FAILURE;
        }
        if(close(fd_skt) != 0){
            perror("ERRORE: close error");
            return EXIT_FAILURE;
        }
        if(waitpid(0, NULL, 0) == -1){
            perror("ERRORE: waitpid error");
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

int my_nanosleep(long msec){
    struct timespec ts;
    int res;
    errno = 0;
    if (msec < 0){
        errno = EINVAL;
        return -1;
    }
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    res = nanosleep(&ts, &ts);
    return res;
}

int is_regular_file(const char *path){
    struct stat pathstat;
    if(stat(path, &pathstat) != 0)
        return 0;
    return S_ISREG(pathstat.st_mode);
}

int is_directory(const char *path){
   struct stat pathstat;
    if(stat(path, &pathstat) != 0)
        return 0;
   return S_ISDIR(pathstat.st_mode);
}

void list_files_recursively(char *basePath){
    char path[PATHLEN];
    char fullpath[PATHLEN];
    memset(path,'\0',PATHLEN);
    memset(fullpath,'\0',PATHLEN);
    size_t pathlen=0;
    size_t basepathlen=strnlen(basePath,PATHLEN-1);
    size_t namelen=0;
    struct dirent *dp = NULL;
    char slash[2]={'/','\0'};
    DIR *dir = opendir(basePath);
    if(dir == NULL){
        if(errno != ENOTDIR && errno != ENOENT)
            perror("ERRORE: opendir error");
        return;
    }
    while ((dp = readdir(dir)) != NULL && term_for_sig){
        if(errno == EBADF){
            perror("ERRORE: readdir error");
            closedir(dir);
            return;
        }
        namelen = strnlen(dp->d_name,PATHLEN-1);
        if (strcmp(dp->d_name,".") != 0 && strcmp(dp->d_name, "..") != 0){
            pathlen = strnlen(path,PATHLEN-1);
            if ( !(pathlen + basepathlen + namelen >= PATHLEN ) ){
                if( dp->d_type == DT_REG && !(basepathlen + namelen  >= PATHLEN) ){
                    strncpy(fullpath,basePath,basepathlen);
                    strncat(fullpath,slash,1);
                    strncat(fullpath,dp->d_name,namelen);
                    if(is_regular_file(fullpath) !=0 && term_for_sig){
                        LOCK(&(Queue->list_mutex));
                        while(Queue->len >= Queue->max_len && term_for_sig){
                             WAIT(&(Queue->list_full_cond),&(Queue->list_mutex));
                        }
                        if(term_for_sig){
                            if( my_nanosleep(Queue->msec) == -1)
                                perror("ERRORE: nanosleep error");
                            size_t len = strnlen(fullpath,PATHLEN-1);
                            head_insertion(fullpath, len);
                            SIGNAL(&(Queue->list_cond));
                        }
                        UNLOCK(&(Queue->list_mutex));
                    }
                    memset(fullpath,'\0',PATHLEN);
                }
                strncpy(path, basePath, pathlen);
                strncat(path, slash,1);
                strncat(path, dp->d_name,namelen);
                list_files_recursively(path);
            }
        }
    }
    if(closedir(dir) == -1)
        perror("ERRORE: closedir error");
    return;
}

int isNumber(const char* s, long* tmp){
    if (s == NULL) 
        return 1;
    if (strnlen(s,PATHLEN-1) == 0) 
        return 1; 
    char* e =  NULL;
    errno =    0;
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
    long tmp = 0; //variabile temporanea che mi raccoglie il long gestito da isNumber
    if (isNumber(n , &tmp) != 0 && tmp < 0)
        return -1;
    else
        return tmp; 
}

char* arg_char_dir(char * n){
    errno =    0;
    DIR *dir = NULL;
    dir =      opendir(n);
    if (dir){ // la directory esiste
        closedir(dir);
        size_t len = strlen(n);
        if(len > PATHLEN -1)
            return NULL;
        DirectoryName = _malloc_(sizeof(char) * (len+1));
        memset(DirectoryName,'\0',len+1);
        strncpy(DirectoryName,n,len);
        return n;
    }
    if (ENOENT == errno){ //la directory non esiste
        return NULL;
    }
    else{ // opendir() ha fallito per altri motivi
        return NULL;
    }
}

int free_resource(){
    DMUTEX(&(th_pool->sck_mutex));
    DMUTEX(&(th_pool->join_mutex));
    DMUTEX(&(Queue->list_mutex));
    DCOND(&(Queue->list_cond));
    DCOND(&(Queue->list_full_cond));
    free(th_pool->arr_th);
    th_pool->arr_th = NULL;
    free(th_pool);
    th_pool = NULL;
    free_list();
    free(Queue);
    Queue = NULL;
    return EXIT_SUCCESS;
}
