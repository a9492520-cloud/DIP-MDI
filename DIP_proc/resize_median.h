#pragma once
#include "image_lib.h"
#include <algorithm>
#include <cmath>

void resizeNearest(int* f, int w, int h, int* g, int nw, int nh)
{
    for (int j = 0; j < nh; j++)
    {
        for (int i = 0; i < nw; i++)
        {
            int srcX = (int)(i * w / (double)nw);
            int srcY = (int)(j * h / (double)nh);
            if (srcX >= w) srcX = w - 1;
            if (srcY >= h) srcY = h - 1;
            g[j * nw + i] = f[srcY * w + srcX];
        }
    }
}

void resizeBilinear(int* f, int w, int h, int* g, int nw, int nh)
{
    for (int j = 0; j < nh; j++)
    {
        for (int i = 0; i < nw; i++)
        {
            double x = i * w / (double)nw;
            double y = j * h / (double)nh;
            int x0 = (int)x;
            int y0 = (int)y;
            if (x0 >= w - 1) x0 = w - 2;
            if (y0 >= h - 1) y0 = h - 2;
            int x1 = x0 + 1;
            int y1 = y0 + 1;
            double fx = x - x0;
            double fy = y - y0;

            int v00 = f[y0 * w + x0];
            int v10 = f[y0 * w + x1];
            int v01 = f[y1 * w + x0];
            int v11 = f[y1 * w + x1];

            double top = v00 + (v10 - v00) * fx;
            double bot = v01 + (v11 - v01) * fx;
            g[j * nw + i] = _clip_img(top + (bot - top) * fy);
        }
    }
}

void resizeBicubic(int* f, int w, int h, int* g, int nw, int nh)
{
    auto cubic = [](double t) -> double {
        t = fabs(t);
        if (t <= 1.0) return (1.5 * t - 2.5) * t * t + 1.0;
        if (t < 2.0) return (( -0.5 * t + 2.5) * t - 4.0) * t + 2.0;
        return 0.0;
    };

    for (int j = 0; j < nh; j++)
    {
        for (int i = 0; i < nw; i++)
        {
            double x = i * w / (double)nw;
            double y = j * h / (double)nh;
            double sum = 0.0, wsum = 0.0;

            for (int dy = -1; dy <= 2; dy++)
            {
                for (int dx = -1; dx <= 2; dx++)
                {
                    int px = (int)x + dx;
                    int py = (int)y + dy;
                    if (px < 0) px = 0;
                    if (px >= w) px = w - 1;
                    if (py < 0) py = 0;
                    if (py >= h) py = h - 1;
                    double c = cubic(px - x) * cubic(py - y);
                    sum += f[py * w + px] * c;
                    wsum += c;
                }
            }
            g[j * nw + i] = (wsum != 0) ? _clip_img(sum / wsum) : 0;
        }
    }
}
