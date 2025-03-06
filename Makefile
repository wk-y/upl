VPATH = src
.PHONY : run test test_ast

CFLAGS = -MMD -fsanitize=address -g -Og -std=c17 -Wall -Wextra -Wpedantic
LDFLAGS = -lm

objects = ast.o eval.o parser.o tokenizer.o value.o builtins.o stack.o

upl : $(objects)

tokenizer.o : tokenizer_first.h

tokenizer_first.h: tokenizer_first.sh
	src/tokenizer_first.sh > tokenizer_first.h

run : upl
	./upl

tokenizer_test : $(objects)

test_tokenizer : tokenizer_test
	./tokenizer_test

include $(wildcard *.d)
