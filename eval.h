#ifndef EVAL_H
#define EVAL_H

lval* lval_eval_sexpr(lval*);
lval* lval_eval(lval*);

lval* builtin_op(lval*, char*);

#endif

