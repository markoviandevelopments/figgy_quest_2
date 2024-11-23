#ifndef GAME_H
#define GAME_H

#include "raylib.h"

// Constants
#define BOARD_SIZE 8
#define SQUARE_SIZE 2.0f
#define PORT 12346
#define BUFFER_SIZE 256
#define PLAYER_HEIGHT 1.6f
#define SERVER_IP "50.188.120.138"

// Function prototypes
void DrawChessboard(int boardSize, float squareSize);
void DrawPlayers(int id, float x, float y, float z, float x1, float y1, float z1);
void DrawThing();

#endif
