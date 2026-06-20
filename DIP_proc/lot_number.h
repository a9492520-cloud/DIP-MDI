#pragma once
#include "image_lib.h"
#include <algorithm>
#include <cmath>
#include <cstring>

//=============================================================================
// Lot Number (Metal Character) Recognition
// Fully self-contained, NO dependency on filters.h / otsu.h / resize_median.h
// to avoid the round/clip conflicts in image_lib.h + <cmath>
//=============================================================================

//=============================================================================
// Local helpers (avoiding round/clip conflicts with image_lib.h)
//=============================================================================
static int _clip_pixel(double s)
{
    if (s > 255.0) s = 255.0;
    if (s < 0.0) s = 0.0;
    return (int)(s + 0.5);
}

static int _iround(double s)
{
    return (s >= 0.0) ? (int)(s + 0.5) : (int)(s - 0.5);
}

//=============================================================================
// Step 1: Extract ROI
//=============================================================================
static void getLotROI(int* f, int w, int h, int*& roi, int& roiW, int& roiH, int& roiX, int& roiY)
{
    roiX = (int)(w * 0.20);
    roiY = (int)(h * 0.40);
    roiW = (int)(w * 0.65);
    roiH = (int)(h * 0.40);

    if (roiX + roiW > w) roiW = w - roiX;
    if (roiY + roiH > h) roiH = h - roiY;
    if (roiW < 1) roiW = 1;
    if (roiH < 1) roiH = 1;

    roi = new int[roiW * roiH];
    for (int j = 0; j < roiH; j++)
        for (int i = 0; i < roiW; i++)
            roi[j * roiW + i] = f[(roiY + j) * w + (roiX + i)];
}

//=============================================================================
// Step 2: Bilinear resize (equivalent to cv2.INTER_LINEAR)
//=============================================================================
static void _resizeBilinear(int* f, int w, int h, int* g, int nw, int nh)
{
    for (int j = 0; j < nh; j++)
        for (int i = 0; i < nw; i++)
        {
            double sx = i * w / (double)nw;
            double sy = j * h / (double)nh;
            int ix = (int)sx;
            int iy = (int)sy;
            double fx = sx - ix;
            double fy = sy - iy;
            int ix1 = (ix + 1 < w) ? ix + 1 : ix;
            int iy1 = (iy + 1 < h) ? iy + 1 : iy;
            double v = (1.0 - fx) * (1.0 - fy) * f[iy * w + ix]
                     + fx * (1.0 - fy) * f[iy * w + ix1]
                     + (1.0 - fx) * fy * f[iy1 * w + ix]
                     + fx * fy * f[iy1 * w + ix1];
            g[j * nw + i] = _clip_pixel(v);
        }
}

//=============================================================================
// Step 3: Adaptive threshold with Gaussian weights
// Equivalent to cv2.adaptiveThreshold + ADAPTIVE_THRESH_GAUSSIAN_C
// Output: 0 = character (dark), 255 = background (bright)
//=============================================================================
static void _adaptiveThreshold(int* f, int w, int h, int* g, int blockSize, int C)
{
    int half = blockSize / 2;
    double sigma = blockSize / 6.0;
    double* weights = new double[blockSize * blockSize];
    double totalWeight = 0;
    for (int dy = -half; dy <= half; dy++)
        for (int dx = -half; dx <= half; dx++)
        {
            double wgt = exp(-(dx * dx + dy * dy) / (2.0 * sigma * sigma));
            weights[(dy + half) * blockSize + (dx + half)] = wgt;
            totalWeight += wgt;
        }

    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
        {
            double wsum = 0, wAcc = 0;
            for (int dy = -half; dy <= half; dy++)
                for (int dx = -half; dx <= half; dx++)
                {
                    int px = i + dx, py = j + dy;
                    if (px >= 0 && px < w && py >= 0 && py < h)
                    {
                        double wgt = weights[(dy + half) * blockSize + (dx + half)];
                        wsum += f[py * w + px] * wgt;
                        wAcc += wgt;
                    }
                }
            double mean = wsum / wAcc;
            g[j * w + i] = (f[j * w + i] > mean - C) ? 255 : 0;
        }
    delete[] weights;
}

//=============================================================================
// Step 4: Morphological close (kernel=ksize x ksize) to bridge dot-matrix gaps
// kernel = 2*radius+1, bridges gaps up to 2*radius pixels apart
// Input/Output: 0 = character, 255 = background
//=============================================================================
static void _morphClose(int* f, int w, int h, int* g, int radius)
{
    int* tmp = new int[w * h];
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
        {
            int mn = 255;
            for (int dy = -radius; dy <= radius; dy++)
                for (int dx = -radius; dx <= radius; dx++)
                {
                    int px = i + dx, py = j + dy;
                    if (px >= 0 && px < w && py >= 0 && py < h)
                        if (f[py * w + px] < mn) mn = f[py * w + px];
                }
            tmp[j * w + i] = mn;
        }
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
        {
            int mx = 0;
            for (int dy = -radius; dy <= radius; dy++)
                for (int dx = -radius; dx <= radius; dx++)
                {
                    int px = i + dx, py = j + dy;
                    if (px >= 0 && px < w && py >= 0 && py < h)
                        if (tmp[py * w + px] > mx) mx = tmp[py * w + px];
                }
            g[j * w + i] = mx;
        }
    delete[] tmp;
}

