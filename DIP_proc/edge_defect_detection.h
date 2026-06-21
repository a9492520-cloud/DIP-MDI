#pragma once
#include "image_lib.h"
#include "otsu.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstring>

struct EdgeDefect {
    int area;
    int x, y, w, h;
    double aspectRatio;
};

static void getEllipseOffsets(int radius, std::vector<int>& kdx, std::vector<int>& kdy)
{
    int r2 = radius * radius;
    for (int j = -radius; j <= radius; j++)
    {
        for (int i = -radius; i <= radius; i++)
        {
            if (i * i + j * j <= r2)
            {
                kdx.push_back(i);
                kdy.push_back(j);
            }
        }
    }
}

static void dilate(int* f, int w, int h, int* g, int radius)
{
    std::vector<int> kdx, kdy;
    getEllipseOffsets(radius, kdx, kdy);
    int ksize = (int)kdx.size();

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int maxVal = 0;
            for (int k = 0; k < ksize; k++)
            {
                int px = x + kdx[k];
                int py = y + kdy[k];
                if (px >= 0 && px < w && py >= 0 && py < h)
                {
                    if (f[py * w + px] > maxVal) maxVal = f[py * w + px];
                }
            }
            g[y * w + x] = maxVal;
        }
    }
}

static void erode(int* f, int w, int h, int* g, int radius)
{
    std::vector<int> kdx, kdy;
    getEllipseOffsets(radius, kdx, kdy);
    int ksize = (int)kdx.size();

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int minVal = 255;
            for (int k = 0; k < ksize; k++)
            {
                int px = x + kdx[k];
                int py = y + kdy[k];
                if (px >= 0 && px < w && py >= 0 && py < h)
                {
                    if (f[py * w + px] < minVal) minVal = f[py * w + px];
                }
            }
            g[y * w + x] = minVal;
        }
    }
}

static void morphClose(int* f, int w, int h, int* g, int radius)
{
    int* temp = new int[w * h];
    dilate(f, w, h, temp, radius);
    erode(temp, w, h, g, radius);
    delete[] temp;
}

struct UFNode {
    int parent;
};

static int connectedComponents(int* f, int w, int h, int* labels)
{
    std::fill(labels, labels + w * h, 0);
    int total = w * h;
    UFNode* uf = new UFNode[total];
    for (int i = 0; i < total; i++) uf[i].parent = i;

    auto find = [&](int x) {
        while (uf[x].parent != x)
        {
            uf[x].parent = uf[uf[x].parent].parent;
            x = uf[x].parent;
        }
        return x;
    };
    auto unite = [&](int a, int b) { uf[find(a)].parent = find(b); };

    int nextLabel = 1;
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int idx = y * w + x;
            if (f[idx] == 0) continue;

            int left = (x > 0 && f[idx - 1] != 0) ? labels[idx - 1] : 0;
            int up = (y > 0 && f[idx - w] != 0) ? labels[idx - w] : 0;

            if (left == 0 && up == 0)
                labels[idx] = nextLabel++;
            else if (left != 0 && up == 0)
                labels[idx] = left;
            else if (left == 0 && up != 0)
                labels[idx] = up;
            else
            {
                labels[idx] = left;
                if (left != up) unite(left, up);
            }
        }
    }

    int* remap = new int[nextLabel]();
    int compCount = 1;
    for (int i = 1; i < nextLabel; i++)
    {
        int root = find(i);
        if (remap[root] == 0) remap[root] = compCount++;
    }

    for (int i = 0; i < total; i++)
    {
        if (labels[i] > 0) labels[i] = remap[find(labels[i])];
    }

    delete[] uf;
    delete[] remap;
    return compCount - 1;
}

