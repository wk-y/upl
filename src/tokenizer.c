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
  t->line = 1;
  t->column = 0;
}

bool literal_char_p(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9');
}

static int tokenizer_getc(struct tokenizer *t, FILE *f) {
  int c = getc(f);
  if (c == '\n') {
    t->line++;
    t->_prev_column = t->column;
    t->column = 0;
  } else {
    // todo: Handle variable width ('\t').
    t->column += 1;
  }
  return c;
}

static int tokenizer_ungetc(struct tokenizer *t, int c, FILE *f) {
  int ret = ungetc(c, f);
  if (c == '\n') {
    t->line--;
    t->column = t->_prev_column;
    t->_prev_column = -1; // missing value
  } else {
    // todo: Handle variable width ('\t').
    t->column -= 1;
  }
  return ret;
}

void tokenizer_deinit(struct tokenizer *t) { free(t->literal); }
void tokenizer_feed(struct tokenizer *t, FILE *f) {
#define TGETC(f) tokenizer_getc(t, f)
#define TUNGETC(c, f) tokenizer_ungetc(t, c, f)

  t->literal_len = 0;
  int c;
  do {
    c = TGETC(f);
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
      c = TGETC(f);
    }
    t->literal[t->literal_len] = 0;
    if (c != EOF) {
      TUNGETC(c, f);
    }
    return;
  }

  if (literal_char_p(c)) {
    t->token_type = tt_symbol;
    while (literal_char_p(c)) {
      t->literal[t->literal_len++] = c;
      c = TGETC(f);
    }
    t->literal[t->literal_len] = 0;
    if (c != EOF) {
      TUNGETC(c, f);
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

  case '"': {
    t->token_type = tt_string;
    bool escaping = false;
    for (;;) {
      c = TGETC(f);

      if (escaping) {
        switch (c) {
        case 'n':
          c = '\n';
          break;
        }
      }

      if (c == EOF) {
        goto error_token;
      }

      if (c == '"' && !escaping) {
        break;
      }

      escaping = !escaping && c == '\\';
      if (!escaping) {
        t->literal[t->literal_len++] = c;
      }
    }
    t->literal[t->literal_len] = 0;
    break;
  }
  default:
  error_token:
    t->token_type = tt_error;
    strcpy(t->literal, "ERROR");
    t->literal_len = strlen(t->literal);
    break;
  }

#undef TGETC
#undef TUNGETC
}
