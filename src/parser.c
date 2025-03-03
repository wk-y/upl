#include "parser.h"
#include "tokenizer.h"
#include <stdbool.h>

void parser_init(struct parser *p, FILE *f) {
  tokenizer_init(&p->tokenizer);
  p->file = f;
  p->peeking = false;
}
void parser_deinit(struct parser *p) { tokenizer_deinit(&p->tokenizer); }

void parser_peek(struct parser *p) {
  if (p->peeking) {
    return;
  }
  tokenizer_feed(&p->tokenizer, p->file);
  p->peeking = true;
}

void parser_next(struct parser *p) {
  parser_peek(p);
  p->peeking = false;
}
