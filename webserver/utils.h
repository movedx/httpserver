#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>

#define MAX_HEADER_FIELDS 30
#define MAX_HEADER_FIELDKEY_SIZE 31
#define MAX_HEADER_FIELDVALUE_SIZE 255
#define MAX_MESSAGE_SIZE 8192

extern const char *ALLOWED_METHODS;

typedef struct HttpRequest
{
    char *method;
    char *path;
    char *version;
    char keys[MAX_HEADER_FIELDS][MAX_HEADER_FIELDKEY_SIZE + 1];
    char values[MAX_HEADER_FIELDS][MAX_HEADER_FIELDVALUE_SIZE + 1];
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
