#ifndef AST_H
#define AST_H

#include "parser.h"
#include <stddef.h>
#include <stdio.h>

enum ast_type {
  at_number,
  at_literal,
};

struct ast_node {
  enum ast_type type;
  union {
    float number;
    char *literal;
  };
  struct ast_node *lhs, *rhs;
  size_t ref_count;
};

struct ast_node *NIL(void);

struct ast_node *ast_make_number(float);
struct ast_node *ast_make_literal(char const *);
struct ast_node *ast_make_cell(struct ast_node *node, struct ast_node *lhs,
                               struct ast_node *rhs);

// Increment the node's ref count.
struct ast_node *ast_node_ref(struct ast_node *);

// Decrement the atom's ref count. If it reaches 0, destroy the atom.
void ast_node_deref(struct ast_node *);

void ast_print(FILE *, struct ast_node *);

struct ast_node *parse_ast_node(struct parser *);

#endif
