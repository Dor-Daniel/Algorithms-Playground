#include <stdio.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "raylib.h"
#include "raymath.h"

#ifdef NDEBUG 
#   define NO_PRINTS
#endif

#define RENDER() for (int __x__ = (BeginDrawing(), 0); __x__ != 1; __x__ = (EndDrawing(), 1))

#define C_ANSI_COLOR_RED "\e[0;31m"
#define C_ANSI_COLOR_GREEN "\e[0;32m"
#define C_ANSI_COLOR_RESET "\e[0m"

#if defined(NO_PRINTS) 
#   define println(...) do { } while(0)
#   define printlnc(c,...) do {  } while(0)
#else
#   define println(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)
#   define printlnc(c,...) do { printf(c __VA_ARGS__); printf(C_ANSI_COLOR_RESET "\n"); } while(0)
#endif

#define ERROR(...) printlnc(C_ANSI_COLOR_RED, __VA_ARGS__)
#define INFO(...) printlnc(C_ANSI_COLOR_GREEN, __VA_ARGS__)

#define WINDOW_TITLE "Bin-Packing 2D"
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800
#define TARGET_FPS 5
#define BG_COLOR (Color){ 20, 20, 20, 255 }
#define RECTS_TO_PLACE_COUNT 256
#define RECTS_TO_PLACE_MIN_W 10
#define RECTS_TO_PLACE_MAX_W 100
#define RECTS_TO_PLACE_MIN_H 10
#define RECTS_TO_PLACE_MAX_H 100
#define RECTS_PLACED_COLOR RAYWHITE

typedef struct Rect {
    float w, h;
} Rect;

static struct GlobalContext {
    enum AlgorithmType {
        ALGORITHM_TYPE_NONE = 0,
        ALGORITHM_TYPE_GUILLOTINE,
        ALGORITHM_TYPE_SKYLINE,
        ALGORITHM_TYPE_COUNT
    } alg_type;
    Rect rects_to_place[ RECTS_TO_PLACE_COUNT ];
    size_t rects_placed_count;
    Rectangle rects_placed [ RECTS_TO_PLACE_COUNT ];
    union AlgorithmData {
        struct Guillotine {
            size_t available_rects_count, available_rects_capacity;
            Rectangle* available_rects;
        } guillotine;
        struct Skyline {

        } skyline;
    } data;
} ctx;

static void update(void);
static void handle_input(void);
static void draw_menu(void);
static void randomize_rects_to_place(void);
static void place_rectangle_g(void);
static void place_rectangle_s(void);
static void visualize_s(void);
static void visualize_g(void);
static void init_s(void);
static void init_g(void);
static void clear_s(void);
static void clear_g(void);

int main (void)
{
    INFO("Starting...");
    ERROR("Error...");
#ifdef NO_PRINTS
    SetTraceLogLevel(LOG_NONE);
#endif
    InitWindow(WINDOW_WIDTH + 100, WINDOW_HEIGHT + 100, WINDOW_TITLE);
    SetTargetFPS(TARGET_FPS);
    
    
    
    while (!WindowShouldClose())
    {
        handle_input();
        update();
        
        RENDER()
        {
            ClearBackground(BG_COLOR);
            if (ctx.alg_type == ALGORITHM_TYPE_NONE)
            {
                draw_menu();
            }
            else if (ctx.alg_type == ALGORITHM_TYPE_GUILLOTINE)
            {
                visualize_g();
            }
            else if (ctx.alg_type == ALGORITHM_TYPE_SKYLINE)
            {
                visualize_s();
            }
        }
    }
}

static void clear_s(void)
{

}

static void clear_g(void)
{
    free(ctx.data.guillotine.available_rects);
    ctx.data.guillotine.available_rects          = NULL;
    ctx.data.guillotine.available_rects_capacity = 0;
    ctx.data.guillotine.available_rects_count    = 0;
    ctx.rects_placed_count                       = 0;
}

static void init_s(void)
{
    randomize_rects_to_place();

}

static int compare_Rect(const void* a, const void* b)
{
    Rect r1 = *(Rect*)a;
    Rect r2 = *(Rect*)b;
    return r2.w * r2.h - r1.w * r1.h; 
}

static void init_g(void)
{
    randomize_rects_to_place();
    qsort(ctx.rects_to_place, RECTS_TO_PLACE_COUNT, sizeof(Rect), compare_Rect);
    const size_t initial_capacity = 32ULL;
    ctx.data.guillotine.available_rects = malloc(sizeof(Rectangle) * initial_capacity);
    ctx.data.guillotine.available_rects_capacity = initial_capacity;
    ctx.data.guillotine.available_rects[ ctx.data.guillotine.available_rects_count ++ ] = (Rectangle){
        .x = 0,
        .y = 0,
        .width = WINDOW_WIDTH,
        .height = WINDOW_HEIGHT
    };
}

