#include "game.h"

void DrawThing() {
    Color squareColor = (Color){255, 0, 255, 200};

    Vector3 position = {
        4.0f + 0.3f * sin(2.31f * GetTime()) + 0.1f * cos(0.4f * GetTime()),
        0.3f,
        4.0f + 0.4f * cos(GetTime())
         };

    DrawCube(position, 0.3f, 0.15f, 0.3f, squareColor);
    DrawCubeWires(position, 0.3f, 0.15f, 0.3f, BLACK);


}