int edgeDefectDetection(int* f, int w, int h, int* g)
{
    int threshold;
    otsuThreshold(f, w, h, threshold);

    int total = w * h;
    int* binary = new int[total];
    for (int i = 0; i < total; i++)
        binary[i] = (f[i] >= threshold) ? 255 : 0;

    int radius = 120;
    int* closed = new int[total];
    morphClose(binary, w, h, closed, radius);

    int* defect = new int[total];
    for (int i = 0; i < total; i++)
    {
        defect[i] = closed[i] - binary[i];
        if (defect[i] < 0) defect[i] = 0;
    }

    int* labels = new int[total];
    int compCount = connectedComponents(defect, w, h, labels);

    struct CompInfo {
        int area;
        int minX, maxX, minY, maxY;
    };
    CompInfo* comps = new CompInfo[compCount + 1];
    for (int ci = 0; ci <= compCount; ci++)
    {
        comps[ci].area = 0;
        comps[ci].minX = w; comps[ci].maxX = 0;
        comps[ci].minY = h; comps[ci].maxY = 0;
    }

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int idx = y * w + x;
            int label = labels[idx];
            if (label > 0)
            {
                comps[label].area++;
                if (x < comps[label].minX) comps[label].minX = x;
                if (x > comps[label].maxX) comps[label].maxX = x;
                if (y < comps[label].minY) comps[label].minY = y;
                if (y > comps[label].maxY) comps[label].maxY = y;
            }
        }
    }

    std::copy(f, f + total, g);

    int defectCount = 0;
    for (int i = 1; i <= compCount; i++)
    {
        int cw = comps[i].maxX - comps[i].minX + 1;
        int ch = comps[i].maxY - comps[i].minY + 1;
        double aspectRatio = (cw > 0) ? (double)ch / cw : 0.0;

        if (comps[i].area >= 20 && comps[i].area <= 2000 && aspectRatio >= 1.2 && ch >= 8)
        {
            defectCount++;
            int x0 = comps[i].minX, x1 = comps[i].maxX;
            int y0 = comps[i].minY, y1 = comps[i].maxY;
            for (int x = x0; x <= x1; x++)
            {
                if (x >= 0 && x < w && y0 >= 0 && y0 < h) g[y0 * w + x] = 255;
                if (x >= 0 && x < w && y1 >= 0 && y1 < h) g[y1 * w + x] = 255;
            }
            for (int y = y0; y <= y1; y++)
            {
                if (x0 >= 0 && x0 < w && y >= 0 && y < h) g[y * w + x0] = 255;
                if (x1 >= 0 && x1 < w && y >= 0 && y < h) g[y * w + x1] = 255;
            }
        }
    }

    delete[] binary;
    delete[] closed;
    delete[] defect;
    delete[] labels;
    delete[] comps;

    return defectCount;
}

void edgeDefectDetectionAll(int* f, int w, int h, int* binaryOut, int* closedOut, int* defectOut, int* resultOut)
{
    int threshold;
    otsuThreshold(f, w, h, threshold);

    int total = w * h;
    for (int i = 0; i < total; i++)
        binaryOut[i] = (f[i] >= threshold) ? 255 : 0;

    int radius = 120;
    morphClose(binaryOut, w, h, closedOut, radius);

    for (int i = 0; i < total; i++)
    {
        int d = closedOut[i] - binaryOut[i];
        defectOut[i] = (d < 0) ? 0 : d;
    }

    int* labels = new int[total];
    int compCount = connectedComponents(defectOut, w, h, labels);

    struct CompInfo {
        int area;
        int minX, maxX, minY, maxY;
    };
    CompInfo* comps = new CompInfo[compCount + 1];
    for (int ci = 0; ci <= compCount; ci++)
    {
        comps[ci].area = 0;
        comps[ci].minX = w; comps[ci].maxX = 0;
        comps[ci].minY = h; comps[ci].maxY = 0;
    }

    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
        {
            int idx = y * w + x;
            int label = labels[idx];
            if (label > 0)
            {
                comps[label].area++;
                if (x < comps[label].minX) comps[label].minX = x;
                if (x > comps[label].maxX) comps[label].maxX = x;
                if (y < comps[label].minY) comps[label].minY = y;
                if (y > comps[label].maxY) comps[label].maxY = y;
            }
        }

    std::copy(f, f + total, resultOut);

    for (int i = 1; i <= compCount; i++)
    {
        int cw = comps[i].maxX - comps[i].minX + 1;
        int ch = comps[i].maxY - comps[i].minY + 1;
        double aspectRatio = (cw > 0) ? (double)ch / cw : 0.0;

        if (comps[i].area >= 20 && comps[i].area <= 2000 && aspectRatio >= 1.2 && ch >= 8)
        {
            int x0 = comps[i].minX, x1 = comps[i].maxX;
            int y0 = comps[i].minY, y1 = comps[i].maxY;
            for (int x = x0; x <= x1; x++)
            {
                if (x >= 0 && x < w && y0 >= 0 && y0 < h) resultOut[y0 * w + x] = 255;
                if (x >= 0 && x < w && y1 >= 0 && y1 < h) resultOut[y1 * w + x] = 255;
            }
            for (int y = y0; y <= y1; y++)
            {
                if (x0 >= 0 && x0 < w && y >= 0 && y < h) resultOut[y * w + x0] = 255;
                if (x1 >= 0 && x1 < w && y >= 0 && y < h) resultOut[y * w + x1] = 255;
            }
        }
    }

    delete[] labels;
    delete[] comps;
}

