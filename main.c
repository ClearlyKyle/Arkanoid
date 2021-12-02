#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "SDL2/SDL.h"
#include "SDL2_gfxPrimitives.h"

const int window_width = 800;
const int window_height = 600;

// BALL VALUES
const float ball_radius = 10.0f;
const float ball_velocity = 1.0f;

// PADDLE VALUES
const float paddle_width = 60.0f;
const float paddle_height = 20.0f;
const float paddle_velocity = 3.0f;

static SDL_Window *window;
static SDL_Renderer *renderer;

const SDL_Colour RED = {255, 000, 000, 255};
const SDL_Colour BLUE = {000, 255, 000, 255};
const SDL_Colour GREEN = {000, 000, 255, 255};

struct Ball
{
    float x;
    float y;
    float radius;
    float vel_x;
    float vel_y;
    SDL_Colour colour;
};

void BALL_Draw(struct Ball *ball)
{
    if (filledCircleRGBA(renderer, ball->x, ball->y, ball->radius, ball->colour.r, ball->colour.g, ball->colour.b, ball->colour.a) < 0)
    {
        fprintf(stderr, "[filledCircleRGBA] Failed to draw circle\n");
        exit(EXIT_FAILURE);
    }
}

void BALL_Move(struct Ball *ball)
{
    ball->x += ball->vel_x;
    ball->y += ball->vel_y;

    const float left = ball->x - ball->radius;
    const float right = ball->x + ball->radius;
    const float top = ball->y - ball->radius;
    const float bottom = ball->y + ball->radius;

    if (left < 0)
        ball->vel_x = ball_velocity;
    else if (right > window_width)
        ball->vel_x = -ball_velocity;

    if (top < 0)
        ball->vel_y = ball_velocity;
    else if (bottom > window_height)
        ball->vel_y = -ball_velocity;
}

struct Paddle
{
    float x;
    float y;
    float w;
    float h;
    float vel_x;
    float vel_y;
    SDL_Colour colour;
};

void PADDLE_Draw(struct Paddle *paddle)
{
    // const float half_paddle_width = paddle_width / 2.0f;
    // const float half_paddle_height = paddle_height / 2.0f;

    const float top_right_x = paddle->x + paddle_width;
    const float top_right_y = paddle->y + paddle_height;

    const float bottom_left_x = paddle->x;
    const float bottom_left_y = paddle->y;

    if (rectangleRGBA(renderer, top_right_x, top_right_y, bottom_left_x, bottom_left_y, paddle->colour.r, paddle->colour.g, paddle->colour.b, paddle->colour.a))
    {
        fprintf(stderr, "[rectangleRGBA] Failed to draw rectangle\n");
        exit(EXIT_FAILURE);
    }
}

void PADDLE_Move(struct Paddle *paddle)
{
    paddle->x += paddle->vel_x;
}

void BALL_PADDLE_collision(struct Ball *ball, struct Paddle *paddle)
{
    const float ball_left = ball->x - ball->radius;
    const float ball_right = ball->x + ball->radius;
    const float ball_top = ball->y - ball->radius;
    const float ball_bottom = ball->y + ball->radius;

    const float paddle_left = paddle->x;
    const float paddle_top = paddle->y;
    const float paddle_bottom = paddle->y + paddle_height;
    const float paddle_right = paddle->x + paddle_width;

    //if (!(ball_right >= paddle_left && ball_left <= paddle_right && ball_bottom >= paddle_top && ball_top <= paddle_bottom))
    if (!(ball_right >= paddle_left && ball_left <= paddle_right && ball_bottom >= paddle_top && ball_top <= paddle_bottom))
        return;

    ball->vel_y = -ball_velocity;

    if (ball->x < paddle->x)
    {
        ball->vel_x = -ball_velocity;
    }
    else
    {
        ball->vel_x = ball_velocity;
    }
}

void Handle_SDL_Fail(const char *error_message)
{
    fprintf(stderr, "[ERROR] %s\n", error_message);
    fprintf(stderr, "\t%s\n", SDL_GetError());

    SDL_Quit();

    if (window)
        SDL_DestroyWindow(window);
    if (renderer)
        SDL_DestroyRenderer(renderer);

    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "[ERROR] %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow("Arkinoid", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, 0);
    if (!window)
    {
        fprintf(stderr, "[ERROR] %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        fprintf(stderr, "[ERROR] %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    struct Ball b;
    b.x = window_width / 2;
    b.y = window_height / 2;
    b.vel_x = -ball_velocity;
    b.vel_y = -ball_velocity;
    b.colour = RED;
    b.radius = ball_radius;

    struct Paddle p;
    p.x = window_width / 2;
    p.y = window_height - 50.0f;
    p.w = paddle_width;
    p.h = paddle_height;
    p.colour = BLUE;
    p.vel_x = 0.0f;
    p.vel_y = 0.0f;

    bool quit = false;
    SDL_Event sdlEvent;
    while (!quit)
    {
        printf("Paddle : (%0.1f, %0.1f)\n", p.x, p.y);

        while (SDL_PollEvent(&sdlEvent) != 0)
        {
            if (sdlEvent.type == SDL_QUIT)
            {
                quit = true;
            }
        }
        const Uint8 *keystates = SDL_GetKeyboardState(NULL);
        if (keystates[SDL_SCANCODE_ESCAPE]) // forward
        {
            quit = true;
            break;
        }
        if (keystates[SDL_SCANCODE_A]) // back
        {
            // LEFT
            if (p.x > 1.0f)
            {
                p.vel_x = -paddle_velocity;
            }
            else
            {
                p.vel_x = 0.0f;
            }
        }
        else if (keystates[SDL_SCANCODE_D]) // back
        {
            // RIGHT
            if (p.x < window_width - paddle_width - 1.0f)
            {
                p.vel_x = paddle_velocity;
            }
            else
            {
                p.vel_x = 0.0f;
            }
        }
        else
        {
            p.vel_x = 0.0f;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        BALL_PADDLE_collision(&b, &p);

        BALL_Move(&b);
        BALL_Draw(&b);

        PADDLE_Move(&p);
        PADDLE_Draw(&p);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}