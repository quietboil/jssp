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

static inline uint32_t object_data(uint8_t json_token, uint32_t current_state, uint32_t next_state, data_cb_t * action_ptr, data_cb_t action_cb) {
    if (json_token == JSON_NUMBER_PART || json_token == JSON_STRING_PART) {
        *action_ptr = action_cb;
        return current_state;
    } else if (JSON_NULL <= json_token && json_token <= JSON_STRING) {
        *action_ptr = action_cb;
        return next_state;
    }
    return 0;
}

uint32_t hello_world_path_next_state(uint32_t state, uint8_t next_elem, data_cb_t * action)
{
    switch (state) {
        case 1: {
            if (next_elem == JSON_OBJECT_BEGIN) return 2;
            break;
        }
        case 2: {
            switch (next_elem) {
                case ACTION: return 3;
                case TARGET: return 4;
                case JSON_OBJECT_END: return 1;
            }
            break;
        }
        case 3: {
            return object_data(next_elem, 3, 2, action, hello_world_set_action);
        }
        case 4: {
            return object_data(next_elem, 4, 2, action, hello_world_set_target);
        }        
    }
    return 0;
}