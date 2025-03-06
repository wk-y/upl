#include "tokenizer.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../tokenizer_first.h"

static void tok_write_char(struct tokenizer *t, char c);
static int tokenizer_peek(struct tokenizer *t);
static int tokenizer_next(struct tokenizer *t);

static int tok_parse(struct tokenizer *t);
static int tok_parse_float(struct tokenizer *t);
static int tok_parse_digits(struct tokenizer *t);
static int tok_parse_literal(struct tokenizer *t);
static int tok_parse_symbol(struct tokenizer *t);
static int tok_parse_string(struct tokenizer *t);

void tokenizer_init(struct tokenizer *t) {
  t->literal_len = 0;
  t->literal_cap = 1024;
  if (!(t->literal = malloc(t->literal_cap))) {
    abort();
  };
  t->line = 1;
  t->column = 0;
}

static int tokenizer_peek(struct tokenizer *t) {
  if (!t->peeking) {
    t->peeking = true;
    t->_peeked = getc(t->_file);
  }
  return t->_peeked;
}

static int tokenizer_next(struct tokenizer *t) {
  int c = tokenizer_peek(t);
  t->peeking = false;

  if (c == '\n') {
    t->line++;
    t->column = 0;
  } else {
    // todo: Handle variable width ('\t').
    t->column += 1;
  }
  return c;

  return c;
}

void tokenizer_deinit(struct tokenizer *t) { free(t->literal); }
void tokenizer_feed(struct tokenizer *t, FILE *f) {
  t->literal_len = 0;
  t->literal[0] = 0;
  t->_file = f;
  if (tok_parse(t)) {
    t->token_type = tt_error;
  }
  t->_file = NULL;
  if (t->peeking) {
    t->peeking = false;
    ungetc(t->_peeked, f);
  }
  t->literal[t->literal_len] = 0;
}

static void tok_write_char(struct tokenizer *t, char c) {
  t->literal[t->literal_len++] = c;
  if (t->literal_len >= t->literal_cap) {
    size_t new_cap = t->literal_cap * 2;
    char *new_lit = realloc(t->literal, new_cap);
    if (!new_lit) {
      abort();
    }
    t->literal = new_lit;
    t->literal_cap = new_cap;
  }
}

static int tok_parse(struct tokenizer *t) {
  if (tokenizer_peek(t) == '-') {
    tok_write_char(t, tokenizer_next(t));
    switch (tokenizer_peek(t)) {
      CASE_FIRST_FLOAT;
      return tok_parse_float(t);

      CASE_FIRST_SYMBOL;
      return tok_parse_symbol(t);

    default:
      t->token_type = tt_symbol;
      return 0;
    }
  }

  switch (tokenizer_peek(t)) {
    CASE_FIRST_WHIESPACE;
    tokenizer_next(t);
    return tok_parse(t);

    CASE_FIRST_SYMBOL;
    return tok_parse_symbol(t);

    CASE_FIRST_LITERAL;
    return tok_parse_literal(t);

    CASE_FIRST_FLOAT;
    return tok_parse_float(t);

    CASE_FIRST_LPAR;
    tok_write_char(t, tokenizer_next(t));
    t->token_type = tt_lpar;
    return 0;

    CASE_FIRST_RPAR;
    tok_write_char(t, tokenizer_next(t));
    t->token_type = tt_rpar;
    return 0;

    CASE_FIRST_SEMICOLON;
    tok_write_char(t, tokenizer_next(t));
    t->token_type = tt_semicolon;
    return 0;

    CASE_FIRST_STRING;
    return tok_parse_string(t);

  case EOF:
    t->token_type = tt_eof;
    return 0;

  default:
    return -1;
  }
}

static int tok_parse_float(struct tokenizer *t) {
  if (tok_parse_digits(t)) {
    return -1;
  }
  if (tokenizer_peek(t) != '.') {
    t->token_type = tt_number;
    return 0;
  }
  return tok_parse_digits(t);
}

static int tok_parse_digits(struct tokenizer *t) {
  for (;;) {
    switch (tokenizer_peek(t)) {
      CASE_FIRST_DIGITS;
      tok_write_char(t, tokenizer_next(t));
      break;

    default:
      return 0;
    }
  }
}

static int tok_parse_literal(struct tokenizer *t) {
  for (;;) {
    switch (tokenizer_peek(t)) {
      CASE_FIRST_LITERAL;
      tok_write_char(t, tokenizer_next(t));
      break;
    default:
      t->token_type = tt_symbol;
      return 0;
    }
  }
}

static int tok_parse_symbol(struct tokenizer *t) {
  for (;;) {
    switch (tokenizer_peek(t)) {
      CASE_FIRST_SYMBOL;
      tok_write_char(t, tokenizer_next(t));
      break;
    default:
      t->token_type = tt_symbol;
      return 0;
    }
  }
}

static int tok_parse_string(struct tokenizer *t) {
  if (tokenizer_peek(t) != '"' || tokenizer_peek(t) == EOF) {
    return -1;
  }
  tokenizer_next(t);

  for (;;) {
    switch (tokenizer_peek(t)) {
    case EOF:
      return -1;

    case '"':
      tokenizer_next(t);
      t->token_type = tt_string;
      return 0;

    case '\\':
      tokenizer_next(t);
      switch (tokenizer_peek(t)) {
      case 'n':
        tokenizer_next(t);
        tok_write_char(t, '\n');
        break;

      default:
        tok_write_char(t, tokenizer_next(t));
      }
      break;

    default:
      tok_write_char(t, tokenizer_next(t));
    }
  }
}
