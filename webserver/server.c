#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "utils.h"

#define DEFAULT_HOST "localhost"
#define DEFAULT_PORT "8080"
#define LISTENQUEUE 256 /* This server can only process one client simultaneously * \ \
                         * How many connections do we want to queue? */

int startServer(const char *iface, const char *port, struct addrinfo *res)
{
    int listenfd;

    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; /* type of socket */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE; /* socket will use bind later */

    if (iface == NULL)
    {
        iface = DEFAULT_HOST;
    }

    if (getaddrinfo(iface, port, &hints, &res) != 0)
    {
        perror("Could not get address info");
        exit(1);
    }

    listenfd = socket(res->ai_family, res->ai_socktype, 0);

    if (bind(listenfd, res->ai_addr, res->ai_addrlen) != 0)
    {
        perror("Could not bind to address");
        exit(1);
    }

    if (listen(listenfd, LISTENQUEUE) != 0)
    {
        perror("Could not listen for connections on socket");
    }

    printf("Server listening on %s:%s...\n", iface, port);

    return listenfd;
}

void closeServer(struct addrinfo *res)
{
    freeaddrinfo(res);
}

int main(int argc, char *argv[])
{
    const char *iface = NULL;
    const char *port = argc == 2 ? argv[1] : DEFAULT_PORT;
    struct addrinfo *res = NULL;

    int listenfd = startServer(iface, port, res);

    char request[MAX_MESSAGE_SIZE];

    while (1)
    {
        struct sockaddr_in clientaddr;
        socklen_t addrlen = sizeof(clientaddr);
        int client = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);

        if (client < 0)
        {
            perror("accept error");
        }

        ssize_t result = recv(client, request, MAX_MESSAGE_SIZE, 0);

        if (result > 0)
        {
            request[result] = '\0';
        }

        if (result == 0)
        {
            printf("Connection closed\n");
            break;
        }

        if (result < 0)
        {
            perror("recv failed");
            break;
        }

        // shutdown(client, SHUT_RD);

        puts("\n=========================REQUEST========================\n");
        printf("%s\n", request);
        puts("========================================================\n");

        HttpRequest request_struct;
        parse_request(&request_struct, request);

        puts("\nMETHOD:");
        printf("%s\n", request_struct.method);

        puts("\nPATH:");
        printf("%s\n", request_struct.path);

        puts("\nCONTENT-LENGTH:");
        printf("%zu\n", request_struct.content_length);

        puts("\nKEYS:");
        print_all_keys(&request_struct);

        puts("\nVALUES:");
        print_all_values(&request_struct);

        int request_res = request_result(&request_struct);

        char *response;
        size_t response_len = generate_response(&response, request_res, request_struct.path, request_struct.keys, request_struct.values, request_struct.fields_amount);

        puts("\n========================RESPONSE========================\n");
        printf("%s", response);
        puts("========================================================\n");

        if (response_len > MAX_MESSAGE_SIZE)
        {
            fprintf(stderr, "response too large to send");
        }

        ssize_t reply = send(client, response, response_len, 0);

        if (reply == -1)
        {
            perror("Send failed");
        }
        else
        {
            perror("Sended");
        }

        close(client);
    }

    closeServer(res);

    return 0;
}
