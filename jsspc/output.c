#include "jsspc.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/**
 * Writes a single object member name recognition state case block.
 * \param  out     Output file handler.
 * \param  prefix  "Namespace" prefix (lowercase version of the specification name).
 * \param  state   State of the name recognition automaton.
 */
static void write_name_state(FILE * out, state_t * state)
{
    if (state->match_cnt > 0) {
        fprintf(out, "\t\tcase %u: {\n", state->no);
        if (state->match_cnt == 1) {
            const char * fmt = ' ' <= state->matches[0] && state->matches[0] < 128 
                             ? "\t\t\tif (next_char == '%c') return %u;\n"
                             : "\t\t\tif (next_char == %u) return %u;\n";
            fprintf(out, fmt, state->matches[0], state->goto_states[0]->no);
        } else {
            fprintf(out, "\t\t\tswitch (next_char) {\n");
            for (int i = 0; i < state->match_cnt; i++) {
                const char * fmt = ' ' <= state->matches[i] && state->matches[i] < 128 
                                 ? "\t\t\t\tcase '%c': return %u;\n"
                                 : "\t\t\t\tcase %u: return %u;\n";
                fprintf(out, fmt, state->matches[i], state->goto_states[i]->no);
            }
            fprintf(out, "\t\t\t}\n");
        }
        fprintf(out, "\t\t\tbreak;\n");
        fprintf(out, "\t\t}\n");

        for (int i = 0; i < state->match_cnt; i++) {
            write_name_state(out, state->goto_states[i]);
        }
    }
}

/**
 * Writes object member name recognition automaton.
 * \param  out         Output file handler.
 * \param  prefix      "Namespace" prefix (lowercase version of the specification name).
 * \param  init_state  Initial state of the name recognition automaton.
 */
static void write_name_automaton(FILE * out, const char * prefix, state_t * init_state)
{
    fprintf(out,
        "uint32_t %s_name_next_state(uint32_t state, char next_char)\n"
        "{\n"
        "\tswitch (state) {\n", prefix);
    write_name_state(out, init_state);
    fprintf(out,
        "\t}\n"
        "\treturn 0;\n"
        "}\n\n");
}

/**
 * Writes a single path matching state case block.
 * \param  out     Output file handler.
 * \param  prefix  "Namespace" prefix (lowercase version of the specification name).
 * \param  state   State of the path matching automaton.
 */
static void write_path_state(FILE * out, const char * prefix, state_t * state)
{
    fprintf(out, "\t\tcase %u: {\n", state->no);
    if (state->type == ACTION) {
        const char * fmt = state->array_elem_cb
                         ? "\t\t\treturn array_data(next_elem, %u, %u, action, %s_%.*s);\n"
                         : "\t\t\treturn object_data(next_elem, %u, %u, action, %s_%.*s);\n";
        fprintf(out, fmt, state->no, state->parent_state->no, prefix, state->data_cb_name_len, state->data_cb_name);
    } else if (state->match_cnt == 1) {
        fprintf(out, "\t\t\tif (next_elem == %u) return %u;\n", state->matches[0], state->goto_states[0]->no);
        fprintf(out, "\t\t\tbreak;\n");
    } else {
        fprintf(out, "\t\t\tswitch (next_elem) {\n");
        for (int i = 0; i < state->match_cnt; i++) {
            fprintf(out, "\t\t\t\tcase %u: return %u;\n", state->matches[i], state->goto_states[i]->no);
        }
        fprintf(out, "\t\t\t}\n\t\t\tbreak;\n");
    }
    fprintf(out, "\t\t}\n");

    if (state->type == MATCH) {
        for (int i = 0; i < state->match_cnt; i++) {
            state_t * next_state = state->goto_states[i];
            if (next_state->no > state->no) {
                write_path_state(out, prefix, next_state);
            }
        }
    }
}

/**
 * Writes path matching automaton.
 * \param  out         Output file handler.
 * \param  prefix      "Namespace" prefix (lowercase version of the specification name).
 * \param  init_state  Initial state of the path matching automaton.
 */
