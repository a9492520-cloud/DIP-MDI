#pragma once
#include "image_lib.h"

void brightness(int* f, int w, int h, int ch, int* g, int delta)
{
    int n = w * h * ch;
    for (int i = 0; i < n; i++)
    {
        g[i] = _clip_img(f[i] + delta);
    }
}

void contrast(int* f, int w, int h, int ch, int* g, double factor)
{
    int n = w * h * ch;
    for (int i = 0; i < n; i++)
    {
        g[i] = _clip_img((f[i] - 128) * factor + 128);
    }
}
