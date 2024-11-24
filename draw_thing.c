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




    Color square2Color = (Color){25, 255, 255, 180};

    float y = sin(0.1f * GetTime()) + sin(0.14 * GetTime()) + sin(0.42f * GetTime());
    if (y > 2.0f) {
        y -= 2.0f;
    } else {
        y = 0.0f;
    }

    Vector3 position2= {
        -4.0f,
        0.3f + y,
        -4.0f
         };

    DrawCube(position2, 0.3f, 0.15f, 0.3f, square2Color);
    DrawCubeWires(position2, 0.3f, 0.15f, 0.3f, BLACK);




}