//=============================================================================
// Step 5: Full preprocessing pipeline (KEPT for backward compatibility)
//=============================================================================
static void preprocessForOCR(int* f, int w, int h, int*& out, int& outW, int& outH,
    int blockSize = 15, int C = 2, int morphRadius = 8)
{
    int rw = w * 3;
    int rh = h * 3;

    int* resized = new int[rw * rh];
    _resizeBilinear(f, w, h, resized, rw, rh);

    int* binary = new int[rw * rh];
    _adaptiveThreshold(resized, rw, rh, binary, blockSize, C);

    int* closed = new int[rw * rh];
    _morphClose(binary, rw, rh, closed, morphRadius);

    out = new int[rw * rh];
    for (int i = 0; i < rw * rh; i++)
        out[i] = (closed[i] == 0) ? 255 : 0;
    outW = rw;
    outH = rh;

    delete[] resized;
    delete[] binary;
    delete[] closed;
}

//=============================================================================
// Step 6: Character segmentation via vertical projection (binary input)
//=============================================================================
static int segmentCharacters(int* f, int w, int h, int* regions, int maxRegions)
{
    int* vproj = new int[w]();
    for (int i = 0; i < w; i++)
    {
        int c = 0;
        for (int j = 0; j < h; j++)
            if (f[j * w + i] == 255) c++;
        vproj[i] = c;
    }

    int* mproj = new int[w]();
    int filterR = 1;
    for (int i = 0; i < w; i++)
    {
        int mv = 0;
        for (int k = -filterR; k <= filterR; k++)
        {
            int idx = i + k;
            if (idx >= 0 && idx < w && vproj[idx] > mv)
                mv = vproj[idx];
        }
        mproj[i] = mv;
    }

    int charThresh = (h > 50) ? std::max(5, h / 10) : 3;
    int gapThresh = (h > 50) ? std::max(2, h / 30) : 1;

    int count = 0, inChar = 0, startX = 0;
    for (int i = 0; i < w; i++)
    {
        if (mproj[i] > charThresh && !inChar) { inChar = 1; startX = i; }
        else if ((mproj[i] <= gapThresh || i == w - 1) && inChar)
        {
            inChar = 0;
            int endX = (mproj[i] <= gapThresh) ? i - 1 : i;
            if (endX - startX >= 3 && count < maxRegions)
            {
                int minY = h, maxY = 0;
                for (int j = startX; j <= endX; j++)
                    for (int k = 0; k < h; k++)
                        if (f[k * w + j] == 255)
                        {
                            if (k < minY) minY = k;
                            if (k > maxY) maxY = k;
                        }
                int segW = endX - startX + 1;
                int segH = (maxY - minY + 1 < 5) ? 5 : (maxY - minY + 1);
                if (segW < 4 || segH < 6 || segW * segH < 60)
                    continue;
                regions[count * 4 + 0] = startX;
                regions[count * 4 + 1] = minY;
                regions[count * 4 + 2] = segW;
                regions[count * 4 + 3] = segH;
                count++;
            }
        }
    }
    delete[] mproj;
    delete[] vproj;
    return count;
}

//=============================================================================
// Templates (12x16 bitmaps for 0-9 and A-Z)
// Packed: 2 bytes/row, 16 rows = 32 bytes per character
//=============================================================================
#define T_W 12
#define T_H 16

static const unsigned char dT[10][32] = {
    {0x0F,0x00,0x1F,0x80,0x30,0xC0,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x30,0xC0,0x1F,0x80,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x0C,0x00,0x1C,0x00,0x3C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x3F,0x00,0x3F,0x00,0x00,0x00,0x00,0x00},
    {0x1F,0x00,0x3F,0x80,0x60,0xC0,0x00,0xC0,0x00,0xC0,0x01,0x80,0x03,0x00,0x06,0x00,0x0C,0x00,0x18,0x00,0x30,0x00,0x60,0x00,0x7F,0xC0,0x7F,0xC0,0x00,0x00,0x00,0x00},
    {0x1F,0x00,0x3F,0x80,0x60,0xC0,0x00,0xC0,0x00,0xC0,0x01,0x80,0x0F,0x00,0x0F,0x00,0x01,0x80,0x00,0xC0,0x00,0xC0,0x60,0xC0,0x3F,0x80,0x1F,0x00,0x00,0x00,0x00,0x00},
    {0x03,0x00,0x07,0x00,0x0F,0x00,0x1B,0x00,0x33,0x00,0x63,0x00,0xC3,0x00,0xFF,0xC0,0xFF,0xC0,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0x00,0x00,0x00,0x00},
    {0x7F,0xC0,0x7F,0xC0,0x60,0x00,0x60,0x00,0x7F,0x00,0x7F,0x80,0x01,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x60,0xC0,0x3F,0x80,0x1F,0x00,0x00,0x00,0x00,0x00},
    {0x0F,0x00,0x1F,0x80,0x38,0x00,0x60,0x00,0x60,0x00,0x6F,0x00,0x7F,0x80,0x70,0xC0,0x60,0x60,0x60,0x60,0x60,0x60,0x30,0xC0,0x1F,0x80,0x0F,0x00,0x00,0x00,0x00,0x00},
    {0x7F,0xC0,0x7F,0xC0,0x00,0xC0,0x01,0x80,0x03,0x00,0x06,0x00,0x0C,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x00,0x00,0x00,0x00},
    {0x1F,0x00,0x3F,0x80,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x30,0x80,0x1F,0x00,0x1F,0x00,0x30,0x80,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x3F,0x80,0x1F,0x00,0x00,0x00,0x00,0x00},
    {0x1F,0x00,0x3F,0x80,0x60,0xC0,0x60,0x60,0x60,0x60,0x60,0x60,0x30,0xE0,0x1F,0xE0,0x07,0x60,0x00,0x60,0x00,0x60,0x00,0xC0,0x3F,0x80,0x1F,0x00,0x00,0x00,0x00,0x00}
};

