#ifndef EVAL_H
#define EVAL_H

#include "ast.h"
#include "value.h"

struct variables {
  char *name;
  struct value value;
  struct variables *next;
};

struct interpreter {
  struct variables *variables;
};

void interpreter_init(struct interpreter *);
void interpreter_deinit(struct interpreter *);

struct value *interpreter_get_variable(struct interpreter, char *name);

struct value interpreter_eval(struct interpreter *, struct ast_node *);

// Creates an interpreter to use
struct value eval(struct ast_node *);

#endif
