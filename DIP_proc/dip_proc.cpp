#include "pch.h"
#include "image_lib.h"
#include "gray_bitslice.h"
#include "brightness_contrast.h"
#include "histogram.h"
#include "filters.h"
#include "resize_median.h"
#include "rotation.h"
#include "otsu.h"
#include "line_detection.h"
#include "circle_detection.h"
#include "lot_number.h"

extern "C" {

    __declspec(dllexport) void rgb_to_gray(int* f, int w, int h, int* g)
    {
        rgbToGray(f, w, h, g);
    }

    __declspec(dllexport) void bit_plane_slice(int* f, int w, int h, int* g, int bit)
    {
        bitPlaneSlice(f, w, h, g, bit);
    }

    __declspec(dllexport) void adjust_brightness(int* f, int w, int h, int ch, int* g, int delta)
    {
        brightness(f, w, h, ch, g, delta);
    }

    __declspec(dllexport) void adjust_contrast(int* f, int w, int h, int ch, int* g, double factor)
    {
        contrast(f, w, h, ch, g, factor);
    }

    __declspec(dllexport) void compute_histogram(int* f, int w, int h, int* hist)
    {
        computeHistogram(f, w, h, hist);
    }

    __declspec(dllexport) void histogram_equalization(int* f, int w, int h, int* g)
    {
        histogramEqualization(f, w, h, g);
    }

    __declspec(dllexport) void histogram_transform(int* f, int w, int h, int* g, int a, int b, int c, int d)
    {
        histogramTransformation(f, w, h, g, a, b, c, d);
    }

    __declspec(dllexport) void filter_mean(int* f, int w, int h, int* g, int ksize)
    {
        meanFilter(f, w, h, g, ksize);
    }

    __declspec(dllexport) void filter_gaussian(int* f, int w, int h, int* g, int ksize, double sigma)
    {
        gaussianFilter(f, w, h, g, ksize, (float)sigma);
    }

    __declspec(dllexport) void filter_median(int* f, int w, int h, int* g, int ksize)
    {
        medianFilter(f, w, h, g, ksize);
    }

    __declspec(dllexport) void filter_sobel(int* f, int w, int h, int* g)
    {
        sobelEdge(f, w, h, g);
    }

    __declspec(dllexport) void filter_prewitt(int* f, int w, int h, int* g)
    {
        prewittEdge(f, w, h, g);
    }

    __declspec(dllexport) void filter_laplacian(int* f, int w, int h, int* g)
    {
        laplacianFilter(f, w, h, g);
    }

    __declspec(dllexport) void filter_log(int* f, int w, int h, int* g, int ksize, double sigma)
    {
        logFilter(f, w, h, g, ksize, (float)sigma);
    }

    __declspec(dllexport) void filter_highpass(int* f, int w, int h, int* g)
    {
        highPassFilter(f, w, h, g);
    }

    __declspec(dllexport) void filter_sharpen(int* f, int w, int h, int* g, double amount)
    {
        sharpenFilter(f, w, h, g, (float)amount);
    }

    __declspec(dllexport) void filter_roberts(int* f, int w, int h, int* g)
    {
        robertsEdge(f, w, h, g);
    }

    __declspec(dllexport) void resize_nearest(int* f, int w, int h, int* g, int nw, int nh)
    {
        resizeNearest(f, w, h, g, nw, nh);
    }

    __declspec(dllexport) void resize_bilinear(int* f, int w, int h, int* g, int nw, int nh)
    {
        resizeBilinear(f, w, h, g, nw, nh);
    }

    __declspec(dllexport) void resize_bicubic(int* f, int w, int h, int* g, int nw, int nh)
    {
        resizeBicubic(f, w, h, g, nw, nh);
    }

    __declspec(dllexport) void rotate_image(int* f, int w, int h, int* g, int gw, int gh, double angle)
    {
        rotateBilinear(f, w, h, g, gw, gh, angle);
    }

    __declspec(dllexport) void rotate_autosize(int* f, int w, int h, int*& g, int& gw, int& gh, double angle)
    {
        rotateAutoSize(f, w, h, g, gw, gh, angle);
    }

    __declspec(dllexport) void otsu_segmentation(int* f, int w, int h, int* g)
    {
        otsuSegmentation(f, w, h, g);
    }

    __declspec(dllexport) int otsu_get_threshold(int* f, int w, int h)
    {
        int t;
        otsuThreshold(f, w, h, t);
        return t;
    }

    __declspec(dllexport) void line_detection_hough(int* f, int w, int h, int* g, int thetaSteps, int threshold)
    {
        detectLines(f, w, h, g, thetaSteps, threshold);
    }

    __declspec(dllexport) void line_detection_sobel_hough(int* f, int w, int h, int* g, int thetaSteps, int threshold)
    {
        detectLinesSobel(f, w, h, g, thetaSteps, threshold);
    }

    __declspec(dllexport) void circle_detection_hough(int* f, int w, int h, int* g, int minR, int maxR)
    {
        detectCircles(f, w, h, g, minR, maxR);
    }

    __declspec(dllexport) void recognize_lot_number(int* f, int w, int h, char* lot_number)
    {
        recognizeLotNumber(f, w, h, lot_number);
    }

    __declspec(dllexport) void debug_lot_number(int* f, int w, int h, int* debug_img, int& debug_w, int& debug_h, int& seg_count, int* seg_boxes)
    {
        debugLotNumber(f, w, h, debug_img, debug_w, debug_h, seg_count, seg_boxes);
    }

}
