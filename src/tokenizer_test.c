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
    printf("Token %s\n", tokenizer.literal);
  }

  tokenizer_deinit(&tokenizer);
  return 0;
}
