#include <stdio.h>
#include <stdbool.h>
#include <string.h>


#define MAX_OBJECT_CHILDES 100

struct string {
    const char *ptr;
    unsigned len;
};

struct sdml_json_node;
struct object {
    struct sdml_json_node *childe[MAX_OBJECT_CHILDES];
    unsigned childe_count;
};

enum sdml_node_type {
    OBJECT,
    ARRAY,
    STRING,
    NUMBER
};

typedef struct sdml_json_node {
    struct string key;
    enum sdml_node_type type;
    union {
        struct string string;
        struct object object;
    };

    struct sdml_json_node *parent;
} sdml_json_node;

/// @brief Parse JSON string into tree.
/// @param[out] root poiter to root of JSON tree.
/// @param json input JSON string (must be null-terminated).
/// @param buff buffer.
/// @param buff_size buffer size.
/// @return number of elements on success, negative code on error.
int sdml_parse(sdml_json_node **root, const char *json, sdml_json_node *buff, unsigned buff_size);

/// @brief Print JSON tree into string.
/// @param root poiter to root of JSON tree.
/// @param buff buffer to storage JSON string.
/// @param buff_size buffer size.
/// @return string len on success, negative code on error.
int sdml_print(const sdml_json_node *root, char *buff, unsigned buff_size);

struct mem_ctrl {
    sdml_json_node *buff;
    unsigned used_count;
    unsigned buff_size;
};

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
    //memset(&ctrl->buff[ctrl->used_count], 0, sizeof(sdml_json_node));
    ctrl->used_count++;
    return &ctrl->buff[ctrl->used_count];
}
int parse_str(struct string *str, const char *json);

int sdml_parse(sdml_json_node **root, const char *json, sdml_json_node *buff, unsigned buff_size)
{
    struct mem_ctrl mem;
    init_mem_ctrl(&mem, buff, buff_size);

    *root = alloc_node(&mem);
    if (!*root) {
        return -1;
    }

    sdml_json_node *top = NULL;
    sdml_json_node *current = *root;
    bool is_key = false;

    for (int i = 0; json[i]; i++) {
        switch (json[i]) {
        case ':':
            is_key = false;
            break;
        case ',':
            is_key = true;
            break;
        case '{':
            if (is_key) {
                return -1; // object can not be key
            }

            current->type = OBJECT;
            top = current;
            current = NULL;
            is_key = true;
            break;
        case '}':
            if (current) {
                return -1; // expect that all subobject closed
            }
            top = top->parent;
            break;
        case '\"': {
            string tmp = {};
            int l = parse_str(&tmp, &json[i]);
            if (l < 0) {
                return -1;
            }
            i += l - 1;

            if (is_key) {
                sdml_json_node *n = alloc_node(&mem);
                if (!n) {
                    return -1;
                }
                current = n;
                if (top->type != OBJECT) {
                    return -1;
                }
                if (top->object.childe_count == MAX_OBJECT_CHILDES) {
                    return -1;
                }
                top->object.childe[top->object.childe_count] = n;
                top->object.childe_count++;
                current->key = tmp;
                current->parent = top;
            } else {
                current->type = STRING;
                current->string = tmp;
                current = NULL;
            }
            }
            break;
        case '-':
        case '+':
        case '0'-'9':
            // TODO: number parse
            return -1;
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

int parse_str(struct string *str, const char *json)
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

int sdml_print(const sdml_json_node *root, char *buff, unsigned buff_size)
{
    return 0;
}

void debug_print(const sdml_json_node *root, int lvl) {

    for (int i = 0; i < lvl; i++) printf("     ");

    printf("\"%.*s\" : ", root->key.len, root->key.ptr);
    switch (root->type) {
    case OBJECT:
        printf("{\n");
        for (int i = 0; i < root->object.childe_count; i++) {
            debug_print(root->object.childe[i], lvl+1);
        }
        for (int i = 0; i < lvl; i++) printf("     ");
        printf("}\n");
        break;
    case STRING:
        printf("\"%.*s\"\n", root->string.len, root->string.ptr);
        break;
    case ARRAY:
        break;
    case NUMBER:
        break;
    };
}

int main()
{
    const char *json_str =  " {      \"user\" : \"Slava\", \"sub_object\" : { \"obj_par\" : \"value\", \"subsub_object\" : { \"obj_par\" : \"value\" } } }  ";

    // parse
    sdml_json_node *json_tree;
    sdml_json_node json_nodes[100];
    int real_cnt = sdml_parse(&json_tree, json_str, json_nodes, 100);
    if (real_cnt < 0) {
        printf("Failed to parse JSON\n");
        return -1;
    }

    debug_print(json_tree, 0);


    // print
    char buff_str[1024] = { };
    int str_len = sdml_print(json_tree, buff_str, 1024);
    if (str_len < 0) {
        printf("Failed to print JSON\n");
        return -1;
    }

    return 0;
}
