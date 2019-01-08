#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "lexer.h"
#include "shared.h"

/********** Parser routines. **********/
static Node* declaration_list(Input* input, Token* tokens);
static Node* declaration(Input* input, Token** tokens);
static Node* var_declaration(Input* input, Token** tokens);
static Node* func_declaration(Input* input, Token** tokens);

static enum Type type_specifier(Input* input, Token** tokens);

static Node* params(Input* input, Token** tokens);
static Node* param_list(Input* input, Token** tokens);
static Node* param(Input* input, Token** tokens);

static Node* local_declarations(Input* input, Token** tokens);

static Node* statement(Input* input, Token** tokens);
static Node* statement_list(Input* input, Token** tokens);
static Node* expression_stmt(Input* input, Token** tokens);
static Node* compound_stmt(Input* input, Token** tokens);
static Node* selection_stmt(Input* input, Token** tokens);
static Node* iteration_stmt(Input* input, Token** tokens);
static Node* return_stmt(Input* input, Token** tokens);

static Node* expression(Input* input, Token** tokens);

static Node* simple_expression(Input* input, Token** tokens);
static Node* additive_exp(Input* input, Token** tokens);
static char* relop(Input* input, Token** tokens);
static char* addop(Input* input, Token** tokens);
static char* mulop(Input* input, Token** tokens);
static Node* term(Input* input, Token** tokens);
static Node* factor(Input* input, Token** tokens);
static Node* call(Input* input, Token** tokens);
static Node* var(Input* input, Token** tokens);

static Node* args(Input* input, Token** tokens);
static Node* arg_list(Input* input, Token** tokens);

/********** Helper functions. **********/
static void unget_token(Input* input, Token** tokens);
static void get_token(Input* input, Token** tokens);
static void print_current_line(Input* input, Token* token);
static void match(Input* input, Token** token, Tokens expected);
static void print_error(Input* input, char* function);

/**
 * declaration_list => { declaration }
 */
static Node* declaration_list(Input* input, Token* tokens)
{
    Node* p = declaration(input, &tokens);
    Node* nc = p;

    while (tokens->token != END_FILE) {
        if (tokens->token == ERROR) {
            print_error(input, "declaration_list()");
            break;
        }
        p->sibling = declaration(input, &tokens);
        p = p->sibling;
    }

    return nc;
}

/**
 * declaration => var-declaration | fun-declaration
 */
static Node* declaration(Input* input, Token** tokens)
{
    Node* node = NULL;

    type_specifier(input, tokens);
    match(input, tokens, ID);

    Tokens chosen = (*tokens)->token;

    unget_token(input, tokens);
    unget_token(input, tokens);

    switch (chosen) {
        case SEMI_COL:
            node = var_declaration(input, tokens);
            break;
        case O_BRACK: 
            node = var_declaration(input, tokens);
            break;
        case O_PAREN:
            node = func_declaration(input, tokens);
            break;
        default:
            print_error(input, "declaration()");
            break;
    }

    return node;
}

/**
 * var_declaration => Type-specifier ID ; | Type-specifier ID [ NUM ] ;
 */
static Node* var_declaration(Input* input, Token** tokens)
{
    Node* node = new_node(NODE_DEC);
    node->element.decl->declaration_kind = DEC_VAR;
    node->element.decl->var->type = type_specifier(input, tokens);

    if ((*tokens)->token == ID) {
        node->token_str = (*tokens)->token_str;
        match(input, tokens, ID);
    }

    switch ((*tokens)->token) {
        case SEMI_COL:
            node->element.decl->var->variable_kind = VAR_SINGLE;
            match(input, tokens, SEMI_COL);
            break;
        case O_BRACK:
            node->element.decl->var->variable_kind = VAR_ARRAY;
            match(input, tokens, O_BRACK);

            if ((*tokens)->token == NUM) {
                node->element.decl->var->arr_len = atoi((*tokens)->token_str);
            } else {
                node->element.decl->var->arr_len = -1;
            }

            match(input, tokens, NUM);
            match(input, tokens, C_BRACK);
            match(input, tokens, SEMI_COL);
            break;
        default: 
            print_error(input, "var_declaration()");
            break;
    }

    return node;
}

/**
 * func_declaration => type_specifier ID ( params ) compound_stmt
 */
