#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "analyser.h"
#include "ast.h"
#include "parser.h"
#include "shared.h"
#include "symbol.h"

#include "cgen.h"
#include "generate.c"

/**
 * call => ID \( args \)
 */
void cgen_call(Node* n, Scope* s, Target* target)
{
    assert((n != NULL) && (s != NULL));
    gen_func_call(n, s, target);
}

/**
 * expression => var = expression | simple_expression
 */
void cgen_assign(Node* n, Scope* s, Target* target)
{
    assert((n != NULL) && (s != NULL));

    cgen_expr(n->child[1], s, target);
    gen_assign(n, s, target);
}

/**
 * var => ID | ID [expression]
 */
void cgen_var(Node* n, Scope* s, Target* target)
{
    assert((n != NULL) && (s != NULL));
    gen_var(n, s, target);
}

/**
 * additive_exp => term { addop term }
 */
void cgen_addit(Node* n, Scope* s, Target* target)
{
    assert((n != NULL) && (s != NULL));

    cgen_term(n->child[0], s, target);
    gen_addit_e1(n, target);

    cgen_term(n->child[1], s, target);
    gen_addit_e2(n, target, n->token_str);

    while (n->sibling != NULL) {
        cgen_addit(n->sibling, s, target);
    }
}

/**
 * simple_expression => additive_exp { relop additive_expr }
 */
void cgen_sexpr(Node* n, Scope* s, Target* target)
{
    assert((n != NULL) && (s != NULL));

    if (n->element.sexpr->simple_expression_kind == SEXPR_RELOP) {
        cgen_addop(n->child[0], s, target);
        gen_addit_e1(n, target);

        cgen_addop(n->child[1], s, target);
        gen_addit_e2(n, target, n->token_str);
    }
}

/**
 * additive_exp => term { addop term }
 */
void cgen_addop(Node* n, Scope* s, Target* target)
{
    assert((n != NULL) && (s != NULL));

    if (n->kind != NODE_ADDIT) {
        cgen_term(n, s, target);
    } else {
        cgen_term(n->child[0], s, target);
        gen_addit_e1(n, target);
        if (n->child[1]->kind == NODE_SEXPR) {
            cgen_sexpr(n->child[1], s, target);
        } else {
            cgen_term(n->child[1], s, target);
            gen_addit_e2(n, target, n->token_str);
        }
    }
}

/**
 * term => factor { mulop factor }
 */
void cgen_term(Node* n, Scope* s, Target* target)
{
    assert((n != NULL) && (s != NULL));

    if (n->kind != NODE_TERM) {
        cgen_factor(n, s, target);
    } else {
        cgen_factor(n->child[0], s, target);
        gen_addit_e1(n, target);
        cgen_factor(n->child[1], s, target);
        gen_addit_e2(n, target, n->token_str);
    }
}

/**
 * num => (0-9)*
 */
void cgen_num(Node* n, Scope* s, Target* target)
{
    assert((n != NULL) && (s != NULL));
    gen_num(n, target);
}

/**
 * factor => '(' expression ')' | var | call | NUM
 */
void cgen_factor(Node* n, Scope* s, Target* target)
{
    assert((n != NULL) && (s != NULL));

    switch (n->kind) {
        case NODE_EXPR:
            cgen_expr(n, s, target);
            break;
        case NODE_VAR:
            cgen_var(n, s, target);
            break;
        case NODE_CALL:
            cgen_call(n, s, target);
            break;
        case NODE_FACTOR:
            cgen_num(n, s, target);
            break;
        case NODE_TERM:
            cgen_term(n, s, target);
            break;
        case NODE_ADDIT:
            cgen_addit(n, s, target);
            break;
        default:
            printf("Error: cgen_factor()\n");
            exit(GENERATOR_ERROR);
    }
}

/**
 * expression => var = expression | simple_expression
 */
void cgen_expr(Node* n, Scope* s, Target* target)
{
    assert((n != NULL) && (s != NULL));

    switch (n->kind) {
        case NODE_STMT:
            if (n->child[0] != NULL) {
                cgen_expr(n->child[0], s, target);
            }
            break;
        case NODE_SEXPR:
            cgen_sexpr(n, s, target);
            break;
        case NODE_VAR:
            cgen_var(n, s, target);
            break;
        case NODE_EXPR:
            cgen_assign(n, s, target);
            break;
        case NODE_CALL:
            cgen_call(n, s, target);
            break;
        case NODE_FACTOR:
            cgen_factor(n, s, target);
            break;
        case NODE_ADDIT:
            cgen_addit(n, s, target);
            break;
        case NODE_TERM:
            cgen_term(n, s, target);
            break;
        default:
            printf("Error: cgen_expr()\n");
            exit(GENERATOR_ERROR);
    }
}

/**
 * selection_stmt => if \( expression \) statement |
 *					 if \( expression \) statement else statement
 */
void cgen_if(Node* n, Scope* s, Target* target)
{
    assert((n != NULL) && (s != NULL));
    gen_if(n, s, target);
}

/**
 * iteration_stmt => while \( expression \) statement
 */
void cgen_while(Node* n, Scope* s, Target* target)
{
    assert((n != NULL) && (s != NULL));
    gen_while(n, s, target);
}

