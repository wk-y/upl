#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tokenizer_init(struct tokenizer *t) {
  t->literal_len = 0;
  t->literal_cap = 1024;
  if (!(t->literal = malloc(t->literal_cap))) {
    abort();
  };
}

void tokenizer_deinit(struct tokenizer *t) { free(t->literal); }
void tokenizer_feed(struct tokenizer *t, FILE *f) {
  t->literal_len = 0;
  int c;
  do {
    c = getc(f);
  } while (c == ' ' || c == '\t' || c == '\n');

  if (c == EOF) {
    t->token_type = tt_eof;
    t->literal[0] = 0;
    return;
  }

  if (c >= '0' && c <= '9') {
    t->token_type = tt_number;
    while (c >= '0' && c <= '9') {
      t->literal[t->literal_len++] = c;
      c = getc(f);
    }
    t->literal[t->literal_len] = 0;
    if (c != EOF) {
      ungetc(c, f);
    }
    return;
  } else if (c == ';') {
    t->token_type = tt_semicolon;
    t->literal_len = 0;
    t->literal[0] = 0;
  } else if (c == '(') {
    t->token_type = tt_lpar;
    t->literal_len = 0;
    t->literal[0] = 0;
  } else if (c == ')') {
    t->token_type = tt_rpar;
    t->literal_len = 0;
    t->literal[0] = 0;
  } else {
    t->token_type = tt_literal;
    while (c != ' ' && c != EOF && c != '\n' && c != '(' && c != ')') {
      t->literal[t->literal_len++] = c;
      c = getc(f);
    }
    t->literal[t->literal_len] = 0;
    if (c != EOF) {
      ungetc(c, f);
    }
    return;
  }

  //   t->token_type = tt_error;
  //   strcpy(t->literal, "ERROR");
  //   t->literal_len = strlen(t->literal);
}
