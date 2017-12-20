//
//  helper_funcs.h
//  CPong
//
//  Created by 神崎航 on 2017/12/18.
//  Copyright © 2017年 神崎航. All rights reserved.
//

#ifndef helper_funcs_h
#define helper_funcs_h

#include <stdio.h>
#include <SDL2/sdl.h>

float f_clamp(float value, float min, float max);
int i_clamp(int value, int min, int max);
float remap(float value, float oldmin, float oldmax, float newmin, float newmax);
SDL_Point get_rect_center(SDL_Rect *rect);
void set_rect_center(SDL_Rect *rect, float x, float y);

#endif /* helper_funcs_h */
