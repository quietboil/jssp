#include "jsspc.h"
#include <jspp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

state_t * state_create(uint32_t type, uint32_t number)
{
    state_t * state  = calloc(1, sizeof(state_t));
    state->type = type;
    state->no = number;
    return state;
}

state_t * state_add_match(state_t * state, uint8_t match, uint32_t * state_no_gen)
{
    if (state->type != MATCH) {
        printf("Internal error - attempt to add a match to a non-match state\n");
        exit(4);
    }
    state_t * next_state = state_create(MATCH, ++*state_no_gen);
    return state_add_goto_on_match(state, match, next_state);
}

state_t * state_add_goto_on_match(state_t * state, uint8_t match, state_t * next_state)
{
    if (state->type != MATCH) {
        printf("Internal error - attempt to add a match to a non-match state\n");
        exit(4);
    }
    uint32_t i = state->match_cnt;
    ++state->match_cnt;

    state->matches = realloc(state->matches, sizeof(uint8_t) * state->match_cnt);
    state->goto_states = realloc(state->goto_states, sizeof(state_t*) * state->match_cnt);

    state->matches[i] = match;
    return state->goto_states[i] = next_state;
}

/**
 * Scans the state transitions for a specified match value.
 * \param  state  State to examine.
 * \param  match  Match value to locate.
 * \return Go-to state of the match transition if found. NULL otherwise.
 */
static state_t * state_get_goto_state(state_t * state, uint8_t match)
{
    if (state->type == MATCH) {
        uint8_t * mptr = state->matches;
        uint8_t * mend = state->matches + state->match_cnt;
        while (mptr != mend) {
            if (*mptr == match) {
                int i = mptr - state->matches;
                return state->goto_states[i];
            }
            ++mptr;
        }
    }
    return NULL;
}

state_t * state_build_name_matcher(state_t * state, uint32_t * state_no_gen, uint32_t * name_id_gen, const char * name, uint16_t name_len)
{
    const char * name_end = name + name_len;
    const char * last_chr = name_end - 1;
    while (name != name_end) {
        state_t * next_state = state_get_goto_state(state, *name);
        if (!next_state) {
            next_state = state_add_match(state, *name, name == last_chr ? name_id_gen : state_no_gen);
        }
        state = next_state;
        ++name;
    }
    return state;
}