static Node* func_declaration(Input* input, Token** tokens)
{
    Node* node = new_node(NODE_DEC);
    node->element.decl->declaration_kind = DEC_FUNC;
    node->element.decl->type = type_specifier(input, tokens);

    if ((*tokens)->token == ID) {
        node->token_str = (*tokens)->token_str;
        match(input, tokens, ID);
    }

    match(input, tokens, O_PAREN);
    node->child[0] = params(input, tokens);
    match(input, tokens, C_PAREN);
    node->child[1] = compound_stmt(input, tokens);

    return node;
}

/**
 * type_specifier => int | void
 */
static enum Type type_specifier(Input* input, Token** tokens)
{
    enum Type type;

    switch ((*tokens)->token) {
        case INT:
            type = TYPE_INT;
            match(input, tokens, INT);
            break;
        case VOID:
            type = TYPE_VOID;
            match(input, tokens, VOID);
            break;
        default: 
            type = TYPE_NONE;
            print_error(input, "type_specifier()");
            break;
    }

    return type;
}

/**
 * params => param_list | void
 */
static Node* params(Input* input, Token** tokens)
{
    Node* node = NULL;

    if ((*tokens)->token == VOID) {
        match(input, tokens, VOID);

        if ((*tokens)->token == ID) {
            unget_token(input, tokens);
            node = param_list(input, tokens);
        } else {
            node = new_node(NODE_PARAMS);
            node->element.params->parameter_kind = PARAM_VOID;
            node->token_str = (*tokens)->token_str;
        }
    } else {
        node = param_list(input, tokens);
    }

    return node;
}

/**
 * param_list => param {, param }
 */
static Node* param_list(Input* input, Token** tokens)
{
    Node* node = NULL;

    node = param(input, tokens);
    node->element.params->parameter_kind = PARAM_LIST;
    Node* nc = node;

    while ((*tokens)->token == COMMA) {
        match(input, tokens, COMMA);
        node->sibling = param(input, tokens);
        node = node->sibling;
    }

    return nc;
}

/**
 * param => type_specifier ID [ ] | type_specifier ID
 */
static Node* param(Input* input, Token** tokens)
{
    Node* node = new_node(NODE_PARAMS);
    node->element.params->parameter_kind = PARAM_NONE;
    node->element.params->type = type_specifier(input, tokens);
    node->token_str = (*tokens)->token_str;

    match(input, tokens, ID);

    if ((*tokens)->token == O_BRACK) {
        match(input, tokens, O_BRACK);
        match(input, tokens, C_BRACK);
        node->element.params->variable_kind = VAR_ARRAY;
    } else {
        node->element.params->variable_kind = VAR_SINGLE;
    }

    return node;
}

/**
 * local_declarations => { var_declaration }
 */
static Node* local_declarations(Input* input, Token** tokens)
{
    Node* node = NULL;

    if ((*tokens)->token == INT || (*tokens)->token == VOID) {
        node = var_declaration(input, tokens);
    }

    Node* nc = node;
    while ((*tokens)->token == INT || (*tokens)->token == VOID) {
        node->sibling = var_declaration(input, tokens);
        node = node->sibling;
    }

    return nc;
}

/**
 * compound_stmt => \{ local_declarations statement_list \}
 */
static Node* compound_stmt(Input* input, Token** tokens)
{
    Node* node = new_node(NODE_CSTMT);
    node->element.cstmt->compound_statement_kind = CSTMT_MAIN;

    match(input, tokens, O_BRACE);

    node->child[0] = local_declarations(input, tokens);
    node->child[1] = statement_list(input, tokens);

    match(input, tokens, C_BRACE);

    return node;
}

/**
 * statement_list => { statements }
 */
static Node* statement_list(Input* input, Token** tokens)
{
    Node* node = NULL;

    if (((*tokens)->token == ID) || ((*tokens)->token == O_BRACE) ||
            ((*tokens)->token == IF) || ((*tokens)->token == WHILE) ||
            ((*tokens)->token == RETURN)) {
        node = statement(input, tokens);
    }

    Node* nc = node;
    while (((*tokens)->token == ID) || ((*tokens)->token == O_BRACE) ||
            ((*tokens)->token == IF) || ((*tokens)->token == WHILE) ||
            ((*tokens)->token == RETURN)) {
        node->sibling = statement(input, tokens);
        node = node->sibling;
    }

    return nc;
}

