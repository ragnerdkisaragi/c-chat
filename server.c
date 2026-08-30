#include "server.h"

int main() {
    init_logs();
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    pthread_t thread_id;

    // record server localtime
    struct tm* local;
    time_t t = time(NULL);
    // Get the localtime
    local = localtime(&t);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("%s Broadcast server listening on port %d...\n", asctime(local), PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        if (client_count >= MAX_CLIENTS) {
            printf("Max clients reached. Connection refused.\n");
            close(client_fd);
            pthread_mutex_unlock(&clients_mutex);
            continue;
        }
        pthread_mutex_unlock(&clients_mutex);

        printf("Client connected: fd = %d\n", client_fd);

        // Create a new thread for the client
        int *client_sock = malloc(sizeof(int));
        *client_sock = client_fd;
        pthread_create(&thread_id, NULL, handle_client, client_sock);
        pthread_detach(thread_id);
    }

    close(server_fd);
    return 0;
}
