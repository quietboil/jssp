# JSON-to-Struct Streaming Parser

**jssp** is a C library for parsing JSON. It's a complementary project to [**jspp**](https://github.com/quietboil/jspp). It rides on top of _jspp_ and provides a semi-automatic way to extract required data from JSON.

## Motivation

_jspp_ provides a number of benefits for parsing JSON in a resource constrained environment. However, the need to build a state machine to be able to parse text split into multiple fragments might be a bit daunting. Hence **jssp** was created. It generates automata to recognize specified elements in the JSON being parsed and execute data extraction actions.

## Features

In the spirit of _jspp_ this library continues to target severely resource constrained environments and offers:

- **No memory allocation** - With the exception of automatic stack allocation by C for function arguments and variables **jssp** does not allocate memory at all - it does not have any static variables, it does not allocate any memory dynamically. It also does not _require_ any memory allocation - static or dynamic - to be performed by the caller.
- **Streaming parsing** - A memory constrained device may not be able to receive the entire JSON message at once. **jssp** is able to parse data fragments as they becomes available and continue from the point where it left off when the next fragment arrives.

Mainly though **jssp** is designed to offer:

- **Semi-automatic data extraction** from the JSON payloads. **jssp** compiler generates the required state machines from the specification. Library user only needs to provide data extraction handlers - functions that are called when specified JSON elements are matched.

## Installation

