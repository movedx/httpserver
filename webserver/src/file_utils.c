#include "file_utils.h"

int listdir(const char *path, StringList **dirs)
{
	DIR *d;
	struct dirent *dir;
	*dirs = stringlist_new("<html><body>\n");

	d = opendir(path);
	if (d)
	{
		//puts("\nLISTING DIRECTORY:");
		while ((dir = readdir(d)) != NULL)
		{
			if (strcmp(dir->d_name, "index.html") == 0)
			{
				stringlist_free(*dirs);
				char path_to_indexhtml[strlen(path) + strlen("index.html") + 1];
				*dirs = readfile(strcat(strcpy(path_to_indexhtml, path), "index.html"));
				return 1;
			}
			char *link;
			asprintf(&link, "<a ref=localhost:8080%s%s> %s</a>\n", path, dir->d_name, dir->d_name);
			stringlist_append(*dirs, link);
		}
		stringlist_append(*dirs, "</body></html>");
		closedir(d);
	}
	return 0;
}

stringlist *readfile(const char *path)
{
	const size_t BUFF_SIZE = 1024;
	FILE *file = fopen(path, "r");
	stringlist *flist;
	size_t count = 0;
	int c;

	if (file == NULL)
	{
		return 0; //could not open file
	}
	// if (strstr(path, ".jpg") || strstr(path, ".png")) // TODO: fix later (file.jpg.html) and move this block from this function
	// {
	// 	flist = stringlist_new("<html><body>");
	// 	char *flink;
	// 	asprintf(&flink, "<img src=\"%s\" alt=\"%s\" >", path, path);
	// 	stringlist_append(flist, flink);
	// 	stringlist_append(flist, "</body></html>");
	// }
	else
	{
		flist = stringlist_new("");
		char *speicher = malloc(BUFF_SIZE);

		while ((c = fgetc(file)) != EOF)
		{
			if (count >= BUFF_SIZE)
			{
				stringlist_append(flist, speicher);
				free(speicher);
				speicher = malloc(BUFF_SIZE);
				count = 0;
			}
			speicher[count] = (char)c;
			count++;
		}

		stringlist_append(flist, speicher);
		free(speicher);
	}
	return flist;
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