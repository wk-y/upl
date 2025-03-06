# Generate Tokenizer FIRST macros;
#
# Grammar
# <parse> ::= <whitespace> <token>
# <token> ::= ['-']<float> | <literal> | <symbol> | <lpar> | <rpar> | <semicolon>
# <float> ::= '.' <digit> {<digit>} | <digit> {<digit} '.' {<digit>}
#
# The grammar isn't quite LL1, as the '-' symbol can start either a negative number or a symbol.
# literals are one or more alphanumeric characters, and symbols are one or more symbols characters. 

print_first() {
    printf "#define CASE_FIRST_$1";
    shift;
    for c in "$@"; do
        printf " case '%s':" "$c";
    done
    printf '\n';
}

cat << 'EOF'
// Generated file, do see tokenizer_first.sh

EOF

print_first "WHIESPACE" ' ' '\t' '\n'
print_first "SYMBOL" + - \* / \, \= \! \< \>
print_first "LITERAL" {a..z} {A..Z} _
print_first "DIGITS" {0..9}
print_first "LPAR" '('
print_first "RPAR" ')'
print_first "SEMICOLON" ';'
print_first "STRING" '"'
printf "#define CASE_FIRST_FLOAT CASE_FIRST_DIGITS\n"
