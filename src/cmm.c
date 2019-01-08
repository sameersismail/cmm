#include <stdio.h>
#include <stdlib.h>

#include "analyser.h"
#include "ast.h"
#include "cgen.h"
#include "lexer.h"
#include "parser.h"
#include "symbol.h"
#include "shared.h"

#include "tree-walker.c"

#define INPUT_FILE_POS   1
#define OUTPUT_FILE_POS  3
#define DEFAULT_OUT_NAME "a.out"

void run(Input* input, Target* output)
{
    Token* tokens = lex(input);
    Node* ast = parse(tokens, input);
    analyse(ast);
    cgen(ast, output);
}

int main(int argc, char* argv[])
{
    if (!(argc == 2 || argc == 4)) {
        printf("Usage: cmm <filename> [-o <output>]\n");
        exit(ARGC_ERROR);
    }

    struct String program_text = read_whole_file(argv[INPUT_FILE_POS]);
    if (program_text.buffer == NULL) {
        return EXIT_FAILURE;
    }

    char* output_filename = argc == 4 ?
                            argv[OUTPUT_FILE_POS] : DEFAULT_OUT_NAME;

    FILE* fd = fopen(output_filename, "w");
    if (!fd) {
        perror("File opening failed");
        return EXIT_FAILURE;
    }

    Input* input = calloc(sizeof(Input), 1);
    *input = (Input) {
        .source = program_text.buffer,
        .length = program_text.size,
        .position = 0,
        .line_num = 1,
        .col_num = 1,
    };

    Target* output = calloc(sizeof(Target), 1);
    *output = (Target) {
        .filename = output_filename,
        .out = fd,
        .in_code = true,
        .label_count = 0
    };

    run(input, output);

    return EXIT_SUCCESS;
}
