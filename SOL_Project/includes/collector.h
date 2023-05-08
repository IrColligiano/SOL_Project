#ifndef __COLLECTOR_H__
#define __COLLECTOR_H__

#include <tree.h>

// legge dalla socket connessa con il master 
int read_from_socket();
// funzione che rappresenta il main del collector
int collector_main();

#endif