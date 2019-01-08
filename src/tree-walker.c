/**
 * Traverse and print out the AST.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "shared.h"

void print_factor(Node* n);
void print_term(Node* n);
void print_addop(Node* n);
void print_sexpr(Node* n);
void print_var(Node* n);
void print_addit(Node* n);
void print_assign(Node* n);
void print_call(Node* n);
void print_expr(Node* n);

void print_while(Node* n);
void print_ret(Node* n);
void print_if(Node* n);
void print_stmts(Node* n);

void print_decs(Node* n);
void print_cstmt(Node* n);
void print_params(Node* n);
void print_all(Node* n);

/**
 * factor => '(' expression ')' | var | call | NUM
 */
void print_factor(Node* n)
{
    switch (n->kind) {
        case NODE_EXPR:
            print_expr(n);
            break;
        case NODE_VAR:
            print_var(n);
            break;
        case NODE_CALL:
            print_call(n);
            break;
        case NODE_FACTOR:
            printf("%s", n->token_str);
            break;
        case NODE_TERM:
            print_term(n);
            break;
        case NODE_ADDIT:
            print_addit(n);
            break;
        default:
            printf("Error: print_factor()\n");
            exit(AST_ERROR);
    }
}

/**
 * term => factor { mulop factor }
 */
void print_term(Node* n)
{
    if (n->kind != NODE_TERM) {
        print_factor(n);
    } else {
        print_factor(n->child[0]);
        printf("%s", n->token_str);
        print_factor(n->child[1]);
    }
}

/**
 * additive_exp => term { addop term }
 */
void print_addop(Node* n)
{
    if (n->kind != NODE_ADDIT) {
        print_term(n);
    } else {
        print_term(n->child[0]);
        printf("%s", n->token_str);
        if (n->child[1]->kind == NODE_SEXPR) {
            print_sexpr(n->child[1]);
        } else {
            print_term(n->child[1]);
        }
    }
}

/**
 * simple_expression => additive_exp | additive_exp relop additive_exp
 */
void print_sexpr(Node* n)
{
    if (n->element.sexpr->simple_expression_kind == SEXPR_RELOP) {
        if (n->child[0]->element.addit->additive_kind == ADDIT_ADDOP) {
            print_addop(n->child[0]);
        } else {
            printf("%s", n->child[0]->token_str);
        }

        printf(" %s ", n->token_str);

        if (n->child[1]->element.addit->additive_kind == ADDIT_ADDOP) {
            print_addop(n->child[1]);
        } else {
            printf("%s", n->child[1]->token_str);
        }
    }
}

/**
 * var => ID | ID [expression]
 */
void print_var(Node* n)
{
    if (n->element.var->variable_kind == VAR_SINGLE) {
        printf("%s", n->token_str);
    } else if (n->element.var->variable_kind == VAR_ARRAY) {
        printf("%s[", n->token_str);
        print_expr(n->child[0]);
        printf("]\n");
    }
}

/**
 * additive_exp => term { addop term }
 */
void print_addit(Node* n)
{
    print_term(n->child[0]);
    printf("%s", n->token_str);
    print_term(n->child[1]);

    if (n->sibling != NULL) {
        print_addit(n->sibling);
    }
}

/**
 * expression => var = expression | simple_expression
 */
void print_assign(Node* n)
{
    print_var(n->child[0]);
    printf(" = ");

    print_expr(n->child[1]);
    printf(";\n");
}

/**
 * call => ID \( args \)
 */
void print_call(Node* n)
{
    printf("%s(", n->token_str);

    Node* nc = n->child[0];
    if (nc == NULL) {
        printf("");
    }

    while (nc != NULL) {
        print_expr(nc);
        nc->sibling == NULL ? printf("") : printf(", ");
        nc = nc->sibling;
    }

    printf(")");
}

/**
 * expression => var = expression | simple_expression
 */
void print_expr(Node* n)
{
    switch (n->kind) {
        case NODE_STMT:
            if (n->child[0] != NULL) {
                print_expr(n->child[0]);
            }
            break;
        case NODE_SEXPR:
            print_sexpr(n);
            break;
        case NODE_VAR:
            print_var(n);
            break;
        case NODE_EXPR:
            print_assign(n);
            break;
        case NODE_CALL:
            print_call(n);
            break;
        case NODE_FACTOR:
            print_factor(n);
            break;
        case NODE_ADDIT:
            print_addit(n);
            break;
        default:
            printf("Error: print_expr()");
            exit(PARSER_ERROR);
    }
}

