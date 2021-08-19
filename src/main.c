
#include "sdltools.h"

int main(int argc, char **argv)
{
    CHIP8 chip;

    int i, j;
    int success, color, running = 1, idx;
    uint64_t start, end;
    float passed, delay;

    SDL_Event event;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Rect rects[WIDTH * HEIGHT];

    char file_name[16];
    unsigned int start_addr = 0x200;
    FILE *program_file;

    // If no program specified, exit
    if (argc > 1)
        strcpy(file_name, argv[1]);
    else
        fprintf(stderr, "File not found, try again.\n");


    // Set up graphics
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(window_width, 
                                window_height, 
                                0, 
                                &window, 
                                &renderer);

    // Fill rects with SDL rectangles, these will represent the 64*32 pixels
    fill_rects(rects);


    // TODO: Set up input

    // Initialize chip
    init_chip(&chip);

    // Load program, if failed, exit
    if (!load_program(&chip, start_addr, program_file, file_name))
        return 0;

    while (running)
    {
        // start = SDL_GetPerformanceCounter();

        while (SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    running = 0;
                    break;

                case SDL_KEYDOWN:
                    idx = get_keypad_idx(event.key.keysym.scancode);
                    if (idx == -1)
                        break;

                    chip.keys[idx] = 1;
                    break;

                case SDL_KEYUP:
                    idx = get_keypad_idx(event.key.keysym.scancode);
                    if (idx == -1)
                        break;

                    chip.keys[idx] = 0;
                    break;
            } 
        }

        // Run a single CPU cycle
        cycle(&chip);

        // Set the rectangles on/off depending on 
        if (chip.draw)
        {
            for (i = 0; i < WIDTH; i++)
            {
                for (j = 0; j < HEIGHT; j++)
                {
                    color = (chip.screen[IX(i, j)] == 1) ? 255 : 0;
                    SDL_SetRenderDrawColor(renderer, color, color, color, 255);
                    SDL_RenderFillRect(renderer, &rects[IX(i, j)]);
                }
            }
            // Update screen
            SDL_RenderPresent(renderer);

            // Reset flag
            chip.draw = 0;
        }
        end = SDL_GetPerformanceCounter();

        // TODO: Store key press state (press and release)

        // Limit FPS to 60
        passed = (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
        delay = 16.666f - passed;

        if (delay >= 0)
            SDL_Delay(floor(16.666f - passed));

        // for (i = 0; i < 16; i++){
        //     printf("%d ", chip.keys[i]);
        // }
        // printf("\n");
    }


    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
