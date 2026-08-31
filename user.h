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

// Record the client socket fd, their username, and current chatroom they are in 
typedef struct {
    int socket_fd;
    char username[50];
    Chatroom *chatroom; // stores the address to the chatroom obj that contains the current client is in
} ClientInfo;

extern ClientInfo clients[MAX_CLIENTS];
extern int client_count;
extern pthread_mutex_t clients_mutex;

void *handle_client(void *arg);
ClientInfo create_client_rec(int sock, char* username, Chatroom* chatroom); // create client record as they have successfully owned a socket_fd, chosen a username, join a chatroom

#endif
