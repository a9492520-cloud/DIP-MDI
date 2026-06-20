#include "pch.h"
extern "C" {
	__declspec(dllexport) void encode_gray(int* f, int* g, int w, int h) {
		for (int i = 0; i < w * h; i++)
			g[i] = 255 - f[i];
	}
}