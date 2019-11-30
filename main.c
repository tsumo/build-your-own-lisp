#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"
#include "lenv.h"
#include "lval.h"
#include "eval.h"

#include <editline/readline.h>
// #include <editline/histedit.h>


int main(int argc, char** argv) {

    // Parsers
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* String = mpc_new("string");
    mpc_parser_t* Sexpr  = mpc_new("sexpr");
    mpc_parser_t* Qexpr  = mpc_new("qexpr");
    mpc_parser_t* Expr   = mpc_new("expr");
    mpc_parser_t* Lispy  = mpc_new("lispy");

    // Language
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                               \
            number : /-?[0-9]+/ ;                       \
            symbol : /[A-Za-z0-9_+\\-*\\/\\\\=<>!&]+/ ; \
            string : /\"(\\\\.|[^\"])*\"/ ;             \
            sexpr  : '(' <expr>* ')' ;                  \
            qexpr  : '{' <expr>* '}' ;                  \
            expr   : <number> | <symbol> | <string> |   \
                     <sexpr> | <qexpr> ;                \
            lispy  : /^/ <expr>* /$/ ;                  \
        ",
        Number, Symbol, String, Sexpr, Qexpr, Expr, Lispy);

    puts("Lispy version 0.0.0.0.1");
    puts("Press Ctrl+c to exit\n");

    // Create environment
    lenv* e = lenv_new();
    lenv_add_builtins(e);

    while (1) {
        // Output prompt and get input
        char* input = readline("lispy> ");
        add_history(input);

        // Parse input
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            // Print results of parsing. Usefull for parser debugging
            // mpc_ast_print(r.output);

            // Evaluate parse tree
            lval* x = lval_eval(e, lval_read(r.output));
            lval_println(x);
            lval_del(x);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    lenv_del(e);
    mpc_cleanup(7, Number, Symbol, String, Sexpr, Qexpr, Expr, Lispy);

    return 0;
}

