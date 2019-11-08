#ifndef LENV_H
#define LENV_H

// Forward declarations
struct lval;
typedef struct lval lval;
struct lenv;
typedef struct lenv lenv;
typedef lval* (*lbuiltin)(lenv*, lval*);


typedef struct lenv {
    int count;
    char** syms;
    lval** vals;
} lenv;

lenv* lenv_new(void);

void lenv_del(lenv*);

lval* lenv_get(lenv*, lval*);
void lenv_put(lenv*, lval*, lval*);

void lenv_add_builtin(lenv*, char*, lbuiltin);
void lenv_add_builtins(lenv*);

#endif

