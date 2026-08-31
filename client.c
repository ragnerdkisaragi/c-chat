#include "client.h"

int sock;


void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (valread <= 0) {
            printf("Disconnected from server.\n");
            break;
        }
        buffer[valread] = '\0';
        //printf("[Debug] Received %d bytes\n", valread);
        printf("\033[1A");
        printf("\033[2W");
        printf("> %s\n", buffer);
    }
    return NULL;
}

int main() {
    struct sockaddr_in server_addr;
    char message[BUFFER_SIZE];
    pthread_t recv_thread;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Server address setup
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_TEST, &server_addr.sin_addr); // use AF_INET6 instead of AF_INET if want to use ipv6

    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server. \n");

/* Messaging */

    // Create thread to receive messages
    pthread_create(&recv_thread, NULL, receive_messages, NULL);

    // Main thread: send messages
    while (1) {
        memset(message, 0, BUFFER_SIZE);
        fgets(message, BUFFER_SIZE, stdin);
        message[strcspn(message, "\n")] = '\0'; // remove newline

        if (strcmp(message, "exit") == 0) {
            break;
        }

        send(sock, message, strlen(message), 0);
    }

    close(sock);
    return 0;
}
