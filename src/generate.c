/**
 * Produce MIPS.
 */

#include "cgen.h"

void gen_input_function(Target* target)
{
    fprintf(target->out, "\n%s:\n", "input");
    fprintf(target->out, "li     $v0, 5\n");
    fprintf(target->out, "syscall\n");
    fprintf(target->out, "move   $a0, $v0\n");
    fprintf(target->out, "jr     $ra\n");
}

void gen_output_function(Target* target)
{
    fprintf(target->out, "\n%s:\n", "output");
    fprintf(target->out, "li     $v0, 1\n");
    fprintf(target->out, "syscall\n");
    fprintf(target->out, "jr     $ra\n");
}

void gen_func_call(Node* n, Scope* s, Target* target)
{
    if (!strcmp(n->token_str, "output") || !strcmp(n->token_str, "input")) {
        fprintf(target->out, "jal    %s\n", n->token_str);
        return;
    }
    
    fprintf(target->out, "sw     $fp, 0($sp)\n");
    fprintf(target->out, "addiu  $sp, $sp, -4\n");

    // Reverse the singly-linked list
    Node* nc = n->child[0];
    Node* new_root = NULL;
    while (nc) {
        Node* next = nc->sibling;
        nc->sibling = new_root;
        new_root = nc;
        nc = next;
    }

    // Evaluate arguments and push them onto the stack
    while (new_root != NULL) {
        cgen_expr(new_root, s, target);
        fprintf(target->out, "sw     $a0, 0($sp)\n");
        fprintf(target->out, "addiu  $sp, $sp, -4\n");
        new_root = new_root->sibling;
    }

    fprintf(target->out, "jal    %s\n", n->token_str);
}

void gen_return(Node* n, Scope* s, Target* target)
{
    cgen_expr(n, s, target);
}

void gen_return_exit(Node* n, Symbol* s, Target* target)
{
    fprintf(target->out, "j      %s_exit\n", s->id);
}

void gen_funcdef_entry(Node* n, Symbol* sym, Target* target)
{
    if (target->in_code == false) {
        fprintf(target->out, ".text\n");
        target->in_code = true;
    }

    fprintf(target->out, "\n%s:\n", n->token_str);
    fprintf(target->out, "move   $fp, $sp\n");
    fprintf(target->out, "sw     $ra, 0($sp)\n");
    fprintf(target->out, "addiu  $sp, $sp, -4\n");
    fprintf(target->out, "\n");
}

void gen_funcdef_exit(Node* n, Symbol* sym, Target* target)
{
    fprintf(target->out, "%s_exit:\n", sym->id);
    fprintf(target->out, "addiu  $sp, $sp, %d\n", sym->offset);
    fprintf(target->out, "lw     $ra, 0($sp)\n");
    fprintf(target->out, "addiu  $sp, $sp, %d\n", (sym->len + 1) * 4);
    fprintf(target->out, "lw     $fp, 4($sp)\n");
    fprintf(target->out, "jr     $ra\n");
}

void gen_if(Node* n, Scope* s, Target* target)
{
    cgen_expr(n->child[0], s, target);
    fprintf(target->out, "bne    $a0, $zero, true_branch%d\n",
            target->label_count);
    fprintf(target->out, "%s%d:\n", "false_branch", target->label_count);

    if (n->child[2] != NULL) {
        cgen_stmts(n->child[2], s, target);
    }

    fprintf(target->out, "b      %s%d\n", "end_if", target->label_count);
    fprintf(target->out, "%s%d:\n", "true_branch", target->label_count);

    cgen_stmts(n->child[1], s, target);

    fprintf(target->out, "%s%d:\n", "end_if", target->label_count);
    target->label_count += 1;
}

void gen_while(Node* n, Scope* s, Target* target)
{
    fprintf(target->out, "%s%d:\n", "while_start", target->label_count);

    cgen_expr(n->child[0], s, target);

    fprintf(target->out, "beq    $a0, $zero, while_end%d\n", target->label_count);

    cgen_stmts(n->child[1], s, target);

    fprintf(target->out, "b      %s%d\n", "while_start", target->label_count);
    fprintf(target->out, "%s%d:\n", "while_end", target->label_count);

    target->label_count += 1;
}

void gen_var(Node* n, Scope* s, Target* target)
{
    Symbol* var = get_sym(&s, n->token_str);

    // Locals are accessed relative to the $fp, globals are accessed 
    // relative to the variable's global address.
    if (var->local == false) {
        if (var->cat == CAT_VAR_SIN) {
            fprintf(target->out, "la     $t8, %s\n", var->id);
            fprintf(target->out, "lw     $a0, 0($t8)\n");
        } else {
            fprintf(target->out, "la     $t8, %s\n", var->id);

            cgen_expr(n->child[0], s, target);

            fprintf(target->out, "li     $t9, 4\n");
            fprintf(target->out, "mul    $a0, $a0, $t9\n");
            fprintf(target->out, "add    $t8, $t8, $a0\n");
            fprintf(target->out, "lw     $a0, 0($t8)\n");
        }
    } else {
        if (var->cat == CAT_VAR_SIN) {
            fprintf(target->out, "lw     $a0, %d($fp)\n", var->offset);
        } else {
            fprintf(target->out, "move   $t8, $fp\n");
            fprintf(target->out, "addiu  $t8, $t8, %d\n", var->offset);

            cgen_expr(n->child[0], s, target);

            fprintf(target->out, "li     $t9, 4\n");
            fprintf(target->out, "mul    $a0, $a0, $t9\n");
            fprintf(target->out, "sub    $t8, $t8, $a0\n");
            fprintf(target->out, "lw     $a0, 0($t8)\n");
        }
    }
}

