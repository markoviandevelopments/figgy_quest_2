#include "game.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>

#define PLAYER_HEIGHT 1.8f  // Camera/player height
#define MOVE_SPEED 5.0f     // Movement speed
#define TURN_SPEED 90.0f    // Turn speed in degrees per second
#define GRAVITY -9.8f       // Gravity

typedef struct {
    Vector3 position;
    float velocityY;
    bool isGrounded;
    float yaw;    // Horizontal angle
    float pitch;  // Vertical angle
} Player;

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    struct sockaddr_in serverAddress = { 0 };
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddress.sin_addr);

    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Connection failed");
        close(clientSocket);
        return EXIT_FAILURE;
    }

    printf("Connected to server.\n");

    // Initialize player
    Player player = { .position = { 0.0f, PLAYER_HEIGHT, 0.0f }, .velocityY = 0.0f, .isGrounded = false, .yaw = 0.0f, .pitch = 0.0f };

    InitWindow(800, 600, "Chessboard POV");
    SetTargetFPS(60);

    // Initialize camera
    Camera3D camera = { 0 };
    camera.position = player.position;
    camera.target = (Vector3){ player.position.x, player.position.y, player.position.z - 1.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        // Apply gravity if not grounded
        if (!player.isGrounded) {
            player.velocityY += GRAVITY * deltaTime;
        }

        // Update player's vertical position
        player.position.y += player.velocityY * deltaTime;

        // Check if the player is grounded (on the chessboard)
        if (player.position.y <= PLAYER_HEIGHT) {
            player.position.y = PLAYER_HEIGHT;
            player.velocityY = 0.0f;
            player.isGrounded = true;
        } else {
            player.isGrounded = false;
        }

        // Handle movement
        Vector3 moveDirection = { 0.0f, 0.0f, 0.0f };
        if (IsKeyDown(KEY_W)) {
            moveDirection.x += cos(DEG2RAD * player.yaw) * MOVE_SPEED * deltaTime;
            moveDirection.z += sin(DEG2RAD * player.yaw) * MOVE_SPEED * deltaTime;
        }
        if (IsKeyDown(KEY_S)) {
            moveDirection.x -= cos(DEG2RAD * player.yaw) * MOVE_SPEED * deltaTime;
            moveDirection.z -= sin(DEG2RAD * player.yaw) * MOVE_SPEED * deltaTime;
        }
        if (IsKeyDown(KEY_A)) {
            moveDirection.x += cos(DEG2RAD * (player.yaw - 90)) * MOVE_SPEED * deltaTime;
            moveDirection.z += sin(DEG2RAD * (player.yaw - 90)) * MOVE_SPEED * deltaTime;
        }
        if (IsKeyDown(KEY_D)) {
            moveDirection.x += cos(DEG2RAD * (player.yaw + 90)) * MOVE_SPEED * deltaTime;
            moveDirection.z += sin(DEG2RAD * (player.yaw + 90)) * MOVE_SPEED * deltaTime;
        }

        player.position.x += moveDirection.x;
        player.position.z += moveDirection.z;

        // Jump
        if (IsKeyPressed(KEY_SPACE) && player.isGrounded) {
            player.velocityY = 5.0f;
            player.isGrounded = false;
        }

        // Handle camera rotation
        if (IsKeyDown(KEY_LEFT)) player.yaw -= TURN_SPEED * deltaTime;
        if (IsKeyDown(KEY_RIGHT)) player.yaw += TURN_SPEED * deltaTime;
        if (IsKeyDown(KEY_UP)) player.pitch += TURN_SPEED * deltaTime;
        if (IsKeyDown(KEY_DOWN)) player.pitch -= TURN_SPEED * deltaTime;

        // Limit pitch to avoid flipping
        if (player.pitch > 89.0f) player.pitch = 89.0f;
        if (player.pitch < -89.0f) player.pitch = -89.0f;

        // Update camera position and target
        camera.position = player.position;
        Vector3 forward = {
            cos(DEG2RAD * player.pitch) * cos(DEG2RAD * player.yaw),
            sin(DEG2RAD * player.pitch),
            cos(DEG2RAD * player.pitch) * sin(DEG2RAD * player.yaw)
        };
        camera.target.x = player.position.x + forward.x;
        camera.target.y = player.position.y + forward.y;
        camera.target.z = player.position.z + forward.z;

        // Send position to server
        char buffer[BUFFER_SIZE];
        snprintf(buffer, BUFFER_SIZE, "%f %f %f", player.position.x, player.position.y, player.position.z);
        send(clientSocket, buffer, strlen(buffer), 0);

        // Receive position update from server
        ssize_t bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            sscanf(buffer, "%f %f %f", &player.position.x, &player.position.y, &player.position.z);
        }

        // Render frame
        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(camera);
        DrawChessboard(BOARD_SIZE, SQUARE_SIZE);
        EndMode3D();

        DrawText("Move with WASD, look with arrow keys, jump with SPACE", 10, 10, 20, RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
    close(clientSocket);
    return EXIT_SUCCESS;
}
