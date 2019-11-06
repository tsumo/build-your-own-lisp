#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"
#include "lval.h"
#include "eval.h"

#include <editline/readline.h>
// #include <editline/histedit.h>


int main(int argc, char** argv) {

    // Parsers
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr  = mpc_new("sexpr");
    mpc_parser_t* Expr   = mpc_new("expr");
    mpc_parser_t* Lispy  = mpc_new("lispy");

    // Language
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                    \
            number : /-?[0-9]+/ ;                            \
            symbol : '+' | '-' | '*' | '/' | '%' | '^' |     \
                     \"min\" | \"max\" ;                     \
            sexpr  : '(' <expr>* ')' ;                       \
            expr   : <number> | <symbol> | <sexpr> ;         \
            lispy  : /^/ <expr>* /$/ ;                       \
        ",
        Number, Symbol, Sexpr, Expr, Lispy);

    puts("Lispy version 0.0.0.0.1");
    puts("Press Ctrl+c to exit\n");

    while (1) {
        // Output prompt and get input
        char* input = readline("lispy> ");
        add_history(input);

        // Parse input
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            // Evaluate parse tree
            lval* x = lval_eval(lval_read(r.output));
            lval_println(x);
            // mpc_ast_print(r.output);
            lval_del(x);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);

    return 0;
}

