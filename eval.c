#include "mpc.h"
#include "lval.h"
#include "lenv.h"
#include "eval.h"

lval* lval_eval_sexpr(lenv* e, lval* v) {
    // Evaluate children
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = lval_eval(e, v->cell[i]);
    }
    // Error checking
    for (int i = 0; i < v->count; i++) {
        if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
    }
    // Empty expression
    if (v->count == 0) { return v; }
    // Single expression
    if (v->count == 1) { return lval_take(v, 0); }
    // Ensure that first element is function after evaluation
    lval* f = lval_pop(v, 0);
    if (f->type != LVAL_FUN) {
        lval_del(f); lval_del(v);
        return lval_err("S-expression does not start with function");
    }
    // Call function
    lval* result = f->fun(e, v);
    lval_del(f);
    return result;
}


lval* lval_eval(lenv* e, lval* v) {
    // Lookup symbols in environment
    if (v->type == LVAL_SYM) {
        lval* x = lenv_get(e, v);
        lval_del(v);
        return x;
    }
    // Evaluate S-expression
    if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(e, v); }
    // All other lval types remain the same
    return v;
}


lval* builtin(lenv* e, lval* a, char* func) {
    if (strcmp("list", func) == 0) { return builtin_list(e, a); }
    if (strcmp("head", func) == 0) { return builtin_head(e, a); }
    if (strcmp("tail", func) == 0) { return builtin_tail(e, a); }
    if (strcmp("join", func) == 0) { return builtin_join(e, a); }
    if (strcmp("eval", func) == 0) { return builtin_eval(e, a); }
    if (
        strcmp("+", func)   == 0 || strcmp("-", func)   == 0 ||
        strcmp("*", func)   == 0 || strcmp("/", func)   == 0
    ) {
        return builtin_op(e, a, func);
    }
    lval_del(a);
    return lval_err("Unknown function");
}


lval* builtin_op(lenv* e, lval* a, char* op) {
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
        if (strcmp(op, "/") == 0) {
            if (y->num == 0) {
                lval_del(x); lval_del(y);
                x = lval_err("Division by zero"); break;
            }
            x->num /= y->num;
        }

        lval_del(y);
    }
    lval_del(a);
    return x;
}

lval* builtin_add(lenv* e, lval* a) {
    return builtin_op(e, a, "+");
}

lval* builtin_sub(lenv* e, lval* a) {
    return builtin_op(e, a, "-");
}

lval* builtin_mul(lenv* e, lval* a) {
    return builtin_op(e, a, "*");
}

lval* builtin_div(lenv* e, lval* a) {
    return builtin_op(e, a, "/");
}

lval* builtin_head(lenv* e, lval* a) {
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

lval* builtin_tail(lenv* e, lval* a) {
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

lval* builtin_list(lenv* e, lval* a) {
    a->type = LVAL_QEXPR;
    return a;
}

lval* builtin_eval(lenv* e, lval* a) {
    LASSERT(a, a->count == 1,
        "Too many arguments to 'eval'");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
        "Only Qexpr can be passed to 'eval'");
    lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(e, x);
}

lval* builtin_def(lenv* e, lval* a) {
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
        "Only Qexpr can be passed to 'def'");
    // First argument is a symbol list
    lval* syms = a->cell[0];
    // Ensure that all elements of first list are symbols
    for (int i = 0; i < syms->count; i++) {
        LASSERT(a, syms->cell[i]->type == LVAL_SYM,
            "Function 'def' cannot define non-symbol");
    }
    // Check for correct number of symbols and values
    LASSERT(a, syms->count == a->count-1,
        "Unmatching number of symbols and values "
        "passed to 'def'");
    // Assign copies of values to symbols
    for (int i = 0; i< syms->count; i++) {
        lenv_put(e, syms->cell[i], a->cell[i+1]);
    }
    lval_del(a);
    // Return empty expression on success
    return lval_sexpr();
}

lval* builtin_join(lenv* e, lval* a) {
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

