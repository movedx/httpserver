#include "utils.h"

const char *ALLOWED_METHODS = "GET"
                              "POST"; //todo: add methods

int parse_request(HttpRequest *request, char *msg)
{
    bool contains_fields = contains_any_fields(msg);
    char *data = strstr(msg, "\r\n\r\n") + 4;

    request->method = strtok(msg, " ");

    // todo: check if method is in ALLOWED_METHODS.

    request->path = strtok(NULL, " ");

    request->version = strtok(NULL, "\n");

    // todo: rfc7230 3.2.4: A server MUST reject any received request message that contains whitespace between a header field-name and colon with a response code of 400 (Bad Request).

    if (contains_fields)
    {
        char *key = strtok(NULL, ":");
        char *value = strtok(NULL, "\n");

        size_t keyidx = 0;
        size_t valueidx = 0;

        while (key && value)
        {
            strcpy(request->keys[keyidx++], str_to_lower_case(key));
            strcpy(request->values[valueidx++], value);

            key = strtok(NULL, ":");
            value = strtok(NULL, "\n");
        }

        request->fields_amount = keyidx;

        request->content_length = 0;
        for (size_t i = 0; i < request->fields_amount; i++)
        {
            if (!strncmp(request->keys[i], "content-length", strlen("content-length")))
                request->content_length = (size_t)atoi(request->values[i]);
        }

        if (request->content_length == 0)
        {
            return 0;
        }

        request->content = malloc(request->content_length);

        if (request->content != NULL)
        {
            strcpy(request->content, data);
            return 0;
        }
        else
        {
            perror("Memory allocation failure\n");
            return -1;
        }
    }
    else
    {
        request->fields_amount = 0;
        request->content_length = 0;
        return 0;
    }
}

bool contains_any_fields(const char *msg)
{
    for (size_t i = 0; msg[i]; i++)
    {
        if (msg[i] == ':')
        {
            return true;
        }
    }
    return false;
}

size_t haskey(const char *key, HttpRequest *request)
{
    size_t i = 0;
    while (request->keys[i++])
    {
        if (strncmp(request->keys[i], key, strlen(key)))
            return i;
    }
    return false;
}

void print_all_keys(HttpRequest *request)
{
    for (size_t i = 0; i < request->fields_amount; i++)
    {
        printf("%s\n", request->keys[i]);
    }
}

void print_all_values(HttpRequest *request)
{
    for (size_t i = 0; i < request->fields_amount; i++)
    {
        printf("%s\n", request->values[i]);
    }
}

const char *str_to_lower_case(char *str)
{
    for (size_t i = 0; str[i]; i++)
    {
        str[i] = (char)tolower(str[i]);
    }
    return str;
}

const char *get_keys(HttpRequest *request)
{
    char *keys = malloc(MAX_MESSAGE_SIZE);
    size_t n = 0;
    for (size_t i = 0; i < request->fields_amount; i++)
    {
        for (size_t j = 0; j < strlen(request->keys[i]); j++)
        {
            keys[n++] = request->keys[i][j];
        }
        keys[n++] = '\n';
    }
    return keys;
}

const char *get_values(HttpRequest *request)
{
    char *values = malloc(MAX_MESSAGE_SIZE);
    size_t n = 0;
    for (size_t i = 0; i < request->fields_amount; i++)
    {
        for (size_t j = 0; j < strlen(request->values[i]); j++)
        {
            values[n++] = request->values[i][j];
        }
        values[n++] = '\n';
    }
    return values;
}
