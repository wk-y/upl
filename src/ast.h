#ifndef AST_H
#define AST_H

#include "parser.h"
#include <stddef.h>
#include <stdio.h>

enum ast_type {
  at_statement_list,
  at_statement,
  at_symbol,
  at_string,
  at_number,
};

struct ast_node_statement_list {
  struct ast_node *statement;
  struct ast_node_statement_list *next;
};

struct ast_node_statement {
  struct ast_node *lhs, *operator, * rhs;
};

struct ast_node_symbol {
  char *literal;
};

struct ast_node_string {
  char *literal;
};

struct ast_node_number {
  double value;
};

struct ast_node {
  enum ast_type type;
  union {
    struct ast_node_statement statement;
    struct ast_node_symbol symbol;
    struct ast_node_string string;
    struct ast_node_number number;
    struct ast_node_statement_list statement_list;
  };
};

struct ast_node *ast_make_number(struct ast_node *dst, double);
struct ast_node *ast_make_literal(struct ast_node *dst, char const *);
struct ast_node *ast_make_string(struct ast_node *dst, char const *);
struct ast_node *ast_make_compound_statement(struct ast_node *dst,
                                             struct ast_node *operator,
                                             struct ast_node * lhs,
                                             struct ast_node *rhs);

struct ast_node *ast_deep_copy(struct ast_node *, struct ast_node *);
void ast_node_destroy(struct ast_node *);

void ast_print(FILE *, struct ast_node *);

int parse_statement_list(struct parser *, struct ast_node **);
int parse_statement(struct parser *, struct ast_node **);
int parse_expr(struct parser *, struct ast_node **);
int parse_atom(struct parser *, struct ast_node **);
int parse_literal(struct parser *, struct ast_node **);
int parse_number(struct parser *, struct ast_node **);
int parse_string(struct parser *, struct ast_node **);

#endif
