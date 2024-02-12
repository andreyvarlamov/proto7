#include <varand/varand_types.h>
#include <varand/varand_raylibhelper.h>
#include <varand/varand_math.h>

#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include <raylib/rlgl.h>

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

    f32 depth;
};

int main(void)
{
    int screenWidth = 1920;
    int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "Initial window");

    InitAudioDevice();

    Music music = LoadMusicStream("resources/20240204 Calling.mp3");

    // PlayMusicStream(music);

    gFontAtlas.texture = LoadTexture("resources/bloody_font.png");
    gFontAtlas.glyphWidth = 48;
    gFontAtlas.glyphHeight = 72;
    gFontAtlas.glyphsPerRow = 16;
    gFontAtlas.glyphCount = 256;

    Rectangle destRect = GetRectangle((f32) gFontAtlas.glyphWidth, (f32) gFontAtlas.glyphHeight);
    int glyphsPerRowOnScreen = (int) (screenWidth / destRect.width);
    int glyphsPerColumnOnScreen = (int) (screenHeight / destRect.height);

    Texture2D groundTexture = LoadTexture("resources/GroundBrushes4.png");
    int groundBrushVariants = 3;
    SetTextureFilter(groundTexture, TEXTURE_FILTER_POINT);
    Rectangle groundSourceRect = GetRectangle(32, 32);

    Texture2D wallTexture = LoadTexture("resources/small wall.png");
    SetTextureFilter(groundTexture, TEXTURE_FILTER_POINT);
    Rectangle wallSourceRect = GetRectangle(24, 36);

    Font font = LoadFontEx("resources/LuxuriousRoman-Regular.ttf", 30, 0, 100);

    Shader shader = LoadShader("resources/quad.vs", "resources/quad.fs");

    GameState gameStateData = {};
    GameState *gameState = &gameStateData;

    int mapWidth = MAP_WIDTH;
    int mapHeight = MAP_HEIGHT;

#if 0
    u8 mapGlyphs[MAP_WIDTH * MAP_HEIGHT];
    for (int i = 0; i < ArrayCount(mapGlyphs); i++)
    {
        int x = i % mapWidth;
        int y = i / mapWidth;

        b32 isWall = (x == 0 || x == mapWidth - 1 ||
                      y == 0 || y == mapHeight - 1);

        mapGlyphs[i] = isWall ? '#' : '.';
    }
#else
    u8 mapGlyphs[MAP_WIDTH * MAP_HEIGHT] = {
        35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 
        35, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 35, 
        35, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 254, 46, 46, 46, 46, 46, 46, 35, 
        35, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 35, 
        35, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 46, 254, 46, 46, 46, 46, 46, 254, 254, 46, 46, 46, 35, 
        35, 46, 46, 46, 254, 254, 254, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 254, 254, 46, 46, 46, 35, 
        35, 46, 46, 46, 46, 254, 254, 254, 46, 46, 46, 46, 46, 46, 46, 46, 254, 254, 254, 254, 46, 46, 46, 35, 
        35, 46, 254, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 46, 254, 254, 254, 254, 254, 46, 46, 46, 46, 35, 
        35, 46, 46, 46, 46, 254, 254, 254, 46, 46, 46, 46, 254, 254, 254, 254, 254, 254, 46, 46, 46, 46, 46, 35, 
        35, 46, 46, 46, 46, 254, 254, 46, 46, 46, 46, 254, 254, 254, 254, 46, 46, 46, 46, 46, 46, 254, 46, 35, 
        35, 46, 46, 46, 46, 254, 254, 254, 254, 254, 254, 254, 254, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 35, 
        35, 46, 46, 46, 46, 254, 254, 254, 254, 254, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 35, 
        35, 46, 46, 46, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 35, 
        35, 46, 46, 254, 46, 46, 46, 254, 254, 46, 46, 46, 46, 254, 46, 46, 46, 247, 46, 46, 46, 46, 46, 35, 
        35, 46, 46, 46, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 46, 46, 247, 247, 46, 46, 46, 46, 46, 35, 
        35, 46, 46, 46, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 46, 46, 247, 247, 46, 46, 46, 46, 46, 35, 
        35, 46, 46, 46, 46, 46, 46, 46, 254, 254, 46, 254, 254, 46, 46, 247, 247, 247, 247, 46, 46, 46, 46, 35, 
        35, 46, 46, 46, 46, 46, 46, 46, 254, 254, 254, 254, 254, 46, 247, 247, 247, 247, 247, 247, 46, 46, 46, 35, 
        35, 46, 254, 46, 46, 46, 46, 46, 254, 254, 254, 254, 46, 46, 247, 247, 247, 247, 247, 247, 247, 46, 46, 35, 
        35, 46, 46, 46, 46, 46, 46, 46, 254, 254, 46, 46, 46, 46, 46, 247, 247, 247, 247, 247, 247, 46, 46, 35, 
        35, 46, 46, 46, 254, 254, 254, 254, 254, 46, 46, 254, 46, 46, 46, 46, 46, 247, 247, 247, 46, 46, 46, 35, 
        35, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 247, 247, 247, 46, 35, 
        35, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 35, 
        35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 
    };
