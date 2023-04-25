#define UNIX_MAX_PATH 108
#include <collector.h>
#include <unistd.h>
#include <signal.h>


int fd_skt;
int fd_c;
abr * root=NULL;

int receive_from_master() {
	errno =       0;
	int r =       0;
	size_t len =  0;
    long res =    0;
    char * path = NULL;
    //printf("sono io\n");
    //printf("voglio leggere\n");
    if((r = read_n(fd_c, &res, sizeof(long))) != sizeof(long)) {
		if(r == 0) 
			return 0;
		fprintf(stderr,"ERRORE: read_n result\n");
		return -1;
	}
    //printf("ho letto il result\n");
    if(res == -2){ // caso in cui arriva sigurs1 e devo stampare
        print_tree(root);
        return 1;
    }
    if(res == -1) // caso in cui sono finiti gli input e devo terminare
        return 2;
    //printf("read len\n");
	if((r = read_n(fd_c, &len, sizeof(size_t))) != sizeof(size_t)) {
		if(r == 0)
			return 0;
		fprintf(stderr,"ERRORE: read_n lenght\n");
		return -1;
	}
    //printf("ho letto la lunghezza\n");
	path = _malloc_(sizeof(char)*(len+1));
	memset(path, '\0', len);
    //printf("read buffer\n");
	if ((r = read_n (fd_c, path, sizeof(char)*(len))) != sizeof(char)*(len)) {
		if(r == 0){
            free(path);
			return 0;
        }
        fprintf(stdout,"%s\n",path);
		fprintf(stderr,"ERRORE: read_n pathname\n");
		free(path);
		return -1;
	}
    //printf("ho letto tutto\n");
    root = create_tree(root,res,path,len);
    free(path);
    path=NULL;
	return 1;
}

int collector_main(){
    /*
    sigset_t mask;
    sigemptyset(&mask);
    //Blocco i segnali che gestisce processo master
    if (sigaddset(&mask, SIGPIPE)!= 0) {
        fprintf(stderr,"ERRORE: sigaddset SIGPIPE\n");
        return -1;
    }
    if (sigaddset(&mask, SIGINT)!= 0) {
        fprintf(stderr,"ERRORE: sigaddset SIGINT\n");
        return -1;
    }
    
    if (sigaddset(&mask, SIGQUIT)!= 0) {
        fprintf(stderr,"ERRORE: sigaddset SIGQUIT\n");
        return -1;
    }

    if (sigaddset(&mask, SIGTERM)!= 0) {
        fprintf(stderr,"ERRORE: sigaddset SIGTERM\n");
        return -1;
    }

    if (sigaddset(&mask, SIGHUP)!= 0) {
        fprintf(stderr,"ERRORE: sigaddset SIGHUP\n");
        return -1;
    }

    if (sigaddset(&mask, SIGUSR1)!= 0) {
        fprintf(stderr,"ERRORE: sigaddset SIGUSR1\n");
        return -1;
    }

    if (sigaddset(&mask, SIGUSR2)!= 0) {
        fprintf(stderr,"ERRORE: sigaddset SIGUSR2\n");
        return -1;
    }

    if (pthread_sigmask(SIG_SETMASK, &mask,NULL) != 0) {
        fprintf(stderr,"ERRORE: pthread_sigmask\n");
        return -1;
    }
    */
    /*
    struct sigaction signal_handler;
    memset(&signal_handler, 0, sizeof(signal_handler));
    signal_handler.sa_handler = SIG_IGN; 
    if(sigaction(SIGPIPE, &signal_handler , NULL) == -1) {
        fprintf(stderr,"ERRORE: sigaction SIGPIPE\n");
        return EXIT_FAILURE;
    }
    if(sigaction(SIGINT, &signal_handler , NULL) == -1) {
        fprintf(stderr,"ERRORE: sigaction SIGINT\n");
        return EXIT_FAILURE;
    }
    if(sigaction(SIGQUIT, &signal_handler , NULL) == -1) {
        fprintf(stderr,"ERRORE: sigaction SIGQUIT\n");
        return EXIT_FAILURE;
    }
    if(sigaction(SIGTERM, &signal_handler , NULL) == -1) {
        fprintf(stderr,"ERRORE: sigaction SIGTERM\n");
        return EXIT_FAILURE;
    }
    */
    struct sigaction signal_handler;
	memset(&signal_handler, 0, sizeof(signal_handler));
	signal_handler.sa_handler = SIG_IGN;
	if(sigaction(SIGHUP,&signal_handler, NULL) == -1) {
	    fprintf(stderr,"ERRORE: sigaction SIGHUP");
	    return EXIT_FAILURE;
	}
	if(sigaction(SIGINT,&signal_handler, NULL) == -1) {
	    fprintf(stderr,"ERRORE: sigaction SIGINT");
	    return EXIT_FAILURE;
	}
	if(sigaction(SIGQUIT,&signal_handler, NULL) == -1) {
	    fprintf(stderr,"ERRORE: sigaction SIGQUIT");
	    return EXIT_FAILURE;
	}
	if(sigaction(SIGTERM,&signal_handler, NULL) == -1) {
	    fprintf(stderr,"ERRORE: sigaction SIGTERM");
	    return EXIT_FAILURE;
	}
	if(sigaction(SIGPIPE,&signal_handler, NULL) == -1) {
	    fprintf(stderr,"ERRORE: sigaction SIGPIPE");
	    return EXIT_FAILURE;
	}
	if(sigaction(SIGUSR1,&signal_handler, NULL) == -1) {
	    fprintf(stderr,"ERRORE: sigaction SIGUSR1\n");
	    return EXIT_FAILURE;
	}

    struct sockaddr_un sa;
    memset(&sa, 0, sizeof(sa));
    sa.sun_family=AF_UNIX;
    strncpy(sa.sun_path, SCKNAME ,SCKLEN-1);
    if((fd_skt = socket(AF_UNIX,SOCK_STREAM,0)) == -1){
        fprintf(stderr,"ERRORE: Sck Collector\n");
        return EXIT_FAILURE;
    }
    printf("COLLECTOR: socket\n");
    if((bind(fd_skt, (struct sockaddr *) &sa, sizeof(sa))) == -1){
        close(fd_skt);
        fprintf(stderr,"ERRORE: Sck bind Collector\n");
        return EXIT_FAILURE;
    }
    printf("COLLECTOR: bind\n");
    if((listen(fd_skt , SOMAXCONN)) == -1){
        close(fd_skt);
        fprintf(stderr,"ERRORE: Sck listen Collector\n");
        return EXIT_FAILURE;
    }
    printf("COLLECTOR: listen\n");
    if((fd_c = accept(fd_skt, NULL,0)) == -1){
        close(fd_skt);
        fprintf(stderr,"ERRORE: Sck accept Collector\n");
        return EXIT_FAILURE;
    }
    printf("COLLECTOR: accept\n");
    int ok =TRUE;
    int err;
    while (ok){
        err= receive_from_master();
        if(err == 2)
            ok=FALSE;
    }
    if(root != NULL){
        print_tree(root);
        free_all_tree(root);
        root=NULL;
    }
    close(fd_skt);
    close(fd_c);
    unlink(SCKNAME);
    printf("COLLECTOR: close\n");
    return EXIT_SUCCESS;
}