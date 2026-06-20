#pragma once
#include "image_lib.h"
#include <cmath>
#include <cfloat>
#include <vector>
#include <algorithm>

struct Line {
    int rho;
    int theta;
    int votes;
};

void houghTransform(int* f, int w, int h, int*& acc, int& maxRho, int thetaSteps)
{
    int maxDist = (int)sqrt((double)(w * w + h * h)) + 1;
    maxRho = maxDist * 2;
    acc = new int[maxRho * thetaSteps]();

    // ����ץ��G�T�O�u�B�z���㪺��t�I
    // �o�̫�ĳ threshold �]���@�ӾA�����ȡA�άO�̾ڹϹ��B�z���G
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            // �u����t�j�׸������I�~�ѻP�벼
            if (f[j * w + i] > 50) 
            {
                for (int t = 0; t < thetaSteps; t++) {
                    double theta = t * 3.14159265 / thetaSteps;
                    int rho = (int)std::round(i * cos(theta) + j * sin(theta)) + maxDist;
                    if (rho >= 0 && rho < maxRho)
                        acc[rho * thetaSteps + t]++;
                }
            }
        }
    }
}

void detectLines(int* f, int w, int h, int* g, int thetaSteps, int threshold)
{
    // ��l�ƭI�����¦�
    std::fill(g, g + w * h, 0);

    int* acc;
    int maxRho;
    houghTransform(f, w, h, acc, maxRho, thetaSteps);

    int totalCells = maxRho * thetaSteps;
    std::vector<int> visited(totalCells, 0);

    // ���N�����h���u
    for (int iter = 0; iter < 10; iter++) // �վ㭡�N����
    {
        int maxVotes = 0;
        int bestRho = 0, bestTheta = 0;
        
        for (int r = 0; r < maxRho; r++) {
            for (int t = 0; t < thetaSteps; t++) {
                if (!visited[r * thetaSteps + t] && acc[r * thetaSteps + t] > maxVotes) {
                    maxVotes = acc[r * thetaSteps + t];
                    bestRho = r;
                    bestTheta = t;
                }
            }
        }

        if (maxVotes < threshold) break;

        // �аO�w�B�z�ϰ�A�קK���ư����P�@���u
        int nr = bestRho, nt = bestTheta;
        for (int dr = -10; dr <= 10; dr++)
            for (int dt = -10; dt <= 10; dt++)
                if (nr + dr >= 0 && nr + dr < maxRho && nt + dt >= 0 && nt + dt < thetaSteps)
                    visited[(nr + dr) * thetaSteps + (nt + dt)] = 1;

        // ø�s�u���쵲�G�� g
        double theta = bestTheta * 3.14159265 / thetaSteps;
        int rho = bestRho - (int)sqrt((double)(w * w + h * h)) - 1;

        // �ϥΰѼƤ�{�e�u
        for (int i = 0; i < (std::max)(w, h); i++) {
            int x, y;
            if (fabs(sin(theta)) > fabs(cos(theta))) {
                x = i;
                y = (int)((rho - x * cos(theta)) / sin(theta));
            } else {
                y = i;
                x = (int)((rho - y * sin(theta)) / cos(theta));
            }
            if (x >= 0 && x < w && y >= 0 && y < h)
                g[y * w + x] = 255;
        }
    }

    delete[] acc;
}

void detectLinesSobel(int* f, int w, int h, int* g, int thetaSteps, int threshold)
{
    int* edges = new int[w * h];
    int* gx = new int[w * h];
    int* gy = new int[w * h];

    const float sobelX[9] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
    const float sobelY[9] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };

    // ���n�p����
    auto conv = [&](int* src, int w, int h, int* dst, const float* k) {
        for (int j = 0; j < h; j++) {
            for (int i = 0; i < w; i++) {
                float s = 0;
                for (int ky = -1; ky <= 1; ky++) {
                    for (int kx = -1; kx <= 1; kx++) {
                        int px = i + kx, py = j + ky;
                        if (px >= 0 && px < w && py >= 0 && py < h)
                            s += src[py * w + px] * k[(ky + 1) * 3 + (kx + 1)];
                    }
                }
                dst[j * w + i] = (int)s;
            }
        }
    };

    conv(f, w, h, gx, sobelX);
    conv(f, w, h, gy, sobelY);

    for (int i = 0; i < w * h; i++) {
        edges[i] = (int)sqrt((double)gx[i] * gx[i] + (double)gy[i] * gy[i]);
    }

    // �I�s�D�������
    detectLines(edges, w, h, g, thetaSteps, threshold);

    delete[] edges;
    delete[] gx;
    delete[] gy;
}
