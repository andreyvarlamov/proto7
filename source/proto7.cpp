#include <varand/varand_types.h>
#include <varand/varand_raylibhelper.h>
#include <varand/varand_math.h>

#include <raylib/raylib.h>
#include <raylib/raymath.h>

#include <varand/varand_util.h>

struct FontAtlas
{
    Texture2D texture;
    int glyphWidth;
    int glyphHeight;
    int glyphsPerRow;
    int glyphCount;
};

static_g FontAtlas gFontAtlas;

void DrawGlyph(Rectangle destRect, int glyphIndex, Color color)
{
    // Texture wraps so this is not even necessary
    glyphIndex = glyphIndex % gFontAtlas.glyphCount;

    int sourceX = (glyphIndex % gFontAtlas.glyphsPerRow) * gFontAtlas.glyphWidth;
    int sourceY = (glyphIndex / gFontAtlas.glyphsPerRow) * gFontAtlas.glyphHeight;
    Rectangle sourceRect = GetRectangle((f32) sourceX, (f32) sourceY, (f32) gFontAtlas.glyphWidth, (f32) gFontAtlas.glyphHeight);

    DrawTexturePro(gFontAtlas.texture, sourceRect, destRect, GetVector2(), 0, color);
}

#define MAP_WIDTH 24
#define MAP_HEIGHT 24

struct GameState
{
    int playerX;
    int playerY;
};

struct GroundPoint
{
    Vector2 pxPos;

    f32 scale;

    int variant;

    f32 rotation;
};

