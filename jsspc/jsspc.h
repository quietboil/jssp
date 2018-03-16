#ifndef __JSSPC_H
#define __JSSPC_H

#include <stdint.h>
#include <stdbool.h>

/**
 * List of state struct variants
 */

///< State of the name recognition automaton
typedef struct _name_state name_state_t;
struct _name_state {
    uint32_t        no;              ///< State number
    uint32_t        match_cnt;       ///< Number of transitions
    char *          matches;         ///< Array of characters to match in this state
    name_state_t ** goto_states;     ///< Array of pointers to states to transtion to
};

/**
 * Allocates and initializes the new name recognition state.
 * \param  number  State number.
 * \return Pointer to the state struct.
 */
name_state_t * name_state_create(uint32_t number);

/**
 * Adds a new character match transition to the name recognition state.
 * \param          state         State to add a new match transition to.
 * \param          match         Character to match.
 * \param[in,out]  state_no_gen  Pointer to the generator of state numbers.
 * \return The go-to state of the added transition.
 */
name_state_t * name_state_add_match(name_state_t * state, char match, uint32_t * state_no_gen);

/**
 * Adds states to the name recognition automaton required to recognize the new name.
 * \param  state         Initial state of the name recognition automaton.
 * \param  state_no_gen  Pointer to the state number "generator".
 * \param  name_id_gen   Pointer to the name ID "generator".
 * \param  name          Pointer to the text of the name to recognize.
 * \param  name_len      Length of the name.
 * \return Final state of the name recognition. This in this state the automaton returns the ID of the recognized name.
 */
name_state_t * build_name_matcher(name_state_t * state, uint32_t * state_no_gen, uint32_t * name_id_gen, const char * name, uint16_t name_len);


///< State of the path matching automaton
typedef struct _path_state path_state_t;
struct _path_state {
    struct {
        uint32_t  no   : 30;                  ///< State number
        uint32_t  type :  2;                  ///< Type of the state structure
    };
    union {
        struct {
            uint32_t        match_cnt;        ///< Number of transitions
            uint8_t *       matches;          ///< Array of path elements to match in this state
            path_state_t ** goto_states;      ///< Array of pointers to states to transtion to
        };
        struct {
            uint32_t        data_cb_name_len; ///< Length of the function name
            const char *    data_cb_name;     ///< Name of the data processing function
            path_state_t *  next_state;       ///< State to go to after the data are processed
        };
    };
};

///< Types of
enum _path_state_types {
    MATCH,
    MATCH_ARRAY_ELEMENT,
    ACTION,
    ARRAY_ELEMENT_ACTION
};

/**
 * Allocates and initializes the new path matching state.
 * \param  number  State number.
 * \return Pointer to the state struct.
 */
path_state_t * path_state_create(uint32_t number);

/**
 * Adds a new path element match transition to the state.
 * \param          state         State to add a new match transition to.
 * \param          match         Path element to match.
 * \param[in,out]  state_no_gen  Pointer to the generator of state numbers.
 * \return The go-to state of the added transition.
 */
path_state_t * path_state_add_match(path_state_t * state, uint8_t match, uint32_t * state_no_gen);

/**
 * Adds a new path element match transition to the state where transition destination is an existing state.
 * \param  state       State to add a new match transition to.
 * \param  match       Value to match.
 * \param  next_state  State to go to when matched.
 * \return next_state
 */
path_state_t * path_state_add_goto_on_match(path_state_t * state, uint8_t match, path_state_t * next_state);

/**
 * Prints on stdout the representation of states of the path mathing automaton.
 * \param  state  Starting state.
 */
void path_state_dump(path_state_t * state);

/**
 * Prints on stdout the representation of states of the object member names mathing automaton.
 * \param  state  Starting state.
 */
void name_state_dump(name_state_t * state);

/**
 * Reads the file content into the heap allocated emory block.
 * \param       file_name  Name of the file to read.
 * \param[out]  size       Pointer to the variable where the size of the file will be stored.
 * \return Pointer to the allocated memory with the file content.
 */
const char * read_file(const char * file_name, uint16_t * size);

/**
 * Writes (as .h and .c files) the implementation of the name and path recognition automata.
 * \param  spec_file_name    The path name to the JSON specification file (the path name that has been provided as the argument to the program).
 * \param  name_start_state  Pointer to the starting state of the object member name recognition automaton.
 * \param  path_start_state  Pointer to the starting state of the path matching automaton.
 */
void write_automata(char * spec_file_name, name_state_t * name_start_state, path_state_t * path_start_state);

#endif