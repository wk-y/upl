VPATH = src
.PHONY : run clean test test_ast 

CFLAGS = -MMD -fsanitize=address -g -Og -std=c17 -Wall -Wextra -Wpedantic
LDLIBS = -lm

objects = ast.o eval.o parser.o tokenizer.o value.o builtins.o stack.o
exes = upl test_ast

upl :

$(exes) : $(objects)

tokenizer.o : tokenizer_first.h

tokenizer_first.h: tokenizer_first.sh
	src/tokenizer_first.sh > tokenizer_first.h

run : upl
	./upl

tokenizer_test : $(objects)

test_tokenizer : tokenizer_test
	./tokenizer_test

clean :
	$(RM) $(exes) ||: 
	$(RM) ./*.o ||:
	$(RM) ./*.d ||:
	$(RM) ./tokenizer_first.h ||:

include $(wildcard *.d)
