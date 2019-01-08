/**
 * Tokenise the input stream.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "shared.h"

static Token* iterate(Input* in);
static char get_char(Input* inp);
static void unget_char(Input* inp);

Token* lex(Input* input)
{
    Token* tokens = iterate(input);
    Token* alias = tokens;

    while (tokens->token != END_FILE) {
        Token* new_token = iterate(input);
        tokens->next = new_token;
        tokens = new_token;
    }

    return alias;
}

/**
 * Iterate over the character stream, construct and return a single token
 * at a time.
 */
static Token* iterate(Input* in)
{
    char token_str[MAX_TOKEN_SIZE + 1];
    enum State state = START;
    Tokens token;
    bool save = true;
    int token_index = 0;
    char c = 0;

    while (state != DONE) {
        c = get_char(in);
        save = true;
        switch (state) {
            case START:
                if (isdigit(c)) {
                    state = IN_NUM;
                }
                else if (isalpha(c)) {
                    state = IN_ID;
                }
                else if (c == '=') {
                    state = IN_ASSIGN;
                }
                else if (c == '\n') {
                    save = false;
                    in->line_num += 1;
                    in->col_num = 0;
                }
                else if (isspace(c)) {
                    save = false;
                }
                else if (c == '/') {
                    save = false;
                    state = IN_DIV;
                }
                else {
                    state = DONE;
                    switch (c) {
                        case EOF:
                            save = false;
                            token = END_FILE;
                            break;
                        case '+': token = PLUS; break;
                        case '-': token = MINUS; break;
                        case '*': token = TIMES; break;
                        case '<': token = LESS; break;
                        case '>': token = GREAT; break;
                        case ',': token = COMMA; break;
                        case ';': token = SEMI_COL; break;
                        case '(': token = O_PAREN; break;
                        case ')': token = C_PAREN; break;
                        case '[': token = O_BRACK; break;
                        case ']': token = C_BRACK; break;
                        case '{': token = O_BRACE; break;
                        case '}': token = C_BRACE; break;
                        default: token = ERROR; break;
                    }
                }
                break;
            case IN_ID:
                if (!isalpha(c)) {
                    unget_char(in);
                    save = false;
                    state = DONE;
                    token = ID;
                }
                break;
            case IN_NUM:
                if (!isdigit(c)) {
                    unget_char(in);
                    save = false;
                    state = DONE;
                    token = NUM;
                }
                break;
            case IN_ASSIGN:
                state = DONE;
                if (c == '=') {
                    token = EQUAL;
                }
                else {
                    unget_char(in);
                    save = false;
                    token = ASSIGN;
                }
                break;
            case IN_DIV:
                if (c == '/') {
                    save = false;
                    state = IN_COMMENT;
                }
                else {
                    state = DONE;
                    unget_char(in);
                    save = true;
                    token = DIV;
                    c = '/';
                }
                break;
            case IN_COMMENT:
                save = false;
                if (c == '\n') {
                    state = START;
                }
                break;
            default: state = DONE; token = ERROR;
        }

        if ((save) && (token_index < MAX_TOKEN_SIZE)) {
            token_str[token_index++] = c;
        }

        if (state == DONE) {
            token_str[token_index] = '\0';
            if (token == ID) {
                if (!strncmp(token_str, "if\0", 3)) {
                    token = IF;
                } else if (!strncmp(token_str, "else\0", 5)) {
                    token = ELSE;
                } else if (!strncmp(token_str, "int\0", 4)) {
                    token = INT;
                } else if (!strncmp(token_str, "return\0", 7)) {
                    token = RETURN;
                } else if (!strncmp(token_str, "void\0", 5)) {
                    token = VOID;
                } else if (!strncmp(token_str, "while\0", 6)) {
                    token = WHILE;
                }
            }
        }
    }

    Token* token_class = calloc(sizeof(Token), 1);
    *token_class = (Token){
            .next = NULL,
            .prev = NULL,
            .line_num = in->line_num,
            .col_num = in->col_num,
            .position = in->position,
            .token = token,
    };
    strcpy(token_class->token_str, token_str);

    return token_class;
}

/**
 * Return the next character from the input stream.
 */
static char get_char(Input* inp)
{
    if (inp->source[inp->position] == '\0') {
        return EOF;
    } else {
        inp->col_num += 1;
        return inp->source[inp->position++];
    }
}

/**
 * Move the input stream backwards one character, within a single line.
 */
static void unget_char(Input* inp)
{
    if (inp->position != 0) {
        inp->position -= 1;
        inp->col_num -= 1;
    } else {
        // Error. Will propagate into parser.
    }
}
