#include <utils.h>
#include <unistd.h>


void* _malloc_(size_t size) {
    void* buffer = malloc(size);
    if(buffer == NULL){
        perror("ERRORE: malloc\n");
        exit(EXIT_FAILURE);
    }
    else
        return buffer;
}

int read_n(int fd, void *buf, size_t size) {
    size_t left = size;
    int r = -2;
    char *bufptr = buf;
    while(left > 0) {
        if((r = read(fd, bufptr, left)) == -1) {
            if (errno == EINTR) 
                continue;
            perror("ERRORE: read_n\n");
            return -1;
        }
        if (r == 0) 
            return 0;   // EOF
        left    -= r;
        bufptr  += r;
    }
    return (int)size;
}

int write_n(int fd, void *buf, size_t size) {
    size_t left =  size;
    ssize_t w = 0;
    while(left > 0) {
		if ((w = write(fd ,buf, left)) < 0) {
	    	perror("ERRORE: write_n\n");
		    return -1;
		}
		if (w == 0) 
            return 0;  
	    left    -= w;
		buf     += w;
	}
    return 1;
}