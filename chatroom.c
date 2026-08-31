#include "chatroom.h"

/*
 * Part of the server that handles chatroom created by user
*/

Chatroom chatrooms[MAX_CHATROOMS];
static int chatroom_count = 0;
pthread_mutex_t chatrooms_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_logs() {
    // TODO: Create logs directory if not exists
    mkdir("logs", 0777);

    pthread_mutex_lock(&chatrooms_mutex);
    for (int i = 0; i < MAX_CHATROOMS; i++) {
        chatrooms[i].name[0] = '\0';
        chatrooms[i].client_count = 0;
        memset(chatrooms[i].client_sockets, 0, sizeof(chatrooms[i].client_sockets));
        chatrooms[i].log_file = NULL;
    }
    pthread_mutex_unlock(&chatrooms_mutex);
}

void init_chatroom(Chatroom *chatroom, const char *name) {
    strncpy(chatroom->name, name, sizeof(chatroom->name) - 1);
    chatroom->name[sizeof(chatroom->name) - 1] = '\0';
    chatroom->client_count = 0;

    create_logs_dir_if_missing();

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "logs/%s.log", name);

    chatroom->log_file = fopen(filepath, "a");
    if (!chatroom->log_file) {
        perror("Failed to open chatroom log file");
        exit(EXIT_FAILURE);
    }
}

Chatroom *create_chatroom(const char *name) {
    if (chatroom_count >= MAX_CHATROOMS) {
        return NULL;
    }
    Chatroom *room = &chatrooms[chatroom_count++];
    strncpy(room->name, name, sizeof(room->name) - 1);
    room->name[sizeof(room->name) - 1] = '\0';

    for (int i = 0; i < MAX_CLIENTS; i++) {
        room->client_sockets[i] = -1;
    }
    pthread_mutex_init(&room->lock, NULL);

    printf("Created chatroom: %s\n", room->name);
    return room;
}

Chatroom *find_chatroom(const char *name) {
    for (int i = 0; i < chatroom_count; i++) {
        if (strcmp(chatrooms[i].name, name) == 0) {
            return &chatrooms[i];
        }
    }
    return NULL;
}

int join_chatroom(Chatroom *room, int sock) {
    pthread_mutex_lock(&room->lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (room->client_sockets[i] == -1) {
            room->client_sockets[i] = sock;
            pthread_mutex_unlock(&room->lock);
            return 1;
        }
    }
    pthread_mutex_unlock(&room->lock);
    return 0; // Room full
}

void leave_chatroom(const char *name, int client_socket) {
    for (int i = 0; i < MAX_CHATROOMS; i++) {
        if (strcmp(chatrooms[i].name, name) == 0) {
            pthread_mutex_lock(&chatrooms[i].lock);
            for (int j = 0; j < chatrooms[i].client_count; j++) {
                if (chatrooms[i].client_sockets[j] == client_socket) {
                    for (int k = j; k < chatrooms[i].client_count - 1; k++) {
                        chatrooms[i].client_sockets[k] = chatrooms[i].client_sockets[k + 1];
                    }
                    chatrooms[i].client_count--;
                    break;
                }
            }
            pthread_mutex_unlock(&chatrooms[i].lock);
            break;
        }
    }
}

void broadcast_to_chatroom(Chatroom *room, const char *message) {
    if (!room) return;
    pthread_mutex_lock(&room->lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (room->client_sockets[i] != -1) {
            send(room->client_sockets[i], message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&room->lock);
}

void cleanup_chatroom(Chatroom *chatroom) {
    if (chatroom->log_file) {
        fclose(chatroom->log_file);
        chatroom->log_file = NULL;
    }
    // Clean up other resources
}

void create_logs_dir_if_missing() {
    struct stat st = {0};
    if (stat("logs", &st) == -1) {
        if (mkdir("logs", 0755) == -1) {
            perror("Failed to create logs directory");
            exit(EXIT_FAILURE);
        }
    }
}

FILE *open_chatroom_log(const char *chatroom_name) {
    create_logs_dir_if_missing();

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "logs/%s.log", chatroom_name);

    FILE *f = fopen(filepath, "a");
    if (!f) {
        perror("Failed to open chatroom log file");
        return NULL;
    }
    return f;
}

void log_chat_message(Chatroom *chatroom, const char *message) {
    if (chatroom->log_file) {
        fprintf(chatroom->log_file, "%s\n", message);
        fflush(chatroom->log_file);
    }
}
