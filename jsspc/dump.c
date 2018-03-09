#include "jsspc.h"
#include <jspp.h>
#include <stdio.h>

void dump_path_state(state_t * state)
{
    if (state->type == ACTION) {
        const char * fmt = state->array_elem_cb ? "%3d: [%.*s] -> %d\n\n" : "%3d: %.*s -> %d\n\n";
        printf(fmt, state->no, state->data_cb_name_len, state->data_cb_name, state->parent_state->no);
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
                default: {
                    printf("   %3d -> %u\n", state->matches[i], state->goto_states[i]->no);
                }
            }
        }
        printf("\n");

        for (int i = 0; i < state->match_cnt; i++) {
            state_t * next_state = state->goto_states[i];
            if (next_state->no > state->no) {
                dump_path_state(next_state);
            }
        }
    }
}

void dump_name_state(state_t * state)
{
    if (state->match_cnt > 0) {
        printf("%3d:\n", state->no);
        for (int i = 0; i < state->match_cnt; i++) {
            printf("   '%c' -> %u\n", state->matches[i], state->goto_states[i]->no);
        }
        printf("\n");

        for (int i = 0; i < state->match_cnt; i++) {
            dump_name_state(state->goto_states[i]);
        }
    }
}

