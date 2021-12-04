#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "SDL2/SDL.h"

const int window_width = 800;
const int window_height = 600;

// BALL VALUES
const float ball_radius = 10.0f;
const float ball_velocity = 3.0f;

// PADDLE VALUES
const float paddle_width = 60.0f;
const float paddle_height = 20.0f;
const float paddle_velocity = 5.0f;

// BLOCKS
const float block_width = 60.0f;
const float block_height = 20.0f;
const int num_block_x = 11;
const int num_block_y = 4;
const int block_offset = window_width / num_block_x - (block_width);

#define NUM_BLOCKS (num_block_x * num_block_y)

static SDL_Window *window;
static SDL_Renderer *renderer;

const SDL_Colour RED = {255, 000, 000, 255}; // r g b a
const SDL_Colour GREEN = {000, 255, 000, 255};
const SDL_Colour BLUE = {000, 000, 255, 255};

#define PRINT_REC(REC) printf("%s {%d, %d, %d, %d}\n", #REC, (REC).x, (REC).y, (REC).w, (REC).h);

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
    SDL_Rect rec;
    float vel_x;
    float vel_y;
    SDL_Colour colour;
};

struct Block
{
    SDL_Rect rec;
    bool alive;
    SDL_Colour colour;
};

void PADDLE_Draw(struct Paddle *paddle)
{
    if (SDL_SetRenderDrawColor(renderer, paddle->colour.r, paddle->colour.g, paddle->colour.b, paddle->colour.a) ||
        SDL_RenderFillRect(renderer, &paddle->rec))
    {
        fprintf(stderr, "[SDL_SetRenderDrawColor/ SDL_RenderDrawRect] Failed to draw rectangle: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

void PADDLE_Move(struct Paddle *paddle)
{
    paddle->rec.x += paddle->vel_x;
}

bool REC_REC_Collision(const SDL_Rect *rect1, const SDL_Rect *rect2)
{
    return (bool)SDL_HasIntersection(rect1, rect2);
}

bool BALL_REC_Collision(struct Ball *ball, const SDL_Rect *rec)
{
    const float ball_left = ball->x - ball->radius;
    const float ball_right = ball->x + ball->radius;
    const float ball_top = ball->y - ball->radius;
    const float ball_bottom = ball->y + ball->radius;

    const float paddle_left = rec->x;
    const float paddle_top = rec->y;
    const float paddle_right = rec->x + rec->w;
    const float paddle_bottom = rec->y + rec->h;

    if (!(ball_right >= paddle_left && ball_left <= paddle_right && ball_bottom >= paddle_top && ball_top <= paddle_bottom))
        return false;

    ball->vel_y = -ball_velocity;

    if (ball->x < rec->x)
    {
        ball->vel_x = -ball_velocity;
    }
    else
    {
        ball->vel_x = ball_velocity;
    }
    return true;
}

void BLOCK_Draw(struct Block *blocks)
{
    for (int y = 0; y < num_block_y; y++)
    {
        for (int x = 0; x < num_block_x; x++)
        {
            const int index = y * num_block_x + x;

            if (blocks[index].alive)
            {
                SDL_SetRenderDrawColor(renderer, blocks[index].colour.r, blocks[index].colour.g, blocks[index].colour.b, blocks[index].colour.a);
                SDL_RenderFillRect(renderer, &blocks[index].rec);
            }
        }
    }
}

void BLOCK_Collision(struct Ball *ball, struct Block *blocks)
{
    for (int y = 0; y < num_block_y; y++)
    {
        for (int x = 0; x < num_block_x; x++)
        {
            const int index = y * num_block_x + x;

            if (blocks[index].alive)
            {
                if (BALL_REC_Collision(ball, &blocks[index].rec))
                {
                    blocks[index].alive = false;
                    ball->vel_x *= -1;
                    ball->vel_y *= -1;
                }
            }
        }
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
    p.rec.x = window_width / 2;
    p.rec.y = window_height - 50.0f;
    p.rec.w = paddle_width;
    p.rec.h = paddle_height;
    p.colour = BLUE;
    p.vel_x = 0.0f;
    p.vel_y = 0.0f;

    struct Block blocks[num_block_x * num_block_y];

    for (int y = 0; y < num_block_y; y++)
    {
        for (int x = 0; x < num_block_x; x++)
        {
            const int index = y * num_block_x + x;

            blocks[index].rec.x = ((x) * (block_width + block_offset)) + block_offset;
            blocks[index].rec.y = (y + 2) * (block_height + 6);
            blocks[index].rec.w = block_width;
            blocks[index].rec.h = block_height;

            blocks[index].colour = BLUE;
            blocks[index].alive = true;
        }
    }

    bool quit = false;
    SDL_Event sdlEvent;
    while (!quit)
    {
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
            if (p.rec.x > 1.0f)
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
            if (p.rec.x < window_width - paddle_width - 1.0f)
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

        BALL_REC_Collision(&b, &p.rec);
        BLOCK_Collision(&b, blocks);

        BALL_Move(&b);
        BALL_Draw(&b);

        PADDLE_Move(&p);
        PADDLE_Draw(&p);

        BLOCK_Draw(blocks);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}