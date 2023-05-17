#include <stdlib.h>
#include <stdio.h>
#include "tags_json.h"

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("get_tags [path]\n");
        exit(-1);
    }
    char const *path = argv[1];

    tags_json_parse();
    tags_json_print_tags(path);
    tags_json_generate();

    return 0;
}
