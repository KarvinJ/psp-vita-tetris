#include "sdl_starter.h"
#include "sdl_assets_loader.h"
#include <time.h>
#include <string>

const int PLAYER_SPEED = 300*scale;

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
SDL_GameController* controller = nullptr;

Sprite playerSprite;

Mix_Chunk *sound = nullptr;
Mix_Music *music = nullptr;

bool isGamePaused;

SDL_Texture *pauseTexture = nullptr;
SDL_Rect pauseBounds;

SDL_Texture *scoreTexture = nullptr;
SDL_Rect scoreBounds;

int score;

TTF_Font *fontSquare = nullptr;

SDL_Rect ball = {SCREEN_WIDTH / 2 + 50, SCREEN_HEIGHT / 2, 32, 32};

int ballVelocityX = 200 * scale;
int ballVelocityY = 200 * scale;

int colorIndex;

SDL_Color colors[] = {
    {128, 128, 128, 0}, // gray 
    {255, 255, 255, 0}, // white
    {255, 0, 0, 0},     // red
    {0, 255, 0, 0},     // green
    {0, 0, 255, 0},     // blue
    {255, 255, 0, 0},   // brown
    {0, 255, 255, 0},   // cyan
    {255, 0, 255, 0},   // purple
};

void quitGame()
{
    Mix_FreeChunk(sound);
    SDL_DestroyTexture(playerSprite.texture);
    SDL_DestroyTexture(pauseTexture);
    SDL_GameControllerClose(controller);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

void handleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) 
    {
        if (event.type == SDL_QUIT) 
        {
            quitGame();
            exit(0);
        }

//when I need a more precise input i should use this method of input reading
        if (event.type == SDL_CONTROLLERBUTTONDOWN && event.cbutton.button == SDL_CONTROLLER_BUTTON_START)
        {
            isGamePaused = !isGamePaused;
            Mix_PlayChannel(-1, sound, 0);
        }
    }
}

int rand_range(int min, int max)
{
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void update(float deltaTime)
{
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP) && playerSprite.bounds.y > 0) 
    {
        playerSprite.bounds.y -= PLAYER_SPEED * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN) && playerSprite.bounds.y < SCREEN_HEIGHT - playerSprite.bounds.h) 
    {
        playerSprite.bounds.y += PLAYER_SPEED * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) && playerSprite.bounds.x > 0) 
    {
        playerSprite.bounds.x -= PLAYER_SPEED * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) && playerSprite.bounds.x < SCREEN_WIDTH - playerSprite.bounds.w) 
    {
        playerSprite.bounds.x += PLAYER_SPEED * deltaTime;
    }

    if (ball.x < 0 || ball.x > SCREEN_WIDTH - ball.w)
    {
        ballVelocityX *= -1;

        colorIndex = rand_range(0, 5);
    }

    else if (ball.y < 0 || ball.y > SCREEN_HEIGHT - ball.h)
    {
        ballVelocityY *= -1;

        colorIndex = rand_range(0, 5);
    }

    else if (SDL_HasIntersection(&playerSprite.bounds, &ball))
    {
        ballVelocityX *= -1;
        ballVelocityY *= -1;

        colorIndex = rand_range(0, 5);

        Mix_PlayChannel(-1, sound, 0);

        score++;

        std::string scoreString = "score: " + std::to_string(score);
        updateTextureText(scoreTexture, scoreString.c_str(), fontSquare, renderer);
    }

    ball.x += ballVelocityX * deltaTime;
    ball.y += ballVelocityY * deltaTime;
}

void render()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, colors[colorIndex].r, colors[colorIndex].g, colors[colorIndex].b, 255);

    SDL_RenderFillRect(renderer, &ball);

    renderSprite(renderer, playerSprite);

    if (isGamePaused)
    {
        SDL_RenderCopy(renderer, pauseTexture, NULL, &pauseBounds);
    }

    SDL_QueryTexture(scoreTexture, NULL, NULL, &scoreBounds.w, &scoreBounds.h);
    scoreBounds.x = 200 * scale;
    scoreBounds.y = scoreBounds.h / 2;
    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreBounds);

    SDL_RenderPresent(renderer);
}

int main(int argc, char *args[])
{
    window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
   
   // Using SDL_RENDERER_PRESENTVSYNC effectively synchronizes your rendering with the display's refresh rate, which should cap the framerate to the refresh rate
    //  of the display (commonly 60 Hz, resulting in 60 FPS). This means that, in most cases, you don't need to manually cap the framerate since VSync already
    //  limits the maximum framerate to match the display's refresh rate.
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
 
    if(startSDL(window, renderer) > 0) 
    {
        return 1;
    }
    
    controller = SDL_GameControllerOpen(0);
  
    fontSquare = TTF_OpenFont("square_sans_serif_7.ttf", 18*scale);

    // load title
    updateTextureText(scoreTexture, "Score: 0", fontSquare, renderer);

    updateTextureText(pauseTexture, "Game Paused", fontSquare, renderer);

    SDL_QueryTexture(pauseTexture, NULL, NULL, &pauseBounds.w, &pauseBounds.h);
    pauseBounds.x = SCREEN_WIDTH / 2 - pauseBounds.w / 2;
    pauseBounds.y = 100;
    // After I use the &titleRect.w, &titleRect.h in the SDL_QueryTexture.
    //  I get the width and height of the actual texture
    
    //The path of the file references the build folder
    playerSprite = loadSprite(renderer, "alien_1.png", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

    sound = loadSound("laser.wav");

    // method to reduce the volume of the sound in half.
    Mix_VolumeChunk(sound, MIX_MAX_VOLUME / 2);

    // Load music file (only one data piece, intended for streaming)
    music = loadMusic("music.wav");

    // reduce music volume in half
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2);

    // Start playing streamed music, put -1 to loop indifinitely
    Mix_PlayMusic(music, -1);

    Uint32 previousFrameTime = SDL_GetTicks();
    Uint32 currentFrameTime = previousFrameTime;
    float deltaTime = 0.0f;

    while (true)
    {
        currentFrameTime = SDL_GetTicks();
        deltaTime = (currentFrameTime - previousFrameTime) / 1000.0f;
        previousFrameTime = currentFrameTime;

        SDL_GameControllerUpdate();

        handleEvents();

        if(!isGamePaused)
        {
            update(deltaTime);
        }

        render();
    }

    quitGame();
}