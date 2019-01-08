#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "parser.h"
#include "shared.h"
#include "symbol.h"

void analyse(Node* n);
void analyse_params(Node* n, Scope* s);
void analyse_cstmt(Node* n, Scope* s);
void analyse_decs(Node* n, Scope* s);
void analyse_stmts(Node* n, Scope* s);

void analyse_if(Node* n, Scope* s);
void analyse_expr(Node* n, Scope* s);
void analyse_while(Node* n, Scope* s);
void analyse_ret(Node* n, Scope* s);

void analyse_call(Node* n, Scope* s);
void analyse_assign(Node* n, Scope* s);
void analyse_addit(Node* n, Scope* s);
void analyse_var(Node* n, Scope* s);
void analyse_sexpr(Node* n, Scope* s);
void analyse_addop(Node* n, Scope* s);
void analyse_term(Node* n, Scope* s);
void analyse_num(Node* n, Scope* s);
void analyse_factor(Node* n, Scope* s);

/**
 * call => ID \( args \)
 */
void analyse_call(Node* n, Scope* s)
{
    assert((n != NULL) && (s != NULL));

    Symbol* var = init_symbol();
    *var = (Symbol) {
        .id = n->token_str,
        .cat = CAT_FUNC,
        .type = TYPE_NONE
    };

    if (find_symbol(&s, var) != true) {
        printf("Error: function '%s' called but not defined\n", n->token_str);
        exit(ANALYSER_ERROR);
    }

    Node* nc = n->child[0];
    while (nc != NULL) {
        analyse_expr(nc, s);
        nc = nc->sibling;
    }
}

/**
 * expression => var = expression | simple_expression
 */
void analyse_assign(Node* n, Scope* s)
{
    assert((n != NULL) && (s != NULL));

    analyse_var(n->child[0], s);
    analyse_expr(n->child[1], s);
}

/**
 * additive_exp => term { addop term }
 */
void analyse_addit(Node* n, Scope* s)
{
    assert((n != NULL) && (s != NULL));

    analyse_term(n->child[0], s);
    analyse_term(n->child[1], s);

    while (n->sibling != NULL) {
        analyse_addit(n->sibling, s);
    }
}

/**
 * var => ID | ID [expression]
 */
void analyse_var(Node* n, Scope* s)
{
    assert((n != NULL) && (s != NULL));

    if (n->element.var->variable_kind == VAR_SINGLE) {
        Symbol* var = init_symbol();
        *var = (Symbol) {
                .id = n->token_str,
                .cat = CAT_VAR_SIN,
                .type = TYPE_NONE
        };

        if (find_symbol(&s, var) != true) {
            printf("Error: id '%s' used but not declared\n", n->token_str);
            exit(ANALYSER_ERROR);
        }
    } else if (n->element.var->variable_kind == VAR_ARRAY) {
        Symbol* var = init_symbol();
        *var = (Symbol) {
            .id = n->token_str,
            .cat = CAT_VAR_ARR, 
            .type = TYPE_NONE
        };

        if (find_symbol(&s, var) != true) {
            printf("Error: id '%s' used but not declared\n", n->token_str);
            exit(ANALYSER_ERROR);
        }
        analyse_expr(n->child[0], s);
    }
}

/**
 * simple_expression => additive_exp | additive_exp relop additive_exp
 */
void analyse_sexpr(Node* n, Scope* s)
{
    assert((n != NULL) && (s != NULL));

    if (n->element.sexpr->simple_expression_kind == SEXPR_RELOP) {
        analyse_addop(n->child[0], s);
        analyse_addop(n->child[1], s);
    }
}

/**
 * additive_exp => term { addop term }
 */
void analyse_addop(Node* n, Scope* s)
{
    assert((n != NULL) && (s != NULL));
    
    if (n->kind != NODE_ADDIT) {
        analyse_term(n, s);
    } else {
        analyse_term(n->child[0], s);
        if (n->child[1]->kind == NODE_SEXPR) {
            analyse_sexpr(n->child[1], s);
        } else {
            analyse_term(n->child[1], s);
        }
    }
}

/**
 * term => factor { mulop factor }
 */
void analyse_term(Node* n, Scope* s)
{
    assert((n != NULL) && (s != NULL));

    if (n->kind != NODE_TERM) {
        analyse_factor(n, s);
    } else {
        analyse_factor(n->child[0], s);
        analyse_factor(n->child[1], s);
    }
}

/**
 * num => (0-9)*
 */
