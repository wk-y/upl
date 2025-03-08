#include "ast.h"
#include "parser.h"
#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CASE_FIRST_STRING case tt_string:
#define CASE_FIRST_NUMBER case tt_number:
#define CASE_FIRST_SYMBOL case tt_symbol:
#define CASE_FIRST_ATOM CASE_FIRST_SYMBOL CASE_FIRST_NUMBER CASE_FIRST_STRING
#define CASE_FIRST_EXPR CASE_FIRST_ATOM case tt_lpar:
#define CASE_FIRST_STATEMENT CASE_FIRST_EXPR
#define CASE_FIRST_STATEMENT_LIST CASE_FIRST_STATEMENT

// Allocate an AST node
static struct ast_node *ast_node_alloc(void) {
  struct ast_node t = {0};
  struct ast_node *result = malloc(sizeof(t));
  if (!result) {
    abort();
  }
  *result = t;
  return result;
}

struct ast_node *ast_node_ref(struct ast_node *node) {
  if (!node) {
    return NULL;
  }
  return node;
}

void ast_node_destroy(struct ast_node *node) {
  if (!node) {
    return;
  }

  switch (node->type) {
  case at_symbol:
    free(node->symbol.literal);
    break;

  case at_string:
    free(node->string.literal);
    break;

  case at_statement:
    ast_node_destroy(node->statement.lhs);
    free(node->statement.lhs);

    ast_node_destroy(node->statement.rhs);
    free(node->statement.rhs);

    ast_node_destroy(node->statement.operator);
    free(node->statement.operator);
    break;

  case at_statement_list:
    ast_node_destroy(node->statement_list.statement);
    free(node->statement_list.statement);

    struct ast_node_statement_list *list = node->statement_list.next;
    while (list) {
      ast_node_destroy(list->statement);
      free(list->statement);

      struct ast_node_statement_list *to_free = list;
      list = list->next;

      free(to_free);
    }
    break;

  case at_number:
    break; // nothing to do here
  }
}

struct ast_node *ast_make_number(struct ast_node *dst, double n) {
  dst->type = at_number;
  dst->number.value = n;
  return dst;
}

struct ast_node *ast_make_string(struct ast_node *dst, char const *literal) {
  dst->type = at_string;

  if (!(dst->string.literal = malloc(strlen(literal) + 1))) {
    abort();
  }
  strcpy(dst->string.literal, literal);

  return dst;
}

static struct ast_node_statement_list *
ast_deep_copy_statement_list(struct ast_node_statement_list *list) {
  if (!list) {
    return NULL;
  }

  struct ast_node_statement_list *result = malloc(sizeof(*result));
  if (!result) {
    abort();
  }
  if (!(result->statement = malloc(sizeof(*result->statement)))) {
    abort();
  }
  ast_deep_copy(result->statement, list->statement);
  result->next = ast_deep_copy_statement_list(list->next);
  return result;
}

struct ast_node *ast_deep_copy(struct ast_node *dst, struct ast_node *node) {
  switch (node->type) {
  case at_number:
    return ast_make_number(dst, node->number.value);
  case at_string:
    return ast_make_string(dst, node->string.literal);
  case at_statement:
    return ast_make_compound_statement(
        dst, ast_deep_copy(ast_node_alloc(), node->statement.operator),
        ast_deep_copy(ast_node_alloc(), node->statement.lhs),
        ast_deep_copy(ast_node_alloc(), node->statement.rhs));
  case at_symbol:
    return ast_make_symbol(dst, node->symbol.literal);
  case at_statement_list:;
    dst->type = at_statement_list;
    if (!(dst->statement_list.statement =
              malloc(sizeof(*dst->statement_list.statement)))) {
      abort();
    }
    ast_deep_copy(dst->statement_list.statement,
                  node->statement_list.statement);
    dst->statement_list.next =
        ast_deep_copy_statement_list(node->statement_list.next);
    return dst;
  }
  abort();
}

void ast_print(FILE *f, struct ast_node *node) {
  if (!node) {
    fputs("NULL", f);
    return;
  }

  switch (node->type) {
  case at_number:
    fprintf(f, "%f", node->number.value);
    break;
  case at_symbol:
    fprintf(f, "%s", node->symbol.literal);
    break;
  case at_statement:
    fputc('(', f);
    ast_print(f, node->statement.operator);
    fputc(' ', f);
    ast_print(f, node->statement.lhs);
    fputc(' ', f);
    ast_print(f, node->statement.rhs);
    fputc(')', f);
    break;
  case at_statement_list:
    for (struct ast_node_statement_list *list = &node->statement_list; list;
         list = list->next) {
      ast_print(f, list->statement);
      fputs("; ", f);
    }
    break;
  case at_string:
    printf("\"%s\"", node->string.literal);
    break;
  default:
    fprintf(f, "???");
  }
}