static void visualize_s(void)
{

}

static void visualize_g(void)
{
    for (size_t i = 0; i < ctx.rects_placed_count; i++)
    {
        DrawRectangleRec(ctx.rects_placed[ i ], RECTS_PLACED_COLOR);
        DrawRectangleLinesEx(ctx.rects_placed[ i ], 1, BLACK);
    }
    for (size_t i = 0; i < ctx.data.guillotine.available_rects_count; i++)
    {
        DrawRectangleLinesEx(ctx.data.guillotine.available_rects[ i ], 1, RED);
    }
}

static void place_rectangle_g(void)
{
    assert(ctx.data.guillotine.available_rects);
    if (ctx.rects_placed_count >= RECTS_TO_PLACE_COUNT)
    {
        INFO("There is no rects to place, Try increasing the value of 'RECTS_TO_PLACE_COUNT'");
        return;
    }

    Rect to_place = ctx.rects_to_place[ ctx.rects_placed_count ];
    Rectangle best = { .x = -1 };
    size_t best_idx = 0;

    for (size_t i = 0; i < ctx.data.guillotine.available_rects_count; i++)
    {
        Rectangle curr = ctx.data.guillotine.available_rects[ i ];
        if (to_place.h <= curr.height && to_place.w <= curr.width)
        {
            float current = fminf(curr.height - to_place.h, curr.width - to_place.w);
            float best_so_far = fminf(best.height - to_place.h, best.width - to_place.w);
            bool is_better_then_best = current < best_so_far;

            if (best.x == -1 || is_better_then_best)
            {
                best = curr;
                best_idx = i;
            }
        }
    }
    bool rotated = false;
    Rect to_place_rotated = { .w = to_place.h, .h = to_place.w };
    for (size_t i = 0; i < ctx.data.guillotine.available_rects_count; i++)
    {
        Rectangle curr = ctx.data.guillotine.available_rects[ i ];
        if (to_place_rotated.h <= curr.height && to_place_rotated.w <= curr.width)
        {
            float current = fminf(curr.height - to_place_rotated.h, curr.width - to_place_rotated.w);
            float best_so_far = fminf(best.height - to_place_rotated.h, best.width - to_place_rotated.w);
            bool is_better_then_best = current < best_so_far;

            if (best.x == -1 || is_better_then_best)
            {
                best = curr;
                rotated = true;
                best_idx = i;
            }
        }
    }

    if (best.x == -1)
    {
        // Failed to insert
        ERROR("Failed to insert rectangle.");
          return;
    }

    Rectangle left = { 0 }, bottom = { 0 };
    
    if (best.height < best.width)
    {
        left = (Rectangle){ 
            .x = rotated ? best.x + to_place_rotated.w : best.x + to_place.w, 
            .y = best.y, 
            .width = rotated ? best.width - to_place_rotated.w : best.width - to_place.w, 
            .height = rotated ? to_place_rotated.h : to_place.h 
        }; 
        
        bottom = (Rectangle){ 
            .x = best.x, 
            .y = rotated ? best.y + to_place_rotated.h : best.y + to_place.h, 
            .width = best.width, 
            .height = rotated ? best.height - to_place_rotated.h : best.height - to_place.h 
        };
    }
    else
    {
        left = (Rectangle){ 
            .x = rotated ? best.x + to_place_rotated.w : best.x + to_place.w, 
            .y = best.y, 
            .width = rotated ? best.width - to_place_rotated.w : best.width - to_place.w, 
            .height = best.height 
        }; 
        
        bottom = (Rectangle){ 
            .x = best.x, 
            .y = rotated ? best.y + to_place_rotated.h : best.y + to_place.h, 
            .width = rotated ? to_place_rotated.w : to_place.w, 
            .height = rotated ? best.height - to_place_rotated.h : best.height - to_place.h 
        }; 
    }

    ctx.rects_placed[ ctx.rects_placed_count++ ] = rotated ? 
         (Rectangle){ .x = best.x, .y = best.y, .width = to_place.h, .height = to_place.w } :
         (Rectangle){ .x = best.x, .y = best.y, .width = to_place.w, .height = to_place.h };
    ctx.data.guillotine.available_rects[ best_idx ] = left;

    if (ctx.data.guillotine.available_rects_capacity <= ctx.data.guillotine.available_rects_count)
    {  
        size_t new_cap = ctx.data.guillotine.available_rects_capacity * 2;
        void * new_block = realloc(ctx.data.guillotine.available_rects, new_cap * sizeof(Rectangle));
        assert(new_block);
        ctx.data.guillotine.available_rects = new_block;
        ctx.data.guillotine.available_rects_capacity = new_cap;
    }

    ctx.data.guillotine.available_rects[ ctx.data.guillotine.available_rects_count++ ] = bottom;
}

