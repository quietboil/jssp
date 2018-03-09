#include "test.h"
#include "hello_world.h"
#include "hello_jsspc.h"
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
    jssp_init(&parser, hello_world_path_next_state, hello_world_name_next_state);

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

/**
 * Structure to store data extracted from the "Hello, JSSPC" JSON.
 * Unlike "Hello, World" (above) this stores string length and does
 * not care about terminating character. Keeping string length make
 * it easy to coalesce split data.
 */
typedef struct _hello_jsspc_result {
    uint8_t action_len;
    char    action[7];
    uint8_t target_len;
    char    target[7];
} hello_jsspc_result_t;

static void hello_jsspc_result_init(hello_jsspc_result_t * result)
{
    result->action_len = 0;
    result->target_len = 0;
}

void hello_jsspc_set_action(const char * data, uint16_t data_length, uint8_t data_type, const jssp_context_t * context)
{
    hello_jsspc_result_t * result = context->result;    
    uint16_t cpylen = min(data_length, sizeof(result->action) - result->action_len);
    strncpy(result->action + result->action_len, data, cpylen);
    result->action_len += data_length;
}

void hello_jsspc_set_target(const char * data, uint16_t data_length, uint8_t data_type, const jssp_context_t * context)
{
    hello_jsspc_result_t * result = context->result;
    uint16_t cpylen = min(data_length, sizeof(result->target) - result->target_len);
    strncpy(result->target + result->target_len, data, cpylen);
    result->target_len += data_length;
}

static int parse_hello_jsspc()
{
    static const char json[] = "{ \"action\": \"Hello\", \"target\": \"World\", \"qualifications\": [\"Brave\", \"New\"] }";

    jssp_t parser;
    jssp_init(&parser, hello_jsspc_path_next_state, hello_jsspc_name_next_state);

    hello_jsspc_result_t result;

    hello_jsspc_result_init(&result);
    check(JSON_END == jssp_start(&parser, &result, json, sizeof(json) - 1));
    check(result.action_len == 5 && strncmp("Hello", result.action, result.action_len) == 0);
    check(result.target_len == 5 && strncmp("World", result.target, result.target_len) == 0);

    static const char json11[] = "{ \"action\": \"Hello\", \"tar";
    static const char json12[] = "get\": \"World\", \"qualificat";
    static const char json13[] = "ions\": [\"Brave\", \"New\"] }";

    hello_jsspc_result_t result1;
    hello_jsspc_result_init(&result1);
    check(JSON_CONTINUE == jssp_start(&parser, &result1, json11, sizeof(json11) - 1));
    check(JSON_CONTINUE == jssp_continue(&parser, json12, sizeof(json12) - 1));
    check(JSON_END == jssp_continue(&parser, json13, sizeof(json13) - 1));
    check(result1.action_len == 5 && strncmp("Hello", result1.action, result1.action_len) == 0);
    check(result1.target_len == 5 && strncmp("World", result1.target, result1.target_len) == 0);

    static const char json21[] = "{ \"target\": \"Worl";
    static const char json22[] = "d\", \"qualification";
    static const char json23[] = "s\": [\"Brave\", \"N";
    static const char json24[] = "ew\"], \"action\":\"";
    static const char json25[] = "Hello\" }";

    hello_jsspc_result_t result2;
    hello_jsspc_result_init(&result2);
    check(JSON_CONTINUE == jssp_start(&parser, &result2, json21, sizeof(json21) - 1));
    check(JSON_CONTINUE == jssp_continue(&parser, json22, sizeof(json22) - 1));
    check(JSON_CONTINUE == jssp_continue(&parser, json23, sizeof(json23) - 1));
    check(JSON_CONTINUE == jssp_continue(&parser, json24, sizeof(json24) - 1));
    check(JSON_END == jssp_continue(&parser, json25, sizeof(json25) - 1));
    check(result2.action_len == 5 && strncmp("Hello", result2.action, result2.action_len) == 0);
    check(result2.target_len == 5 && strncmp("World", result2.target, result2.target_len) == 0);

    return 0;
}

int main()
{
    test(parse_hello_world, "Parse 'Hello, World'");
    test(parse_hello_jsspc, "Parse 'Hello, Jsspc'");

    printf("DONE: %d/%d\n", num_tests_passed, num_tests_passed + num_tests_failed);
    return num_tests_failed > 0;
}