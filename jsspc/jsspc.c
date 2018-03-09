#include "jsspc.h"
#include <jspp.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _states_stack {
    state_t * states[32];
    int       top;
} state_stack_t;

static inline void state_stack_init(state_stack_t * stack)
{
    stack->top = -1;
}

static void state_stack_push(state_stack_t * stack, state_t * state)
{
    if (++stack->top == sizeof(stack->states) / sizeof(state_t *)) {
        printf("State stack overflow - JSON has too many nesting levels\n");
        exit(3);
    }
    stack->states[stack->top] = state;
}

static state_t * state_stack_peek(state_stack_t * stack)
{
    return stack->states[stack->top];
}

static state_t * state_stack_peek_at(state_stack_t * stack, uint32_t from_top)
{
    int i = stack->top - from_top;
    if (i < 0) {
        printf("Looking too deep into the state stack\n");
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

static state_t * state_stack_pop(state_stack_t * stack)
{
    state_t * state = state_stack_peek(stack);
    state_stack_reduce(stack);
    return state;
}

static state_t * state_stack_pop_and_peek(state_stack_t * stack)
{
    return --stack->top >= 0 ? state_stack_peek(stack) : NULL;
}


typedef struct _container_stack {
    uint32_t levels;
    uint32_t pointer;
} container_stack_t;

static inline void container_stack_init(container_stack_t * stack)
{
    stack->levels = 0;
    stack->pointer = 1;
}

static inline void container_stack_push_object(container_stack_t * stack)
{
    stack->pointer <<= 1;
    stack->levels &= ~stack->pointer;
}

static inline void container_stack_push_array(container_stack_t * stack)
{
    stack->pointer <<= 1;
    stack->levels |= stack->pointer;
}

static inline void container_stack_push(container_stack_t * stack, uint8_t begin_token)
{
    if (begin_token == JSON_OBJECT_BEGIN) {
        container_stack_push_object(stack);
    } else {
        container_stack_push_array(stack);
    }
}

static inline bool container_stack_peek_is_array(container_stack_t * stack)
{
    return stack->levels & stack->pointer;
}

static inline void container_stack_pop(container_stack_t * stack)
{
    stack->pointer >>= 1;
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

    state_t * name_init_state = state_create(MATCH, 1);
    uint32_t  last_name_state_no = 255;
    uint32_t  last_name_id = JSON_ARRAY_END;

    state_t * path_init_state = state_create(MATCH, 1);
    uint32_t  last_path_state_no = 1;

    state_stack_t path_state_stack;
    state_stack_init(&path_state_stack);
    state_stack_push(&path_state_stack, path_init_state);

    container_stack_t container_stack;
    container_stack_init(&container_stack);

    while (t > JSON_CONTINUE) {
        switch (t) {
            case JSON_OBJECT_BEGIN:
            case JSON_ARRAY_BEGIN: {
                state_t * curr_state = state_stack_peek(&path_state_stack);
                state_t * next_state = state_add_match(curr_state, t, &last_path_state_no);
                state_stack_push(&path_state_stack, next_state);
                container_stack_push(&container_stack, t);
                break;
            }
            case JSON_OBJECT_END:
            case JSON_ARRAY_END: {
                container_stack_pop(&container_stack);
                state_t * curr_state = state_stack_pop(&path_state_stack);
                state_t * next_state = container_stack_peek_is_array(&container_stack)
                                     ? state_stack_peek(&path_state_stack)
                                     : state_stack_pop_and_peek(&path_state_stack);
                if (!next_state) {
                    next_state = path_init_state;
                }
                state_add_goto_on_match(curr_state, t, next_state);
                break;
            }
            case JSON_MEMBER_NAME: {
                uint16_t name_len;
                const char * name = jspp_text(&parser, &name_len);
                state_t * name_match_final_state = state_build_name_matcher(name_init_state, &last_name_state_no, &last_name_id, name, name_len);
                state_t * curr_state = state_stack_peek(&path_state_stack);
                state_t * next_state = state_add_match(curr_state, name_match_final_state->no, &last_path_state_no);
                state_stack_push(&path_state_stack, next_state);
                break;
            }
            case JSON_STRING: {
                uint16_t data_cb_name_len;
                const char * data_cb_name = jspp_text(&parser, &data_cb_name_len);
                state_t * curr_state = state_stack_peek(&path_state_stack);
                curr_state->type = ACTION;
                curr_state->data_cb_name = data_cb_name;
                curr_state->data_cb_name_len = data_cb_name_len;
                curr_state->array_elem_cb = container_stack_peek_is_array(&container_stack);
                if (curr_state->array_elem_cb) {
                    curr_state->parent_state = state_stack_peek_at(&path_state_stack, 1);
                } else {
                    curr_state->parent_state = state_stack_pop_and_peek(&path_state_stack);
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

    write_automata(argv[1], name_init_state, path_init_state);

    return 0;
}
