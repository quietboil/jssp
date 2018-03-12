#include "test.h"
#include "hello_world.h"
#include <string.h>
#include <stdio.h>

static inline size_t min(size_t s1, size_t s2)
{
    return s1 <= s2 ? s1 : s2;
}

/**
 * Structure to store data extracted from the "Hello, World" JSON.
 * It stores string length and does not store or care about the
 * terminating nil character. Keeping string length make it easy
 * to coalesce split data.
 */
typedef struct _hello_world_result {
    uint8_t action_len;
    char    action[7];
    uint8_t target_len;
    char    target[7];
} hello_world_result_t;

static inline void hello_world_result_init(hello_world_result_t * result)
{
    result->action_len = 0;
    result->target_len = 0;
}

void hello_world_set_action(const char * data, uint16_t data_length, uint8_t data_type, const jssp_context_t * context)
{
    hello_world_result_t * result = context->result;
    uint16_t cpylen = min(data_length, sizeof(result->action) - result->action_len);
    strncpy(result->action + result->action_len, data, cpylen);
    result->action_len += data_length;
}

void hello_world_set_target(const char * data, uint16_t data_length, uint8_t data_type, const jssp_context_t * context)
{
    hello_world_result_t * result = context->result;
    uint16_t cpylen = min(data_length, sizeof(result->target) - result->target_len);
    strncpy(result->target + result->target_len, data, cpylen);
    result->target_len += data_length;
}

int parse_hello_world()
{
    static const char json[] = "{ \"action\": \"Hello\", \"target\": \"World\", \"qualifications\": [\"Brave\", \"New\"] }";

    hello_world_result_t result;
    hello_world_result_init(&result);

    jssp_t parser;
    jssp_init(&parser, hello_world_path_next_state, hello_world_name_next_state);

    check(JSON_END == jssp_start(&parser, &result, json, sizeof(json) - 1));
    check(result.action_len == 5 && strncmp("Hello", result.action, result.action_len) == 0);
    check(result.target_len == 5 && strncmp("World", result.target, result.target_len) == 0);

    // Fragmented JSON.
    static const char json11[] = "{ \"action\": \"Hello\", \"tar";
    static const char json12[] = "get\": \"World\", \"qualificat";
    static const char json13[] = "ions\": [\"Brave\", \"New\"] }";

    hello_world_result_t result1;
    hello_world_result_init(&result1);

    check(JSON_CONTINUE == jssp_start(&parser, &result1, json11, sizeof(json11) - 1));
    check(JSON_CONTINUE == jssp_continue(&parser, json12, sizeof(json12) - 1));
    check(JSON_END == jssp_continue(&parser, json13, sizeof(json13) - 1));
    check(result1.action_len == 5 && strncmp("Hello", result1.action, result1.action_len) == 0);
    check(result1.target_len == 5 && strncmp("World", result1.target, result1.target_len) == 0);

    return 0;
}
