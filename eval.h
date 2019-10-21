#ifndef EVAL_H
#define EVAL_H

lval eval_op(lval, char*, lval);
lval eval(mpc_ast_t*);

#endif

