#define _GNU_SOURCE
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <thread.h>

pthread_attr_t tattr;// attributi del signal handler
volatile sig_atomic_t count_sigusr1; // variabile che indica quante print request sono state gestite
volatile sig_atomic_t _sigusr1;// variabile che indica quante print request sono arrivate da SIGUSR1
volatile sig_atomic_t is_connected = TRUE;
volatile sig_atomic_t term_for_sig = TRUE;
int exit_ = TRUE; // variabile che indica lo stato di terminazione dei thread
pthread_t sig_handler_thread; 
sigset_t mask; // masckera per il signal handler
int fd_skt; // sck
struct sockaddr_un sck_addr; 
pool * th_pool = NULL;

long result_from_file(char* path) {
    if(path == NULL) 
        return -1;
    errno = 0;
    FILE* fp = NULL;
    if((fp = fopen(path, "rb")) == NULL) {
        HANDLE_ERROR("fopen()");
        return -1;
    }
    size_t n_byte =          0;
    size_t n_byte_for_time = 1;
    long res =               0;
    long i =                 0;
    long x =                 0;
    while((n_byte = fread(&x , sizeof(long), n_byte_for_time , fp)) == 1) {
        res = res +(x * i);
        i ++;
    }
    if(feof(fp)){
        if(fclose(fp) != 0)
            HANDLE_ERROR("fclose()");
        return res;
    }
    else{
        if(ferror(fp) != 0){
            errno = EFAULT;
            HANDLE_ERROR("fread()");
        }
        if(fclose(fp) != 0)
            HANDLE_ERROR("fclose()");
        return -1;
    }
}

