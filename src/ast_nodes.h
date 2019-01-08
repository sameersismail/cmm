#pragma once

enum DeclarationKind {
    DEC_NONE, 
    DEC_FUNC, 
    DEC_VAR
};

enum CompoundStatementKind { 
    CSTMT_NONE,
    CSTMT_MAIN,
    CSTMT_DECL,
    CSTMT_STMT
};

enum VariableKind { 
    VAR_NONE,
    VAR_SINGLE,
    VAR_ARRAY
};

enum StatementKind {
    STMT_NONE,
    STMT_EXPR,
    STMT_COMPOUND,
    STMT_IF,
    STMT_WHILE,
    STMT_RETURN
};

enum ParameterKind { 
    PARAM_NONE,
    PARAM_VOID,
    PARAM_LIST
};

enum ExpressionKind { 
    EXPR_NONE,
    EXPR_SIMPLE,
    EXPR_VAR
};

enum SimpleExpressionKind { 
    SEXPR_NONE,
    SEXPR_RELOP
};

enum AdditiveKind { 
    ADDIT_NONE,
    ADDIT_TERM,
    ADDIT_ADDOP
};

enum TermKind { 
    TERM_NONE,
    TERM_FACTOR,
    TERM_MULOP
};

enum FactorKind { 
    FAC_NONE,
    FAC_EXPR,
    FAC_VAR,
    FAC_CALL,
    FAC_NUM
};

enum CallKind { 
    CALL_NONE,
    CALL_EMPTY,
    CALL_ARGS
};

enum ArgumentKind { 
    ARG_NONE,
    ARG_EMPTY,
    ARG_ARGS
};
