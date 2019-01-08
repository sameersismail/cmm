/**
 * Generate MIPS assembly.
 */

#pragma once

#include "ast.h"
#include "symbol.h"

/* Data Structures */
typedef struct Target {
    FILE* out;
    char* filename;
    bool in_code;
    int label_count;
} Target;

/* Function Prototypes */
void cgen(Node* n, Target* target);

// Internal
void cgen_params(Node* n, Scope* s, Target* target);
void cgen_cstmt(Node* n, Scope* s, Target* target);
void cgen_decs(Node* n, Scope* s, Target* target);
void cgen_stmts(Node* n, Scope* s, Target* target);

void cgen_if(Node* n, Scope* s, Target* target);
void cgen_expr(Node* n, Scope* s, Target* target);
void cgen_while(Node* n, Scope* s, Target* target);
void cgen_ret(Node* n, Scope* s, Target* target);

void cgen_call(Node* n, Scope* s, Target* target);
void cgen_assign(Node* n, Scope* s, Target* target);
void cgen_addit(Node* n, Scope* s, Target* target);
void cgen_var(Node* n, Scope* s, Target* target);
void cgen_sexpr(Node* n, Scope* s, Target* target);
void cgen_addop(Node* n, Scope* s, Target* target);
void cgen_term(Node* n, Scope* s, Target* target);
void cgen_num(Node* n, Scope* s, Target* target);
void cgen_factor(Node* n, Scope* s, Target* target);
