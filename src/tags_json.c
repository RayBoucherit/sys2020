#define _GNU_SOURCE
#include <glib-object.h>
#include <json-glib/json-glib.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <sys/stat.h>
#include <gmodule.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "tags_json.h"

JsonNode *root = NULL;

int tags_json_generate() {
    if (root == NULL) return -1;
    GError *error;
    JsonGenerator *generator;

    generator = json_generator_new();
    json_generator_set_root(generator, root);
    json_generator_set_pretty(generator, TRUE);

    error = NULL;
    json_generator_to_file(generator, TAGS_JSON_PATH, &error);
    if (error) {
        printf("Unable to generate %s: %s\n", TAGS_JSON_PATH, error->message);
        g_error_free(error);
        g_object_unref(generator);
        return -1;
    }

    json_node_free(root);
    g_object_unref(generator);
    return 0;
}

int tags_json_parse() {
    GError *error;
    JsonParser *parser;
    JsonNode *old_root;

    error = NULL;
    parser = json_parser_new();
    json_parser_load_from_file(parser, TAGS_JSON_PATH, &error);
    if (error) {
        printf("Unable to parse %s: %s\n", TAGS_JSON_PATH, error->message);
        g_error_free(error);
        g_object_unref(parser);
        return -1;
    }

    old_root = json_parser_get_root(parser);
    root = json_node_copy(old_root);

    g_object_unref(parser);
    return 0;
}

int tags_json_find(JsonArray *array, char const *tag_name) {
    if (root == NULL || array == NULL || tag_name == NULL) return -1;

    for (int i = 0; i < json_array_get_length(array); i++) {
        JsonObject *obj = json_array_get_object_element(array, i);
        char const *node_name = json_object_get_string_member(obj, "name");
        if (strcmp(node_name, tag_name) == 0) return i;
    }

    return -1;
}

int tags_json_has_parent(char const *child, char const *parent) {
    if (child == NULL || parent == NULL || root == NULL) return 0;
    if (strcmp(child, parent) == 0) return 0;

    JsonArray *array = json_node_get_array(root);
    int i = tags_json_find(array, child);
    if (i < 0) return -1;

    JsonObject *obj = json_array_get_object_element(array, i);
    char const *node_name = json_object_get_string_member(obj, "name");

    if (strcmp(node_name, child) == 0) {
        JsonArray *parents = json_object_get_array_member(obj, "parents");
        for (int j = 0; j < json_array_get_length(parents); j++) {
            char const *parent_name = json_array_get_string_element(parents, j);
            if (strcmp(parent_name, parent) == 0) return 1;
            else if (tags_json_has_parent(parent_name, parent)) return 1;
        }
        return 0;
    }
    return 0;
}

JsonNode *tags_json_create_node(char const *tag_name) {
    if (tag_name == NULL) return NULL;
    JsonNode *tag;
    JsonObject *obj;

    obj = json_object_new();
    if (obj == NULL) return NULL;
    json_object_set_string_member(obj, "name", tag_name);
    json_object_set_array_member(obj, "parents", json_array_new());
    json_object_seal(obj);

    tag = json_node_alloc();
    tag = json_node_init_object(tag, obj);
    if (tag == NULL) return NULL;

    return tag;
}

int tags_json_add_tag(char const *tag_name) {
    if (root == NULL) return -1;

    JsonArray *array = json_node_get_array(root);
    if (tags_json_find(array, tag_name) >= 0) return -1;

    JsonNode *node = tags_json_create_node(tag_name);
    if (node == NULL) {
        printf("Unable to create JsonNode for tag \"%s\"\n", tag_name);
        return -1;
    }
    json_array_add_element(array, node);

    return 0;
}

int tags_json_add_parent(char const *child, char const *parent) {
    if (child == NULL || parent == NULL || root == NULL) return -1;
    if (strcmp(child, parent) == 0) return -1;

    JsonArray *array = json_node_get_array(root);
    int i = tags_json_find(array, child);
    int j = tags_json_find(array, parent);

    if (i < 0 || j < 0) {
        printf("Tags \"%s\" and \"%s\" must exist before creating a child-parent relationship\n", child, parent);
        return -1;
    }

    JsonObject *child_obj = json_array_get_object_element(array, i);
    JsonObject *parent_obj = json_array_get_object_element(array, j);
    JsonArray *child_parents = json_object_get_array_member(child_obj, "parents");
    JsonArray *parent_parents = json_object_get_array_member(parent_obj, "parents");

    for (int k = 0; k < json_array_get_length(child_parents); k++) {
        char const *parent_name = json_array_get_string_element(child_parents, k);
        if (strcmp(parent_name, parent) == 0) {
            printf("Tag \"%s\" already has parent \"%s\"\n", child, parent);
            return -1;
        }
    }

    for (int k = 0; k < json_array_get_length(parent_parents); k++) {
        char const *parent_name = json_array_get_string_element(parent_parents, k);
        if (strcmp(parent_name, child) == 0) {
            printf("Trying to create a circular relationship between tags \"%s\" and \"%s\", operation aborted\n", parent, child);
            return -1;
        }
    }

    json_array_add_string_element(child_parents, parent);
    return 0;
}

