using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Windows.Forms;

namespace DIP
{
    public partial class LineDetectForm : Form
    {
        internal Bitmap SourceImage;

        public LineDetectForm()
        {
            InitializeComponent();
        }

        private void LineDetectForm_Load(object sender, EventArgs e)
        {
            if (SourceImage != null)
                pictureBox1.Image = SourceImage;
        }

        private unsafe void button1_Click(object sender, EventArgs e)
        {
            if (SourceImage == null) return;
            Cursor = Cursors.WaitCursor;

            int thetaSteps = 180;
            int threshold = 30;
            Bitmap src = new Bitmap(SourceImage);
            if (src.PixelFormat != PixelFormat.Format24bppRgb)
            {
                Bitmap converted = new Bitmap(src.Width, src.Height, PixelFormat.Format24bppRgb);
                using (Graphics g = Graphics.FromImage(converted))
                    g.DrawImage(src, 0, 0);
                src.Dispose();
                src = converted;
            }
            int w = src.Width, h = src.Height;
            int maxRho = (int)(Math.Sqrt(w * w + h * h) * 2);
            int[,] acc = new int[maxRho, thetaSteps];

            Rectangle rect = new Rectangle(0, 0, w, h);
            BitmapData srcData = src.LockBits(rect, ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);
            byte* s = (byte*)srcData.Scan0;
            int stride = srcData.Stride;

            int[,] grayImg = new int[w, h];
            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                    grayImg[x, y] = (int)(s[y * stride + x * 3 + 2] * 0.299 + s[y * stride + x * 3 + 1] * 0.587 + s[y * stride + x * 3] * 0.114);

            int[,] edges = new int[w, h];
            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                {
                    int sx = 0, sy = 0;
                    for (int ky = -1; ky <= 1; ky++)
                        for (int kx = -1; kx <= 1; kx++)
                        {
                            int px = x + kx, py = y + ky;
                            if (px >= 0 && px < w && py >= 0 && py < h)
                            {
                                int v = grayImg[px, py];
                                sx += v * sobelX[ky + 1, kx + 1];
                                sy += v * sobelY[ky + 1, kx + 1];
                            }
                        }
                    edges[x, y] = (int)Math.Sqrt(sx * sx + sy * sy);
                }
            src.UnlockBits(srcData);

            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                    if (edges[x, y] > threshold)
                    {
                        for (int t = 0; t < thetaSteps; t++)
                        {
                            double theta = t * Math.PI / thetaSteps;
                            int rho = (int)(x * Math.Cos(theta) + y * Math.Sin(theta));
                            rho += maxRho / 2;
                            if (rho >= 0 && rho < maxRho)
                                acc[rho, t]++;
                        }
                    }

            Bitmap dst = new Bitmap(w, h, PixelFormat.Format24bppRgb);
            BitmapData dstData = dst.LockBits(rect, ImageLockMode.WriteOnly, PixelFormat.Format24bppRgb);
            byte* d = (byte*)dstData.Scan0;
            int dStride = dstData.Stride;

            BitmapData srcData2 = src.LockBits(rect, ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);
            byte* s2 = (byte*)srcData2.Scan0;
            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                    for (int c = 0; c < 3; c++)
                        d[y * dStride + x * 3 + c] = s2[y * stride + x * 3 + c];
            src.UnlockBits(srcData2);

            int linesFound = 0;
            bool[,] visited = new bool[maxRho, thetaSteps];
            for (int iter = 0; iter < 30; iter++)
            {
                int maxVotes = 0, bestRho = 0, bestTheta = 0;
                for (int r = 0; r < maxRho; r++)
                    for (int t = 0; t < thetaSteps; t++)
                        if (!visited[r, t] && acc[r, t] > maxVotes)
                        { maxVotes = acc[r, t]; bestRho = r; bestTheta = t; }

                if (maxVotes < threshold) break;
                linesFound++;

                for (int dr = -5; dr <= 5; dr++)
                    for (int dt = -5; dt <= 5; dt++)
                    {
                        int nr = bestRho + dr, nt = bestTheta + dt;
                        if (nr >= 0 && nr < maxRho && nt >= 0 && nt < thetaSteps)
                            visited[nr, nt] = true;
                    }

                double theta = bestTheta * Math.PI / thetaSteps;
                int rhoVal = bestRho - maxRho / 2;

                if (Math.Abs(Math.Sin(theta)) >= Math.Abs(Math.Cos(theta)))
                {
                    for (int x = 0; x < w; x++)
                    {
                        int y = (int)Math.Round((rhoVal - x * Math.Cos(theta)) / Math.Sin(theta));
                        if (y >= 0 && y < h)
                        {
                            d[y * dStride + x * 3 + 0] = 0;
                            d[y * dStride + x * 3 + 1] = 0;
                            d[y * dStride + x * 3 + 2] = 255;
                        }
                    }
                }
                else
                {
                    for (int y = 0; y < h; y++)
                    {
                        int x = (int)Math.Round((rhoVal - y * Math.Sin(theta)) / Math.Cos(theta));
                        if (x >= 0 && x < w)
                        {
                            d[y * dStride + x * 3 + 0] = 0;
                            d[y * dStride + x * 3 + 1] = 0;
                            d[y * dStride + x * 3 + 2] = 255;
                        }
                    }
                }
            }

            dst.UnlockBits(dstData);

            using (Graphics g = Graphics.FromImage(dst))
            using (Font font = new Font("微軟正黑體", 12, FontStyle.Bold))
                g.DrawString($"找到 {linesFound} 條線", font, Brushes.Yellow, 5, 5);

            pictureBox2.Image = dst;
            Cursor = Cursors.Default;
        }

