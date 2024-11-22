#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

// Player structure
typedef struct {
    int id;
    float x, y, z;
} Player;

// Globals
Player playerData = { .id = 1, .x = 0.0f, .y = 0.0f, .z = 0.0f };
pthread_mutex_t playerMutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg) {
    int clientSocket = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE];

    while (1) {
        ssize_t bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesRead <= 0) {
            printf("Client disconnected.\n");
            break;
        }
        buffer[bytesRead] = '\0';

        pthread_mutex_lock(&playerMutex);
        sscanf(buffer, "%f %f %f", &playerData.x, &playerData.y, &playerData.z);
        pthread_mutex_unlock(&playerMutex);

        snprintf(buffer, BUFFER_SIZE, "%f %f %f", playerData.x, playerData.y, playerData.z);
        send(clientSocket, buffer, strlen(buffer), 0);
    }

    close(clientSocket);
    return NULL;
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    struct sockaddr_in serverAddress = { 0 };
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        return EXIT_FAILURE;
    }

    if (listen(serverSocket, 1) < 0) {
        perror("Listen failed");
        close(serverSocket);
        return EXIT_FAILURE;
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        struct sockaddr_in clientAddress;
        socklen_t clientLen = sizeof(clientAddress);
        int *clientSocket = malloc(sizeof(int));
        *clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientLen);
        if (*clientSocket < 0) {
            perror("Accept failed");
            free(clientSocket);
            continue;
        }

        printf("Client connected.\n");
        pthread_t clientThread;
        pthread_create(&clientThread, NULL, handle_client, clientSocket);
        pthread_detach(clientThread);
    }

    close(serverSocket);
    return EXIT_SUCCESS;
}
