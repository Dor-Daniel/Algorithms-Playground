#include <stdio.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include "raylib.h"
#include "raymath.h"

#define C_ANSI_COLOR_RED "\e[0;31m"
#define C_ANSI_COLOR_GREEN "\e[0;32m"
#define C_ANSI_COLOR_RESET "\e[0m"

#define NO_RAYLIB_PRINTS 1
// #define NO_PRINTS 1

#define WINDOW_TITLE "Bin Packing 2D"
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800
#define BG_COLOR (Color){ 20, 20, 20, 255 }
#define RECTS_COUNT 250
#define RECT_MAX_W 300
#define RECT_MAX_H 300
#define RECT_MIN_W 10
#define RECT_MIN_H 10

#define RENDER() for (int __x__ = (BeginDrawing(), 0); __x__ != 1; __x__ = (EndDrawing(), 1))

#if defined(NO_PRINTS)
#   define println(...) do { } while(0)
#   define printlnc(c,...) do {  } while(0)
#else
#   define println(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)
#   define printlnc(c,...) do { printf(c __VA_ARGS__); printf(C_ANSI_COLOR_RESET "\n"); } while(0)
#endif

typedef struct Rect {
    Vector2 wh;
    Color col;
} Rect;

typedef struct Node {
    bool available;
    Vector2 pos;
    Rect rect;
    struct Node * left, * bottom;
} Node;

Rect rects [ RECTS_COUNT ] = { 0 };

Node root = { 
    .pos = { 0, 0 }, 
    .rect = { .wh = { WINDOW_WIDTH, WINDOW_HEIGHT }, .col = BG_COLOR }, 
    .bottom = NULL, 
    .left = NULL,
    .available = true
};

static void init_rects(void);
static bool pop_rect_rec(Node* n, Rectangle rect);
static bool push_rect_rec(Node* n, Rect r, Rectangle* out_rect);
static void draw_nodes(Node* n);
static void clear_tree_rec(Node* root);

#define clear_tree() clear_tree_rec(&root)
#define pop_rect(rect) pop_rect_rec(&root, rect)
#define push_rect(rect, out_rect) push_rect_rec(&root, rect, out_rect)

int main(int argc, char* argv[])
{
    (void)argc; (void)argv;

#if defined(NO_PRINTS) || defined(NO_RAYLIB_PRINTS)
    SetTraceLogLevel(LOG_NONE);
#else
    SetTraceLogLevel(LOG_ALL);
#endif
    
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);
    
    init_rects();
    
    size_t rects_count = 0;
    size_t iteration = 0;

    Rectangle rectangles [ RECTS_COUNT ] = { 0 };

    while(!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_SPACE))
        {
            if (iteration < RECTS_COUNT)
            {
                bool res = push_rect(rects[ iteration ], &rectangles[ rects_count ]);
                if (!res) printlnc(C_ANSI_COLOR_RED, "Insertion failed!");
                else printlnc(
                    C_ANSI_COLOR_GREEN,
                    "Insertion succeed! Inserted Rect = { width: %.02f, height %.02f } At Position = { x: %.02f, y: %.02f }", 
                    rectangles[ rects_count ].width,
                    rectangles[ rects_count ].height,
                    rectangles[ rects_count ].x,
                    rectangles[ rects_count ].y
                );
                if (res) rects_count++;
                iteration++;
            }
        }
        else if (IsKeyPressed(KEY_R))
        {
            if (rects_count > 0)
            {
                int i = GetRandomValue(0, rects_count - 1);
                Rectangle to_remove = rectangles[ i ];
                rectangles[ i ] = rectangles[ --rects_count ];
                bool res = pop_rect(to_remove);
                if (!res) printlnc(C_ANSI_COLOR_RED, "Removal failed!");
                else printlnc(
                    C_ANSI_COLOR_GREEN,
                    "Removal succeed! Removed Rect = { width: %.02f, height: %.02f } At position = { x: %.02f, y: %.02f }",
                    to_remove.width,
                    to_remove.height,
                    to_remove.x,
                    to_remove.y
                );
            }
            else
            {
                printlnc(C_ANSI_COLOR_RED, "Removal failed! Thre is no rects.");
            }
        }
        
        RENDER()
        {
            ClearBackground(BG_COLOR);

            draw_nodes(&root);

            for (size_t i = 0; i < rects_count; i++)
            {
                DrawRectangleRec(rectangles[ i ], rects[ i ].col);
                DrawRectangleLinesEx(rectangles[ i ], 5, WHITE);
            }
        }
    }

    clear_tree();

    CloseWindow();

    return 0;
}

