/**
 * Tokenise the input stream.
 */

#pragma once

#include "shared.h"

enum State { 
    START, 
    IN_ID, 
    IN_NUM, 
    IN_ASSIGN, 
    IN_DIV, 
    IN_COMMENT, 
    IN_EQ, 
    DONE 
};

Token* lex(Input* inp);
