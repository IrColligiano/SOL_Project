#ifndef __TREE_H__
#define __TREE_H__

#include <utils.h>


abr *create_tree(abr *tree, long res, char * path);

void print_tree(abr * tree);

void free_tree (abr * tree);

#endif