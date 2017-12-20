//
//  helper_funcs.c
//  CPong
//
//  Created by 神崎航 on 2017/12/18.
//  Copyright © 2017年 神崎航. All rights reserved.
//

#include "helper_funcs.h"

float f_clamp(float value, float min, float max)
{
    if(value > max)
        return max;
    if(value < min)
        return min;
    
    return value;
}

int i_clamp(int value, int min, int max)
{
    if(value > max)
        return max;
    if(value < min)
        return min;
    
    return value;
}

float remap(float value, float oldmin, float oldmax, float newmin, float newmax)
{
    float real_range = oldmax - oldmin;
    float new_range = newmax - newmin;
    return (((value - oldmin) * new_range) / real_range) + newmin;
}

SDL_Point get_rect_center(SDL_Rect *rect)
{
    SDL_Point center = {rect->x + (rect->w / 2), rect->y + (rect->h / 2)};
    return center;
}

void set_rect_center(SDL_Rect *rect, float x, float y)
{
    rect->x = (x - (rect->w / 2));
    rect->y = (y - (rect->h / 2));
}
