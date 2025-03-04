VPATH = src
.PHONY : run test test_ast

CFLAGS = -MMD -fsanitize=address -g -Og -std=c17 -Wall -Wextra -Wpedantic

objects = ast.o eval.o parser.o tokenizer.o value.o

ast_test : $(objects)

test_ast : ast_test
	./ast_test

tokenizer_test : $(objects)

test_tokenizer : tokenizer_test
	./tokenizer_test

include $(wildcard *.d)
