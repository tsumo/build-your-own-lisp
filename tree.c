#include "mpc.h"
#include "utils.h"


// Counts number of nodes in a parse tree
int nodes_num(mpc_ast_t* t) {
    if (t->children_num == 0) { return 1; }
    if (t->children_num >= 1) {
        int total = 1;
        for (int i = 0; i < t->children_num; i++) {
            total = total + nodes_num(t->children[i]);
        }
        return total;
    }
    return 0;
}


// Counts number of leaves in a parse tree
int leaves_num(mpc_ast_t* t) {
    if (t->children_num == 0) { return 1; }
    if (t->children_num >= 1) {
        int total = 0;
        for (int i = 0; i < t->children_num; i++) {
            total = total + leaves_num(t->children[i]);
        }
        return total;
    }
    return 0;
}


// Counts number of branches in a parse tree
int branches_num(mpc_ast_t* t) {
    if (t->children_num == 0) { return 0; }
    if (t->children_num >= 1) {
        int total = 1;
        for (int i = 0; i < t->children_num; i++) {
            total = total + branches_num(t->children[i]);
        }
        return total;
    }
    return 0;
}


// Finds the most number of children spanning from one branch
int max_children(mpc_ast_t* t) {
    if (t->children_num == 0) { return 0; }
    if (t->children_num >= 1) {
        int children = t->children_num;
        for (int i = 0; i < t->children_num; i++) {
            int x = max_children(t->children[i]);
            children = max(children, x);
        }
        return children;
    }
    return 0;
}

