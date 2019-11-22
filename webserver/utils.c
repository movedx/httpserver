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
            request->keys[keyidx] = malloc(strlen(key));
            request->values[valueidx] = malloc(strlen(value));

            strcpy(request->keys[keyidx++], str_to_lower_case(key));
            strcpy(request->values[valueidx++], trimstr(value));

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
    keys[n] = '\0';
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
    values[n] = '\0';
    return values;
}

int request_result(HttpRequest *request)
{
    if (strcasecmp(get_value_by_key(request, "Host"), "localhost") != 0)
    {
        /* Note: HTTP 1.0 lacks Host field, so this breaks it, but that's OK for us */
        return 400;
    }

    if (strcasecmp(request->method, "GET ") != 0)
    {
        return 501;
    }

    if (strcasecmp(request->method, "GET") != 0 || request->path[0] != '/')
    {
        return 400;
    }

    return 200;
}

char *get_value_by_key(HttpRequest *request, const char *key)
{
    for (size_t i = 0; i < request->fields_amount; i++)
    {
        if (strcasecmp(request->keys[i], key) == 0)
        {
            return request->values[i];
        }
    }
    perror("Key doesn't exist.\n");
    return NULL;
}

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

size_t generate_response(char **responsebuffer, int statuscode, char *requestpath, char **fieldkeys, char **fieldvalues, size_t fields_len)
{
    /* size for response line (+'\0') except for reason phrase */
    char response[MAX_MESSAGE_SIZE];
    int position = sprintf(response, RESPONSESTART);
    char responsedata[MAX_MESSAGE_SIZE] = "";
    int dataposition = 0;

    /* From now on position will be equal to the response size ('\0' cancels) */
    if (statuscode == 400)
    {
        position += sprintf(response + position, RESPONSE400);
    }
    else if (statuscode == 501)
    {
        position += sprintf(response + position, RESPONSE501);
    }
    else
    {
        if (strcmp(requestpath, "/index.php") == 0)
        {
            position += sprintf(response + position, RESPONSE301);
        }
        else if ((strcmp(requestpath, "/index.html") == 0) || (strcmp(requestpath, "/") == 0))
        {
            position += sprintf(response + position, RESPONSE200);
            strcpy(responsedata, HELLOWORLD);
        }
        else if (strcmp(requestpath, "/keys") == 0)
        {
            position += sprintf(response + position, RESPONSE200);
            for (size_t i = 0; i < fields_len; i++)
            {
                dataposition += sprintf(responsedata + dataposition, "%s\n", fieldkeys[i]);
            }
        }
        else if (strcmp(requestpath, "/values") == 0)
        {
            position += sprintf(response + position, RESPONSE200);
            for (size_t i = 0; i < fields_len; i++)
            {
                dataposition += sprintf(responsedata + dataposition, "%s\n", fieldvalues[i]);
            }
        }
        else if (strncmp(requestpath, "/header/", strlen("/header/")) == 0)
        {
            char *requestedkey = requestpath + strlen("/header/");
            size_t i;
            for (i = 0; i < fields_len; i++)
            {
                if (strcasecmp(fieldkeys[i], requestedkey) == 0)
                {
                    dataposition += sprintf(responsedata + dataposition, "%s\n", fieldvalues[i]);
                    position += sprintf(response + position, RESPONSE200);
                }
            }
            if (i == fields_len) /* fieldkey has not been found */
            {
                position += sprintf(response + position, RESPONSE404);
            }
        }
        else
        {
            position += sprintf(response + position, RESPONSE404);
        }
    }
    position += sprintf(response + position, RESPONSESERVER);

    /* Bonus Part */
    char timestring[50];
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(timestring, 50, "%a, %d %b %Y %H:%M:%S %Z", &tm);

    position += sprintf(response + position, "%s%s\r\n", RESPONSEDATE, timestring);
    position += sprintf(response + position, RESPONSECLOSE);
    if (statuscode == 0)
    {
        position += sprintf(response + position, responsedata);
    }
    response[position] = '\0';
    *responsebuffer = response;
    return position > 0 ? (size_t)position : 0;
}