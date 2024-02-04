#include <varand/varand_types.h>
#include <varand/varand_util.h>
#include <varand/varand_raylibhelper.h>

#include <raylib/raylib.h>

int main(void)
{
    int screenWidth = 1920;
    int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "Initial window");

    SetTargetFPS(60);
	
    int frameCounter = 0;

    while (!WindowShouldClose())
    {
        // Update
        frameCounter++;

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawText(TextFormat("Hello! You created your first window! Frame: %d", frameCounter), 190, 200, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow();
    
    return 0;
}
