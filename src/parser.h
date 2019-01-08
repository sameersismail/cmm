/**
 * Iterate over the token stream and produce an AST.
 */

#pragma once

#include "ast.h"
#include "shared.h"

Node* parse(Token* tokens, Input* input);
