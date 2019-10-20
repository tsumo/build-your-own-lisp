#include "mpc.h"


/* Counts number of nodes in a parse tree */
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

