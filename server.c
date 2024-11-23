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


// Client information structure
typedef struct {
    int clientSocket;
    int player_id;
} ClientInfo;


// Globals
Player playerData [2] = {{ .id = 0, .x = 0.0f, .y = 0.0f, .z = 0.0f }, { .id = 1, .x = 0.0f, .y = 0.0f, .z = 0.0f }};
int nextPlayerId = 0;

pthread_mutex_t playerMutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg) {
    ClientInfo *clientInfo = (ClientInfo *)arg;
    int clientSocket = clientInfo->clientSocket;
    int player_id = clientInfo->player_id;
    free(clientInfo);
    nextPlayerId = (nextPlayerId + 1) % 2;
    char buffer[BUFFER_SIZE];

    while (1) {
        ssize_t bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesRead <= 0) {
            printf("Client %d disconnected.\n", player_id);
            break;
        }
        buffer[bytesRead] = '\0';

        pthread_mutex_lock(&playerMutex);
        sscanf(buffer, "%f %f %f %f %f %f", &playerData[player_id].x, &playerData[player_id].y, &playerData[player_id].z,  &playerData[1 - player_id].x, &playerData[1 - player_id].y, &playerData[1 - player_id].z);
        pthread_mutex_unlock(&playerMutex);

        snprintf(buffer, BUFFER_SIZE, "%d %f %f %f %f %f %f", player_id, playerData[player_id].x, playerData[player_id].y, playerData[player_id].z, playerData[1 - player_id].x, playerData[1 - player_id].y, playerData[1 - player_id].z);
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
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientLen);
        if (clientSocket < 0) {
            perror("Accept failed");
            free(clientSocket);
            continue;
        }

        printf("Client %d connected.\n", nextPlayerId);


        // Allocate memory for client information
        ClientInfo *clientInfo = malloc(sizeof(ClientInfo));
        if (!clientInfo) {
            perror("Memory allocation failed");
            close(clientSocket);
            continue;
        }

        // Set client information
        clientInfo->clientSocket = clientSocket;
        clientInfo->player_id = nextPlayerId;


        pthread_t clientThread;
        pthread_create(&clientThread, NULL, handle_client, clientInfo);
        pthread_detach(clientThread);

        //nextPlayerId = (nextPlayerId + 1) % 2;
    }

    close(serverSocket);
    return EXIT_SUCCESS;
}