/**
 * return_stmt => return [expression] ;
 */
void cgen_ret(Node* n, Scope* s, Target* target)
{
    assert((n != NULL) && (s != NULL));

    Symbol* f = get_func(&s);
    if (n->child[0] != NULL) {
        gen_return(n->child[0], s, target);
    }

    gen_return_exit(n, f, target);
}

/**
 * statement => expression_stmt | compound_stmt | selection_stmt |
 *				iteration_stmt | return_stmt
 */
void cgen_stmts(Node* n, Scope* s, Target* target)
{
    assert((n != NULL) && (s != NULL));

    while (n != NULL) {
        if (n->kind == NODE_STMT) {
            switch (n->element.stmt->statement_kind) {
                case STMT_EXPR:
                    cgen_expr(n, s, target);
                    break;
                case STMT_IF:
                    cgen_if(n, s, target);
                    break;
                case STMT_WHILE:
                    cgen_while(n, s, target);
                    break;
                case STMT_RETURN:
                    cgen_ret(n, s, target);
                    break;
                case STMT_NONE:
                default:
                    printf("Error: cgen_stmts()\n");
                    exit(GENERATOR_ERROR);
            }
        } else if (n->kind == NODE_CSTMT) {
            cgen_cstmt(n, s, target);
        }
        n = n->sibling;
    }
}

/**
 * local_declarations => { var_declaration }
 */
void cgen_decs(Node* n, Scope* s, Target* target)
{
    assert(s != NULL);

    // Start at 4 as $ra is already on the stack
    int offset = 4;
    while (n != NULL) {
        assert((n->element.decl->declaration_kind == DEC_VAR) &&
                (n->kind == NODE_DEC));

        Variable* var = n->element.decl->var;
        Symbol* local = init_symbol();
        *local = (Symbol) {
            .id = n->token_str,
            .len = var->arr_len,
            .cat = var->variable_kind == VAR_SINGLE ? CAT_VAR_SIN : CAT_VAR_ARR,
            .type = var->type,
            .local = true
        };

        local->offset = -offset;
        offset += local->cat == CAT_VAR_SIN ? 4 : local->len * 4;

        gen_func_locals(n, local, target);
        add_symbol(&s, local);
        n = n->sibling;
    }
}

/**
 * compound_stmt => \{ local_declarations statement_list \}
 */
void cgen_cstmt(Node* n, Scope* s, Target* target)
{
    assert((n != NULL) && (s != NULL));
    assert((n->element.cstmt->compound_statement_kind == CSTMT_MAIN) &&
            (n->kind == NODE_CSTMT));

    cgen_decs(n->child[0], s, target);
    cgen_stmts(n->child[1], s, target);
}

/**
 * param_list => param {, param }
 */
void cgen_params(Node* n, Scope* s, Target* target)
{
    assert((n != NULL) && (s != NULL));

    int offset = 4;
    while (n != NULL) {
        if (n->kind == NODE_PARAMS) {
            if (n->element.params->parameter_kind == PARAM_VOID) {
                break;
            } else {
                Symbol* local = init_symbol();
                *local = (Symbol) {
                    .id = n->token_str,
                    .local = true,
                    .cat = n->element.params->variable_kind == VAR_ARRAY ?
                            CAT_VAR_ARR :
                            CAT_VAR_SIN,
                    .type = n->element.params->type
                };

                local->offset = offset;
                offset += 4;
                add_symbol(&s, local);
            }
        }
        n = n->sibling;
    }
}

/**
 * program => {( var_declaration | fun_declaraiton )}
 */
void cgen(Node* n, Target* target)
{
    Scope* s = init_scope();
    enter_scope(&s);

    while (n != NULL) {
        assert(n->kind == NODE_DEC);

        if (n->element.decl->declaration_kind == DEC_VAR) {
            Variable* var = n->element.decl->var;

            Symbol* global_var = init_symbol();
            *global_var = (Symbol) {
                .id = n->token_str,
                .type = var->type,
                .cat = var->variable_kind == VAR_SINGLE ? CAT_VAR_SIN :
                                                          CAT_VAR_ARR,
                .len = var->arr_len,
                .local = false,
                .offset = 0
            };

            add_symbol(&s, global_var);
            gen_global_var(n, global_var, target);
        } else if (n->element.decl->declaration_kind == DEC_FUNC) {
            Symbol* global_func = init_symbol();
            *global_func = (Symbol) {
                .id = n->token_str,
                .cat = CAT_FUNC,
                .type = n->element.decl->type,
                .len = count_params(n->child[0]),
                .offset = count_local_space(n->child[1]->child[0])
            };

            add_symbol(&s, global_func);

            enter_scope(&s);
            cgen_params(n->child[0], s, target);

            if (!strcmp(n->token_str, "main")) {
                gen_main_entry(n, global_func, target);
            } else {
                gen_funcdef_entry(n, global_func, target);
            }

            cgen_cstmt(n->child[1], s, target);

            if (!strcmp(n->token_str, "main")) {
                gen_main_exit(n, global_func, target);
            } else {
                gen_funcdef_exit(n, global_func, target);
            }
            exit_scope(&s);
        }
        n = n->sibling;
    }

    exit_scope(&s);
}
