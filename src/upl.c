#include "ast.h"
#include "eval.h"
#include "parser.h"
#include "tokenizer.h"
#include "value.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  struct interpreter interpreter;
  interpreter_init(&interpreter);

  struct parser parser;
  parser_init(&parser, stdin);

  for (;;) {
    fputs("$ ", stdout);

    // Read
    struct ast_node *ast = NULL;
    int err = parse_statement(&parser, &ast);
    if (!err) {
      parser_peek(&parser);
      if (parser.tokenizer.token_type != tt_semicolon) {
        err = 1;
      } else {
        parser_next(&parser);
      }
    }
    if (err) {
      printf("Failed to parse\n");

      // Reset the parser
      parser_deinit(&parser);
      parser_init(&parser, stdin);

      int c;
      do {
        c = getc(stdin);
      } while (c != EOF && c != '\n');

      goto cleanup_read;
    }

    // Eval
    struct value v = interpreter_eval(&interpreter, ast);

    // Print
    if (v.type != vt_null) {
      value_print(stdout, v);
      fputs("\n", stdout);
    }

    // Cleanup
    value_dec_ref(&v);

  cleanup_read:
    ast_node_destroy(ast);
    free(ast);
  }

  parser_deinit(&parser);
  interpreter_deinit(&interpreter);
  return 0;
}

int run_script(char *filename) {
  int exit_code = 0;
  FILE *input_file = fopen(filename, "r");
  if (!input_file) {
    fputs("Failed to open script file\n", stderr);
    return 1;
  }
  struct parser parser;
  parser_init(&parser, input_file);
  struct ast_node *ast = NULL;
  if (parse_statement_list(&parser, &ast) ||
      (parser_peek(&parser), parser.tokenizer.token_type != tt_eof)) {
    fprintf(stderr, "%s:%zu:%zu: Parse error\n", filename,
            parser.tokenizer.start_line, parser.tokenizer.start_column);

    if (!fseek(input_file,
               -(long)(parser.tokenizer.start_column +
                       parser.tokenizer.token_source_chars),
               SEEK_CUR)) {
      // Print the error line
      for (size_t i = 0; i < parser.tokenizer.start_column; i++) {
        int c = getc(input_file);
        if (c == EOF) {
          goto print_err;
        }
        fputc(c, stderr);
      }

      fputs("\x1b[31m", stderr); // highlight red

      for (size_t i = 0; i < parser.tokenizer.token_source_chars; i++) {
        int c = getc(input_file);
        if (c == EOF) {
          goto print_err;
        }
        fputc(c, stderr);
      }

      fputs("\x1b[0m", stderr); // reset color

      for (;;) {
        int c = getc(input_file);
        if (c == EOF || c == '\n') {
          break;
        }
        fputc(c, stderr);
      }

      fputc('\n', stderr);
      for (size_t i = 0; i < parser.tokenizer.start_column; i++) {
        fputc(' ', stderr);
      }

      fprintf(stderr, "^ Unexpected %s\n",
              token_type_string(parser.tokenizer.token_type));
      if (0) {
      print_err:
        fputs("\x1b[0m", stderr); // reset color
        fprintf(stderr, "\nCould not print source code.\n");
      }
    }
    exit_code = 1;
  } else {
    struct value v = eval(ast);
    value_dec_ref(&v);
  }

  parser_deinit(&parser);
  ast_node_destroy(ast);
  free(ast);

  fclose(input_file);
  return exit_code;
}