int  thread_create(long number_th){
    th_pool =               (pool*)_malloc_(sizeof(pool));
    th_pool->n_th =         (size_t)number_th;
    th_pool->arr_th =       (pthread_t*)_malloc_(sizeof(pthread_t)*(th_pool->n_th));
    th_pool->n_th_on_work = (size_t)number_th;
    errno=0;
    if( pthread_mutex_init(&(th_pool->join_mutex),NULL) != 0){
        errno = EINVAL;
        HANDLE_ERROR("pthread_mutex_init()");
        return EXIT_FAILURE;
    }
    if( pthread_mutex_init(&(th_pool->sck_mutex),NULL) != 0){
        errno = EINVAL;
        HANDLE_ERROR("pthread_mutex_init()");
        return EXIT_FAILURE;
    }
    for(int i = 0; i < th_pool->n_th; i++){
        int err;
        if((err = pthread_create(&(th_pool->arr_th[i]) , NULL , &thread_work , NULL) != 0) ){
            HANDLE_ERROR("pthread_create()");
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
    int s;
    errno = 0;
    if ((s = write_n(fd_skt, &res, sizeof( long ))) < 0) {
        HANDLE_ERROR("write_n() result");
        return -1;
    }
    if (s == 0) {
        HANDLE_ERROR("write_n() result");
        return 0;
    }
    if ((s = write_n(fd_skt, &len, sizeof(size_t))) < 0) {
        HANDLE_ERROR("write_n() len");
        return -1;
    }
    if (s == 0){
        HANDLE_ERROR("write_n() len");
        return 0;
    }
    if ((s = write_n(fd_skt, path ,len)) < 0) {
        HANDLE_ERROR("write_n() pathname");
        return -1;
    }
    if (s == 0){
        HANDLE_ERROR("write_n() pathname"); 
        return 0;
    }
    return 1;
}

int write_on_socket_sigusr1() {
    int s;
    long n = -SIGUSR1;
    errno = 0;
    if ((s = write_n(fd_skt, &n, sizeof(long))) < 0){
        HANDLE_ERROR("write_on_socket_sigusr1()");
        return -1;
    }
    if (s == 0) {
        HANDLE_ERROR("write_on_socket_sigusr1()");
        return 0;
    }
    return 1;
}

int write_on_socket_finish(){
    int s;
    long n = -SIGTERM;
    errno = 0;
    LOCK(&(th_pool->sck_mutex));
    if ((s = write_n(fd_skt, &n, sizeof(long))) < 0){
        HANDLE_ERROR("write_on_socket_finish()");
        UNLOCK(&(th_pool->sck_mutex));
        return -1;
    }
    if (s == 0) {
        HANDLE_ERROR("write_on_socket_finish()");
        UNLOCK(&(th_pool->sck_mutex));
        return 0;
    }
    UNLOCK(&(th_pool->sck_mutex));
    return 1;
}


void * thread_work(void * arg){
    int exit_while = TRUE;
    while(exit_while){
        errno = 0;
        LOCK(&(Queue->list_mutex));
        if( term_for_sig == FALSE && Queue->len == 0 ){
                exit_ =      FALSE;
                BCAST(&(Queue->list_cond));
                exit_while = FALSE;
        }

        else{
            if( Queue->cond_term == FALSE && Queue->len == 0 ){
                exit_ =      FALSE;
                BCAST(&(Queue->list_cond));
                exit_while = FALSE;
            }
        }

        while( Queue->len == 0 && exit_ && exit_while && term_for_sig )
            WAIT(&(Queue->list_cond),&(Queue->list_mutex));

        if(exit_ && exit_while){
            int s =       0;
            long result = 0;
            size_t len =  Queue->tail->lenght;
            char *buffer= _malloc_(sizeof(char) * (len+1));
            memset(buffer,'\0',len+1);
            delete_last(&buffer);
            if(term_for_sig)
                SIGNAL(&(Queue->list_full_cond));
            UNLOCK(&(Queue->list_mutex));
            if((result = result_from_file(buffer)) == -1){
                HANDLE_ERROR("result_from_file()");
                free(buffer);
            }
            else{
                LOCK(&(th_pool->sck_mutex));
                while(_sigusr1 > count_sigusr1){
                    count_sigusr1 = count_sigusr1 + 1;
                    if(( s = write_on_socket_sigusr1()) <= 0)
                        fprintf(stderr,"write_on_scocket_sigusr1 number %d\n",count_sigusr1);
                }
                if(( s = write_on_socket(buffer,result,len+1)) <= 0 ){
                    fprintf(stderr,"write_on_socket %s %ld %ld\n",buffer,result,(len+1));
                }
                UNLOCK(&(th_pool->sck_mutex));
                free(buffer);
                buffer=NULL;
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
        int s;
        if(( s = write_on_socket_finish()) <= 0 )
            HANDLE_ERROR("write_on_socket_finish()");
    }
    UNLOCK(&(th_pool->join_mutex));
    return (void*)NULL;
}



int JOIN(pthread_t th ){
    errno = 0;
    if(pthread_join(th,NULL) != 0){
        if(errno == EDEADLK || errno == EINVAL || errno == ESRCH){
            HANDLE_ERROR("join()");
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
 /////////////////////////////////thread for signalhandler//////////////////////////////


void* signal_handler_thread_work(void* arg) {
    count_sigusr1 = 0;
    _sigusr1 =      0;
    while(TRUE) {
        int signum;
        int err = sigwait(&mask , &signum);
        if(err != 0){
            errno = EINVAL;
            HANDLE_ERROR("sigwait()");
            return(void*)NULL;
        }
	    switch(signum){
	        case SIGHUP: 
	        case SIGINT:
	        case SIGQUIT:
	        case SIGTERM:
	            term_for_sig = FALSE;
	            break;
            case SIGUSR1:
                _sigusr1 = _sigusr1 + 1;
                break;
            case SIGUSR2:
                return (void*)NULL;
	        default:; 
	    }
    }
    return (void*)NULL;
}

int signal_handler_master() {
    struct sigaction signal_handler;
    memset(&signal_handler, 0, sizeof(signal_handler));
    signal_handler.sa_handler = SIG_IGN; 
    if(sigaction(SIGPIPE, &signal_handler , NULL) == -1) {
        HANDLE_ERROR("sigaction() SIGPIPE");
        return EXIT_FAILURE;
    }
    if(sigemptyset(&mask) != 0){
        HANDLE_ERROR("sigemptyset()");
        errno = EINVAL;
        return EXIT_FAILURE;
    }
    if(sigaddset(&mask, SIGHUP) != 0){
        HANDLE_ERROR("sigaddset() SIGHUP");
        errno = EINVAL;
        return EXIT_FAILURE;
    }
    if(sigaddset(&mask, SIGINT) != 0){
        HANDLE_ERROR("sigaddset() SIGINT");
        errno = EINVAL;
        return EXIT_FAILURE;
    }   
    if(sigaddset(&mask, SIGQUIT) != 0){
        HANDLE_ERROR("sigaddset() SIGQUIT");
        errno = EINVAL;
        return EXIT_FAILURE;
    }  
    if(sigaddset(&mask, SIGTERM) != 0){
        HANDLE_ERROR("sigaddset() SIGTERM");
        errno = EINVAL;
        return EXIT_FAILURE;
    }  
    if(sigaddset(&mask, SIGUSR1) != 0){
        HANDLE_ERROR("sigaddset() SIGUSR1");
        errno = EINVAL;
        return EXIT_FAILURE;
    }
    if(sigaddset(&mask, SIGUSR2) != 0){
        HANDLE_ERROR("sigaddset() SIGUSR2");
        errno = EINVAL;
        return EXIT_FAILURE;
    }  
    if(pthread_sigmask(SIG_BLOCK, &mask, NULL) != 0) {
        HANDLE_ERROR("pthread_sigmask()");
        errno = EINVAL;
        return EXIT_FAILURE;
    }
    if (pthread_attr_init(&tattr)!= 0) {
        HANDLE_ERROR("pthread_attr_init()");
        errno = ENOMEM;
        return EXIT_FAILURE;
    }
    if (pthread_attr_setdetachstate(&tattr , PTHREAD_CREATE_DETACHED) != 0) {
        HANDLE_ERROR("pthread_attr_setdetachstate()");
        errno = EINVAL;
        return EXIT_FAILURE;
    }
    if (pthread_create(&sig_handler_thread, &tattr, signal_handler_thread_work,(void*) NULL ) != 0) {
        HANDLE_ERROR("pthread_create() signal handler thread");
        errno = EFAULT;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

