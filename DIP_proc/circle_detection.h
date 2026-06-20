#pragma once
#include "image_lib.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cfloat>

struct Circle {
    int x, y, r;
    int votes;
};

static void sobelEdges(int* f, int w, int h, int* edges)
{
    int* gx = new int[w * h];
    int* gy = new int[w * h];

    const float sobelX[9] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
    const float sobelY[9] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };

    auto conv = [&](int* src, int* dst, const float* k) {
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

    conv(f, gx, sobelX);
    conv(f, gy, sobelY);

    for (int i = 0; i < w * h; i++) {
        edges[i] = (int)sqrt((double)gx[i] * gx[i] + (double)gy[i] * gy[i]);
    }

    delete[] gx;
    delete[] gy;
}

void detectCircles(int* f, int w, int h, int* g, int minR, int maxR)
{
    std::fill(g, g + w * h, 0);

    int* edges = new int[w * h];
    sobelEdges(f, w, h, edges);

    int edgeThreshold = 50;
    int radiusSteps = maxR - minR + 1;

    int accSize = w * h * radiusSteps;
    int* acc = new int[accSize]();

    int totalEdges = 0;
    for (int i = 0; i < w * h; i++) {
        if (edges[i] > edgeThreshold) totalEdges++;
    }

    int* edgePixels = new int[totalEdges * 2];
    int idx = 0;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (edges[y * w + x] > edgeThreshold) {
                edgePixels[idx * 2 + 0] = x;
                edgePixels[idx * 2 + 1] = y;
                idx++;
            }
        }
    }

    for (int e = 0; e < totalEdges; e++) {
        int ex = edgePixels[e * 2 + 0];
        int ey = edgePixels[e * 2 + 1];
        for (int r = minR; r <= maxR; r++) {
            for (int t = 0; t < 360; t += 2) {
                double theta = t * 3.14159265 / 180.0;
                int a = (int)(ex - r * cos(theta));
                int b = (int)(ey - r * sin(theta));
                if (a >= 0 && a < w && b >= 0 && b < h)
                    acc[(b * w + a) * radiusSteps + (r - minR)]++;
            }
        }
    }

    delete[] edgePixels;

    int* visited = new int[accSize]();

    for (int iter = 0; iter < 50; iter++) {
        int maxVotes = 0;
        int bestX = 0, bestY = 0, bestR = 0;

        for (int r = 0; r < radiusSteps; r++) {
            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    int accIdx = (y * w + x) * radiusSteps + r;
                    if (!visited[accIdx] && acc[accIdx] > maxVotes) {
                        maxVotes = acc[accIdx];
                        bestX = x;
                        bestY = y;
                        bestR = r;
                    }
                }
            }
        }

        int circleThreshold = (int)(3.14159265 * 2 * (minR + bestR) * 0.3);
        if (maxVotes < circleThreshold) break;

        for (int dr = -3; dr <= 3; dr++) {
            for (int dy = -5; dy <= 5; dy++) {
                for (int dx = -5; dx <= 5; dx++) {
                    int nx = bestX + dx, ny = bestY + dy, nr = bestR + dr;
                    if (nx >= 0 && nx < w && ny >= 0 && ny < h && nr >= 0 && nr < radiusSteps)
                        visited[(ny * w + nx) * radiusSteps + nr] = 1;
                }
            }
        }

        int radius = minR + bestR;
        for (int t = 0; t < 360; t++) {
            double theta = t * 3.14159265 / 180.0;
            int cx = (int)(bestX + radius * cos(theta));
            int cy = (int)(bestY + radius * sin(theta));
            if (cx >= 0 && cx < w && cy >= 0 && cy < h)
                g[cy * w + cx] = 255;
        }
    }

    delete[] acc;
    delete[] edges;
    delete[] visited;
}