static const unsigned char lT[26][32] = {
    {0x0C,0x00,0x1E,0x00,0x33,0x00,0x33,0x00,0x61,0x80,0x61,0x80,0x61,0x80,0x7F,0x80,0xFF,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0x00,0x00,0x00,0x00},
    {0xFE,0x00,0xFF,0x00,0xC3,0x00,0xC3,0x00,0xC3,0x00,0xC6,0x00,0xFE,0x00,0xFE,0x00,0xC3,0x00,0xC1,0x80,0xC1,0x80,0xC1,0x80,0xFF,0x00,0xFE,0x00,0x00,0x00,0x00,0x00},
    {0x0F,0x00,0x1F,0x80,0x38,0xC0,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x38,0xC0,0x1F,0x80,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0xFC,0x00,0xFE,0x00,0xC7,0x00,0xC3,0x80,0xC1,0x80,0xC1,0x80,0xC1,0x80,0xC1,0x80,0xC1,0x80,0xC1,0x80,0xC3,0x80,0xC7,0x00,0xFE,0x00,0xFC,0x00,0x00,0x00,0x00,0x00},
    {0xFF,0x80,0xFF,0x80,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xFE,0x00,0xFE,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xFF,0x80,0xFF,0x80,0x00,0x00,0x00,0x00},
    {0xFF,0x80,0xFF,0x80,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xFE,0x00,0xFE,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0x00,0x00,0x00,0x00},
    {0x0F,0x00,0x1F,0x80,0x38,0xC0,0x60,0x00,0x60,0x00,0x60,0x00,0x67,0xC0,0x67,0xC0,0x60,0xC0,0x60,0xC0,0x38,0xC0,0x1F,0xC0,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0xC1,0x80,0xC1,0x80,0xC1,0x80,0xC1,0x80,0xC1,0x80,0xC1,0x80,0xFF,0x80,0xFF,0x80,0xC1,0x80,0xC1,0x80,0xC1,0x80,0xC1,0x80,0xC1,0x80,0xC1,0x80,0x00,0x00,0x00,0x00},
    {0x3F,0x00,0x3F,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x3F,0x00,0x3F,0x00,0x00,0x00,0x00,0x00},
    {0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0xC1,0x80,0xC1,0x80,0x63,0x00,0x3E,0x00,0x1C,0x00,0x00,0x00,0x00,0x00},
    {0xC1,0x80,0xC3,0x00,0xC6,0x00,0xCC,0x00,0xD8,0x00,0xF0,0x00,0xF0,0x00,0xD8,0x00,0xCC,0x00,0xC6,0x00,0xC3,0x00,0xC1,0x80,0xC1,0x80,0xC0,0x00,0x00,0x00,0x00,0x00},
    {0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xFF,0x80,0xFF,0x80,0x00,0x00,0x00,0x00},
    {0xE0,0xE0,0xF1,0xE0,0xF1,0xE0,0xDB,0x60,0xDB,0x60,0xCE,0x60,0xCE,0x60,0xC4,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x60,0x00,0x00,0x00,0x00},
    {0xE0,0x60,0xE0,0x60,0xF0,0x60,0xD8,0x60,0xD8,0x60,0xCC,0x60,0xC6,0x60,0xC6,0x60,0xC3,0x60,0xC1,0xE0,0xC1,0xE0,0xC0,0xE0,0xC0,0x60,0xC0,0x60,0x00,0x00,0x00,0x00},
    {0x1F,0x00,0x3F,0x80,0x60,0xC0,0x60,0xC0,0xC0,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x60,0x60,0xC0,0x60,0xC0,0x3F,0x80,0x1F,0x00,0x00,0x00,0x00,0x00},
    {0xFE,0x00,0xFF,0x00,0xC3,0x80,0xC1,0x80,0xC1,0x80,0xC1,0x80,0xC3,0x80,0xFF,0x00,0xFE,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0x00,0x00,0x00,0x00},
    {0x1F,0x00,0x3F,0x80,0x60,0xC0,0x60,0xC0,0xC0,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x60,0xCE,0x60,0x6E,0xC0,0x67,0xC0,0x3F,0x80,0x1D,0x00,0x00,0x00,0x00,0x00},
    {0xFE,0x00,0xFF,0x00,0xC3,0x80,0xC1,0x80,0xC1,0x80,0xC3,0x80,0xFF,0x00,0xFE,0x00,0xC6,0x00,0xC3,0x00,0xC1,0x80,0xC1,0x80,0xC0,0xC0,0xC0,0x60,0x00,0x00,0x00,0x00},
    {0x1F,0x00,0x3F,0x80,0x60,0xC0,0x60,0x00,0x60,0x00,0x3C,0x00,0x0F,0x00,0x03,0x80,0x00,0xC0,0x00,0xC0,0x60,0xC0,0x60,0xC0,0x3F,0x80,0x1F,0x00,0x00,0x00,0x00,0x00},
    {0xFF,0xC0,0xFF,0xC0,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x00,0x00,0x00,0x00},
    {0xC0,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x60,0x60,0xC0,0x60,0xC0,0x3F,0x80,0x1F,0x00,0x00,0x00,0x00,0x00},
    {0xC0,0x60,0xC0,0x60,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x31,0x80,0x31,0x80,0x31,0x80,0x1B,0x00,0x1B,0x00,0x1B,0x00,0x0E,0x00,0x0E,0x00,0x04,0x00,0x00,0x00,0x00,0x00},
    {0xC0,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x60,0xC4,0x60,0xCE,0x60,0xCE,0x60,0xDB,0x60,0xDB,0x60,0xF1,0xE0,0xF1,0xE0,0x60,0xC0,0x60,0xC0,0x00,0x00,0x00,0x00},
    {0xC0,0x60,0xC0,0x60,0x60,0xC0,0x71,0xC0,0x31,0x80,0x1B,0x00,0x0E,0x00,0x0E,0x00,0x1B,0x00,0x31,0x80,0x71,0xC0,0x60,0xC0,0xC0,0x60,0xC0,0x60,0x00,0x00,0x00,0x00},
    {0xC0,0x60,0xC0,0x60,0x60,0xC0,0x60,0xC0,0x31,0x80,0x1B,0x00,0x1B,0x00,0x0E,0x00,0x0E,0x00,0x04,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x00,0x00,0x00,0x00},
    {0xFF,0xC0,0xFF,0xC0,0x00,0xC0,0x01,0x80,0x03,0x00,0x06,0x00,0x0C,0x00,0x18,0x00,0x30,0x00,0x60,0x00,0xC0,0x00,0xC0,0x00,0xFF,0xC0,0xFF,0xC0,0x00,0x00,0x00,0x00}
};

