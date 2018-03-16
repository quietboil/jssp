#include "test.h"
#include "utils.h"
#include "simple_array.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define RESULT_CAPACITY 3
#define MAX_NAME_LENGTH 8

typedef struct {
    uint8_t num_names;
    uint8_t name_lengths[RESULT_CAPACITY];
    char    names[RESULT_CAPACITY][MAX_NAME_LENGTH];
} simple_array_result_t;

static inline void simple_array_result_init(simple_array_result_t * result)
{
    result->num_names = 0;
    for (int i = 0; i < RESULT_CAPACITY; i++) {
        result->name_lengths[i] = 0;
    }
}

void simple_array_add_name(const char * data, uint16_t data_length, uint8_t data_type, const jssp_context_t * context)
{
    simple_array_result_t * result = context->result;
    uint8_t i = result->num_names;
    if (i < RESULT_CAPACITY) {
        uint16_t cpylen = min(data_length, MAX_NAME_LENGTH - result->name_lengths[i]);
        strncpy(result->names[i] + result->name_lengths[i], data, cpylen);
        result->name_lengths[i] += data_length;
        if (data_type > JSON_STRING_PART) {
            ++result->num_names;
        }
    }
}

int parse_simple_array()
{
    const char json[] = "[ \"R2-D2\", \"C-3PO\", \"BB-8\" ]";

    jssp_t parser;
    jssp_init(&parser, simple_array_path_next_state, simple_array_name_next_state);

    simple_array_result_t result;
    simple_array_result_init(&result);

    check(JSON_END == jssp_start(&parser, &result, json, sizeof(json) - 1));
    check(result.num_names == 3);    
    check(result.name_lengths[0] == 5 && strncmp("R2-D2", result.names[0], result.name_lengths[0]) == 0);
    check(result.name_lengths[1] == 5 && strncmp("C-3PO", result.names[1], result.name_lengths[1]) == 0);
    check(result.name_lengths[2] == 4 && strncmp("BB-8",  result.names[2], result.name_lengths[2]) == 0);
    
    return 0;
}
