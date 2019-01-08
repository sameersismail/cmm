#pragma once

#include <stdio.h>
#include <stdlib.h>

#define MAX_TOKEN_SIZE 20

enum Error {
    ARGC_ERROR = 1,
    PARSER_ERROR,
    AST_ERROR,
    ANALYSER_ERROR,
    GENERATOR_ERROR
};

typedef enum Tokens {
    // Bookkeeping values
    END_FILE,
    ERROR,
    // Keywords
    IF,
    ELSE,
    INT,
    RETURN,
    VOID,
    WHILE,
    // Special symbols
    PLUS,
    MINUS,
    TIMES,
    DIV,
    LESS,
    LEQ,
    GREAT,
    GEQ,
    EQUAL,
    N_EQUAL,
    ASSIGN,
    SEMI_COL,
    COMMA,
    O_PAREN,
    C_PAREN,
    O_BRACK,
    C_BRACK,
    O_BRACE,
    C_BRACE,
    // Multi-character tokens
    NUM,
    ID
} Tokens;

typedef struct Input {
    char* source;      // Entire source file
    uint64_t length;   // Length of source
    uint64_t position; // Character position in file

    uint32_t line_num;
    uint32_t col_num;
} Input;

typedef struct Token {
    Tokens token;
    char token_str[MAX_TOKEN_SIZE + 1];

    uint64_t position;
    int line_num;
    int col_num;

    struct Token* next;
    struct Token* prev;
} Token;

struct String {
    char* buffer;
    uint64_t size;
};

extern const char* TOKEN_STRINGS[];

/* Functions */
struct String read_whole_file(const char* filename);