//=============================================================================
// Unpack template to int array (0 or 255)
//=============================================================================
static void _unpack(const unsigned char* src, int* dst)
{
    for (int j = 0; j < T_H; j++)
    {
        int row = (src[j * 2] << 8) | src[j * 2 + 1];
        for (int i = 0; i < T_W; i++)
            dst[j * T_W + i] = ((row >> (15 - i)) & 1) ? 255 : 0;
    }
}

//=============================================================================
// Normalize char to template size
//=============================================================================
static void _normChar(int* src, int sw, int sh, int* dst, int dw, int dh)
{
    for (int j = 0; j < dh; j++)
        for (int i = 0; i < dw; i++)
        {
            int sx = (int)(i * sw / (double)dw);
            int sy = (int)(j * sh / (double)dh);
            if (sx >= sw) sx = sw - 1;
            if (sy >= sh) sy = sh - 1;
            dst[j * dw + i] = src[sy * sw + sx];
        }
}

//=============================================================================
// NCC (no sqrt from <cmath> needed - use squared comparison)
//=============================================================================
static double _mySqrt(double x)
{
    if (x <= 0) return 0;
    double r = x;
    for (int i = 0; i < 20; i++)
        r = (r + x / r) * 0.5;
    return r;
}

static double _ncc(int* a, int* b, int n)
{
    double sa = 0, sb = 0;
    for (int i = 0; i < n; i++) { sa += a[i]; sb += b[i]; }
    double ma = sa / n, mb = sb / n;
    double aa = 0, bb = 0, ab = 0;
    for (int i = 0; i < n; i++)
    {
        double da = a[i] - ma, db = b[i] - mb;
        aa += da * da; bb += db * db; ab += da * db;
    }
    if (aa < 1e-10 || bb < 1e-10) return 0.0;
    return ab / _mySqrt(aa * bb);
}

//=============================================================================
// Match a character against templates
//=============================================================================
static int _matchChar(int* img, int cw, int ch, char& best, double& score)
{
    int* norm = new int[T_W * T_H];
    _normChar(img, cw, ch, norm, T_W, T_H);
    int* t = new int[T_W * T_H];
    score = -1.0; best = '?';

    for (int d = 0; d < 10; d++) {
        _unpack(dT[d], t);
        double s = _ncc(norm, t, T_W * T_H);
        if (s > score) { score = s; best = '0' + d; }
    }
    for (int l = 0; l < 26; l++) {
        _unpack(lT[l], t);
        double s = _ncc(norm, t, T_W * T_H);
        if (s > score) { score = s; best = 'A' + l; }
    }

    delete[] norm; delete[] t;
    return (score > 0.15) ? 1 : 0;
}

//=============================================================================
// Grayscale darkness conversion (255 - pixel)
//=============================================================================
static void _toDarkness(int* f, int w, int h, int* g)
{
    for (int i = 0; i < w * h; i++)
        g[i] = 255 - f[i];
}

