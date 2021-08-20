
#include "debugtools.h"

static int window_height = 320;
static int window_width = 640;

// Fills the rectangles with proper values on height, width, etc.
// This is useful in case of resolution changes.
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

// Polls inputs into the chip->keys value
void get_inputs(CHIP8 *chip, int *running)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            *running = 0;
            break;
        }
        if (event.type == SDL_KEYDOWN)
        {
            switch (event.key.keysym.scancode)
            {
                case SDL_SCANCODE_1:
                    chip->keys[0] = 1;
                    break;

                case SDL_SCANCODE_2:
                    chip->keys[1] = 1;
                    break;

                case SDL_SCANCODE_3:
                    chip->keys[2] = 1;
                    break;

                case SDL_SCANCODE_4:
                    chip->keys[3] = 1;
                    break;

                case SDL_SCANCODE_Q:
                    chip->keys[4] = 1;
                    break;

                case SDL_SCANCODE_W:
                    chip->keys[5] = 1;
                    break;

                case SDL_SCANCODE_E:
                    chip->keys[6] = 1;
                    break;

                case SDL_SCANCODE_R:
                    chip->keys[7] = 1;
                    break;

                case SDL_SCANCODE_A:
                    chip->keys[8] = 1;
                    break;

                case SDL_SCANCODE_S:
                    chip->keys[9] = 1;
                    break;

                case SDL_SCANCODE_D:
                    chip->keys[10] = 1;
                    break;

                case SDL_SCANCODE_F:
                    chip->keys[11] = 1;
                    break;

                case SDL_SCANCODE_Z:
                    chip->keys[12] = 1;
                    break;

                case SDL_SCANCODE_X:
                    chip->keys[13] = 1;
                    break;

                case SDL_SCANCODE_C:
                    chip->keys[14] = 1;
                    break;

                case SDL_SCANCODE_V:
                    chip->keys[15] = 1;
                    break;

                default: break;
            }
        }
        if (event.type == SDL_KEYUP)
        {
            switch (event.key.keysym.scancode)
            {
                case SDL_SCANCODE_1:
                    chip->keys[0] = 0;
                    break;

                case SDL_SCANCODE_2:
                    chip->keys[1] = 0;
                    break;

                case SDL_SCANCODE_3:
                    chip->keys[2] = 0;
                    break;

                case SDL_SCANCODE_4:
                    chip->keys[3] = 0;
                    break;

                case SDL_SCANCODE_Q:
                    chip->keys[4] = 0;
                    break;

                case SDL_SCANCODE_W:
                    chip->keys[5] = 0;
                    break;

                case SDL_SCANCODE_E:
                    chip->keys[6] = 0;
                    break;

                case SDL_SCANCODE_R:
                    chip->keys[7] = 0;
                    break;

                case SDL_SCANCODE_A:
                    chip->keys[8] = 0;
                    break;

                case SDL_SCANCODE_S:
                    chip->keys[9] = 0;
                    break;

                case SDL_SCANCODE_D:
                    chip->keys[10] = 0;
                    break;

                case SDL_SCANCODE_F:
                    chip->keys[11] = 0;
                    break;

                case SDL_SCANCODE_Z:
                    chip->keys[12] = 0;
                    break;

                case SDL_SCANCODE_X:
                    chip->keys[13] = 0;
                    break;

                case SDL_SCANCODE_C:
                    chip->keys[14] = 0;
                    break;

                case SDL_SCANCODE_V:
                    chip->keys[15] = 0;
                    break;

                default: break;
            }
        }
    }
}

// Set the array of rectangles to represent the array of pixels that chip holds
void update_screen(CHIP8 *chip,
                   SDL_Renderer *renderer,
                   SDL_Rect rects[WIDTH*HEIGHT])
{
    int c, i, j;

    if (!chip->draw_flag)
        return;

    for (i = 0; i < WIDTH; i++)
    {
        for (j = 0; j < HEIGHT; j++)
        {
            // If the pixel is on, change the rectangle to be white
            // If it's off, change the rectangle to be black
            c = (chip->screen[IX(i,j)] == 1) ? 255 : 0;
            SDL_SetRenderDrawColor(renderer, c, c, c, 255);
            SDL_RenderFillRect(renderer, &rects[IX(i, j)]);
        }
    }

    // Update the screen
    SDL_RenderPresent(renderer);

    // Reset draw flag
    chip->draw_flag = 0;
}

void play_sound(CHIP8 *chip, 
                SDL_AudioDeviceID device, 
                uint32_t wav_length, 
                uint8_t *wav_buffer)
{
    if (chip->sound_flag)
    {
        // Queue audio, then un-pause it
        SDL_QueueAudio(device, wav_buffer, wav_length); 
        SDL_PauseAudioDevice(device, 0);
        chip->sound_flag = 0;
    }
}