static bool push_rect_rec(Node* n, Rect r, Rectangle* out_rect)
{
    if (!n) return false;

    if (n->available && n->rect.wh.x >= r.wh.x && n->rect.wh.y >= r.wh.y)
    {
        float left_w = n->rect.wh.x - r.wh.x;
        float left_h = n->rect.wh.y - r.wh.y;

        /*
        Two ways to seperate the rect:
        1. bottom will be (node_width,left_h), left will be (left_w, rect_hegiht)
        2. bottom will be (rect_width,left_h), left will be (left_w, node_hegiht)
        */
        
       if (!n->left || !n->bottom)
       {
           float way_1 = fabsf( n->rect.wh.x / left_h - 1.0F ) + fabsf( left_w / r.wh.y - 1.0F );
           float way_2 = fabsf( r.wh.x / left_h - 1.0F ) + fabsf( left_w / n->rect.wh.y - 1.0F );
           Node* left = NULL;
           Node* bottom = NULL;
           if (!n->left) { left = malloc(sizeof(Node)); assert(left); }
           if (!n->bottom) { bottom = malloc(sizeof(Node)); assert(bottom); }
           
           if (left)
           {
               left->bottom    = NULL;
               left->left      = NULL;
               left->available = true;
               left->pos       = Vector2Add(n->pos, (Vector2){ r.wh.x, 0 });
               
            }

            if (bottom)
            {
                bottom->bottom    = NULL;
                bottom->available = true;
                bottom->left      = NULL;
                bottom->pos       = Vector2Add(n->pos, (Vector2){ 0, r.wh.y });
            }
           
           if (way_1 < way_2)
           {
               if (left) left->rect.wh = (Vector2){ left_w, r.wh.y }; 
               if (bottom) bottom->rect.wh = (Vector2){ n->rect.wh.x, left_h }; 
            }
            else
            {
                if (left) left->rect.wh = (Vector2){ left_w, n->rect.wh.y }; 
                if (bottom) bottom->rect.wh = (Vector2){ r.wh.x, left_h }; 
            }
            
            if (bottom) n->bottom = bottom;
            if (left) n->left = left;
        }

        n->available = false;
        n->rect = r;

        if (out_rect) *out_rect = (Rectangle){ .x = n->pos.x, .y = n->pos.y, .width = r.wh.x, .height = r.wh.y };

        return true;
    }
    
    return (n->left && push_rect_rec(n->left, r, out_rect)) || (n->bottom && push_rect_rec(n->bottom, r, out_rect));
}

static bool pop_rect_rec(Node* n, Rectangle rect)
{
    if (n && n->pos.x == rect.x && n->pos.y == rect.y && n->rect.wh.x == rect.width && n->rect.wh.y == rect.height)
    {
        bool res = !n->available;
        n->available = true;
        return res;
    }

    return (n->left && pop_rect_rec(n->left, rect)) || (n->bottom && pop_rect_rec(n->bottom, rect));
}

static void draw_nodes(Node* n)
{
    if (!n) return;
    DrawRectangleLinesEx((Rectangle){n->pos.x, n->pos.y, n->rect.wh.x, n->rect.wh.y}, 3, RED);
    draw_nodes(n->left);
    draw_nodes(n->bottom);
}

static inline void init_rects(void)
{
    SetRandomSeed(time(0));

    for (size_t i = 0; i < RECTS_COUNT; i++)
    {
        float w = GetRandomValue(RECT_MIN_W, RECT_MAX_W);
        float h = GetRandomValue(RECT_MIN_H, RECT_MAX_H);
        rects[ i ] = (Rect){ .wh = { w, h }, .col = ColorFromHSV(GetRandomValue(0, 360), 1.0, 1.0)};
    }
}

static void clear_tree_rec(Node* root)
{
    if (root)
    {
        if (root->left)
        {
            clear_tree_rec(root->left);
            free(root->left);
        }
        if (root->bottom) 
        {
            clear_tree_rec(root->bottom);
            free(root->bottom);
        }
    }
}
