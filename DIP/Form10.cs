using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Windows.Forms;

namespace DIP
{
    public partial class EdgeDefectForm : Form
    {
        internal Bitmap SourceImage;

        private int[] gray;
        private int w, h;
        private int kernelRadius = 120;

        public EdgeDefectForm()
        {
            InitializeComponent();
        }

        private void EdgeDefectForm_Load(object sender, EventArgs e)
        {
            if (SourceImage != null)
                pictureBox1.Image = SourceImage;
        }

        private unsafe void btnDetect_Click(object sender, EventArgs e)
        {
            if (SourceImage == null) return;
            Cursor = Cursors.WaitCursor;

            Bitmap src = new Bitmap(SourceImage);
            if (src.PixelFormat != PixelFormat.Format24bppRgb)
            {
                Bitmap converted = new Bitmap(src.Width, src.Height, PixelFormat.Format24bppRgb);
                using (Graphics g = Graphics.FromImage(converted))
                    g.DrawImage(src, 0, 0);
                src.Dispose();
                src = converted;
            }
            w = src.Width; h = src.Height;
            gray = bitmapToGray(src, w, h);

            int otsuVal = computeOtsuThreshold(gray, w, h);

            thresholdTrackBar.Value = otsuVal;
            thresholdTrackBar.Enabled = true;
            lblThreshold.Text = "門檻值: " + otsuVal;

            updateDisplay(otsuVal, kernelRadius);

            Cursor = Cursors.Default;
        }

        private void thresholdTrackBar_Scroll(object sender, EventArgs e)
        {
            if (gray == null) return;
            int t = thresholdTrackBar.Value;
            lblThreshold.Text = "門檻值: " + t;
            updateDisplay(t, kernelRadius);
        }

        private void updateDisplay(int threshold, int radius)
        {
            int[] binary = applyThreshold(gray, w, h, threshold);

            int[] closed = morphClose(binary, w, h, radius);

            int[] defect = new int[w * h];
            for (int i = 0; i < w * h; i++)
            {
                int d = closed[i] - binary[i];
                defect[i] = (d < 0) ? 0 : d;
            }

            int[] labels = new int[w * h];
            int compCount = labelComponents(defect, w, h, labels);

            int[] area = new int[compCount + 1];
            int[] minX = new int[compCount + 1];
            int[] maxX = new int[compCount + 1];
            int[] minY = new int[compCount + 1];
            int[] maxY = new int[compCount + 1];
            for (int i = 1; i <= compCount; i++)
            {
                minX[i] = w; maxX[i] = 0;
                minY[i] = h; maxY[i] = 0;
            }

            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                {
                    int label = labels[y * w + x];
                    if (label > 0)
                    {
                        area[label]++;
                        if (x < minX[label]) minX[label] = x;
                        if (x > maxX[label]) maxX[label] = x;
                        if (y < minY[label]) minY[label] = y;
                        if (y > maxY[label]) maxY[label] = y;
                    }
                }

            int[] result = new int[w * h];
            Array.Copy(gray, result, w * h);

            bool[] keep = new bool[compCount + 1];
            for (int i = 1; i <= compCount; i++)
            {
                int cw = maxX[i] - minX[i] + 1;
                int ch = maxY[i] - minY[i] + 1;
                double aspect = (cw > 0) ? (double)ch / cw : 0;
                keep[i] = (area[i] >= 20 && area[i] <= 2000 && aspect >= 1.2 && ch >= 8);
            }

            for (int y = 1; y < h - 1; y++)
                for (int x = 1; x < w - 1; x++)
                {
                    int idx = y * w + x;
                    int lb = labels[idx];
                    if (lb > 0 && keep[lb] && defect[idx] == 255)
                    {
                        if (defect[idx - 1] == 0 || defect[idx + 1] == 0 ||
                            defect[idx - w] == 0 || defect[idx + w] == 0)
                        {
                            result[idx] = -1;
                            result[idx - 1] = -1;
                            result[idx + 1] = -1;
                            result[idx - w] = -1;
                            result[idx + w] = -1;
                            result[idx - w - 1] = -1;
                            result[idx - w + 1] = -1;
                            result[idx + w - 1] = -1;
                            result[idx + w + 1] = -1;
                        }
                    }
                }

            pictureBox2.Image = intToResultBitmap(result, w, h);
            pictureBox3.Image = intToGrayBitmap(binary, w, h);
            pictureBox4.Image = intToGrayBitmap(closed, w, h);
            pictureBox5.Image = intToGrayBitmap(defect, w, h);
        }

        private unsafe int[] bitmapToGray(Bitmap bmp, int w, int h)
        {
            Rectangle rect = new Rectangle(0, 0, w, h);
            BitmapData data = bmp.LockBits(rect, ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);
            byte* s = (byte*)data.Scan0;
            int stride = data.Stride;

            int[] gray = new int[w * h];
            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                {
                    int idx = y * stride + x * 3;
                    gray[y * w + x] = (int)(s[idx + 2] * 0.299 + s[idx + 1] * 0.587 + s[idx] * 0.114);
                }
            bmp.UnlockBits(data);
            return gray;
        }

        private static int computeOtsuThreshold(int[] gray, int w, int h)
        {
            int[] hist = new int[256];
            int total = w * h;
            for (int i = 0; i < total; i++) hist[gray[i]]++;

            double sum = 0;
            for (int i = 0; i < 256; i++) sum += i * hist[i];

            double sumB = 0;
            int wB = 0, wF = 0;
            double maxVar = 0;
            int threshold = 0;

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
                if (var > maxVar) { maxVar = var; threshold = t; }
            }
            return threshold;
        }

