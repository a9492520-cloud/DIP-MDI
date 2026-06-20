using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Windows.Forms;

namespace DIP
{
    public partial class RotateForm : Form
    {
        internal Bitmap SourceImage;

        public RotateForm()
        {
            InitializeComponent();
        }

        private void RotateForm_Load(object sender, EventArgs e)
        {
            if (SourceImage != null)
                pictureBox1.Image = SourceImage;
        }

        private unsafe Bitmap RotateBitmap(Bitmap src, double angleDeg)
        {
            double rad = angleDeg * Math.PI / 180.0;
            double cosA = Math.Abs(Math.Cos(rad));
            double sinA = Math.Abs(Math.Sin(rad));
            int nw = (int)(src.Width * cosA + src.Height * sinA);
            int nh = (int)(src.Width * sinA + src.Height * cosA);

            Bitmap dst = new Bitmap(nw, nh);
            double cx = src.Width / 2.0, cy = src.Height / 2.0;
            double gcx = nw / 2.0, gcy = nh / 2.0;
            cosA = Math.Cos(rad);
            sinA = Math.Sin(rad);

            Rectangle rect = new Rectangle(0, 0, src.Width, src.Height);
            BitmapData srcData = src.LockBits(rect, ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);
            BitmapData dstData = dst.LockBits(new Rectangle(0, 0, nw, nh), ImageLockMode.WriteOnly, PixelFormat.Format24bppRgb);

            byte* s = (byte*)srcData.Scan0;
            byte* d = (byte*)dstData.Scan0;
            int sStride = srcData.Stride;
            int dStride = dstData.Stride;
            int sw = src.Width, sh = src.Height;

            for (int y = 0; y < nh; y++)
                for (int x = 0; x < nw; x++)
                {
                    double rx = (x - gcx) * cosA - (y - gcy) * sinA + cx;
                    double ry = (x - gcx) * sinA + (y - gcy) * cosA + cy;
                    int x0 = (int)rx, y0 = (int)ry;
                    if (x0 < 0 || x0 >= sw - 1 || y0 < 0 || y0 >= sh - 1) continue;

                    double fx = rx - x0, fy = ry - y0;
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

            src.UnlockBits(srcData);
            dst.UnlockBits(dstData);
            return dst;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (SourceImage == null) return;
            Cursor = Cursors.WaitCursor;

            double angle = (double)numericUpDown1.Value;
            Bitmap result = RotateBitmap(new Bitmap(SourceImage), angle);
            pictureBox2.Image = result;

            Cursor = Cursors.Default;
        }
    }
}
