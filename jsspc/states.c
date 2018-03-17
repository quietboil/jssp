#include "jsspc.h"
#include <jspp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

name_state_t * name_state_create(uint32_t number)
{
    name_state_t * state = calloc(1, sizeof(name_state_t));
    state->no = number;
    return state;
}

/**
 * Adds a new character match transition to the state where transition destination is an existing state.
 * \param  state       State to add a new match transition to.
 * \param  match       Character to match.
 * \param  next_state  State to go to when matched.
 * \return next_state
 */
static name_state_t * name_state_add_goto_on_match(name_state_t * state, char match, name_state_t * next_state)
{
    uint32_t i = state->match_cnt;
    ++state->match_cnt;

    state->matches = realloc(state->matches, sizeof(char) * state->match_cnt);
    state->goto_states = realloc(state->goto_states, sizeof(name_state_t*) * state->match_cnt);

    state->matches[i] = match;
    return state->goto_states[i] = next_state;
}

name_state_t * name_state_add_match(name_state_t * state, char match, uint32_t * state_no_gen)
{
    name_state_t * next_state = name_state_create(++*state_no_gen);
    return name_state_add_goto_on_match(state, match, next_state);
}

/**
 * Scans the state transitions for a specified match value.
 * \param  state  State to examine.
 * \param  match  Match value to locate.
 * \return Go-to state of the match transition if found. NULL otherwise.
 */
static name_state_t * name_state_get_goto_state(name_state_t * state, char match)
{
    const char * mptr = state->matches;
    const char * mend = state->matches + state->match_cnt;
    while (mptr != mend) {
        if (*mptr == match) {
            int i = mptr - state->matches;
            return state->goto_states[i];
        }
        ++mptr;
    }
    return NULL;
}

name_state_t * build_name_matcher(name_state_t * state, uint32_t * state_no_gen, uint32_t * name_id_gen, const char * name, uint16_t name_len)
{
    const char * name_end = name + name_len;
    const char * last_chr = name_end - 1;
    while (name != name_end) {
        name_state_t * next_state = name_state_get_goto_state(state, *name);
        if (!next_state) {
            next_state = name_state_add_match(state, *name, name == last_chr ? name_id_gen : state_no_gen);
        }
        state = next_state;
        ++name;
    }
    if (state->no >= 256) {
        // this name is a substring of a longer one that is already matched
        state->no = ++*name_id_gen;
    }
    return state;
}


path_state_t * path_state_create(uint32_t number)
{
    path_state_t * state  = calloc(1, sizeof(path_state_t));
    state->no = number;
    return state;
}

path_state_t * path_state_add_goto_on_match(path_state_t * state, uint8_t match, path_state_t * next_state)
{
    if (state->type == ACTION) {
        printf("Internal error - attempt to add a match to the action state\n");
        exit(4);
    }
    uint32_t i = state->match_cnt;
    ++state->match_cnt;

    state->matches = realloc(state->matches, sizeof(uint8_t) * state->match_cnt);
    state->goto_states = realloc(state->goto_states, sizeof(path_state_t*) * state->match_cnt);

    state->matches[i] = match;
    return state->goto_states[i] = next_state;
}

path_state_t * path_state_add_match(path_state_t * state, uint8_t match, uint32_t * state_no_gen)
{
    if (state->type == ACTION) {
        printf("Internal error - attempt to add a match to the action state\n");
        exit(4);
    }
    path_state_t * next_state = path_state_create(++*state_no_gen);
    return path_state_add_goto_on_match(state, match, next_state);
}