//=============================================================================
// Bilinear normChar for grayscale images
//=============================================================================
static void _bilinearNormChar(int* src, int sw, int sh, int* dst, int dw, int dh)
{
    for (int j = 0; j < dh; j++)
        for (int i = 0; i < dw; i++)
        {
            double sx = i * sw / (double)dw;
            double sy = j * sh / (double)dh;
            int ix = (int)sx, iy = (int)sy;
            double fx = sx - ix, fy = sy - iy;
            int ix1 = (ix + 1 < sw) ? ix + 1 : ix;
            int iy1 = (iy + 1 < sh) ? iy + 1 : iy;
            double v = (1.0 - fx) * (1.0 - fy) * src[iy * sw + ix]
                     + fx * (1.0 - fy) * src[iy * sw + ix1]
                     + (1.0 - fx) * fy * src[iy1 * sw + ix]
                     + fx * fy * src[iy1 * sw + ix1];
            dst[j * dw + i] = _clip_pixel(v);
        }
}

//=============================================================================
// Grayscale-based character segmentation (baseline-subtracted projection)
//=============================================================================
static int segmentGrayscale(int* dark, int w, int h, int* regions, int maxRegions)
{
    int* hproj = new int[h]();
    for (int j = 0; j < h; j++)
    {
        int sum = 0;
        for (int i = 0; i < w; i++)
            sum += dark[j * w + i];
        hproj[j] = sum;
    }

    int hMin = hproj[0], hMax = hproj[0];
    for (int j = 1; j < h; j++)
    {
        if (hproj[j] < hMin) hMin = hproj[j];
        if (hproj[j] > hMax) hMax = hproj[j];
    }
    int hRange = hMax - hMin;
    int hThresh = hMin + hRange / 4;

    int yStart = -1, yEnd = -1;
    for (int j = 0; j < h; j++)
        if (hproj[j] > hThresh) { if (yStart < 0) yStart = j; yEnd = j; }
    if (yStart < 0) { yStart = 0; yEnd = h - 1; }

    int bandH = yEnd - yStart + 1;

    int* vproj = new int[w]();
    for (int i = 0; i < w; i++)
    {
        int sum = 0;
        for (int j = yStart; j <= yEnd; j++)
            sum += dark[j * w + i];
        vproj[i] = sum;
    }

    int vMin = vproj[0], vMax = vproj[0];
    for (int i = 1; i < w; i++)
    {
        if (vproj[i] < vMin) vMin = vproj[i];
        if (vproj[i] > vMax) vMax = vproj[i];
    }
    int vRange = vMax - vMin;

    int* mproj = new int[w]();
    int filterR = 3;
    for (int i = 0; i < w; i++)
    {
        int mv = 0;
        for (int k = -filterR; k <= filterR; k++)
        {
            int idx = i + k;
            if (idx >= 0 && idx < w && vproj[idx] > mv) mv = vproj[idx];
        }
        mproj[i] = mv;
    }

    int charThresh = vMin + vRange * 2 / 5;
    int gapThresh = vMin + vRange / 5;
    if (gapThresh >= charThresh) gapThresh = charThresh / 2;

    int count = 0, inChar = 0, startX = 0;
    for (int i = 0; i < w; i++)
    {
        if (mproj[i] > charThresh && !inChar) { inChar = 1; startX = i; }
        else if ((mproj[i] <= gapThresh || i == w - 1) && inChar)
        {
            inChar = 0;
            int endX = (mproj[i] <= gapThresh) ? i - 1 : i;
            int segW = endX - startX + 1;
            if (segW >= 4 && segW < w / 2 && count < maxRegions)
            {
                int minY = bandH, maxY = 0;
                for (int j = startX; j <= endX; j++)
                    for (int k = yStart; k <= yEnd; k++)
                        if (dark[k * w + j] > 60)
                        {
                            if (k - yStart < minY) minY = k - yStart;
                            if (k - yStart > maxY) maxY = k - yStart;
                        }
                int segH = (maxY - minY + 1 < 5) ? 5 : (maxY - minY + 1);

                int* sHproj = new int[segH]();
                for (int k = 0; k < segH; k++)
                    for (int j = startX; j <= endX; j++)
                        if (dark[(yStart + minY + k) * w + j] > 60)
                            sHproj[k]++;

                int curStart = -1, bestSy = 0, bestSye = segH - 1;
                for (int k = 0; k <= segH; k++)
                {
                    bool active = (k < segH && sHproj[k] > std::max(2, segW / 4));
                    if (active && curStart < 0) curStart = k;
                    if (!active && curStart >= 0)
                    {
                        if (k - curStart > 5) { bestSy = curStart; bestSye = k - 1; }
                        curStart = -1;
                    }
                }
                delete[] sHproj;
                segH = bestSye - bestSy + 1;

                if (segH < 6) continue;

                regions[count * 4 + 0] = startX;
                regions[count * 4 + 1] = yStart + minY + bestSy;
                regions[count * 4 + 2] = segW;
                regions[count * 4 + 3] = segH;
                count++;
            }
        }
    }

    delete[] mproj;
    delete[] vproj;
    delete[] hproj;
    return count;
}

