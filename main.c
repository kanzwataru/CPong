//
//  main.c
//  CPong
//
//  Created by 神崎航 on 2017/12/17.
//  Copyright © 2017年 神崎航. All rights reserved.
//

#include <stdio.h>
#include <SDL2/sdl.h>
#include "helper_funcs.h"

#define TRUE SDL_TRUE
#define FALSE SDL_FALSE
typedef SDL_bool BOOL;

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FIXED_UPDATE_FPS 120
#define MAX_FPS 60

#define GATE_W 3
#define GATE_H 5
#define SCORE_W 10
#define SCORE_H 30
#define PADDLE_W 30
#define PADDLE_H 120
#define PADDLE_GAP 320
#define BALL_SIZE 20

#define PADDLE_ACCEL 0.1
#define PADDLE_DECEL 0.01
#define PADDLE_SPEED 10
#define BALL_SPEED 3
#define SMACK_FORCE_MULT 3
#define AI_PREDICT_STEP 12

const float TOP = 0;
const float BTM = SCREEN_HEIGHT - PADDLE_H;
const int SCREEN_HALF = SCREEN_WIDTH / 2;
const int PLAYER_SCORE_X = 10;
const int AI_SCORE_X = SCREEN_WIDTH - (2 * SCORE_W);
const int SCORE_Y = 10;
const int AI_MAX_REACT = SCREEN_WIDTH * 0.95;
const int AI_MIN_REACT = SCREEN_WIDTH * 0.5;
const int AI_PADDLE_UPPER_BOUND = PADDLE_H * 0.2;
const int AI_PADDLE_LOWER_BOUND = PADDLE_H * 0.7;
const unsigned int UPDATE_STEP_SIZE = 1000/FIXED_UPDATE_FPS;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

typedef struct _aiattrs {
    int frame_count;
    SDL_Point prediction;
} AIAttrs;

typedef struct _paddle {
    SDL_Rect rect;
    float speed;
    int direction;
    int score;
} Paddle;

typedef struct _ball {
    SDL_Rect rect;
    float direction_x;
    float direction_y;
    BOOL start;
} Ball;

Paddle player;
Paddle ai;
AIAttrs aiattrs;
Ball ball;
BOOL game_paused = FALSE;

SDL_Rect debug_prediction;

float calc_speed(float current_speed, int direction)
{
    return f_clamp(current_speed + (PADDLE_ACCEL * abs(direction)
                                 - (PADDLE_DECEL * !abs(direction))), 0.0, 1.0);
}

void calc_paddle_movement(Paddle *paddle)
{
    paddle->speed = calc_speed(paddle->speed, paddle->direction);
    paddle->rect.y = f_clamp(paddle->rect.y + paddle->direction * paddle->speed * PADDLE_SPEED, TOP, BTM);
}

float away_from_paddle(Paddle *paddle)
{
    if(paddle->rect.x >= SCREEN_HALF)
        return -1;
    else
        return 1;
}

void calc_ball_paddle_collision(Paddle *paddle)
{
    BOOL bounce = FALSE;
    float percent = remap(ball.rect.y + (ball.rect.h / 2),
                    paddle->rect.y,
                    paddle->rect.y + paddle->rect.h,
                    -1.0, 1.0);

    while(SDL_HasIntersection(&ball.rect, &paddle->rect)) {
        bounce = TRUE;
        
        ball.rect.x += away_from_paddle(paddle);
    }
    if(bounce) {
        if(fabsf(percent) > 1.0) {
            ball.direction_x = -ball.direction_x;
            ball.direction_y = -ball.direction_y;
        }
        else {
            ball.direction_x = -ball.direction_x;
            ball.direction_y = -percent + (paddle->speed * paddle->direction * SMACK_FORCE_MULT);
        }
    }
    
    
}

void calc_ball_collision()
{
    if(ball.rect.x + ball.rect.w <= 0) {
        ball.start = TRUE;
        ai.score++;
    }
    else if(ball.rect.x >= SCREEN_WIDTH) {
        ball.start = TRUE;
        player.score++;
    }
    else if(ball.rect.y <= 0
            || ball.rect.y + ball.rect.h >= SCREEN_HEIGHT)
    {
        ball.direction_y = -ball.direction_y;
    }
    
    calc_ball_paddle_collision(&ai);
    calc_ball_paddle_collision(&player);
}

void calc_ball_movement()
{
    if(ball.start) {
        ball.direction_x = 1;
        ball.direction_y = 1;
        ball.rect.x = player.rect.x + player.rect.w;
        ball.rect.y = player.rect.y + (player.rect.h / 2) - (ball.rect.h / 2);
        return;
    }

    ball.rect.x += (BALL_SPEED * ball.direction_x);
    ball.rect.y += (BALL_SPEED * ball.direction_y);
}

