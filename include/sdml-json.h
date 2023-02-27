#ifndef SDML_JSON_LIB
#define SDML_JSON_LIB

#include <stdbool.h>

#define MAX_OBJECT_CHILDES 100

struct sdml_string {
    const char *ptr;
    int len;
};

struct sdml_json_node;
struct sdml_object {
    struct sdml_json_node *childe[MAX_OBJECT_CHILDES];
    unsigned childe_count;
};

enum sdml_node_type {
    SDML_UNDEFINE,
    SDML_OBJECT,
    SDML_ARRAY,
    SDML_STRING,
    SDML_NUMBER,
    SDML_BOOLEAN,
    SDML_NULL
};

struct sdml_array {
    struct sdml_json_node *elements[MAX_OBJECT_CHILDES];
    unsigned element_count;
};

struct sdml_number {
    double value;
};

struct sdml_bool {
    bool value;
};

typedef struct sdml_json_node {
    struct sdml_string key;
    enum sdml_node_type type;
    union {
        struct sdml_string string;
        struct sdml_object object;
        struct sdml_array array;
        struct sdml_number number;
        struct sdml_bool boolean;
    };

    struct sdml_json_node *parent;
} sdml_json_node;

struct mem_ctrl {
    sdml_json_node *buff;
    unsigned used_count;
    unsigned buff_size;
};

typedef struct sdml_json {
    struct mem_ctrl mem;
    sdml_json_node *root;
} sdml_json;

/// @brief Parse JSON string into tree.
/// @param[inout] ctx poiter to parser context.
/// @param json input JSON string (must be null-terminated).
/// @param buff buffer.
/// @param buff_size buffer size.
/// @return number of elements on success, negative code on error.
int sdml_parse(sdml_json *ctx, const char *json, sdml_json_node *buff, unsigned buff_size);

/// @brief Print JSON tree into string.
/// @param[in] ctx poiter to parser context.
/// @param buff buffer to storage JSON string.
/// @param buff_size buffer size.
/// @return string len on success, negative code on error.
int sdml_print(const sdml_json *ctx, char *buff, unsigned buff_size);

void debug_print(const sdml_json_node *root, int lvl);

#endif // SDML_JSON_LIB
