#include "server_utils.h"

bool _contains_any_headers(const char *msg);

const char *ALLOWED_METHODS = "GET"
                              "POST"; //todo: add methods

pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;

int parse_request(Request *request, char *msg)
{
    bool contains_fields = _contains_any_headers(msg);
    char *data = strstr(msg, "\r\n\r\n") + 4;

    request->method = strtok(msg, " ");

    // todo: check if method is in ALLOWED_METHODS.

    request->path = stringlist_new(strtok(NULL, " "));

    request->version = strtok(NULL, "\n");

    // todo: rfc7230 3.2.4: A server MUST reject any received request message that contains whitespace between a header field-name and colon with a response code of 400 (Bad Request).

    if (contains_fields)
    {
        request->headers_amount = 0;

        char *key = strtok(NULL, " :\r\n");
        char *value = strtok(NULL, "\r\n");

        while (key && value)
        {
            request_add_header_key_value(request, key, value);

            key = strtok(NULL, " :\r\n");
            value = strtok(NULL, "\r\n");
        }

        request->content_length = 0;

        request_add_content(request, data);
        return 0;
    }
    else
    {
        request->headers_amount = 0;
        request->content_length = 0;
        return 0;
    }
}

bool _contains_any_headers(const char *msg)
{
    for (size_t i = 0; msg[i]; i++)
    {
        if (msg[i] == '\n')
        {
            if (msg[i - 3] == '\r' && msg[i - 2] == '\n' && msg[i - 1] == '\r')
            {
                return false;
            }
        }
        if (msg[i] == ':')
        {
            return true;
        }
    }
    return false;
}

bool request_haskey(const char *key, Request *request)
{
    struct stringlistnode *current = request->headers->first;

    while (current)
    {
        if (strncasecmp(current->string, key, strlen(key)) == 0)
        {
            return true;
        }
        current = current->next;
    }

    return false;
}

void request_print_all_keys(Request *request)
{
    char *headers_str = stringlist_string(request->headers);

    char *key = strtok(headers_str, ": \r\n");

    while (key != NULL)
    {
        printf("%s\n", key);
        strtok(NULL, "\r\n");
        key = strtok(NULL, " :\r\n");
    }

    free(headers_str);
}

void request_print_all_values(Request *request)
{
    char *headers_str = stringlist_string(request->headers);

    char *value = strtok(headers_str, ": ");
    value = strtok(NULL, "\r\n");

    while (value != NULL)
    {
        printf("%s\n", trimstr(value));
        strtok(NULL, ": \r\n");
        value = strtok(NULL, "\r\n");
    }

    free(headers_str);
}

int request_validate(char *request)
{
    //printf("\n%s\n", request);
    regex_t regex;
    const char *regex_str = "^GET\\s\\/([^\\s\\/\\]+\\/?)*\\sHTTP\\/1\\.1\r\n([^:\r\n]+:[^\r\n]+\r\n)*(\r\n)$.*";
    if (regcomp(&regex, regex_str, REG_EXTENDED))
    {
        perror("Could not compile regex\n");
        exit(1);
    }
    int reg_res = regexec(&regex, request, 0, NULL, 0);
    regfree(&regex);
    if (reg_res == 0)
    {
        return 0;
    }
    else if (reg_res == REG_NOMATCH)
    {
        return -1;
    }
    else
    {
        perror("Regex error");
        return -1;
    }
}

char *request_get_value_by_key(Request *request, const char *key)
{
    if (request->headers_amount == 0)
    {
        return NULL;
    }

    struct stringlistnode *current = request->headers->first;

    while (current)
    {
        if (strncasecmp(current->string, key, strlen(key)) == 0)
        {
            char str[strlen(current->string) + 1];
            strcpy(str, current->string);
            return trimstr(strstr(str, ":") + 1);
        }
        current = current->next;
    }

    perror("Key doesn't exist.\n");
    return NULL;
}

