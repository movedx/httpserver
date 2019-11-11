#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h> 

#define DEFAULT_HOST "localhost"
#define DEFAULT_PORT "8080"
#define BUF_SIZE 8192
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

_Bool beginsWith(const char *test, const char *line)
{
	return strncmp(test, line, strlen(test)) == 0;
}
const char * replyDy(_Bool isLocalhost, int exist){
	time_t rawtime;
	struct tm *info;
	time( &rawtime);
	info = localtime( &rawtime );
	char *string = malloc (sizeof (char) * BUF_SIZE);
	if (isLocalhost == 1 && exist == 1) {
		snprintf(string, BUF_SIZE, "HTTP/1.1 200 OK\nDate: %s\nServer: myServer 1.0\nContent-Type: text/html\nContent-Length: 48\nConnection: close\n\n<html><body><h1>Hello, World!</h1></body></html>",asctime(info));
	}
	else if (isLocalhost == 1 && exist == -1) {
		snprintf(string, BUF_SIZE, "HTTP/1.1 301 Moved Permanently\nDate: %s\nServer: myServer 1.0\nContent-Type: text/html\nContent-Length: 48\nLocation: /index.html\nConnection: close\n\n",asctime(info));
	}
	else if (isLocalhost==1 && exist == 0){
		snprintf(string, BUF_SIZE, "HTTP/1.1 404 Not Found\nDate: %s\nServer: myServer 1.0\nContent-Type: text/html\nContent-Length: 48\nConnection: close\n",asctime(info));
	}
	else{
		snprintf(string, BUF_SIZE, "HTTP/1.1 403 Forbidden\nDate: %s\nServer: myServer 1.0\nContent-Type: text/html\nContent-Length: 48\nConnection: close\n",asctime(info));
	}
	return string;
}
int main(int argc, char *argv[])
{
    const char *iface = NULL;
    const char *port = argc == 2 ? argv[1] : DEFAULT_PORT;
    struct addrinfo *res = NULL;

    int listenfd = startServer(iface, port, res);
    char request[BUF_SIZE];

    while (1)
    {
        struct sockaddr_in clientaddr;
        socklen_t addrlen = sizeof(clientaddr);
        int client = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);

        if (client < 0)
        {
            perror("accept error");
        }

        ssize_t result = recv(client, request, BUF_SIZE, 0);

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

        shutdown(client, SHUT_RD);

        printf("%s", request);
	const char trenner[2] = "\n";
	char *ver = strtok((void *) request, trenner);
	ssize_t reply;
	_Bool isLocalhost = 0;
	int exist=0;
	while (ver != NULL){
		if(beginsWith((char *) "GET", ver)){
			if(strstr(ver, " / ")!=NULL || strstr(ver, "/index.html")!=NULL){
				exist=1;
			}
			else if (strstr(ver, (void *)"/index.php")!=NULL){
					exist=-1;
			}
		}
		else if (beginsWith((char *) "Host: localhost", ver)){
			isLocalhost=1;
		}
		ver = strtok(NULL, trenner);	
	}
	const char * response = replyDy(isLocalhost, exist);
	reply = send(client, response, strlen(response), 0);
	
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
