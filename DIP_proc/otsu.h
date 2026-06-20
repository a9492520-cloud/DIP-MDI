#pragma once
#include "image_lib.h"

void otsuThreshold(int* f, int w, int h, int& threshold)
{
    int hist[256] = { 0 };
    int total = w * h;
    for (int i = 0; i < total; i++)
        hist[f[i]]++;

    double sum = 0;
    for (int i = 0; i < 256; i++)
        sum += i * hist[i];

    double sumB = 0;
    int wB = 0;
    int wF = 0;
    double maxVar = 0;
    threshold = 0;

    for (int t = 0; t < 256; t++)
    {
        wB += hist[t];
        if (wB == 0) continue;
        wF = total - wB;
        if (wF == 0) break;

        sumB += t * hist[t];
        double mB = sumB / wB;
        double mF = (sum - sumB) / wF;
        double var = (double)wB * wF * (mB - mF) * (mB - mF);

        if (var > maxVar)
        {
            maxVar = var;
            threshold = t;
        }
    }
}

void otsuSegmentation(int* f, int w, int h, int* g)
{
    int threshold;
    otsuThreshold(f, w, h, threshold);
    for (int i = 0; i < w * h; i++)
    {
        g[i] = (f[i] >= threshold) ? 255 : 0;
    }
}