/**
 * statement => expression_stmt | compound_stmt | selection_stmt |
 *				iteration_stmt | return_stmt
 */
static Node* statement(Input* input, Token** tokens)
{
    Node* node = NULL;

    switch ((*tokens)->token) {
        case ID:
            node = expression_stmt(input, tokens);
            break;
        case O_BRACE:
            node = compound_stmt(input, tokens);
            break;
        case IF:
            node = selection_stmt(input, tokens);
            break;
        case WHILE:
            node = iteration_stmt(input, tokens);
            break;
        case RETURN:
            node = return_stmt(input, tokens);
            break;
        default:
            print_error(input, "statement()");
            break;
    }

    return node;
}

/**
 * expression_stmt => [expression] ;
 */
static Node* expression_stmt(Input* input, Token** tokens)
{
    Node* node = new_node(NODE_STMT);
    node->element.stmt->statement_kind = STMT_EXPR;

    if ((*tokens)->token == SEMI_COL) {
        node->child[0] = NULL;
        match(input, tokens, SEMI_COL);
    } else {
        node->child[0] = expression(input, tokens);
        match(input, tokens, SEMI_COL);
    }

    return node;
}

/**
 * selection_stmt => if \( expression \) statement |
 *					 if \( expression \) statement else statement
 */
static Node* selection_stmt(Input* input, Token** tokens)
{
    Node* node = new_node(NODE_STMT);
    node->element.stmt->statement_kind = STMT_IF;

    match(input, tokens, IF);
    match(input, tokens, O_PAREN);

    node->child[0] = expression(input, tokens);

    match(input, tokens, C_PAREN);

    node->child[1] = statement(input, tokens);

    if ((*tokens)->token == ELSE) {
        match(input, tokens, ELSE);
        node->child[2] = statement(input, tokens);
    } else {
        node->child[2] = NULL;
    }

    return node;
}

/**
 * iteration_stmt => while \( expression \) statement
 */
static Node* iteration_stmt(Input* input, Token** tokens)
{
    Node* node = new_node(NODE_STMT);
    node->element.stmt->statement_kind = STMT_WHILE;

    match(input, tokens, WHILE);
    match(input, tokens, O_PAREN);

    node->child[0] = expression(input, tokens);

    match(input, tokens, C_PAREN);

    node->child[1] = statement(input, tokens);

    return node;
}

/**
 * return_stmt => return [expression] ;
 */
static Node* return_stmt(Input* input, Token** tokens)
{
    Node* node = new_node(NODE_STMT);
    node->element.stmt->statement_kind = STMT_RETURN;

    match(input, tokens, RETURN);

    if ((*tokens)->token == SEMI_COL) {
        node->child[0] = NULL;
        match(input, tokens, SEMI_COL);
    } else {
        node->child[0] = expression(input, tokens);
        match(input, tokens, SEMI_COL);
    }

    return node;
}

/**
 * expression => var = expression | simple_expression
 */
static Node* expression(Input* input, Token** tokens)
{
    Node* node = new_node(NODE_EXPR);
    node->element.expr->expression_kind = EXPR_VAR;

    /* Disambiguating between an `expression` and a `simple_expression`
     * disallows an LL(1) parse and necessitates an LL(k) parse.
     * Hence the extreme messiness of the following: an arbitrary k tokens of
     * backtracking are required to properly disambiguate.
     */
    if (((*tokens)->token == NUM) || ((*tokens)->token == O_PAREN)) {
        node = simple_expression(input, tokens);
    } else if ((*tokens)->token == ID) {
        match(input, tokens, ID);

        if ((*tokens)->token == O_PAREN) {
            unget_token(input, tokens);
            node = call(input, tokens);
        } else {
            unget_token(input, tokens);
            node->child[0] = var(input, tokens);

            if ((*tokens)->token == ASSIGN) {
                node->token_str = (*tokens)->token_str;
                match(input, tokens, ASSIGN);
                node->child[1] = expression(input, tokens);
            } else {
                if (node->child[0]->element.var->variable_kind == VAR_ARRAY) {
                    while (strcmp((*tokens)->prev->token_str, "[")) {
                        unget_token(input, tokens);
                    }
                    unget_token(input, tokens);
                    unget_token(input, tokens);
                } else {
                    unget_token(input, tokens);
                }
                node = simple_expression(input, tokens);
            }
        }
    } else {
        print_error(input, "expression()");
    }

    return node;
}

