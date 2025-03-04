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
  }

  if (literal_char_p(c)) {
    t->token_type = tt_symbol;
    while (literal_char_p(c)) {
      t->literal[t->literal_len++] = c;
      c = getc(f);
    }
    t->literal[t->literal_len] = 0;
    if (c != EOF) {
      ungetc(c, f);
    }
    return;
  }

  switch (c) {
  case ';':
    t->token_type = tt_semicolon;
    strcpy(t->literal, ";");
    t->literal_len = strlen(t->literal);
    break;
  case '(':
    t->token_type = tt_lpar;
    strcpy(t->literal, "(");
    t->literal_len = strlen(t->literal);
    break;
  case ')':
    t->token_type = tt_rpar;
    strcpy(t->literal, ")");
    t->literal_len = strlen(t->literal);
    break;

  case ',':
  case '+':
  case '-':
  case '*':
  case '/':
  case '=':
  case '<':
  case '>':
    t->token_type = tt_symbol;
    t->literal_len = 1;
    t->literal[0] = c;
    t->literal[1] = 0;
    break;

  default:
    t->token_type = tt_error;
    strcpy(t->literal, "ERROR");
    t->literal_len = strlen(t->literal);
    break;
  }
}