int edgeDefectDetection(int* f, int w, int h, int* g, std::vector<EdgeDefect>& defects)
{
    defects.clear();

    int threshold;
    otsuThreshold(f, w, h, threshold);

    int total = w * h;
    int* binary = new int[total];
    for (int i = 0; i < total; i++)
        binary[i] = (f[i] >= threshold) ? 255 : 0;

    int radius = 120;
    int* closed = new int[total];
    morphClose(binary, w, h, closed, radius);

    int* defect = new int[total];
    for (int i = 0; i < total; i++)
    {
        defect[i] = closed[i] - binary[i];
        if (defect[i] < 0) defect[i] = 0;
    }

    int* labels = new int[total];
    int compCount = connectedComponents(defect, w, h, labels);

    struct CompInfo {
        int area;
        int minX, maxX, minY, maxY;
    };
    CompInfo* comps = new CompInfo[compCount + 1];
    for (int ci = 0; ci <= compCount; ci++)
    {
        comps[ci].area = 0;
        comps[ci].minX = w; comps[ci].maxX = 0;
        comps[ci].minY = h; comps[ci].maxY = 0;
    }

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int idx = y * w + x;
            int label = labels[idx];
            if (label > 0)
            {
                comps[label].area++;
                if (x < comps[label].minX) comps[label].minX = x;
                if (x > comps[label].maxX) comps[label].maxX = x;
                if (y < comps[label].minY) comps[label].minY = y;
                if (y > comps[label].maxY) comps[label].maxY = y;
            }
        }
    }

    std::copy(f, f + total, g);

    for (int i = 1; i <= compCount; i++)
    {
        int cw = comps[i].maxX - comps[i].minX + 1;
        int ch = comps[i].maxY - comps[i].minY + 1;
        double aspectRatio = (cw > 0) ? (double)ch / cw : 0.0;

        if (comps[i].area >= 20 && comps[i].area <= 2000 && aspectRatio >= 1.2 && ch >= 8)
        {
            EdgeDefect d;
            d.area = comps[i].area;
            d.x = comps[i].minX;
            d.y = comps[i].minY;
            d.w = cw;
            d.h = ch;
            d.aspectRatio = aspectRatio;
            defects.push_back(d);

            int x0 = comps[i].minX, x1 = comps[i].maxX;
            int y0 = comps[i].minY, y1 = comps[i].maxY;
            for (int x = x0; x <= x1; x++)
            {
                if (x >= 0 && x < w && y0 >= 0 && y0 < h) g[y0 * w + x] = 255;
                if (x >= 0 && x < w && y1 >= 0 && y1 < h) g[y1 * w + x] = 255;
            }
            for (int y = y0; y <= y1; y++)
            {
                if (x0 >= 0 && x0 < w && y >= 0 && y < h) g[y * w + x0] = 255;
                if (x1 >= 0 && x1 < w && y >= 0 && y < h) g[y * w + x1] = 255;
            }
        }
    }

    delete[] binary;
    delete[] closed;
    delete[] defect;
    delete[] labels;
    delete[] comps;

    return (int)defects.size();
}
