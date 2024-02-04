#include <varand/varand_types.h>
#include <varand/varand_raylibhelper.h>
#include <varand/varand_math.h>

#include <raylib/raylib.h>
#include <raylib/raymath.h>

#define JC_VORONOI_IMPLEMENTATION
#include <misc/jc_voronoi.h>

#include <varand/varand_util.h>

struct FontAtlas
{
    Texture2D texture;
    int glyphWidth;
    int glyphHeight;
    int glyphsPerRow;
    int glyphCount;
};

global_variable FontAtlas gFontAtlas;

void DrawGlyph(Rectangle destRect, int glyphIndex, Color color)
{
    // Texture wraps so this is not even necessary
    glyphIndex = glyphIndex % gFontAtlas.glyphCount;

    int sourceX = (glyphIndex % gFontAtlas.glyphsPerRow) * gFontAtlas.glyphWidth;
    int sourceY = (glyphIndex / gFontAtlas.glyphsPerRow) * gFontAtlas.glyphHeight;
    Rectangle sourceRect = GetRectangle((f32) sourceX, (f32) sourceY, (f32) gFontAtlas.glyphWidth, (f32) gFontAtlas.glyphHeight);

    DrawTexturePro(gFontAtlas.texture, sourceRect, destRect, GetVector2(), 0, color);
}

#define VORONOI_SITE_COUNT 1000

global_variable jcv_rect gBoundingBox;

void RelaxVoronoiPoints(jcv_point *points, int pointCount)
{
    jcv_diagram diagram = {};
    jcv_diagram_generate(pointCount, points, &gBoundingBox, 0, &diagram);

    jcv_site* sites = (jcv_site *) jcv_diagram_get_sites(&diagram);

    for(int i = 0; i < diagram.numsites; i++)
    {
        jcv_site* site = &sites[i];
        jcv_point sum = site->p;
        int count = 1;

        jcv_graphedge* edge = site->edges;
        while(edge)
        {
            sum.x += edge->pos[0].x;
            sum.y += edge->pos[0].y;
            count++;
            edge = edge->next;
        }

        points[site->index].x = sum.x / (jcv_real)count;
        points[site->index].y = sum.y / (jcv_real)count;
    }

    jcv_diagram_free(&diagram);
}

struct GroundSamplingPoint
{
    f32 x;
    f32 y;
    f32 area;
    int variant;
    f32 rotation;
    Color color;
};

void GetGroundSamplingPoints(jcv_point *points, int pointCount,
                             GroundSamplingPoint *groundSamplingPoints)
{
    jcv_diagram diagram = {};
    jcv_diagram_generate(pointCount, points, &gBoundingBox, 0, &diagram);

    jcv_site* sites = (jcv_site *) jcv_diagram_get_sites(&diagram);

    for(int i = 0; i < diagram.numsites; i++)
    {
        jcv_site* site = &sites[i];
        jcv_point sum = site->p;
        int count = 1;

        jcv_point a = points[site->index];

        jcv_graphedge* edge = site->edges;

        f32 siteArea = 0;

        while(edge)
        {
            jcv_point b = edge->pos[0];
            jcv_point c = edge->pos[1];

            siteArea += AbsF(a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y)) * 0.5f;

            edge = edge->next;
        }

        groundSamplingPoints[site->index].x = a.x;
        groundSamplingPoints[site->index].y = a.y;
        groundSamplingPoints[site->index].area = siteArea;
    }

    jcv_diagram_free(&diagram);
}

