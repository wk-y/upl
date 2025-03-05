# Untitled Programming Language (UPL)

This is a toy programming language.

## Builtins

- **,** - Construct (LHS, RHS)
- **and** - Construct (LHS, (RHS, NULL))
- **print** - Print RHS (ignores LHS)
- **while** - while the LHS evaluates to true, evaluate the RHS.
- **=** - while the LHS evaluates to true, evaluate the RHS.
- **<** - 1 if LHS < RHS, else 0.
- **+** - LHS + RHS
- **\*** - LHS * RHS

## Grammar

```
<statement_list> ::= <statement> [<semicolon> [<statement_list>]]
<statement> ::= <expr> [<symbol> <statement>]
<expr> ::= <atom> | "(" <statement> ")"
<atom> ::= <symbol> | <number> | <string>
```
