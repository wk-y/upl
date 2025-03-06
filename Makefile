VPATH = src
.PHONY : run test test_ast

CFLAGS = -MMD -fsanitize=address -g -Og -std=c17 -Wall -Wextra -Wpedantic

objects = ast.o eval.o parser.o tokenizer.o value.o builtins.o stack.o

upl : $(objects)

run : upl
	./upl

tokenizer_test : $(objects)

test_tokenizer : tokenizer_test
	./tokenizer_test

include $(wildcard *.d)