//=============================================================================
// 5x5 Gaussian blur (sigma ≈ 1.0)
// kernel = [1 4 6 4 1; 4 16 24 16 4; 6 24 36 24 6; 4 16 24 16 4; 1 4 6 4 1] / 256
//=============================================================================
static void _gaussianBlur5x5(int* f, int w, int h, int* g)
{
    const int k[5][5] = {
        {1,4,6,4,1},{4,16,24,16,4},{6,24,36,24,6},{4,16,24,16,4},{1,4,6,4,1}
    };
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
        {
            int s = 0;
            for (int dy = -2; dy <= 2; dy++)
                for (int dx = -2; dx <= 2; dx++)
                {
                    int px = i + dx, py = j + dy;
                    if (px >= 0 && px < w && py >= 0 && py < h)
                        s += f[py * w + px] * k[dy + 2][dx + 2];
                    else
                        s += f[j * w + i] * k[dy + 2][dx + 2];
                }
            g[j * w + i] = s / 256;
        }
}

//=============================================================================
// Local contrast enhancement (simplified CLAHE equivalent)
// 1. Estimate background via 31x31 box filter (integral image)
// 2. enhanced = original + 0.4 * (original - background), clipped to [0,255]
//=============================================================================
static void _enhanceContrast(int* f, int w, int h, int* g)
{
    int* integral = new int[(w + 1) * (h + 1)];
    for (int j = 0; j <= h; j++) integral[j * (w + 1)] = 0;
    for (int i = 0; i <= w; i++) integral[i] = 0;
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            integral[(j + 1) * (w + 1) + (i + 1)] =
                f[j * w + i]
                + integral[(j + 1) * (w + 1) + i]
                + integral[j * (w + 1) + (i + 1)]
                - integral[j * (w + 1) + i];

    int r = 15;
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
        {
            int x1 = i - r, y1 = j - r;
            int x2 = i + r, y2 = j + r;
            if (x1 < 0) x1 = 0; if (y1 < 0) y1 = 0;
            if (x2 >= w) x2 = w - 1; if (y2 >= h) y2 = h - 1;
            int area = (x2 - x1 + 1) * (y2 - y1 + 1);
            int sum = integral[(y2 + 1) * (w + 1) + (x2 + 1)]
                    - integral[(y2 + 1) * (w + 1) + x1]
                    - integral[y1 * (w + 1) + (x2 + 1)]
                    + integral[y1 * (w + 1) + x1];
            int mean = sum / area;
            int v = f[j * w + i] + (int)(0.4 * (f[j * w + i] - mean));
            if (v < 0) v = 0; if (v > 255) v = 255;
            g[j * w + i] = v;
        }
    delete[] integral;
}

//=============================================================================
// Binary dilation (1 = character pixel) — bridges dot-matrix gaps
//=============================================================================
static void _dilateBinary(int* f, int w, int h, int* g, int radius)
{
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
        {
            int mx = 0;
            for (int dy = -radius; dy <= radius; dy++)
                for (int dx = -radius; dx <= radius; dx++)
                {
                    int px = i + dx, py = j + dy;
                    if (px >= 0 && px < w && py >= 0 && py < h)
                        if (f[py * w + px] == 255) { mx = 255; break; }
                }
            g[j * w + i] = mx;
        }
}

//=============================================================================
// Grayscale NCC matching (with binary threshold + dilation to bridge dots)
//=============================================================================
//=============================================================================
// Binary morph close (255 = character pixel)
//=============================================================================
static void _closeBinary(int* f, int w, int h, int* g, int radius)
{
    int* tmp = new int[w * h];
    // Dilate
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
        {
            int mx = 0;
            for (int dy = -radius; dy <= radius; dy++)
                for (int dx = -radius; dx <= radius; dx++)
                {
                    int px = i + dx, py = j + dy;
                    if (px >= 0 && px < w && py >= 0 && py < h)
                        if (f[py * w + px] == 255) { mx = 255; break; }
                }
            tmp[j * w + i] = mx;
        }
    // Erode
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
        {
            int mn = 255;
            for (int dy = -radius; dy <= radius; dy++)
                for (int dx = -radius; dx <= radius; dx++)
                {
                    int px = i + dx, py = j + dy;
                    if (px >= 0 && px < w && py >= 0 && py < h)
                        if (tmp[py * w + px] < mn) mn = tmp[py * w + px];
                }
            g[j * w + i] = mn;
        }
    delete[] tmp;
}

//=============================================================================
// Compute projection profiles (vertical + horizontal) for a binary image
// Concatenated into a feature vector of length (w + h)
//=============================================================================
static void _projectionProfile(int* img, int w, int h, double* feat)
{
    for (int i = 0; i < w; i++) {
        int sum = 0;
        for (int j = 0; j < h; j++)
            if (img[j * w + i] == 255) sum++;
        feat[i] = (double)sum / h;
    }
    for (int j = 0; j < h; j++) {
        int sum = 0;
        for (int i = 0; i < w; i++)
            if (img[j * w + i] == 255) sum++;
        feat[w + j] = (double)sum / w;
    }
}

//=============================================================================
// Cosine similarity between two feature vectors
//=============================================================================
static double _cosineSim(double* a, double* b, int len)
{
    double aa = 0, bb = 0, ab = 0;
    for (int i = 0; i < len; i++) {
        aa += a[i] * a[i];
        bb += b[i] * b[i];
        ab += a[i] * b[i];
    }
    if (aa < 1e-10 || bb < 1e-10) return 0;
    return ab / _mySqrt(aa * bb);
}

//=============================================================================
// Template projection profiles (precomputed once)
//=============================================================================
static double _tplProj[36][28]; // 36 chars × (T_W + T_H) = 12 + 16 = 28
static int _tplProjReady = 0;