void request_add_header_key_value(Request *request, const char *key, const char *value)
{
    char *header = malloc(strlen(key) + strlen(value) + 5);
    strcpy(header, key);
    strcat(header, ":");
    strcat(header, value);
    strcat(header, "\r\n");

    if (request->headers_amount == 0)
    {
        request->headers = stringlist_new(header);
        request->headers_amount++;
    }
    else
    {
        stringlist_append(request->headers, header);
        request->headers_amount++;
    }

    free(header);
}

void request_add_content(Request *request, const char *data)
{
    if (request->content_length == 0)
    {
        request->content = stringlist_new(data);
        request->content_length = strlen(data);
    }
    else
    {
        stringlist_append(request->content, data);
        request->content_length += strlen(data);
    }
}

void request_free(Request *request)
{
    stringlist_free(request->content);
    stringlist_free(request->headers);
    stringlist_free(request->path);
    free(request);
}

/* 
 * 
 * 
 * 
 * 
 * 
 * 
 * Response functions
 * 
 * 
 * 
 * 
 * 
 * 
 */

int response_status_code(Request *request)
{
    if (request->headers_amount == 0)
    {
        perror("No host field");
        return 400;
    }

    if (strncasecmp(request_get_value_by_key(request, "Host"), "localhost", strlen("localhost")) != 0)
    {
        /* Note: HTTP 1.0 lacks Host field, so this breaks it, but that's OK for us */
        return 400;
    }

    if (strcasecmp(request->method, "GET") != 0) // TODO: Add more methods later and use ALLOWED_METHODS
    {
        return 501;
    }

    if (strcasecmp(request->method, "GET") != 0 || request->path->first->string[0] != '/')
    {
        return 400;
    }

    if (!is_path_exists(absPath(request->path->first->string)))
    {
        return 404;
    }

    if (strstr(request->path->first->string, "/.."))
    {
        return 404;
    }

    return 200;
}

size_t response_to_string(Response *response, char **resp_str)
{
    size_t resp_size = strlen(response->status_line) + response->headers->length_concatenated + response->content_length + 3;
    char *res = malloc(resp_size);
    size_t offset = 0;

    memcpy(res, response->status_line, strlen(response->status_line));
    offset += strlen(response->status_line);
    memcpy(res + offset, stringlist_string(response->headers), response->headers->length_concatenated);
    offset += response->headers->length_concatenated;
    memcpy(res + offset, "\r\n", 2);
    offset += 2;
    memcpy(res + offset, response->content, response->content_length);
    *resp_str = res;
    return resp_size;
}

void response_set_status_line(Response *response, int statuscode)
{

    response->statuscode = statuscode;

    char *line = malloc(strlen(RESPONSESTART) + 32);

    strcpy(line, RESPONSESTART);

    if (response->statuscode == 200)
    {
        strcat(line, RESPONSE200);
    }
    else if (response->statuscode == 301)
    {
        strcat(line, RESPONSE301);
    }
    else if (response->statuscode == 304)
    {
        strcat(line, RESPONSE304);
    }
    else if (response->statuscode == 400)
    {
        strcat(line, RESPONSE400);
    }
    else if (response->statuscode == 404)
    {
        strcat(line, RESPONSE404);
    }
    else if (response->statuscode == 501)
    {
        strcat(line, RESPONSE501);
    }

    response->status_line = malloc(strlen(line) + 1);
    strcpy(response->status_line, line);

    free(line);
}

void response_add_content(Response *response, char *data, size_t size)
{
    size_t offset = response->content_length;
    response->content = realloc(response->content, offset + size);
    memcpy((response->content) + offset, data, size);
    response->content_length = offset + size;
}