void analyse_num(Node* n, Scope* s)
{
    assert((n != NULL) && (s != NULL));
    // Already checked in the scanner
}

/**
 * factor => '(' expression ')' | var | call | NUM
 */
void analyse_factor(Node* n, Scope* s)
{
    assert((n != NULL) && (s != NULL));

    switch (n->kind) {
        case NODE_EXPR:
            analyse_expr(n, s);
            break;
        case NODE_VAR:
            analyse_var(n, s);
            break;
        case NODE_CALL:
            analyse_call(n, s);
            break;
        case NODE_FACTOR:
            analyse_num(n, s);
            break;
        case NODE_TERM:
            analyse_term(n, s);
            break;
        case NODE_ADDIT:
            analyse_addit(n, s);
            break;
        default:
            printf("Error: analyse_factor()\n");
            exit(ANALYSER_ERROR);
    }
}

/**
 * expression => var = expression | simple_expression
 */
void analyse_expr(Node* n, Scope* s)
{
    assert((n != NULL) && (s != NULL));

    switch (n->kind) {
        case NODE_STMT:
            if (n->child[0] != NULL) {
                analyse_expr(n->child[0], s);
            }
            break;
        case NODE_SEXPR:
            analyse_sexpr(n, s);
            break;
        case NODE_VAR:
            analyse_var(n, s);
            break;
        case NODE_EXPR:
            analyse_assign(n, s);
            break;
        case NODE_CALL:
            analyse_call(n, s);
            break;
        case NODE_FACTOR:
            analyse_factor(n, s);
            break;
        case NODE_ADDIT:
            analyse_addit(n, s);
            break;
        case NODE_TERM:
            analyse_term(n, s);
            break;
        default:
            printf("Error: analyse_expr()\n");
            exit(ANALYSER_ERROR);
    }
}

/**
 * selection_stmt => if \( expression \) statement |
 *					 if \( expression \) statement else statement
 */
void analyse_if(Node* n, Scope* s)
{
    assert((n != NULL) && (s != NULL));

    analyse_expr(n->child[0], s);
    analyse_stmts(n->child[1], s);

    if (n->child[2] != NULL) {
        analyse_stmts(n->child[2], s);
    }
}

/**
 * iteration_stmt => while \( expression \) statement
 */
void analyse_while(Node* n, Scope* s)
{
    assert((n != NULL) && (s != NULL));

    analyse_expr(n->child[0], s);
    analyse_stmts(n->child[1], s);
}

/**
 * return_stmt => return [expression] ;
 */
void analyse_ret(Node* n, Scope* s)
{
    assert((n != NULL) && (s != NULL));

    if (n->child[0] != NULL) {
        analyse_expr(n->child[0], s);
    }
}

/**
 * statement => expression_stmt | compound_stmt | selection_stmt |
 *				iteration_stmt | return_stmt
 */
void analyse_stmts(Node* n, Scope* s)
{
    assert((n != NULL) && (s != NULL));

    while (n != NULL) {
        if (n->kind == NODE_STMT) {
            switch (n->element.stmt->statement_kind) {
                case STMT_EXPR:
                    analyse_expr(n, s);
                    break;
                case STMT_IF:
                    analyse_if(n, s);
                    break;
                case STMT_WHILE:
                    analyse_while(n, s);
                    break;
                case STMT_RETURN:
                    analyse_ret(n, s);
                    break;
                case STMT_NONE:
                default:
                    printf("Error: analyse_stmts()\n");
                    exit(ANALYSER_ERROR);
            }
        } else if (n->kind == NODE_CSTMT) {
            analyse_cstmt(n, s);
        }
        n = n->sibling;
    }
}

/**
 * local_declarations => { var_declaration }
 */
void analyse_decs(Node* n, Scope* s)
{
    assert(s != NULL);

    while (n != NULL) {
        assert((n->element.decl->declaration_kind == DEC_VAR) &&
                (n->kind == NODE_DEC));

        Variable* var = n->element.decl->var;
        Symbol* global = init_symbol();

        global->id = n->token_str;
        global->cat =
                var->variable_kind == VAR_SINGLE ? CAT_VAR_SIN : CAT_VAR_ARR;

        if ((global->cat == CAT_VAR_ARR) && (var->arr_len == 0)) {
            printf("Error: variable's array size '%d' illegal\n", var->arr_len);
            exit(ANALYSER_ERROR);
        }

        if (var->type != TYPE_INT) {
            printf("Error: variable's type must be of type int\n");
            exit(ANALYSER_ERROR);
        }

        global->type = var->type;
        add_symbol(&s, global);
        n = n->sibling;
    }
}

