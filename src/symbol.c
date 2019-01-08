/**
 * Symbol table.
 *
 * A horrendous two-level linked list.
 */

#include <stdio.h>
#include "symbol.h"
#include "shared.h"

static bool check_scope(Scope** scope, Symbol* sym);
static void free_syms(Symbol* syms);

Scope* init_scope(void)
{
    Scope* scope = calloc(sizeof(Scope), 1);
    *scope = (Scope) {
        .up = NULL,
        .down = NULL,
        .syms = NULL,
    };
    return scope;
}

Symbol* init_symbol(void)
{
    Symbol* sym = calloc(sizeof(Symbol), 1);
    *sym = (Symbol) {
        .next = NULL,
        .prev = NULL,
        .cat = CAT_NONE,
        .type = TYPE_NONE,
        .id = NULL,
    };
    return sym;
}

void enter_scope(Scope** scope)
{
    assert(*scope != NULL);

    if ((*scope)->syms == NULL) {
        assert((*scope)->down == NULL && (*scope)->up == NULL);

        (*scope)->syms = init_symbol();
    } else {
        assert((*scope)->up == NULL);

        Scope* new = init_scope();
        new->syms = init_symbol();
        (*scope)->up = new;
        new->down = *scope;
        *scope = new;
    }
}

void exit_scope(Scope** scope)
{
    assert((*scope) != NULL);

    if ((*scope)->down == NULL) {
        assert((*scope)->syms != NULL);

        free_syms((*scope)->syms);
        free(*scope);
    } else {
        Scope* new = (*scope)->down;
        new->up = NULL;

        free_syms((*scope)->syms);
        free(*scope);

        *scope = new;
    }
}

void add_symbol(Scope** scope, Symbol* sym)
{
    assert((*scope)->syms != NULL);

    if (check_scope(scope, sym) == true) {
        printf("Error: variable %s already defined\n", sym->id);
        exit(ANALYSER_ERROR);
    }

    if ((*scope)->syms->id == NULL) {
        assert((*scope)->syms->next == NULL && (*scope)->syms->prev == NULL);
        (*scope)->syms = sym;
    } else {
        sym->next = (*scope)->syms;
        (*scope)->syms->prev = sym;
        (*scope)->syms = sym;
    }
}

bool find_symbol(Scope** scope, Symbol* sym)
{
    assert(((*scope) != NULL) && ((*scope)->syms != NULL) && (sym != NULL));

    bool cond = false;
    Scope** orig = scope;

    while ((*scope) != NULL) {
        cond = check_scope(scope, sym);
        if (cond == true) {
            return true;
        }
        scope = &((*scope)->down);
    }
    scope = orig;
    return cond;
}

Symbol* get_sym(Scope** scope, char* id)
{
    Scope** orig = scope;
    Symbol* val = NULL;

    while ((*scope) != NULL) {
        Symbol* orig_sym = (*scope)->syms;

        while ((*scope)->syms != NULL && (*scope)->syms->cat != CAT_NONE) {
            if (strcmp((*scope)->syms->id, id) == 0) {
                val = (*scope)->syms;
                (*scope)->syms = orig_sym;
                scope = orig;
                return val;
            }
            (*scope)->syms = (*scope)->syms->next;
        }

        (*scope)->syms = orig_sym;
        scope = &((*scope)->down);
    }
    scope = orig;
    return val;
}

Symbol* get_func(Scope** scope)
{
    Scope** orig = scope;
    Symbol* val = NULL;

    while ((*scope) != NULL) {
        Symbol* orig_sym = (*scope)->syms;

        while ((*scope)->syms != NULL && (*scope)->syms->cat != CAT_NONE) {
            if ((*scope)->syms->cat == CAT_FUNC) {
                val = (*scope)->syms;
                (*scope)->syms = orig_sym;
                scope = orig;
                return val;
            }
            (*scope)->syms = (*scope)->syms->next;
        }
        (*scope)->syms = orig_sym;
        scope = &((*scope)->down);
    }

    scope = orig;
    return val;
}

/* Private */

static bool is_sym_equal(Symbol* s1, Symbol* s2)
{
    assert((s1 != NULL) & (s2 != NULL));

    if ((s1->id == NULL) || (s2->id == NULL)) {
        return false;
    } else if (!(strcmp(s1->id, s2->id))) {
        return true;
    }

    return false;
}

static void free_syms(Symbol* syms)
{
    assert(syms != NULL);

    while (syms != NULL) {
        if (syms->next == NULL) {
            free(syms);
            syms = NULL;
        } else {
            Symbol* orig = syms;
            syms = syms->next;
            free(orig);
            syms->prev = NULL;
        }
    }
}

static bool check_scope(Scope** scope, Symbol* sym)
{
    assert(((*scope) != NULL) && ((*scope)->syms != NULL) && (sym != NULL));

    Symbol* orig = (*scope)->syms;

    while ((*scope)->syms != NULL) {
        if (is_sym_equal((*scope)->syms, sym)) {
            (*scope)->syms = orig;
            return true;
        }
        (*scope)->syms = (*scope)->syms->next;
    }

    (*scope)->syms = orig;
    return false;
}
