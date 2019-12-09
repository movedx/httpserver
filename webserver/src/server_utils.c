#include "server_utils.h"

bool _contains_any_fields(const char *msg);

const char *ALLOWED_METHODS = "GET"
                              "POST"; //todo: add methods

int parse_request(Request *request, char *msg)
{
    bool contains_fields = _contains_any_fields(msg);
    char *data = strstr(msg, "\r\n\r\n") + 4;

    request->method = strtok(msg, " ");

    // todo: check if method is in ALLOWED_METHODS.

    // char *path = strtok(NULL, " ");
    // char new_path[MAX_PATH_SIZE];
    // new_path[0] = '\0';
    // strcpy(new_path, ROOTDIR);
    // strcat(new_path, path);
    // strcpy(request->path, new_path);

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

bool _contains_any_fields(const char *msg)
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

bool request_haskey(const char *key, Request *request)
{
    struct stringlistnode *current = request->headers->first;

    while (current != request->headers->last)
    {
        if (strncasecmp(current->string, key, strlen(key)) == 0)
        {
            return true;
        }
        current = current->next;
    }

    if (strncasecmp(current->string, key, strlen(key)) == 0)
    {
        return true;
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

// const char *get_keys(Request *request)
// {
//     char *keys = malloc(MAX_MESSAGE_SIZE);
//     size_t n = 0;
//     for (size_t i = 0; i < request->headers_amount; i++)
//     {
//         for (size_t j = 0; j < strlen(request->keys[i]); j++)
//         {
//             keys[n++] = request->keys[i][j];
//         }
//         keys[n++] = '\n';
//     }
//     keys[n] = '\0';
//     return keys;
// }

// const char *get_values(Request *request)
// {
//     char *values = malloc(MAX_MESSAGE_SIZE);
//     size_t n = 0;
//     for (size_t i = 0; i < request->headers_amount; i++)
//     {
//         for (size_t j = 0; j < strlen(request->values[i]); j++)
//         {
//             values[n++] = request->values[i][j];
//         }
//         values[n++] = '\n';
//     }
//     values[n] = '\0';
//     return values;
// }

bool validate_request(char *request)
{
    // TODO: implement validation.
    request = request;
    return 1;
}

int request_result(Request *request)
{
    if (strncasecmp(request_get_value_by_key(request, "Host"), "localhost", 9) != 0)
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

    return 200;
}

char *request_get_value_by_key(Request *request, const char *key)
{
    struct stringlistnode *current = request->headers->first;

    while (current != request->headers->last)
    {
        if (strncasecmp(current->string, key, strlen(key)) == 0)
        {
            return trimstr(strstr(current->string, ":"));
        }
        current = current->next;
    }

    if (strncasecmp(current->string, key, strlen(key)) == 0)
    {
        return trimstr(strstr(current->string, ":"));
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

// size_t generate_response_deprecated(char **responsebuffer, int statuscode, char *requestpath, char **fieldkeys, char **fieldvalues, size_t fields_len)
// {
//     /* size for response line (+'\0') except for reason phrase */
//     char response[MAX_MESSAGE_SIZE];
//     int position = sprintf(response, RESPONSESTART);
//     char responsedata[MAX_MESSAGE_SIZE] = "";
//     int dataposition = 0;
//     ssize_t content_length = 0;

//     /* From now on position will be equal to the response size ('\0' cancels) */
//     if (statuscode == 400)
//     {
//         position += sprintf(response + position, RESPONSE400);
//     }
//     else if (statuscode == 501)
//     {
//         position += sprintf(response + position, RESPONSE501);
//     }
//     else if (statuscode == 200)
//     {
//         char *data = NULL;
//         if (is_directory(requestpath))
//         {
//             listdir(requestpath);
//         }
//         if (is_regular_file(requestpath))
//         {
//             content_length = readfile(data, requestpath);
//         }
//     }
//     else
//     {
//         if (strcmp(requestpath, "/index.php") == 0)
//         {
//             position += sprintf(response + position, RESPONSE301);
//         }
//         else if ((strcmp(requestpath, "/index.html") == 0) || (strcmp(requestpath, "/") == 0))
//         {
//             position += sprintf(response + position, RESPONSE200);
//             strcpy(responsedata, HELLOWORLD);
//         }
//         else if (strcmp(requestpath, "/keys") == 0)
//         {
//             position += sprintf(response + position, RESPONSE200);
//             for (size_t i = 0; i < fields_len; i++)
//             {
//                 dataposition += sprintf(responsedata + dataposition, "%s\n", fieldkeys[i]);
//             }
//         }
//         else if (strcmp(requestpath, "/values") == 0)
//         {
//             position += sprintf(response + position, RESPONSE200);
//             for (size_t i = 0; i < fields_len; i++)
//             {
//                 dataposition += sprintf(responsedata + dataposition, "%s\n", fieldvalues[i]);
//             }
//         }
//         else if (strncmp(requestpath, "/header/", strlen("/header/")) == 0)
//         {
//             char *requestedkey = requestpath + strlen("/header/");
//             size_t i;
//             for (i = 0; i < fields_len; i++)
//             {
//                 if (strcasecmp(fieldkeys[i], requestedkey) == 0)
//                 {
//                     dataposition += sprintf(responsedata + dataposition, "%s\n", fieldvalues[i]);
//                     position += sprintf(response + position, RESPONSE200);
//                 }
//             }
//             if (i == fields_len) /* fieldkey has not been found */
//             {
//                 position += sprintf(response + position, RESPONSE404);
//             }
//         }
//         else
//         {
//             position += sprintf(response + position, RESPONSE404);
//         }
//     }

//     if (statuscode == 200)
//     {
//         position += sprintf(response + position, RESPONSE200);
//     }

//     position += sprintf(response + position, RESPONSESERVER);

//     if (content_length > 0 && content_length < MAX_MESSAGE_SIZE)
//     {
//         position += sprintf(response + position, "%s%zd\r\n", CONTENT_LENGTH, content_length);
//         position += sprintf(response + position, CONTENT_TYPE_TEXT_HTML);
//     }
//     else if (content_length > MAX_MESSAGE_SIZE)
//     {
//         puts("\nERROR: EXCEEDED MAX_MESSAGE_SIZE <<<<<<<\n");
//     }

//     /* Bonus Part */
//     char timestring[50];
//     time_t now = time(0);
//     struct tm tm = *gmtime(&now);
//     strftime(timestring, 50, "%a, %d %b %Y %H:%M:%S %Z", &tm);

//     position += sprintf(response + position, "%s%s\r\n", RESPONSEDATE, timestring);
//     position += sprintf(response + position, RESPONSECLOSE);
//     if (statuscode == 0)
//     {
//         position += sprintf(response + position, responsedata);
//     }

//     response[position] = '\0';
//     *responsebuffer = response;
//     return position > 0 ? (size_t)position : 0;
// }

char *response_to_string(Response *response)
{
    StringList *rsl = stringlist_new(response->status_line);

    stringlist_append(rsl, stringlist_string(response->headers));

    stringlist_append(rsl, "\r\n");

    stringlist_append(rsl, stringlist_string(response->content));

    char *response_str = malloc(rsl->length_concatenated + 1);

    strcpy(response_str, stringlist_string(rsl));

    stringlist_free(rsl);

    return response_str;
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
}

void response_add_content(Response *response, const char *data)
{
    if (response->content_length == 0)
    {
        response->content = stringlist_new(data);
        response->content_length = strlen(data);
    }
    else
    {
        stringlist_append(response->content, data);
        response->content_length += strlen(data);
    }
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
    stringlist_free(response->content);
    free(response);
}

Response *response_generate(Request *request)
{
    Response *response = malloc(sizeof(Response));
    response->headers_amount = 0;
    response->statuscode = request_result(request);
    response_set_status_line(response, response->statuscode);

    response_add_header_line(response, RESPONSESERVER);

    char *date_header = response_make_date_header();
    response_add_header_line(response, date_header);
    free(date_header);

    response_add_header_line(response, RESPONSECLOSE);

    const char *data = HELLOWORLD;

    response->content_length = 0;
    response_add_content(response, data);

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