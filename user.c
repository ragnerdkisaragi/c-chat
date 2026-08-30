#include "user.h"

ClientInfo clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

static int is_valid_username(const char *username) {
    if (strlen(username) == 0) return 0;
    for (int i = 0; username[i]; i++) {
        if (!isalnum(username[i])) return 0;
    }
    return 1;
}

static int is_username_taken(const char *username) {
    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].username, username) == 0) {
            return 1;
        }
    }
    return 0;
}

void *handle_client(void *arg) {
    int sock = *((int *)arg);
    free(arg);

    char buffer[BUFFER_SIZE];
    char username[50] = {0};
    char chatroom_name[50] = {0};
    char msg[BUFFER_SIZE];

    // Ask for username
    pthread_mutex_lock(&clients_mutex);
    memset(msg, 0, sizeof(msg));
    strcpy(msg, "Hello, please enter your username: ");
    send(sock, msg, strlen(msg), 0);
    pthread_mutex_unlock(&clients_mutex);

    // Receive username
    memset(username, 0, sizeof(username));
    int r = recv(sock, username, sizeof(username) - 1, 0);
    if (r <= 0) {
        close(sock);
        return NULL;
    }
    username[r] = '\0';

    // Validate username
    pthread_mutex_lock(&clients_mutex);
    if (!is_valid_username(username)) {
        memset(msg, 0, sizeof(msg));
        strcpy(msg, "Invalid username. Only alphanumeric characters are allowed.\n");
        send(sock, msg, strlen(msg), 0);
        pthread_mutex_unlock(&clients_mutex);
        close(sock);
        return NULL;
    }
    if (is_username_taken(username)) {
        memset(msg, 0, sizeof(msg));
        strcpy(msg, "Username already taken. Please choose another.\n");
        send(sock, msg, strlen(msg), 0);
        pthread_mutex_unlock(&clients_mutex);
        close(sock);
        return NULL;
    }
    memset(msg, 0, sizeof(msg));
    snprintf(msg, sizeof(msg), "Welcome, %s! You can now join or create a chatroom.", username);
    send(sock, msg, strlen(msg), 0);
    pthread_mutex_unlock(&clients_mutex);

    // Ask for chatroom name
    memset(msg, 0, sizeof(msg));
    pthread_mutex_lock(&clients_mutex);
    snprintf(msg, sizeof(msg), "Please enter the chatroom name you want to join or create: ");
    send(sock, msg, strlen(msg), 0);
    pthread_mutex_unlock(&clients_mutex);

    // Receive chatroom name
    memset(chatroom_name, 0, sizeof(chatroom_name));
    r = recv(sock, chatroom_name, sizeof(chatroom_name) - 1, 0);
    if (r <= 0) {
        close(sock);
        return NULL;
    }
    chatroom_name[r] = '\0';
    
    Chatroom *room = find_chatroom(chatroom_name);
    if (!room) {
        room = create_chatroom(chatroom_name);
        if (!room) {
            send(sock, "Chatroom full\n", 14, 0);
            close(sock);
            return NULL;
        }
    }

    if (!join_chatroom(room, sock)) {
        send(sock, "Chatroom is full\n", 17, 0);
        close(sock);
        return NULL;
    }

    // Announce join
    char join_msg[BUFFER_SIZE];
    snprintf(join_msg, sizeof(join_msg), "[System]: %s has joined the chatroom.\n", username);
    broadcast_to_chatroom(room, join_msg);

    // Listen for messages
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            printf("Client %s disconnected.\n", username);
            snprintf(buffer, sizeof(buffer), "[System]: %s has left the chatroom.\n", username);
            broadcast_to_chatroom(room, buffer);
            break;
        }

        buffer[bytes_received] = '\0';

        if (strcmp(buffer, "exit") == 0) {
            printf("Client %s exited the chatroom.\n", username);
            snprintf(buffer, sizeof(buffer), "[System]: %s has left the chatroom.\n", username);
            broadcast_to_chatroom(room, buffer);
            break;
        }

        // Broadcast regular chat message
        char formatted[BUFFER_SIZE];
        // Ensure message length won't overflow formatted buffer
snprintf(formatted, sizeof(formatted), "[%s]: %.*s", username, (int)(sizeof(formatted) - strlen(username) - 5), buffer);
        broadcast_to_chatroom(room, formatted);
    }

    // Remove client from list
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket_fd == sock) {
            clients[i] = clients[client_count - 1];
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    close(sock);
    return NULL;
}
