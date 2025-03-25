# Untitled Programming Language (UPL)

This is a toy programming language.

All operators are infix and right-to-left associative.
For operators that only use the value on one side, it is convenient to use one of the single-character operators as a filler (ex `,`)

```
,print "Hello World!\n";
```

For more example code, see the `examples` directory.

See `builtins_load_all` in builtins.c for the list of builtins.

## Building

```
src/tokenizer_first.sh > tokenizer_first.h
cmake -S . -B build
make
```

## Grammar

```
<statement_list> ::= <statement> [<semicolon> [<statement_list>]]
<statement> ::= <expr> [<expr> <statement>]
<expr> ::= <atom> | "(" <statement> ")"
<atom> ::= <symbol> | <number> | <string>
```
