# sys-2020

Projet de Systèmes d'exploitation 2020

## Dépendances

**APT**
pkg-config
libjson-glib-dev
libglib2.0-dev

## Scripts

Le fichier `init.sh` doit être exécuté avant toute utilisation des commandes fournies. Il permet de créer un fichier `tags.d/tags.json` qui servira au stockage des tags et des relations de parenté.

## Compilation

`make all` Pour tout compiler.

`make build` Pour changer les fichier sources en fichier objets.

`make add_tags` Pour linker les fichier objet et créer l'executable add_tags.

`make find_tags` Pour linker les fichier objet et créer l'executable find_tags.

`make get_tags` Pour linker les fichier objet et créer l'executable get_tags.

`make remove_tag` Pour linker les fichier objet et créer l'executable remove_tag.

`make clean` Pour supprimmer tout les fichier objets et executables.

## Description des commandes

`./add_tags fichier tag parent_1 parent_2 ... parent_n`
Ajoute 'tag' au fichier passé en argument et lie 'parent_1 parent_2 ... parent_n' au 'tag' dans tags.json.
Ex: `./add_tags fichier rose couleur fleur`

`./find_tags '"chemin"' '"tag_1 tag_2 ... tag_n"'`
Cherche les fichier à partir de 'chemin' contenant les 'tag_1 tag_2 ... tag_n' ou leur tags enfants.
Ex: `./find_tags fichier '"bleu"' et 'pas("rouge")'`

`./get_tags fichier`
Liste les tags attachés au fichier pointé par 'fichier'.
Ex: `./get_tags chemin`

`./remove_tag fichier tag`
Supprime les 'tag' attachés à 'fichier'.
Ex: `./remove_tag fichier rouge`
