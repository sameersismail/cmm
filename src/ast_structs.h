#pragma once

#include "ast_nodes.h"
#include "types.h"

typedef struct CompoundStatement {
    enum CompoundStatementKind compound_statement_kind;
} CompoundStatement;

typedef struct Variable {
    enum VariableKind variable_kind;
    enum Type type;
    int arr_len;
} Variable;

typedef struct Declaration {
    enum DeclarationKind declaration_kind;
    enum Type type;
    Variable* var;
} Declaration;

typedef struct Statement {
    enum StatementKind statement_kind;
} Statement;

typedef struct Parameter {
    enum ParameterKind parameter_kind;
    enum Type type;
    enum VariableKind variable_kind;
} Parameter;

typedef struct Expression {
    enum ExpressionKind expression_kind;
} Expression;

typedef struct SimpleExpression {
    enum SimpleExpressionKind simple_expression_kind;
} SimpleExpression;

typedef struct AdditiveExpression {
    enum AdditiveKind additive_kind;
} AdditiveExpression;

typedef struct Term {
    enum TermKind term_kind;
} Term;

typedef struct Factor {
    enum FactorKind factor_kind;
} Factor;

typedef struct Call {
    enum CallKind call_kind;
} Call;

typedef struct Arguments {
    enum ArgumentKind argument_kind;
} Arguments;
