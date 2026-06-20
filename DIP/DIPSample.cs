using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace DIP
{
    public partial class DIPSample : Form
    {
        public DIPSample()
        {
            InitializeComponent();
        }

        //[DllImport("dip_proc.dll", CallingConvention = CallingConvention.Cdecl)]
        //unsafe public static extern void encode_gray(int* f, int w, int h, int* g, int d);

        //[DllImport("DIP_proc.dll", CallingConvention = CallingConvention.Cdecl)]
        //public static extern void recognize_lot_number(int[] f, int w, int h, StringBuilder lotNumber);

        [DllImport("DIP_proc.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void debug_lot_number(int[] f, int w, int h, int[] debugImg, ref int debugW, ref int debugH, ref int segCount, int[] segBoxes);

        private Bitmap IntArrayToBitmap(int[] data, int w, int h)
        {
            Bitmap bmp = new Bitmap(w, h);
            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                {
                    int v = data[y * w + x];
                    v = Math.Max(0, Math.Min(255, v));
                    bmp.SetPixel(x, y, Color.FromArgb(v, v, v));
                }
            return bmp;
        }

        Bitmap NpBitmap;
        int[] f;
        int[] g;
        int w, h;

        private void DIPSample_Load(object sender, EventArgs e)
        {
            this.IsMdiContainer = true;
            this.WindowState = FormWindowState.Maximized;
            this.stStripLabel.Text = "";
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            oFileDlg.CheckFileExists = true;
            oFileDlg.CheckPathExists = true;
            oFileDlg.Title = "Open File - DIP Sample";
            oFileDlg.ValidateNames = true;
            oFileDlg.Filter = "bmp files (*.bmp)|*.bmp";
            oFileDlg.FileName = "";

            if (oFileDlg.ShowDialog() == DialogResult.OK)
            {
                MSForm childForm = new MSForm();
                childForm.MdiParent = this;
                childForm.pf1 = stStripLabel;
                NpBitmap = bmp_read(oFileDlg);
                childForm.pBitmap = NpBitmap;
                w = NpBitmap.Width;
                h = NpBitmap.Height;
                childForm.Show();
            }
        }

        private Bitmap bmp_read(OpenFileDialog oFileDlg)
        {
            Bitmap pBitmap;
            string fileloc = oFileDlg.FileName;
            pBitmap = new Bitmap(fileloc);
            w = pBitmap.Width;
            h = pBitmap.Height;
            return pBitmap;
        }

        private int[] dyn_bmp2array(Bitmap myBitmap, ref int ByteDepth, ref PixelFormat pixelFormat, ref ColorPalette palette)
        {
            BitmapData byteArray = myBitmap.LockBits(new Rectangle(0, 0, myBitmap.Width, myBitmap.Height),
                                          ImageLockMode.ReadWrite,
                                          myBitmap.PixelFormat);
            pixelFormat = myBitmap.PixelFormat;
            palette = myBitmap.Palette;
            ByteDepth = (int)(byteArray.Stride / myBitmap.Width);
            int[] ImgData = new int[myBitmap.Width * myBitmap.Height * ByteDepth];
            int ByteOfSkip = byteArray.Stride - byteArray.Width * ByteDepth;
            unsafe
            {
                byte* imgPtr = (byte*)(byteArray.Scan0);
                for (int y = 0; y < byteArray.Height; y++)
                {
                    for (int x = 0; x < byteArray.Width; x++)
                    {
                        for (int c = 0; c < ByteDepth; c++)
                        {
                            ImgData[(x + byteArray.Height * y) * ByteDepth + c] = (int)*(imgPtr);
                            imgPtr += (int)(byteArray.Stride / myBitmap.Width) / ByteDepth;
                        }
                    }
                    imgPtr += ByteOfSkip;
                }
            }
            myBitmap.UnlockBits(byteArray);
            return ImgData;
        }

        private static Bitmap dyn_array2bmp(int[] ImgData, int ByteDepth, PixelFormat pixelFormat, ColorPalette palette)//灰階
        {
            int Width = (int)Math.Sqrt(ImgData.GetLength(0) / ByteDepth);
            int Height = (int)Math.Sqrt(ImgData.GetLength(0) / ByteDepth);
            Bitmap myBitmap = new Bitmap(Width, Height, pixelFormat);
            BitmapData byteArray = myBitmap.LockBits(new Rectangle(0, 0, Width, Height),
                                           ImageLockMode.WriteOnly,
                                           pixelFormat);
            try
            {
                myBitmap.Palette = palette;
            }
            catch
            {

            }
            //Padding bytes的長度
            int ByteOfSkip = byteArray.Stride - myBitmap.Width * ByteDepth;
            unsafe
            {                                   // 指標取出影像資料
                byte* imgPtr = (byte*)byteArray.Scan0;
                for (int y = 0; y < Height; y++)
                {
                    for (int x = 0; x < Width; x++)
                    {
                        for (int c = 0; c < ByteDepth; c++)
                        {
                            *imgPtr = (byte)ImgData[(x + Height * y) * ByteDepth + c];       //B
                            imgPtr += 1;
                        }
                    }
                    imgPtr += ByteOfSkip; // 跳過Padding bytes
                }
            }
            myBitmap.UnlockBits(byteArray);
            return myBitmap;
        }

        private void RGBtoGrayToolStripMenuItem_Click(object sender, EventArgs e)
        {
            int[] f;
            int[] g;
            foreach (MSForm cF in MdiChildren)
            {
                if (cF.Focused)
                {
                    int ByteDepth = 0;
                    PixelFormat pixelFormat = new PixelFormat();
                    ColorPalette palette = null;
                    f = dyn_bmp2array(cF.pBitmap, ref ByteDepth, ref pixelFormat, ref palette);
                    g = new int[w * h * ByteDepth];
                    unsafe
                    {
                        fixed (int* f0 = f) fixed (int* g0 = g)
                        {
                            for (int i = 0; i < h; i++)
                            {
                                for (int j = 0; j < w; j++)
                                {
                                    int idx = (i * w + j) * ByteDepth;

                                    // 取出 RGB 通道（BMP 預設順序：B、G、R）
                                    int b = f0[idx + 0];
                                    int gVal = f0[idx + 1];
                                    int r = f0[idx + 2];

                                    int gray = (int)(r * 0.299 + gVal * 0.587 + b * 0.114);

                                    g0[idx + 0] = gray;
                                    g0[idx + 1] = gray;
                                    g0[idx + 2] = gray;
                                }
                            }


                        }
                    }
                    NpBitmap = dyn_array2bmp(g, ByteDepth, pixelFormat, palette);
                    break;
                }
            }
            MSForm childForm = new MSForm();
            childForm.MdiParent = this;
            childForm.pf1 = stStripLabel;
            childForm.pBitmap = NpBitmap;
            childForm.Show();
        }

        private void fffToolStripMenuItem_Click(object sender, EventArgs e)
        {
            int[] f;
            int[] g;
            foreach (MSForm cF in MdiChildren)
            {
                if (cF.Focused)
                {
                    int ByteDepth = 0;
                    PixelFormat pixelFormat = new PixelFormat();
                    ColorPalette palette = null;
                    f = dyn_bmp2array(cF.pBitmap, ref ByteDepth, ref pixelFormat, ref palette);
                    g = new int[w * h * ByteDepth];
                    unsafe
                    {
                        fixed (int* f0 = f) fixed (int* g0 = g)
                        {



                        }
                    }
                    NpBitmap = dyn_array2bmp(g, ByteDepth, pixelFormat, palette);
                    break;
                }
            }
            MSForm childForm = new MSForm();
            childForm.MdiParent = this;
            childForm.pf1 = stStripLabel;
            childForm.pBitmap = NpBitmap;
            childForm.Show();
        }

        private void toolStripMenuItem2_Click(object sender, EventArgs e)
        {
            int[] f;
            int[] g;

            // 你要切第幾個位元？藏圖通常在 0
            int targetBit = 0;  // 0 ~ 7 都可以試

            foreach (MSForm cF in MdiChildren)
            {
                if (cF.Focused)
                {
                    int ByteDepth = 0;
                    PixelFormat pixelFormat = new PixelFormat();
                    ColorPalette palette = null;

                    // 讀灰階圖成陣列
                    f = dyn_bmp2array(cF.pBitmap, ref ByteDepth, ref pixelFormat, ref palette);

                    // 取得圖片大小
                    int w = cF.pBitmap.Width;
                    int h = cF.pBitmap.Height;

                    g = new int[w * h * ByteDepth];

                    unsafe
                    {
                        fixed (int* f0 = f) fixed (int* g0 = g)
                        {
                            for (int i = 0; i < h; i++)
                            {
                                for (int j = 0; j < w; j++)
                                {
                                    int idx = (i * w + j) * ByteDepth;

                                    // ===================== 重點修正 =====================
                                    // 灰階圖 → 直接取單一通道即可，不用轉灰階
                                    int gray = f0[idx];
                                    // ====================================================

                                    // 取出第 N 個位元
                                    int bitVal = (gray >> targetBit) & 1;
                                    int result = bitVal == 1 ? 255 : 0;

                                    // 灰階輸出：三個通道都給一樣的值
                                    g0[idx + 0] = result;
                                    g0[idx + 1] = result;
                                    g0[idx + 2] = result;
                                }
                            }
                        }
                    }

                    NpBitmap = dyn_array2bmp(g, ByteDepth, pixelFormat, palette);
                    break;
                }
            }

            // 開新視窗顯示位元切面結果
            MSForm childForm = new MSForm();
            childForm.MdiParent = this;
            childForm.pf1 = stStripLabel;
            childForm.pBitmap = NpBitmap;
            childForm.Show();
        }

        private void toolStripMenuItem3_Click(object sender, EventArgs e)
        {
            int[] f;
            int[] g;
            foreach (MSForm cF in MdiChildren)
            {
                if (cF.Focused)
                {
                    int ByteDepth = 0;
                    PixelFormat pixelFormat = new PixelFormat();
                    ColorPalette palette = null;
                    f = dyn_bmp2array(cF.pBitmap, ref ByteDepth, ref pixelFormat, ref palette);
                    g = new int[w * h * ByteDepth];
                    unsafe
                    {
                        fixed (int* f0 = f) fixed (int* g0 = g)
                        {
                            for (int i = 0; i < h; i++)
                            {
                                for (int j = 0; j < w; j++)
                                {
                                    int idx = (i * w + j) * ByteDepth;
                                    int b = f0[idx + 0];
                                    int gVal = f0[idx + 1];
                                    int r = f0[idx + 2];
                                    int gray = (int)(r * 0.299 + gVal * 0.587 + b * 0.114);
                                    int bit = (gray >> 1) & 1;
                                    int result = bit == 1 ? 255 : 0;
                                    g0[idx + 0] = result;
                                    g0[idx + 1] = result;
                                    g0[idx + 2] = result;

                                }
                            }
                        }
                    }
                    NpBitmap = dyn_array2bmp(g, ByteDepth, pixelFormat, palette);
                    break;
                }
            }
            MSForm childForm = new MSForm();
            childForm.MdiParent = this;
            childForm.pf1 = stStripLabel;
            childForm.pBitmap = NpBitmap;
            childForm.Show();
        }

        private void toolStripMenuItem4_Click(object sender, EventArgs e)
        {
            int[] f;
            int[] g;
            foreach (MSForm cF in MdiChildren)
            {
                if (cF.Focused)
                {
                    int ByteDepth = 0;
                    PixelFormat pixelFormat = new PixelFormat();
                    ColorPalette palette = null;
                    f = dyn_bmp2array(cF.pBitmap, ref ByteDepth, ref pixelFormat, ref palette);
                    g = new int[w * h * ByteDepth];
                    unsafe
                    {
                        fixed (int* f0 = f) fixed (int* g0 = g)
                        {
                            for (int i = 0; i < h; i++)
                            {
                                for (int j = 0; j < w; j++)
                                {
                                    int idx = (i * w + j) * ByteDepth;
                                    int b = f0[idx + 0];
                                    int gVal = f0[idx + 1];
                                    int r = f0[idx + 2];
                                    int gray = (int)(r * 0.299 + gVal * 0.587 + b * 0.114);
                                    int bit = (gray >> 2) & 1;
                                    int result = bit == 1 ? 255 : 0;
                                    g0[idx + 0] = result;
                                    g0[idx + 1] = result;
                                    g0[idx + 2] = result;

                                }
                            }
                        }
                    }
                    NpBitmap = dyn_array2bmp(g, ByteDepth, pixelFormat, palette);
                    break;
                }
            }
            MSForm childForm = new MSForm();
            childForm.MdiParent = this;
            childForm.pf1 = stStripLabel;
            childForm.pBitmap = NpBitmap;
            childForm.Show();
        }

        private void toolStripMenuItem5_Click(object sender, EventArgs e)
        {
            int[] f;
            int[] g;
            foreach (MSForm cF in MdiChildren)
            {
                if (cF.Focused)
                {
                    int ByteDepth = 0;
                    PixelFormat pixelFormat = new PixelFormat();
                    ColorPalette palette = null;
                    f = dyn_bmp2array(cF.pBitmap, ref ByteDepth, ref pixelFormat, ref palette);
                    g = new int[w * h * ByteDepth];
                    unsafe
                    {
                        fixed (int* f0 = f) fixed (int* g0 = g)
                        {
                            for (int i = 0; i < h; i++)
                            {
                                for (int j = 0; j < w; j++)
                                {
                                    int idx = (i * w + j) * ByteDepth;
                                    int b = f0[idx + 0];
                                    int gVal = f0[idx + 1];
                                    int r = f0[idx + 2];
                                    int gray = (int)(r * 0.299 + gVal * 0.587 + b * 0.114);
                                    int bit = (gray >> 3) & 1;
                                    int result = bit == 1 ? 255 : 0;
                                    g0[idx + 0] = result;
                                    g0[idx + 1] = result;
                                    g0[idx + 2] = result;

                                }
                            }
                        }
                    }
                    NpBitmap = dyn_array2bmp(g, ByteDepth, pixelFormat, palette);
                    break;
                }
            }
            MSForm childForm = new MSForm();
            childForm.MdiParent = this;
            childForm.pf1 = stStripLabel;
            childForm.pBitmap = NpBitmap;
            childForm.Show();
        }

        private void toolStripMenuItem6_Click(object sender, EventArgs e)
        {
            int[] f;
            int[] g;
            foreach (MSForm cF in MdiChildren)
            {
                if (cF.Focused)
                {
                    int ByteDepth = 0;
                    PixelFormat pixelFormat = new PixelFormat();
                    ColorPalette palette = null;
                    f = dyn_bmp2array(cF.pBitmap, ref ByteDepth, ref pixelFormat, ref palette);
                    g = new int[w * h * ByteDepth];
                    unsafe
                    {
                        fixed (int* f0 = f) fixed (int* g0 = g)
                        {
                            for (int i = 0; i < h; i++)
                            {
                                for (int j = 0; j < w; j++)
                                {
                                    int idx = (i * w + j) * ByteDepth;
                                    int b = f0[idx + 0];
                                    int gVal = f0[idx + 1];
                                    int r = f0[idx + 2];
                                    int gray = (int)(r * 0.299 + gVal * 0.587 + b * 0.114);
                                    int bit = (gray >> 4) & 1;
                                    int result = bit == 1 ? 255 : 0;
                                    g0[idx + 0] = result;
                                    g0[idx + 1] = result;
                                    g0[idx + 2] = result;

                                }
                            }
                        }
                    }
                    NpBitmap = dyn_array2bmp(g, ByteDepth, pixelFormat, palette);
                    break;
                }
            }
            MSForm childForm = new MSForm();
            childForm.MdiParent = this;
            childForm.pf1 = stStripLabel;
            childForm.pBitmap = NpBitmap;
            childForm.Show();
        }

        private void toolStripMenuItem7_Click(object sender, EventArgs e)
        {
            int[] f;
            int[] g;
            foreach (MSForm cF in MdiChildren)
            {
                if (cF.Focused)
                {
                    int ByteDepth = 0;
                    PixelFormat pixelFormat = new PixelFormat();
                    ColorPalette palette = null;
                    f = dyn_bmp2array(cF.pBitmap, ref ByteDepth, ref pixelFormat, ref palette);
                    g = new int[w * h * ByteDepth];
                    unsafe
                    {
                        fixed (int* f0 = f) fixed (int* g0 = g)
                        {
                            for (int i = 0; i < h; i++)
                            {
                                for (int j = 0; j < w; j++)
                                {
                                    int idx = (i * w + j) * ByteDepth;
                                    int b = f0[idx + 0];
                                    int gVal = f0[idx + 1];
                                    int r = f0[idx + 2];
                                    int gray = (int)(r * 0.299 + gVal * 0.587 + b * 0.114);
                                    int bit = (gray >> 5) & 1;
                                    int result = bit == 1 ? 255 : 0;
                                    g0[idx + 0] = result;
                                    g0[idx + 1] = result;
                                    g0[idx + 2] = result;

                                }
                            }
                        }
                    }
                    NpBitmap = dyn_array2bmp(g, ByteDepth, pixelFormat, palette);
                    break;
                }
            }
            MSForm childForm = new MSForm();
            childForm.MdiParent = this;
            childForm.pf1 = stStripLabel;
            childForm.pBitmap = NpBitmap;
            childForm.Show();
        }

        private void toolStripMenuItem8_Click(object sender, EventArgs e)
        {
            int[] f;
            int[] g;
            foreach (MSForm cF in MdiChildren)
            {
                if (cF.Focused)
                {
                    int ByteDepth = 0;
                    PixelFormat pixelFormat = new PixelFormat();
                    ColorPalette palette = null;
                    f = dyn_bmp2array(cF.pBitmap, ref ByteDepth, ref pixelFormat, ref palette);
                    g = new int[w * h * ByteDepth];
                    unsafe
                    {
                        fixed (int* f0 = f) fixed (int* g0 = g)
                        {
                            for (int i = 0; i < h; i++)
                            {
                                for (int j = 0; j < w; j++)
                                {
                                    int idx = (i * w + j) * ByteDepth;
                                    int b = f0[idx + 0];
                                    int gVal = f0[idx + 1];
                                    int r = f0[idx + 2];
                                    int gray = (int)(r * 0.299 + gVal * 0.587 + b * 0.114);
                                    int bit = (gray >> 6) & 1;
                                    int result = bit == 1 ? 255 : 0;
                                    g0[idx + 0] = result;
                                    g0[idx + 1] = result;
                                    g0[idx + 2] = result;

                                }
                            }
                        }
                    }
                    NpBitmap = dyn_array2bmp(g, ByteDepth, pixelFormat, palette);
                    break;
                }
            }
            MSForm childForm = new MSForm();
            childForm.MdiParent = this;
            childForm.pf1 = stStripLabel;
            childForm.pBitmap = NpBitmap;
            childForm.Show();
        }

        private void toolStripMenuItem9_Click(object sender, EventArgs e)
        {
            int[] f;
            int[] g;
            foreach (MSForm cF in MdiChildren)
            {
                if (cF.Focused)
                {
                    int ByteDepth = 0;
                    PixelFormat pixelFormat = new PixelFormat();
                    ColorPalette palette = null;
                    f = dyn_bmp2array(cF.pBitmap, ref ByteDepth, ref pixelFormat, ref palette);
                    g = new int[w * h * ByteDepth];
                    unsafe
                    {
                        fixed (int* f0 = f) fixed (int* g0 = g)
                        {
                            for (int i = 0; i < h; i++)
                            {
                                for (int j = 0; j < w; j++)
                                {
                                    int idx = (i * w + j) * ByteDepth;
                                    int b = f0[idx + 0];
                                    int gVal = f0[idx + 1];
                                    int r = f0[idx + 2];
                                    int gray = (int)(r * 0.299 + gVal * 0.587 + b * 0.114);
                                    int bit = (gray >> 7) & 1;
                                    int result = bit == 1 ? 255 : 0;
                                    g0[idx + 0] = result;
                                    g0[idx + 1] = result;
                                    g0[idx + 2] = result;

                                }
                            }
                        }
                    }
                    NpBitmap = dyn_array2bmp(g, ByteDepth, pixelFormat, palette);
                    break;
                }
            }
            MSForm childForm = new MSForm();
            childForm.MdiParent = this;
            childForm.pf1 = stStripLabel;
            childForm.pBitmap = NpBitmap;
            childForm.Show();
        }

        private void iPToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

        private void hggggToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MSForm currentForm = ActiveMdiChild as MSForm;

            if (currentForm == null) return;

            BrightFrom f = new BrightFrom();

            f.BrightImage = currentForm.pBitmap;

            f.Show();
        }

        private void contrastToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MSForm currentForm = ActiveMdiChild as MSForm;

            if (currentForm == null) return;

            Contrast f = new Contrast();

            f.ContrastImage = currentForm.pBitmap;

            f.Show();
        }

        private void 直圖圖轉換與直圖等化ToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

        private void transformationToolStripMenuItem_Click(object sender, EventArgs e)
        {
            PictureBox pb = GetActivePictureBox();
            if (pb == null || pb.Image == null) return;

            Form5 f = new Form5((Bitmap)pb.Image);
            f.MdiParent = this;
            f.Show();
        }
        private void equalizationToolStripMenuItem_Click(object sender, EventArgs e)
        {
            PictureBox pb = GetActivePictureBox();
            if (pb == null || pb.Image == null) return;

            Bitmap src = (Bitmap)pb.Image;
            Bitmap dst = new Bitmap(src.Width, src.Height);

            int[] histR = new int[256];
            int[] histG = new int[256];
            int[] histB = new int[256];

            for (int y = 0; y < src.Height; y++)
            {
                for (int x = 0; x < src.Width; x++)
                {
                    Color c = src.GetPixel(x, y);
                    histR[c.R]++;
                    histG[c.G]++;
                    histB[c.B]++;
                }
            }

            int[] cdfR = new int[256];
            int[] cdfG = new int[256];
            int[] cdfB = new int[256];

            cdfR[0] = histR[0];
            cdfG[0] = histG[0];
            cdfB[0] = histB[0];

            for (int i = 1; i < 256; i++)
            {
                cdfR[i] = cdfR[i - 1] + histR[i];
                cdfG[i] = cdfG[i - 1] + histG[i];
                cdfB[i] = cdfB[i - 1] + histB[i];
            }

            int total = src.Width * src.Height;

            for (int y = 0; y < src.Height; y++)
            {
                for (int x = 0; x < src.Width; x++)
                {
                    Color c = src.GetPixel(x, y);

                    int r = cdfR[c.R] * 255 / total;
                    int g = cdfG[c.G] * 255 / total;
                    int b = cdfB[c.B] * 255 / total;

                    r = Math.Max(0, Math.Min(255, r));
                    g = Math.Max(0, Math.Min(255, g));
                    b = Math.Max(0, Math.Min(255, b));

                    dst.SetPixel(x, y, Color.FromArgb(r, g, b));
                }
            }

            ShowImage(dst);
        }
        private void ShowImage(Bitmap img)
        {
            Form newForm = new Form();
            PictureBox pb = new PictureBox();

            pb.Image = img;
            pb.SizeMode = PictureBoxSizeMode.AutoSize;
            pb.Location = new Point(0, 0);

            newForm.Controls.Add(pb);
            newForm.ClientSize = new Size(img.Width + 4, img.Height + 4);
            newForm.MdiParent = this;
            newForm.Show();
        }
        private PictureBox GetActivePictureBox()
        {
            Form active = this.ActiveMdiChild;

            if (active == null)
            {
                MessageBox.Show("沒有開啟圖片視窗");
                return null;
            }

            if (active.Controls.Count == 0)
            {
                MessageBox.Show("視窗沒有圖片");
                return null;
            }

            PictureBox pb = active.Controls[0] as PictureBox;

            if (pb == null)
            {
                MessageBox.Show("找不到 PictureBox");
                return null;
            }

            return pb;
        }
        private void fileToolStripMenuItem_Click(object sender, EventArgs e)
        {
        }

        private void filterToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MSForm current = ActiveMdiChild as MSForm;
            if (current == null) return;
            FilterForm f = new FilterForm();
            f.SourceImage = current.pBitmap;
            f.Show();
        }

        private void resizeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MSForm current = ActiveMdiChild as MSForm;
            if (current == null) return;
            ResizeForm f = new ResizeForm();
            f.SourceImage = current.pBitmap;
            if (f.ShowDialog() == DialogResult.OK && f.ResultImage != null)
                ShowImage(f.ResultImage);
        }

        private void rotateToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MSForm current = ActiveMdiChild as MSForm;
            if (current == null) return;
            RotateForm f = new RotateForm();
            f.SourceImage = current.pBitmap;
            f.Show();
        }

        private void otsuToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MSForm current = ActiveMdiChild as MSForm;
            if (current == null) return;

            Bitmap src = new Bitmap(current.pBitmap);
            int w = src.Width, h = src.Height;
            int[] hist = new int[256];
            int total = w * h;

            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                {
                    Color c = src.GetPixel(x, y);
                    int gray = (int)(c.R * 0.299 + c.G * 0.587 + c.B * 0.114);
                    hist[gray]++;
                }

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

            MessageBox.Show("Otsu threshold = " + threshold, "Threshold");
            Bitmap dst = new Bitmap(w, h);
            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                {
                    Color c = src.GetPixel(x, y);
                    int gray = (int)(c.R * 0.299 + c.G * 0.587 + c.B * 0.114);
                    int val = (gray >= threshold) ? 255 : 0;
                    dst.SetPixel(x, y, Color.FromArgb(val, val, val));
                }

            ShowImage(dst);
        }

        private void lineDetectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MSForm current = ActiveMdiChild as MSForm;
            if (current == null) return;
            LineDetectForm f = new LineDetectForm();
            f.SourceImage = current.pBitmap;
            f.Show();
        }

        private void metalCharToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MSForm current = ActiveMdiChild as MSForm;
            if (current == null) { MessageBox.Show("請先開啟圖片"); return; }

            Bitmap src = current.pBitmap;
            int w = src.Width, h = src.Height;

            // ROI 計算（與 lot_number.h 的 getLotROI 一致）
            int roiX = (int)(w * 0.20);
            int roiY = (int)(h * 0.40);
            int roiW = (int)(w * 0.65);
            int roiH = (int)(h * 0.40);
            if (roiX + roiW > w) roiW = w - roiX;
            if (roiY + roiH > h) roiH = h - roiY;
            if (roiW < 1) roiW = 1;
            if (roiH < 1) roiH = 1;

            int[] gray = new int[w * h];
            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                {
                    Color c = src.GetPixel(x, y);
                    gray[y * w + x] = (int)(c.R * 0.299 + c.G * 0.587 + c.B * 0.114);
                }

            // === 除錯：取得前處理後的二值圖 + 區段數 ===
            int debugW = 0, debugH = 0, segCount = 0;
            int[] debugImg = new int[w * h * 9]; // 最大 3w*3h
            int[] segBoxes = new int[8 * 4];
            debug_lot_number(gray, w, h, debugImg, ref debugW, ref debugH, ref segCount, segBoxes);

            // 顯示除錯二值圖（在 ROI 範圍內）
            if (debugW > 0 && debugH > 0)
            {
                Bitmap debugBmp = IntArrayToBitmap(debugImg, debugW, debugH);
                MSForm df = new MSForm();
                df.MdiParent = this;
                df.pf1 = stStripLabel;
                df.pBitmap = debugBmp;
                df.Text = "前處理二值圖 - 區段數: " + segCount;
                df.Show();
            }

            // === 使用 Python PaddleOCR 進行辨識 ===
            string result = "ERROR";
            string tempImg = Path.GetTempFileName() + ".bmp";
            try
            {
                src.Save(tempImg, ImageFormat.Bmp);
                // 向上搜尋 DIP_proc\metal_ocr.py（支援 AnyCPU / x86 不同輸出目錄深度）
                string scriptPath = null;
                DirectoryInfo di = new DirectoryInfo(Application.StartupPath);
                while (di != null)
                {
                    string test = Path.Combine(di.FullName, "DIP_proc", "metal_ocr.py");
                    if (File.Exists(test)) { scriptPath = test; break; }
                    di = di.Parent;
                }
                if (scriptPath == null) { result = "ERR:找不到 metal_ocr.py"; return; }
                string pythonExe = @"D:\Python\python.exe";
                if (!File.Exists(pythonExe)) pythonExe = "python";
                ProcessStartInfo psi = new ProcessStartInfo
                {
                    FileName = pythonExe,
                    Arguments = $"\"{scriptPath}\" \"{tempImg}\"",
                    UseShellExecute = false,
                    RedirectStandardOutput = true,
                    CreateNoWindow = true,
                    StandardOutputEncoding = Encoding.UTF8
                };
                using (Process p = Process.Start(psi))
                {
                    string output = p.StandardOutput.ReadToEnd().Trim();
                    p.WaitForExit();
                    if (!string.IsNullOrEmpty(output))
                        result = output;
                }
            }
            catch (Exception ex)
            {
                result = "ERR:" + ex.Message;
            }
            finally
            {
                if (File.Exists(tempImg)) File.Delete(tempImg);
            }

            // === 在原圖上畫紅框 + 標註文字 ===
            Bitmap annotated = new Bitmap(src);
            using (Graphics g = Graphics.FromImage(annotated))
            {
                g.DrawRectangle(new Pen(Color.Red, 3), roiX, roiY, roiW, roiH);
                g.DrawString("Lot: " + result, new Font("Arial", 20, FontStyle.Bold),
                    new SolidBrush(Color.Lime), roiX, Math.Max(0, roiY - 30));
            }

            MSForm af = new MSForm();
            af.MdiParent = this;
            af.pf1 = stStripLabel;
            af.pBitmap = annotated;
            af.Text = "金屬字元結果 - " + result;
            af.Show();

            MessageBox.Show("辨識結果: " + result, "金屬字元辨識");
        }
    }
}