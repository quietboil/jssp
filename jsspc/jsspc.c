#include "jsspc.h"
#include <jspp.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _states_stack {
    path_state_t * states[32];
    int       top;
} state_stack_t;

static inline void state_stack_init(state_stack_t * stack)
{
    stack->top = -1;
}

static void state_stack_push(state_stack_t * stack, path_state_t * state)
{
    if (++stack->top == sizeof(stack->states) / sizeof(path_state_t *)) {
        printf("State stack overflow - JSON has too many nesting levels\n");
        exit(3);
    }
    stack->states[stack->top] = state;
}

static path_state_t * state_stack_peek(state_stack_t * stack)
{
    return stack->states[stack->top];
}

static path_state_t * state_stack_peek_at(state_stack_t * stack, uint32_t from_top)
{
    int i = stack->top - from_top;
    if (i < 0) {
        printf("Peeking below the bottom of the state stack\n");
        exit(4);
    }
    return stack->states[i];
}

static void state_stack_reduce(state_stack_t * stack)
{
    if (--stack->top < 0) {
        printf("State stack underflow\n");
        exit(4);
    }
}

static path_state_t * state_stack_pop(state_stack_t * stack)
{
    path_state_t * state = state_stack_peek(stack);
    state_stack_reduce(stack);
    return state;
}

static path_state_t * state_stack_pop_and_peek(state_stack_t * stack)
{
    return --stack->top >= 0 ? state_stack_peek(stack) : NULL;
}


int main(int argc, char * argv[])
{
    if (argc != 2) {
        printf("Usage: %s jssp_spec.json\n", argv[0]);
        return 1;
    }

    uint16_t spec_text_len;
    const char * spec_text = read_file(argv[1], &spec_text_len);
    if (!spec_text) {
        printf("Failed to read %s\n", argv[1]);
        return 2;
    }

    // while the spec is being parsed we need to collect 2 sets of data:
    // - object member names (to build the name matching automaton) and
    // - paths to data (to build the path matching automaton)
    jspp_t parser;
    uint8_t t = jspp_start(&parser, spec_text, spec_text_len);
    if (t != JSON_OBJECT_BEGIN && t != JSON_ARRAY_BEGIN) {
        printf("Unsupported spec: top level JSON element must be an object or an array\n");
        return 3;
    }

    name_state_t * name_start_state = name_state_create(1);
    uint32_t  last_name_state_no = 255;
    uint32_t  last_name_id = JSON_ARRAY_END;

    path_state_t * path_end_state = path_state_create(0); // 0 is a temporaty number for this state
    path_state_t * path_start_state = path_state_create(1);
    uint32_t  last_path_state_no = 1;

    state_stack_t path_state_stack;
    state_stack_init(&path_state_stack);
    state_stack_push(&path_state_stack, path_end_state);
    state_stack_push(&path_state_stack, path_start_state);

    while (t > JSON_CONTINUE) {
        switch (t) {
            case JSON_OBJECT_BEGIN: {
                path_state_t * curr_state = state_stack_peek(&path_state_stack);
                path_state_t * next_state = path_state_add_match(curr_state, t, &last_path_state_no);
                state_stack_push(&path_state_stack, next_state);
                break;
            }
            case JSON_ARRAY_BEGIN: {
                path_state_t * curr_state = state_stack_peek(&path_state_stack);
                path_state_t * next_state = path_state_add_match(curr_state, t, &last_path_state_no);
                next_state->type = MATCH_ARRAY_ELEMENT;
                state_stack_push(&path_state_stack, next_state);
                break;
            }
            case JSON_OBJECT_END: {
                path_state_t * curr_state = state_stack_pop(&path_state_stack); // after this pop it is at the state that matched {
                path_state_t * next_state = state_stack_peek(&path_state_stack);
                if (next_state->type != MATCH_ARRAY_ELEMENT) {
                    next_state = state_stack_pop_and_peek(&path_state_stack);
                }
                path_state_add_goto_on_match(curr_state, t, next_state);
                break;
            }
            case JSON_ARRAY_END: {
                path_state_t * curr_state = state_stack_pop(&path_state_stack); // after this pop it is at the state that matched {
                path_state_t * next_state = state_stack_peek(&path_state_stack);
                if (next_state->type != MATCH_ARRAY_ELEMENT) {
                    next_state = state_stack_pop_and_peek(&path_state_stack);
                }
                if (curr_state->type == MATCH_ARRAY_ELEMENT) {
                    path_state_add_goto_on_match(curr_state, t, next_state);
                }
                break;
            }
            case JSON_MEMBER_NAME: {
                uint16_t name_len;
                const char * name = jspp_text(&parser, &name_len);
                name_state_t * name_match_final_state = build_name_matcher(name_start_state, &last_name_state_no, &last_name_id, name, name_len);
                path_state_t * curr_state = state_stack_peek(&path_state_stack);
                path_state_t * next_state = path_state_add_match(curr_state, name_match_final_state->no, &last_path_state_no);
                state_stack_push(&path_state_stack, next_state);
                break;
            }
            case JSON_STRING: {
                uint16_t data_cb_name_len;
                const char * data_cb_name = jspp_text(&parser, &data_cb_name_len);
                path_state_t * curr_state = state_stack_peek(&path_state_stack);
                curr_state->type = curr_state->type == MATCH_ARRAY_ELEMENT ? ARRAY_ELEMENT_ACTION : ACTION;
                curr_state->data_cb_name = data_cb_name;
                curr_state->data_cb_name_len = data_cb_name_len;
                if (curr_state->type == ACTION) {
                    curr_state->next_state = state_stack_pop_and_peek(&path_state_stack);
                } else {
                    curr_state->next_state = state_stack_peek_at(&path_state_stack, 1);
                    if (curr_state->next_state->type != MATCH_ARRAY_ELEMENT) {
                        curr_state->next_state = state_stack_peek_at(&path_state_stack, 2);
                    }
                }
                break;
            }
            default: {
                uint16_t text_len;
                const char * text = jspp_text(&parser, &text_len);
                printf("Unexpected element %.*s\n", text_len, text);
                return 3;
            }
        }
        t = jspp_next(&parser);
    }
    path_end_state->no = ++last_path_state_no;

    write_automata(argv[1], name_start_state, path_start_state);

    return 0;
}
