#define MY_DEBUG 1

#include "main.h"
#include "gui.cpp"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 1020

#define BAR_HEIGHT 40

#define GRID_WIDTH 128                  // 43
#define GRID_HEIGHT 96 // 30

#define ALIVE true
#define DEAD false


// n - number of random cells to make ALIVE
void RandInit(bool *world, int32 n, int32 seed)
{
    SetRandomSeed(seed);

    int32 alive = 0;

    while (alive != n)
    {
        int32 rx = GetRandomValue(0, GRID_WIDTH);
        int32 ry = GetRandomValue(0, GRID_HEIGHT);
        if(world[ry*GRID_WIDTH+rx] == DEAD)
        {
            world[ry*GRID_WIDTH+rx] = ALIVE;
            alive++;
        }
    }
}


int32 CountNeighbors(bool *world, int32 cellX, int32 cellY)
{
    int32 result = 0;

    for(int32 yd = -1; yd < 2; yd++)
    {
        for(int32 xd = -1; xd < 2; xd++)
        {
            if(yd == 0 && xd == 0) continue;

            int32 x = (GRID_WIDTH + cellX + xd) % GRID_WIDTH;
            int32 y = (GRID_HEIGHT + cellY + yd) % GRID_HEIGHT;

            int32 index = y*GRID_WIDTH+x;
            if(world[index]) result++;
        }
    }

    return result;
}


void NextGen(bool *cur, bool *next)
{
    for(int32 y = 0; y < GRID_HEIGHT; y++)
    {
        for(int32 x = 0; x < GRID_WIDTH; x++)
        {
            int32 index = y*GRID_WIDTH+x;
            bool state = cur[index];
            int32 neighborCount = CountNeighbors(cur, x, y);

            //if(state == ALIVE && neighborCount < 2)                         next[index] = DEAD;
            //if(state == ALIVE && neighborCount > 3)                         next[index] = DEAD;
            if(state == ALIVE && neighborCount > 1 && neighborCount < 4)    next[index] = ALIVE;
            else if(state == DEAD && neighborCount == 3)                    next[index] = ALIVE;
            else                                                            next[index] = DEAD;
        }
    }
}

void WorldToImage(bool *world, Color *img)
{
    for(int32 y = 0; y < GRID_HEIGHT; y++)
    {
        for(int32 x = 0; x < GRID_WIDTH; x++)
        {
            int32 index = y*GRID_WIDTH+x;
            Color c = world[index] ? BLACK : WHITE;
            img[index] = c;
        }
    }
}


void Restart(bool *world, int32 nAlive, int32 seed)
{
    memset(world, 0, GRID_WIDTH * GRID_HEIGHT * sizeof(bool));

    RandInit(world, nAlive, seed);


}

int main() 
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "gol");

    Camera2D camera = { 0 };
    camera.offset = Vec2(0.0f, 0.0f);
    camera.target = Vec2(0.0f, 0.0f);
    camera.zoom = 1.0f;
    
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    bool *cur = (bool *)calloc(GRID_WIDTH*GRID_HEIGHT, sizeof(bool));
    bool *next = (bool *)calloc(GRID_WIDTH*GRID_HEIGHT, sizeof(bool));

    int32 seed = 1;
    bool seedEditMode = false;

    int32 nAlive = GRID_WIDTH*GRID_HEIGHT * 0.5f;
    bool nAliveEditMode = false;

    float simSpeed = 0.5f;
    bool simSpeedEditMode = false;

    int32 gen = 0;

    RandInit(cur, nAlive, seed);

    // cur[0*GRID_WIDTH+1] = true;
    // cur[1*GRID_WIDTH+1] = true;

    Image img = GenImageColor(GRID_WIDTH, GRID_HEIGHT, WHITE);
    WorldToImage(cur, (Color *)img.data);
    Texture2D tex = LoadTextureFromImage(img);

    float lastSimTime = 0.0f;

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        float timePassed = GetTime() - lastSimTime;
        float delay = 1.0f - simSpeed;

        if(timePassed >= delay)
        {
            memset(next, 0, GRID_WIDTH * GRID_HEIGHT * sizeof(bool));
            NextGen(cur, next);
            memcpy(cur, next, GRID_WIDTH * GRID_HEIGHT * sizeof(bool));

            WorldToImage(cur, (Color *)img.data);
            UpdateTexture(tex, img.data);

            gen++;

            lastSimTime = GetTime();
        }


        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawTexturePro(tex, Rec(0, 0, tex.width, tex.height), Rec(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - BAR_HEIGHT), Vec2(), 0.0f, WHITE);
            
            v2 barStart = Vec2(0, SCREEN_HEIGHT - BAR_HEIGHT);
            v2 barEnd = Vec2(SCREEN_WIDTH, SCREEN_HEIGHT - BAR_HEIGHT);

            const int32 padding = 5;
            const int32 spinnerWidth = 50;
            const int32 spinnerHeight = 25;

            v2 elmStart = Vec2(barStart.x+padding, barStart.y+padding);

            if(GUISpinner(Rec(elmStart.x, elmStart.y, spinnerWidth, spinnerHeight), "Seed", &seed, 0, INT32_MAX, &seedEditMode));

            elmStart.x += spinnerWidth + padding;

            if(GUISpinner(Rec(elmStart.x, elmStart.y, spinnerWidth, spinnerHeight), "nAlive", &nAlive, 1, GRID_WIDTH*GRID_HEIGHT, &nAliveEditMode));

            elmStart.x += spinnerWidth + padding;

            GUISlider(Rec(elmStart.x, elmStart.y, spinnerWidth, spinnerHeight), "Sim Speed", &simSpeed, 0.0f, 1.0f, &simSpeedEditMode);
            elmStart.x += spinnerWidth + padding;
            
            const char *genText = TextFormat("GENERATION: %d", gen);
            DrawTextPro(GetFontDefault(), genText, elmStart, Vec2(), 0.0f, 24.0f, 1.0f, BLACK);
            elmStart.x += MeasureText(genText, 24.0f) + padding;


            if(GUIButton(Rec(SCREEN_WIDTH-55, elmStart.y, 50, spinnerHeight), "RESTART"))
            {
                Restart(cur, nAlive, seed);
                WorldToImage(cur, (Color *)img.data);
                UpdateTexture(tex, img.data);
                gen = 0;
            }

            DrawLineEx(barStart, barEnd, 2.0f, BLACK);

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}