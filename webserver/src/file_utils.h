#pragma once

#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include "server_utils.h"

void listdir(const char *path);

ssize_t readfile(char *data, const char *path);

int is_regular_file(const char *path);

int is_directory(const char *path);