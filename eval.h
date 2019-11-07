#ifndef EVAL_H
#define EVAL_H

lval* lval_eval_sexpr(lval*);
lval* lval_eval(lval*);

lval* builtin(lval*, char*);
lval* builtin_op(lval*, char*);

#define LASSERT(args, cond, err) \
    if (!(cond)) { lval_del(args); return lval_err(err); }

lval* builtin_head(lval*);
lval* builtin_tail(lval*);
lval* builtin_list(lval*);
lval* builtin_eval(lval*);
lval* builtin_join(lval*);

#endif

