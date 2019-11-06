#include "mpc.h"
#include "lval.h"
#include "utils.h"
#include "eval.h"

lval* lval_eval_sexpr(lval* v) {
    // Evaluate children
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = lval_eval(v->cell[i]);
    }
    // Error checking
    for (int i = 0; i < v->count; i++) {
        if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
    }
    // Empty expression
    if (v->count == 0) { return v; }
    // Single expression
    if (v->count == 1) { return lval_take(v, 0); }
    // Ensure that first element is Symbol
    lval* f = lval_pop(v, 0);
    if (f->type != LVAL_SYM) {
        lval_del(f); lval_del(v);
        return lval_err("S-expression does not start with symbol!");
    }
    // Call builtin with operator
    lval* result = builtin_op(v, f->sym);
    lval_del(f);
    return result;
}


lval* lval_eval(lval* v) {
    // Evaluate S-expression
    if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(v); }
    // All other lval types remain the same
    return v;
}


lval* builtin_op(lval* a, char* op) {
    // Ensure all arguments are numbers
    for (int i = 0; i < a->count; i++) {
        if (a->cell[i]->type != LVAL_NUM) {
            lval_del(a);
            return lval_err("Cannot operate on non-number");
        }
    }
    // Pop the first element
    lval* x = lval_pop(a, 0);
    // When "-" receives one argument, it should negate it
    if ((strcmp(op, "-") == 0) && a->count == 0) {
        x->num = -x->num;
    }
    // While there are still elements remaining
    while (a->count > 0) {
        lval* y = lval_pop(a, 0);

        if (strcmp(op, "+") == 0) { x->num += y->num; }
        if (strcmp(op, "-") == 0) { x->num -= y->num; }
        if (strcmp(op, "*") == 0) { x->num *= y->num; }
        if (strcmp(op, "/") == 0 || strcmp(op, "%") == 0) {
            if (y->num == 0) {
                lval_del(x); lval_del(y);
                x = lval_err("Division or modulo by zero!"); break;
            }
            if (strcmp(op, "/") == 0) {
                x->num /= y->num;
            } else {
                x->num %= y->num;
            }
        }
        if (strcmp(op, "^") == 0) { x->num = pow(x->num, y->num); }
        if (strcmp(op, "min") == 0) { x->num = min(x->num, y->num); }
        if (strcmp(op, "max") == 0) { x->num = max(x->num, y->num); }

        lval_del(y);
    }
    lval_del(a);
    return x;
}

