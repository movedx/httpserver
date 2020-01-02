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

//caller must free buffer
void file_to_string(const char *path, char *buffer)
{
	FILE *fp;
	size_t lSize;

	fp = fopen(path, "rb");
	if (!fp)
		perror(path), exit(1);

	fseek(fp, 0L, SEEK_END);
	lSize = (size_t)ftell(fp);
	rewind(fp);

	/* allocate memory for entire content */
	buffer = calloc(1, lSize + 1);
	if (!buffer)
		fclose(fp), fputs("memory alloc fails", stderr), exit(1);

	/* copy the file into the buffer */
	if (1 != fread(buffer, lSize, 1, fp))
		fclose(fp), free(buffer), fputs("entire read fails", stderr), exit(1);

	fclose(fp);
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