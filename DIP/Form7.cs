using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Windows.Forms;

namespace DIP
{
    public partial class ResizeForm : Form
    {
        internal Bitmap SourceImage;
        public Bitmap ResultImage { get; private set; }
        int origW, origH;

        public ResizeForm()
        {
            InitializeComponent();
        }

        private void ResizeForm_Load(object sender, EventArgs e)
        {
            if (SourceImage != null)
            {
                pictureBox1.Image = SourceImage;
                origW = SourceImage.Width;
                origH = SourceImage.Height;
                numericUpDown1.Value = 1.0m;
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (SourceImage == null) return;
            Cursor = Cursors.WaitCursor;

            double scale = (double)numericUpDown1.Value;
            int nw = Math.Max(1, (int)(origW * scale));
            int nh = Math.Max(1, (int)(origH * scale));

            string method = "Bilinear";
            if (radioButton1.Checked) method = "Nearest";
            else if (radioButton2.Checked) method = "Bilinear";
            else if (radioButton3.Checked) method = "Bicubic";

            Bitmap src = new Bitmap(SourceImage);
            if (src.PixelFormat != PixelFormat.Format24bppRgb)
            {
                Bitmap converted = new Bitmap(src.Width, src.Height, PixelFormat.Format24bppRgb);
                using (Graphics g = Graphics.FromImage(converted))
                    g.DrawImage(src, 0, 0);
                src.Dispose();
                src = converted;
            }
            Bitmap dst = new Bitmap(nw, nh);

            unsafe
            {
                Rectangle srcRect = new Rectangle(0, 0, src.Width, src.Height);
                BitmapData srcData = src.LockBits(srcRect, ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);
                BitmapData dstData = dst.LockBits(new Rectangle(0, 0, nw, nh), ImageLockMode.WriteOnly, PixelFormat.Format24bppRgb);

                byte* s = (byte*)srcData.Scan0;
                byte* d = (byte*)dstData.Scan0;
                int sStride = srcData.Stride;
                int dStride = dstData.Stride;
                int sw = src.Width, sh = src.Height;

                for (int y = 0; y < nh; y++)
                {
                    for (int x = 0; x < nw; x++)
                    {
                        double sx = x * sw / (double)nw;
                        double sy = y * sh / (double)nh;

                        if (method == "Nearest")
                        {
                            int ix = (int)(sx + 0.5);
                            int iy = (int)(sy + 0.5);
                            if (ix >= sw) ix = sw - 1;
                            if (iy >= sh) iy = sh - 1;
                            for (int c = 0; c < 3; c++)
                                d[y * dStride + x * 3 + c] = s[iy * sStride + ix * 3 + c];
                        }
                        else if (method == "Bilinear")
                        {
                            int x0 = (int)sx; if (x0 >= sw - 1) x0 = sw - 2;
                            int y0 = (int)sy; if (y0 >= sh - 1) y0 = sh - 2;
                            double fx = sx - x0, fy = sy - y0;
                            for (int c = 0; c < 3; c++)
                            {
                                double v00 = s[y0 * sStride + x0 * 3 + c];
                                double v10 = s[y0 * sStride + (x0 + 1) * 3 + c];
                                double v01 = s[(y0 + 1) * sStride + x0 * 3 + c];
                                double v11 = s[(y0 + 1) * sStride + (x0 + 1) * 3 + c];
                                double top = v00 + (v10 - v00) * fx;
                                double bot = v01 + (v11 - v01) * fx;
                                d[y * dStride + x * 3 + c] = (byte)Math.Min(255, Math.Max(0, (int)(top + (bot - top) * fy)));
                            }
                        }
                        else
                        {
                            for (int c = 0; c < 3; c++)
                            {
                                double sum = 0, wsum = 0;
                                for (int dy = -1; dy <= 2; dy++)
                                    for (int dx = -1; dx <= 2; dx++)
                                    {
                                        int px = (int)sx + dx, py = (int)sy + dy;
                                        if (px < 0) px = 0; if (px >= sw) px = sw - 1;
                                        if (py < 0) py = 0; if (py >= sh) py = sh - 1;
                                        double t = Math.Abs(px - sx);
                                        double cx = (t <= 1) ? (1.5 * t - 2.5) * t * t + 1 : ((-0.5 * t + 2.5) * t - 4) * t + 2;
                                        t = Math.Abs(py - sy);
                                        double cy = (t <= 1) ? (1.5 * t - 2.5) * t * t + 1 : ((-0.5 * t + 2.5) * t - 4) * t + 2;
                                        double w = cx * cy;
                                        sum += s[py * sStride + px * 3 + c] * w;
                                        wsum += w;
                                    }
                                d[y * dStride + x * 3 + c] = (byte)((wsum != 0) ? Math.Min(255, Math.Max(0, (int)(sum / wsum))) : 0);
                            }
                        }
                    }
                }

                src.UnlockBits(srcData);
                dst.UnlockBits(dstData);
            }

            ResultImage = dst;
            DialogResult = DialogResult.OK;
            Close();
        }
    }
}
