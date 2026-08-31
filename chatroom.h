#ifndef CHATROOM_H
#define CHATROOM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/stat.h>  // mkdir
#include <errno.h>

// param of server: server can host 10 chatrooms at same time, max client of 50 at same time
#define MAX_CHATROOMS 10
#define MAX_CLIENTS 50

#define BUFFER_SIZE 1024

typedef struct {
    char name[50];
    int client_sockets[MAX_CLIENTS];
    int client_count;
    pthread_mutex_t lock;
	FILE *log_file; // Log file for the chatroom
} Chatroom;

extern Chatroom chatrooms[MAX_CHATROOMS];
extern pthread_mutex_t chatrooms_mutex;

/* init logs on server startup and clear the chatroom list */
void init_logs();

void init_chatroom(Chatroom *chatroom, const char *name);

Chatroom* create_chatroom(const char *name);

/* find chatroom by name */
Chatroom* find_chatroom(const char *name);

/* add client to room */
int join_chatroom(Chatroom *room, int client_socket);

void leave_chatroom(const char *name, int client_socket);

void broadcast_to_chatroom(Chatroom *room, const char *message);

void log_chat_message(Chatroom *chatroom, const char *message);

void cleanup_chatroom(Chatroom *chatroom);

void create_logs_dir_if_missing();

FILE *open_chatroom_log(const char *chatroom_name);

#endif
