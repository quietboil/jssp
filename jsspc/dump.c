#include "jsspc.h"
#include <jspp.h>
#include <stdio.h>

void path_state_dump(path_state_t * state)
{
    if (state->type == ACTION) {
        printf("%3d: %.*s -> %d\n\n", state->no, state->data_cb_name_len, state->data_cb_name, state->next_state->no);
    } else if (state->type == ARRAY_ELEMENT_ACTION) {
        printf("%3d: [%.*s] -> %d\n\n", state->no, state->data_cb_name_len, state->data_cb_name, state->next_state->no);
    } else {
        printf("%3d:\n", state->no);
        for (int i = 0; i < state->match_cnt; i++) {
            switch (state->matches[i]) {
                case JSON_OBJECT_BEGIN: {
                    printf("     { -> %u\n", state->goto_states[i]->no);
                    break;
                }
                case JSON_OBJECT_END: {
                    printf("     } -> %u\n", state->goto_states[i]->no);
                    break;
                }
                case JSON_ARRAY_BEGIN: {
                    printf("     [ -> %u\n", state->goto_states[i]->no);
                    break;
                }
                case JSON_ARRAY_END: {
                    printf("     ] -> %u\n", state->goto_states[i]->no);
                    break;
                }
                case JSON_END: {
                    printf("   END -> %u\n", state->goto_states[i]->no);
                    break;
                }
                default: {
                    printf("   %3d -> %u\n", state->matches[i], state->goto_states[i]->no);
                }
            }
        }
        printf("\n");

        for (int i = 0; i < state->match_cnt; i++) {
            path_state_t * next_state = state->goto_states[i];
            if (next_state->no > state->no) {
                path_state_dump(next_state);
            }
        }
    }
}

void name_state_dump(name_state_t * state)
{
    if (state->match_cnt > 0) {
        printf("%3d:\n", state->no);
        for (int i = 0; i < state->match_cnt; i++) {
            printf("   '%c' -> %u\n", state->matches[i], state->goto_states[i]->no);
        }
        printf("\n");

        for (int i = 0; i < state->match_cnt; i++) {
            name_state_dump(state->goto_states[i]);
        }
    }
}
