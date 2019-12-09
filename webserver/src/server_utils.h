#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <strings.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "file_utils.h"
#include "string_utils.h"
#include "stringlist.h"

#define MAX_HEADER_FIELDS 30
#define MAX_HEADER_FIELDKEY_SIZE 32
#define MAX_HEADER_FIELDVALUE_SIZE 256
#define MAX_MESSAGE_SIZE 4096
#define MAX_PATH_SIZE 2048

#define RESPONSESTART "HTTP/1.1 "
#define RESPONSE400 "400 Bad Request\r\n"
#define RESPONSE501 "501 Not Implemented\r\n"
#define RESPONSE301 "301 Moved Permanently\r\nLocation: /index.html\r\n"
#define RESPONSE200 "200 OK\r\n"
#define RESPONSE404 "404 Not Found\r\n"
#define RESPONSE304 "304 Not Modified\r\n"

#define RESPONSESERVER "Server: PK\r\n"
#define RESPONSEDATE "Date: " /* Date value needs to be added at runtime */
#define RESPONSECLOSE "Connection: close\r\n"
#define CONTENT_LENGTH "Content-Length: " /* Content-Length needs to be added at runtime */
#define CONTENT_TYPE_TEXT_HTML "Content-Type: text/html\r\n"
#define ROOTDIR "/tmp/www"

#define HELLOWORLD "Hello World!\n"

extern const char *ALLOWED_METHODS;

typedef struct Request
{
    char *method;
    char path[MAX_PATH_SIZE];
    char *version;
    StringList *headers;
    size_t headers_amount;
    StringList *content;
    size_t content_length;
} Request;

typedef struct Response
{
    int statuscode;
    char *status_line;
    StringList *headers;
    size_t content_length;
    StringList *content;
    size_t headers_amount;
} Response;

// check if message is fully recieved and we can start parse request
bool is_end_of_msg(const char *msg); // TODO: implement

/*
 * Request functions 
 */

int parse_request(Request *request, char *msg);

bool request_haskey(const char *key, Request *request);

void request_print_all_keys(Request *request);

void request_print_all_values(Request *request);

// const char *get_keys(Request *request);

// const char *get_values(Request *request);

int request_result(Request *request);

char *request_get_value_by_key(Request *request, const char *key);

bool validate_request(char *request);

void request_add_header_key_value(Request *request, const char *key, const char *value);

void request_add_content(Request *request, const char *data);

void request_free(Request *request); // TODO: implement

/*
 * Response functions 
 */

void response_free(Response *response);

// Returns the string, caller needs to free it
char *response_to_string(Response *response);

void response_add_header_key_value(Response *response, const char *key, const char *value);

void response_add_header_line(Response *response, const char *header);

char *response_get_header_value(Response *response, const char *key); // TODO: implement

void response_add_content(Response *response, const char *data);

void response_set_status_line(Response *response, int statuscode);

Response *response_generate(Request *request); // TODO: implement

// Returns the string, caller needs to free it
char *response_make_date_header(void);

size_t generate_response_deprecated(char **responsebuffer, int statuscode, char *requestpath, char **fieldkeys, char **fieldvalues, size_t fields_len);
