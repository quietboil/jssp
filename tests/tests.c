#include "test.h"
#include <stdio.h>

int parse_hello_world();
int parse_hello_jsspc();
int parse_simple_array();
int parse_embedded_array();
int parse_array_of_objects();

int main()
{
    test(parse_hello_world, "Parse 'Hello, World'");
    test(parse_hello_jsspc, "Parse 'Hello, Jsspc'");
    test(parse_simple_array, "Parse simple array");
    test(parse_embedded_array, "Parse object with an array of values member");
    test(parse_array_of_objects, "Parse object with an array of objects member");

    printf("DONE: %d/%d\n", num_tests_passed, num_tests_passed + num_tests_failed);
    return num_tests_failed > 0;
}