static void write_path_automaton(FILE * out, const char * prefix, state_t * init_state)
{
    fprintf(out,
        "static inline uint32_t object_data(uint8_t json_token, uint32_t current_state, uint32_t next_state, data_cb_t * action_ptr, data_cb_t action_cb) {\n"
        "\tif (json_token == JSON_NUMBER_PART || json_token == JSON_STRING_PART) {\n"
        "\t\t*action_ptr = action_cb;\n"
        "\t\treturn current_state;\n"
        "\t}\n"
        "\tif (JSON_NULL <= json_token && json_token <= JSON_STRING) {\n"
        "\t\t*action_ptr = action_cb;\n"
        "\t\treturn next_state;\n"
        "\t}\n"
        "\treturn 0;\n"
        "}\n\n");
    fprintf(out,
        "static inline uint32_t array_data(uint8_t json_token, uint32_t current_state, uint32_t next_state, data_cb_t * action_ptr, data_cb_t action_cb) {\n"
        "\tif (JSON_NUMBER_PART <= json_token && json_token <= JSON_STRING) {\n"
        "\t\t*action_ptr = action_cb;\n"
        "\t\treturn current_state;\n"
        "\t}\n"
        "\treturn json_token == JSON_ARRAY_END ? next_state : 0;\n"
        "}\n\n");

    fprintf(out,
        "uint32_t %s_path_next_state(uint32_t state, uint8_t next_elem, data_cb_t * action)\n"
        "{\n"
        "\tswitch (state) {\n", prefix);
    write_path_state(out, prefix, init_state);
    fprintf(out, 
        "\t}\n"
        "\treturn 0;\n"
        "}\n");
}

/**
 * Writes prototypes of the data processing actions.
 * \param  out     Output file handler.
 * \param  prefix  "Namespace" prefix (lowercase version of the specification name).
 * \param  state   Path matching state.
 */
static void write_action_declarations(FILE * out, const char * prefix, state_t * state)
{
    if (state->type == ACTION) {
        fprintf(out, "void %s_%.*s(const char * data, uint16_t data_length, uint8_t data_type, const jssp_context_t * context);\n", prefix, state->data_cb_name_len, state->data_cb_name);
    } else {
        for (int i = 0; i < state->match_cnt; i++) {
            state_t * next_state = state->goto_states[i];
            if (next_state->no > state->no) {
                write_action_declarations(out, prefix, next_state);
            }
        }
    }
}

void write_automata(char * spec_file_name, state_t * name_init_state, state_t * path_init_state)
{
    // "Remove" the extension from the file name (if any)
    char * spec_rootname_end = strrchr(spec_file_name, '.');
    if (!spec_rootname_end) {
        spec_rootname_end = strrchr(spec_file_name, '\0');
    }
    size_t spec_root_name_len = spec_rootname_end - spec_file_name;

    // Generate .h file name (we start with .h)
    char * output_file_name = alloca(spec_root_name_len + 3);
    strncpy(output_file_name, spec_file_name, spec_root_name_len);
    strcpy(output_file_name + spec_root_name_len, ".h");

    // Find the "name" of the specification in the file name.
    // It will be used as a "namespace" of sorts.
    char * spec_name = strrchr(output_file_name, '/');
    if (spec_name) {
        ++spec_name;
    } else {
        spec_name = strrchr(output_file_name, '\\');
        if (spec_name) {
            ++spec_name;
        } else {
            spec_name = output_file_name;
        }
    }
    size_t spec_name_len = spec_root_name_len - (spec_name - output_file_name);

    // Create lower and upper case versions of the specification name
    // to be used as "namespace" prefix and .h guard respectively.
    char * lowercase_prefix = alloca(spec_name_len + 1);
    char * uppercase_prefix = alloca(spec_name_len + 1);
    for (int i = 0; i < spec_name_len; i++) {
        // Mask characters that cannot be used in the generated code.
        char c = isalnum(spec_name[i]) ? spec_name[i] : '_';
        lowercase_prefix[i] = tolower(c);
        uppercase_prefix[i] = toupper(c);
    }
    lowercase_prefix[spec_name_len] = '\0';
    uppercase_prefix[spec_name_len] = '\0';

    // Generate sources, starting with .h
    FILE * out = fopen(output_file_name, "w");
    if (out) {
        fprintf(out, "#ifndef __%s_H\n", uppercase_prefix);
        fprintf(out, "#define __%s_H\n\n", uppercase_prefix);
        fprintf(out, "#include <jssp.h>\n\n");
        // Prototypes of the state machines' entries
        fprintf(out, "uint32_t %s_name_next_state(uint32_t current_state, char next_char);\n", lowercase_prefix);
        fprintf(out, "uint32_t %s_path_next_state(uint32_t current_state, uint8_t next_elem, data_cb_t * action_cb_ptr);\n\n", lowercase_prefix);
        write_action_declarations(out, lowercase_prefix, path_init_state);
        fprintf(out, "\n#endif\n");
        fclose(out);
    }

    strcpy(output_file_name + spec_root_name_len, ".c");
    out = fopen(output_file_name, "w");
    if (out) {
        // Now that .c is open we need .h back for #include
        strcpy(output_file_name + spec_root_name_len, ".h");
        fprintf(out, "#include \"%s\"\n\n", spec_name);
        write_name_automaton(out, lowercase_prefix, name_init_state);
        write_path_automaton(out, lowercase_prefix, path_init_state);
    }
}