static void _initTplProj()
{
    if (_tplProjReady) return;
    int* t = new int[T_W * T_H];
    for (int d = 0; d < 10; d++) {
        _unpack(dT[d], t);
        _projectionProfile(t, T_W, T_H, _tplProj[d]);
    }
    for (int l = 0; l < 26; l++) {
        _unpack(lT[l], t);
        _projectionProfile(t, T_W, T_H, _tplProj[10 + l]);
    }
    delete[] t;
    _tplProjReady = 1;
}

//=============================================================================
// Max-pool normalization: for each output cell, take the maximum pixel
// value from the corresponding source region. Preserves dot-matrix
// character structure (unlike bilinear which blurs dots into gray).
//=============================================================================
static void _maxPoolNormChar(int* src, int sw, int sh, int* dst, int dw, int dh)
{
    for (int j = 0; j < dh; j++)
        for (int i = 0; i < dw; i++)
        {
            int x0 = (i * sw) / dw;
            int x1 = ((i + 1) * sw) / dw;
            int y0 = (j * sh) / dh;
            int y1 = ((j + 1) * sh) / dh;
            if (x1 > sw) x1 = sw;
            if (y1 > sh) y1 = sh;
            if (x0 >= x1) x1 = x0 + 1;
            if (y0 >= y1) y1 = y0 + 1;
            int mx = 0;
            for (int y = y0; y < y1; y++)
                for (int x = x0; x < x1; x++)
                    if (src[y * sw + x] > mx) mx = src[y * sw + x];
            dst[j * dw + i] = mx;
        }
}

//=============================================================================
// Grayscale matching via NCC on brightness image
// (max-pool norm to 12×16 → NCC vs solid templates)
// Input: bright = character pixels bright, background dark (template convention)
//=============================================================================
static int _matchGrayscale(int* bright, int cw, int ch, char& best, double& score)
{
    int* norm = new int[T_W * T_H];
    _maxPoolNormChar(bright, cw, ch, norm, T_W, T_H);
    // Binarize max-pool output so both input and template are 0/255
    for (int i = 0; i < T_W * T_H; i++)
        norm[i] = (norm[i] > 128) ? 255 : 0;

    int* t = new int[T_W * T_H];
    score = -1.0; best = '?';

    for (int d = 0; d < 10; d++) {
        _unpack(dT[d], t);
        double s = _ncc(norm, t, T_W * T_H);
        if (s > score) { score = s; best = '0' + d; }
    }
    for (int l = 0; l < 26; l++) {
        _unpack(lT[l], t);
        double s = _ncc(norm, t, T_W * T_H);
        if (s > score) { score = s; best = 'A' + l; }
    }

    delete[] norm; delete[] t;
    return (score > 0.50) ? 1 : 0;
}

//=============================================================================
// Format correction
//=============================================================================
static void _formatNumber(const char* raw, int n, char* out)
{
    char cl[8] = { 0 };
    int ci = 0;
    for (int i = 0; i < n && ci < 6; i++)
    {
        char c = raw[i];
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z'))
            cl[ci++] = c;
    }

    const char dBad[] = { 'O','Q','D','I','L','Z','S','G','B' };
    const char dGood[] = { '0','0','0','1','1','2','5','6','8' };
    const char lBad[] = { '8','0','1','5','2','6' };
    const char lGood[] = { 'B','O','I','S','Z','G' };

    for (int i = 0; i < 6; i++)
    {
        char c = cl[i];
        if (c == 0) c = (i < 5) ? '0' : 'A';

        if (i < 5)
        {
            int found = 0;
            for (int k = 0; k < 9; k++) if (c == dBad[k]) { out[i] = dGood[k]; found = 1; break; }
            if (!found) out[i] = (c >= '0' && c <= '9') ? c : '0';
        }
        else
        {
            int found = 0;
            for (int k = 0; k < 6; k++) if (c == lBad[k]) { out[i] = lGood[k]; found = 1; break; }
            if (!found) out[i] = (c >= 'A' && c <= 'Z') ? c : 'A';
        }
    }
    out[6] = '\0';

    // Filter excessive consecutive '1's (oversegmentation safety net)
    for (int i = 1; i < 4; i++)
    {
        if (out[i] == '1' && out[i-1] == '1' && out[i+1] == '1')
            out[i] = '4';
    }
}

