#include "eval.h"
#include "ast.h"
#include "builtins.h"
#include "stack.h"
#include "value.h"
#include <stdlib.h>
#include <string.h>

void interpreter_init(struct interpreter *m) {
  stack_init(&m->stack);
  builtins_load_all(m);
}

void interpreter_deinit(struct interpreter *m) { stack_deinit(&m->stack); }

struct value *interpreter_get_variable(struct interpreter *m, char *name) {
  return stack_get_variable(&m->stack, name);
}

void interpreter_set_variable(struct interpreter *m, char *name,
                              struct value value) {
  stack_set_variable(&m->stack, name, value);
}

// The returned value will have a ref count of 1.
// Use value_dec_ref if the value is not used.
struct value interpreter_eval(struct interpreter *interpreter,
                              struct ast_node *node) {
  struct value verror = {.type = vt_error};
  switch (node->type) {
  case at_string: {
    struct value result = {.type = vt_string};
    if (!(result.string = malloc(sizeof(*result.string)))) {
      abort();
    }
    if (!(result.string->string = malloc(strlen(node->string.literal) + 1))) {
      abort();
    }
    result.string->ref_count = 1;
    strcpy(result.string->string, node->string.literal);
    return result;
  }

  case at_symbol: {
    struct value *var =
        interpreter_get_variable(interpreter, node->symbol.literal);
    if (var) {
      value_inc_ref(var);
      return *var;
    } else {
      return verror;
    }
  }

  case at_number: {
    struct value result = {.type = vt_number, .number = node->number.value};
    return result;
  }

  case at_statement:;
    struct value func = interpreter_eval(interpreter, node->statement.operator);
    switch (func.type) {
    case vt_cfunc:
      return func.cfunc(interpreter, node->statement.lhs, node->statement.rhs);
    case vt_func:
      struct value lvalue = interpreter_eval(interpreter, node->statement.lhs);
      struct value rvalue = interpreter_eval(interpreter, node->statement.rhs);
      stack_push(&interpreter->stack);
      stack_set_variable(&interpreter->stack, "LHS", lvalue);
      stack_set_variable(&interpreter->stack, "RHS", rvalue);
      value_dec_ref(&lvalue);
      value_dec_ref(&rvalue);
      struct value result = interpreter_eval(interpreter, &func.func->ast);
      stack_pop(&interpreter->stack);
      return result;
    default:
      value_dec_ref(&func);
      return verror;
    }

    return verror;
  case at_statement_list: {
    struct value result = {.type = vt_null};

    for (struct ast_node_statement_list *list = &node->statement_list; list;
         list = list->next) {
      value_dec_ref(&result);
      result = interpreter_eval(interpreter, list->statement);
    }
    return result;
  }
  }
  abort();
}

struct value eval(struct ast_node *ast) {
  struct interpreter interpreter;
  interpreter_init(&interpreter);
  struct value value = interpreter_eval(&interpreter, ast);
  interpreter_deinit(&interpreter);
  return value;
}
