#include "file_utils.h"

int listdir(const char *path, char **buffer)
{
	DIR *d;
	struct dirent *dir;
	*buffer = malloc(strlen("<html><body>\n") + 1);
	strcpy(*buffer, "<html><body>\n");
	(*buffer)[strlen("<html><body>\n")] = '\0';

	d = opendir(path);
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (strcmp(dir->d_name, "index.html") == 0)
			{
				free(*buffer);
				char path_to_indexhtml[strlen(path) + strlen("index.html") + 1];
				readfile(strcat(strcpy(path_to_indexhtml, path), "index.html"), &*buffer);
				return 1;
			}
			char *link;
			asprintf(&link, "<a ref=localhost:8080%s%s> %s</a>\n", path, dir->d_name, dir->d_name);
			*buffer = realloc(*buffer, strlen(*buffer) + strlen(link) + 1);
			strcat(*buffer, link);
		}
		*buffer = realloc(*buffer, strlen(*buffer) + strlen("</body></html>") + 1);
		strcat(*buffer, "</body></html>");
		closedir(d);
	}
	sleep(3);
	return 0;
}

ssize_t readfile(const char *path, char **buffer)
{
	FILE *fp = fopen(path, "rb");

	if (!fp)
	{
		perror("fopen");
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	size_t flen = (size_t)ftell(fp);
	rewind(fp);

	*buffer = malloc(flen);

	if (!*buffer)
	{
		perror("malloc");
		return -1;
	}

	if (fread(*buffer, 1, flen, fp) != flen)
	{
		perror("fread");
		return -1;
	}

	fclose(fp);
	sleep(3);
	return (int)flen;
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

int is_path_exists(const char *path)
{
	struct stat path_stat;

	return stat(path, &path_stat) == 0 && (S_ISDIR(path_stat.st_mode) || S_ISREG(path_stat.st_mode));
}

const char *absPath(char *path)
{
	char *abs;
	asprintf(&abs, "%s%s", ROOTDIR, path);
	return abs;
}

const char *get_filename_ext(const char *filename)
{
	const char *dot = strrchr(filename, '.');
	if (!dot || dot == filename)
		return "";
	return dot + 1;
}