//=============================================================================
// Debug entry point: returns darkness image + hybrid segment boxes
// C# must allocate debugImg buffer with max size (roiW*3 * roiH*3)
//=============================================================================
void debugLotNumber(int* f, int w, int h,
    int* debugImg, int& debugW, int& debugH,
    int& segCount, int* segBoxes)
{
    if (!f || w < 1 || h < 1 || !debugImg) {
        debugW = 0; debugH = 0; segCount = 0;
        return;
    }

    int rx, ry, rw, rh, *roi = NULL;
    getLotROI(f, w, h, roi, rw, rh, rx, ry);

    int uw = rw * 3, uh = rh * 3;
    int* up = new int[uw * uh];
    _resizeBilinear(roi, rw, rh, up, uw, uh);

    int* enhanced = new int[uw * uh];
    _enhanceContrast(up, uw, uh, enhanced);

    // Light blur for segmentation (same as recognizeLotNumber)
    int* smoothed = new int[uw * uh];
    _gaussianBlur5x5(enhanced, uw, uh, smoothed);

    // Copy enhanced image to output (preprocessed version)
    for (int i = 0; i < uw * uh; i++)
        debugImg[i] = enhanced[i];
    debugW = uw;
    debugH = uh;

    // Binary pipeline for segmentation (same as recognizeLotNumber)
    int* binary = new int[uw * uh];
    _adaptiveThreshold(smoothed, uw, uh, binary, 21, 3);

    int* hproj = new int[uh]();
    for (int j = 0; j < uh; j++) {
        int c = 0;
        for (int i = 0; i < uw; i++)
            if (binary[j * uw + i] == 0) c++;
        hproj[j] = c;
    }
    int hTh = std::max(5, uw / 40);
    int ys = -1, ye = -1;
    for (int j = 0; j < uh; j++)
        if (hproj[j] > hTh) { if (ys < 0) ys = j; ye = j; }
    if (ys < 0) { ys = 0; ye = uh - 1; }
    int bandH = ye - ys + 1;

    int* inverted = new int[uw * bandH];
    for (int j = 0; j < bandH; j++)
        for (int i = 0; i < uw; i++)
            inverted[j * uw + i] = (binary[(ys + j) * uw + i] == 0) ? 255 : 0;

    segCount = segmentCharacters(inverted, uw, bandH, segBoxes, 8);

    // Translate segment y coordinates from band-local to image-local
    for (int i = 0; i < segCount && i < 8; i++)
        segBoxes[i * 4 + 1] += ys;

    delete[] inverted;
    delete[] hproj;
    delete[] binary;
    delete[] smoothed;
    delete[] enhanced;
    delete[] roi;
    delete[] up;
}


//=============================================================================
// Main entry point — hybrid pipeline (binary for segmentation, grayscale NCC)
//=============================================================================
void recognizeLotNumber(int* f, int w, int h, char* lotNumber)
{
    if (!f || w < 1 || h < 1 || !lotNumber) {
        if (lotNumber) lotNumber[0] = '\0';
        return;
    }

    int rx, ry, rw, rh, *roi = NULL;
    getLotROI(f, w, h, roi, rw, rh, rx, ry);

    // 3x bilinear upscale
    int uw = rw * 3, uh = rh * 3;
    int* up = new int[uw * uh];
    _resizeBilinear(roi, rw, rh, up, uw, uh);

    // Contrast enhancement (CLAHE approximation)
    int* enhanced = new int[uw * uh];
    _enhanceContrast(up, uw, uh, enhanced);

    // Light blur before adaptive threshold for band detection
    int* smoothed = new int[uw * uh];
    _gaussianBlur5x5(enhanced, uw, uh, smoothed);

    // Adaptive threshold on full image for band detection + binary
    int bs = 21, Cval = 3;
    int* binary = new int[uw * uh];
    _adaptiveThreshold(smoothed, uw, uh, binary, bs, Cval);

    // Band detection: count BRIGHT pixels (255 = character dots) per row
    int* hproj = new int[uh]();
    for (int j = 0; j < uh; j++) {
        int c = 0;
        for (int i = 0; i < uw; i++)
            if (binary[j * uw + i] == 255) c++;
        hproj[j] = c;
    }
    int hTh = std::max(5, uw / 80);
    int ys = -1, ye = -1;
    for (int j = 0; j < uh; j++)
        if (hproj[j] > hTh) { if (ys < 0) ys = j; ye = j; }
    if (ys < 0) { ys = 0; ye = uh - 1; }
    int bandH = ye - ys + 1;

    // Extract band from binary (255 = character dots, 0 = metal)
    int* bandBin = new int[uw * bandH];
    for (int j = 0; j < bandH; j++)
        for (int i = 0; i < uw; i++)
            bandBin[j * uw + i] = binary[(ys + j) * uw + i];

    // Close BEFORE inversion: fills gaps in character dots (255 → solid)
    _closeBinary(bandBin, uw, bandH, bandBin, 4);

    int reg[8 * 4], nc = segmentCharacters(bandBin, uw, bandH, reg, 8);
    char raw[8]; int rc = 0;

    // === NCC matching on closed binary (255 = character, 0 = background) ===
    for (int c = 0; c < nc && rc < 6; c++)
    {
        int px = reg[c * 4], py = reg[c * 4 + 1];
        int bw = reg[c * 4 + 2], bh = reg[c * 4 + 3];
        if (bw < 4 || bh < 6) continue;

        int* seg = new int[bw * bh];
        for (int j = 0; j < bh; j++)
            for (int i = 0; i < bw; i++)
                seg[j * bw + i] = bandBin[(py + j) * uw + (px + i)];

        char bestChar; double bestScore;
        _matchGrayscale(seg, bw, bh, bestChar, bestScore);

        if (bestScore > 0.50) raw[rc++] = bestChar;
        printf("    Seg[%d] ncc: %c (%.4f)%s (segH=%d)\n", c, bestChar, bestScore, (bestScore > 0.50) ? " ***" : "", bh);
        delete[] seg;
    }

    _formatNumber(raw, rc, lotNumber);
    delete[] bandBin;
    delete[] hproj;
    delete[] binary;
    delete[] smoothed;
    delete[] roi;
    delete[] up;
    delete[] enhanced;
}
