#ifndef GAME_H
#define GAME_H

#include "raylib.h"

// Constants
#define BOARD_SIZE 8
#define SQUARE_SIZE 2.0f
#define PORT 12345
#define BUFFER_SIZE 256
#define SERVER_IP "50.188.120.138"

// Function prototypes
void DrawChessboard(int boardSize, float squareSize);

#endif
