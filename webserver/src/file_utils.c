#include "file_utils.h"

void listdir(const char *path)
{
    DIR *d;
    struct dirent *dir;
    d = opendir(path);
    if (d)
    {
        puts("\nLISTING DIRECTORY:");
        while ((dir = readdir(d)) != NULL)
        {
            printf("%s\n", dir->d_name);
        }
        closedir(d);
    }
}

ssize_t readfile(char *data, const char *path)
{
    FILE *file = fopen(path, "r");
    ssize_t n = 0;
    int c;

    if (file == NULL)
        return 0; //could not open file

    data = malloc(MAX_MESSAGE_SIZE);

    while ((c = fgetc(file)) != EOF)
    {
        data[n++] = (char)c;
        if (n >= MAX_MESSAGE_SIZE)
            break;
    }

    // don't forget to terminate with the null character
    data[n] = '\0';
    return n;
}

int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

int is_directory(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}