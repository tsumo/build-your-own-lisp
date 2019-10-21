#ifndef LVAL_H
#define LVAL_H

/* Result of evaluation */
typedef struct {
    int type;
    long num;
    int err;
} lval;

/* lval types */
enum { LVAL_NUM, LVAL_ERR };

/* lval errors */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

lval lval_num(long);
lval lval_err(int);

void lval_print(lval);
void lval_println(lval);

#endif

