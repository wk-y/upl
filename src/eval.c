#include "eval.h"
#include "ast.h"
#include <stdio.h>
#include <string.h>

static struct ast_node *eval_print(struct ast_node *lhs, struct ast_node *rhs) {
  (void)lhs;
  ast_print(stdout, rhs);
  return rhs;
}

static struct ast_node *(*lookup_func(char *name))(struct ast_node *,
                                                   struct ast_node *) {
  if (!strcmp(name, "print")) {
    return eval_print;
  }
  return NULL;
}

struct ast_node *eval(struct ast_node *node) {
  switch (node->type) {
  case at_invalid:
    return node;
  case at_literal:
    return node;
  case at_number:
    return node;
  case at_statement:
    lookup_func(node->statement.operator->literal.literal)(node->statement.lhs,
                                                           node->statement.rhs);
    return NULL;

  case at_statement_list:
    for (struct ast_node_statement_list *list = &node->statement_list; list;
         list = list->next) {
      eval(list->statement);
    }
  default:
    return node;
  }
}