int tags_json_remove_tag(char const *tag_name) {
    if (root == NULL) return -1;

    JsonArray *nodes = json_node_get_array(root);
    int i = tags_json_find(nodes, tag_name);
    if (i < 0) return -1;
    json_array_remove_element(nodes, i);

    for (int i = 0; i < json_array_get_length(nodes); i++) {
        JsonObject *obj = json_array_get_object_element(nodes, i);
        JsonArray *parents = json_object_get_array_member(obj, "parents");
        for (int j = 0; j < json_array_get_length(parents); j++) {
            char const *parent_name = json_array_get_string_element(parents, j);
            if (strcmp(parent_name, tag_name) == 0) {
                json_array_remove_element(parents, j);
                break;
            }
        }
    }

    return 0;
}

int tags_json_print_tag_full(int explored[], char const *tag_name) {
    if (root == NULL || tag_name == NULL) return -1;

    JsonArray *nodes = json_node_get_array(root);
    for (int i = 0; i < json_array_get_length(nodes); i++) {
        JsonObject *node = json_array_get_object_element(nodes, i);
        char const *node_name = json_object_get_string_member(node, "name");
        if (strcmp(node_name, tag_name) == 0) {
            if (!explored[i]) {
                JsonArray *parents = json_object_get_array_member(node, "parents");
                for (int j = 0; j < json_array_get_length(parents); j++) {
                    char const *parent_name = json_array_get_string_element(parents, j);
                    tags_json_print_tag_full(explored, parent_name);
                }

                printf("%s\n", tag_name);
                explored[i] = 1;
            }
            return 0;
        }
    }

    printf("Tag \"%s\" not found in %s\n", tag_name, TAGS_JSON_PATH);
    return -1;
}

int tags_json_print_tags(char const *path) {
    if (path == NULL || root == NULL) return -1;

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        printf("Unable to open %s: %s\n", path, strerror(errno));
        return -1;
    }
    int len = json_array_get_length(json_node_get_array(root));
    int explored[len];
    for (int i = 0; i < len; i++) explored[i] = 0;

    int buf_size = flistxattr(fd, NULL, 0);
    if (buf_size < 0) {
        printf("Unable to retrieve size of list of tags from %s: %s\n", path, strerror(errno));
        return -1;
    }

    char buf[buf_size];
    if (flistxattr(fd, buf, buf_size) < 0) {
        printf("Unable to retrieve list of tags from %s: %s\n", path, strerror(errno));
        return -1;
    }
    for (int i = 0; i < buf_size - 1; i++) if (buf[i] == '\0') buf[i] = ' ';

    char *tok = strtok(buf, " ");
    while (tok != NULL) {
        if (strstr(tok, "user.tag.") == tok) tags_json_print_tag_full(explored, &tok[strlen("user.tag.")]);
        tok = strtok(NULL, " ");
    }

    return 0;
}

int tags_json_has_tag(char const *path, char const *tag_name) {
    if (path == NULL || tag_name == NULL || root == NULL) return 0;

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        printf("Unable to open %s: %s\n", path, strerror(errno));
        return 0;
    }

    int buf_size = flistxattr(fd, NULL, 0);
    if (buf_size < 0) {
        printf("Unable to retrieve list of tags from %s: %s\n", path, strerror(errno));
        return 0;
    }

    char buf[buf_size];
    if (flistxattr(fd, buf, buf_size) < 0) {
        printf("Unable to retrieve list of tags from %s: %s\n", path, strerror(errno));
        return 0;
    }
    for (int i = 0; i < buf_size - 1; i++) if (buf[i] == '\0') buf[i] = ' ';

    JsonArray *array = json_node_get_array(root);
    char *tok = strtok(buf, " ");
    while (tok != NULL) {
        if (strstr(tok, "user.tag.") != tok) return 0;
        int i = tags_json_find(array, tag_name);
        if (i < 0) return 0;
        if (strcmp(&tok[strlen("user.tag.")], tag_name) == 0) return 1;
        if (tags_json_has_parent(&tok[strlen("user.tag.")], tag_name)) return 1;
        tok = strtok(NULL, " ");
    }

    return 0;
}
