#include "test.h"
#include "utils.h"
#include "embedded_array.h"
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define RESULT_CAPACITY 6
#define MAX_NAME_LENGTH 8

typedef struct {
    uint8_t num_types;
    uint8_t num_received;
    uint8_t type_name_lengths[RESULT_CAPACITY];
    char    types[RESULT_CAPACITY][MAX_NAME_LENGTH];
} embedded_array_result_t;

static inline void embedded_array_result_init(embedded_array_result_t * result)
{
    result->num_types = 0;
    result->num_received = 0;
    for (int i = 0; i < RESULT_CAPACITY; i++) {
        result->type_name_lengths[i] = 0;
    }
}

///< Helper function to convert a sequence of numeric hcracters into an integer
static uint8_t add(uint8_t val, const char * data, uint16_t data_length)
{
    const char * end = data + data_length;
    while (data < end && isdigit(*data)) {
        val = val * 10 + (*data++ - '0');
    }
    return val;
}

void embedded_array_set_response_size(const char * data, uint16_t data_length, uint8_t data_type, const jssp_context_t * context)
{
    embedded_array_result_t * result = context->result;
    result->num_types = add(result->num_types, data, data_length);
}

void embedded_array_add_type(const char * data, uint16_t data_length, uint8_t data_type, const jssp_context_t * context)
{
    embedded_array_result_t * result = context->result;
    uint8_t i = result->num_received;
    if (i < RESULT_CAPACITY) {
        uint16_t cpylen = min(data_length, MAX_NAME_LENGTH - result->type_name_lengths[i]);
        strncpy(result->types[i] + result->type_name_lengths[i], data, cpylen);
        result->type_name_lengths[i] += data_length;
        if (data_type > JSON_STRING_PART) {
            ++result->num_received;
        }
    }
}

int parse_embedded_array()
{
    const char json[] = "{ \"num_types\": 5, \"types\": [ \"R2-D2\", \"C-3PO\", \"BB-8\", \"R5-D4\", \"C1-10P\" ] }";

    jssp_t parser;
    jssp_init(&parser, embedded_array_path_next_state, embedded_array_name_next_state);

    embedded_array_result_t result;
    embedded_array_result_init(&result);

    check(JSON_END == jssp_start(&parser, &result, json, sizeof(json) - 1));
    check(result.num_types == 5);    
    check(result.num_received == 5);    
    check(result.type_name_lengths[0] == 5 && strncmp("R2-D2",  result.types[0], result.type_name_lengths[0]) == 0);
    check(result.type_name_lengths[1] == 5 && strncmp("C-3PO",  result.types[1], result.type_name_lengths[1]) == 0);
    check(result.type_name_lengths[2] == 4 && strncmp("BB-8",   result.types[2], result.type_name_lengths[2]) == 0);
    check(result.type_name_lengths[3] == 5 && strncmp("R5-D4",  result.types[3], result.type_name_lengths[3]) == 0);
    check(result.type_name_lengths[4] == 6 && strncmp("C1-10P", result.types[4], result.type_name_lengths[4]) == 0);
    
    return 0;
}
