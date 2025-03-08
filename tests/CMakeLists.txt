include(CTest)

add_test(NAME Fibonacci COMMAND upl fibonacci.upl WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
