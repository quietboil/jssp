#ifndef __JSSPC_H
#define __JSSPC_H

#include <stdint.h>
#include <stdbool.h>

/**
 * List of state struct variants
 */
enum _state_types {
    MATCH,
    ACTION
};

typedef struct _state state_t;

/**
 * Representation of the states of the name recognition and the path matching state machines.
 */
struct _state {
    struct {
        uint32_t  no   : 31;                ///< State number
        uint32_t  type :  1;                ///< Type of the state structure
    };
    union {
        struct { // MATCH
            state_t ** goto_states;         ///< Array of pointers to states to transtion to
            uint8_t *  matches;             ///< Array of values to match in this state
            uint32_t   match_cnt;           ///< Number of transitions
        };
        struct { // ACTION
            state_t *     parent_state;     ///< State to "return" to after the data processing action is run
            const char *  data_cb_name;     ///< Name of the data processing function
            uint16_t      data_cb_name_len; ///< Length of the function name
            bool          array_elem_cb;    ///< Flag that indicates whether action will be executed for array elements
        };
    };
};

/**
 * Allocates and initializes the new state.
 * \param  type    Type - MATCH or ACTION - of the new state.
 * \param  number  State number.
 * \return Pointer to the state struct.
 */
state_t * state_create(uint32_t type, uint32_t number);

/**
 * Adds a new match transition to the state.
 * \param          state         State to add a new match transition to.
 * \param          match         Value to match.
 * \param[in,out]  state_no_gen  Pointer to the generator of state numbers.
 * \return The go-to state of the added transition.
 */
state_t * state_add_match(state_t * state, uint8_t match, uint32_t * state_no_gen);

/**
 * Adds a new match transition to the state where transition destination is an existing state.
 * \param  state       State to add a new match transition to.
 * \param  match       Value to match.
 * \param  next_state  State to go to when matched.
 * \return next_state
 */
state_t * state_add_goto_on_match(state_t * state, uint8_t match, state_t * next_state);

/**
 * Adds states to the name recognition automaton required to recognize the new name.
 * \param  state         Initial state of the name recognition automaton.
 * \param  state_no_gen  Pointer to the state number "generator".
 * \param  name_id_gen   Pointer to the name ID "generator".
 * \param  name          Pointer to the text of the name to recognize.
 * \param  name_len      Length of the name.
 * \return Final state of the name recognition. This in this state the automaton returns the ID of the recognized name.
 */
state_t * state_build_name_matcher(state_t * state, uint32_t * state_no_gen, uint32_t * name_id_gen, const char * name, uint16_t name_len);

/**
 * Prints on stdout the representation of states of the path mathing automaton.
 * \param  state  Starting state.
 */
void dump_path_state(state_t * state);

/**
 * Prints on stdout the representation of states of the object member names mathing automaton.
 * \param  state  Starting state.
 */
void dump_name_state(state_t * state);

/**
 * Reads the file content into the heap allocated emory block.
 * \param       file_name  Name of the file to read.
 * \param[out]  size       Pointer to the variable where the size of the file will be stored.
 * \return Pointer to the allocated memory with the file content.
 */
const char * read_file(const char * file_name, uint16_t * size);

/**
 * Writes (as .h and .c files) the implementation of the name and path recognition automata.
 * \param  spec_file_name   The path name to the JSON specification file (the path name that has been provided as the argument to the program).
 * \param  name_init_state  Pointer to the initial state of the object member name recognition automaton.
 * \param  path_init_state  Pointer to the initial state of the path matching automaton.
 */
void write_automata(char * spec_file_name, state_t * name_init_state, state_t * path_init_state);

#endif