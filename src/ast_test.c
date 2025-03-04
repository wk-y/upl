#include "ast.h"
#include "parser.h"
#include "stdio.h"

int main(void) {
  struct parser parser;
  parser_init(&parser, stdin);
  struct ast_node *ast = NULL;
  if (parse_statement_list(&parser, &ast)) {
    printf("Error encountered\n");
  } else {
    ast_print(stdout, ast);
  }
  parser_deinit(&parser);
  ast_node_free(ast);
  fputs("\n", stdout);

  return 0;
}
