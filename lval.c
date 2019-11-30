
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

lval* lval_str(char* s) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_STR;
    v->str = malloc(strlen(s) + 1);
    strcpy(v->str, s);
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
        // Special case for variable number of arguments
        if (strcmp(sym->sym, "&") == 0) {
            // Make sure that '&' is followed by another symbol
            // TODO: try to rewrite as ASSERT
            if (f->formals->count != 1) {
                lval_del(a);
                return lval_err("Function format invalid. "
                    "Symbol '&' not followed by single symbol");
            }
            // Bind remaining arguments to the last formal
            lval* nsym = lval_pop(f->formals, 0);
            lenv_put(f->env, nsym, builtin_list(e, a));
            lval_del(sym); lval_del(nsym);
            break;
        }
        lval* val = lval_pop(a, 0);
        // Bind a copy of value into function environment
        lenv_put(f->env, sym, val);
        lval_del(sym); lval_del(val);
    }
    // Argument list is now bound and can be cleaned up
    lval_del(a);
    // If '&' remains in the formal list - bind it to empty list
    if (f->formals->count > 0 &&
        strcmp(f->formals->cell[0]->sym, "&") == 0) {
        if (f->formals->count != 2) {
            return lval_err("Function format invalid. "
                "Symbol '&' not followed by single symbol.");
        }
        // Pop and delete the '&' symbol
        lval_del(lval_pop(f->formals, 0));
        // Pop last symbol and create empty list
        lval* sym = lval_pop(f->formals, 0);
        lval* val = lval_qexpr();
        // Bind to environment and delete
        lenv_put(f->env, sym, val);
        lval_del(sym); lval_del(val);
    }
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
        // Free string data
        case LVAL_ERR: free(v->err); break;
        case LVAL_SYM: free(v->sym); break;
        case LVAL_STR: free(v->str); break;
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


lval* lval_read_str(mpc_ast_t* t) {
    // Cut off final quote character
    t->contents[strlen(t->contents)-1] = '\0';
    // Copy the string ommiting the first quote character
    char* unescaped = malloc(strlen(t->contents+1)+1);
    strcpy(unescaped, t->contents+1);
    // Pass through the unescape function
    unescaped = mpcf_unescape(unescaped);
    // Construct a new lval using the string
    lval* str = lval_str(unescaped);
    // Free the string
    free(unescaped);
    return str;
}


lval* lval_read(mpc_ast_t* t) {
    // For Symbol or Number return conversion to that type
    if (strstr(t->tag, "number")) { return lval_read_num(t); }
    if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }
    if (strstr(t->tag, "string")) { return lval_read_str(t); }
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
        case LVAL_STR:
            x->str = malloc(strlen(v->str) + 1);
            strcpy(x->str, v->str); break;
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


int lval_eq(lval* x, lval* y) {
    // Different types are always unequal
    if (x->type != y->type) { return 0; }
    switch (x->type) {
        // Compare numbers
        case LVAL_NUM: return (x->num == y->num);
        // Compare strings
        case LVAL_ERR: return (strcmp(x->err, y->err) == 0);
        case LVAL_SYM: return (strcmp(x->sym, y->sym) == 0);
        case LVAL_STR: return (strcmp(x->str, y->str) == 0);
        // Compare builtins directly
        // Compare formals and body for user-defined functions
        case LVAL_FUN:
           if (x->builtin || y->builtin) {
               return x->builtin == y->builtin;
           } else {
               return lval_eq(x->formals, y->formals)
                   && lval_eq(x->body, y->body);
           }
        // Compare every element of the list
        case LVAL_QEXPR:
        case LVAL_SEXPR:
           if (x->count != y->count) { return 0; };
           for (int i = 0; i < x->count; i++) {
               if (!lval_eq(x->cell[i], y->cell[i])) { return 0; }
           }
           return 1;
        break;
    }
    return 0;
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
        case LVAL_STR:   lval_print_str(v); break;
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


void lval_print_str(lval* v) {
    // Make a copy of the string
    char* escaped = malloc(strlen(v->str)+1);
    strcpy(escaped, v->str);
    // Pass it through the escape function
    escaped = mpcf_escape(escaped);
    // Print it between quote characters
    printf("\"%s\"", escaped);
    // Free the copied string
    free(escaped);
}


char* ltype_name(int t) {
    switch(t) {
        case LVAL_ERR: return "Error";
        case LVAL_NUM: return "Number";
        case LVAL_SYM: return "Symbol";
        case LVAL_STR: return "String";
        case LVAL_FUN: return "Function";
        case LVAL_SEXPR: return "S-Expr";
        case LVAL_QEXPR: return "Q-Expr";
    }
    return "Unknown";
}

