#pragma once
#include "image_lib.h"

void rgbToGray(int* f, int w, int h, int* g)
{
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            int idx = (j * w + i) * 3;
            int b = f[idx + 0];
            int gv = f[idx + 1];
            int r = f[idx + 2];
            int gray = (int)(r * 0.299 + gv * 0.587 + b * 0.114);
            if (gray > 255) gray = 255;
            g[j * w + i] = gray;
        }
    }
}

void bitPlaneSlice(int* f, int w, int h, int* g, int bit)
{
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            int gray = f[j * w + i];
            int bitVal = (gray >> bit) & 1;
            g[j * w + i] = bitVal == 1 ? 255 : 0;
        }
    }
}
