#include "hello_world.h"

/*  Parser Template:
    {
        "action": "set_action",
        "target": "set_target"
    }
*/

uint32_t hello_world_name_next_state(uint32_t state, char next_char)
{
    switch (state) {
        case 1: {
            switch (next_char) {
                case 'a': return 256;
                case 't': return 261;
            }
            break;
        }
        case 256: {
            if (next_char == 'c') return 257;
            break;
        }
        case 257: {
            if (next_char == 't') return 258;
            break;
        }
        case 258: {
            if (next_char == 'i') return 259;
            break;
        }
        case 259: {
            if (next_char == 'o') return 260;
            break;
        }
        case 260: {
            if (next_char == 'n') return ACTION;
            break;
        }
        case 261: {
            if (next_char == 'a') return 262;
            break;
        }
        case 262: {
            if (next_char == 'r') return 263;
            break;
        }
        case 263: {
            if (next_char == 'g') return 264;
            break;
        }
        case 264: {
            if (next_char == 'e') return 265;
            break;
        }
        case 265: {
            if (next_char == 't') return TARGET;
            break;
        }
    }
    return 0;
}

uint32_t hello_world_next_path_state(uint32_t state, uint8_t next_elem, data_cb_t * action)
{
    switch (state) {
        case 0: {
            if (next_elem == JSON_OBJECT_BEGIN) return 1;
            break;
        }
        case 1: {
            switch (next_elem) {
                case ACTION: return 2;
                case TARGET: return 3;
                case JSON_OBJECT_END: return UINT32_MAX;
            }
            break;
        }
        case 2: {
            if (next_elem == JSON_NUMBER_PART || next_elem == JSON_STRING_PART) {
                *action = hello_world_set_action;
                return 2;
            } else if (JSON_NULL <= next_elem && next_elem <= JSON_STRING) {
                *action = hello_world_set_action;
                return 1;
            }
            break;
        }
        case 3: {
            if (next_elem == JSON_NUMBER_PART || next_elem == JSON_STRING_PART) {
                *action = hello_world_set_target;
                return 3;
            } else if (JSON_NULL <= next_elem && next_elem <= JSON_STRING) {
                *action = hello_world_set_target;
                return 1;
            }
            break;
        }        
    }
    return 0;
}