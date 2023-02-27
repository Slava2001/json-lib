#include <stdio.h>

#include "sdml-json.h"

int main()
{
    const char *json_str =  "["
                                "{"
                                    "\"_id\": \"5973782bdb9a930533b05cb2\","
                                    "\"isActive\": true,"
                                    "\"balance\": \"$1,446.35\","
                                    "\"age\": 32,"
                                    "\"eyeColor\": \"green\","
                                    "\"name\": \"Logan Keller\","
                                    "\"gender\": \"male\","
                                    "\"company\": \"ARTIQ\","
                                    "\"email\": \"logankeller@artiq.com\","
                                    "\"phone\": \"+1 (952) 533-2258\","
                                    "\"friends\": ["
                                    "{"
                                        "\"id\": 0,"
                                        "\"name\": \"Colon Salazar\""
                                    "},"
                                    "{"
                                        "\"id\": 1,"
                                        "\"name\": \"French Mcneil\""
                                    "},"
                                    "{"
                                        "\"id\": 2,"
                                        "\"name\": \"Carol Martin\""
                                    "}"
                                    "],"
                                    "\"favoriteFruit\": \"banana\","
                                    "\"favoriteBook\": null"
                                "}"
                            "]";

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