        private unsafe void button2_Click(object sender, EventArgs e)
        {
            if (SourceImage == null) return;
            Cursor = Cursors.WaitCursor;

            int minR = 10;
            int maxR = 80;
            Bitmap src = new Bitmap(SourceImage);
            if (src.PixelFormat != PixelFormat.Format24bppRgb)
            {
                Bitmap converted = new Bitmap(src.Width, src.Height, PixelFormat.Format24bppRgb);
                using (Graphics g = Graphics.FromImage(converted))
                    g.DrawImage(src, 0, 0);
                src.Dispose();
                src = converted;
            }
            int w = src.Width, h = src.Height;

            Rectangle rect = new Rectangle(0, 0, w, h);
            BitmapData srcData = src.LockBits(rect, ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);
            byte* s = (byte*)srcData.Scan0;
            int stride = srcData.Stride;

            int[] grayImg = new int[w * h];
            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                    grayImg[y * w + x] = (int)(s[y * stride + x * 3 + 2] * 0.299 + s[y * stride + x * 3 + 1] * 0.587 + s[y * stride + x * 3] * 0.114);

            int[] gx = new int[w * h];
            int[] gy = new int[w * h];
            int[] edgeMag = new int[w * h];

            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                {
                    int sx = 0, sy = 0;
                    for (int ky = -1; ky <= 1; ky++)
                        for (int kx = -1; kx <= 1; kx++)
                        {
                            int px = x + kx, py = y + ky;
                            if (px >= 0 && px < w && py >= 0 && py < h)
                            {
                                int v = grayImg[py * w + px];
                                sx += v * sobelX[ky + 1, kx + 1];
                                sy += v * sobelY[ky + 1, kx + 1];
                            }
                        }
                    gx[y * w + x] = sx;
                    gy[y * w + x] = sy;
                    edgeMag[y * w + x] = (int)Math.Sqrt(sx * sx + sy * sy);
                }
            src.UnlockBits(srcData);

            int edgeThreshold = 20;
            int angleRange = 5;
            int radiusSteps = maxR - minR + 1;

            int totalEdgePixels = 0;
            for (int i = 0; i < w * h; i++)
                if (edgeMag[i] > edgeThreshold) totalEdgePixels++;

            int[] edgePx = new int[totalEdgePixels];
            int[] edgePy = new int[totalEdgePixels];
            double[] edgeDir = new double[totalEdgePixels];
            int ei = 0;
            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                {
                    int idx = y * w + x;
                    if (edgeMag[idx] > edgeThreshold)
                    {
                        edgePx[ei] = x;
                        edgePy[ei] = y;
                        edgeDir[ei] = Math.Atan2(gy[idx], gx[idx]);
                        ei++;
                    }
                }

            int[] acc = new int[w * h * radiusSteps];

            for (int edgeIdx = 0; edgeIdx < totalEdgePixels; edgeIdx++)
            {
                int ex = edgePx[edgeIdx], ey = edgePy[edgeIdx];
                double dir = edgeDir[edgeIdx];
                for (int r = minR; r <= maxR; r++)
                {
                    for (int da = -angleRange; da <= angleRange; da++)
                    {
                        double a1 = dir + da * Math.PI / 180.0;
                        double a2 = dir + Math.PI + da * Math.PI / 180.0;

                        int ax1 = (int)(ex - r * Math.Cos(a1));
                        int ay1 = (int)(ey - r * Math.Sin(a1));
                        if (ax1 >= 0 && ax1 < w && ay1 >= 0 && ay1 < h)
                            acc[(ay1 * w + ax1) * radiusSteps + (r - minR)]++;

                        int ax2 = (int)(ex - r * Math.Cos(a2));
                        int ay2 = (int)(ey - r * Math.Sin(a2));
                        if (ax2 >= 0 && ax2 < w && ay2 >= 0 && ay2 < h)
                            acc[(ay2 * w + ax2) * radiusSteps + (r - minR)]++;
                    }
                }
            }

            Bitmap dst = new Bitmap(w, h, PixelFormat.Format24bppRgb);
            BitmapData dstData = dst.LockBits(rect, ImageLockMode.WriteOnly, PixelFormat.Format24bppRgb);
            byte* d = (byte*)dstData.Scan0;
            int dStride = dstData.Stride;

            BitmapData srcData2 = src.LockBits(rect, ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);
            byte* s2 = (byte*)srcData2.Scan0;
            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                    for (int c = 0; c < 3; c++)
                        d[y * dStride + x * 3 + c] = s2[y * stride + x * 3 + c];
            src.UnlockBits(srcData2);

            int[] visited = new int[w * h * radiusSteps];
            string results = "";

            int circleCount = 0;
            for (int iter = 0; iter < 50 && circleCount < 5; iter++)
            {
                int maxVotes = 0, bestIdx = -1;
                for (int i = 0; i < w * h * radiusSteps; i++)
                {
                    if (visited[i] == 0 && acc[i] > maxVotes)
                    {
                        maxVotes = acc[i];
                        bestIdx = i;
                    }
                }
                if (bestIdx < 0) break;

                int bestRStep = bestIdx % radiusSteps;
                int bestXY = bestIdx / radiusSteps;
                int bestX = bestXY % w;
                int bestY = bestXY / w;

                int radius = minR + bestRStep;
                int circleThreshold = (int)(Math.PI * 2 * radius * 0.15);
                if (circleThreshold < 10) circleThreshold = 10;
                if (maxVotes < circleThreshold) break;
                if (maxVotes < 5) break;

                circleCount++;

                for (int dr = -3; dr <= 3; dr++)
                    for (int dy = -15; dy <= 15; dy++)
                        for (int dx = -15; dx <= 15; dx++)
                        {
                            int nx = bestX + dx, ny = bestY + dy, nr = bestRStep + dr;
                            if (nx >= 0 && nx < w && ny >= 0 && ny < h && nr >= 0 && nr < radiusSteps)
                                visited[(ny * w + nx) * radiusSteps + nr] = 1;
                        }

                for (int t = 0; t < 360; t++)
                {
                    double theta = t * Math.PI / 180.0;
                    int cx = (int)(bestX + radius * Math.Cos(theta));
                    int cy = (int)(bestY + radius * Math.Sin(theta));
                    if (cx >= 0 && cx < w && cy >= 0 && cy < h)
                    {
                        d[cy * dStride + cx * 3 + 0] = 0;
                        d[cy * dStride + cx * 3 + 1] = 0;
                        d[cy * dStride + cx * 3 + 2] = 255;
                    }
                }

                if (circleCount <= 1)
                {
                    for (int t = -3; t <= 3; t++)
                    {
                        int cx = bestX + t;
                        int cy = bestY;
                        if (cx >= 0 && cx < w && cy >= 0 && cy < h)
                        {
                            d[cy * dStride + cx * 3 + 0] = 0;
                            d[cy * dStride + cx * 3 + 1] = 255;
                            d[cy * dStride + cx * 3 + 2] = 255;
                        }
                        cx = bestX;
                        cy = bestY + t;
                        if (cx >= 0 && cx < w && cy >= 0 && cy < h)
                        {
                            d[cy * dStride + cx * 3 + 0] = 0;
                            d[cy * dStride + cx * 3 + 1] = 255;
                            d[cy * dStride + cx * 3 + 2] = 255;
                        }
                    }
                }

                results += $"中心({bestX},{bestY}) 半徑={radius}\n";
            }

            dst.UnlockBits(dstData);

            using (Graphics g = Graphics.FromImage(dst))
            using (Font font = new Font("微軟正黑體", 12, FontStyle.Bold))
            {
                g.DrawString($"找到 {circleCount} 個圓\n{results}", font, Brushes.Yellow, 5, 5);
            }

            pictureBox2.Image = dst;
            Cursor = Cursors.Default;
        }

        private static readonly int[,] sobelX = { { -1, 0, 1 }, { -2, 0, 2 }, { -1, 0, 1 } };
        private static readonly int[,] sobelY = { { -1, -2, -1 }, { 0, 0, 0 }, { 1, 2, 1 } };
    }
}
