#include "test.h"
#include <stdio.h>

int parse_hello_world();
int parse_hello_jsspc();
int parse_simple_array();
int parse_embedded_array();

int main()
{
    test(parse_hello_world, "Parse 'Hello, World'");
    test(parse_hello_jsspc, "Parse 'Hello, Jsspc'");
    test(parse_simple_array, "Parse simple array");
    test(parse_embedded_array, "Parse object with array member");

    printf("DONE: %d/%d\n", num_tests_passed, num_tests_passed + num_tests_failed);
    return num_tests_failed > 0;
}