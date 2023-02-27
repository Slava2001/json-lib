#include <stdio.h>











#ifndef SDML_JSON_LIB
#define SDML_JSON_LIB

#define MAX_OBJECT_CHILDES 10

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














#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//#include "sdml-json.h"

int init_mem_ctrl(struct mem_ctrl *ctrl, sdml_json_node *buff, unsigned buff_size) {
    ctrl->buff = buff;
    ctrl->buff_size = buff_size;
    ctrl->used_count = 0;
    return 0;
}

sdml_json_node *alloc_node(struct mem_ctrl *ctrl) {
    if (ctrl->used_count >= ctrl->buff_size) {
        return NULL;
    }
    memset(&ctrl->buff[ctrl->used_count], 0, sizeof(sdml_json_node));
    ctrl->used_count++;
    return &ctrl->buff[ctrl->used_count - 1];
}

sdml_json_node *create_node(struct mem_ctrl *mem, sdml_json_node *top)
{
    sdml_json_node *n = alloc_node(mem);
    if (!n) {
        return NULL;
    }
    if (top->type != SDML_OBJECT && top->type != SDML_ARRAY) {
        return NULL;
    }
    if (top->object.childe_count == MAX_OBJECT_CHILDES) {
        return NULL;
    }
    top->object.childe[top->object.childe_count] = n;
    top->object.childe_count++;
    return n;
}

int parse_str(struct sdml_string *str, const char *json);
int parce_number(struct sdml_number *num, const char *json);
int parce_boolean(struct sdml_bool *val, const char *json);
int parce_null(const char *json);

int sdml_parse(sdml_json *ctx, const char *json, sdml_json_node *buff, unsigned buff_size)
{
    init_mem_ctrl(&ctx->mem, buff, buff_size);

    ctx->root = alloc_node(&ctx->mem);
    if (!ctx->root) {
        return -1;
    }

    sdml_json_node *top = NULL;
    sdml_json_node *current = ctx->root;
    bool is_key = false;
    current->key.len = -1;

    for (int i = 0; json[i]; i++) {
        switch (json[i]) {
        case ':':
            is_key = false;
            break;
        case ',':
            is_key = true;
            break;
        case '{':
        case '[':

            if (top && top->type == SDML_ARRAY) {
                current = create_node(&ctx->mem, top);
                if (!current) {
                    return -1;
                }
                current->parent = top;
                current->key.len = -1;
            } else if (is_key) {
                return -1; // object can not be key
            }

            current->type = json[i] == '{'? SDML_OBJECT: SDML_ARRAY;
            top = current;
            current = NULL;
            is_key = true;
            break;
        case '}':
        case ']':
            if (current) {
                return -1; // expect that all subobject closed
            }
            top = top->parent;
            break;
        case '\"': {
            struct sdml_string tmp = {};
            int len = parse_str(&tmp, &json[i]);
            if (len < 0) {
                return -1;
            }
            i += len - 1;

            if (is_key) {
                current = create_node(&ctx->mem, top);
                if (!current) {
                    return -1;
                }
                if (top->type == SDML_ARRAY) {
                    current->key.len = -1;
                    current->type = SDML_STRING;
                    current->string = tmp;
                    current = NULL;
                } else {
                    current->parent = top;
                    current->key = tmp;
                }
            } else {
                current->type = SDML_STRING;
                current->string = tmp;
                current = NULL;
            }
            }
            break;
        case '-':
        case '+':
        case '0'...'9': {
            if (top && top->type == SDML_ARRAY) {
                current = create_node(&ctx->mem, top);
                if (!current) {
                    return -1;
                }
                current->parent = top;
                current->key.len = -1;
            } else if (is_key) {
                return -1; // number can not be key
            }
            current->type = SDML_NUMBER;
            int len = parce_number(&current->number, &json[i]);
            if (len < 0) {
                return -1;
            }
            i += len - 1;
            current = NULL; }
            break;
        case 't':
        case 'T':
        case 'f':
        case 'F': {
            if (top && top->type == SDML_ARRAY) {
                current = create_node(&ctx->mem, top);
                if (!current) {
                    return -1;
                }
                current->parent = top;
                current->key.len = -1;
            } else if (is_key) {
                return -1; // boolean can not be key
            }
            current->type = SDML_BOOLEAN;
            int len = parce_boolean(&current->boolean, &json[i]);
            if (len < 0) {
                return -1;
            }
            i += len - 1;
            current = NULL; }
            break;
        case 'N':
        case 'n': {
            if (top && top->type == SDML_ARRAY) {
                current = create_node(&ctx->mem, top);
                if (!current) {
                    return -1;
                }
                current->parent = top;
                current->key.len = -1;
            } else if (is_key) {
                return -1; // null can not be key
            }
            current->type = SDML_NULL;
            int len = parce_null(&json[i]);
            if (len < 0) {
                return -1;
            }
            i += len - 1;
            current = NULL; }
            break;
        case ' ':
        case '\n':
        case '\t':
            break;
        default:
            printf("Unexpected char: \'%c\' \n", json[i]);
            return -1;
        }
    }

    return 0;
}

