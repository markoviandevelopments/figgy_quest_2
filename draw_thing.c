#include "game.h"

void DrawThing() {
    Color squareColor = (Color){255, 0, 255, 200};

    Vector3 position = {
        4.0f,
        0.3f,
        4.0f
         };

    DrawCube(position, 0.3f, 0.15f, 0.3f, squareColor);
    DrawCubeWires(position, 0.3f, 0.15f, 0.3f, BLACK);


}
