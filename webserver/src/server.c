#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "server_utils.h"
#include <pthread.h>

#define DEFAULT_HOST "localhost"
#define DEFAULT_PORT "8080"
#define LISTENQUEUE 256 /* This server can only process one client simultaneously * \ \
						 * How many connections do we want to queue? */
#define MAX_THREADS 10

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

void *socketThread(void *arg);

int main(int argc, char *argv[])
{
	const char *iface = NULL;
	const char *port = argc == 2 ? argv[1] : DEFAULT_PORT;
	struct addrinfo *res = NULL;

	int listenfd = startServer(iface, port, res);

	pthread_t tid[MAX_THREADS];
	int i = 0;

	while (1)
	{
		struct sockaddr_in clientaddr;
		socklen_t addrlen = sizeof(clientaddr);
		int client = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);

		if (client < 0)
		{
			perror("accept error");
		}

		if (pthread_create(&tid[i], NULL, socketThread, &client) != 0)
		{
			printf("Failed to create thread\n");
		}

		if (i >= MAX_THREADS)
		{
			i = 0;
			while (i < MAX_THREADS)
			{
				pthread_join(tid[i++], NULL);
			}
			i = 0;
		}
	}

	closeServer(res);

	return 0;
}

void *socketThread(void *arg)
{
	char request[MAX_MESSAGE_SIZE];

	int client = *((int *)arg);
	ssize_t result = recv(client, request, MAX_MESSAGE_SIZE, 0);

	if (result > 0)
	{
		request[result] = '\0';
	}

	if (result == 0)
	{
		printf("Connection closed\n");
	}

	if (result < 0)
	{
		perror("recv failed");
	}

	// shutdown(client, SHUT_RD);

	printf("\n=========================REQUEST=========================\n");
	printf("%s\n", request);
	puts("\n========================================================\n");

	Request *request_struct = malloc(sizeof(Request));

	parse_request(request_struct, request);

	// int request_res = 0;

	// if (validate_request(request))
	// {
	//     parse_request(request_struct, request);
	//     request_res = request_result(request_struct);
	// }
	// else
	// {
	//     request_res = 400;
	// }

	puts("\nMETHOD:");
	printf("%s\n", request_struct->method);

	puts("\nPATH:");
	printf("%s\n", request_struct->path->first->string);

	puts("\nABSOLUTE PATH:");
	printf("%s\n", absPath(request_struct->path->first->string));

	puts("\nCONTENT-LENGTH:");
	printf("%zu\n", request_struct->content_length);

	puts("\nKEYS:");
	request_print_all_keys(request_struct);

	puts("\nVALUES:");
	request_print_all_values(request_struct);

	Response *response = response_generate(request_struct);
	char *response_str = NULL;
	size_t resp_str_len = response_to_string(response, &response_str);

	puts("\n========================RESPONSE========================\n");
	fwrite(response_str, 1, resp_str_len, stdout);
	puts("\n=======================================================\n");

	ssize_t reply = send(client, response_str, resp_str_len, 0);

	if (reply == -1)
	{
		perror("Send failed");
	}
	else
	{
		perror("Sended");
	}

	request_free(request_struct);
	response_free(response);
	free(response_str);

	close(client);
	pthread_exit(NULL);
}