int parse_str(struct sdml_string *str, const char *json)
{
    json++;
    str->ptr = json;
    for (int i = 0; json[i]; i++) {
        if(json[i] == '\\') {
            i++;
        } else if (json[i] == '\"') {
            str->len = i;
            return i + 2;
        }
    }
    return -1;
}

int parce_number(struct sdml_number *num, const char *json)
{
    char *end_ptr = NULL;
    num->value = strtod(json, &end_ptr);
    if (end_ptr == json) {
        return -1;
    }
    return end_ptr - json;
}

int parce_boolean(struct sdml_bool *val, const char *json)
{
    const size_t tlen = sizeof("true") - 1;
    if (!strncmp("true", json, tlen) || !strncmp("True", json, tlen) ||
        !strncmp("TRUE", json, tlen)) {
        val->value = true;
        return tlen;
    }
    const size_t flen = sizeof("false") - 1;
    if (!strncmp("false", json, flen) || !strncmp("False", json, flen) ||
        !strncmp("FALSE", json, flen))  {
        val->value = false;
        return flen;
    }
    return -1;
}

int parce_null(const char *json)
{
    const size_t len = sizeof("null") - 1;
    if (!strncmp("null", json, len) || !strncmp("Null", json, len) || !strncmp("NULL", json, len)) {
            return sizeof("null");
    }
    return -1;
}

int sdml_print(const sdml_json *ctx, char *buff, unsigned buff_size)
{
    return 0;
}

#define TAB_SIZE 2
void debug_print(const sdml_json_node *root, int lvl)
{
    printf("%*s", lvl * TAB_SIZE, "");
    if (root->key.len >= 0) {
        printf("\"%.*s\" : ", root->key.len, root->key.ptr);
    }
    switch (root->type) {
    case SDML_OBJECT:
    case SDML_ARRAY:
        printf("%c\n", root->type == SDML_OBJECT? '{': '[');
        for (int i = 0; i < root->object.childe_count; i++) {
            debug_print(root->object.childe[i], lvl + 1);
            if (i < root->object.childe_count - 1) {
                printf(",");
            }
            printf("\n");
        }
        printf("%*s", lvl * TAB_SIZE, "");
        printf("%c", root->type == SDML_OBJECT? '}': ']');
        break;
    case SDML_STRING:
        printf("\"%.*s\"", root->string.len, root->string.ptr);
        break;
    case SDML_NUMBER:
        printf("%lf", root->number.value);
        break;
    case SDML_BOOLEAN:
        printf("%s", root->boolean.value? "true": "false");
        break;
    case SDML_NULL:
        printf("null", root->string.len, root->string.ptr);
        break;
    };
}






























//#include "sdml-json.h"

int main()
{
    const char *json_str =  "{    \"boolean\" : true, "
                                " \"number\" : 12345, "
                                " \"float_number\" : 12.34, "
                                " \"null\" : null"
                            "}";

    // parse
    sdml_json json = {};
    sdml_json_node json_nodes[100];
    int real_cnt = sdml_parse(&json, json_str, json_nodes, 100);
    if (real_cnt < 0) {
        printf("Failed to parse JSON\n");
        return -1;
    }

    debug_print(json.root, 0);

    // print
    char buff_str[1024] = { };
    int str_len = sdml_print(&json, buff_str, 1024);
    if (str_len < 0) {
        printf("Failed to print JSON\n");
        return -1;
    }

    return 0;
}
