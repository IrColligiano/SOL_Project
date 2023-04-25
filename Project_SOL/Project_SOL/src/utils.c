#include <utils.h>
#include <unistd.h>


void* _malloc_(size_t size) {
    void* buffer=malloc(size);
    if(buffer==NULL){
        fprintf(stderr,"ERRORE: Malloc\n");
        exit(EXIT_FAILURE);
    }
    return buffer;
}

int read_n(int fd, void *buf, size_t size) {
    size_t left = size;
    int r = -2;
    char *bufptr = buf;
    while(left > 0) {
        if((r = read( (int)fd, bufptr, left)) == -1) {
            if (errno == EINTR) 
                continue;
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
	    	fprintf(stderr,"ERRORE: write_n\n");
		    return -1;
		}
		if (w == 0) 
            return 0;  
	    left    -= w;
		buf     += w;
	}
    return 1;
}

ssize_t writen1(int fd, void *ptr, size_t n) {  
    size_t   nleft = n;
    ssize_t  nwritten;
    while (nleft > 0) {
        if((nwritten = write(fd, ptr, nleft)) < 0) {
            if (nleft == n) 
                return -1; /* error, return -1 */
            else 
                break; /* error, return amount written so far */
        } 
        else{ 
            if (nwritten == 0) 
                break;
        }
        nleft -= nwritten;
        ptr   += nwritten;
   }
   return(n - nleft); /* return >= 0 */
}