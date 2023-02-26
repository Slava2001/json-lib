#include <stdio.h>

#include "sdml-json.h"

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

sdml_json_node *create_node(struct mem_ctrl *mem, sdml_json_node *top)
{
    sdml_json_node *n = alloc_node(mem);
    if (!n) {
        return NULL;
    }
    if (top->type != OBJECT && top->type != ARRAY) {
        return NULL;
    }
    if (top->object.childe_count == MAX_OBJECT_CHILDES) {
        return NULL;
    }
    top->object.childe[top->object.childe_count] = n;
    top->object.childe_count++;
    return n;
}

int parse_str(struct string *str, const char *json);

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

            if (top && top->type == ARRAY) {
                current = create_node(&ctx->mem, top);
                if (!current) {
                    return -1;
                }
                current->parent = top;
                current->key.len = -1;
            } else if (is_key) {
                return -1; // object can not be key
            }

            current->type = json[i] == '{'? OBJECT: ARRAY;
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
            struct string tmp = {};
            int l = parse_str(&tmp, &json[i]);
            if (l < 0) {
                return -1;
            }
            i += l - 1;

            if (is_key) {
                current = create_node(&ctx->mem, top);
                if (!current) {
                    return -1;
                }
                if (top->type == ARRAY) {
                    current->key.len = -1;
                    current->type = STRING;
                    current->string = tmp;
                    current = NULL;
                } else {
                    current->parent = top;
                    current->key = tmp;
                }
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
        case 't':
        case 'T':
        case 'f':
        case 'F':
            // TODO: bool parse
            return -1;
            break;
        case 'N':
        case 'n':
            // TODO: NULL parse
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

int sdml_print(const sdml_json *ctx, char *buff, unsigned buff_size)
{
    return 0;
}

void debug_print(const sdml_json_node *root, int lvl)
{
    for (int i = 0; i < lvl; i++) printf("  ");
    if (root->key.len >= 0) {
        printf("\"%.*s\" : ", root->key.len, root->key.ptr);
    }
    switch (root->type) {
    case OBJECT:
        printf("{\n");
        for (int i = 0; i < root->object.childe_count; i++) {
            debug_print(root->object.childe[i], lvl+1);
            if (i < root->object.childe_count - 1) {
                printf(",");
            }
            printf("\n");
        }
        for (int i = 0; i < lvl; i++) printf("  ");
        printf("}");
        break;
    case STRING:
        printf("\"%.*s\"", root->string.len, root->string.ptr);
        break;
    case ARRAY:
        printf("[\n");
        for (int i = 0; i < root->object.childe_count; i++) {
            debug_print(root->object.childe[i], lvl+1);
            if (i < root->object.childe_count - 1) {
                printf(",");
            }
            printf("\n");
        }
        for (int i = 0; i < lvl; i++) printf("  ");
        printf("]");
        break;
    case NUMBER:
        break;
    };
}
