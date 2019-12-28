#include "stringlist.h"

StringList *stringlist_new(const char *initialstring)
{
    struct stringlistnode *node = stringlistnode_new(initialstring);

    StringList *list = malloc(sizeof(StringList));
    assert(list != NULL);

    list->first = node;
    list->length = 1;
    list->length_concatenated = node->stringlength;
    list->last = node;

    return list;
}

StringList *stringlist_append(StringList *list, const char *string)
{
    struct stringlistnode *node = stringlistnode_new(string);
    list->last->next = node;
    list->last = node;
    list->length++;
    list->length_concatenated += node->stringlength;
    return list;
}

char *stringlist_string(StringList *list)
{
    if (list == NULL || list->first == NULL)
    {
        char *empty = malloc(1);
        assert(empty != NULL);
        strcpy(empty, "");
        return empty;
    }

    struct stringlistnode *current = list->first;
    char *str = malloc(list->length_concatenated + 1);
    assert(str != NULL);

    size_t n = 0;

    while (current != list->last)
    {
        strncpy(str + n, current->string, current->stringlength);
        n += current->stringlength;
        current = current->next;
    }
    strcpy(str + n, current->string);

    return str;
}

size_t stringlist_length(StringList *list)
{
    return list->length_concatenated;
}

void stringlist_free(StringList *list)
{
    struct stringlistnode *current = list->first;
    while (current != list->last)
    {
        free(current->string);
        struct stringlistnode *tmp = current;
        current = current->next;
        free(tmp);
    }
    free(current->string);
    free(current);
    free(list);
}

struct stringlistnode *stringlistnode_new(const char *initialstring)
{
    const char *str;
    if (initialstring == NULL)
    {
        str = "";
    }
    else
    {
        str = initialstring;
    }

    size_t len = strlen(str);
    struct stringlistnode *node = malloc(sizeof(struct stringlistnode));
    assert(node != NULL);

    node->string = malloc(len + 1);
    assert(node->string != NULL);
    strcpy(node->string, str);

    node->stringlength = len;

    node->next = NULL;

    return node;
}
