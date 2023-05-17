# Architecture

Le répertoire **src** contient le code source du projet. À la compilation, les fichiers objets sont créés dans le répertoire **build** et les exécutables sont créés à la racine du projet. La compilation se fait à l'aide du `Makefile` à la racine du projet.

Pour répondre au sujet, nous avons créé plusieurs fichiers (`add_tags.c`, `find_tags.c`, `get_tags.c`, `remove_tags.c`) contenant chacun une méthode principale afin d'avoir un exécutable pour chaque commande.

Pour garder en mémoire les tags enregistrés par l'utilisateur, les tags sont sauvegardés dans le fichier `tags.d/tags.json` à l'aide des fonctions contenues dans le fichier `src/tags_json.c`. Nous utilisons les fonctions de la librairie [json-glib](https://wiki.gnome.org/Projects/JsonGlib) qui facilite la gestion des fichiers JSON.
Le fichier `tags.d/tags.json` contient un tableau d'objets représentant les tags. Chaque objet a pour attributs une chaîne de caractères représentant le nom d'un tag qui est unique et un tableau de chaînes de caractères représentant les parents d'un tag.

Les tags sont assignés à un fichier en temps qu'attributs étendus de la forme `user.tag.NomDuTag`. Un fichier possède un tag si ee dernier est directement dans le fichier en tant qu'attribut étendu ou s'il existe une relation de parenté entre un tag du fichier et le tag spécifié, avec un nombre quelconque de relations les séparant.

Notre système de gestion de tags est compatible avec le système de gestion de fichier de Linux. En effet, si on crée un lien physique sur un fichier à l'aide de `ln`, les tags resteront les même quelque soit le lien choisi, même si on ajoute des tags après coup. Si on crée un lien symbolique sur un fichier, toutes nos commandes influenceront le fichier ciblée par le lien. Si on supprime tous les liens physiques vers un fichier avec `rm`, il n'apparaîtra plus dans notre système de tags. Si on déplace un fichier à l'aide de la fonction `mv`, les tags suivent. Si on copie un fichier à l'aide de `cp`, la copie possèdera les mêmes tags que le fichier d'origine au moment de la copie.