**jssp** depends on [**jspp**](https://github.com/quietboil/jspp) which would need to be instaleld first. At the moment, once _jspp_ has been installed, the easiest way to install **jssp** is to make it a sibling project to _jspp_. For example, to install both you might execute these commands:
```sh
$ git clone https://github.com/quietboil/jspp
$ git clone https://github.com/quietboil/jssp
```
Then change the current directory to `jssp` and execute `make` to build the library.

> **Note:** if you are crosscompiling, create `config.mk` script and define the target compiler - `CC`, `CFLAGS`, `AR` - in that file.

`make` creates 2 targets - the **jssp** run-time library `libjssp.a` and the state machines compiler `jsspc`. The former can be found in the root of the project directory and the latter in the `jsspc` subdirectory. You can either reference the files you need from this project directly by adding appropriate `-I` and `-L` options or copy/move the following files to appropriate directories:
- `jssp.h` to an `include` directory,
- `libjssp.a` to a `lib` directory, and
- `jsspc` (from the `jsspc` subdirectory) to a `bin` directory.

## Example

Let's say we need to extract data from a "Hello, World" web service responses, which might look like this:
```json
{
    "target": "World",
    "qualifications": [ "Brave", "New" ],
    "action": "Hello"
}
```

And that we are using the following HTTP API:
```h
typedef void (*http_response_cb_t)(const char * content, size_t content_length);
void http_get(const char * url, http_response_cb_t cb);
```

In addition let's also assume that for one reason or another the undelying transport cannot return the entire response to our request in a single fragment and will have to execute the response callback multiple times.

Here's what we need to do to create an application that would process data returned by that web service:
1. Write a specification that describes what data from the reponse to extract and how to report them to our application.
2. Compiler the specification.
3. Write the main body of the application that will execute HTTP request and then call **jssp** to parse the web service response.

For this example we are only interested in some of the response fields, namely `target` and `action`. With that in mind our spec might look like this:
```json
{
    "target": "set_target",
    "action": "set_action"
}
```

Let's save it into a `hello_world.json` and then compile:
```sh
$ jsspc hello_world_json
```

Compilation creates 2 new files - `hello_world.h` and `hello_world.c`. The latter needs to be included into our application:
```c
#include "hello_world.h"
#include <string.h>

// define a structure to store the results of the response processing
typedef struct {
    uint8_t action_len;
    char    action[7];
    uint8_t target_len;
    char    target[7];
} hello_world_result_t;

static inline void hello_world_result_init(hello_world_result_t * result)
{
    result->action_len = 0;
    result->target_len = 0;
}

// helper function
static inline size_t min(size_t s1, size_t s2)
{
    return s1 <= s2 ? s1 : s2;
}

// implement set_target and set_action that we used in the JSON specification
// Note: as we have defined statically allocated buffers (becuase we have a priory knowledge
// about the possible returned data) we need to ensure that:
//  - we do not overrun the available space in case the response is larger than anticipated
//  - we can coalesce data fragments in case `action` or `target` value was split in two by
//    a fragmented response
void hello_world_set_action(const char * data, uint16_t data_length, uint8_t data_type, const jssp_context_t * context)
{
    hello_world_result_t * result = context->result;
    uint16_t cpylen = min(data_length, sizeof(result->action) - result->action_len);
    strncpy(result->action + result->action_len, data, cpylen);
    result->action_len += data_length;
}

void hello_jsspc_set_target(const char * data, uint16_t data_length, uint8_t data_type, const jssp_context_t * context)
{
    hello_world_result_t * result = context->result;
    uint16_t cpylen = min(data_length, sizeof(result->target) - result->target_len);
    strncpy(result->target + result->target_len, data, cpylen);
    result->target_len += data_length;
}

// To make this example simpler let's assume that we will only ever process one request
// at a time and thus it would be acceptable to allocate required elements statically:
static jssp_t               jssp;
static hello_world_result_t result;
static uint8_t              fragment_no; // a simplistic indicator of where we are in the processing

// HTTP response callback
void process_hello_world_response(const char * content, size_t content_length)
{
    uint8_t rc;
    if (fragment_no == 0) {
        // Process headers...
        // ... 
        // Processed. At this point:
        //  - `content` pointer has been moved to the JSON payload,
        //  - `content_length` has been updated to reflect the processed headers.

        rc = jssp_start(&jssp, &result, content, content_length);
    } else {
        rc = jssp_continue(&jssp, content, content_length);
    }
    switch (rc) {
        case JSON_END: {
            // Done. Notify somehow the part of the application that expects the result
            break;
        }
        case JSON_CONTINUE: {
            // waiting for the next fragment
            ++fragment_no; 
            break;
        }
        default: {
            // One of the two possible errors has happened.
        }
    }
}

// somewhere else in the application...
{
    // Prepare jssp to handle "Hello, World" JSONs
    // Note that the state machines callback prototypes are defined in the `hello_world.h`
    jssp_init(&jssp, hello_world_path_next_state, hello_world_name_next_state);

    // reset fragment No to indicate that this is a new request
    fragment_no = 0;
    http_get("http://example.net/task?client=123", process_hello_world_response);
}
```

Also review the test cases in `tests`. They will provide concrete working examples of the **jssp** usages.

## API Reference

### Callbacks

Data handling callback:
```h
typedef void (*data_cb_t)(const char * data, uint16_t data_length, uint8_t data_type, const jssp_context_t * context);
```
These are the functions that will receive JSON data recognized by _jspp_. It is assumed that the data handling callbacks will process data in some way. For instance, store them somewhere as parts of the `result` of the parsing.

Name and path recognition automata:
```h
typedef uint32_t (*jssp_name_scanner_cb_t)(uint32_t state, char next_char);
typedef uint32_t (*jssp_path_matcher_cb_t)(uint32_t state, uint8_t next_elem, data_cb_t * action);
```
The implementations for these 2 callbacks are generated by the **jssp** compiler. Unless you are planning to roll them manually (like the `hello_world` test case does), that's all you need to know.

### Init

```h
void jssp_init(jssp_t * jssp, jssp_path_matcher_cb_t path_matcher_cb, jssp_name_scanner_cb_t name_scanner_cb)
```
This function associates **jssp** with a particular JSON specification by setting state machines for the name and path recognition.

#### Usage Example

Let's say a JSON specification - `hello_world.json` - has been compiled. This would create `hello_world.c` and `hello_world.h` as the implementation of the required automata. This generated implementation, among other things, will create 2 functions that calculate state machines states for name and path recognition:
```h
uint32_t hello_world_name_next_state(uint32_t state, char next_char);
uint32_t hello_world_path_next_state(uint32_t state, uint8_t next_elem, data_cb_t * action);
```
These 2 functions are the callbacks that `jssp_init` expects:
```c
jssp_t jssp;
jssp_init(&jssp, hello_world_path_next_state, hello_world_name_next_state);
```
> **Note:** depending on your needs **jssp** can be fit with the recognition automata only once and then this structure would be reused to parse JSON of the specified type. Or, if for example you will only ever execute a single request at a time and really only need one **jssp** instance, you could allocate it statically and re-fit it with different automata depending on what you are expecting to parse.

### Start

```h
uint8_t jssp_start(jssp_t * jssp, void * result, const char * json_text, uint16_t text_length);
```
This functions prepares **jssp** to parse a new JSON payload, invokes the parser and executes matching actions until it encounters either the end of the fragment or parses the entire JSON (if it fit entirely in the first fragment).

This function returns:
- `JSON_END` if JSON is completely processed.
- `JSON_CONTINUE` if the JSON parser (_jspp_) needs to see the rest of the payload (that would arrive later in the subsequent fragments) to finish data extraction.
- `JSON_INVALID` if _jspp_ encountered an invalid JSON and cannot continue parsing it.
- `JSON_TOO_DEEP` if the JSON being parsed has more levels of nesting than the library can handle. If such deeply nested structures are expected, then  _jspp_ needs to be altered (JSON_MAX_STACK increased) and recompiled.

### Continue

```h
uint8_t jssp_continue(jssp_t * jssp, const char * json_text, uint16_t text_length);
```
This function is called to continue data extraction from the second and sunsequent JSON fragments. It returns the same codes as `jssp_start`.

## Tests

To build **jssp** unit tests go to the `tests` directory and execute `make`:
```sh
$ cd tests
$ make
```

> **Note:** `tests` makefile assumes that _jspp_ is installed as a sibling project as outlined in the Installation.

This will build a single executable that contains all unit tests. To run them execute:
```sh
tests
```
