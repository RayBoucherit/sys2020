#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/xattr.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#define PREFIX "user.tag."

int tag_is_valid(char const *tag) {
    if (tag == NULL) return 0;
    for (size_t i = 0; i < strlen(tag); i++) {
        if (!isalnum(tag[i])) return 0;
    }
    return 1;
}

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printf("remove_tag [PATH] [TAG]\n");
        exit(-1);
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror(strerror(errno));
        exit(-1);
    }

    if (!tag_is_valid(argv[2])) {
        printf("tag must be only alphanumerical characters\n");
        exit(-1);
    }

    char tag[256];
    strcpy(tag, PREFIX);
    strcat(tag, argv[2]);

    if (fgetxattr(fd, tag, NULL, 0) >= 0) {
        if (fremovexattr(fd, tag) < 0) {
            perror(strerror(errno));
            close(fd);
            exit(-1);
        }
        printf("tag \"%s\" successfully removed from %s\n", argv[2], argv[1]);
    }
    else if (errno == ENODATA) printf("%s does not contain tag \"%s\"\n", argv[1], argv[2]);
    else {
        perror(strerror(errno));
        close(fd);
        exit(-1);
    }

    close(fd);
    return 0;
}
