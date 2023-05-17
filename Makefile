SRCDIR=./src
BUILDDIR=./build

CC=gcc
CFLAGS= -g -Wall
JSONCFLAGS= `pkg-config --cflags glib-2.0 json-glib-1.0`
JSONLIBS= `pkg-config --libs glib-2.0 json-glib-1.0`

build:
	$(CC) $(CFLAGS) -c -o $(BUILDDIR)/tags_json.o $(JSONCFLAGS) $(SRCDIR)/tags_json.c
	$(CC) $(CFLAGS) -c -o $(BUILDDIR)/add_tags.o $(SRCDIR)/add_tags.c
	$(CC) $(CFLAGS) -c -o $(BUILDDIR)/get_tags.o $(SRCDIR)/get_tags.c
	$(CC) $(CFLAGS) -c -o $(BUILDDIR)/find_tags.o $(SRCDIR)/find_tags.c

add_tags:
	$(CC) $(CFLAGS) -o add_tags $(BUILDDIR)/tags_json.o $(BUILDDIR)/add_tags.o $(JSONLIBS)

remove_tag:
	$(CC) $(CFLAGS) -o remove_tag $(SRCDIR)/remove_tag.c

get_tags:
	$(CC) $(CFLAGS) -o get_tags $(BUILDDIR)/tags_json.o $(BUILDDIR)/get_tags.o $(JSONLIBS)

find_tags:
	$(CC) $(CFLAGS) -o find_tags $(BUILDDIR)/tags_json.o $(BUILDDIR)/find_tags.o $(JSONLIBS)

all:
	make -B build; make -B add_tags; make -B remove_tag; make -B get_tags; make -B find_tags

clean:
	rm -f add_tags remove_tag get_tags find_tags build/*.o
