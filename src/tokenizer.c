#include "tokenizer.h"
#include <stdbool.h>
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

bool literal_char_p(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9');
  ;
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
  } else if (c == ',') { // currently "," is a special literal
    t->token_type = tt_literal;
    t->literal_len = 1;
    strcpy(t->literal, ",");
  } else if (literal_char_p(c)) {
    t->token_type = tt_literal;
    while (literal_char_p(c)) {
      t->literal[t->literal_len++] = c;
      c = getc(f);
    }
    t->literal[t->literal_len] = 0;
    if (c != EOF) {
      ungetc(c, f);
    }
  } else {
    t->token_type = tt_error;
    strcpy(t->literal, "ERROR");
    t->literal_len = strlen(t->literal);
  }
}
