
#include <collector.h>
#include <unistd.h>
#include <signal.h>


int fd_skt;
int fd_c;
abr * root = NULL;

int read_from_socket() {
	errno =       0;
	int l =       0;
	size_t len =  0;
    long res =    0;
    char * path = NULL;
    //printf("voglio leggere\n");
    if((l = read_n(fd_c, &res, sizeof(long))) != sizeof(long)) {
		if(l == 0) 
			return 0;
		perror("ERRORE: read_n result\n");
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
	if((l = read_n(fd_c, &len, sizeof(size_t))) != sizeof(size_t)) {
		if(l == 0)
			return 0;
		perror("ERRORE: read_n lenght\n");
		return -1;
	}
    //printf("ho letto la lunghezza\n");
	path = _malloc_(sizeof(char)*(len+1));
	memset(path, '\0', len+1);
    //printf("read buffer\n");
	if ((l = read_n (fd_c, path, sizeof(char)*(len))) != sizeof(char)*(len)) {
		if(l == 0){
            free(path);
			return 0;
        }
        //fprintf(stderr,"%s\n",path);
		perror("ERRORE: read_n pathname\n");
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
	    perror("ERRORE: sigaction SIGHUP\n");
	    return EXIT_FAILURE;
	}
	if(sigaction(SIGINT,&signal_handler, NULL) == -1) {
	    perror("ERRORE: sigaction SIGINT\n");
	    return EXIT_FAILURE;
	}
	if(sigaction(SIGQUIT,&signal_handler, NULL) == -1) {
	    perror("ERRORE: sigaction SIGQUIT\n");
	    return EXIT_FAILURE;
	}
	if(sigaction(SIGTERM,&signal_handler, NULL) == -1) {
	    perror("ERRORE: sigaction SIGTERM\n");
	    return EXIT_FAILURE;
	}
	if(sigaction(SIGPIPE,&signal_handler, NULL) == -1) {
	    perror("ERRORE: sigaction SIGPIPE\n");
	    return EXIT_FAILURE;
	}
	if(sigaction(SIGUSR1,&signal_handler, NULL) == -1) {
	    perror("ERRORE: sigaction SIGUSR1\n");
	    return EXIT_FAILURE;
	}

    struct sockaddr_un sck_addr;
    memset(&sck_addr, 0, sizeof(sck_addr));
    sck_addr.sun_family = AF_UNIX;
    strncpy(sck_addr.sun_path, SCKNAME ,SCKLEN-1);

//__________________socket_______________________//
    if((fd_skt = socket(AF_UNIX,SOCK_STREAM,0)) == -1){
        perror("ERRORE: sck collector\n");
        return EXIT_FAILURE;
    }
    //printf("COLLECTOR: socket\n");
    if((bind(fd_skt, (struct sockaddr *) &sck_addr, sizeof(sck_addr))) == -1){
        close(fd_skt);
        perror("ERRORE: sck bind collector\n");
        return EXIT_FAILURE;
    }
    //printf("COLLECTOR: bind\n");
    if((listen(fd_skt , SOMAXCONN)) == -1){
        close(fd_skt);
        perror("ERRORE: sck listen collector\n");
        return EXIT_FAILURE;
    }
    //printf("COLLECTOR: listen\n");
    if((fd_c = accept(fd_skt, NULL,0)) == -1){
        close(fd_skt);
        perror("ERRORE: sck accept collector\n");
        return EXIT_FAILURE;
    }
    //printf("COLLECTOR: accept\n");
//___________________leggo dalla socket________________//
    int ok = TRUE;
    int err;
    while (ok){
        err = read_from_socket();
        if(err == 2)
            ok = FALSE;
    }
//__________________stampo e libero l' albero____________//
    if(root != NULL){
        print_tree(root);
        free_all_tree(root);
        root = NULL;
    }
//_________________chiudo la connessione__________________//
    close(fd_skt);
    close(fd_c);
    unlink(SCKNAME);
    //printf("COLLECTOR: close\n");
    return EXIT_SUCCESS;
}