/**
 * compound_stmt => \{ local_declarations statement_list \}
 */
void analyse_cstmt(Node* n, Scope* s)
{
    assert((n != NULL) && (s != NULL));
    assert((n->kind == NODE_CSTMT) &&
            (n->element.cstmt->compound_statement_kind == CSTMT_MAIN));

    analyse_decs(n->child[0], s);
    analyse_stmts(n->child[1], s);
}

/**
 * param_list => param {, param }
 */
void analyse_params(Node* n, Scope* s)
{
    assert((n != NULL) && (s != NULL));

    while (n != NULL) {
        if (n->kind == NODE_PARAMS) {
            if (n->element.params->parameter_kind == PARAM_VOID) {
                break;
            } else {
                Symbol* param = init_symbol();
                param->id = n->token_str;
                param->cat = n->element.params->variable_kind == VAR_ARRAY ?
                        CAT_VAR_ARR :
                        CAT_VAR_SIN;
                if (n->element.params->type != TYPE_INT) {
                    printf("Error: function parameters must be of type int\n");
                    exit(ANALYSER_ERROR);
                }
                param->type = n->element.params->type;
                add_symbol(&s, param);
            }
        }
        n = n->sibling;
    }
}

void check_decs(Node* n)
{
    if (n->sibling == NULL) {
        if (!((n->element.decl->declaration_kind == DEC_FUNC) &&
                    (!strcmp(n->token_str, "main")) &&
                    (n->element.decl->type == TYPE_VOID) &&
                    (n->child[0]->element.params->parameter_kind ==
                            PARAM_VOID))) {
            printf("Error: last declaration must be 'void main(void)'\n");
            exit(ANALYSER_ERROR);
        }
    }
}

/**
 * Inserts prefined symbols into the symbol table.
 */
void init_symtab(Scope* s)
{
    char* predefined_keywords[] = {
            "else", "int", "return", "void", "while", "if"};
    int num_keywords = 6;
    for (int i = 0; i < num_keywords; ++i) {
        Symbol* keyword = init_symbol();
        *keyword = (Symbol) {
            .id = predefined_keywords[i],
            .cat = CAT_NONE,
            .type = TYPE_NONE,
            .next = NULL,
            .prev = NULL
        };
        add_symbol(&s, keyword);
    }

    char* predefined_functions[] = {"input", "output"};
    int num_functions = 2;
    for (int i = 0; i < num_functions; ++i) {
        Symbol* keyw_input = init_symbol();
        *keyw_input = (Symbol) {
            .id = predefined_functions[i],
            .cat = CAT_FUNC,
            .type = TYPE_INT,
            .next = NULL,
            .prev = NULL
        };
        add_symbol(&s, keyw_input);
    }
}

/**
 * program => {( var_declaration | fun_declaraiton )}
 */
void analyse(Node* n)
{
    Scope* s = init_scope();
    enter_scope(&s);

    if (n == NULL) {
        printf("Error: program has no declarations\n");
        exit(ANALYSER_ERROR);
    }

    init_symtab(s);
    while (n != NULL) {
        assert(n->kind == NODE_DEC);
        check_decs(n);
        if (n->element.decl->declaration_kind == DEC_VAR) {
            Variable* var = n->element.decl->var;
            Symbol* global = init_symbol();

            global->id = n->token_str;
            global->cat = var->variable_kind == VAR_SINGLE ? CAT_VAR_SIN :
                                                             CAT_VAR_ARR;

            if ((global->cat == CAT_VAR_ARR) && (var->arr_len == 0)) {
                printf("Error: variable's array size '%d' illegal\n",
                        var->arr_len);
                exit(ANALYSER_ERROR);
            }
            if (var->type != TYPE_INT) {
                printf("Error: variable's type must be of type int\n");
                exit(ANALYSER_ERROR);
            }

            global->type = var->type;
            global->local = false;

            add_symbol(&s, global);
        } else if (n->element.decl->declaration_kind == DEC_FUNC) {
            Symbol* global_func = init_symbol();
            *global_func = (Symbol) {
                .id = n->token_str,
                .cat = CAT_FUNC,
                .type = n->element.decl->type
            };
            add_symbol(&s, global_func);

            enter_scope(&s);
            analyse_params(n->child[0], s);
            analyse_cstmt(n->child[1], s);
            exit_scope(&s);
        }
        n = n->sibling;
    }
    exit_scope(&s);
}
