#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <strings.h>

#define MAX_HEADER_FIELDS 30
#define MAX_HEADER_FIELDKEY_SIZE 32
#define MAX_HEADER_FIELDVALUE_SIZE 256
#define MAX_MESSAGE_SIZE 4096

#define RESPONSESTART "HTTP/1.1 "
#define RESPONSE400 "400 Bad Request\r\n"
#define RESPONSE501 "501 Not Implemented\r\n"
#define RESPONSE301 "301 Moved Permanently\r\nLocation: /index.html\r\n"
#define RESPONSE200 "200 OK\r\n"
#define RESPONSE404 "404 Not Found\r\n"
#define RESPONSE304 "304 Not Modified\r\n"

#define RESPONSESERVER "Server: PK\r\n"
#define RESPONSEDATE "Date: " /* Date value needs to be added at runtime */
#define RESPONSECLOSE "Connection: close\r\n\r\n"

#define HELLOWORLD "Hello World!"

extern const char *ALLOWED_METHODS;

typedef struct HttpRequest
{
    char *method;
    char *path;
    char *version;
    char *keys[MAX_HEADER_FIELDS];
    char *values[MAX_HEADER_FIELDS];
    size_t fields_amount;
    char *content;
    size_t content_length;
} HttpRequest;

bool contains_any_fields(const char *msg);

int parse_request(HttpRequest *request, char *msg);

size_t haskey(const char *key, HttpRequest *request);

void print_all_keys(HttpRequest *request);

void print_all_values(HttpRequest *request);

const char *get_keys(HttpRequest *request);

const char *get_values(HttpRequest *request);

const char *str_to_lower_case(char *str);

int request_result(HttpRequest *request);

char *get_value_by_key(HttpRequest *request, const char *key);

char *trimstr(char *str);

size_t generate_response(char **responsebuffer, int statuscode, char *requestpath, char **fieldkeys, char **fieldvalues, size_t fields_len);
