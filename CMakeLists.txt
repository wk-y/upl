cmake_minimum_required(VERSION 3.10)

project(Upl)

add_subdirectory(src)

add_executable(upl src/upl.c)
target_link_libraries(upl PUBLIC Eval Parser Tokenizer Value)

