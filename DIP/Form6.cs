using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Windows.Forms;

namespace DIP
{
    public partial class FilterForm : Form
    {
        internal Bitmap SourceImage;

        public FilterForm()
        {
            InitializeComponent();
        }

        private void FilterForm_Load(object sender, EventArgs e)
        {
            if (SourceImage != null)
                pictureBox1.Image = SourceImage;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (SourceImage == null) return;
            Cursor = Cursors.WaitCursor;

            int ksize = (int)numericUpDown1.Value;
            if (ksize % 2 == 0) ksize++;
            string filter = comboBox1.SelectedItem?.ToString() ?? "Mean";

            Bitmap src = new Bitmap(SourceImage);
            if (src.PixelFormat != PixelFormat.Format24bppRgb)
            {
                Bitmap converted = new Bitmap(src.Width, src.Height, PixelFormat.Format24bppRgb);
                using (Graphics g = Graphics.FromImage(converted))
                    g.DrawImage(src, 0, 0);
                src.Dispose();
                src = converted;
            }
            Bitmap dst = new Bitmap(src.Width, src.Height);

            unsafe
            {
                Rectangle rect = new Rectangle(0, 0, src.Width, src.Height);
                BitmapData srcData = src.LockBits(rect, ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);
                BitmapData dstData = dst.LockBits(rect, ImageLockMode.WriteOnly, PixelFormat.Format24bppRgb);

                byte* s = (byte*)srcData.Scan0;
                byte* d = (byte*)dstData.Scan0;
                int stride = srcData.Stride;
                int w = src.Width;
                int h = src.Height;

                if (filter == "Mean")
                {
                    int half = ksize / 2;
                    for (int y = 0; y < h; y++)
                        for (int x = 0; x < w; x++)
                            for (int c = 0; c < 3; c++)
                            {
                                int sum = 0, count = 0;
                                for (int ky = -half; ky <= half; ky++)
                                    for (int kx = -half; kx <= half; kx++)
                                    {
                                        int px = x + kx, py = y + ky;
                                        if (px >= 0 && px < w && py >= 0 && py < h)
                                        { sum += s[py * stride + px * 3 + c]; count++; }
                                    }
                                d[y * stride + x * 3 + c] = (byte)(sum / count);
                            }
                }
                else if (filter == "Gaussian")
                {
                    int half = ksize / 2;
                    double sigma = (double)numericUpDown2.Value;
                    double[,] kernel = new double[ksize, ksize];
                    double ksum = 0;
                    for (int ky = -half; ky <= half; ky++)
                        for (int kx = -half; kx <= half; kx++)
                        {
                            kernel[ky + half, kx + half] = Math.Exp(-(kx * kx + ky * ky) / (2 * sigma * sigma));
                            ksum += kernel[ky + half, kx + half];
                        }
                    for (int y = 0; y < h; y++)
                        for (int x = 0; x < w; x++)
                            for (int c = 0; c < 3; c++)
                            {
                                double sum = 0;
                                for (int ky = -half; ky <= half; ky++)
                                    for (int kx = -half; kx <= half; kx++)
                                    {
                                        int px = x + kx, py = y + ky;
                                        if (px >= 0 && px < w && py >= 0 && py < h)
                                            sum += s[py * stride + px * 3 + c] * kernel[ky + half, kx + half];
                                    }
                                d[y * stride + x * 3 + c] = (byte)Math.Min(255, Math.Max(0, (int)(sum / ksum)));
                            }
                }
                else if (filter == "Median")
                {
                    int half = ksize / 2;
                    int[] window = new int[ksize * ksize];
                    for (int y = 0; y < h; y++)
                        for (int x = 0; x < w; x++)
                            for (int c = 0; c < 3; c++)
                            {
                                int idx = 0;
                                for (int ky = -half; ky <= half; ky++)
                                    for (int kx = -half; kx <= half; kx++)
                                    {
                                        int px = x + kx, py = y + ky;
                                        if (px >= 0 && px < w && py >= 0 && py < h)
                                            window[idx++] = s[py * stride + px * 3 + c];
                                    }
                                Array.Sort(window, 0, idx);
                                d[y * stride + x * 3 + c] = (byte)window[idx / 2];
                            }
                }
                else if (filter == "Sobel")
                {
                    int[,] sobelX = { { -1, 0, 1 }, { -2, 0, 2 }, { -1, 0, 1 } };
                    int[,] sobelY = { { -1, -2, -1 }, { 0, 0, 0 }, { 1, 2, 1 } };
                    for (int y = 0; y < h; y++)
                        for (int x = 0; x < w; x++)
                            for (int c = 0; c < 3; c++)
                            {
                                int gx = 0, gy = 0;
                                for (int ky = -1; ky <= 1; ky++)
                                    for (int kx = -1; kx <= 1; kx++)
                                    {
                                        int px = x + kx, py = y + ky;
                                        if (px >= 0 && px < w && py >= 0 && py < h)
                                        {
                                            byte v = s[py * stride + px * 3 + c];
                                            gx += v * sobelX[ky + 1, kx + 1];
                                            gy += v * sobelY[ky + 1, kx + 1];
                                        }
                                    }
                                int val = (int)Math.Sqrt(gx * gx + gy * gy);
                                d[y * stride + x * 3 + c] = (byte)Math.Min(255, Math.Max(0, val));
                            }
                }
                else if (filter == "Prewitt")
                {
                    int[,] preX = { { -1, 0, 1 }, { -1, 0, 1 }, { -1, 0, 1 } };
                    int[,] preY = { { -1, -1, -1 }, { 0, 0, 0 }, { 1, 1, 1 } };
                    for (int y = 0; y < h; y++)
                        for (int x = 0; x < w; x++)
                            for (int c = 0; c < 3; c++)
                            {
                                int gx = 0, gy = 0;
                                for (int ky = -1; ky <= 1; ky++)
                                    for (int kx = -1; kx <= 1; kx++)
                                    {
                                        int px = x + kx, py = y + ky;
                                        if (px >= 0 && px < w && py >= 0 && py < h)
                                        {
                                            byte v = s[py * stride + px * 3 + c];
                                            gx += v * preX[ky + 1, kx + 1];
                                            gy += v * preY[ky + 1, kx + 1];
                                        }
                                    }
                                int val = (int)Math.Sqrt(gx * gx + gy * gy);
                                d[y * stride + x * 3 + c] = (byte)Math.Min(255, Math.Max(0, val));
                            }
                }
                else if (filter == "Laplacian")
                {
                    int[,] lap = { { 0, -1, 0 }, { -1, 4, -1 }, { 0, -1, 0 } };
                    for (int y = 0; y < h; y++)
                        for (int x = 0; x < w; x++)
                            for (int c = 0; c < 3; c++)
                            {
                                int sum = 0;
                                for (int ky = -1; ky <= 1; ky++)
                                    for (int kx = -1; kx <= 1; kx++)
                                    {
                                        int px = x + kx, py = y + ky;
                                        if (px >= 0 && px < w && py >= 0 && py < h)
                                            sum += s[py * stride + px * 3 + c] * lap[ky + 1, kx + 1];
                                    }
                                d[y * stride + x * 3 + c] = (byte)Math.Min(255, Math.Max(0, sum));
                            }
                }
                else if (filter == "LoG")
                {
                    int half = ksize / 2;
                    double sigma = (double)numericUpDown2.Value;
                    double[,] kernel = new double[ksize, ksize];
                    double ksum = 0;
                    for (int ky = -half; ky <= half; ky++)
                        for (int kx = -half; kx <= half; kx++)
                        {
                            double r2 = kx * kx + ky * ky;
                            double s2 = sigma * sigma;
                            kernel[ky + half, kx + half] = -(1 - r2 / s2) * Math.Exp(-r2 / (2 * s2));
                            ksum += kernel[ky + half, kx + half];
                        }
                    for (int y = 0; y < h; y++)
                        for (int x = 0; x < w; x++)
                            for (int c = 0; c < 3; c++)
                            {
                                double sum = 0;
                                for (int ky = -half; ky <= half; ky++)
                                    for (int kx = -half; kx <= half; kx++)
                                    {
                                        int px = x + kx, py = y + ky;
                                        if (px >= 0 && px < w && py >= 0 && py < h)
                                            sum += s[py * stride + px * 3 + c] * kernel[ky + half, kx + half];
                                    }
                                d[y * stride + x * 3 + c] = (byte)Math.Min(255, Math.Max(0, (int)sum));
                            }
                }
                else if (filter == "High-Pass")
                {
                    int[,] hp = { { -1, -1, -1 }, { -1, 8, -1 }, { -1, -1, -1 } };
                    for (int y = 0; y < h; y++)
                        for (int x = 0; x < w; x++)
                            for (int c = 0; c < 3; c++)
                            {
                                int sum = 0;
                                for (int ky = -1; ky <= 1; ky++)
                                    for (int kx = -1; kx <= 1; kx++)
                                    {
                                        int px = x + kx, py = y + ky;
                                        if (px >= 0 && px < w && py >= 0 && py < h)
                                            sum += s[py * stride + px * 3 + c] * hp[ky + 1, kx + 1];
                                    }
                                d[y * stride + x * 3 + c] = (byte)Math.Min(255, Math.Max(0, sum));
                            }
                }
                else if (filter == "Low-Pass")
                {
                    int[,] lp = { { 1, 1, 1 }, { 1, 1, 1 }, { 1, 1, 1 } };
                    for (int y = 0; y < h; y++)
                        for (int x = 0; x < w; x++)
                            for (int c = 0; c < 3; c++)
                            {
                                int sum = 0;
                                for (int ky = -1; ky <= 1; ky++)
                                    for (int kx = -1; kx <= 1; kx++)
                                    {
                                        int px = x + kx, py = y + ky;
                                        if (px >= 0 && px < w && py >= 0 && py < h)
                                            sum += s[py * stride + px * 3 + c];
                                    }
                                d[y * stride + x * 3 + c] = (byte)(sum / 9);
                            }
                }
                else if (filter == "Sharpen")
                {
                    int[,] sharp = { { 0, -1, 0 }, { -1, 5, -1 }, { 0, -1, 0 } };
                    for (int y = 0; y < h; y++)
                        for (int x = 0; x < w; x++)
                            for (int c = 0; c < 3; c++)
                            {
                                int sum = 0;
                                for (int ky = -1; ky <= 1; ky++)
                                    for (int kx = -1; kx <= 1; kx++)
                                    {
                                        int px = x + kx, py = y + ky;
                                        if (px >= 0 && px < w && py >= 0 && py < h)
                                            sum += s[py * stride + px * 3 + c] * sharp[ky + 1, kx + 1];
                                    }
                                d[y * stride + x * 3 + c] = (byte)Math.Min(255, Math.Max(0, sum));
                            }
                }
                else if (filter == "Roberts")
                {
                    for (int y = 0; y < h - 1; y++)
                        for (int x = 0; x < w - 1; x++)
                            for (int c = 0; c < 3; c++)
                            {
                                int p1 = s[y * stride + x * 3 + c];
                                int p2 = s[y * stride + (x + 1) * 3 + c];
                                int p3 = s[(y + 1) * stride + x * 3 + c];
                                int p4 = s[(y + 1) * stride + (x + 1) * 3 + c];
                                int val = Math.Abs(p1 - p4) + Math.Abs(p2 - p3);
                                d[y * stride + x * 3 + c] = (byte)Math.Min(255, val);
                            }
                }
                else if (filter == "Erosion")
                {
                    int half = ksize / 2;
                    for (int y = 0; y < h; y++)
                        for (int x = 0; x < w; x++)
                            for (int c = 0; c < 3; c++)
                            {
                                int minVal = 255;
                                for (int ky = -half; ky <= half; ky++)
                                    for (int kx = -half; kx <= half; kx++)
                                    {
                                        int px = x + kx, py = y + ky;
                                        if (px >= 0 && px < w && py >= 0 && py < h)
                                        {
                                            int v = s[py * stride + px * 3 + c];
                                            if (v < minVal) minVal = v;
                                        }
                                    }
                                d[y * stride + x * 3 + c] = (byte)minVal;
                            }
                }
                else if (filter == "Dilation")
                {
                    int half = ksize / 2;
                    for (int y = 0; y < h; y++)
                        for (int x = 0; x < w; x++)
                            for (int c = 0; c < 3; c++)
                            {
                                int maxVal = 0;
                                for (int ky = -half; ky <= half; ky++)
                                    for (int kx = -half; kx <= half; kx++)
                                    {
                                        int px = x + kx, py = y + ky;
                                        if (px >= 0 && px < w && py >= 0 && py < h)
                                        {
                                            int v = s[py * stride + px * 3 + c];
                                            if (v > maxVal) maxVal = v;
                                        }
                                    }
                                d[y * stride + x * 3 + c] = (byte)maxVal;
                            }
                }
                else if (filter == "Opening")
                {
                    int half = ksize / 2;
                    int[] temp = new int[w * h * 3];
                    for (int y = 0; y < h; y++)
                        for (int x = 0; x < w; x++)
                            for (int c = 0; c < 3; c++)
                            {
                                int minVal = 255;
                                for (int ky = -half; ky <= half; ky++)
                                    for (int kx = -half; kx <= half; kx++)
                                    {
                                        int px = x + kx, py = y + ky;
                                        if (px >= 0 && px < w && py >= 0 && py < h)
                                        {
                                            int v = s[py * stride + px * 3 + c];
                                            if (v < minVal) minVal = v;
                                        }
                                    }
                                temp[y * w * 3 + x * 3 + c] = minVal;
                            }
                    for (int y = 0; y < h; y++)
                        for (int x = 0; x < w; x++)
                            for (int c = 0; c < 3; c++)
                            {
                                int maxVal = 0;
                                for (int ky = -half; ky <= half; ky++)
                                    for (int kx = -half; kx <= half; kx++)
                                    {
                                        int px = x + kx, py = y + ky;
                                        if (px >= 0 && px < w && py >= 0 && py < h)
                                        {
                                            int v = temp[py * w * 3 + px * 3 + c];
                                            if (v > maxVal) maxVal = v;
                                        }
                                    }
                                d[y * stride + x * 3 + c] = (byte)maxVal;
                            }
                }
                else if (filter == "Closing")
                {
                    int half = ksize / 2;
                    int[] temp = new int[w * h * 3];
                    for (int y = 0; y < h; y++)
                        for (int x = 0; x < w; x++)
                            for (int c = 0; c < 3; c++)
                            {
                                int maxVal = 0;
                                for (int ky = -half; ky <= half; ky++)
                                    for (int kx = -half; kx <= half; kx++)
                                    {
                                        int px = x + kx, py = y + ky;
                                        if (px >= 0 && px < w && py >= 0 && py < h)
                                        {
                                            int v = s[py * stride + px * 3 + c];
                                            if (v > maxVal) maxVal = v;
                                        }
                                    }
                                temp[y * w * 3 + x * 3 + c] = maxVal;
                            }
                    for (int y = 0; y < h; y++)
                        for (int x = 0; x < w; x++)
                            for (int c = 0; c < 3; c++)
                            {
                                int minVal = 255;
                                for (int ky = -half; ky <= half; ky++)
                                    for (int kx = -half; kx <= half; kx++)
                                    {
                                        int px = x + kx, py = y + ky;
                                        if (px >= 0 && px < w && py >= 0 && py < h)
                                        {
                                            int v = temp[py * w * 3 + px * 3 + c];
                                            if (v < minVal) minVal = v;
                                        }
                                    }
                                d[y * stride + x * 3 + c] = (byte)minVal;
                            }
                }

                src.UnlockBits(srcData);
                dst.UnlockBits(dstData);
            }

            pictureBox2.Image = dst;
            Cursor = Cursors.Default;
        }
    }
}
