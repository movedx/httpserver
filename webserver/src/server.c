#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "server_utils.h"
#include <pthread.h>
#include <pwd.h>
#include <tls.h>

#define DEFAULT_HOST "localhost"
#define DEFAULT_PORT "443"
#define LISTENQUEUE 256
#define THREAD_LIMIT_DEFAULT 11
#define TIMEOUT 5

#define CERTIFICATE_FILE "/etc/letsencrypt/live/9d99aca9-6769-4307-9c15-332e214ec31b.fr.bw-cloud-instance.org/fullchain.pem"
#define CERTIFICATE_KEY "/etc/letsencrypt/live/9d99aca9-6769-4307-9c15-332e214ec31b.fr.bw-cloud-instance.org/privkey.pem"

Cache *cache;

int startServer(const char *iface, const char *port, struct addrinfo *res)
{
	int listenfd;

	struct addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; /* type of socket */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE; /* socket will use bind later */

	// if (iface == NULL)
	// {
	// 	iface = DEFAULT_HOST;
	// }

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

void *socket_thread(void *arg);

static int thread_count = 0;
pthread_mutex_t thread_count_mutex;
pthread_mutex_t condition_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_cond = PTHREAD_COND_INITIALIZER;

int THREADS_LIMIT = THREAD_LIMIT_DEFAULT;

int main(int argc, char *argv[])
{
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

	struct passwd *pwd = getpwnam("www-data");
	if (pwd == NULL)
	{
		perror("getpwnam");
	}

	uid_t uid_www_data = pwd->pw_uid;
	gid_t gid_www_data = pwd->pw_gid;

	if (getuid() != uid_www_data) // Falls der Server nicht mit den Rechten von  www-data ausgeführt wird
	{
		struct stat st = {0};
		if (stat(ROOTDIR, &st) == -1)
		{
			mkdir(ROOTDIR, 0755);
		}
		if (chown(ROOTDIR, uid_www_data, gid_www_data) == -1)
		{
			perror("chown");
		}

		if (getuid() != 0)
		{
			setuid(uid_www_data);
			setgid(gid_www_data);
		}
	}

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	// pthread_attr_setstacksize(&attr, 8388608);
	// pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

	if (THREADS_SPAWN != 0)
	{
		THREADS_LIMIT = THREADS_SPAWN;
	}

	pthread_t tid[THREADS_LIMIT];

	cache = calloc(1, sizeof(Cache));

	while (1)
	{
		pthread_mutex_lock(&thread_count_mutex);
		if (thread_count < THREADS_LIMIT)
		{
			if (pthread_create(&tid[thread_count], &attr, socket_thread, &listenfd) != 0)
			{
				perror("Failed to create thread");
			}
			else
			{
				thread_count++;
			}
		}
		pthread_mutex_unlock(&thread_count_mutex);
		pthread_mutex_lock(&condition_mutex);
		while (thread_count >= THREADS_LIMIT)
		{
			pthread_cond_wait(&condition_cond, &condition_mutex);
		}
		pthread_mutex_unlock(&condition_mutex);
	}
	cache_free(cache);
	free(cache);
	closeServer(res);
	return 0;
}

void socket_thread_exit(void);

void *socket_thread(void *arg)
{
	usleep(10);
	struct sockaddr_in clientaddr;
	socklen_t addrlen = sizeof(clientaddr);
	int listenfd = *((int *)arg);

	uint8_t *mem;
	size_t mem_len;
	struct tls_config *cfg = NULL;
	struct tls *ctx = NULL;  /* Server Context */
	struct tls *cctx = NULL; /* Individual Client Socket */

	cfg = tls_config_new();
	/* Lädt das Zertifikat inklusive Chain */
	mem = tls_load_file(CERTIFICATE_FILE, &mem_len, NULL);
	tls_config_set_cert_mem(cfg, mem, mem_len);
	/* Lädt den private Key eures Zertifikats */
	mem = tls_load_file(CERTIFICATE_KEY, &mem_len, NULL);
	tls_config_set_key_mem(cfg, mem, mem_len);
	ctx = tls_server();
	tls_configure(ctx, cfg);

	int client = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);

	tls_accept_socket(ctx, &cctx, client);

	if (client < 0)
	{
		perror("accept error\n");
	}

	char *request = malloc(MAX_MESSAGE_SIZE);

	struct timeval timeout;
	timeout.tv_sec = TIMEOUT;
	timeout.tv_usec = 0;

	if (setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
	{
		perror("setsockopt failed\n");
	}

	//ssize_t result = recv(client, request, MAX_MESSAGE_SIZE, 0);
	ssize_t result = tls_read(cctx, request, sizeof(request));

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

	// if (request_validate(request) != 0)
	// {
	// 	perror(request);
	// 	perror("request validation failed");
	// 	close(client);
	// 	socket_thread_exit();
	// }

	// shutdown(client, SHUT_RD);

	// ************** DEBUG ****************
	// printf("\n=========================REQUEST=========================\n");
	// printf("%s\n", request);
	// puts("\n========================================================\n");
	// ******************************

	Request *request_struct = malloc(sizeof(Request));

	parse_request(request_struct, request);

	// ************** DEBUG ****************
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
	// ******************************

	Response *response = response_generate(request_struct);
	char *response_str = NULL;
	size_t resp_str_len = response_to_string(response, &response_str);

	// ************** DEBUG ****************
	// puts("\n========================RESPONSE========================\n");
	// fwrite(response_str, 1, resp_str_len, stdout);
	// puts("\n=======================================================\n");
	// ******************************

	// ssize_t reply = send(client, response_str, resp_str_len, 0);
	ssize_t reply = tls_write(cctx, response_str, resp_str_len);

	if (reply == -1)
	{
		perror("Send failed");
	}

	request_free(request_struct);
	response_free(response);
	free(response_str);
	free(request);

	close(client);

	tls_free(cctx);
	tls_close(ctx);
	tls_free(ctx);

	socket_thread_exit();

	return 0;
}

void socket_thread_exit()
{
	pthread_mutex_lock(&condition_mutex);
	pthread_mutex_lock(&thread_count_mutex);
	thread_count--;

	if (thread_count < THREADS_LIMIT)
	{
		pthread_cond_signal(&condition_cond);
	}
	pthread_mutex_unlock(&thread_count_mutex);
	pthread_mutex_unlock(&condition_mutex);

	pthread_exit(NULL);
}
