#include "ast.h"
#include "parser.h"
#include "stdio.h"

int main(void) {
  struct parser parser;
  parser_init(&parser, stdin);
  struct ast_node *ast = ast_node_ref(parse_ast_node(&parser));
  parser_deinit(&parser);
  if (ast) {
    ast_print(stdout, ast);
  } else {
    printf("No ast???\n");
  }
  ast_node_deref(ast);
  fputs("\n", stdout);

  return 0;
}