#endif

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

        point->variant = 0;

        switch (mapGlyphs[i])
        {
            case 35, 46:
            {
                point->variant = 0;
            } break;

            case 254:
            {
                point->variant = 1;
            } break;

            case 247:
            {
                point->variant = 2;
            } break;

            default: break;
        }

        point->scale = 1.0f + GetRandomValue(0, 255) / 1024.0f;

        int x = i % mapWidth;
        int y = i / mapWidth;

        f32 xPx = (f32) x * gFontAtlas.glyphWidth + gFontAtlas.glyphWidth / 2.0f;
        f32 yPx = (f32) y * gFontAtlas.glyphHeight + gFontAtlas.glyphHeight / 2.0f;

        f32 offsetXPct = 0;// GetRandomValue(0, 1000) / 10000.0f - 0.05f; // [-0.05, 0.05]
        f32 offsetYPct = 0;// GetRandomValue(0, 1000) / 10000.0f - 0.05f;

        xPx += offsetXPct * gFontAtlas.glyphWidth;
        yPx += offsetYPct * gFontAtlas.glyphHeight;

        point->pxPos = GetVector2(xPx, yPx);

        point->depth = GetRandomValue(0, 10000) / 10000.0f;

        point->rotation = 0; //(f32) GetRandomValue(0, 359);
    }

    f32 apronScale = 2.0f;

    int apronScaleLoc = GetShaderLocation(shader, "apronScale");
    SetShaderValue(shader, apronScaleLoc, &apronScale, SHADER_UNIFORM_FLOAT);
    
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

        static_p u8 c1 = 0;//26;
        static_p u8 a1 = 0;//56;
        static_p u8 c2 = 0;//26;
        static_p u8 a2 = 0;//190;//56;

        u8 *toC = &c1;
        u8 *toA = &a1;

        if (IsKeyDown(KEY_LEFT_SHIFT))
        {
            toC = &c2;
            toA = &a2;
        }

        u8 *toInc = toC;

        if (IsKeyDown(KEY_LEFT_CONTROL))
        {
            toInc = toA;
        }

        if (IsKeyPressed(KEY_UP))
        {
            *toInc += 10;
        }
        if (IsKeyPressed(KEY_DOWN))
        {
            *toInc -= 10;
        }

        // Draw
        BeginDrawing();
        {
            ClearBackground(BLACK);

            BeginMode2D(camera);
            {
                {
                    // NOTE: Draw map floor brushes
                    for (int i = 0; i < ArrayCount(mapGlyphs); i++)
                    // for (int i = 128; i < 129; i++)
                    {
                        GroundPoint *point = groundPoints + i;
                        groundSourceRect.y = point->variant * groundSourceRect.height;

                        f32 heightUV = 1.0f / groundBrushVariants;
                        f32 minYUV = heightUV * point->variant;

                        int minYUVLoc = GetShaderLocation(shader, "minYUV");
                        SetShaderValue(shader, minYUVLoc, &minYUV, SHADER_UNIFORM_FLOAT);
                        int heightUVLoc = GetShaderLocation(shader, "heightUV");
                        SetShaderValue(shader, heightUVLoc, &heightUV, SHADER_UNIFORM_FLOAT);

                        // float depth = -1.0f;
                        int depthLoc = GetShaderLocation(shader, "depth");
                        BeginShaderMode(shader);
                        rlEnableDepthTest();
                        SetShaderValue(shader, depthLoc, &point->depth, SHADER_UNIFORM_FLOAT);

                        if (point->variant == 0)
                        {
                            int x = i % mapWidth;
                            int y = i / mapWidth;

                            int wPx = gFontAtlas.glyphWidth;
                            int hPx = gFontAtlas.glyphHeight;
                            int xPx = x * wPx;
                            int yPx = y * hPx;

                            f32 sWidth = gFontAtlas.glyphWidth * apronScale;
                            f32 sHeight = gFontAtlas.glyphHeight * apronScale;
                            Rectangle groundDistRect = GetRectangle(point->pxPos.x, point->pxPos.y, sWidth, sHeight);

                            DrawTexturePro(groundTexture, groundSourceRect, groundDistRect, GetVector2(sWidth / 2.0f, sHeight / 2.0f), point->rotation, WHITE);
                        }

                        EndShaderMode();
                                                rlDisableDepthTest();
                    }

                    // for (int i = 0; i < ArrayCount(mapGlyphs); i++)
                    // for (int i = 128; i < 129; i++)
                    // {
                    //     GroundPoint *point = groundPoints + i;
                    //     groundSourceRect.y = point->variant * groundSourceRect.height;

                    //     if (point->variant != 0)
                    //     {
                    //         int x = i % mapWidth;
                    //         int y = i / mapWidth;

                    //         int wPx = gFontAtlas.glyphWidth;
                    //         int hPx = gFontAtlas.glyphHeight;
                    //         int xPx = x * wPx;
                    //         int yPx = y * hPx;

                    //         f32 baseScale = 1.0f;
                    //         f32 sWidth = groundSourceRect.width * baseScale * point->scale;
                    //         f32 sHeight = groundSourceRect.height * baseScale * point->scale;
                    //         Rectangle groundDistRect = GetRectangle(point->pxPos.x, point->pxPos.y, sWidth, sHeight);
                    //         DrawTexturePro(groundTexture, groundSourceRect, groundDistRect, GetVector2(sWidth / 2.0f, sHeight / 2.0f), point->rotation, WHITE);
                    //     }
                    // }
                }

                // NOTE: Draw checkerboard
                Color col1 = GetColor(c1, c1, c1, a1);
                Color col2 = GetColor(c2, c2, c2, a2);
                for (int i = 0; i < ArrayCount(mapGlyphs); i++)
                {
                    int x = i % mapWidth;
                    int y = i / mapWidth;

                    int wPx = gFontAtlas.glyphWidth;
                    int hPx = gFontAtlas.glyphHeight;
                    int xPx = x * wPx;
                    int yPx = y * hPx;

                    Color col = ((((x + y) % 2) == 0) ? col1 : col2);

                    if (i == 128) { col.r = 255; col.a = 50; }

                    DrawRectangle(xPx, yPx, wPx, hPx, col);
                }
#if 1
                
                // Draw map walls
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
                    else if (mapGlyphs[i] != '.' && mapGlyphs[i] !=  254 && mapGlyphs[i] != 247)
                    {
                        DrawGlyph(glyphRect, mapGlyphs[i], GetColor(255, 255, 255));
                    }
                }
#endif

                // Draw player
                Rectangle playerRect = GetRectangle((f32) gameState->playerX * gFontAtlas.glyphWidth,
                                                    (f32) gameState->playerY * gFontAtlas.glyphHeight,
                                                    (f32) gFontAtlas.glyphWidth,
                                                    (f32) gFontAtlas.glyphHeight);

                DrawGlyph(playerRect, '@', YELLOW);

                DrawRectangle(-200, -200, 200, gFontAtlas.glyphHeight * mapHeight + 400, BLACK);
                DrawRectangle(-200, -200, gFontAtlas.glyphWidth * mapWidth + 400, 200, BLACK);
                DrawRectangle(gFontAtlas.glyphWidth * mapWidth, -200, 200, gFontAtlas.glyphHeight * mapHeight + 400, BLACK);
                DrawRectangle(-200, gFontAtlas.glyphHeight * mapHeight, gFontAtlas.glyphWidth * mapWidth + 400, 200, BLACK);
            }
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

            DrawTextEx(font, TextFormat("C1: (%d,%d,%d,%d)", c1, c1, c1, a1), GetVector2(5.0f, 100.0f), (float)font.baseSize, 2, WHITE);
            DrawTextEx(font, TextFormat("C2: (%d,%d,%d,%d)", c2, c2, c2, a2), GetVector2(5.0f, 130.0f), (float)font.baseSize, 2, WHITE);

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
        }
        EndDrawing();
    }

    // UnloadMusicStream(music); 

    // CloseAudioDevice();

    CloseWindow();
    
    return 0;
}
