#ifndef CHIP8_GUI_H
#define CHIP8_GUI_H

#include "common.h"
#include <SDL2/SDL.h>
#include <stdexcept>

#define COLOR_BLACK 0x00'00'00'00
#define COLOR_MONOCHROME 0x00'00'79'39

/// mapping 16 chip8 keys
constexpr byte key_mapping[] = {
    SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4,
    SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_R,
    SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_F,
    SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_C, SDL_SCANCODE_V
};

class Gui {
public:
    Gui(int width, int height, int pixel_size)
        : width(width), height(height), pixel_size(pixel_size)
    {
        init();
    }

    ~Gui() {
        if (window)
            SDL_DestroyWindow(window);

        SDL_Quit();
    }

    /// clear screen
    void clear() {
        SDL_FillRect(surface, NULL, COLOR_BLACK);
        SDL_UpdateWindowSurface(window);
    }

    /// update screen with video ram data
    void update_screen(byte *vram) {
        SDL_Rect rect;
        rect.h = pixel_size;
        rect.w = pixel_size;

        for (int i = 0; i < height; i++) {
            rect.y = i * pixel_size;
            rect.x = 0;
            for (int j = 0; j < width; j++) {
                if (vram[width * i + j]) {
                    SDL_FillRect(surface, &rect, COLOR_MONOCHROME);
                } else {
                    SDL_FillRect(surface, &rect, COLOR_BLACK);
                }
                rect.x += pixel_size;
            }
        }

        SDL_UpdateWindowSurface(window);
    }

    /// update key buffer
    void update_keys(bool *keys) {
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        for (byte i = 0; i < sizeof(key_mapping); i++) {
            keys[i] = state[key_mapping[i]] == 1;
        }
    }

    /// get ticks from program startup
    uint32_t get_ticks() {
        return SDL_GetTicks();
    }

    /// poll SDL events, return true if should quit
    bool should_quit() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return true;
            }
        }
        return false;
    }

private:
    int width;
    int height;
    int pixel_size;

    SDL_Window *window;
    SDL_Surface *surface;

    /// intialize SDL context and window
    void init() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw std::runtime_error("initialize sdl failed");
        }

        window = SDL_CreateWindow("CHIP8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            width * pixel_size, height * pixel_size, SDL_WINDOW_SHOWN);
        if (!window) {
            throw std::runtime_error("create sdl window failed");
        }

        surface = SDL_GetWindowSurface(window);
    }
};

#endif
