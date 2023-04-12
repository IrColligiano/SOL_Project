#include <tree.h>

void print_tree(abr * tree){
    if(tree==NULL)
        return;
    print_tree(tree->left);
    printf("%ld %s\n", tree->res,tree->path);
    print_tree(tree->right); 
	return;
}
 
void free_tree (abr * tree){
    if(tree==NULL)
        return;
    free_tree(tree->right);
    free(tree->path);
    free_tree(tree->left);
    free(tree);
}

abr *create_tree(abr *tree, long res, char *path){
    if(tree == NULL){
        abr * tree = _malloc_(sizeof(abr));
        tree->path=_malloc_(sizeof(char)*MSGSCK);
        tree->left = NULL;
        tree->right = NULL;
        tree->res = res;
        _strcpy_(&(tree->path),path,MSGSCK);
        //printf("TREE: figlio %s , %ld\n",tree->path, tree->res);
        return tree;

    }
    else{
        if(res <= tree->res )
            tree->left = create_tree(tree->left,res,path);       
        else{
            if(res > tree->res )
                //if(strncmp(path,tree->path,PATHLEN!=0)){
                    tree->right = create_tree(tree->right,res,path);
                //} 
        }
        return tree;
    }
}
