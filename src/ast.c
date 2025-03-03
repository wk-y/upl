/* Grammar:
    <node> ::= (<atom> | <lpar> <node> <rpar>) [<literal> <node>]
*/
#include "ast.h"
#include "parser.h"
#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct ast_node nil = {
    .lhs = NULL,
    .rhs = NULL,
    .type = at_literal,
    .ref_count = 1,
};

struct ast_node *NIL(void) { return &nil; }

struct ast_node *ast_node_ref(struct ast_node *node) {
  if (!node) {
    return NULL;
  }
  node->ref_count++;
  return node;
}

void ast_node_deref(struct ast_node *node) {
  if (!node) {
    return;
  }
  if (node->ref_count-- <= 1) {
    ast_node_deref(node->lhs);
    ast_node_deref(node->rhs);

    if (node->type == at_literal) {
      free(node->literal);
    }

    free(node);
  }
}

struct ast_node *ast_make_number(float n) {
  struct ast_node *result = malloc(sizeof(*result));
  if (!result) {
    abort();
  }

  result->type = at_number;
  result->number = n;
  result->lhs = NULL;
  result->rhs = NULL;
  return result;
}

void ast_print(FILE *f, struct ast_node *node) {
  if (!node) {
    fputs("NULL", f);
    return;
  }

  if (node->lhs) {
    fputs("(", f);
    ast_print(f, node->lhs);
    fputs(" ", f);
  }
  switch (node->type) {
  case at_number:
    fprintf(f, "%f", node->number);
    break;
  case at_literal:
    fprintf(f, "%s", node->literal);
    break;
  default:
    fprintf(f, "???");
  }
  if (node->rhs) {
    fputs(" ", f);
    ast_print(f, node->rhs);
    fputs(")", f);
  }
}

struct ast_node *ast_make_cell(struct ast_node *node, struct ast_node *lhs,
                               struct ast_node *rhs) {
  ast_node_deref(node->lhs);
  node->lhs = lhs;

  ast_node_deref(node->rhs);
  node->rhs = rhs;
  return node;
}

struct ast_node *ast_make_literal(char const *literal) {
  struct ast_node *result = malloc(sizeof(*result));
  if (!result) {
    abort();
  }

  if (!(result->literal = malloc(strlen(literal) + 1))) {
    abort();
  }
  strcpy(result->literal, literal);

  result->type = at_literal;
  result->lhs = NULL;
  result->rhs = NULL;
  result->ref_count = 0;
  return result;
}

struct ast_node *parse_ast_node(struct parser *p) {
  parser_peek(p);
  struct ast_node *lhs = NULL;
  switch (p->tokenizer.token_type) {
  case tt_lpar:
    parser_next(p);
    lhs = parse_ast_node(p);
    if (lhs == NULL) {
      return NULL;
    }

    parser_next(p);
    if (p->tokenizer.token_type != tt_rpar) {
      ast_node_deref(lhs);
      return NULL;
    }
    break;

  case tt_number:
    parser_next(p);
    float n;
    sscanf(p->tokenizer.literal, "%f", &n);
    lhs = ast_make_number(n);
    break;

  case tt_literal:
    parser_next(p);
    lhs = ast_make_literal(p->tokenizer.literal);
    break;

  default:
    return NULL;
    break;
  }

  parser_peek(p);
  if (p->tokenizer.token_type != tt_literal) {
    return lhs;
  }

  struct ast_node *functor = ast_make_literal(p->tokenizer.literal);
  parser_next(p);

  struct ast_node *rhs = parse_ast_node(p);
  if (!rhs) {
    ast_node_deref(lhs);
    ast_node_deref(functor);
    return NULL;
  }

  return ast_make_cell(functor, lhs, rhs);
}
