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
    int w, h;
    int* gray = readBMP("lot_number_01.bmp", w, h);
    if (!gray) { printf("Cannot open image\n"); return 1; }

    printf("Image: %dx%d\n", w, h);

    // Get ROI
    int rx, ry, rw, rh, *roi = NULL;
    getLotROI(gray, w, h, roi, rw, rh, rx, ry);
    printf("ROI: (%d,%d) %dx%d\n", rx, ry, rw, rh);

    // Dump ROI grayscale values - sample rows
    printf("\nROI grayscale (middle rows):\n");
    for (int y = rh/2 - 5; y < rh/2 + 5; y++)
    {
        printf("  row %d: ", y);
        for (int x = 0; x < rw; x += 10)
            printf("%3d ", roi[y * rw + x]);
        printf("\n");
    }

    // Resize
    int rw3 = rw * 3, rh3 = rh * 3;
    int* resized = new int[rw3 * rh3];
    _resizeBilinear(roi, rw, rh, resized, rw3, rh3);
    writePGM("debug_resize.pgm", resized, rw3, rh3);

    // Adaptive threshold - try multiple C values, save each
    for (int C : {2, 3, 4, 5})
    {
        int* binary = new int[rw3 * rh3];
        _adaptiveThreshold(resized, rw3, rh3, binary, 15, C);
        char fn[64];
        sprintf(fn, "debug_bin_C%d.pgm", C);
        writePGM(fn, binary, rw3, rh3);

        // Also compute histogram of binary image
        int nBlack = 0, nWhite = 0;
        for (int i = 0; i < rw3 * rh3; i++)
            if (binary[i] == 0) nBlack++; else nWhite++;
        printf("\nAdaptiveThreshold C=%d: black=%d white=%d ratio=%.2f\n",
            C, nBlack, nWhite, (double)nBlack / (nWhite + 1));

        // Vertical projection of binary image
        int* vproj = new int[rw3]();
        for (int i = 0; i < rw3; i++)
        {
            int c = 0;
            for (int j = 0; j < rh3; j++)
                if (binary[j * rw3 + i] == 0) c++;
            vproj[i] = c;
        }
        // Find min/mean/max of projection in center region
        int vmin = rh3, vmax = 0;
        double vsum = 0;
        for (int i = rw3/4; i < rw3*3/4; i++)
        {
            if (vproj[i] < vmin) vmin = vproj[i];
            if (vproj[i] > vmax) vmax = vproj[i];
            vsum += vproj[i];
        }
        printf("  Vproj (center): min=%d max=%d avg=%.1f\n", vmin, vmax, vsum / (rw3/2));

        // Morph close with mr=3 and save
        int* closed = new int[rw3 * rh3];
        _morphClose(binary, rw3, rh3, closed, 3);
        sprintf(fn, "debug_close_C%d_mr3.pgm", C);
        writePGM(fn, closed, rw3, rh3);

        // Invert
        int* inverted = new int[rw3 * rh3];
        for (int i = 0; i < rw3 * rh3; i++)
            inverted[i] = (closed[i] == 0) ? 255 : 0;
        sprintf(fn, "debug_invert_C%d_mr3.pgm", C);
        writePGM(fn, inverted, rw3, rh3);

        // Run segmentation on inverted
        int reg[8*4], nc = segmentCharacters(inverted, rw3, rh3, reg, 8);
        printf("  Segments: %d\n", nc);
        for (int c = 0; c < nc && c < 8; c++)
        {
            int px = reg[c*4], py = reg[c*4+1];
            int cw = reg[c*4+2], ch = reg[c*4+3];
            printf("    [%d] (%d,%d) %dx%d\n", c, px, py, cw, ch);
        }

        delete[] inverted;
        delete[] closed;
        delete[] binary;
        delete[] vproj;
    }

    // Also try without morph close at all
    printf("\n--- NO MORPH CLOSE ---\n");
    for (int C : {2, 3, 4, 5})
    {
        int* binary = new int[rw3 * rh3];
        _adaptiveThreshold(resized, rw3, rh3, binary, 15, C);

        // Directly invert and segment
        int* inverted = new int[rw3 * rh3];
        for (int i = 0; i < rw3 * rh3; i++)
            inverted[i] = (binary[i] == 0) ? 255 : 0;

        int reg[8*4], nc = segmentCharacters(inverted, rw3, rh3, reg, 8);
        printf("C=%d: Segments=%d\n", C, nc);
        for (int c = 0; c < nc && c < 8; c++)
            printf("  [%d] (%d,%d) %dx%d\n", c, reg[c*4], reg[c*4+1], reg[c*4+2], reg[c*4+3]);
        delete[] inverted;
        delete[] binary;
    }

    // NEW: Horizontal projection to find vertical character band
    printf("\n--- HORIZONTAL PROJECTION (C=2 no morph) ---\n");
    {
        int* binary = new int[rw3 * rh3];
        _adaptiveThreshold(resized, rw3, rh3, binary, 15, 2);

        int* hproj = new int[rh3]();
        for (int j = 0; j < rh3; j++)
        {
            int c = 0;
            for (int i = 0; i < rw3; i++)
                if (binary[j * rw3 + i] == 0) c++;
            hproj[j] = c;
        }

        // Print every 10th row
        for (int j = 0; j < rh3; j += 10)
            printf("  row %d: %d\n", j, hproj[j]);

        // Find character band: rows where hproj > threshold
        int hThresh = std::max(5, rw3 / 30);
        printf("  Horizontal thresh: %d\n", hThresh);
        int yStart = -1, yEnd = -1;
        for (int j = 0; j < rh3; j++)
        {
            if (hproj[j] > hThresh) { if (yStart < 0) yStart = j; yEnd = j; }
        }
        printf("  Character Y-band: %d to %d (height %d)\n", yStart, yEnd, yEnd - yStart + 1);

        // Now do vertical projection WITHIN this Y-band and segment
        int* vproj = new int[rw3]();
        for (int i = 0; i < rw3; i++)
        {
            int c = 0;
            for (int j = yStart; j <= yEnd; j++)
                if (binary[j * rw3 + i] == 0) c++;
            vproj[i] = c;
        }

        // Same max-filter as segmentCharacters
        int* mproj = new int[rw3]();
        int filterR = 3;
        for (int i = 0; i < rw3; i++)
        {
            int mv = 0;
            for (int k = -filterR; k <= filterR; k++)
            {
                int idx = i + k;
                if (idx >= 0 && idx < rw3 && vproj[idx] > mv)
                    mv = vproj[idx];
            }
            mproj[i] = mv;
        }

        int charTh = std::max(5, (yEnd - yStart + 1) / 10);
        int gapTh = std::max(2, (yEnd - yStart + 1) / 30);
        printf("  CharThresh=%d GapThresh=%d\n", charTh, gapTh);

        // Print projection (every 10 cols)
        printf("  VProj (band, every 10):\n    ");
        for (int i = 0; i < rw3 && i < 600; i += 10)
            printf("%d ", vproj[i]);
        printf("\n");

        delete[] mproj;
        delete[] vproj;
        delete[] binary;
    }

    // NEW: Full "band-first" pipeline
    printf("\n--- BAND-FIRST PIPELINE ---\n");
    for (int C : {2, 3})
    for (int mr : {2, 3})
    {
        // Step 1: Adaptive threshold (no morph)
        int* binary = new int[rw3 * rh3];
        _adaptiveThreshold(resized, rw3, rh3, binary, 15, C);

        // Step 2: Horizontal projection on binary (0=char) to find Y-band
        int* hproj = new int[rh3]();
        for (int j = 0; j < rh3; j++)
        {
            int c = 0;
            for (int i = 0; i < rw3; i++)
                if (binary[j * rw3 + i] == 0) c++;
            hproj[j] = c;
        }
        int hThresh = std::max(5, rw3 / 40);
        int ys = -1, ye = -1;
        for (int j = 0; j < rh3; j++)
            if (hproj[j] > hThresh) { if (ys < 0) ys = j; ye = j; }
        if (ys < 0) { ys = 0; ye = rh3 - 1; }
        // Pad band slightly
        int pad = 5;
        ys = std::max(0, ys - pad);
        ye = std::min(rh3 - 1, ye + pad);
        int bandH = ye - ys + 1;
        if (bandH < 10) { ys = 0; ye = rh3 - 1; bandH = rh3; }

        // Step 3: Crop binary to Y-band, then morph close, then invert
        int* crop = new int[rw3 * bandH];
        for (int j = 0; j < bandH; j++)
            for (int i = 0; i < rw3; i++)
                crop[j * rw3 + i] = binary[(ys + j) * rw3 + i];

        int* closed = new int[rw3 * bandH];
        _morphClose(crop, rw3, bandH, closed, mr);

        int* inverted = new int[rw3 * bandH];
        for (int i = 0; i < rw3 * bandH; i++)
            inverted[i] = (closed[i] == 0) ? 255 : 0;

        // Step 4: Segment within the band
        int reg[8*4], nc = segmentCharacters(inverted, rw3, bandH, reg, 8);
        printf("C=%d mr=%d band=[%d,%d] segments=%d\n", C, mr, ys, ye, nc);
        for (int c = 0; c < nc && c < 8; c++)
        {
            int px = reg[c*4], py = reg[c*4+1];
            int cw = reg[c*4+2], ch = reg[c*4+3];
            printf("  [%d] (%d,%d) %dx%d\n", c, px, py, cw, ch);

            if (cw < 4 || ch < 4) continue;
            int* ci = new int[cw * ch];
            for (int j = 0; j < ch; j++)
                for (int i = 0; i < cw; i++)
                    ci[j * cw + i] = inverted[(py + j) * rw3 + (px + i)];

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

        delete[] inverted; delete[] closed; delete[] crop;
        delete[] hproj; delete[] binary;
    }

    // NEW: Try global threshold + morph close
    printf("\n--- GLOBAL THRESHOLD ---\n");
    {
        // Find global threshold using Otsu on ROI
        int hist[256] = {0};
        for (int i = 0; i < rw * rh; i++) hist[roi[i]]++;
        int total = rw * rh;
        double sum = 0;
        for (int i = 0; i < 256; i++) sum += i * hist[i];
        double sumB = 0, wB = 0, wF = 0, maxVar = 0;
        int otsuThresh = 0;
        for (int t = 0; t < 256; t++)
        {
            wB += hist[t];
            if (wB == 0) continue;
            wF = total - wB;
            if (wF == 0) break;
            sumB += t * hist[t];
            double mB = sumB / wB;
            double mF = (sum - sumB) / wF;
            double var = wB * wF * (mB - mF) * (mB - mF);
            if (var > maxVar) { maxVar = var; otsuThresh = t; }
        }
        printf("Otsu threshold: %d\n", otsuThresh);

        // Apply global threshold: 0 = character (dark), 255 = bg
        int* global = new int[rw3 * rh3];
        for (int i = 0; i < rw3 * rh3; i++)
            global[i] = (resized[i] < otsuThresh - 10) ? 0 : 255;

        int nBlack = 0, nWhite = 0;
        for (int i = 0; i < rw3 * rh3; i++)
            if (global[i] == 0) nBlack++; else nWhite++;
        printf("Global thresh: black=%d white=%d ratio=%.2f\n", nBlack, nWhite, (double)nBlack / (nWhite+1));

        // Horizontal projection
        int* hproj = new int[rh3]();
        for (int j = 0; j < rh3; j++)
        {
            int c = 0;
            for (int i = 0; i < rw3; i++)
                if (global[j * rw3 + i] == 0) c++;
            hproj[j] = c;
        }
        int hTh = std::max(5, rw3 / 40);
        int ys = -1, ye = -1;
        for (int j = 0; j < rh3; j++)
            if (hproj[j] > hTh) { if (ys < 0) ys = j; ye = j; }
        if (ys < 0) { ys = 0; ye = rh3 - 1; }
        printf("Global thresh band: [%d,%d]\n", ys, ye);

        for (int mr : {0, 1, 2, 3})
        {
            int* crop = new int[rw3 * (ye - ys + 1)];
            for (int j = 0; j < ye - ys + 1; j++)
                for (int i = 0; i < rw3; i++)
                    crop[j * rw3 + i] = global[(ys + j) * rw3 + i];

            int* closed;
            if (mr > 0)
            {
                closed = new int[rw3 * (ye - ys + 1)];
                _morphClose(crop, rw3, ye - ys + 1, closed, mr);
            }
            else
                closed = crop;

            int* inverted = new int[rw3 * (ye - ys + 1)];
            for (int i = 0; i < rw3 * (ye - ys + 1); i++)
                inverted[i] = (closed[i] == 0) ? 255 : 0;

            int reg[8*4], nc = segmentCharacters(inverted, rw3, ye - ys + 1, reg, 8);
            printf("mr=%d segments=%d\n", mr, nc);
            for (int c = 0; c < nc && c < 8; c++)
            {
                int px = reg[c*4], py = reg[c*4+1];
                int cw = reg[c*4+2], ch = reg[c*4+3];
                printf("  [%d] (%d,%d) %dx%d\n", c, px, py, cw, ch);
                if (cw < 4 || ch < 4) continue;
                int* ci = new int[cw * ch];
                for (int j = 0; j < ch; j++)
                    for (int i = 0; i < cw; i++)
                        ci[j * cw + i] = inverted[(py + j) * rw3 + (px + i)];

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

            if (mr > 0) delete[] closed;
            delete[] crop; delete[] inverted;
        }
        delete[] hproj; delete[] global;
    }

    // NEW: Aggressive vertical cropping + alternative matching
    printf("\n--- AGGRESSIVE VERTICAL CROP ---\n");
    for (int C : {2})
    for (int mr : {1, 2})
    {
        int* binary = new int[rw3 * rh3];
        _adaptiveThreshold(resized, rw3, rh3, binary, 15, C);

        // Find horizontal band on binary
        int* hproj = new int[rh3]();
        for (int j = 0; j < rh3; j++)
            for (int i = 0; i < rw3; i++)
                if (binary[j * rw3 + i] == 0) hproj[j]++;
        int hTh = std::max(5, rw3 / 40);
        int ys = -1, ye = -1;
        for (int j = 0; j < rh3; j++)
            if (hproj[j] > hTh) { if (ys < 0) ys = j; ye = j; }
        if (ys < 0) { ys = 0; ye = rh3 - 1; }
        int bandH = ye - ys + 1;

        // Crop, morph close, invert
        int* crop = new int[rw3 * bandH];
        for (int j = 0; j < bandH; j++)
            for (int i = 0; i < rw3; i++)
                crop[j * rw3 + i] = binary[(ys + j) * rw3 + i];
        int* closed = new int[rw3 * bandH];
        _morphClose(crop, rw3, bandH, closed, mr);
        int* inverted = new int[rw3 * bandH];
        for (int i = 0; i < rw3 * bandH; i++)
            inverted[i] = (closed[i] == 0) ? 255 : 0;

        // Segment
        int reg[8*4], nc = segmentCharacters(inverted, rw3, bandH, reg, 8);

        for (int c = 0; c < nc && c < 6; c++)
        {
            int px = reg[c*4], py = reg[c*4+1];
            int cw = reg[c*4+2], ch = reg[c*4+3];

            // Step 1: horizontal projection in segment
            int* shproj = new int[ch]();
            int maxProj = 0;
            for (int j = 0; j < ch; j++) {
                int cnt = 0;
                for (int i = 0; i < cw; i++)
                    if (inverted[(py + j) * rw3 + (px + i)] == 255) cnt++;
                shproj[j] = cnt;
                if (cnt > maxProj) maxProj = cnt;
            }

            // More aggressive threshold: use 20% of max
            int sth = std::max(3, maxProj / 5);
            int sy = -1, sye = -1;
            for (int j = 0; j < ch; j++)
                if (shproj[j] > sth) { if (sy < 0) sy = j; sye = j; }
            if (sy < 0) { sy = 0; sye = ch - 1; }

            // Further tighten: find the core dense band
            // Look for longest run where projection > 30% of max
            int denseTh = std::max(3, maxProj / 3);
            int bestStart = sy, bestEnd = sye, bestLen = sye - sy + 1;
            int curStart = -1;
            for (int j = 0; j < ch; j++) {
                if (shproj[j] > denseTh) {
                    if (curStart < 0) curStart = j;
                } else {
                    if (curStart >= 0) {
                        int len = j - curStart;
                        if (len > 10 && len < bestLen) { // prefer shorter (tighter)
                            bestStart = curStart;
                            bestEnd = j - 1;
                            bestLen = len;
                        }
                        curStart = -1;
                    }
                }
            }
            if (curStart >= 0 && ch - curStart > 10 && (ch - curStart) < bestLen) {
                bestStart = curStart;
                bestEnd = ch - 1;
                bestLen = ch - curStart;
            }

            // Use the densest contiguous band
            sy = bestStart;
            sye = bestEnd;
            // Pad 2px
            sy = std::max(0, sy - 2);
            sye = std::min(ch - 1, sye + 2);
            int tightH = sye - sy + 1;

            delete[] shproj;

            if (cw < 4 || tightH < 4) continue;

            // Extract tightly cropped char
            int* ci = new int[cw * tightH];
            for (int j = 0; j < tightH; j++)
                for (int i = 0; i < cw; i++)
                    ci[j * cw + i] = inverted[(py + sy + j) * rw3 + (px + i)];

            int* norm = new int[T_W * T_H];
            _normChar(ci, cw, tightH, norm, T_W, T_H);
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

            printf("C=%d mr=%d seg[%d] (%d,%d) %dx%d tight=%dpx Best: %c (%.4f)%s\n",
                C, mr, c, px, py, cw, ch, tightH, bestChar, bestScore,
                bestScore > 0.15 ? " ***" : "");

            // Dump first char's normalized pixels
            if (c == 0) {
                printf("  Norm (%dx%d):\n", T_W, T_H);
                for (int j = 0; j < T_H; j++) {
                    printf("    ");
                    for (int i = 0; i < T_W; i++)
                        printf("%c", norm[j * T_W + i] > 128 ? '#' : '.');
                    printf("\n");
                }
                // Also dump the template for '1'
                _unpack(dT[1], t);
                printf("  Template '1':\n");
                for (int j = 0; j < T_H; j++) {
                    printf("    ");
                    for (int i = 0; i < T_W; i++)
                        printf("%c", t[j * T_W + i] > 128 ? '#' : '.');
                    printf("\n");
                }
            }

            delete[] norm; delete[] t; delete[] ci;
        }

        delete[] inverted; delete[] closed; delete[] crop;
        delete[] hproj; delete[] binary;
    }

    // NEW: Try different BS/C combos for cleaner binary
    printf("\n--- CLEANER BINARY (various bs/C) ---\n");
    struct { int bs; int C; int mr; } cleanParams[] = {
        {15, 4, 1}, {15, 4, 2}, {21, 3, 1}, {21, 4, 1}, {31, 3, 1}, {31, 4, 1}
    };
    for (int pi = 0; pi < sizeof(cleanParams)/sizeof(cleanParams[0]); pi++)
    {
        int bs = cleanParams[pi].bs, C = cleanParams[pi].C, mr = cleanParams[pi].mr;

        int* binary = new int[rw3 * rh3];
        _adaptiveThreshold(resized, rw3, rh3, binary, bs, C);

        // Find horizontal band
        int* hproj = new int[rh3]();
        for (int j = 0; j < rh3; j++)
            for (int i = 0; i < rw3; i++)
                if (binary[j * rw3 + i] == 0) hproj[j]++;
        int hTh = std::max(5, rw3 / 50);
        int ys = -1, ye = -1;
        for (int j = 0; j < rh3; j++)
            if (hproj[j] > hTh) { if (ys < 0) ys = j; ye = j; }
        if (ys < 0) { ys = 0; ye = rh3 - 1; }
        int bandH = ye - ys + 1;

        int* crop = new int[rw3 * bandH];
        for (int j = 0; j < bandH; j++)
            for (int i = 0; i < rw3; i++)
                crop[j * rw3 + i] = binary[(ys + j) * rw3 + i];

        int* processed;
        if (mr > 0) {
            processed = new int[rw3 * bandH];
            _morphClose(crop, rw3, bandH, processed, mr);
        } else {
            processed = crop;
        }

        int* inverted = new int[rw3 * bandH];
        for (int i = 0; i < rw3 * bandH; i++)
            inverted[i] = (processed[i] == 0) ? 255 : 0;

        int reg[8*4], nc = segmentCharacters(inverted, rw3, bandH, reg, 8);
        if (nc != 6) { // Skip if not 6 segments
            delete[] inverted;
            if (mr > 0) delete[] processed; else /* crop == processed */
            delete[] crop; delete[] hproj; delete[] binary;
            if (mr == 0) processed = NULL;
            continue;
        }

        char result[7] = {0}; int rc = 0;
        for (int c = 0; c < nc && c < 6; c++)
        {
            int px = reg[c*4], py = reg[c*4+1];
            int cw = reg[c*4+2], ch = reg[c*4+3];

            // Aggressive vertical crop
            int* shproj = new int[ch]();
            int maxProj = 0;
            for (int j = 0; j < ch; j++) {
                int cnt = 0;
                for (int i = 0; i < cw; i++)
                    if (inverted[(py + j) * rw3 + (px + i)] == 255) cnt++;
                shproj[j] = cnt;
                if (cnt > maxProj) maxProj = cnt;
            }
            int sth = std::max(2, maxProj / 3);
            int sy = -1, sye = -1;
            for (int j = 0; j < ch; j++)
                if (shproj[j] > sth) { if (sy < 0) sy = j; sye = j; }

            // Find best dense run
            int bestSy = 0, bestSye = ch - 1, bestLen = ch;
            if (sy >= 0) {
                int curStart = -1;
                for (int j = 0; j <= ch; j++) {
                    bool active = (j < ch && shproj[j] > sth);
                    if (active && curStart < 0) curStart = j;
                    if (!active && curStart >= 0) {
                        int len = j - curStart;
                        if (len > 5 && len < bestLen) { bestSy = curStart; bestSye = j - 1; bestLen = len; }
                        curStart = -1;
                    }
                }
            }

            delete[] shproj;
            int tightH = bestSye - bestSy + 1;
            if (cw < 4 || tightH < 4) continue;

            int* ci = new int[cw * tightH];
            for (int j = 0; j < tightH; j++)
                for (int i = 0; i < cw; i++)
                    ci[j * cw + i] = inverted[(py + bestSy + j) * rw3 + (px + i)];

            int* norm = new int[T_W * T_H];
            _normChar(ci, cw, tightH, norm, T_W, T_H);
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
            if (bestScore > 0.15) result[rc++] = bestChar;

            printf("bs=%d C=%d mr=%d seg[%d] (%d,%d) %dx%d tight=%d Best: %c (%.4f)%s\n",
                bs, C, mr, c, px, py, cw, ch, tightH, bestChar, bestScore,
                bestScore > 0.15 ? " ***" : "");

            delete[] norm; delete[] t; delete[] ci;
        }
        if (rc > 0) {
            result[rc] = '\0';
            printf("  => Result: %s\n", result);
        }

        delete[] inverted;
        if (mr > 0) delete[] processed; else processed = NULL;
        delete[] crop; delete[] hproj; delete[] binary;
    }

    // Grayscale vertical projection on original ROI
    printf("\n--- GRAYSCALE VERTICAL PROJECTION ---\n");
    {
        int* vproj = new int[rw]();
        for (int i = 0; i < rw; i++) {
            int sum = 0;
            for (int j = 0; j < rh; j++)
                sum += roi[j * rw + i];
            vproj[i] = sum / rh; // mean grayscale per column
        }
        printf("  Mean gray per column (every 5th):\n    ");
        for (int i = 0; i < rw; i += 5)
            printf("%d ", vproj[i]);
        printf("\n");

        // Compute gradient of the vertical projection to find character boundaries
        // Character columns: darker (lower value), background: lighter (higher value)
        // Gradient (negative) = transition from light to dark = start of character
        printf("  Diff from smoothed (every 5th):\n    ");
        int smoothR = 3;
        for (int i = 0; i < rw; i += 5) {
            int sum = 0, cnt = 0;
            for (int k = -smoothR; k <= smoothR; k++) {
                int idx = i + k;
                if (idx >= 0 && idx < rw) { sum += vproj[idx]; cnt++; }
            }
            int sm = sum / cnt;
            printf("%d ", sm - vproj[i]); // positive = darker than neighbors = character
        }
        printf("\n");
        delete[] vproj;
    }

    // FINAL APPROACH: use raw grayscale NCC on tightly cropped characters
    // without any binary threshold
    printf("\n--- GRAYSCALE NCC ---\n");
    {
        // Use horizontal projection on grayscale (sum of darkness = 255-pixel value)
        // to find character band
        int* hproj = new int[rh]();
        for (int j = 0; j < rh; j++) {
            int sum = 0;
            for (int i = 0; i < rw; i++)
                sum += (255 - roi[j * rw + i]); // "darkness" sum
            hproj[j] = sum;
        }
        int hTh = std::max(100, rw * 20); // darkness threshold
        int ys = -1, ye = -1;
        for (int j = 0; j < rh; j++)
            if (hproj[j] > hTh) { if (ys < 0) ys = j; ye = j; }
        if (ys < 0) { ys = 0; ye = rh - 1; }
        printf("  Grayscale band: [%d,%d]\n", ys, ye);

        // Within this band, do vertical projection
        int* gvproj = new int[rw]();
        for (int i = 0; i < rw; i++) {
            int sum = 0;
            for (int j = ys; j <= ye; j++)
                sum += (255 - roi[j * rw + i]);
            gvproj[i] = sum;
        }

        // Find character boundaries by looking for dips in grayscale
        printf("  Vertical darkness proj (every 5th):\n    ");
        for (int i = 0; i < rw; i += 5)
            printf("%d ", gvproj[i]);
        printf("\n");

        delete[] hproj; delete[] gvproj;
    }

    delete[] resized;
    delete[] roi;
    delete[] gray;
    return 0;
}
