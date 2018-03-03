#ifndef __HELLO_WORLD_H
#define __HELLO_WORLD_H

#include <jssp.h>

uint32_t hello_world_name_next_state(uint32_t state, char next_char);
uint32_t hello_world_next_path_state(uint32_t state, uint8_t next_elem, data_cb_t * action);

enum _hello_world_names {
    ACTION = JSON_ARRAY_END + 1,
    TARGET
};

void hello_world_set_action(const char * data, uint16_t data_length, uint8_t data_type, const jssp_context_t * context);
void hello_world_set_target(const char * data, uint16_t data_length, uint8_t data_type, const jssp_context_t * context);

#endif