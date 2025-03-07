#include "tokenizer.h"
#include <stdio.h>

int main(void) {
  struct tokenizer tokenizer;
  tokenizer_init(&tokenizer);

  for (;;) {
    tokenizer_feed(&tokenizer, stdin);
    if (tokenizer.token_type == tt_eof) {
      break;
    }
    if (tokenizer.token_type == tt_error) {
      printf("Error token at %zu:%zu: %s", tokenizer.start_line,
             tokenizer.start_column, tokenizer.literal);
      break;
    }
    printf("Token %s\n", tokenizer.literal);
  }

  tokenizer_deinit(&tokenizer);
  return 0;
}
