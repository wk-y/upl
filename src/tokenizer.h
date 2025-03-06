#ifndef TOKENIZER_H
#define TOKENIZER_H
#include "stddef.h"
#include <stdbool.h>
#include <stdio.h>

enum token_type {
  tt_error,
  tt_eof,
  tt_number,
  tt_symbol,
  tt_lpar,
  tt_rpar,
  tt_semicolon,
  tt_string,
};

struct tokenizer {
  enum token_type token_type;
  char *literal;
  size_t literal_len;
  size_t literal_cap;
  size_t line;
  size_t column;
  int _peeked;
  bool peeking;
  FILE *_file;
};

void tokenizer_init(struct tokenizer *);
void tokenizer_deinit(struct tokenizer *);
void tokenizer_feed(struct tokenizer *, FILE *);

#endif
