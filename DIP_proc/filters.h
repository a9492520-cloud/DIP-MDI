#pragma once
#include "image_lib.h"
#include <algorithm>
#include <cmath>

static void convolve(int* f, int w, int h, int* g, const float* kernel, int ksize)
{
    int half = ksize / 2;
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            float sum = 0.0f;
            for (int ky = -half; ky <= half; ky++)
            {
                for (int kx = -half; kx <= half; kx++)
                {
                    int px = i + kx;
                    int py = j + ky;
                    if (px >= 0 && px < w && py >= 0 && py < h)
                    {
                        sum += f[py * w + px] * kernel[(ky + half) * ksize + (kx + half)];
                    }
                }
            }
            g[j * w + i] = _clip_img(sum);
        }
    }
}

void meanFilter(int* f, int w, int h, int* g, int ksize)
{
    float* kernel = new float[ksize * ksize];
    float inv = 1.0f / (ksize * ksize);
    for (int i = 0; i < ksize * ksize; i++) kernel[i] = inv;
    convolve(f, w, h, g, kernel, ksize);
    delete[] kernel;
}

void gaussianFilter(int* f, int w, int h, int* g, int ksize, float sigma)
{
    float* kernel = new float[ksize * ksize];
    int half = ksize / 2;
    float sum = 0.0f;
    for (int ky = -half; ky <= half; ky++)
    {
        for (int kx = -half; kx <= half; kx++)
        {
            float v = expf(-(kx * kx + ky * ky) / (2.0f * sigma * sigma));
            kernel[(ky + half) * ksize + (kx + half)] = v;
            sum += v;
        }
    }
    for (int i = 0; i < ksize * ksize; i++) kernel[i] /= sum;
    convolve(f, w, h, g, kernel, ksize);
    delete[] kernel;
}

void medianFilter(int* f, int w, int h, int* g, int ksize)
{
    int half = ksize / 2;
    int* window = new int[ksize * ksize];
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            int idx = 0;
            for (int ky = -half; ky <= half; ky++)
            {
                for (int kx = -half; kx <= half; kx++)
                {
                    int px = i + kx;
                    int py = j + ky;
                    if (px >= 0 && px < w && py >= 0 && py < h)
                        window[idx++] = f[py * w + px];
                    else
                        window[idx++] = 0;
                }
            }
            std::sort(window, window + idx);
            g[j * w + i] = window[idx / 2];
        }
    }
    delete[] window;
}

void sobelEdge(int* f, int w, int h, int* g)
{
    const float sobelX[9] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
    const float sobelY[9] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };
    int* gx = new int[w * h];
    int* gy = new int[w * h];
    convolve(f, w, h, gx, sobelX, 3);
    convolve(f, w, h, gy, sobelY, 3);
    for (int i = 0; i < w * h; i++)
    {
        g[i] = _clip_img((int)sqrt((double)gx[i] * gx[i] + (double)gy[i] * gy[i]));
    }
    delete[] gx;
    delete[] gy;
}

void prewittEdge(int* f, int w, int h, int* g)
{
    const float prewittX[9] = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
    const float prewittY[9] = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };
    int* gx = new int[w * h];
    int* gy = new int[w * h];
    convolve(f, w, h, gx, prewittX, 3);
    convolve(f, w, h, gy, prewittY, 3);
    for (int i = 0; i < w * h; i++)
    {
        g[i] = _clip_img((int)sqrt((double)gx[i] * gx[i] + (double)gy[i] * gy[i]));
    }
    delete[] gx;
    delete[] gy;
}

void laplacianFilter(int* f, int w, int h, int* g)
{
    const float laplacian[9] = { 0, -1, 0, -1, 4, -1, 0, -1, 0 };
    convolve(f, w, h, g, laplacian, 3);
}

void logFilter(int* f, int w, int h, int* g, int ksize, float sigma)
{
    int half = ksize / 2;
    float* kernel = new float[ksize * ksize];
    float sum = 0.0f;
    for (int ky = -half; ky <= half; ky++)
    {
        for (int kx = -half; kx <= half; kx++)
        {
            float r2 = (float)(kx * kx + ky * ky);
            float s2 = sigma * sigma;
            float v = -(1.0f - r2 / s2) * expf(-r2 / (2.0f * s2)) / (3.14159265f * s2);
            kernel[(ky + half) * ksize + (kx + half)] = v;
            sum += v;
        }
    }
    for (int i = 0; i < ksize * ksize; i++) kernel[i] /= sum;
    convolve(f, w, h, g, kernel, ksize);
    delete[] kernel;
}

void highPassFilter(int* f, int w, int h, int* g)
{
    const float hp[9] = { -1, -1, -1, -1, 8, -1, -1, -1, -1 };
    convolve(f, w, h, g, hp, 3);
}

void sharpenFilter(int* f, int w, int h, int* g, float amount)
{
    int* blurred = new int[w * h];
    gaussianFilter(f, w, h, blurred, 5, 1.0f);
    for (int i = 0; i < w * h; i++)
    {
        g[i] = _clip_img(f[i] + (int)((f[i] - blurred[i]) * amount));
    }
    delete[] blurred;
}

void robertsEdge(int* f, int w, int h, int* g)
{
    for (int j = 0; j < h - 1; j++)
    {
        for (int i = 0; i < w - 1; i++)
        {
            int p1 = f[j * w + i];
            int p2 = f[j * w + i + 1];
            int p3 = f[(j + 1) * w + i];
            int p4 = f[(j + 1) * w + i + 1];
            int gx = abs(p1 - p4);
            int gy = abs(p2 - p3);
            g[j * w + i] = _clip_img(gx + gy);
        }
    }
    for (int i = 0; i < w; i++) g[(h - 1) * w + i] = 0;
    for (int j = 0; j < h; j++) g[j * w + (w - 1)] = 0;
}
