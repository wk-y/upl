#include "eval.h"
#include "ast.h"
#include "builtins.h"
#include "value.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void interpreter_init(struct interpreter *m) {
  m->variables = NULL;
  builtins_load_all(m);
}

void interpreter_deinit(struct interpreter *m) {
  while (m->variables) {
    struct variables *v = m->variables;
    m->variables = m->variables->next;
    free(v->name);
    value_dec_ref(&v->value);
    free(v);
  }
}

struct value *interpreter_get_variable(struct interpreter *m, char *name) {
  for (struct variables *v = m->variables; v; v = v->next) {
    if (!strcmp(v->name, name)) {
      return &v->value;
    }
  }
  return NULL;
}

void interpreter_set_variable(struct interpreter *m, char *name,
                              struct value value) {
  struct value *var = interpreter_get_variable(m, name);
  if (!var) {
    struct variables *new_variable = malloc(sizeof(*new_variable));
    if (!new_variable) {
      abort();
    }

    new_variable->name = malloc(strlen(name) + 1);
    if (!new_variable->name) {
      abort();
    }
    strcpy(new_variable->name, name);
    new_variable->value.type = vt_null;
    new_variable->next = m->variables;
    m->variables = new_variable;
    var = &new_variable->value;
  }
  value_set(var, value);
}

// The returned value will have a ref count of 1.
// Use value_dec_ref if the value is not used.
struct value interpreter_eval(struct interpreter *interpreter,
                              struct ast_node *node) {
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
    struct value result = {.type = vt_null};
    struct value *var =
        interpreter_get_variable(interpreter, node->symbol.literal);
    if (var) {
      value_inc_ref(var);
      return *var;
    } else {
      struct value result = {.type = vt_error};
      return result;
    }
    return result;
  }

  case at_number: {
    struct value result = {.type = vt_number, .number = node->number.value};
    return result;
  }

  case at_statement:;
    struct value *func = interpreter_get_variable(
        interpreter, node->statement.operator->symbol.literal);
    if (!func || func->type != vt_cfunc) {
      struct value result = {.type = vt_error};
      return result;
    }

    return func->cfunc(interpreter, node->statement.lhs, node->statement.rhs);

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
