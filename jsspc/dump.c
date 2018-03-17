#include "jsspc.h"
#include <jspp.h>

void path_state_dump(FILE * out, path_state_t * state)
{
    if (state->type == ACTION) {
        fprintf(out,"%3d: %.*s -> %d\n\n", state->no, state->data_cb_name_len, state->data_cb_name, state->next_state->no);
    } else if (state->type == ARRAY_ELEMENT_ACTION) {
        fprintf(out,"%3d: [%.*s] -> %d\n\n", state->no, state->data_cb_name_len, state->data_cb_name, state->next_state->no);
    } else if (state->match_cnt > 0) {
        fprintf(out,"%3d:\n", state->no);
        for (int i = 0; i < state->match_cnt; i++) {
            switch (state->matches[i]) {
                case JSON_OBJECT_BEGIN: {
                    fprintf(out,"     { -> %u\n", state->goto_states[i]->no);
                    break;
                }
                case JSON_OBJECT_END: {
                    fprintf(out,"     } -> %u\n", state->goto_states[i]->no);
                    break;
                }
                case JSON_ARRAY_BEGIN: {
                    fprintf(out,"     [ -> %u\n", state->goto_states[i]->no);
                    break;
                }
                case JSON_ARRAY_END: {
                    fprintf(out,"     ] -> %u\n", state->goto_states[i]->no);
                    break;
                }
                case JSON_END: {
                    fprintf(out,"   END -> %u\n", state->goto_states[i]->no);
                    break;
                }
                default: {
                    fprintf(out,"   %3d -> %u\n", state->matches[i], state->goto_states[i]->no);
                }
            }
        }
        fprintf(out,"\n");

        for (int i = 0; i < state->match_cnt; i++) {
            path_state_t * next_state = state->goto_states[i];
            if (next_state->no > state->no) {
                path_state_dump(out, next_state);
            }
        }
    }
}

void name_state_dump(FILE * out, name_state_t * state)
{
    if (state->match_cnt > 0) {
        fprintf(out,"%3d:\n", state->no);
        for (int i = 0; i < state->match_cnt; i++) {
            fprintf(out,"   '%c' -> %u\n", state->matches[i], state->goto_states[i]->no);
        }
        fprintf(out,"\n");

        for (int i = 0; i < state->match_cnt; i++) {
            name_state_dump(out, state->goto_states[i]);
        }
    }
}
