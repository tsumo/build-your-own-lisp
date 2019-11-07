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
        return lval_err("S-expression does not start with symbol");
    }
    // Call builtin with operator
    lval* result = builtin(v, f->sym);
    lval_del(f);
    return result;
}


lval* lval_eval(lval* v) {
    // Evaluate S-expression
    if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(v); }
    // All other lval types remain the same
    return v;
}


lval* builtin(lval* a, char* func) {
    if (strcmp("list", func) == 0) { return builtin_list(a); }
    if (strcmp("head", func) == 0) { return builtin_head(a); }
    if (strcmp("tail", func) == 0) { return builtin_tail(a); }
    if (strcmp("join", func) == 0) { return builtin_join(a); }
    if (strcmp("eval", func) == 0) { return builtin_eval(a); }
    if (
        strcmp("+", func)   == 0 || strcmp("-", func)   == 0 ||
        strcmp("min", func) == 0 || strcmp("max", func) == 0 ||
        strcmp("*", func)   == 0 || strcmp("/", func)   == 0 ||
        strcmp("%", func)   == 0 || strcmp("^", func)   == 0
    ) {
        return builtin_op(a, func);
    }
    lval_del(a);
    return lval_err("Unknown function");
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
                x = lval_err("Division or modulo by zero"); break;
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


lval* builtin_head(lval* a) {
    LASSERT(a, a->count == 1,
        "Too many arguments to 'head'");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
        "Only Qexpr can be passed to 'head'");
    LASSERT(a, a->cell[0]->count != 0,
        "Empty Qexpr passed to 'head'");
    lval* v = lval_take(a, 0);
    // Delete everything but the head
    while (v->count > 1) { lval_del(lval_pop(v, 1)); }
    return v;
}


lval* builtin_tail(lval* a) {
    LASSERT(a, a->count == 1,
        "Too many argument to 'tail'");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
        "Only Qexpr can be passed to 'tail'");
    LASSERT(a, a->cell[0]->count != 0,
        "Empty Qexpr passed to 'tail'");
    lval* v = lval_take(a, 0);
    // Delete head
    lval_del(lval_pop(v, 0));
    return v;
}


lval* builtin_list(lval* a) {
    a->type = LVAL_QEXPR;
    return a;
}


lval* builtin_eval(lval* a) {
    LASSERT(a, a->count == 1,
        "Too many arguments to 'eval'");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
        "Only Qexpr can be passed to 'eval'");
    lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(x);
}


lval* builtin_join(lval* a) {
    for (int i = 0; i < a->count; i++) {
        LASSERT(a, a->cell[i]->type == LVAL_QEXPR,
            "Only Qexpr can be passed to 'join'");
    }
    lval* x = lval_pop(a, 0);
    while (a->count) {
        x = lval_join(x, lval_pop(a, 0));
    }
    lval_del(a);
    return x;
}

