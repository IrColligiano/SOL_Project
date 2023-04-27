
#include <tree.h>
 
abr * root;

void print_tree(abr * tree){
    if(tree == NULL)
        return;
    print_tree(tree->left);
    printf("%ld %s\n", tree->res ,tree->path);
    print_tree(tree->right); 
	return;
}
 
void free_tree (abr * tree){
    if(tree == NULL)
        return;
    free_tree(tree->right);
    free_tree(tree->left);
    free(tree->path);
    free(tree);
}

void free_all_tree (abr *tree){
	if (tree -> left != NULL)		/* Se esiste un sottoalbero sinistro */
	    free_all_tree (tree -> left);	/* Libera prima la memoria di quel sottoalbero */
	if (tree -> right != NULL)		/* Se esiste un sottoalbero destro */
	    free_all_tree (tree -> right);
    free(tree -> path);
	free(tree);
}

abr *create_tree(abr *tree, long res, char *path,size_t len){
    if(tree == NULL){
        abr * tree =  _malloc_(sizeof(abr));
        tree->path =  _malloc_(sizeof(char)*len);
        memset(tree->path,'\0',len);
        tree->len =   len;
        tree->left =  NULL;
        tree->right = NULL;
        tree->res =   res;
        strncpy(tree->path,path,len);
        return tree;

    }
    else{
        if(res <= tree->res )
            tree->left = create_tree(tree->left,res,path,len);       
        else{
            if(res > tree->res )
                    tree->right = create_tree(tree->right,res,path,len);
        }
        return tree;
    }
}
