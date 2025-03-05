#include "eval.h"
#include "ast.h"
#include "value.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void value_print(FILE *f, struct value const value) {
  switch (value.type) {
  case vt_cons:
    fputs("(", f);
    value_print(f, value.cell->lhs);
    fputs(", ", f);
    value_print(f, value.cell->rhs);
    fputs(")", f);
    break;

  case vt_null:
    fputs("NULL", f);
    break;

  case vt_number:
    fprintf(f, "%f", value.number);
    break;

  case vt_string:
    fputs(value.string->string, f);
    break;

  default:
    fputs("???", f);
  }
}

static struct value eval_print(struct interpreter *interpreter,
                               struct ast_node *lhs, struct ast_node *rhs) {
  struct value lvalue = interpreter_eval(interpreter, lhs);
  value_dec_ref(&lvalue);
  struct value rvalue = interpreter_eval(interpreter, rhs);
  value_print(stdout, rvalue);
  return rvalue;
}

static struct value eval_cons(struct interpreter *interpreter,
                              struct ast_node *lhs, struct ast_node *rhs) {
  struct value lvalue = interpreter_eval(interpreter, lhs);
  struct value rvalue = interpreter_eval(interpreter, rhs);
  struct value result = cons(lvalue, rvalue);
  value_dec_ref(&lvalue);
  value_dec_ref(&rvalue);
  return result;
}

static struct value eval_plus(struct interpreter *interpreter,
                              struct ast_node *lhs, struct ast_node *rhs) {
  struct value result = {.type = vt_null};
  struct value lvalue = interpreter_eval(interpreter, lhs);
  struct value rvalue = interpreter_eval(interpreter, rhs);
  if (lvalue.type != vt_number || rvalue.type != vt_number) {
    value_dec_ref(&lvalue);
    value_dec_ref(&rvalue);
    return result;
  }
  result.type = vt_number;
  result.number = lvalue.number + rvalue.number;
  return result;
}

static struct value eval_multiply(struct interpreter *interpreter,
                                  struct ast_node *lhs, struct ast_node *rhs) {
  struct value result = {.type = vt_null};
  struct value lvalue = interpreter_eval(interpreter, lhs);
  struct value rvalue = interpreter_eval(interpreter, rhs);
  if (lvalue.type != vt_number || rvalue.type != vt_number) {
    value_dec_ref(&lvalue);
    value_dec_ref(&rvalue);
    return result;
  }
  result.type = vt_number;
  result.number = lvalue.number * rvalue.number;
  return result;
}

static struct value eval_invalid(struct interpreter *interpreter,
                                 struct ast_node *lhs, struct ast_node *rhs) {
  (void)interpreter;
  (void)lhs;
  (void)rhs;
  struct value result = {.type = vt_null};
  return result;
}

static struct value eval_set(struct interpreter *interpreter,
                             struct ast_node *lhs, struct ast_node *rhs) {
  if (lhs->type != at_symbol) {
    struct value result = {.type = vt_null};
    return result;
  }

  struct value value = interpreter_eval(interpreter, rhs);

  interpreter_set_variable(interpreter, lhs->symbol.literal, value);
  return value;
}

static struct value eval_lt(struct interpreter *interpreter,
                            struct ast_node *lhs, struct ast_node *rhs) {
  struct value result = {.type = vt_null};
  struct value lvalue = interpreter_eval(interpreter, lhs);
  struct value rvalue = interpreter_eval(interpreter, rhs);
  if (lvalue.type != vt_number || rvalue.type != vt_number) {
    value_dec_ref(&lvalue);
    value_dec_ref(&rvalue);
    return result;
  }
  result.type = vt_number;
  result.number = lvalue.number < rvalue.number;
  return result;
}

static struct value eval_while(struct interpreter *interpreter,
                               struct ast_node *lhs, struct ast_node *rhs) {
  for (;;) {
    struct value value = interpreter_eval(interpreter, lhs);
    if (value.type != vt_number) {
      value_dec_ref(&value);
      break;
    }
    if (!value.number) {
      value_dec_ref(&value);
      break;
    }

    value_dec_ref(&value);

    value = interpreter_eval(interpreter, rhs);
    value_dec_ref(&value);
  }

  struct value result = {.type = vt_null};
  return result;
}

static struct value (*lookup_func(char *name))(struct interpreter *interpreter,
                                               struct ast_node *,
                                               struct ast_node *) {
  if (!strcmp(name, "print")) {
    return eval_print;
  }
  if (!strcmp(name, ",")) {
    return eval_cons;
  }
  if (!strcmp(name, "+")) {
    return eval_plus;
  }
  if (!strcmp(name, "*")) {
    return eval_multiply;
  }
  if (!strcmp(name, "=")) {
    return eval_set;
  }
  if (!strcmp(name, "<")) {
    return eval_lt;
  }
  if (!strcmp(name, "while")) {
    return eval_while;
  }
  return eval_invalid;
}

void interpreter_init(struct interpreter *m) { m->variables = NULL; }
void interpreter_deinit(struct interpreter *m) {
  while (m->variables) {
    struct variables *v = m->variables;
    m->variables = m->variables->next;
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

    new_variable->name = name;
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
    }
    return result;
  }

  case at_number: {
    struct value result = {.type = vt_number, .number = node->number.value};
    return result;
  }

  case at_statement:
    return lookup_func(node->statement.operator->symbol.literal)(
        interpreter, node->statement.lhs, node->statement.rhs);

  case at_statement_list: {
    struct value result = {.type = vt_null};

    for (struct ast_node_statement_list *list = &node->statement_list; list;
         list = list->next) {
      value_dec_ref(&result);
      result = interpreter_eval(interpreter, list->statement);
    }
    return result;
  }
  default: {
    // todo: error?
    struct value result = {.type = vt_null};
    return result;
  }
  }
}

struct value eval(struct ast_node *ast) {
  struct interpreter interpreter;
  interpreter_init(&interpreter);
  struct value value = interpreter_eval(&interpreter, ast);
  interpreter_deinit(&interpreter);
  return value;
}
