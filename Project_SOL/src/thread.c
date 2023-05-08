#define _GNU_SOURCE
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <thread.h>

pthread_attr_t tattr;// attributi del signal handler
volatile sig_atomic_t count_print_request; // variabile che indica quante print request sono state gestite
volatile sig_atomic_t print_request;// variabile che indica quante print request sono arrivate da SIGUSR1
volatile sig_atomic_t is_connected = TRUE;
volatile sig_atomic_t term_for_sig = TRUE;
int exit_ = TRUE; // variabile che indica lo stato di terminazione dei thread
pthread_t sig_handler_thread; 
sigset_t mask; // masckera per il signal handler
int fd_skt; // sck
struct sockaddr_un sck_addr; 
pool * th_pool = NULL;

long result_from_file(char* path) {
    if(path == NULL) {
        errno = EINVAL;
        return -1;
    }
    FILE* fp =               NULL;
    if((fp = fopen(path, "rb")) == NULL) {
        perror("ERRORE: fopen error");
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
        fclose(fp);
        return res;
    }
    else{
        fclose(fp);
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
        perror("ERRORE: Inizializzazione variabile di mutex error");
        errno = EINVAL;
        return EXIT_FAILURE;
    }
    if( pthread_mutex_init(&(th_pool->sck_mutex),NULL) != 0){
        perror("ERRORE: Inizializzazione variabile di mutex error");
        errno = EINVAL;
        return EXIT_FAILURE;
    }
    for(int i = 0; i < th_pool->n_th; i++){
        int err;
        if((err = pthread_create(&(th_pool->arr_th[i]) , NULL , &thread_work , NULL) != 0) ){
            perror("ERRORE: creazione thread error");
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
    int s;
    errno = 0;
    if ((s = write_n(fd_skt, &res, sizeof( long ))) < 0) {
        perror("ERRORE: write_n result error");
        return -1;
    }
    if (s == 0) {
        perror("ERRORE: write_n result error");
        return 0;
    }
    if ((s = write_n(fd_skt, &len, sizeof(size_t))) < 0) {
        perror("ERRORE: write_n len error");
        return -1;
    }
    if (s == 0){
        perror("ERRORE: write_n len error");
        return 0;
    }
    if ((s = write_n(fd_skt, path ,len)) < 0) {
        perror("ERRORE: write_n pathname error");
        return -1;
    }
    if (s == 0) {
        perror("ERRORE: write_n pathname error"); 
        return 0;
    }
    return 1;
}

int write_on_socket_print_request() {
    int s;
    long n = -2;
    errno = 0;
    if ((s = write_n(fd_skt, &n, sizeof(long))) < 0){
        perror("ERRORE: write_on_socket_print_request error");
        return -1;
    }
    if (s == 0) {
        perror("ERRORE: write_on_socket_print_request error");
        return 0;
    }
    return 1;
}

int write_on_socket_term_request(){
    int s;
    long n = -1;
    errno = 0;
    LOCK(&(th_pool->sck_mutex));
    if ((s = write_n(fd_skt, &n, sizeof(long))) < 0){
        perror("ERRORE: write_on_socket_term_request error");
        UNLOCK(&(th_pool->sck_mutex));
        return -1;
    }
    if (s == 0) {
        perror("ERRORE: write_on_socket_term_request error");
        UNLOCK(&(th_pool->sck_mutex));
        return 0;
    }
    UNLOCK(&(th_pool->sck_mutex));
    return 1;
}


void * thread_work(void * arg){
    int exit_while = TRUE;
    while(exit_while){

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
                perror("ERRORE: result_from_file error");
                free(buffer);
            }
            else{
                LOCK(&(th_pool->sck_mutex));
                while(print_request > count_print_request){
                    count_print_request = count_print_request + 1;
                    if(( s = write_on_socket_print_request()) <= 0)
                        fprintf(stderr,"write_on_scocket_print_request number %d\n",count_print_request);
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
        if(( s = write_on_socket_term_request()) <= 0 )
            perror("ERRORE: write_on_socket_term_request error\n");
    }
    UNLOCK(&(th_pool->join_mutex));
    return (void*)NULL;
}



int JOIN(pthread_t th ){
    errno = 0;
    if(pthread_join(th,NULL) != 0){
        if(errno==EDEADLK || errno == EINVAL || errno==ESRCH){
            perror("ERRORE: join error");
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
 /////////////////////////////////thread for signalhandler//////////////////////////////


void* signal_handler_thread_work(void* arg) {
    count_print_request = 0;
    print_request =       0;
    while(TRUE) {
        int signum;
        int err = sigwait(&mask , &signum);
        if(err != 0){
            errno = EINVAL;
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
                if(!is_connected)
                    print_request = print_request + 1;
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
        perror("ERRORE: sigaction SIGPIPE error");
        return EXIT_FAILURE;
    }
    if(sigemptyset(&mask) != 0){
        errno = EINVAL;
        perror("ERRORE: sigemptyset error");
        return EXIT_FAILURE;
    }
    if(sigaddset(&mask, SIGHUP) != 0){
        errno = EINVAL;
        perror("ERRORE: sigaddset SIGHUP error");
        return EXIT_FAILURE;
    }
    if(sigaddset(&mask, SIGINT) != 0){
        errno = EINVAL;
        perror("ERRORE: sigaddset SIGINT error");
        return EXIT_FAILURE;
    }   
    if(sigaddset(&mask, SIGQUIT) != 0){
        errno = EINVAL;
        perror("ERRORE: sigaddset SIGQUIT error");
        return EXIT_FAILURE;
    }  
    if(sigaddset(&mask, SIGTERM) != 0){
        errno = EINVAL;
        perror("ERRORE: sigaddset SIGTERM error");
        return EXIT_FAILURE;
    }  
    if(sigaddset(&mask, SIGUSR1) != 0){
        errno = EINVAL;
        perror("ERRORE: sigaddset SIGUSR1 error");
        return EXIT_FAILURE;
    }
    if(sigaddset(&mask, SIGUSR2) != 0){
        errno = EINVAL;
        perror("ERRORE: sigaddset SIGUSR2 error");
        return EXIT_FAILURE;
    }  
    if(pthread_sigmask(SIG_BLOCK, &mask, NULL) != 0) {
        errno = EINVAL;
        perror("ERRORE: pthread_sigmask error");
        return EXIT_FAILURE;
    }
    if (pthread_attr_init(&tattr)!= 0) {
        errno = ENOMEM;
        perror("ERRORE: pthread_attr_init error");
        return EXIT_FAILURE;
    }
    if (pthread_attr_setdetachstate(&tattr , PTHREAD_CREATE_DETACHED) != 0) {
        errno = EINVAL;
        perror("ERRORE: pthread_attr_setdetachstate error");
        return EXIT_FAILURE;
    }
    if (pthread_create(&sig_handler_thread, &tattr, signal_handler_thread_work,(void*) NULL ) != 0) {
        errno = EFAULT;
        perror("ERRORE: pthread_create signal handler thread error");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