/**
 * var => ID [ \[expression\] ]
 */
static Node* var(Input* input, Token** tokens)
{
    Node* node = new_node(NODE_VAR);
    node->token_str = (*tokens)->token_str;

    match(input, tokens, ID);

    if ((*tokens)->token == O_BRACK) {
        node->element.var->variable_kind = VAR_ARRAY;
        match(input, tokens, O_BRACK);
        node->child[0] = expression(input, tokens);
        match(input, tokens, C_BRACK);
    } else {
        node->child[0] = NULL;
        node->element.var->variable_kind = VAR_SINGLE;
    }

    return node;
}

/**
 * simple_expression => additive_exp | additive_exp relop additive_exp
 */
static Node* simple_expression(Input* input, Token** tokens)
{
    Node* node = new_node(NODE_SEXPR);
    node->element.sexpr->simple_expression_kind = SEXPR_NONE;

    node->child[0] = additive_exp(input, tokens);

    Tokens t = (*tokens)->token;
    if ((t == LESS) || (t == LEQ) || (t == GREAT) || (t == GEQ) ||
            (t == EQUAL) || (t == N_EQUAL)) {
        node->element.sexpr->simple_expression_kind = SEXPR_RELOP;
        node->token_str = relop(input, tokens);
        node->child[1] = additive_exp(input, tokens);
    } else {
        node = node->child[0];
    }

    return node;
}

/**
 * relop => <= | > | < | >= | == | !=
 */
static char* relop(Input* input, Token** tokens)
{
    char* rel = (*tokens)->token_str;

    switch ((*tokens)->token) {
        case LESS:
            match(input, tokens, LESS);
            break;
        case LEQ: 
            match(input, tokens, LEQ);
            break;
        case GREAT:
            match(input, tokens, GREAT);
            break;
        case GEQ:
            match(input, tokens, GEQ);
            break;
        case EQUAL:
            match(input, tokens, EQUAL);
            break;
        case N_EQUAL:
            match(input, tokens, N_EQUAL);
            break;
        default:
            print_error(input, "relop()");
            break;
    }

    return rel;
}

/**
 * additive_exp => term { addop term }
 */
static Node* additive_exp(Input* input, Token** tokens)
{
    Node* base = term(input, tokens);
    if (((*tokens)->token != PLUS) && ((*tokens)->token != MINUS)) {
        return base;
    }

    Node* node = new_node(NODE_ADDIT);

    node->element.addit->additive_kind = ADDIT_ADDOP;
    node->token_str = addop(input, tokens);
    node->child[0] = base;
    node->child[1] = term(input, tokens);

    Node* nc = node;

    while (((*tokens)->token == PLUS) || ((*tokens)->token == MINUS)) {
        Node* nd = new_node(NODE_ADDIT);
        nd->element.addit->additive_kind = ADDIT_ADDOP;
        nd->token_str = addop(input, tokens);

        Node* prev = node->child[1];

        node->child[1] = nd;
        node->child[1]->child[0] = prev;

        node->child[1]->child[1] = term(input, tokens);
        node = node->child[1];
    }

    return nc;
}

/**
 * addop => + | -
 */
static char* addop(Input* input, Token** tokens)
{
    char* add = (*tokens)->token_str;

    switch ((*tokens)->token) {
        case PLUS:
            match(input, tokens, PLUS);
            break;
        case MINUS:
            match(input, tokens, MINUS);
            break;
        default:
            print_error(input, "addop()");
            break;
    }

    return add;
}

/**
 * term => factor { mulop factor }
 */
static Node* term(Input* input, Token** tokens)
{
    Node* base = factor(input, tokens);
    if (((*tokens)->token != TIMES) && ((*tokens)->token != DIV)) {
        return base;
    }

    Node* node = new_node(NODE_TERM);
    node->element.term->term_kind = TERM_MULOP;
    node->token_str = mulop(input, tokens);
    node->child[0] = base;
    node->child[1] = factor(input, tokens);

    Node* nc = node;

    while (((*tokens)->token == TIMES) || ((*tokens)->token == DIV)) {
        Node* nd = new_node(NODE_TERM);
        nd->element.term->term_kind = TERM_MULOP;
        nd->token_str = mulop(input, tokens);

        Node* prev = node->child[1];

        node->child[1] = nd;
        node->child[1]->child[0] = prev;

        node->child[1]->child[1] = factor(input, tokens);

        node = node->child[1];
    }

    return nc;
}

