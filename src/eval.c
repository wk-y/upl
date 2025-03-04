#include "eval.h"
#include "ast.h"
#include "value.h"
#include <stdio.h>
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

  default:
    fputs("???", f);
  }
}

static struct value eval_print(struct ast_node *lhs, struct ast_node *rhs) {
  struct value lvalue = eval(lhs);
  value_dec_ref(&lvalue);
  struct value rvalue = eval(rhs);
  value_print(stdout, rvalue);
  return rvalue;
}

static struct value eval_cons(struct ast_node *lhs, struct ast_node *rhs) {
  struct value lvalue = eval(lhs);
  struct value rvalue = eval(rhs);
  struct value result = cons(lvalue, rvalue);
  value_dec_ref(&lvalue);
  value_dec_ref(&rvalue);
  return result;
}

static struct value eval_plus(struct ast_node *lhs, struct ast_node *rhs) {
  struct value result = {.type = vt_null};
  struct value lvalue = eval(lhs);
  struct value rvalue = eval(rhs);
  if (lvalue.type != vt_number || rvalue.type != vt_number) {
    value_dec_ref(&lvalue);
    value_dec_ref(&rvalue);
    return result;
  }
  result.type = vt_number;
  result.number = lvalue.number + rvalue.number;
  return result;
}

static struct value eval_multiply(struct ast_node *lhs, struct ast_node *rhs) {
  struct value result = {.type = vt_null};
  struct value lvalue = eval(lhs);
  struct value rvalue = eval(rhs);
  if (lvalue.type != vt_number || rvalue.type != vt_number) {
    value_dec_ref(&lvalue);
    value_dec_ref(&rvalue);
    return result;
  }
  result.type = vt_number;
  result.number = lvalue.number * rvalue.number;
  return result;
}

static struct value (*lookup_func(char *name))(struct ast_node *,
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
  return NULL;
}

// The returned value will have a ref count of 1.
// Use value_dec_ref if the value is not used.
struct value eval(struct ast_node *node) {
  switch (node->type) {
  case at_literal: {
    // not implemented
    struct value result = {.type = vt_null};
    return result;
  }

  case at_number: {
    struct value result = {.type = vt_number, .number = node->number.value};
    return result;
  }

  case at_statement:
    return lookup_func(node->statement.operator->literal.literal)(
        node->statement.lhs, node->statement.rhs);

  case at_statement_list: {
    struct value result = {.type = vt_null};

    for (struct ast_node_statement_list *list = &node->statement_list; list;
         list = list->next) {
      value_dec_ref(&result);
      result = eval(list->statement);
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
