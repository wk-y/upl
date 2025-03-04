# Untitled Programming Language (UPL)

This is a toy programming language.

Grammar:

```
<statement_list> ::= <statement> [; [<statement_list>]]
<statement> ::= <expr> [<literal> <statement>]
<expr> ::= <atom> | "(" <statement> ")"
<atom> ::= <literal> | <number> | <string>
```
