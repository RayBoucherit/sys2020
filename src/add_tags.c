#include <sys/types.h>
#include <sys/xattr.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include "tags_json.h"
#define PREFIX_LEN 9

/*prend une chaîne de charactère "name" et y ajoute la chaîne "user.tag." et terminant le charactère nul à la fin(format_tag)
*vérifie si "name" est bien composé de caractères alphanumérique
*vérifie si le tag obtenu par format_tag est absent de la liste des tags
*l'ajoute si ces conditions sont remplies
*si argc > 3 alors la liste de parent est ajoutée au fichier JSON
*/
void format_tag(char *name, size_t size, char *buffer){
    char *head = "user.tag.";
    memcpy(buffer, head, PREFIX_LEN);
    memcpy(buffer + PREFIX_LEN, name, size);
    buffer[PREFIX_LEN + size] = 0;
}

int tag_is_present(int fd, char *name, size_t size){
    char buffer[size + PREFIX_LEN + 1];
    format_tag(name, size, buffer);
    fgetxattr(fd, buffer, NULL, 0);
    if(errno == ENODATA){
        return 0;
    }
    return 1;
}

int tag_is_valid(char const *tag)
{
    if (tag == NULL) return 0;
    for (size_t i = 0; i < strlen(tag); i++)
    {
        if (!isalnum(tag[i])) return 0;
    }
    return 1;
}

void add_tag(int fd, char *name, size_t size){
    int present = tag_is_present(fd, name, size);
    int valid = tag_is_valid(name);
    if(!present && valid){
        char buffer[size + PREFIX_LEN + 1];
        format_tag(name, size, buffer);
        int rc = tags_json_add_tag(name);
        if(rc < 0){
            perror("tags_json_add_tag");
            exit(1);
        }
        rc = fsetxattr(fd, buffer, NULL, 0, 0);
        if(rc < 0){
            perror("in add_tag : fgetxattr\n");
            exit(1);
        }
        printf("new tag %s added to file\n", name);
    }
    else if(!valid){
        printf("tag may only contain alphanumerical characters\n");
    }
    else{
        printf("in add_tag : tag %s already exists in file\n", name);
    }
}

void add_tag_parents(int fd, int argc, char *argv[]){
    //cas du tag lui même
    add_tag(fd, argv[2], strlen(argv[2]));
    for(int i = 3; i < argc; i++){
        //cas des parents
        tags_json_add_tag(argv[i]);
        tags_json_add_parent(argv[2], argv[i]);
    }
}

int main(int argc, char *argv[]){
    int fd;
    if(argc < 3){
        printf("./addtag file tag (optionnel) parent1 parent2 ... parentn\n");
        exit(1);
    }
    fd = open(argv[1], O_WRONLY);
    if(fd < 0){
        perror("open");
        exit(1);
    }
    tags_json_parse();
    add_tag_parents(fd, argc, argv);
    tags_json_generate();
    close(fd);
}
