#include "ast.h"
#include "eval.h"
#include "parser.h"
#include "stdio.h"
#include "tokenizer.h"
#include "value.h"

int run_repl(void);
int run_script(char *filename);

int main(int argc, char *argv[]) {
  switch (argc) {
  case 1:
    return run_repl();
  case 2:
    return run_script(argv[1]);
  }

  return 0;
}

int run_repl(void) {
  for (;;) {
    fputs("$ ", stdout);
    // Read
    struct parser parser;
    parser_init(&parser, stdin);
    struct ast_node *ast = NULL;
    if (parse_statement(&parser, &ast)) {
      printf("Failed to parse\n");
      goto cleanup_read;
    }

    // Eval
    struct value v = eval(ast);

    // Print
    if (v.type != vt_null) {
      value_print(stdout, v);
      fputs("\n", stdout);
    }

    // Cleanup
    value_dec_ref(&v);

  cleanup_read:
    value_dec_ref(&v);
    parser_deinit(&parser);
    ast_node_free(ast);
  }
  return 0;
}

int run_script(char *filename) {
  FILE *input_file = fopen(filename, "r");
  if (!input_file) {
    fputs("Failed to open script file\n", stderr);
    return 1;
  }
  struct parser parser;
  parser_init(&parser, input_file);
  struct ast_node *ast = NULL;
  if (parse_statement_list(&parser, &ast)) {
    printf("Failed to parse\n");
  } else {
    struct value v = eval(ast);
    value_dec_ref(&v);
  }

  parser_deinit(&parser);
  ast_node_free(ast);
  fclose(input_file);
  return 0;
}
