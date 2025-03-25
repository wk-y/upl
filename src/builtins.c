#include "builtins.h"
#include "ast.h"
#include "eval.h"
#include "value.h"
#include <math.h>
#include <stdlib.h>

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

static struct value eval_and(struct interpreter *interpreter,
                             struct ast_node *lhs, struct ast_node *rhs) {
  struct value lvalue = interpreter_eval(interpreter, lhs);
  struct value rvalue = interpreter_eval(interpreter, rhs);
  struct value null = {.type = vt_null};
  struct value tail = cons(rvalue, null);
  struct value result = cons(lvalue, tail);
  value_dec_ref(&tail);
  value_dec_ref(&lvalue);
  value_dec_ref(&rvalue);
  return result;
}

static struct value eval_plus(struct interpreter *interpreter,
                              struct ast_node *lhs, struct ast_node *rhs) {
  struct value result = {.type = vt_error};
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

static struct value eval_minus(struct interpreter *interpreter,
                               struct ast_node *lhs, struct ast_node *rhs) {
  struct value result = {.type = vt_error};
  struct value lvalue = interpreter_eval(interpreter, lhs);
  struct value rvalue = interpreter_eval(interpreter, rhs);
  if (lvalue.type != vt_number || rvalue.type != vt_number) {
    value_dec_ref(&lvalue);
    value_dec_ref(&rvalue);
    return result;
  }
  result.type = vt_number;
  result.number = lvalue.number - rvalue.number;
  return result;
}

static struct value eval_multiply(struct interpreter *interpreter,
                                  struct ast_node *lhs, struct ast_node *rhs) {
  struct value result = {.type = vt_error};
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

static struct value eval_divide(struct interpreter *interpreter,
                                struct ast_node *lhs, struct ast_node *rhs) {
  struct value result = {.type = vt_error};
  struct value lvalue = interpreter_eval(interpreter, lhs);
  struct value rvalue = interpreter_eval(interpreter, rhs);
  if (lvalue.type != vt_number || rvalue.type != vt_number) {
    value_dec_ref(&lvalue);
    value_dec_ref(&rvalue);
    return result;
  }
  result.type = vt_number;
  result.number = lvalue.number / rvalue.number;
  return result;
}

static struct value eval_modulo(struct interpreter *interpreter,
                                struct ast_node *lhs, struct ast_node *rhs) {
  struct value result = {.type = vt_error};
  struct value lvalue = interpreter_eval(interpreter, lhs);
  struct value rvalue = interpreter_eval(interpreter, rhs);
  if (lvalue.type != vt_number || rvalue.type != vt_number) {
    value_dec_ref(&lvalue);
    value_dec_ref(&rvalue);
    return result;
  }
  result.type = vt_number;
  result.number = fmodl(lvalue.number, rvalue.number);
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

static struct value eval_equals(struct interpreter *interpreter,
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
  result.number = lvalue.number == rvalue.number;
  return result;
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

static struct value eval_gt(struct interpreter *interpreter,
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
  result.number = lvalue.number > rvalue.number;
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

static struct value eval_then(struct interpreter *interpreter,
                              struct ast_node *lhs, struct ast_node *rhs) {
  struct value vnull = {.type = vt_null};
  struct value value = interpreter_eval(interpreter, lhs);
  if (value.type != vt_number) {
    value_dec_ref(&value);
    return vnull;
  }

  if (!value.number) {
    value_dec_ref(&value);
    return vnull;
  }

  value = interpreter_eval(interpreter, rhs);
  value_dec_ref(&value);

  return vnull;
}

static struct value eval_func(struct interpreter *interpreter,
                              struct ast_node *lhs, struct ast_node *rhs) {
  (void)interpreter;
  (void)lhs;
  struct value result = {.type = vt_func};
  if (!(result.func = malloc(sizeof(*result.func)))) {
    abort();
  }

  ast_deep_copy(&result.func->ast, rhs);
  result.func->ref_count = 1;
  return result;
}

static void add_cfunc(struct interpreter *m, char *name,
                      struct value (*cfunc)(struct interpreter *,
                                            struct ast_node *,
                                            struct ast_node *)) {
  struct value v = {.type = vt_cfunc, .cfunc = cfunc};
  interpreter_set_variable(m, name, v);
}

void builtins_load_all(struct interpreter *m) {
  struct value vnull = {.type = vt_null};
  interpreter_set_variable(m, ".", vnull);
  add_cfunc(m, ",", eval_cons);
  add_cfunc(m, "=", eval_set);
  add_cfunc(m, "and", eval_and);
  add_cfunc(m, "print", eval_print);

  add_cfunc(m, "while", eval_while);
  add_cfunc(m, "then", eval_then);

  add_cfunc(m, "==", eval_equals);
  add_cfunc(m, "<", eval_lt);
  add_cfunc(m, ">", eval_gt);
  add_cfunc(m, "mod", eval_modulo);

  add_cfunc(m, "+", eval_plus);
  add_cfunc(m, "-", eval_minus);
  add_cfunc(m, "*", eval_multiply);
  add_cfunc(m, "/", eval_divide);
  add_cfunc(m, "func", eval_func);
}
