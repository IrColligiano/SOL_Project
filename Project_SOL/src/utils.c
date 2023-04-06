#include <utils.h>

void* _malloc_(size_t size) {
    void* buffer=malloc(size);
    if(buffer==NULL){
        fprintf(stderr,"ERRORE: Malloc\n");
        exit(EXIT_FAILURE);
    }
    return buffer;
}