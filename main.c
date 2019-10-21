#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpc.h"
#include "tree.h"
#include "utils.h"
#include "lval.h"

#include <editline/readline.h>
/* #include <editline/histedit.h> */


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
    if (strcmp(op, "%") == 0) { return lval_num(x.num * y.num); }
    if (strcmp(op, "^") == 0) { return lval_num(pow(x.num, y.num)); }
    if (strcmp(op, "min") == 0) { return lval_num(min(x.num, y.num)); }
    if (strcmp(op, "max") == 0) { return lval_num(max(x.num, y.num)); }
    return lval_err(LERR_BAD_OP);
}


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


int main(int argc, char** argv) {

    /* Parsers */
    mpc_parser_t* Number   = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr     = mpc_new("expr");
    mpc_parser_t* Lispy    = mpc_new("lispy");

    /* Language */
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                      \
            number   : /-?[0-9]+/ ;                            \
            operator : '+' | '-' | '*' | '/' | '%' | '^' |     \
                       \"min\" | \"max\" ;                     \
            expr     : <number> | '(' <operator> <expr>+ ')' ; \
            lispy    : /^/ <operator> <expr>+ /$/ ;            \
        ",
        Number, Operator, Expr, Lispy);

    puts("Lispy version 0.0.0.0.1");
    puts("Press Ctrl+c to exit\n");

    while (1) {
        /* Output prompt and get input */
        char* input = readline("lispy> ");
        add_history(input);

        /* Parse input */
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            /* Evaluate parse tree */
            lval result = eval(r.output);
            lval_println(result);
            /* mpc_ast_print(r.output); */
            /* printf("Number of nodes: %i\n", nodes_num(r.output)); */
            /* printf("Number of leaves: %i\n", leaves_num(r.output)); */
            /* printf("Number of branches: %i\n", branches_num(r.output)); */
            /* printf("Max children: %i\n", max_children(r.output)); */
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    mpc_cleanup(4, Number, Operator, Expr, Lispy);

    return 0;
}