struct ast_node *ast_make_compound_statement(struct ast_node *dst,
                                             struct ast_node *operator,
                                             struct ast_node * lhs,
                                             struct ast_node *rhs) {
  dst->type = at_statement;
  dst->statement.lhs = lhs;
  dst->statement.rhs = rhs;
  dst->statement.operator= operator;
  return dst;
}

struct ast_node *ast_make_symbol(struct ast_node *dst, char const *literal) {
  dst->type = at_symbol;

  if (!(dst->symbol.literal = malloc(strlen(literal) + 1))) {
    abort();
  }
  strcpy(dst->symbol.literal, literal);

  return dst;
}
static int parse_statement_list_helper(struct parser *p,
                                       struct ast_node_statement_list *r) {
  r->statement = NULL;
  r->next = NULL;

  parser_peek(p);
  switch (p->tokenizer.token_type) {
    CASE_FIRST_STATEMENT
    if (parse_statement(p, &r->statement)) {
      return -1;
    }

    parser_peek(p);
    if (p->tokenizer.token_type != tt_semicolon) {
      return 0;
    }
    parser_next(p);

    parser_peek(p);
    switch (p->tokenizer.token_type) {
      CASE_FIRST_STATEMENT_LIST
      break;
    default:
      return 0;
    }

    if (!(r->next = malloc(sizeof(*r->next)))) {
      abort();
    }

    if (parse_statement_list_helper(p, r->next)) {
      ast_node_destroy(r->statement);
      free(r->statement);

      r->statement = NULL;
      free(r->next);
      r->next = NULL;
      return -1;
    }
    return 0;
  default:
    return -1;
  }
}
int parse_statement_list(struct parser *p, struct ast_node **r) {
  struct ast_node_statement_list list;
  if (parse_statement_list_helper(p, &list)) {
    return -1;
  }

  *r = ast_node_alloc();
  (*r)->type = at_statement_list;
  (*r)->statement_list = list;
  return 0;
}

int parse_statement(struct parser *p, struct ast_node **r) {
  parser_peek(p);
  switch (p->tokenizer.token_type) {
    CASE_FIRST_EXPR;
    int err = parse_expr(p, r);
    if (err) {
      return err;
    }

    parser_peek(p);
    if (p->tokenizer.token_type != tt_symbol) {
      return 0; // Just an expression
    }

    struct ast_node *operator, * rhs;
    if ((err = parse_literal(p, &operator))) {
      ast_node_destroy(*r);
      free(*r);

      *r = NULL;
      return err;
    }

    if ((err = parse_statement(p, &rhs))) {
      ast_node_destroy(*r);
      free(*r);

      *r = NULL;
      ast_node_destroy(operator);
      free(operator);
      return err;
    }

    *r = ast_make_compound_statement(ast_node_alloc(), operator, * r, rhs);
    return 0;
  default:
    return -1;
  }
}

int parse_expr(struct parser *p, struct ast_node **r) {
  parser_peek(p);
  switch (p->tokenizer.token_type) {
    CASE_FIRST_ATOM
    return parse_atom(p, r);
  case tt_lpar:
    parser_next(p);

    if (parse_statement_list(p, r)) {
      return -1;
    }

    parser_next(p);
    if (p->tokenizer.token_type != tt_rpar) {
      ast_node_destroy(*r);
      free(*r);
      *r = NULL;
      return -1;
    }
    return 0;
  default:
    return -1;
  }
}

int parse_atom(struct parser *p, struct ast_node **r) {
  parser_peek(p);
  switch (p->tokenizer.token_type) {
    CASE_FIRST_NUMBER
    return parse_number(p, r);
    CASE_FIRST_SYMBOL
    return parse_literal(p, r);
    CASE_FIRST_STRING
    return parse_string(p, r);
  default:
    return -1;
  }
}

int parse_literal(struct parser *p, struct ast_node **r) {
  parser_next(p);
  switch (p->tokenizer.token_type) {
  case tt_symbol:
    *r = ast_make_symbol(ast_node_alloc(), p->tokenizer.literal);
    return 0;
  default:
    return -1;
  }
}

int parse_number(struct parser *p, struct ast_node **r) {
  parser_next(p);
  switch (p->tokenizer.token_type) {
  case tt_number:;
    double n;
    sscanf(p->tokenizer.literal, "%lf", &n);
    *r = ast_make_number(ast_node_alloc(), n);
    return 0;
  default:
    return -1;
  }
}

int parse_string(struct parser *p, struct ast_node **r) {
  parser_next(p);
  switch (p->tokenizer.token_type) {
  case tt_string:
    *r = ast_make_string(ast_node_alloc(), p->tokenizer.literal);
    return 0;
  default:
    return -1;
  }
}
