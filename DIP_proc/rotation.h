#pragma once
#include "image_lib.h"
#include <cmath>

void rotateNearest(int* f, int w, int h, int* g, int gw, int gh, double angleDeg)
{
    double rad = angleDeg * 3.14159265 / 180.0;
    double cosA = cos(rad);
    double sinA = sin(rad);
    double cx = w / 2.0;
    double cy = h / 2.0;
    double gcx = gw / 2.0;
    double gcy = gh / 2.0;

    for (int j = 0; j < gh; j++)
    {
        for (int i = 0; i < gw; i++)
        {
            double rx = (i - gcx) * cosA - (j - gcy) * sinA + cx;
            double ry = (i - gcx) * sinA + (j - gcy) * cosA + cy;
            int sx = (int)(rx + 0.5);
            int sy = (int)(ry + 0.5);
            if (sx >= 0 && sx < w && sy >= 0 && sy < h)
                g[j * gw + i] = f[sy * w + sx];
            else
                g[j * gw + i] = 0;
        }
    }
}

void rotateBilinear(int* f, int w, int h, int* g, int gw, int gh, double angleDeg)
{
    double rad = angleDeg * 3.14159265 / 180.0;
    double cosA = cos(rad);
    double sinA = sin(rad);
    double cx = w / 2.0;
    double cy = h / 2.0;
    double gcx = gw / 2.0;
    double gcy = gh / 2.0;

    for (int j = 0; j < gh; j++)
    {
        for (int i = 0; i < gw; i++)
        {
            double rx = (i - gcx) * cosA - (j - gcy) * sinA + cx;
            double ry = (i - gcx) * sinA + (j - gcy) * cosA + cy;
            int x0 = (int)rx;
            int y0 = (int)ry;
            if (x0 < 0 || x0 >= w - 1 || y0 < 0 || y0 >= h - 1)
            {
                g[j * gw + i] = 0;
                continue;
            }
            double fx = rx - x0;
            double fy = ry - y0;

            int v00 = f[y0 * w + x0];
            int v10 = f[y0 * w + x0 + 1];
            int v01 = f[(y0 + 1) * w + x0];
            int v11 = f[(y0 + 1) * w + x0 + 1];

            double top = v00 + (v10 - v00) * fx;
            double bot = v01 + (v11 - v01) * fx;
            g[j * gw + i] = _clip_img(top + (bot - top) * fy);
        }
    }
}

void rotateAutoSize(int* f, int w, int h, int*& g, int& gw, int& gh, double angleDeg)
{
    double rad = angleDeg * 3.14159265 / 180.0;
    double cosA = fabs(cos(rad));
    double sinA = fabs(sin(rad));
    gw = (int)(w * cosA + h * sinA);
    gh = (int)(w * sinA + h * cosA);
    g = new int[gw * gh];
    rotateBilinear(f, w, h, g, gw, gh, angleDeg);
}
