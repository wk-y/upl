# Untitled Programming Language (UPL)

This is a toy programming language.

Grammar: 
```
<program> ::= {<statement>}
<statement> ::= <expr> [<infix> <statement>]
<expr> ::= <atom> | "(" <statement> ")"
```
