#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>

#include "lot_number.h"

static int* readBMP(const char* path, int& w, int& h)
{
    FILE* f = fopen(path, "rb");
    if (!f) { printf("Cannot open %s\n", path); return NULL; }

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
    const char* testFiles[] = {
        "lot_number_01.bmp",
        "lot_number_02.bmp",
        "lot_number_03.bmp",
        "lot_number_04.bmp",
        "lot_number_05.bmp",
        "lot_number_06.bmp",
        "lot_number_07.bmp",
        "lot_number_08.bmp",
        "lot_number_09.bmp",
        "lot_number_10.bmp",
        "lot_number_11.bmp",
        "lot_number_12.bmp"
    };
    const char* expected[] = {
        "11240B",
        "31576A",
        "31787A",
        "40748A",
        "40896A",
        "41418A",
        "41883A",
        "50718A",
        "60303A",
        "70878A",
        "71358A",
        "80468A"
    };
    int nFiles = sizeof(testFiles) / sizeof(testFiles[0]);

    printf("=== Testing recognizeLotNumber (grayscale pipeline) ===\n\n");

    for (int fi = 0; fi < nFiles; fi++)
    {
        int w, h;
        int* gray = readBMP(testFiles[fi], w, h);
        if (!gray) { printf("%s: SKIPPED (file not found)\n", testFiles[fi]); continue; }

        char result[64] = {0};
        recognizeLotNumber(gray, w, h, result);

        int match = (strcmp(result, expected[fi]) == 0);
        printf("%s: got=\"%s\" expected=\"%s\" %s\n",
            testFiles[fi], result, expected[fi],
            match ? "OK" : "MISMATCH");

        delete[] gray;
    }

    // Detail test on first image using EXACT hybrid pipeline
    printf("\n=== Hybrid pipeline details for %s ===\n", testFiles[0]);
    {
        int w, h;
        int* gray = readBMP(testFiles[0], w, h);
        if (gray)
        {
            int rx, ry, rw, rh, *roi = NULL;
            getLotROI(gray, w, h, roi, rw, rh, rx, ry);

            int uw = rw * 3, uh = rh * 3;
            int* up = new int[uw * uh];
            _resizeBilinear(roi, rw, rh, up, uw, uh);

            int* enhanced = new int[uw * uh];
            _enhanceContrast(up, uw, uh, enhanced);

            // Light blur for segmentation (same as recognizeLotNumber)
            int* smoothed = new int[uw * uh];
            _gaussianBlur5x5(enhanced, uw, uh, smoothed);

            // Same binary segmentation as recognizeLotNumber
            int bs = 21, Cval = 3;
            int* binary = new int[uw * uh];
            _adaptiveThreshold(smoothed, uw, uh, binary, bs, Cval);

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

            printf("  Y-band: [%d,%d] height=%d\n", ys, ye, bandH);

            int* inverted = new int[uw * bandH];
            for (int j = 0; j < bandH; j++)
                for (int i = 0; i < uw; i++)
                    inverted[j * uw + i] = (binary[(ys + j) * uw + i] == 0) ? 255 : 0;

            int reg[8 * 4], nc = segmentCharacters(inverted, uw, bandH, reg, 8);
            printf("  Segments found: %d\n", nc);

            for (int c = 0; c < nc && c < 8; c++)
            {
                int px = reg[c * 4], py = reg[c * 4 + 1];
                int cw = reg[c * 4 + 2], ch = reg[c * 4 + 3];
                printf("  Seg[%d]: (%d,%d) %dx%d (image-coord y=%d..%d)\n",
                    c, px, py, cw, ch, ys + py, ys + py + ch - 1);

                if (cw < 4 || ch < 6) { printf("    (too small)\n"); continue; }

                printf("    Segment height (from binary): %d\n", ch);

                int* ci = new int[cw * ch];
                for (int j = 0; j < ch; j++)
                    for (int i = 0; i < cw; i++)
                        ci[j * cw + i] = enhanced[(ys + py + j) * uw + (px + i)];

                // Display normalized NCC segment
                int* norm = new int[T_W * T_H];
                _bilinearNormChar(ci, cw, ch, norm, T_W, T_H);
                printf("    Normalized segment (bilinear):\n");
                for (int j = 0; j < T_H; j++) {
                    printf("    ");
                    for (int i = 0; i < T_W; i++)
                        printf("%c", norm[j * T_W + i] > 128 ? '#' : '.');
                    printf("\n");
                }
                delete[] norm;

                char bc; double bs;
                int matched = _matchGrayscale(ci, cw, ch, bc, bs);
                printf("    Best: %c (%.4f)%s\n", bc, bs, matched ? " ***" : "");
                delete[] ci;
            }

            delete[] inverted; delete[] hproj; delete[] binary;
            delete[] smoothed; delete[] enhanced; delete[] up; delete[] roi; delete[] gray;
        }
    }

    return 0;
}
