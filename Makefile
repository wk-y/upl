VPATH = src
.PHONY : run test test_ast

CFLAGS = -fsanitize=address -g -Og -std=c17 -Wall -Wextra -Wpedantic

ast_test : ast.o parser.o tokenizer.o
ast.o : parser.o tokenizer.o
parser.o : tokenizer.o

test_ast : ast_test
	./ast_test

tokenizer_test : tokenizer.o

test_tokenizer : tokenizer_test
	./tokenizer_test


