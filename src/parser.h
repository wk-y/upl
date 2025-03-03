#ifndef PARSER_H
#define PARSER_H

#include "stdbool.h"
#include "tokenizer.h"

struct parser {
  struct tokenizer tokenizer;
  FILE *file;
  bool peeking;
};

void parser_init(struct parser *, FILE *);
void parser_deinit(struct parser *);

void parser_peek(struct parser *);
void parser_next(struct parser *);

#endif
