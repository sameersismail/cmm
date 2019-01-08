/**
 * Abstract Syntax Tree.
 */

#pragma once

#include "shared.h"

#include "ast_nodes.h"
#include "ast_structs.h"
#include "types.h"

#define MAX_CHILDREN 3

/* Data Structures */
typedef enum NodeKind {
    NODE_NONE,
    NODE_DEC,
    NODE_CSTMT,
    NODE_VAR,
    NODE_STMT,
    NODE_PARAMS,
    NODE_EXPR,
    NODE_SEXPR,
    NODE_ADDIT,
    NODE_TERM,
    NODE_FACTOR,
    NODE_CALL,
    NODE_ARGS,
} NodeKind;

typedef struct Node Node;
struct Node {
    NodeKind kind;

    union Elements {
        Declaration* decl;
        CompoundStatement* cstmt;
        Variable* var;
        Statement* stmt;
        Parameter* params;
        Expression* expr;
        SimpleExpression* sexpr;
        AdditiveExpression* addit;
        Term* term;
        Factor* factor;
        Call* call;
        Arguments* args;
    } element;

    Node* child[MAX_CHILDREN];
    Node* sibling;
    char* token_str;
};

/* Function prototypes */
Node* new_node(NodeKind kind);
