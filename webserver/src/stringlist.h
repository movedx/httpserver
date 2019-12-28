#ifndef STRINGLIST_H
#define STRINGLIST_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

struct stringlistnode
{
	char *string;
	size_t stringlength;
	struct stringlistnode *next;
};
typedef struct StringList
{
	struct stringlistnode *first;
	struct stringlistnode *last;
	size_t length;
	size_t length_concatenated;
} StringList;

/* Initializes a stringlist with initialstring, if no initialstring is desired
 * NULL should be passed */
StringList *stringlist_new(const char *initialstring);
/* Appends a duplicate of the string to the list. Can be NULL or empty */
StringList *stringlist_append(StringList *list, const char *string);
/* Returns the entire string, caller needs to free it */
char *stringlist_string(StringList *list);
/* Returns the length of the entire string without any NULL-bytes*/
size_t stringlist_length(StringList *list);
/* Frees all memory associated for the stringlist */
void stringlist_free(StringList *list);

struct stringlistnode *stringlistnode_new(const char *initialstring);

#endif
