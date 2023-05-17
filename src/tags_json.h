#ifndef TAGS_JSON_H
#define TAGS_JSON_H
#define TAGS_JSON_PATH "./tags.d/tags.json"

int tags_json_generate();
int tags_json_parse();

int tags_json_add_tag(char const *tag);
int tags_json_add_parent(char const *child, char const *parent);
int tags_json_remove_tag(char const *tag_name);
int tags_json_print_tags(char const *path);
int tags_json_has_tag(char const *path, char const *tag);
#endif
