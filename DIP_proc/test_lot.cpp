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
        "lot_number_04.bmp"
    };

    for (int fi = 0; fi < 1; fi++)
    {
        printf("========== %s ==========\n", testFiles[fi]);

        int w, h;
        int* gray = readBMP(testFiles[fi], w, h);
        if (!gray) { printf("  SKIPPED (file not found)\n"); continue; }

        // Test different parameters
        struct { int bs; int C; int mr; } params[] = {
            {15, 2, 8},   // current
            {15, 2, 4},
            {15, 2, 3},
            {15, 2, 2},
            {15, 2, 1},
            {15, 4, 4},
            {15, 4, 3},
            {15, 5, 3},
            {15, 5, 2},
            {15, 3, 2},
            {21, 2, 3},
            {21, 3, 3},
        };

        int rx, ry, rw, rh, *roi = NULL;
        getLotROI(gray, w, h, roi, rw, rh, rx, ry);

        for (int pi = 0; pi < sizeof(params)/sizeof(params[0]); pi++)
        {
            int bs = params[pi].bs, C = params[pi].C, mr = params[pi].mr;
            printf("\n--- bs=%d C=%d mr=%d ---\n", bs, C, mr);

            int pw, ph, *proc = NULL;
            preprocessForOCR(roi, rw, rh, proc, pw, ph, bs, C, mr);

            int reg[8 * 4], nc = segmentCharacters(proc, pw, ph, reg, 8);
            printf("Segments: %d\n", nc);

            for (int c = 0; c < nc && c < 8; c++)
            {
                int px = reg[c * 4], py = reg[c * 4 + 1];
                int cw = reg[c * 4 + 2], ch = reg[c * 4 + 3];
                printf("  Char %d: (%d,%d) %dx%d\n", c, px, py, cw, ch);

                if (cw < 4 || ch < 4) { printf("    (too small)\n"); continue; }

                int* ci = new int[cw * ch];
                for (int j = 0; j < ch; j++)
                    for (int i = 0; i < cw; i++)
                    {
                        int p = (py + j) * pw + (px + i);
                        ci[j * cw + i] = (p >= 0 && p < pw * ph) ? proc[p] : 255;
                    }

                int* norm = new int[T_W * T_H];
                _normChar(ci, cw, ch, norm, T_W, T_H);
                int* t = new int[T_W * T_H];

                double bestScore = -1.0;
                char bestChar = '?';

                for (int d = 0; d < 10; d++) {
                    _unpack(dT[d], t);
                    double s = _ncc(norm, t, T_W * T_H);
                    if (s > bestScore) { bestScore = s; bestChar = '0' + d; }
                }
                for (int l = 0; l < 26; l++) {
                    _unpack(lT[l], t);
                    double s = _ncc(norm, t, T_W * T_H);
                    if (s > bestScore) { bestScore = s; bestChar = 'A' + l; }
                }

                printf("    Best: %c (%.4f)\n", bestChar, bestScore);
                delete[] ci; delete[] norm; delete[] t;
            }

            // Save debug PGM
            if (mr == 3) {
                char fname[64];
                sprintf(fname, "debug_%02d_bs%d_C%d_mr%d.pgm", fi + 1, bs, C, mr);
                writePGM(fname, proc, pw, ph);
                // Dump column projection
                printf("  Column proj (every 10th col, max 100):\n    ");
                for (int i = 0; i < pw && i < 500; i += 10) {
                    int cnt = 0;
                    for (int j = 0; j < ph; j++)
                        if (proc[j * pw + i] == 255) cnt++;
                    printf("%d ", cnt);
                }
                printf("\n");
            }

            delete[] proc;
        }

        delete[] roi; delete[] gray;
        printf("\n");
    }

    return 0;
}
