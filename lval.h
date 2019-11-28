#ifndef LVAL_H
#define LVAL_H

// Forward declarations
struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;


// Function pointer to a builtin
typedef lval*(*lbuiltin)(lenv*, lval*);

// Value - result of reading input of doing computation
typedef struct lval {
    int type;

    // Basic properties
    long num;
    // Error and Symbol types have some string data
    char* err;
    char* sym;

    // Function properties
    lbuiltin builtin;
    lenv* env;
    lval* formals;
    lval* body;

    // Expression properties
    int count;
    struct lval** cell;
} lval;

// lval types
enum { LVAL_ERR, LVAL_NUM,   LVAL_SYM,
       LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR };

lval* lval_num(long);
lval* lval_err(char*, ...);
lval* lval_sym(char*);
lval* lval_fun(lbuiltin);
lval* lval_lambda(lval*, lval*);
lval* lval_sexpr(void);
lval* lval_qexpr(void);

lval* lval_call(lenv*, lval*, lval*);

void lval_del(lval*);

lval* lval_read_num(mpc_ast_t*);
lval* lval_read(mpc_ast_t*);

lval* lval_add(lval*, lval*);

lval* lval_pop(lval*, int);
lval* lval_take(lval*, int);

lval* lval_join(lval*, lval*);

lval* lval_copy(lval*);

void lval_expr_print(lval*, char, char);
void lval_print(lval*);
void lval_println(lval*);

char* ltype_name(int);

#endif

