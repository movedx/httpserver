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
#define LISTENQUEUE 256
#define THREAD_LIMIT_DEFAULT 10

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

static int thread_count = 0;
pthread_mutex_t thread_count_mutex;

int main(int argc, char *argv[])
{
	int THREADS_LIMIT = THREAD_LIMIT_DEFAULT;
	int THREADS_SPAWN = 0;
	char *PORT = (char *)DEFAULT_PORT;

	if (argc == 2)
	{
		PORT = argv[1];
	}
	if (argc == 3 || argc == 4)
	{
		if (strcmp(argv[1], "-tl") == 0)
		{
			THREADS_LIMIT = atoi(argv[2]);
		}
		if (strcmp(argv[1], "-tp") == 0)
		{
			THREADS_SPAWN = atoi(argv[2]);
		}
		if (argc == 4)
		{
			PORT = argv[3];
		}
	}

	// printf("Threads limit: %d\n", THREADS_LIMIT);
	// printf("Threads spawn: %d\n", THREADS_SPAWN);
	// printf("Port: %s\n", PORT);

	const char *iface = NULL;
	const char *port = argc == 2 ? argv[1] : PORT;
	struct addrinfo *res = NULL;

	int listenfd = startServer(iface, port, res);

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	// pthread_attr_setstacksize(&attr, 8388608);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

	pthread_t tid[THREADS_LIMIT];

	if (THREADS_SPAWN != 0)
	{
		THREADS_LIMIT = THREADS_SPAWN;
	}

	while (1)
	{
		if (thread_count < THREADS_LIMIT)
		{
			pthread_mutex_lock(&thread_count_mutex);
			if (pthread_create(&tid[thread_count], &attr, socketThread, &listenfd) != 0)
			{
				pthread_mutex_unlock(&thread_count_mutex);
				perror("Failed to create thread\n");
			}
			else
			{
				thread_count++;
				pthread_mutex_unlock(&thread_count_mutex);
			}
		}
		usleep(100);
	}

	closeServer(res);

	return 0;
}

void *socketThread(void *arg)
{
	struct sockaddr_in clientaddr;
	socklen_t addrlen = sizeof(clientaddr);
	int listenfd = *((int *)arg);
	int client = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);

	if (client < 0)
	{
		perror("accept error");
	}

	char *request = malloc(MAX_MESSAGE_SIZE);

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

	// printf("\n=========================REQUEST=========================\n");
	// printf("%s\n", request);
	// puts("\n========================================================\n");

	Request *request_struct = malloc(sizeof(Request));

	parse_request(request_struct, request);

	// puts("\nMETHOD:");
	// printf("%s\n", request_struct->method);

	// puts("\nPATH:");
	// printf("%s\n", request_struct->path->first->string);

	// puts("\nABSOLUTE PATH:");
	// printf("%s\n", absPath(request_struct->path->first->string));

	// puts("\nCONTENT-LENGTH:");
	// printf("%zu\n", request_struct->content_length);

	// puts("\nKEYS:");
	// request_print_all_keys(request_struct);

	// puts("\nVALUES:");
	// request_print_all_values(request_struct);

	Response *response = response_generate(request_struct);
	char *response_str = NULL;
	size_t resp_str_len = response_to_string(response, &response_str);

	// puts("\n========================RESPONSE========================\n");
	// fwrite(response_str, 1, resp_str_len, stdout);
	// puts("\n=======================================================\n");

	ssize_t reply = send(client, response_str, resp_str_len, 0);

	if (reply == -1)
	{
		perror("Send failed");
	}
	else
	{
		printf("Sended");
	}

	request_free(request_struct);
	response_free(response);
	free(response_str);
	free(request);

	close(client);

	pthread_mutex_lock(&thread_count_mutex);
	thread_count--;
	pthread_mutex_unlock(&thread_count_mutex);

	pthread_exit(NULL);
}
