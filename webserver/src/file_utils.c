#include "file_utils.h"

stringlist *listdir(const char *path)
{
	DIR               *d;
	struct dirent     *dir;
	struct stringlist *dirs = stringlist_new("<html><body>\n");

	d = opendir(path);
	if (d)
	{
		//puts("\nLISTING DIRECTORY:");
		while ((dir = readdir(d)) != NULL)
		{
			char *link;
			asprintf(&link, "<a ref=localhost:8080%s%s> %s</a>\n", path, dir->d_name, dir->d_name);
			stringlist_append(dirs, link);
		}
		stringlist_append(dirs, "</body></html>");
		closedir(d);
	}
	return dirs;
}


stringlist *readfile(const char *path)
{
	FILE       *file = fopen(path, "r");
	stringlist *flist;
	int        count = 0;
	int        c;

	if (file == NULL)
	{
		return 0;             //could not open file
	}
	if (strstr(path, ".jpg") || strstr(path, ".png"))
	{
		flist = stringlist_new("<html><body>");
		char *flink;
		asprintf(&flink, "<img src=\"%s\" alt=\"%s\" >", path, path);
		stringlist_append(flist, flink);
		stringlist_append(flist, "</body></html>");
	}
	else
	{
		flist = stringlist_new("");
		char *speicher = malloc(MAX_MESSAGE_SIZE);

		while ((c = fgetc(file)) != EOF)
		{
			if (count >= MAX_MESSAGE_SIZE)
			{
				stringlist_append(flist, speicher);
				free(speicher);
				speicher = malloc(MAX_MESSAGE_SIZE);
				count    = 0;
			}
			speicher[count] = (char)c;
			count++;
		}
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
