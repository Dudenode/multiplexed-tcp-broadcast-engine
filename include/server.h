#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <pthread.h>
#include <netinet/in.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>

#define MAX_CLIENT 100
#define PORT 8080
#define SA struct sockaddr
#define MAXLINE 4096

int init_server(int port);

struct client_record{
    int client_fd;
    bool is_active;

};


#endif