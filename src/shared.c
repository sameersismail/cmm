#include "shared.h"

const char* TOKEN_STRINGS[] = {
    // Bookkeeping values
    "END_FILE", "ERROR",
    // Keywords
    "IF", "ELSE", "INT", "RETURN", "VOID", "WHILE",
    // Special Symbols
    "PLUS", "MINUS", "TIMES", "DIV", "LESS", "LEQ", "GREAT", "GEQ", "EQUAL",
    "N_EQUAL", "ASSIGN", "SEMI_COL", "COMMA", "O_PAREN", "C_PAREN",
    "O_BRACK", "C_BRACK", "O_BRACE", "C_BRACE",
    // Multi-character tokens
    "NUM", "ID"
};


/**
 * Source:
 * https://stackoverflow.com/questions/14002954/
 * c-programming-how-to-read-the-whole-file-contents-into-a-buffer
 */
struct String read_whole_file(const char* filename)
{
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("File opening failed");
        return (struct String){
            .buffer = NULL,
            .size = 0
        };
    }

    fseek(f, 0, SEEK_END);

    long fsize = ftell(f);
    if (fsize == -1) {
        perror("Cannot obtain offset");
        fclose(f);
        return (struct String) {
            .buffer = NULL,
            .size = 0
        };
    }

    fseek(f, 0, SEEK_SET);

    char* string = malloc(fsize + 1);
    fread(string, 1, fsize, f);
    string[fsize] = '\0';

    fclose(f);

    return (struct String){
        .buffer = string,
        .size = fsize
    };
}
