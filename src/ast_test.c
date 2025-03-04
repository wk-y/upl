#include "ast.h"
#include "eval.h"
#include "parser.h"
#include "stdio.h"
#include "value.h"

int main(void) {
  struct parser parser;
  parser_init(&parser, stdin);
  struct ast_node *ast = NULL;
  if (parse_statement_list(&parser, &ast)) {
    printf("Error encountered\n");
  } else {
    ast_print(stdout, ast);
    fputs("\n", stdout);
    struct value v = eval(ast);
    value_dec_ref(&v);
    fputs("\n", stdout);
  }
  parser_deinit(&parser);
  ast_node_free(ast);
  fputs("\n", stdout);

  return 0;
}
