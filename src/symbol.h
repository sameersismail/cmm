#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

/* Data Structures */
typedef enum { 
    CAT_NONE,
    CAT_VAR,
    CAT_VAR_ARR,
    CAT_VAR_SIN,
    CAT_FUNC
} Category;

typedef struct Symbol {
    char* id;
    short len;

    Category cat;
    enum Type type;

    bool local;
    int offset;

    struct Symbol* next;
    struct Symbol* prev;
} Symbol;

typedef struct Scope {
    Symbol* syms;

    struct Scope* up;
    struct Scope* down;
} Scope;

/* Function Prototypes */
Scope* init_scope(void);
Symbol* init_symbol(void);
Symbol* get_func(Scope** scope);
Symbol* get_sym(Scope** scope, char* id);

void enter_scope(Scope** scope);
void exit_scope(Scope** scope);
void add_symbol(Scope** scope, Symbol* sym);
bool find_symbol(Scope** scope, Symbol* sym);
