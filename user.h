#ifndef USER_H
#define USER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "chatroom.h"
#include "param.h"

typedef struct {
    int socket_fd;
    char username[50];
    char chatroom[50];
} ClientInfo;

extern ClientInfo clients[MAX_CLIENTS];
extern int client_count;
extern pthread_mutex_t clients_mutex;

void *handle_client(void *arg);

#endif
