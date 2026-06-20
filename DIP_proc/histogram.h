#pragma once
#include "image_lib.h"

void computeHistogram(int* f, int w, int h, int* hist)
{
    for (int i = 0; i < 256; i++) hist[i] = 0;
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            hist[f[j * w + i]]++;
        }
    }
}

void histogramEqualization(int* f, int w, int h, int* g)
{
    int hist[256] = { 0 };
    int cdf[256];
    computeHistogram(f, w, h, hist);

    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++)
        cdf[i] = cdf[i - 1] + hist[i];

    int total = w * h;
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            int val = f[j * w + i];
            g[j * w + i] = _clip_img(cdf[val] * 255 / total);
        }
    }
}

void histogramTransformation(int* f, int w, int h, int* g, int a, int b, int c, int d)
{
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            int val = f[j * w + i];
            if (val < a)
                g[j * w + i] = c;
            else if (val > b)
                g[j * w + i] = d;
            else
                g[j * w + i] = _clip_img((val - a) * (d - c) / (b - a) + c);
        }
    }
}
