#include "test.h"
#include "utils.h"
#include "hello_jsspc.h"
#include <string.h>
#include <stdio.h>

/**
 * Structure to store data extracted from the "Hello, JSSPC" JSON.
 */
typedef struct _hello_jsspc_result {
    uint8_t action_len;
    char    action[7];
    uint8_t target_len;
    char    target[7];
} hello_jsspc_result_t;

static inline void hello_jsspc_result_init(hello_jsspc_result_t * result)
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

int parse_hello_jsspc()
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
