#ifndef EVAL_H
#define EVAL_H

// Forward declarations
struct lenv;
typedef struct lenv lenv;


lval* lval_eval_sexpr(lenv*, lval*);
lval* lval_eval(lenv*, lval*);

lval* builtin(lenv*, lval*, char*);

#define LASSERT(cleanup, cond, fmt, ...) \
    if (!(cond)) { \
        lval* err = lval_err(fmt, ##__VA_ARGS__); \
        lval_del(cleanup); \
        return err; \
    }

// TODO: no need not pass both i and l
#define LASSERT_ARG_TYPE(cleanup, func, i, l, t) \
    if (l->type != t) { \
        lval* err = lval_err("Function '%s' got incorrect type " \
            "of argument %i. Got %s, expected %s", \
            func, i, ltype_name(l->type), ltype_name(t)); \
        lval_del(cleanup); \
        return err; \
    }

#define LASSERT_ARG_COUNT(cleanup, func, c) \
    if (cleanup->count != c) { \
        lval* err = lval_err("Function '%s' got incorrect number " \
            "of arguments. Got %i, expected %i", \
            func, cleanup->count, c); \
        lval_del(cleanup); \
        return err; \
    }

#define LASSERT_EMPTY_QEXPR(cleanup, q, func) \
    if (q->count != 0) { \
        lval_del(cleanup); \
        return lval_err("Function '%s' got non-empty Q-Expr", func); \
    }

#define LASSERT_NON_EMPTY_QEXPR(cleanup, q, func) \
    if (q->count == 0) { \
        lval_del(cleanup); \
        return lval_err("Function '%s' got empty Q-Expr", func); \
    }

lval* builtin_add(lenv*, lval*);
lval* builtin_sub(lenv*, lval*);
lval* builtin_mul(lenv*, lval*);
lval* builtin_div(lenv*, lval*);
lval* builtin_head(lenv*, lval*);
lval* builtin_tail(lenv*, lval*);
lval* builtin_list(lenv*, lval*);
lval* builtin_eval(lenv*, lval*);
lval* builtin_def(lenv*, lval*);
lval* builtin_put(lenv*, lval*);
lval* builtin_var(lenv*, lval*, char*);
lval* builtin_lambda(lenv*, lval*);
lval* builtin_join(lenv*, lval*);

#endif

