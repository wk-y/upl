# Untitled Programming Language (UPL)

This is a toy programming language.

Grammar:

```
<statement_list> ::= <statement> [<semicolon> [<statement_list>]]
<statement> ::= <expr> [<symbol> <statement>]
<expr> ::= <atom> | "(" <statement> ")"
<atom> ::= <symbol> | <number> | <string>
```
