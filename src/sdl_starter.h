#pragma once

#include <SDL2/SDL.h>

// psp = 1, vita = 2, pc = 3
const int SCALE = 2;
const int SCREEN_WIDTH = 480*SCALE;
const int SCREEN_HEIGHT = 272*SCALE;

int startSDL(SDL_Window *window, SDL_Renderer *renderer);
