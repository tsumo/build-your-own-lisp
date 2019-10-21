#include "mpc.h"
#include "lval.h"
#include "utils.h"


/* Evaluate application of single operation */
lval eval_op(lval x, char* op, lval y) {
    if (x.type == LVAL_ERR) { return x; }
    if (y.type == LVAL_ERR) { return y; }

    if (strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
    if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
    if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }
    if (strcmp(op, "/") == 0) {
        return y.num == 0
            ? lval_err(LERR_DIV_ZERO)
            : lval_num(x.num / y.num);
    }
    if (strcmp(op, "%") == 0) { return lval_num(x.num % y.num); }
    if (strcmp(op, "^") == 0) { return lval_num(pow(x.num, y.num)); }
    if (strcmp(op, "min") == 0) { return lval_num(min(x.num, y.num)); }
    if (strcmp(op, "max") == 0) { return lval_num(max(x.num, y.num)); }
    return lval_err(LERR_BAD_OP);
}


/* Evaluate a parse tree */
lval eval(mpc_ast_t* t) {
    /* If tagged as number - convert to long */
    if (strstr(t->tag, "number")) {
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
    }

    /* The operator is always the second child, "(" is the first one */
    char* op = t->children[1]->contents;

    /* Store third child in x */
    lval x = eval(t->children[2]);

    /* When "-" receives one argument, it should negate it */
    if (strcmp(op, "-") == 0 && !strstr(t->children[3]->tag, "expr")) {
        return lval_num(-x.num);
    }

    /* Iterate through remaining children and combine results.
       "expr" check stops iteration on the ")" node. */
    int i = 3;
    while (strstr(t->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }

    return x;
}

