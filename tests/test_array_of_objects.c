#include "test.h"
#include "utils.h"
#include "array_of_objects.h"
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
} array_of_objects_result_t;

static inline void array_of_objects_result_init(array_of_objects_result_t * result)
{
    result->num_types = 0;
    result->num_received = 0;
    for (int i = 0; i < RESULT_CAPACITY; i++) {
        result->type_name_lengths[i] = 0;
    }
}

///< Helper function to convert a sequence of numeric characters into an integer
static uint8_t add(uint8_t val, const char * data, uint16_t data_length)
{
    const char * end = data + data_length;
    while (data < end && isdigit(*data)) {
        val = val * 10 + (*data++ - '0');
    }
    return val;
}

void array_of_objects_set_response_size(const char * data, uint16_t data_length, uint8_t data_type, const jssp_context_t * context)
{
    array_of_objects_result_t * result = context->result;
    result->num_types = add(result->num_types, data, data_length);
}

void array_of_objects_set_type_name(const char * data, uint16_t data_length, uint8_t data_type, const jssp_context_t * context)
{
    array_of_objects_result_t * result = context->result;
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

int parse_array_of_objects()
{
    const char json[] = 
        "{ \"response_size\": 5,"
        "  \"response\": [ "
        "     { \"type\": \"R2-D2\",  \"description\": \"Astromech\" },"
        "     { \"type\": \"C-3PO\",  \"description\": \"Protocol\" },"
        "     { \"type\": \"BB-8\",   \"description\": \"Astromech\" },"
        "     { \"type\": \"R5-D4\",  \"description\": \"Astromech\" },"
        "     { \"type\": \"C1-10P\", \"description\": \"Astromech\" }"
        "  ]"
        "}";

    jssp_t parser;
    jssp_init(&parser, array_of_objects_path_next_state, array_of_objects_name_next_state);

    array_of_objects_result_t result;
    array_of_objects_result_init(&result);

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
