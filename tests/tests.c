#include "test.h"
#include "hello_world.h"
#include <string.h>
#include <stdio.h>

static inline size_t min(size_t s1, size_t s2)
{
    return s1 <= s2 ? s1 : s2;
}

typedef struct _hello_world_result {
    char action[8];
    char target[8];
} hello_world_result_t;

void hello_world_set_action(const char * data, uint16_t data_length, uint8_t data_type, const jssp_context_t * context)
{
    hello_world_result_t * result = context->result;
    uint16_t cpylen = min(data_length, sizeof(result->action) - 1);
    strncpy(result->action, data, cpylen);
    result->action[cpylen] = '\0';
}

void hello_world_set_target(const char * data, uint16_t data_length, uint8_t data_type, const jssp_context_t * context)
{
    hello_world_result_t * result = context->result;
    uint16_t cpylen = min(data_length, sizeof(result->target) - 1);
    strncpy(result->target, data, cpylen);
    result->target[cpylen] = '\0';
}

static int parse_hello_world()
{
    static const char json[] = "{ \"action\": \"Hello\", \"target\": \"World\", \"qualifications\": [\"Brave\", \"New\"] }";

    hello_world_result_t result;
    jssp_t parser;
    jssp_init(&parser, hello_world_next_path_state, hello_world_name_next_state);

    check(JSON_END == jssp_start(&parser, &result, json, sizeof(json) - 1));
    check(strcmp(result.action, "Hello") == 0);
    check(strcmp(result.target, "World") == 0);

    // Fragmented JSON.
    // Note that the data callback (above) are not written to handle partial strings,
    // so we pretend for this test that the split happens elsewhere
    static const char json11[] = "{ \"action\": \"Hello\", \"tar";
    static const char json12[] = "get\": \"World\", \"qualificat";
    static const char json13[] = "ions\": [\"Brave\", \"New\"] }";

    hello_world_result_t result1;
    check(JSON_CONTINUE == jssp_start(&parser, &result1, json11, sizeof(json11) - 1));
    check(JSON_CONTINUE == jssp_continue(&parser, json12, sizeof(json12) - 1));
    check(JSON_END == jssp_continue(&parser, json13, sizeof(json13) - 1));
    check(strcmp(result1.action, "Hello") == 0);
    check(strcmp(result1.target, "World") == 0);

    return 0;
}

int main()
{
    test(parse_hello_world, "Parse 'Hello World'");

    printf("DONE: %d/%d\n", num_tests_passed, num_tests_passed + num_tests_failed);
    return num_tests_failed > 0;
}