int main(void)
{
    int screenWidth = 1920;
    int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "Initial window");

    gFontAtlas.texture = LoadTexture("resources/bloody_font.png");
    gFontAtlas.glyphWidth = 48;
    gFontAtlas.glyphHeight = 72;
    gFontAtlas.glyphsPerRow = 16;
    gFontAtlas.glyphCount = 256;

    Rectangle destRect = GetRectangle((f32) gFontAtlas.glyphWidth, (f32) gFontAtlas.glyphHeight);
    int glyphsPerRowOnScreen = (int) (screenWidth / destRect.width);
    int glyphsPerColumnOnScreen = (int) (screenHeight / destRect.height);

    Texture2D groundTexture = LoadTexture("resources/GroundBrushes2.png");
    SetTextureFilter(groundTexture, TEXTURE_FILTER_POINT);
    Rectangle groundSourceRect = GetRectangle(64, 64);

    jcv_point points[VORONOI_SITE_COUNT];
    for (int i = 0; i < ArrayCount(points); i++)
    {
        points[i].x = (f32) GetRandomValue(0, screenWidth - 1);
        points[i].y = (f32) GetRandomValue(0, screenHeight - 1);
    }
    int relaxIterations = 15;
    for (int i = 0; i < relaxIterations; i++)
    {
        RelaxVoronoiPoints(points, ArrayCount(points));
    }
    jcv_diagram diagram = {};
    gBoundingBox = { { 0.0f, 0.0f }, { (f32) screenWidth, (f32) screenHeight } };
    jcv_diagram_generate(ArrayCount(points), points, &gBoundingBox, 0, &diagram);
    GroundSamplingPoint groundSamplingPoints[VORONOI_SITE_COUNT] = {};
    GetGroundSamplingPoints(points, ArrayCount(points), groundSamplingPoints);
    
    for (int i = 0; i < ArrayCount(groundSamplingPoints); i++)
    {
        groundSamplingPoints[i].variant = GetRandomValue(0, 3);
        groundSamplingPoints[i].rotation = GetRandomValue(0, 5000) / 1000.0f;
        groundSamplingPoints[i].color = GetColor(GetRandomValue(0,255) << 24 | 
                                                 GetRandomValue(0,255) << 16 |
                                                 GetRandomValue(0,255) << 8 |
                                                 255);
    }

    SetTargetFPS(60);
	
    while (!WindowShouldClose())
    {
        // Update

        // Draw
        BeginDrawing();
            ClearBackground(BLACK);

            // Draw texture quads
            for (int i = 0; i < ArrayCount(groundSamplingPoints); i++)
            {
                GroundSamplingPoint *point = groundSamplingPoints + i;

                f32 ratio = groundSourceRect.width / groundSourceRect.height;
                f32 area = point->area;

                groundSourceRect.y = point->variant * groundSourceRect.height;

                // Scaled
                f32 sHeight = SqrtF(area / ratio) * 3.0f;
                f32 sWidth = ratio * sHeight;

                Rectangle groundDistRect = GetRectangle(point->x, point->y, sWidth, sHeight);

                DrawTexturePro(groundTexture, groundSourceRect, groundDistRect, GetVector2(sWidth / 2.0f, sHeight / 2.0f), point->rotation, WHITE);
            }

#if 0
            // Draw colored quads
            for (int i = 0; i < ArrayCount(groundSamplingPoints); i++)
            {
                GroundSamplingPoint *point = groundSamplingPoints + i;
                f32 ratio = groundSourceRect.width / groundSourceRect.height;
                f32 area = point->area;

                groundSourceRect.y = point->variant * groundSourceRect.height;

                // Scaled
                f32 sHeight = SqrtF(area / ratio) * 3.0f;
                f32 sWidth = ratio * sHeight;

                Rectangle groundDistRect = GetRectangle(point->x, point->y, sWidth, sHeight);

                DrawRectanglePro(groundDistRect, GetVector2(sWidth / 2.0f, sHeight / 2.0f), point->rotation, point->color);
                f32 circleRadius = SqrtF(point->area / PI32);
                DrawCircle((int) point->x, (int) point->y, circleRadius, point->color);
            }

            // Draw scaled circles
            for (int i = 0; i < ArrayCount(groundSamplingPoints); i++)
            {
                GroundSamplingPoint *point = groundSamplingPoints + i;

                f32 circleRadius = SqrtF(point->area / PI32);
                DrawCircle((int) point->x, (int) point->y, circleRadius, point->color);
            }

            jcv_edge* edge = (jcv_edge *) jcv_diagram_get_edges(&diagram);
            while(edge)
            {
                DrawLine((int) edge->pos[0].x, (int) edge->pos[0].y, (int) edge->pos[1].x, (int) edge->pos[1].y, WHITE);
                edge = (jcv_edge *) jcv_diagram_get_next_edge(edge);
            }
#endif

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

            DrawText(TextFormat("Voronoi relaxation iterations: %d", relaxIterations), 10, 10, 30, WHITE);

        EndDrawing();
    }

    jcv_diagram_free(&diagram);

    CloseWindow();
    
    return 0;
}
