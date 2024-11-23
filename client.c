#include "game.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>
#include <time.h>

#define MOVE_SPEED 5.0f     // Movement speed
#define TURN_SPEED 170.0f    // Turn speed in degrees per second
#define GRAVITY -9.8f       // Gravity

typedef struct {
    Vector3 position;
    float velocityY;
    bool isGrounded;
    float yaw;    // Horizontal angle
    float pitch;  // Vertical angle
} Player;

typedef struct {
    Vector3 position;
} OtherPlayer;

int player_id;

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
    OtherPlayer other_player = { .position = {0.0f, PLAYER_HEIGHT, 0.0f}};
    InitWindow(800, 600, "Chessboard POV");
    SetTargetFPS(60);

    // Initialize camera
    Camera3D camera = { 0 };
    camera.position = player.position;
    camera.target = (Vector3){ player.position.x, player.position.y, player.position.z - 1.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Variables to track frequency and data rate
    int frameCount = 0;
    double lastTime = GetTime();
    int dataSent = 0;
    int dataReceived = 0;
    float frequency = 0.0f;
    float avgBitsPerSecond = 0.0f;

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
        if (IsKeyDown(KEY_E)) {
            moveDirection.x += 3.0f * cos(DEG2RAD * player.yaw) * MOVE_SPEED * deltaTime;
            moveDirection.z += 3.0f * sin(DEG2RAD * player.yaw) * MOVE_SPEED * deltaTime;
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
        int bytesSent = send(clientSocket, buffer, strlen(buffer), 0);
        if (bytesSent > 0) {
            dataSent += bytesSent;
        }

        // Receive position update from server
        ssize_t bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            sscanf(buffer, "%d %f %f %f %f %f %f", &player_id, &player.position.x, &player.position.y, &player.position.z, &other_player.position.x, &other_player.position.y, &other_player.position.z);
            dataReceived += bytesRead;
            frameCount++;
        }
        printf("%s\n", buffer);
        // Calculate frequency and average bits per second every second
        double currentTime = GetTime();
        if (currentTime - lastTime >= 1.0) {
            frequency = (float)frameCount / (currentTime - lastTime);
            avgBitsPerSecond = ((dataSent + dataReceived) * 8.0f) / (currentTime - lastTime);
            frameCount = 0;
            dataSent = 0;
            dataReceived = 0;
            lastTime = currentTime;
        }

        // Render frame
        BeginDrawing();

        ClearBackground((Color){0, 0, 0, 255});

        Color topColor = (Color){135, 206, 250, 255};    // Light blue (sky)
        Color bottomColor = (Color){25, 25, 112, 255};   // Dark blue (near horizon)
        DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), topColor, bottomColor);


        BeginMode3D(camera);

        DrawChessboard(BOARD_SIZE, SQUARE_SIZE);
        DrawPlayers(player_id, player.position.x, player.position.y, player.position.z, other_player.position.x, other_player.position.y, other_player.position.z);
        DrawThing();
        EndMode3D();

        DrawText("Move with WASD, look with arrow keys, jump with SPACE", 10, 10, 20, RAYWHITE);
        DrawText(TextFormat("Frequency: %.2f Hz", frequency), 10, 40, 20, RAYWHITE);
        DrawText(TextFormat("Avg bits/s: %.2f", avgBitsPerSecond), 10, 70, 20, RAYWHITE);
        DrawText(TextFormat("X: %.2f  Y: %.2f  Z: %.2f", player.position.x, player.position.y, player.position.z), 10, 100, 20, RAYWHITE);
        DrawText(TextFormat("Session Time: %.2f", GetTime()), 10, 130, 20, RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
    close(clientSocket);
    return EXIT_SUCCESS;
}
