#pragma once

#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include "server_utils.h"
#include "stringlist.h"

// returns
// 1 if index.html exists
// 0 if index.html not found.
int listdir(const char *path, char **data);

// returns readed length in bytes or -1
ssize_t readfile(const char *path, char **buffer);

int is_regular_file(const char *path);

int is_directory(const char *path);

int is_path_exists(const char *path);

const char *absPath(char *path);

const char *get_filename_ext(const char *filename);