static void place_rectangle_s(void)
{

}

static void randomize_rects_to_place(void)
{
    SetRandomSeed(GetTime());
    for (size_t i = 0; i < RECTS_TO_PLACE_COUNT; i++)
    {
        // float w = (RECTS_TO_PLACE_MIN_W + RECTS_TO_PLACE_MAX_W) * 0.5F;
        // float h = (RECTS_TO_PLACE_MIN_H + RECTS_TO_PLACE_MAX_H) * 0.5F;
        float w = GetRandomValue(RECTS_TO_PLACE_MIN_W, RECTS_TO_PLACE_MAX_W);
        float h = GetRandomValue(RECTS_TO_PLACE_MIN_H, RECTS_TO_PLACE_MAX_H);
        ctx.rects_to_place[ i ] = (Rect){ .w = w, .h = h };
    }
}

static void draw_menu(void)
{
    int h, w, x, y; Color c;

    const char* main_text = "Bin Packing 2D";
    h = WINDOW_HEIGHT / 6.0F;
    w = MeasureText(main_text, h);
    c = RAYWHITE;
    x = (WINDOW_WIDTH - w) / 2.0F;  
    y = WINDOW_HEIGHT / 3.0F - h / 2.0F;
    DrawText(main_text, x, y, h, c);
    const char* second_text = "Algorithm Visualization";
    y += h + WINDOW_HEIGHT / 10.0F;
    h = WINDOW_HEIGHT / 10.0F;
    w = MeasureText(second_text, h);
    x = (WINDOW_WIDTH - w) / 2.0F;
    DrawText(second_text, x, y, h, c);
    int r = 25;
    const char * guillotine = "Guillotine";
    y += h + WINDOW_HEIGHT / 10.0F;
    h = WINDOW_HEIGHT / 12.0F;
    w = MeasureText(guillotine, h);
    x = (WINDOW_WIDTH - w) / 2.0F;
    DrawText(guillotine, x, y, h, c);
    DrawCircleLines(x - 3 * r, y + h / 2.0F, r, c);
    const char * opt1 = "A";
    w = MeasureText(opt1, r );
    DrawText(opt1, x - 3 * r - w / 2, y + (h  - r) / 2.0F, r, c);
    const char* skyline = "Skyline";
    y += h + WINDOW_HEIGHT / 18.0F;
    w = MeasureText(skyline, h);
    x = (WINDOW_WIDTH - w) / 2.0F;
    DrawText(skyline, x, y, h, c);
    DrawCircleLines(x - 3 * r, y + h / 2.0F, r, c);
    const char * opt2 = "B";
    w = MeasureText(opt2, r );
    DrawText(opt2, x - 3 * r - w / 2, y + (h  - r) / 2.0F, r, c);
}

static void handle_input(void)
{
    if (ctx.alg_type == ALGORITHM_TYPE_NONE)
    {
        if (IsKeyPressed(KEY_A))
        {
            ctx.alg_type = ALGORITHM_TYPE_GUILLOTINE;
            init_g();
        }
        else if (IsKeyPressed(KEY_B))
        {
            ctx.alg_type = ALGORITHM_TYPE_SKYLINE;
            init_s();
        }
    }
    else if (ctx.alg_type == ALGORITHM_TYPE_GUILLOTINE)
    {
        if (IsKeyPressed(KEY_R))
        {
            clear_g();
            ctx.alg_type = ALGORITHM_TYPE_NONE;
        }
        else if (IsKeyPressed(KEY_SPACE))
        {

        }
    }
    else if (ctx.alg_type == ALGORITHM_TYPE_SKYLINE)
    {
        if (IsKeyPressed(KEY_R))
        {
            clear_s();
            ctx.alg_type = ALGORITHM_TYPE_NONE;
        }
        else if (IsKeyPressed(KEY_SPACE))
        {
            place_rectangle_s();
        }
    }
}

static void update(void)
{
    switch (ctx.alg_type)
    {
        case ALGORITHM_TYPE_GUILLOTINE:
        {
            place_rectangle_g();
        } break;
        case ALGORITHM_TYPE_SKYLINE:
        {

        } break;
        default: break;
    }
}
