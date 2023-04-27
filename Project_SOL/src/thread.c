#define _GNU_SOURCE
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <thread.h>

volatile sig_atomic_t term_for_sig;
volatile sig_atomic_t print_for_sig;
volatile sig_atomic_t count_print_for_sig;
int exit_ = TRUE;
pthread_t sig_handler_thread;
sigset_t mask;
int fd_skt; // sck
struct sockaddr_un sa; 
t_pool * th_pool = NULL;

long result_from_path(char* path) {
    if(path == NULL) {
        errno = EINVAL;
        return -1;
    }
    FILE* fp =               NULL;
    if((fp = fopen(path, "rb")) == NULL) {
        fprintf(stderr,"ERRORE: fopen %s\n",path);
        return -1;
    }
    size_t n_byte =          0;
    size_t n_byte_for_time = 1;
    long res =               0;
    long i =                 0;
    long x =                 0;
    while((n_byte = fread(&x , sizeof(long), n_byte_for_time , fp)) == 1) {
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
    th_pool =               (t_pool*)_malloc_(sizeof(t_pool));
    th_pool->n_th =         (size_t)number_th;
    th_pool->arr_th =       (pthread_t*)_malloc_(sizeof(pthread_t)*(th_pool->n_th));
    th_pool->n_th_on_work = (size_t)number_th;
    errno=0;
    if( pthread_mutex_init(&(th_pool->join_mutex),NULL) != 0){
        perror("ERRORE: Inizializzazione variabile di mutex\n");
        errno = EINVAL;
        return EXIT_FAILURE;
    }
    if( pthread_mutex_init(&(th_pool->sck_mutex),NULL) != 0){
        perror("ERRORE: Inizializzazione variabile di mutex\n");
        errno = EINVAL;
        return EXIT_FAILURE;
    }
    if( pthread_cond_init(&(th_pool->join_cond),NULL) != 0){
        perror("ERRORE: Inizializzazione variabile di condizione\n");
        errno = EINVAL;
        return EXIT_FAILURE;
    }
    for(int i = 0; i < th_pool->n_th; i++){
        int err;
        if((err = pthread_create(&(th_pool->arr_th[i]) , NULL , &thread_work , NULL) != 0) ){
            perror("ERRORE: creazione thread\n");
            errno=EFAULT;
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
int write_on_socket(char* path, long res, size_t len) {
    if(path == NULL) {
        errno = EINVAL;
        return -1;
    }
    int w;
    errno = 0;
    if ((w = write_n(fd_skt, &res, sizeof( long ))) < 0) {
        perror("ERRORE: write_n result\n");
        return -1;
    }
    if (w == 0) {
        fprintf(stderr, "ERRORE: write_n result di %s\n", path);
        return 0;
    }
    if ((w = write_n(fd_skt, &len, sizeof(size_t))) < 0) {
        perror("ERRORE: write_n len");
        return -1;
    }
    if (w == 0){
        fprintf(stderr,"ERRORE: write_n len di %s\n", path);
        return 0;
    }
    if ((w = write_n(fd_skt, path ,len)) < 0) {
        perror("ERRORE: write_n pathname\n");
        return -1;
    }
    if (w == 0) {
        fprintf(stderr,"ERRORE: write_n pathname di %s\n", path); 
        return 0;
    }
    return 1;
}

int write_on_socket_print_request() {
    int w;
    long n = -2;
    errno = 0;
    LOCK(&(th_pool->sck_mutex));
    if ((w = write_n(fd_skt, &n, sizeof(long))) < 0){
        UNLOCK(&(th_pool->sck_mutex));
        return -1;
    }
    if (w == 0) {
        UNLOCK(&(th_pool->sck_mutex));
        return 0;
    }
    UNLOCK(&(th_pool->sck_mutex));
    return 1;
}

int write_on_socket_term_request(){
    int w;
    long n = -1;
    errno = 0;
    LOCK(&(th_pool->sck_mutex));
    if ((w = write_n(fd_skt, &n, sizeof(long))) < 0){
        UNLOCK(&(th_pool->sck_mutex));
        return -1;
    }
    if (w == 0) {
        UNLOCK(&(th_pool->sck_mutex));
        return 0;
    }
    UNLOCK(&(th_pool->sck_mutex));
    return 1;
}


void * thread_work(void * arg){
    int exit_while = TRUE;
    while(exit_while && term_for_sig){
        LOCK(&(Queue->list_mutex));
        if(!(Queue->cond_term) && Queue->len == 0 ){
            exit_ =      FALSE;
            BCAST(&(Queue->list_cond));
            exit_while = FALSE;
            }
        while(Queue->len==0 && exit_ && exit_while && term_for_sig){
            WAIT(&(Queue->list_cond),&(Queue->list_mutex));
        }
        if(exit_ && exit_while && term_for_sig){
            
            int w =       0;
            long result=  0;
            char *buffer= _malloc_(sizeof(char)*PATHLEN);
            memset(buffer,'\0',PATHLEN);
            delete_last_mod(&buffer);
            SIGNAL(&(Queue->list_full_cond));
            UNLOCK(&(Queue->list_mutex));
            if((result = result_from_path(buffer)) == -1){
                fprintf(stderr,"ERRORE: Lettura file\n");
                free(buffer);
            }
            else{
                size_t len=strnlen(buffer,PATHLEN-1);
                LOCK(&(th_pool->sck_mutex));
                if(( w = write_on_socket(buffer,result,len+1)) <= 0 ){
                    fprintf(stderr,"ERRORE: Volevo scrivere %s %ld %ld\n",buffer,result,(len+1));
                }
                free(buffer);
                buffer=NULL;
                UNLOCK(&(th_pool->sck_mutex));
            }
        }
        else{
            exit_while = FALSE;
            BCAST(&(Queue->list_cond));
            UNLOCK(&(Queue->list_mutex));
        }
    }
    LOCK(&(th_pool->join_mutex));
    th_pool->n_th_on_work --;
    if(th_pool->n_th_on_work <= 0){
        int w;
        if(( w = write_on_socket_term_request()) <= 0 ){
            perror("ERRORE: write on socket term request\n");
        }
        SIGNAL(&(th_pool->join_cond));
    }
    UNLOCK(&(th_pool->join_mutex));
    return (void*)NULL;
}



int JOIN(pthread_t th ){
    errno = 0;
    if(pthread_join(th,NULL) != 0){
        if(errno==EDEADLK || errno == EINVAL || errno==ESRCH){
            perror("ERRORE: join\n");
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
 /////////////////////////////////thread for signalhandler//////////////////////////////


void* signal_handler_thread_work(void* arg) {
    print_for_sig=0;
    count_print_for_sig=0;
    while(TRUE) {
        int signum;
        int err = sigwait(&mask , &signum);
        if(err != 0){
            return(void*)NULL;
        }
	    switch(signum) {
	        case SIGHUP: 
	        case SIGINT:
	        case SIGQUIT:
	        case SIGTERM:
	            term_for_sig = FALSE;
	            return (void*)NULL;
            case SIGUSR1:
                write_on_socket_print_request();
                //print_for_sig +=1;
                break;
	        default: ; 
	    }
    }
    return (void*)NULL;
}

void term_signal_handler() {
    pthread_kill(sig_handler_thread, SIGQUIT);
    JOIN(sig_handler_thread);
}

int signal_handler_master() {
    struct sigaction signal_handler;
    memset(&signal_handler, 0, sizeof(signal_handler));
    signal_handler.sa_handler = SIG_IGN; 
    if(sigaction(SIGPIPE, &signal_handler , NULL) == -1) {
        fprintf(stderr,"ERRORE: sigaction SIGPIPE\n");
        return EXIT_FAILURE;
    }
    sigemptyset(&mask);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGINT);   
    sigaddset(&mask, SIGQUIT);  
    sigaddset(&mask, SIGTERM);  
    sigaddset(&mask, SIGUSR1);  
    if(pthread_sigmask(SIG_BLOCK, &mask, NULL) != 0) {
        fprintf(stderr,"ERRORE: pthread_sigmask\n");
        return EXIT_FAILURE;
    }
    if (pthread_create( &sig_handler_thread , NULL, signal_handler_thread_work, (void*) &mask ) != 0) {
        errno = EFAULT;
        fprintf(stderr,"ERRORE: Creazione thread signal handler\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

