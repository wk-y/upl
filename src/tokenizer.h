#ifndef TOKENIZER_H
#define TOKENIZER_H
#include "stddef.h"
#include <stdio.h>

enum token_type {
  tt_error,
  tt_eof,
  tt_number,
  tt_literal,
  tt_lpar,
  tt_rpar,
};

struct tokenizer {
  enum token_type token_type;
  char *literal;
  size_t literal_len;
  size_t literal_cap;
};

void tokenizer_init(struct tokenizer *);
void tokenizer_deinit(struct tokenizer *);
void tokenizer_feed(struct tokenizer *, FILE *);

#endif
