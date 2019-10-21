#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"
#include "tree.h"
#include "lval.h"
#include "eval.h"

#include <editline/readline.h>
/* #include <editline/histedit.h> */


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

