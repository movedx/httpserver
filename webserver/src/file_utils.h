#pragma once

#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include "server_utils.h"
#include "stringlist.h"

// return value:
// - 1 if index.html exists
// - 0 if not.
// parameters:
// - const char *path: path
// - StringList *dirs: pointer to StringList where the directory structure converted to html will be stored
// or content of index.html (if exists)
int listdir(const char *path, StringList **dirs);

StringList *readfile(const char *path);

int is_regular_file(const char *path);

int is_directory(const char *path);

void file_to_string(const char *path, char *buffer);

int is_path_exists(const char *path);