void response_add_header_key_value(Response *response, const char *key, const char *value)
{
    char *header = malloc(strlen(key) + strlen(value) + 5);
    strcpy(header, key);
    strcat(header, ": ");
    strcat(header, value);
    strcat(header, "\r\n");

    if (response->headers_amount == 0)
    {
        response->headers = stringlist_new(header);
        response->headers_amount++;
    }
    else
    {
        stringlist_append(response->headers, header);
        response->headers_amount++;
    }

    free(header);
}

void response_add_header_line(Response *response, const char *header)
{
    if (response->headers_amount == 0)
    {
        response->headers = stringlist_new(header);
        response->headers_amount++;
    }
    else
    {
        stringlist_append(response->headers, header);
        response->headers_amount++;
    }
}

void response_free(Response *response)
{
    free(response->status_line);
    stringlist_free(response->headers);
    free(response->content);
    free(response);
}

Response *response_generate(Request *request)
{
    Response *response = malloc(sizeof(Response));

    response->content = malloc(0);
    response->content_length = 0;

    response->headers_amount = 0;
    response->statuscode = response_status_code(request);

    const char *abspath = absPath(request->path->first->string);
    if (response->statuscode == 200)
    {
        if (is_regular_file(abspath))
        {
            CacheEntry *entry = malloc(sizeof(CacheEntry));
            char *file_bytes = NULL;
            if (!cache_is_file_in(cache, abspath))
            {
                ssize_t file_size = readfile(abspath, &file_bytes);
                if (file_size < 0)
                {
                    exit(1);
                }
                entry->path = calloc(1, sizeof(abspath) + 1);
                strcpy(entry->path, abspath);
                entry->data = calloc(1, (size_t)file_size + 1);
                strcpy(entry->data, file_bytes);
                entry->lastacc = 10;
                pthread_mutex_lock(&cache_mutex);
                ssize_t insert = cache_insert_entry(cache, &entry);
                pthread_mutex_unlock(&cache_mutex);

                if (insert < 0)
                {
                    exit(1);
                }
            }
            pthread_mutex_lock(&cache_mutex);
            cache_get_entry(cache, &entry, abspath);
            pthread_mutex_unlock(&cache_mutex);
            if (entry == NULL)
            {
                exit(1);
            }
            response_add_content(response, entry->data, sizeof(entry->data) + 1);
            free(file_bytes);

            response_set_content_type(response, abspath);
        }
        else if (is_directory(abspath))
        {
            char *strlist = NULL;
            listdir(abspath, &strlist);
            response_add_content(response, strlist, strlen(strlist));
            free(strlist);
            response_set_content_type(response, "42.html");
        }
        else
        {
            response->statuscode = 404;
        }
    }
    response_set_status_line(response, response->statuscode);

    response_add_header_line(response, RESPONSESERVER);

    char *date_header = response_make_date_header();
    response_add_header_line(response, date_header);
    free(date_header);

    response_add_header_key_value(response, "Content-Length", size_t_to_string(response->content_length));

    response_add_header_line(response, RESPONSECLOSE);

    return response;
}

char *response_make_date_header()
{
    size_t timestring_len = 50;
    char timestring[timestring_len];
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(timestring, timestring_len, "%a, %d %b %Y %H:%M:%S %Z", &tm);

    char *date_header = malloc(strlen(RESPONSEDATE) + strlen(timestring) + 3);

    sprintf(date_header, "%s%s\r\n", RESPONSEDATE, timestring);
    return date_header;
}

void response_set_content_type(Response *response, const char *path)
{
    const char *extention = get_filename_ext(path);

    if (strcasecmp(extention, "jpg") == 0)
    {
        response_add_header_line(response, CONTENT_TYPE_IMAGE_JPEG);
        return;
    }
    if (strcasecmp(extention, "html") == 0)
    {
        response_add_header_line(response, CONTENT_TYPE_TEXT_HTML);
        return;
    }
    else
    {
        response_add_header_line(response, CONTENT_TYPE_APPLICATION_OCTETSTREAM);
        return;
    }
}