void ai_predict_ball()
{
    if(aiattrs.frame_count > AI_PREDICT_STEP) {
        aiattrs.frame_count = 0;
        
        aiattrs.prediction.x = ai.rect.x;
        aiattrs.prediction.y = ball.rect.y;
    }
    aiattrs.frame_count++;
}

int ai_decision()
{
    SDL_Point ball_center = get_rect_center(&ball.rect);
    
    if(ball_center.x >= AI_MIN_REACT && ball_center.x <= AI_MAX_REACT && ball.direction_x > 0) {
        ai_predict_ball();
        if(aiattrs.prediction.y <= ai.rect.y + AI_PADDLE_UPPER_BOUND)
            return -1;
        else if(aiattrs.prediction.y >= ai.rect.y + AI_PADDLE_LOWER_BOUND)
            return 1;
        else
            return 0;
    }
    return 0;
}

BOOL handle_input()
{
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_QUIT) {
            return FALSE;
        }
        if(event.type == SDL_KEYDOWN) {
            if(event.key.keysym.sym == SDLK_UP)
                player.direction = -1;
            else if(event.key.keysym.sym == SDLK_DOWN)
                player.direction = 1;
            else if(event.key.keysym.sym == SDLK_SPACE)
                ball.start = FALSE;
            else if(event.key.keysym.sym == SDLK_ESCAPE)
                return FALSE;
            else if(event.key.keysym.sym == SDLK_p)
                game_paused = !game_paused;
        }
        else if(event.type == SDL_KEYUP) {
            if(event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_DOWN)
                player.direction = 0;
        }
    }
    return TRUE;
}

void update()
{
    if(game_paused)
        return;
    
    /* Game logic */
    ai.direction = ai_decision();
    calc_paddle_movement(&ai);
    calc_paddle_movement(&player);
    
    calc_ball_collision();
    calc_ball_movement();
}

void debug_info()
{
     SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
     
     SDL_Rect prediction_rect;
     prediction_rect.x = aiattrs.prediction.x;
     prediction_rect.y = aiattrs.prediction.y;
     prediction_rect.h = ball.rect.h;
     prediction_rect.w = ball.rect.w;
     SDL_RenderDrawRect(renderer, &prediction_rect);
}

void render()
{
    if(game_paused)
        SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
    else
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if(game_paused)
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    else
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &player.rect);
    SDL_RenderFillRect(renderer, &ai.rect);
    SDL_RenderFillRect(renderer, &ball.rect);
    
    /* draw scores */
    SDL_Rect rect;
    rect.w = SCORE_W;
    rect.h = SCORE_H;
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    for(int i = 0; i < player.score; i++) {
        rect.x = PLAYER_SCORE_X + (i * (rect.w * 2));
        rect.y = SCORE_Y;
        SDL_RenderFillRect(renderer, &rect);
    }
    for(int i = 0; i < ai.score; i++) {
        rect.x = AI_SCORE_X - (i * (rect.w * 2));
        rect.y = SCORE_Y;
        SDL_RenderFillRect(renderer, &rect);
    }
    
    rect.h = GATE_H;
    rect.w = GATE_W;
    for(int i = 0; i < 60; i++) {
        rect.x = SCREEN_HALF;
        rect.y = i * (GATE_H * 2);
        SDL_RenderFillRect(renderer, &rect);
    }
    
    SDL_RenderPresent(renderer);
}

void game_loop()
{
    BOOL running = TRUE;
    unsigned int last_time = 0;
    unsigned int accumulator = 0;
    unsigned int current_time;
    
    while(running) {
        running = handle_input();
        
        current_time = SDL_GetTicks();
        unsigned int delta_time = current_time - last_time;
        last_time = current_time;
        
        /* Game Logic */
        accumulator += delta_time;
        while(accumulator > UPDATE_STEP_SIZE) {
            update();
            accumulator -= UPDATE_STEP_SIZE;
        }
        
        /* Render the stuff */
        render();
        
        /* Cap at fixed update */
        SDL_Delay(UPDATE_STEP_SIZE);
    }
}

void init_game()
{
    player.rect.w = PADDLE_W;
    player.rect.h = PADDLE_H;
    set_rect_center(&player.rect, SCREEN_HALF - PADDLE_GAP, (SCREEN_HEIGHT / 2));
    
    ai.rect.w = PADDLE_W;
    ai.rect.h = PADDLE_H;
    set_rect_center(&ai.rect, SCREEN_HALF + PADDLE_GAP, (SCREEN_HEIGHT / 2));
    
    ball.rect.h = BALL_SIZE;
    ball.rect.w = BALL_SIZE;
    ball.start = TRUE;
    ball.direction_x = 1;
    ball.direction_y = 1;
    
    aiattrs.frame_count = 0;
}

int main(int argc, const char * argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);
    
    window = SDL_CreateWindow("CPong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    init_game();
    game_loop();
}
