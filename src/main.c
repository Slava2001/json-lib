#include <stdio.h>

#include "sdml-json.h"

int main()
{
    const char *json_str =  " {  \"user\" : \"Slava\", \"my_array\" : [ \"el_1\", \"el_2\", \"el_3\", { \"key\" : \"val\"}, { \"key\" : \"val\"}, [\"el_1\", \"el_2\", \"el_3\"], [\"el_1\", \"el_2\", \"el_3\"] ], \"sub_object\" : { \"obj_par\" : \"value\", \"subsub_object\" : { \"obj_par\" : \"value\" } } }  ";

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
