
#include "debugtools.h"

static int window_height = 320;
static int window_width = 640;

void fill_rects(SDL_Rect rects[WIDTH*HEIGHT])
{
    int i, j;

    for (i = 0; i < WIDTH; i++)
    {
        for (j = 0; j < HEIGHT; j++)
        {
            rects[IX(i, j)].w = window_width / WIDTH;
            rects[IX(i, j)].h = window_height / HEIGHT;
            rects[IX(i, j)].x = i * rects[IX(i, j)].w;
            rects[IX(i, j)].y = j * rects[IX(i, j)].h;
        }
    }
}

int get_keypad_idx(SDL_Scancode k)
{
    switch (k)
    {
        case SDL_SCANCODE_1:
            return 0;

        case SDL_SCANCODE_2:
            return 1;

        case SDL_SCANCODE_3:
            return 2;

        case SDL_SCANCODE_4:
            return 3;

        case SDL_SCANCODE_Q:
            return 4;

        case SDL_SCANCODE_W:
            return 5;

        case SDL_SCANCODE_E:
            return 6;

        case SDL_SCANCODE_R:
            return 7;

        case SDL_SCANCODE_A:
            return 8;

        case SDL_SCANCODE_S:
            return 9;

        case SDL_SCANCODE_D:
            return 10;

        case SDL_SCANCODE_F:
            return 11;

        case SDL_SCANCODE_Z:
            return 12;

        case SDL_SCANCODE_X:
            return 13;

        case SDL_SCANCODE_C:
            return 14;

        case SDL_SCANCODE_V:
            return 15;
        
        default:
            return -1;
    }

    return -1;
}