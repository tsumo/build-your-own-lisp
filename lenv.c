#include <stdlib.h>
#include "mpc.h"
#include "lenv.h"
#include "lval.h"
#include "eval.h"

lenv* lenv_new(void) {
    lenv* e = malloc(sizeof(lenv));
    e->par = NULL;
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}


void lenv_del(lenv* e) {
    for (int i = 0; i < e->count; i++) {
        free(e->syms[i]);
        lval_del(e->vals[i]);
    }
    free(e->syms);
    free(e->vals);
    free(e);
}


lval* lenv_get(lenv* e, lval* k) {
    for (int i = 0; i < e->count; i++) {
        // Check if stored string matches the symbol string
        if (strcmp(e->syms[i], k->sym) == 0) {
            return lval_copy(e->vals[i]);
        }
    }
    // Search the parent environment
    if (e->par) {
        return lenv_get(e->par, k);
    } else {
        return lval_err("Unbound symbol '%s'", k->sym);
    }
}


void lenv_put(lenv* e, lval* k, lval* v) {
    for (int i = 0; i < e->count; i++) {
        // If variable if found delete item at that position
        // and replace with supplied variable
        if (strcmp(e->syms[i], k->sym) == 0) {
            lval_del(e->vals[i]);
            e->vals[i] = lval_copy(v);
            return;
        }
    }
    // If no existing entry found allocate space for new entry
    e->count++;
    e->vals = realloc(e->vals, sizeof(lval*) * e->count);
    e->syms = realloc(e->syms, sizeof(char*) * e->count);
    // Copy contents into new locations
    e->vals[e->count-1] = lval_copy(v);
    e->syms[e->count-1] = malloc(strlen(k->sym)+1);
    strcpy(e->syms[e->count-1], k->sym);
}


void lenv_def(lenv* e, lval* k, lval* v) {
    // Iterate till e has no parent
    while (e->par) { e = e->par; }
    // Put value in outermost e
    lenv_put(e, k, v);
}


lenv* lenv_copy(lenv* e) {
    lenv* n = malloc(sizeof(lenv));
    n->par = e->par;
    n->count = e->count;
    n->syms = malloc(sizeof(char*) * n->count);
    n->vals = malloc(sizeof(lval*) * n->count);
    for (int i = 0; i < e->count; i++) {
        n->syms[i] = malloc(strlen(e->syms[i]) + 1);
        strcpy(n->syms[i], e->syms[i]);
        n->vals[i] = lval_copy(e->vals[i]);
    }
    return n;
}


void lenv_add_builtin(lenv* e, char* name, lbuiltin func) {
    lval* k = lval_sym(name);
    lval* v = lval_fun(func);
    lenv_put(e, k ,v);
    lval_del(k); lval_del(v);
}


void lenv_add_builtins(lenv* e) {
    // Math functions
    lenv_add_builtin(e, "+", builtin_add);
    lenv_add_builtin(e, "-", builtin_sub);
    lenv_add_builtin(e, "*", builtin_mul);
    lenv_add_builtin(e, "/", builtin_div);
    // List functions
    lenv_add_builtin(e, "list", builtin_list);
    lenv_add_builtin(e, "head", builtin_head);
    lenv_add_builtin(e, "tail", builtin_tail);
    lenv_add_builtin(e, "eval", builtin_eval);
    lenv_add_builtin(e, "join", builtin_join);
    // Symbol functions
    lenv_add_builtin(e, "def", builtin_def);
    lenv_add_builtin(e, "=", builtin_put);
    // Function functions?!
    lenv_add_builtin(e, "\\", builtin_lambda);
    // Comparison functions
    lenv_add_builtin(e, ">", builtin_gt);
    lenv_add_builtin(e, "<", builtin_lt);
    lenv_add_builtin(e, ">=", builtin_ge);
    lenv_add_builtin(e, "<=", builtin_le);
}

