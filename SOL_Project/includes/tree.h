#ifndef __TREE_H__
#define __TREE_H__

#include <utils.h>

// dato un nuovo nodo lo inserisce nell abr
abr *create_tree(abr *tree, long res, char *path,size_t len);
// stampa l albero con il formato "nome file" "risultato"
void print_tree(abr * tree);
// libera l albero
void free_all_tree (abr *tree);

#endif