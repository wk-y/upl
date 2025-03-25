add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tokenizer_first.h
    COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/tokenizer_first.sh
    DEPENDS tokenizer_first.sh
)

add_library(Ast ast.c)
add_library(Builtins builtins.c)
add_library(Eval eval.c)
add_library(Parser parser.c)
add_library(Stack stack.c)
add_library(Tokenizer tokenizer.c ${CMAKE_CURRENT_BINARY_DIR}/tokenizer_first.h)
add_library(Value value.c)

target_include_directories(Tokenizer PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(Eval PUBLIC m Builtins Stack)
target_link_libraries(Value PUBLIC Ast)