        private static int[] applyThreshold(int[] gray, int w, int h, int threshold)
        {
            int[] binary = new int[w * h];
            for (int i = 0; i < w * h; i++)
                binary[i] = (gray[i] >= threshold) ? 255 : 0;
            return binary;
        }

        private void kernelTrackBar_Scroll(object sender, EventArgs e)
        {
            if (gray == null) return;
            kernelRadius = kernelTrackBar.Value;
            lblKernel.Text = "核心半徑: " + kernelRadius;
            updateDisplay(thresholdTrackBar.Value, kernelRadius);
        }

        private static int[] computeDistanceMap(int[] f, int w, int h)
        {
            int N = w * h;
            int INF = w + h;
            int[] dist = new int[N];
            for (int i = 0; i < N; i++)
                dist[i] = (f[i] == 1) ? INF : 0;

            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                {
                    int idx = y * w + x;
                    if (f[idx] == 1)
                    {
                        if (x > 0) dist[idx] = Math.Min(dist[idx], dist[idx - 1] + 1);
                        if (y > 0) dist[idx] = Math.Min(dist[idx], dist[idx - w] + 1);
                    }
                }

            for (int y = h - 1; y >= 0; y--)
                for (int x = w - 1; x >= 0; x--)
                {
                    int idx = y * w + x;
                    if (f[idx] == 1)
                    {
                        if (x < w - 1) dist[idx] = Math.Min(dist[idx], dist[idx + 1] + 1);
                        if (y < h - 1) dist[idx] = Math.Min(dist[idx], dist[idx + w] + 1);
                    }
                }

            return dist;
        }

        private static int[] dilate(int[] binary, int w, int h, int radius)
        {
            int[] inv = new int[w * h];
            for (int i = 0; i < w * h; i++)
                inv[i] = (binary[i] == 0) ? 1 : 0;

            int[] dist = computeDistanceMap(inv, w, h);

            int[] result = new int[w * h];
            for (int i = 0; i < w * h; i++)
                result[i] = (dist[i] <= radius) ? 255 : 0;
            return result;
        }

        private static int[] erode(int[] binary, int w, int h, int radius)
        {
            int[] f = new int[w * h];
            for (int i = 0; i < w * h; i++)
                f[i] = (binary[i] == 255) ? 1 : 0;

            int[] dist = computeDistanceMap(f, w, h);

            int[] result = new int[w * h];
            for (int i = 0; i < w * h; i++)
                result[i] = (f[i] == 1 && dist[i] > radius) ? 255 : 0;
            return result;
        }

        private static int[] morphClose(int[] binary, int w, int h, int radius)
        {
            int[] dilated = dilate(binary, w, h, radius);
            return erode(dilated, w, h, radius);
        }

        private static int labelComponents(int[] f, int w, int h, int[] labels)
        {
            int total = w * h;
            int[] parent = new int[total];
            for (int i = 0; i < total; i++) parent[i] = i;

            Func<int, int> find = null;
            find = (x) =>
            {
                while (parent[x] != x)
                {
                    parent[x] = parent[parent[x]];
                    x = parent[x];
                }
                return x;
            };
            Action<int, int> unite = (a, b) => { parent[find(a)] = find(b); };

            int nextLabel = 1;
            for (int y = 0; y < h; y++)
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

            int[] remap = new int[nextLabel];
            int compCount = 1;
            for (int i = 1; i < nextLabel; i++)
            {
                int root = find(i);
                if (remap[root] == 0) remap[root] = compCount++;
            }

            for (int i = 0; i < total; i++)
                if (labels[i] > 0)
                    labels[i] = remap[find(labels[i])];

            return compCount - 1;
        }

        private unsafe Bitmap intToGrayBitmap(int[] data, int w, int h)
        {
            Bitmap bmp = new Bitmap(w, h, PixelFormat.Format24bppRgb);
            Rectangle rect = new Rectangle(0, 0, w, h);
            BitmapData bd = bmp.LockBits(rect, ImageLockMode.WriteOnly, PixelFormat.Format24bppRgb);
            byte* d = (byte*)bd.Scan0;
            int stride = bd.Stride;

            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                {
                    int val = data[y * w + x];
                    d[y * stride + x * 3 + 0] = (byte)val;
                    d[y * stride + x * 3 + 1] = (byte)val;
                    d[y * stride + x * 3 + 2] = (byte)val;
                }
            bmp.UnlockBits(bd);
            return bmp;
        }

        private unsafe Bitmap intToResultBitmap(int[] data, int w, int h)
        {
            Bitmap bmp = new Bitmap(w, h, PixelFormat.Format24bppRgb);
            Rectangle rect = new Rectangle(0, 0, w, h);
            BitmapData bd = bmp.LockBits(rect, ImageLockMode.WriteOnly, PixelFormat.Format24bppRgb);
            byte* d = (byte*)bd.Scan0;
            int stride = bd.Stride;

            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                {
                    int val = data[y * w + x];
                    if (val == -1)
                    {
                        d[y * stride + x * 3 + 0] = 0;
                        d[y * stride + x * 3 + 1] = 0;
                        d[y * stride + x * 3 + 2] = 255;
                    }
                    else
                    {
                        d[y * stride + x * 3 + 0] = (byte)val;
                        d[y * stride + x * 3 + 1] = (byte)val;
                        d[y * stride + x * 3 + 2] = (byte)val;
                    }
                }
            bmp.UnlockBits(bd);
            return bmp;
        }
    }
}