/**
 * mulop => * | /
 */
static char* mulop(Input* input, Token** tokens)
{
    char* mul = (*tokens)->token_str;

    switch ((*tokens)->token) {
        case TIMES:
            match(input, tokens, TIMES);
            break;
        case DIV:
            match(input, tokens, DIV);
            break;
        default:
            print_error(input, "mulop()");
            break;
    }

    return mul;
}

/**
 * factor => \( expression \) | var | call | NUM
 */
static Node* factor(Input* input, Token** tokens)
{
    Node* node = NULL;

    switch ((*tokens)->token) {
        case O_PAREN:
            match(input, tokens, O_PAREN);
            node = expression(input, tokens);
            match(input, tokens, C_PAREN);
            break;
        case ID:
            match(input, tokens, ID);
            if ((*tokens)->token == O_PAREN) {
                unget_token(input, tokens);
                node = call(input, tokens);
            }
            else {
                unget_token(input, tokens);
                node = var(input, tokens);
            }
            break;
        case NUM:
            node = new_node(NODE_FACTOR);
            node->element.factor->factor_kind = FAC_NUM;
            node->token_str = (*tokens)->token_str;
            match(input, tokens, NUM);
            break;
        default:
            print_error(input, "factor()");
            break;
    }

    return node;
}

/**
 * call => ID \( args \)
 */
static Node* call(Input* input, Token** tokens)
{
    Node* node = new_node(NODE_CALL);
    node->token_str = (*tokens)->token_str;

    match(input, tokens, ID);
    match(input, tokens, O_PAREN);

    node->child[0] = args(input, tokens);

    match(input, tokens, C_PAREN);

    return node;
}

/**
 * args => [arg-list]
 */
static Node* args(Input* input, Token** tokens)
{
    Node* node = NULL;

    Tokens t = (*tokens)->token;
    if ((t == ID) || (t == O_PAREN) || (t == NUM)) {
        node = arg_list(input, tokens);
    }

    return node;
}

/**
 * arg_list => expression {, expression}
 */
static Node* arg_list(Input* input, Token** tokens)
{
    Node* node = NULL;

    node = expression(input, tokens);
    Node* nc = node;
    while ((*tokens)->token == COMMA) {
        match(input, tokens, COMMA);
        node->sibling = expression(input, tokens);
        node = node->sibling;
    }

    return nc;
}

/**
 * The entry method to the recursive descent parser.
 */
Node* parse(Token* tokens, Input* input)
{
    return declaration_list(input, tokens);
}

/********** Helper functions. **********/

static void match(Input* input, Token** token, Tokens expected)
{
    if ((*token)->token == expected) {
        get_token(input, token);
    }
    else {
        printf("[Error] Line/Col %d:%d\n\n", 
                (*token)->line_num,
                (*token)->col_num);

        print_current_line(input, *token);
        printf("%*s\n", (*token)->col_num, "^");
        printf("\nExpected: '%s' got '%s'\n", 
               TOKEN_STRINGS[expected],
               TOKEN_STRINGS[(*token)->token]);

        exit(PARSER_ERROR);
    }
}

static void get_token(Input* input, Token** tokens)
{
    Token* next_tok = (*tokens)->next;
    next_tok->prev = (*tokens);
    (*tokens)->next = next_tok;
    (*tokens) = next_tok;
}

static void unget_token(Input* input, Token** tokens)
{
    if ((*tokens)->prev == NULL) {
        printf("Error: Cannot unget first token\n");
        exit(PARSER_ERROR);
    }
    else {
        Token* prev_token = (*tokens)->prev;
        (*tokens) = prev_token;
    }
}

static void print_current_line(Input* input, Token* token)
{
    uint64_t new_pos = token->position;
    while (new_pos > 0 && input->source[new_pos] != '\n') {
        new_pos -= 1;
    }

    if (new_pos != 0) { new_pos += 1; }

    while (input->source[new_pos] != '\n' && input->source[new_pos] != EOF) {
        printf("%c", input->source[new_pos]);
        new_pos += 1;
    }
}

static void print_error(Input* input, char* function)
{
    printf("[Error] In '%s'. Line/Col: %d:%d\n", 
            function, 
            input->line_num,
            input->col_num);
}
