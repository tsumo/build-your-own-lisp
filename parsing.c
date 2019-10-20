#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

#include <editline/readline.h>
/* #include <editline/histedit.h> */

/* Counting number of nodes in a parse tree */
int number_of_nodes(mpc_ast_t* t) {
    if (t->children_num == 0) { return 1; }
    if (t->children_num >= 1) {
        int total = 1;
        for (int i = 0; i < t->children_num; i++) {
            total = total + number_of_nodes(t->children[i]);
        }
        return total;
    }
    return 0;
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
            number   : /-?[0-9]+(\\.[0-9]+)?/ ;                \
            operator : '+' | '-' | '*' | '/' ;                 \
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
            mpc_ast_print(r.output);
            printf("Number of nodes: %i\n", number_of_nodes(r.output));
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

