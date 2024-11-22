#include "game.h"

void DrawChessboard(int boardSize, float squareSize) {
    for (int z = 0; z < boardSize; z++) {
        for (int x = 0; x < boardSize; x++) {
            Color squareColor = ((x + z) % 2 == 0) ? DARKGRAY : LIGHTGRAY;

            Vector3 position = {
                (x - boardSize / 2) * squareSize,
                0.0f,
                (z - boardSize / 2) * squareSize
            };

            DrawCube(position, squareSize, 0.1f, squareSize, squareColor);
            DrawCubeWires(position, squareSize, 0.1f, squareSize, BLACK);
        }
    }
}