int main(void)
{
    int screenWidth = 1920;
    int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "Initial window");

    InitAudioDevice();

    Music music = LoadMusicStream("resources/20240204 Calling.mp3");

    PlayMusicStream(music);

    gFontAtlas.texture = LoadTexture("resources/bloody_font.png");
    gFontAtlas.glyphWidth = 48;
    gFontAtlas.glyphHeight = 72;
    gFontAtlas.glyphsPerRow = 16;
    gFontAtlas.glyphCount = 256;

    Rectangle destRect = GetRectangle((f32) gFontAtlas.glyphWidth, (f32) gFontAtlas.glyphHeight);
    int glyphsPerRowOnScreen = (int) (screenWidth / destRect.width);
    int glyphsPerColumnOnScreen = (int) (screenHeight / destRect.height);

    Texture2D groundTexture = LoadTexture("resources/GroundBrushes2.png");
    int groundBrushVariants = 4;
    SetTextureFilter(groundTexture, TEXTURE_FILTER_POINT);
    Rectangle groundSourceRect = GetRectangle(64, 64);

    Texture2D wallTexture = LoadTexture("resources/small wall.png");
    SetTextureFilter(groundTexture, TEXTURE_FILTER_POINT);
    Rectangle wallSourceRect = GetRectangle(24, 36);

    Font font = LoadFontEx("resources/LuxuriousRoman-Regular.ttf", 30, 0, 100);

    GameState gameStateData = {};
    GameState *gameState = &gameStateData;

    int mapWidth = MAP_WIDTH;
    int mapHeight = MAP_HEIGHT;
    u8 mapGlyphs[MAP_WIDTH * MAP_HEIGHT];
    for (int i = 0; i < ArrayCount(mapGlyphs); i++)
    {
        int x = i % mapWidth;
        int y = i / mapWidth;

        b32 isWall = (x == 0 || x == mapWidth - 1 ||
                      y == 0 || y == mapHeight - 1);

        mapGlyphs[i] = isWall ? '#' : '.';
    }

    int enemyHealths[MAP_WIDTH * MAP_HEIGHT] = {};
    for (int i = 0; i < ArrayCount(mapGlyphs); i++)
    {
        if (GetRandomValue(0,100) < 5)
        {
            enemyHealths[i] = 10;
        }
    }

    Color testColors[MAP_WIDTH * MAP_HEIGHT];
    for (int i = 0; i < ArrayCount(mapGlyphs); i++)
    {
        testColors[i] = GetColor(GetRandomValue(0,255), GetRandomValue(0,255), GetRandomValue(0,255));
    }

    GroundPoint groundPoints[MAP_WIDTH * MAP_HEIGHT];
    for (int i = 0; i < ArrayCount(mapGlyphs); i++)
    {
        GroundPoint *point = groundPoints + i;

        point->variant = GetRandomValue(0, groundBrushVariants - 1);
        point->scale = 1.0f + GetRandomValue(0, 255) / 1024.0f;

        int x = i % mapWidth;
        int y = i / mapWidth;

        f32 xPx = (f32) x * gFontAtlas.glyphWidth + gFontAtlas.glyphWidth / 2.0f;
        f32 yPx = (f32) y * gFontAtlas.glyphHeight + gFontAtlas.glyphHeight / 2.0f;

        f32 offsetXPct = GetRandomValue(0, 1000) / 10000.0f - 0.05f; // [-0.05, 0.05]
        f32 offsetYPct = GetRandomValue(0, 1000) / 10000.0f - 0.05f;

        xPx += offsetXPct * gFontAtlas.glyphWidth;
        yPx += offsetYPct * gFontAtlas.glyphHeight;

        point->pxPos = GetVector2(xPx, yPx);

        point->rotation = (f32) GetRandomValue(0, 359);
    }

    gameState->playerX = 1;
    gameState->playerY = 1;

    Camera2D camera = {};
    camera.target = GetVector2(gameState->playerX * gFontAtlas.glyphWidth + gFontAtlas.glyphWidth / 2.0f,
                               gameState->playerY * gFontAtlas.glyphHeight + gFontAtlas.glyphHeight / 2.0f);
    camera.offset = GetVector2(screenWidth / 2.0f, screenHeight / 2.0f);
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    Matrix matrix = GetCameraMatrix2D(camera);

    SetTargetFPS(60);
	
    while (!WindowShouldClose())
    {
        // Update
        UpdateMusicStream(music);

        int playerDesiredX = gameState->playerX;
        int playerDesiredY = gameState->playerY;

        if (IsKeyPressed(KEY_D) || IsKeyPressedRepeat(KEY_D))
        {
            playerDesiredX++;
        }

        if (IsKeyPressed(KEY_A) || IsKeyPressedRepeat(KEY_A))
        {
            playerDesiredX--;
        }

        if (IsKeyPressed(KEY_W) || IsKeyPressedRepeat(KEY_W))
        {
            playerDesiredY--;
        }

        if (IsKeyPressed(KEY_S) || IsKeyPressedRepeat(KEY_S))
        {
            playerDesiredY++;
        }

        if (enemyHealths[playerDesiredY * mapWidth + playerDesiredX] > 0)
        {
            enemyHealths[playerDesiredY * mapWidth + playerDesiredX] -= 4;
        }
        else if (mapGlyphs[playerDesiredY * mapWidth + playerDesiredX] != '#')
        {
            gameState->playerX = playerDesiredX;
            gameState->playerY = playerDesiredY;
            camera.target = GetVector2(gameState->playerX * gFontAtlas.glyphWidth + gFontAtlas.glyphWidth / 2.0f,
                                       gameState->playerY * gFontAtlas.glyphHeight + gFontAtlas.glyphHeight / 2.0f);
        }

        // Draw
        BeginDrawing();
            ClearBackground(BLACK);

            BeginMode2D(camera);

                for (int i = 0; i < ArrayCount(mapGlyphs); i++)
                {
                    int x = i % mapWidth;
                    int y = i / mapWidth;

                    int wPx = gFontAtlas.glyphWidth;
                    int hPx = gFontAtlas.glyphHeight;
                    int xPx = x * wPx;
                    int yPx = y * hPx;

                    // DrawRectangle(xPx, yPx, wPx, hPx, testColors[i]);
                    
                    GroundPoint *point = groundPoints + i;
                    groundSourceRect.y = point->variant * groundSourceRect.height;
                    f32 baseScale = 5.0f;
                    f32 sWidth = groundSourceRect.width * baseScale * point->scale;
                    f32 sHeight = groundSourceRect.height * baseScale * point->scale;
                    Rectangle groundDistRect = GetRectangle(point->pxPos.x, point->pxPos.y, sWidth, sHeight);
                    DrawTexturePro(groundTexture, groundSourceRect, groundDistRect, GetVector2(sWidth / 2.0f, sHeight / 2.0f), point->rotation, WHITE);

                }

                for (int i = 0; i < ArrayCount(mapGlyphs); i++)
                {
                    int x = i % mapWidth;
                    int y = i / mapWidth;

                    int wPx = gFontAtlas.glyphWidth;
                    int hPx = gFontAtlas.glyphHeight;
                    int xPx = x * wPx;
                    int yPx = y * hPx;

                    Rectangle glyphRect = GetRectangle((f32) xPx, (f32) yPx, (f32) wPx, (f32) hPx);
                    if (mapGlyphs[i] == '#')
                    {
                        if (((y + 1) * mapWidth + x < mapWidth * mapHeight) &&
                            (mapGlyphs[(y + 1) * mapWidth + x] == '#'))
                        {
                            wallSourceRect.y = 0 * wallSourceRect.height;
                        }
                        else
                        {
                            wallSourceRect.y = 1 * wallSourceRect.height;
                        }

                        DrawTexturePro(wallTexture, wallSourceRect, glyphRect, GetVector2(), 0.0f, WHITE);
                    }
                    else if (enemyHealths[i] > 0)
                    {
                        DrawGlyph(glyphRect, 9*16+1, RAYWHITE);
                    }
                    else if (mapGlyphs[i] != '.')
                    {
                        DrawGlyph(glyphRect, mapGlyphs[i], GetColor(255, 255, 255));
                    }
                }

                Rectangle playerRect = GetRectangle((f32) gameState->playerX * gFontAtlas.glyphWidth,
                                                    (f32) gameState->playerY * gFontAtlas.glyphHeight,
                                                    (f32) gFontAtlas.glyphWidth,
                                                    (f32) gFontAtlas.glyphHeight);

                DrawGlyph(playerRect, '@', YELLOW);

                DrawRectangle(-200, -200, 200, gFontAtlas.glyphHeight * mapHeight + 400, BLACK);
                DrawRectangle(-200, -200, gFontAtlas.glyphWidth * mapWidth + 400, 200, BLACK);
                DrawRectangle(gFontAtlas.glyphWidth * mapWidth, -200, 200, gFontAtlas.glyphHeight * mapHeight + 400, BLACK);
                DrawRectangle(-200, gFontAtlas.glyphHeight * mapHeight, gFontAtlas.glyphWidth * mapWidth + 400, 200, BLACK);
            EndMode2D();

            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
            int mX = (int) mouseWorldPos.x / gFontAtlas.glyphWidth;
            int mY = (int) mouseWorldPos.y / gFontAtlas.glyphHeight;

            if (mX >= 0 && mX < mapWidth && mY >= 0 && mY < mapHeight)
            {
                if (mapGlyphs[mY * mapWidth + mX] == '#')
                {
                    DrawTextEx(font, "Wall", GetVector2(20.0f, 20.0f), (float)font.baseSize, 2, WHITE);
                }
                else if (gameState->playerX == mX && gameState->playerY == mY)
                {
                    DrawTextEx(font, "Player", GetVector2(20.0f, 20.0f), (float)font.baseSize, 2, WHITE);
                }
                else if (enemyHealths[mY * mapWidth + mX] > 0)
                {
                    DrawTextEx(font, TextFormat("Sterling Ant at %d/%d HP", enemyHealths[mY * mapWidth + mX], 10), GetVector2(20.0f, 20.0f), (float)font.baseSize, 2, WHITE);
                }

            }

            DrawFPS(5, 5);

#if 0
            // Draw test atlas
            int glyphIndex = 0;
            for (int y = 0; y < glyphsPerColumnOnScreen; y++)
            {
                for (int x = 0; x < glyphsPerRowOnScreen; x++)
                {
                    destRect.x = x * destRect.width;
                    destRect.y = y * destRect.height;

                    DrawGlyph(destRect, glyphIndex++, GetColor(255, 255, 255));
                }
            }
#endif

        EndDrawing();
    }

    // UnloadMusicStream(music); 

    // CloseAudioDevice();

    CloseWindow();
    
    return 0;
}
