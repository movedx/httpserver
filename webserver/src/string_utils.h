#pragma once

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>

char *trimstr(char *str);

const char *str_to_lower_case(char *str);

char *absPath(char *path);

char *size_t_to_string(size_t num);
