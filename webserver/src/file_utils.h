#pragma once

#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include "server_utils.h"
#include "stringlist.h"

stringlist *listdir(const char *path);

stringlist *readfile(const char *path);

int is_regular_file(const char *path);

int is_directory(const char *path);

void file_to_string(const char *path, char *buffer);