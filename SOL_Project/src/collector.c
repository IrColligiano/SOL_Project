
#include <collector.h>
#include <unistd.h>
#include <signal.h>


int fd_skt; //sck
int fd_c;  //sck
abr * root = NULL; // puntatore albero abr

int read_from_socket() {
	errno =       0;
	int l =       0;
	size_t len =  0;
    long res =    0;
    char * path = NULL;
    if((l = read_n(fd_c, &res, sizeof(long))) != sizeof(long)) {
		if(l == 0){
            perror("ERRORE: read_n result error\n");
			return 0;
        }
		perror("ERRORE: read_n result error\n");
		return -1;
	}
    if(res == -SIGUSR1){ // caso in cui arriva sigurs1 e devo stampare
        print_tree(root);
        return 1;
    }
    if(res == -SIGTERM){// caso in cui devo terminare odinariamnete oppure e' arrivato un segnale di terminazione
        return 2;
    }
	if( (l = read_n(fd_c, &len, sizeof(size_t))) != sizeof(size_t) ) {
		if(l == 0){
            perror("ERRORE: read_n lenght error");
			return 0;
        }
		perror("ERRORE: read_n lenght error");
		return -1;
	}
	path = _malloc_(sizeof(char)*(len));
	memset(path, '\0', len);
	if ((l = read_n (fd_c, path, sizeof(char)*(len))) != sizeof(char)*(len)) {
		if(l == 0){
            perror("ERRORE: read_n pathname error");
            free(path);
			return 0;
        }
		perror("ERRORE: read_n pathname error");
		free(path);
		return -1;
	}
    root = create_tree(root,res,path,len);
    free(path);
	return 1;
}

int collector_main(){
    struct sigaction signal_handler;
	memset(&signal_handler, 0, sizeof(signal_handler));
	signal_handler.sa_handler = SIG_IGN;

//______________ignoro tutti i segnali da gestire__________________//
	if(sigaction(SIGHUP,&signal_handler, NULL) == -1) {
	    perror("ERRORE: sigaction SIGHUP error");
	    return EXIT_FAILURE;
	}
	if(sigaction(SIGINT,&signal_handler, NULL) == -1) {
	    perror("ERRORE: sigaction SIGINT error");
	    return EXIT_FAILURE;
	}
	if(sigaction(SIGQUIT,&signal_handler, NULL) == -1) {
	    perror("ERRORE: sigaction SIGQUIT error");
	    return EXIT_FAILURE;
	}
	if(sigaction(SIGTERM,&signal_handler, NULL) == -1) {
	    perror("ERRORE: sigaction SIGTERM error");
	    return EXIT_FAILURE;
	}
	if(sigaction(SIGPIPE,&signal_handler, NULL) == -1) {
	    perror("ERRORE: sigaction SIGPIPE error");
	    return EXIT_FAILURE;
	}
	if(sigaction(SIGUSR1,&signal_handler, NULL) == -1) {
	    perror("ERRORE: sigaction SIGUSR1 error");
	    return EXIT_FAILURE;
	}
    if(sigaction(SIGUSR2,&signal_handler, NULL) == -1) {
	    perror("ERRORE: sigaction SIGUSR2 error");
	    return EXIT_FAILURE;
	}

    struct sockaddr_un sck_addr;
    memset(&sck_addr, 0, sizeof(sck_addr));
    sck_addr.sun_family = AF_UNIX;
    strncpy(sck_addr.sun_path, SCKNAME ,SCKLEN);

//__________________socket_______________________//
    if((fd_skt = socket(AF_UNIX,SOCK_STREAM,0)) == -1){
        perror("ERRORE: socket collector error");
        return EXIT_FAILURE;
    }
    if((bind(fd_skt, (struct sockaddr *) &sck_addr, sizeof(sck_addr))) == -1){
        perror("ERRORE: bind collector error");
        return EXIT_FAILURE;
    }
    if((listen(fd_skt , SOMAXCONN)) == -1){
        perror("ERRORE: listen collector error");
        return EXIT_FAILURE;
    }
    if((fd_c = accept(fd_skt, NULL,0)) == -1){
        perror("ERRORE: accept collector error");
        return EXIT_FAILURE;
    }
//___________________leggo dalla socket________________//
    int ok = TRUE;
    int err;
    while (ok){
        err = read_from_socket();
        if(err == -1 || err == 0){
            perror("ERRORE: read_from_socket error");
            ok = FALSE;
        }
        if(err == 2)
            ok = FALSE;
    }
//__________________stampo e libero l' albero____________//
    if(root != NULL && err == 2){
        print_tree(root);
        free_all_tree(root);
        root = NULL;
    }
//_________________chiudo la connessione__________________//
    if(close(fd_skt) != 0){
        perror("ERRORE: close error");
        return EXIT_FAILURE;
    }
    if(close(fd_c) != 0){
        perror("ERRORE: close error");
        return EXIT_FAILURE;
    }
    if(unlink(SCKNAME) != 0){
        perror("ERRORE: unlink error");
        return EXIT_FAILURE;
    }
    if(err != 2)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}