
#include "mpc.h"
#include "lval.h"
#include "lenv.h"
#include "eval.h"

// Constructors of pointers to a new lval structs
lval* lval_num(long x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

lval* lval_err(char* fmt, ...) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    // Initializing variable argument struct
    va_list va;
    va_start(va, fmt);
    // Allocate space for message
    v->err = malloc(512);
    // printf the error string with a maximum of 511 chars
    vsnprintf(v->err, 511, fmt, va);
    // Reallocate to number of bytes actually used
    v->err = realloc(v->err, strlen(v->err)+1);
    // Cleanup va list
    va_end(va);
    return v;
}

lval* lval_sym(char* s) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1);
    strcpy(v->sym, s);
    return v;
}

lval* lval_fun(lbuiltin func) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
    v->builtin = func;
    return v;
}

lval* lval_lambda(lval* formals, lval* body) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
    // Mark function as user-defined
    v->builtin = NULL;
    // Environment for values of formal arguments
    v->env = lenv_new();
    // Assign function data
    v->formals = formals;
    v->body = body;
    return v;
}

lval* lval_sexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}


lval* lval_qexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}


lval* lval_call(lenv* e, lval* f, lval* a) {
    // Call builtins directly
    if (f->builtin) { return f->builtin(e, a); }
    // Record argument counts
    int given = a->count;
    int total = f->formals->count;
    // While we have arguments
    while (a->count) {
        if (f->formals->count == 0) {
            lval_del(a);
            // TODO: pass function name for better UX
            return lval_err("Function got incorrect number "
                "of arguments. Got %i, expected %i",
                given, total);
        }
        lval* sym = lval_pop(f->formals, 0);
        lval* val = lval_pop(a, 0);
        // Bind a copy of value into function environment
        lenv_put(f->env, sym, val);
        lval_del(sym); lval_del(val);
    }
    lval_del(a);
    // Evaluate if all formals are bound
    if (f->formals->count == 0) {
        // Set parent environment
        f->env->par = e;
        return builtin_eval(f->env,
            lval_add(lval_sexpr(), lval_copy(f->body)));
    } else {
        // Otherwise return partially evaluated function
        return lval_copy(f);
    }
}


void lval_del(lval* v) {
    switch (v->type) {
        // Do nothing special for number type
        case LVAL_NUM: break;
        // For Errors or Symbols free the string data
        case LVAL_ERR: free(v->err); break;
        case LVAL_SYM: free(v->sym); break;
        case LVAL_FUN:
            // Free user-defined function data
            if (!v->builtin) {
                lenv_del(v->env);
                lval_del(v->formals);
                lval_del(v->body);
            }
            // Don't touch builtin function pointer
            break;
        // Recursively free up Sexprs and Qexprs
        case LVAL_SEXPR:
        case LVAL_QEXPR:
           for (int i = 0; i < v->count; i++) {
               lval_del(v->cell[i]);
           }
           // Also free the memory allocated to contain the pointers
           free(v->cell);
           break;
    }
    // Free the memory of the lval struct itself
    free(v);
}


lval* lval_read_num(mpc_ast_t* t) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE
        ? lval_num(x)
        : lval_err("Invalid number '%s'", t->contents);
}


lval* lval_read(mpc_ast_t* t) {
    // For Symbol or Number return conversion to that type
    if (strstr(t->tag, "number")) { return lval_read_num(t); }
    if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }
    // For root (>) or Sexpr create empty list
    lval* x = NULL;
    if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
    if (strstr(t->tag, "sexpr"))  { x = lval_sexpr(); }
    if (strstr(t->tag, "qexpr"))  { x = lval_qexpr(); }
    // Fill this list with any valid expressions contained within
    for (int i = 0; i < t->children_num; i++) {
        if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
        if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
        if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
        if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
        if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
        x = lval_add(x, lval_read(t->children[i]));
    }
    return x;
}


lval* lval_add(lval* v, lval* x) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    v->cell[v->count-1] = x;
    return v;
}


lval* lval_pop(lval* v, int i) {
    // Extract the item at i
    lval* x = v->cell[i];
    // Shift the rest of the memory after the item backwards
    memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*) * (v->count-i-1));
    // Decrease the count of items in the list
    v->count--;
    // Reallocate the memory used
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    return x;
}


// Similar to lval_pop, but deletes the provided list
lval* lval_take(lval* v, int i) {
    lval* x = lval_pop(v, i);
    lval_del(v);
    return x;
}


lval* lval_join(lval* x, lval* y) {
    // Add every cell in 'y' to 'x'
    while (y->count) {
        x = lval_add(x, lval_pop(y, 0));
    }
    lval_del(y);
    return x;
}


lval* lval_copy(lval* v) {
    lval* x = malloc(sizeof(lval));
    x->type = v->type;

    switch (v->type) {
        // Copy numbers directly
        case LVAL_NUM: x->num = v->num; break;
        // Copy strings with malloc and strcpy
        case LVAL_ERR:
            x->err = malloc(strlen(v->err) + 1);
            strcpy(x->err, v->err); break;
        case LVAL_SYM:
            x->sym = malloc(strlen(v->sym) + 1);
            strcpy(x->sym, v->sym); break;
        // Copy function
        case LVAL_FUN:
            if (v->builtin) {
                x->builtin = v->builtin;
            } else {
                x->builtin = NULL;
                x->env = lenv_copy(v->env);
                x->formals = lval_copy(v->formals);
                x->body = lval_copy(v->body);
            }
            break;
        // Copy lists by copying each sub-expression
        case LVAL_SEXPR:
        case LVAL_QEXPR:
            x->count = v->count;
            x->cell = malloc(sizeof(lval*) * x->count);
            for (int i = 0; i < x->count; i++) {
                x->cell[i] = lval_copy(v->cell[i]);
            }
            break;
    }
    return x;
}


void lval_expr_print(lval* v, char open, char close) {
    putchar(open);
    for (int i = 0; i < v->count; i++) {
        // Print value contained within
        lval_print(v->cell[i]);
        // Don't print trailing space if last element
        if (i != (v->count-1)) {
            putchar(' ');
        }
    }
    putchar(close);
}


void lval_print(lval* v) {
    switch (v->type) {
        case LVAL_NUM:   printf("%li", v->num); break;
        case LVAL_ERR:   printf("Error: %s", v->err); break;
        case LVAL_SYM:   printf("%s", v->sym); break;
        case LVAL_FUN:
            if (v->builtin) {
                printf("<builtin>");
            } else {
                printf("(\\ "); lval_print(v->formals);
                putchar(' '); lval_print(v->body); putchar(')');
            }
            break;
        case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
        case LVAL_QEXPR: lval_expr_print(v, '{', '}'); break;
    }
}


void lval_println(lval* v) { lval_print(v); putchar('\n'); }


char* ltype_name(int t) {
    switch(t) {
        case LVAL_ERR: return "Error";
        case LVAL_NUM: return "Number";
        case LVAL_SYM: return "Symbol";
        case LVAL_FUN: return "Function";
        case LVAL_SEXPR: return "S-Expr";
        case LVAL_QEXPR: return "Q-Expr";
    }
    return "Unknown";
}

