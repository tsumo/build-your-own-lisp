#ifndef EVAL_H
#define EVAL_H

// Forward declarations
struct lenv;
typedef struct lenv lenv;


lval* lval_eval_sexpr(lenv*, lval*);
lval* lval_eval(lenv*, lval*);

lval* builtin(lenv*, lval*, char*);
lval* builtin_op(lenv*, lval*, char*);

#define LASSERT(args, cond, err) \
    if (!(cond)) { lval_del(args); return lval_err(err); }

lval* builtin_add(lenv*, lval*);
lval* builtin_sub(lenv*, lval*);
lval* builtin_mul(lenv*, lval*);
lval* builtin_div(lenv*, lval*);
lval* builtin_head(lenv*, lval*);
lval* builtin_tail(lenv*, lval*);
lval* builtin_list(lenv*, lval*);
lval* builtin_eval(lenv*, lval*);
lval* builtin_join(lenv*, lval*);

#endif

