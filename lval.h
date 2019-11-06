#ifndef LVAL_H
#define LVAL_H

// Result of evaluation
typedef struct lval {
    int type;
    long num;
    // Error and Symbol types have some string data
    char* err;
    char* sym;
    // Count and pointer to a list of lval*
    int count;
    struct lval** cell;
} lval;

// lval types
enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR };

lval* lval_num(long);
lval* lval_err(char*);
lval* lval_sym(char*);
lval* lval_sexpr(void);
lval* lval_qexpr(void);

void lval_del(lval*);

lval* lval_read_num(mpc_ast_t*);
lval* lval_read(mpc_ast_t*);

lval* lval_add(lval*, lval*);

lval* lval_pop(lval*, int);
lval* lval_take(lval*, int);

void lval_expr_print(lval*, char, char);
void lval_print(lval*);
void lval_println(lval*);

#endif

