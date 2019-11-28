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
    // Ensure that first element is function
    lval* f = lval_pop(v, 0);
    if (f->type != LVAL_FUN) {
        lval* err = lval_err("S-Expr starts with %s instead of %s",
            ltype_name(f->type), ltype_name(LVAL_FUN));
        lval_del(f); lval_del(v);
        return err;
    }
    // Call function
    lval* result = lval_call(e, f, v);
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


lval* builtin_op(lenv* e, lval* a, char* op) {
    // Ensure all arguments are numbers
    for (int i = 0; i < a->count; i++) {
        LASSERT_ARG_TYPE(a, op, i+1, a->cell[i], LVAL_NUM);
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
    LASSERT_ARG_COUNT(a, "head", 1);
    LASSERT_ARG_TYPE(a, "head", 0, a->cell[0], LVAL_QEXPR);
    LASSERT_NON_EMPTY_QEXPR(a, a->cell[0], "head");
    lval* v = lval_take(a, 0);
    // Delete everything but the head
    while (v->count > 1) { lval_del(lval_pop(v, 1)); }
    return v;
}

lval* builtin_tail(lenv* e, lval* a) {
    LASSERT_ARG_COUNT(a, "tail", 1);
    LASSERT_ARG_TYPE(a, "tail", 0, a->cell[0], LVAL_QEXPR);
    LASSERT_NON_EMPTY_QEXPR(a, a->cell[0], "tail");
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
    LASSERT_ARG_COUNT(a, "eval", 1);
    LASSERT_ARG_TYPE(a, "eval", 0, a->cell[0], LVAL_QEXPR);
    lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(e, x);
}

lval* builtin_def(lenv* e, lval* a) {
    return builtin_var(e, a, "def");
}

lval* builtin_put(lenv* e, lval* a) {
    return builtin_var(e, a, "=");
}

lval* builtin_var(lenv* e, lval* a, char* func) {
    LASSERT_ARG_TYPE(a, func, 0, a->cell[0], LVAL_QEXPR);
    // First argument is a symbol list
    lval* syms = a->cell[0];
    // Ensure that all elements of first list are symbols
    for (int i = 0; i < syms->count; i++) {
        LASSERT_SYM_LIST(a, func, i, syms)
    }
    // Check for correct number of symbols and values
    LASSERT(a, syms->count == a->count-1,
        "Unmatched number of symbols and values "
        "passed to 'def'. Symbols: %i, values: %i",
        syms->count, a->count-1);
    // Assign copies of values to symbols
    for (int i = 0; i< syms->count; i++) {
        // If 'def' define globally
        if (strcmp(func, "def") == 0) {
            lenv_def(e, syms->cell[i], a->cell[i+1]);
        }
        // If 'put' define locally
        if (strcmp(func, "=") == 0) {
            lenv_put(e, syms->cell[i], a->cell[i+1]);
        }
    }
    lval_del(a);
    // Return empty expression on success
    return lval_sexpr();
}

lval* builtin_lambda(lenv* e, lval* a) {
    LASSERT_ARG_COUNT(a, "\\", 2);
    LASSERT_ARG_TYPE(a, "\\", 0, a->cell[0], LVAL_QEXPR);
    LASSERT_ARG_TYPE(a, "\\", 1, a->cell[1], LVAL_QEXPR);
    // Check that first Q-Expr contains only Symbols
    for (int i = 0; i < a->cell[0]->count; i++) {
        LASSERT_SYM_LIST(a, "\\", i, a->cell[0])
    }
    lval* formals = lval_pop(a, 0);
    lval* body = lval_pop(a, 0);
    lval_del(a);
    return lval_lambda(formals, body);
}

lval* builtin_join(lenv* e, lval* a) {
    for (int i = 0; i < a->count; i++) {
        LASSERT_ARG_TYPE(a, "join", i, a->cell[i], LVAL_QEXPR);
    }
    lval* x = lval_pop(a, 0);
    while (a->count) {
        x = lval_join(x, lval_pop(a, 0));
    }
    lval_del(a);
    return x;
}

