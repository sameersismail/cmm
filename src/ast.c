#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Create new node conditional on the node's type.
 */
Node* new_node(NodeKind type)
{
    Node* node = calloc(sizeof(Node), 1);
    node->kind = type;
    int num_children = 0;

    switch (type) {
        case NODE_DEC:
            node->element.decl = calloc(sizeof(Declaration), 1);
            node->element.decl->var = calloc(sizeof(Variable), 1);
            num_children = 2;
            break;
        case NODE_CSTMT:
            node->element.cstmt = calloc(sizeof(CompoundStatement), 1);
            num_children = 2;
            break;
        case NODE_VAR:
            node->element.var = calloc(sizeof(Variable), 1);
            num_children = 1;
            break;
        case NODE_STMT:
            node->element.stmt = calloc(sizeof(Statement), 1);
            num_children = 3;
            break;
        case NODE_PARAMS:
            node->element.params = calloc(sizeof(Parameter), 1);
            num_children = 0;
            break;
        case NODE_EXPR:
            node->element.expr = calloc(sizeof(Expression), 1);
            num_children = 2;
            break;
        case NODE_SEXPR:
            node->element.sexpr = calloc(sizeof(SimpleExpression), 1);
            num_children = 2;
            break;
        case NODE_ADDIT:
            node->element.addit = calloc(sizeof(AdditiveExpression), 1);
            num_children = 2;
            break;
        case NODE_TERM:
            node->element.term = calloc(sizeof(Term), 1);
            num_children = 2;
            break;
        case NODE_FACTOR:
            node->element.factor = calloc(sizeof(Factor), 1);
            num_children = 0;
            break;
        case NODE_CALL:
            node->element.call = calloc(sizeof(Call), 1);
            num_children = 1;
            break;
        case NODE_ARGS:
            node->element.args = calloc(sizeof(Arguments), 1);
            num_children = 0;
            break;
        case NODE_NONE:
        default:
            printf("Error: Cannot create new node\n");
            exit(AST_ERROR);
    }

    // Conditionally allocate children
    for (int i = 0; i < num_children; ++i) {
        node->child[i] = calloc(sizeof(Node), 1);
    }

    return node;
}
