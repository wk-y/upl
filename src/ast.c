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

void ast_node_free(struct ast_node *node) {
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
    ast_node_free(node->statement.lhs);
    ast_node_free(node->statement.rhs);
    ast_node_free(node->statement.operator);
    break;

  case at_statement_list:
    ast_node_free(node->statement_list.statement);
    struct ast_node_statement_list *list = node->statement_list.next;
    while (list) {
      ast_node_free(list->statement);
      struct ast_node_statement_list *to_free = list;
      list = list->next;
      free(to_free);
    }
    break;

  case at_number:
  case at_invalid:
    break; // nothing to do here
  }

  free(node);
}

struct ast_node *ast_make_number(float n) {
  struct ast_node *result = ast_node_alloc();

  result->type = at_number;
  result->number.value = n;
  return result;
}

struct ast_node *ast_make_string(char const *literal) {
  struct ast_node *result = ast_node_alloc();
  result->type = at_string;

  if (!(result->string.literal = malloc(strlen(literal) + 1))) {
    abort();
  }
  strcpy(result->string.literal, literal);

  return result;
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

struct ast_node *ast_make_compound_statement(struct ast_node *operator,
                                             struct ast_node * lhs,
                                             struct ast_node *rhs) {
  struct ast_node *result = ast_node_alloc();
  result->type = at_statement;
  result->statement.lhs = lhs;
  result->statement.rhs = rhs;
  result->statement.operator= operator;
  return result;
}

struct ast_node *ast_make_literal(char const *literal) {
  struct ast_node *result = ast_node_alloc();
  result->type = at_symbol;

  if (!(result->symbol.literal = malloc(strlen(literal) + 1))) {
    abort();
  }
  strcpy(result->symbol.literal, literal);

  return result;
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
      ast_node_free(r->statement);
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
      ast_node_free(*r);
      *r = NULL;
      return err;
    }

    if ((err = parse_statement(p, &rhs))) {
      parse_statement(p, &rhs);
      ast_node_free(*r);
      *r = NULL;
      ast_node_free(operator);
      return err;
    }

    *r = ast_make_compound_statement(operator, * r, rhs);
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
      ast_node_free(*r);
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
    *r = ast_make_literal(p->tokenizer.literal);
    return 0;
  default:
    return -1;
  }
}

int parse_number(struct parser *p, struct ast_node **r) {
  parser_next(p);
  switch (p->tokenizer.token_type) {
  case tt_number:;
    float n;
    sscanf(p->tokenizer.literal, "%f", &n);
    *r = ast_make_number(n);
    return 0;
  default:
    return -1;
  }
}

int parse_string(struct parser *p, struct ast_node **r) {
  parser_next(p);
  switch (p->tokenizer.token_type) {
  case tt_string:
    *r = ast_make_string(p->tokenizer.literal);
    return 0;
  default:
    return -1;
  }
}
