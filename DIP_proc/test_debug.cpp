#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>

#include "lot_number.h"

static int* readBMP(const char* path, int& w, int& h)
{
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    unsigned char header[54];
    fread(header, 1, 54, f);
    w = *(int*)&header[18];
    h = abs(*(int*)&header[22]);
    int rowSize = ((w * 24 + 31) / 32) * 4;
    int stride = rowSize;
    unsigned char* bmpData = new unsigned char[stride * h];
    fread(bmpData, 1, stride * h, f);
    fclose(f);
    int* gray = new int[w * h];
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
        {
            int by = h - 1 - y;
            unsigned char b = bmpData[by * stride + x * 3 + 0];
            unsigned char g = bmpData[by * stride + x * 3 + 1];
            unsigned char r = bmpData[by * stride + x * 3 + 2];
            gray[y * w + x] = (int)(r * 0.299 + g * 0.587 + b * 0.114);
        }
    delete[] bmpData;
    return gray;
}

static void writePGM(const char* path, int* data, int w, int h)
{
    FILE* f = fopen(path, "wb");
    if (!f) return;
    fprintf(f, "P5\n%d %d\n255\n", w, h);
    for (int i = 0; i < w * h; i++)
    {
        unsigned char v = (unsigned char)(data[i] > 255 ? 255 : (data[i] < 0 ? 0 : data[i]));
        fwrite(&v, 1, 1, f);
    }
    fclose(f);
}

int main()
{
    int w, h;
    int* gray = readBMP("lot_number_01.bmp", w, h);
    if (!gray) { printf("Cannot open image\n"); return 1; }

    int rx, ry, rw, rh, *roi = NULL;
    getLotROI(gray, w, h, roi, rw, rh, rx, ry);
    printf("ROI: %dx%d (original coords: %d,%d)\n", rw, rh, rx, ry);

    int uw = rw * 3, uh = rh * 3;
    int* up = new int[uw * uh];
    _resizeBilinear(roi, rw, rh, up, uw, uh);
    writePGM("debug_upscale.pgm", up, uw, uh);

    int* dark = new int[uw * uh];
    _toDarkness(up, uw, uh, dark);
    writePGM("debug_dark.pgm", dark, uw, uh);

    // Dump horizontal projection of darkness
    printf("\nHorizontal projection (darkness sum per row, every 10th row):\n");
    int hmaxVal = 0, hmaxRow = 0;
    for (int j = 0; j < uh; j++)
    {
        int sum = 0;
        for (int i = 0; i < uw; i++)
            sum += dark[j * uw + i];
        if (sum > hmaxVal) { hmaxVal = sum; hmaxRow = j; }
        if (j % 10 == 0 || j == uh - 1)
            printf("  row %3d: %d\n", j, sum);
    }
    printf("  Max row: %d (value=%d)\n", hmaxRow, hmaxVal);

    // Vertical projection of center rows (where characters are)
    int centerStart = std::max(0, hmaxRow - 30);
    int centerEnd = std::min(uh - 1, hmaxRow + 30);
    printf("\nVertical projection (darkness sum per column, rows %d-%d):\n", centerStart, centerEnd);
    for (int i = 0; i < uw; i += 20)
    {
        int sum = 0;
        for (int j = centerStart; j <= centerEnd; j++)
            sum += dark[j * uw + i];
        printf("  col %3d: %d\n", i, sum);
    }

    // Now test segmentGrayscale
    printf("\nsegmentGrayscale result:\n");
    int reg[8 * 4], nc = segmentGrayscale(dark, uw, uh, reg, 8);
    printf("  Segments: %d\n", nc);
    for (int c = 0; c < nc && c < 8; c++)
        printf("  [%d] (%d,%d) %dx%d\n", c, reg[c*4], reg[c*4+1], reg[c*4+2], reg[c*4+3]);

    // Let's also check what segmentGrayscale's hproj and vproj look like
    // by manually computing them with lower thresholds
    printf("\nManual band detection with lower thresholds:\n");
    for (int hThreshMul : {1, 2, 5, 10, 20, 50, 100})
    {
        int hThresh = std::max(100, uw * hThreshMul);
        int* hproj = new int[uh]();
        for (int j = 0; j < uh; j++) {
            int sum = 0;
            for (int i = 0; i < uw; i++)
                sum += dark[j * uw + i];
            hproj[j] = sum;
        }
        int ys = -1, ye = -1;
        for (int j = 0; j < uh; j++)
            if (hproj[j] > hThresh) { if (ys < 0) ys = j; ye = j; }
        if (ys >= 0) {
            int bandH = ye - ys + 1;
            // Now do vertical projection in this band
            int* vproj = new int[uw]();
            for (int i = 0; i < uw; i++) {
                int sum = 0;
                for (int j = ys; j <= ye; j++)
                    sum += dark[j * uw + i];
                vproj[i] = sum;
            }
            int* mproj = new int[uw]();
            int filterR = 3;
            for (int i = 0; i < uw; i++) {
                int mv = 0;
                for (int k = -filterR; k <= filterR; k++) {
                    int idx = i + k;
                    if (idx >= 0 && idx < uw && vproj[idx] > mv) mv = vproj[idx];
                }
                mproj[i] = mv;
            }
            int charThresh = std::max(50, bandH * 4);
            int gapThresh = std::max(10, bandH);
            printf("  hThreshMul=%d -> hThresh=%d band=[%d,%d] bandH=%d charTh=%d gapTh=%d\n",
                hThreshMul, hThresh, ys, ye, bandH, charThresh, gapThresh);

            // Count chars found
            int count = 0, inChar = 0, startX = 0;
            for (int i = 0; i < uw; i++)
            {
                if (mproj[i] > charThresh && !inChar) { inChar = 1; startX = i; }
                else if ((mproj[i] <= gapThresh || i == uw - 1) && inChar)
                {
                    inChar = 0;
                    int endX = (mproj[i] <= gapThresh) ? i - 1 : i;
                    int segW = endX - startX + 1;
                    if (segW >= 4 && segW < uw / 2) count++;
                }
            }
            printf("    Characters found: %d\n", count);

            delete[] mproj; delete[] vproj;
        } else {
            printf("  hThreshMul=%d -> hThresh=%d NO band found\n", hThreshMul, hThresh);
        }
        delete[] hproj;
    }

    delete[] dark; delete[] up; delete[] roi; delete[] gray;
    return 0;
}