void gen_assign(Node* n, Scope* s, Target* target)
{
    fprintf(target->out, "sw     $a0, 0($sp)\n");
    fprintf(target->out, "addiu  $sp, $sp, -4\n");

    n = n->child[0];

    Symbol* var = get_sym(&s, n->token_str);
    if (var->local == false) {
        if (var->cat == CAT_VAR_SIN) {
            fprintf(target->out, "la     $t8, %s\n", var->id);
            fprintf(target->out, "sw     $a0, %s($t8)\n", "0");
        } else {
            fprintf(target->out, "la     $t8, %s\n", var->id);

            cgen_expr(n->child[0], s, target);

            fprintf(target->out, "li     $t9, 4\n");
            fprintf(target->out, "mul    $a0, $a0, $t9\n");
            fprintf(target->out, "add    $t8, $t8, $a0\n");
            fprintf(target->out, "lw     $a0, 4($sp)\n");
            fprintf(target->out, "addiu  $sp, $sp, 4\n");
            fprintf(target->out, "sw     $a0, 0($t8)\n");
        }
    } else {
        if (var->cat == CAT_VAR_SIN) {
            fprintf(target->out, "sw     $a0, %d($fp)\n", var->offset);
        } else {
            fprintf(target->out, "move   $t8, $fp\n");
            fprintf(target->out, "addiu  $t8, $t8, %d\n", var->offset);

            cgen_expr(n->child[0], s, target);

            fprintf(target->out, "li     $t9, 4\n");
            fprintf(target->out, "mul    $a0, $a0, $t9\n");
            fprintf(target->out, "sub    $t8, $t8, $a0\n");
            fprintf(target->out, "lw     $a0, 4($sp)\n");
            fprintf(target->out, "addiu  $sp, $sp, 4\n");
            fprintf(target->out, "sw     $a0, 0($t8)\n");
        }
    }
    fprintf(target->out, "addiu  $sp, $sp, 4\n");
}

void gen_num(Node* n, Target* target)
{
    int num = atoi(n->token_str);
    fprintf(target->out, "li     $a0, %d\n", num);
}

void gen_addit_e2(Node* n, Target* target, char* op)
{
    char* operation = NULL;
    switch (*op) {
        case '*':
            operation = "mul";
            break;
        case '/':
            operation = "div";
            break;
        case '+':
            operation = "add";
            break;
        case '-':
            operation = "sub";
            break;
        case '<':
            operation = "slt";
            break;
        case '>':
            operation = "sgt";
            break;
        case '=':
            operation = "seq";
            break;
        default:
            printf("Error: gen_global_var()\n");
            exit(GENERATOR_ERROR);
    }

    fprintf(target->out, "lw     $t1, 4($sp)\n");
    fprintf(target->out, "%s    $a0, $t1, $a0\n", operation);
    fprintf(target->out, "addiu  $sp, $sp, 4\n");
}

void gen_addit_e1(Node* n, Target* target)
{
    fprintf(target->out, "sw     $a0, 0($sp)\n");
    fprintf(target->out, "addiu  $sp, $sp, -4\n");
}

void gen_func_locals(Node* n, Symbol* sym, Target* target)
{
    switch (sym->cat) {
        case CAT_VAR_SIN:
            fprintf(target->out, "addiu  $sp, $sp, -4\n");
            break;
        case CAT_VAR_ARR:
            fprintf(target->out, "addiu  $sp, $sp, -%d\n", sym->len * 4);
            break;
        default:
            printf("Error: gen_global_var()\n");
            exit(GENERATOR_ERROR);
    }
}

void gen_main_entry(Node* n, Symbol* sym, Target* target)
{
    if (target->in_code == false) {
        fprintf(target->out, ".text\n");
        target->in_code = true;
    }
    gen_input_function(target);
    gen_output_function(target);

    fprintf(target->out, "\n.globl main\n%s:\n", n->token_str);
    fprintf(target->out, "move   $fp, $sp\n");
    fprintf(target->out, "sw     $ra, 0($sp)\n");
    fprintf(target->out, "addiu  $sp, $sp, -4\n");
    fprintf(target->out, "\n");
}

void gen_main_exit(Node* n, Symbol* sym, Target* target)
{
    fprintf(target->out, "\nmain_exit:\n");
    fprintf(target->out, "li     $v0, 10\n");
    fprintf(target->out, "syscall\n");
}

void gen_global_var(Node* n, Symbol* sym, Target* target)
{
    if (target->in_code == true) {
        fprintf(target->out, ".data\n");
        target->in_code = false;
    }

    switch (sym->cat) {
        case CAT_VAR_SIN:
            fprintf(target->out, "%s: .word 0:1\n", sym->id);
            break;
        case CAT_VAR_ARR:
            fprintf(target->out, "%s: .word 0:%d\n", sym->id, sym->len);
            break;
        default:
            printf("Error: gen_global_var()\n");
            exit(GENERATOR_ERROR);
    }
}

/**
 * Calculate the number of parameters that the function requires.
 */
int count_params(Node* n)
{
    assert(n != NULL);

    int params = 0;
    while (n != NULL) {
        if (n->kind == NODE_PARAMS &&
                n->element.params->parameter_kind != PARAM_VOID) {
            params += 1;
        }
        n = n->sibling;
    }

    return params;
}

/**
 * Calculate the number of bytes/words used by local variables on the stack.
 */
int count_local_space(Node* n)
{
    // Start with 4 as the $ra is already on the stack
    int total = 4;
    while (n != NULL) {
        Variable* var = n->element.decl->var;
        total += var->variable_kind == VAR_SINGLE ? 4 : var->arr_len * 4;
        n = n->sibling;
    }
    return total;
}
