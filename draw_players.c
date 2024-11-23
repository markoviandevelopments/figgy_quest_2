#include "game.h"

void DrawPlayers(int id, float x, float y, float z, float x1, float y1, float z1) {
    Color squareColor = id ? (Color){255, 0, 0, 200} : (Color){0, 0, 255, 200};

    Vector3 position = {
        x,
        y - PLAYER_HEIGHT + 0.2f,
        z
         };

    DrawCube(position, 0.15f, 0.3f, 0.15f, squareColor);
    DrawCubeWires(position, 0.15f, 0.3f, 0.15f, BLACK);

    Color square2Color = id ?  (Color){0, 0, 255, 200} : (Color){255, 0, 0, 200};

    Vector3 position2 = {
        x1,
        y1 - PLAYER_HEIGHT + 0.2f,
        z1
         };

    DrawCube(position2, 0.15f, 0.3f, 0.15f, square2Color);
    DrawCubeWires(position2, 0.15f, 0.3f, 0.15f, BLACK);




}
