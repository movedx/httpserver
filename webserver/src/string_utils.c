#include "string_utils.h"
#include "server_utils.h"

// Note: This function returns a pointer to a substring of the original string.
// If the given string was allocated dynamically, the caller must not overwrite
// that pointer with the returned value, since the original pointer must be
// deallocated using the same allocator with which it was allocated.  The return
// value must NOT be deallocated using free() etc.
char *trimstr(char *str)
{
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str))
        str++;

    if (*str == 0) // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

const char *str_to_lower_case(char *str)
{
    for (size_t i = 0; str[i]; i++)
    {
        str[i] = (char)tolower(str[i]);
    }
    return str;
}

char *size_t_to_string(size_t num)
{
    char *str_num;
    if (num == 0)
    {
        str_num = malloc(2);
        str_num[0] = '0';
        str_num[1] = '\0';
        return str_num;
    }
    str_num = malloc((size_t)((ceil(log10((double)num)) + 1) * sizeof(char)));
    sprintf(str_num, "%zu", num);
    return str_num;
}
