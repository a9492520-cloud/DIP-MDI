#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>

// Include the lot_number header directly
#include "DIP_proc/lot_number.h"

// Simple BMP reader (24-bit only)
static unsigned char* readBMP(const char* path, int& w, int& h)
{
    FILE* f = fopen(path, "rb");
    if (!f) { printf("Cannot open %s\n", path); return NULL; }
    
    unsigned char header[54];
    fread(header, 1, 54, f);
    w = *(int*)&header[18];
    h = *(int*)&header[22];
    
    // BMP stores rows bottom-to-top
    int rowSize = ((w * 24 + 31) / 32) * 4;
    unsigned char* bmpData = new unsigned char[rowSize * h];
    fread(bmpData, 1, rowSize * h, f);
    fclose(f);
    
    // Convert to grayscale int array (top-to-bottom)
    int* gray = new int[w * h];
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
        {
            int by = h - 1 - y; // BMP is bottom-up
            unsigned char b = bmpData[by * rowSize + x * 3 + 0];
            unsigned char g = bmpData[by * rowSize + x * 3 + 1];
            unsigned char r = bmpData[by * rowSize + x * 3 + 2];
            gray[y * w + x] = (int)(r * 0.299 + g * 0.587 + b * 0.114);
        }
    
    delete[] bmpData;
    return (unsigned char*)gray;
}

static void writePGM(const char* path, int* data, int w, int h)
{
    FILE* f = fopen(path, "wb");
    fprintf(f, "P5\n%d %d\n255\n", w, h);
    for (int i = 0; i < w * h; i++)
    {
        unsigned char v = (unsigned char)data[i];
        fwrite(&v, 1, 1, f);
    }
    fclose(f);
}

int main()
{
    const char* testFiles[] = {
        "../圖/lot_number_01.bmp",
        "../圖/lot_number_02.bmp",
        "../圖/lot_number_05.bmp",
        "../圖/lot_number_10.bmp"
    };
    
    for (int fi = 0; fi < 4; fi++)
    {
        printf("========== %s ==========\n", testFiles[fi]);
        
        int w, h;
        int* gray = (int*)readBMP(testFiles[fi], w, h);
        if (!gray) continue;
        
        // Run recognizeLotNumber
        char lotNumber[7] = {0};
        recognizeLotNumber(gray, w, h, lotNumber);
        printf("Result: %s\n", lotNumber);
        
        // Now run the debug pipeline
        int rx, ry, rw, rh, *roi = NULL;
        getLotROI(gray, w, h, roi, rw, rh, rx, ry);
        
        int pw, ph, *proc = NULL;
        preprocessForOCR(roi, rw, rh, proc, pw, ph, 15, 2, 8);
        
        writePGM("debug_preproc.pgm", proc, pw, ph);
        
        // Segment and print each character's NCC scores
        int reg[8 * 4], nc = segmentCharacters(proc, pw, ph, reg, 8);
        printf("Segments: %d\n", nc);
        
        for (int c = 0; c < nc && c < 8; c++)
        {
            int px = reg[c * 4], py = reg[c * 4 + 1];
            int cw = reg[c * 4 + 2], ch = reg[c * 4 + 3];
            printf("  Char %d: (%d,%d) %dx%d\n", c, px, py, cw, ch);
            
            // Extract character
            int* ci = new int[cw * ch];
            for (int j = 0; j < ch; j++)
                for (int i = 0; i < cw; i++)
                {
                    int p = (py + j) * pw + (px + i);
                    ci[j * cw + i] = (p >= 0 && p < pw * ph) ? proc[p] : 255;
                }
            
            // Print NCC scores for top-3 matches
            int* norm = new int[T_W * T_H];
            _normChar(ci, cw, ch, norm, T_W, T_H);
            int* t = new int[T_W * T_H];
            
            // Score all digits
            double bestScore = -1.0;
            char bestChar = '?';
            double scores[36] = {0};
            
            for (int d = 0; d < 10; d++) {
                _unpack(dT[d], t);
                scores[d] = _ncc(norm, t, T_W * T_H);
                if (scores[d] > bestScore) { bestScore = scores[d]; bestChar = '0' + d; }
            }
            for (int l = 0; l < 26; l++) {
                _unpack(lT[l], t);
                scores[10 + l] = _ncc(norm, t, T_W * T_H);
                if (scores[10 + l] > bestScore) { bestScore = scores[10 + l]; bestChar = 'A' + l; }
            }
            
            printf("    Best: %c (score=%.4f)\n", bestChar, bestScore);
            printf("    Top 5 scores: ");
            
            // Find top 5
            int topIdx[5] = {0};
            double topVal[5] = {-2, -2, -2, -2, -2};
            for (int s = 0; s < 36; s++) {
                for (int k = 0; k < 5; k++) {
                    if (scores[s] > topVal[k]) {
                        for (int m = 4; m > k; m--) { topVal[m] = topVal[m-1]; topIdx[m] = topIdx[m-1]; }
                        topVal[k] = scores[s];
                        topIdx[k] = s;
                        break;
                    }
                }
            }
            for (int k = 0; k < 5; k++) {
                char ch2 = (topIdx[k] < 10) ? ('0' + topIdx[k]) : ('A' + topIdx[k] - 10);
                printf("%c:%.4f ", ch2, topVal[k]);
            }
            printf("\n");
            
            delete[] ci;
            delete[] norm;
            delete[] t;
        }
        
        delete[] roi;
        delete[] proc;
        delete[] gray;
        printf("\n");
    }
    
    return 0;
}
