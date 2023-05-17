#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/xattr.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include "tags_json.h"

int n_tok;
char const **tokens;

int token_is_conjunction(char const *tok) {
    if (tok == NULL) return 0;

    if (strcmp("et", tok) == 0) return 1;
    return 0;
}

int token_is_tag(char const *tok) {
    if (tok == NULL) return 0;

    int tok_size = strlen(tok);
    if (tok_size < 3) return 0;

    if (tok[0] == '\"' && tok[tok_size - 1] == '\"') {
        for (int i = 1; i < tok_size - 1; i++)
            if (!isalnum(tok[i])) return 0;
        return 1;
    }
    return 0;
}

int token_is_negation(char const *tok) {
    if (tok == NULL) return 0;

    int tok_size = strlen(tok);
    if (tok_size <= 7) return 0;

    if (strstr(tok, "pas(\"") == tok && tok[tok_size - 1] == ')') {
        if (tok[tok_size - 2] == '\"') {
            for (int i = 5; i < tok_size - 2; i++)
                if (!isalnum(tok[i])) return 0;
            return 1;
        }
    }

    return 0;
}

int valid_tokens() {
    if (n_tok % 2 == 0) return 0;
    for (int i = 0; i < n_tok; i++) {
        char const *tok = tokens[i];
        if (i % 2 == 0) {
            if (!(token_is_tag(tok) || token_is_negation(tok))) return 0;
        }
        else if (!token_is_conjunction(tok)) return 0;
    }
    return 1;
}

int check_file(char *path) {
    int show = 1;
    for (int i = 0; i < n_tok; i += 2) {
        char const *tok = tokens[i];
        if (tok[0] == '\"') {
            int tag_len = strlen(tok) - 2;
            char tag_name[tag_len + 1];
            memcpy(tag_name, &tok[1], tag_len);
            tag_name[tag_len] = '\0';
            if (!tags_json_has_tag(path, tag_name)) show = 0;
        } else {
            int tag_len = strlen(tok) - 7;
            char tag_name[tag_len + 1];
            memcpy(tag_name, &tok[5], tag_len);
            tag_name[tag_len] = '\0';
            if (tags_json_has_tag(path, tag_name)) show = 0;
        }
        if (show == 0) return 0;
    }
    return 1;
}

void find_tags(char *path) {
    if (access(path, R_OK) < 0) return;
    struct stat stat_buf;
    int rc = stat(path, &stat_buf);
    if (rc < 0) {
        perror(strerror(errno));
        exit(-1);
    }

    int show = check_file(path);
    if (show) printf("%s\n", path);

    if ((stat_buf.st_mode & S_IFMT) == S_IFDIR) {
        DIR *dirp = opendir(path);
        if (dirp == NULL) {
            perror(strerror(errno));
            exit(-1);
        }

        struct dirent *entp;

        int path_size = strlen(path);

        while ((entp = readdir(dirp)) != NULL) {
            if (strcmp("..", entp->d_name) != 0 && strcmp(".", entp->d_name) != 0) {
                int name_size = strlen(entp->d_name);
                int nextpath_size = path_size + name_size + 2;

                char nextpath[nextpath_size];
                nextpath[path_size] = '/';
                nextpath[nextpath_size - 1] = '\0';

                memcpy(nextpath, path, path_size);
                memcpy(&nextpath[path_size + 1], entp->d_name, name_size);

                if (access(nextpath, F_OK) == 0) find_tags(nextpath);
            }
        }

        closedir(dirp);
    }
}

int main(int argc, char const *argv[]) {
    if (argc < 3) {
        printf("find_tags [path] [tag ...]\n");
        exit(-1);
    }

    n_tok = argc - 2;
    tokens = malloc(sizeof(char*) * n_tok);
    if (tokens == NULL) {
        printf("Failed to allocate memory for token storage\n");
        exit(-1);
    }

    for (int i = 0; i < n_tok; i++) tokens[i] = argv[i + 2];
    if (!valid_tokens()) {
        printf("Invalid chain of tokens\nformat must be: \"token1\" et pas(\"token2\") et ... \"tokenN\"\n");
        free(tokens);
        exit(-1);
    }

    char buffer[PATH_MAX];
    if (realpath(argv[1], buffer) == NULL) {
        perror(strerror(errno));
        free(tokens);
        exit(-1);
    }

    tags_json_parse();
    find_tags(buffer);
    tags_json_generate();

    free(tokens);
    return 0;
}