/**
 * iteration_stmt => while \( expression \) statement
 */
void print_while(Node* n)
{
    printf("while (");
    print_expr(n->child[0]);

    printf(") {\n");
    print_stmts(n->child[1]);

    printf("\n}\n");
}

/**
 * return_stmt => return [expression] ;
 */
void print_ret(Node* n)
{
    printf("return ");

    if (n->child[0] != NULL) {
        print_expr(n->child[0]);
    }

    printf(";");
}

/**
 * selection_stmt => if \( expression \) statement |
 *					 if \( expression \) statement else statement
 */
void print_if(Node* n)
{
    printf("if(");
    print_expr(n->child[0]);

    printf(") {\n");
    print_stmts(n->child[1]);

    printf("\n}\n");

    if (n->child[2] != NULL) {
        printf("else {\n");
        print_stmts(n->child[2]);
    }

    printf("\n}");
}

/**
 * statement => expression_stmt | compound_stmt | selection_stmt |
 *				iteration_stmt | return_stmt
 */
void print_stmts(Node* n)
{
    while (n != NULL) {
        if (n->kind == NODE_STMT) {
            switch (n->element.stmt->statement_kind) {
                case STMT_EXPR:
                    print_expr(n);
                    break;
                case STMT_IF:
                    print_if(n);
                    break;
                case STMT_WHILE:
                    print_while(n);
                    break;
                case STMT_RETURN:
                    print_ret(n);
                    break;
                case STMT_NONE:
                default:
                    printf("Error: print_stmts()");
                    exit(PARSER_ERROR);
            }
        } else if (n->kind == NODE_CSTMT) {
            print_cstmt(n);
        }
        n = n->sibling;
    }
}

/**
 * local_declarations => { var_declaration }
 */
void print_decs(Node* n)
{
    while (n != NULL) {
        if ((n->kind == NODE_DEC) &&
                (n->element.decl->declaration_kind == DEC_VAR)) {

            printf("%s %s",
                    n->element.decl->var->type == TYPE_INT ? "int" : "void",
                    n->token_str);

            n->element.decl->var->arr_len == 0 ?
                    printf("") :
                    printf("[%d]", n->element.decl->var->arr_len);

            printf(";\n");
        }
        n = n->sibling;
    }
}

/**
 * compound_stmt => \{ local_declarations statement_list \}
 */
void print_cstmt(Node* n)
{
    assert((n->kind == NODE_CSTMT) &&
            (n->element.cstmt->compound_statement_kind == CSTMT_MAIN));
    print_decs(n->child[0]);
    print_stmts(n->child[1]);
}

/**
 * param_list => param {, param }
 */
void print_params(Node* n)
{
    assert(n != NULL);

    while (n != NULL) {
        if (n->kind == NODE_PARAMS) {
            if (n->element.params->parameter_kind == PARAM_VOID) {
                printf("void");
            } else {
                printf("%s %s%s",
                        n->element.params->type == TYPE_INT ? "int" : "void",
                        n->token_str,
                        n->element.params->variable_kind == VAR_ARRAY ? "[]" :
                                                                        "");
            }
            if (n->sibling != NULL) {
                printf(", ");
            }
        }
        n = n->sibling;
    }
}

/**
 * program => {( var_declaration | fun_declaraiton )}
 */
void print_all(Node* n)
{
    while (n != NULL) {
        if ((n->kind == NODE_DEC) &&
                (n->element.decl->declaration_kind == DEC_FUNC)) {
            printf("%s", n->element.decl->type == TYPE_INT ? "int" : "void");
            printf(" %s(", n->token_str);

            print_params(n->child[0]);

            printf(")\n");
            printf("{\n");

            print_cstmt(n->child[1]);

            printf("\n}\n");
        } else if ((n->kind == NODE_DEC) &&
                (n->element.decl->declaration_kind == DEC_VAR)) {
            Variable* v = n->element.decl->var;
            v->type == TYPE_INT ? printf("int") : printf("void");

            printf(" %s", n->token_str);

            v->variable_kind == VAR_SINGLE ? printf("") : printf("[]");
            printf(";\n");
        }
        printf("\n");
        n = n->sibling;
    }
}
