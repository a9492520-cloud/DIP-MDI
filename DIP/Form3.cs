using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Imaging;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace DIP
{
    public partial class Contrast : Form
    {
        internal Bitmap ContrastImage;

        public Contrast()
        {
            InitializeComponent();
        }

        private void Contrast_Load(object sender, EventArgs e)
        {
            if (ContrastImage != null)
            {
                pictureBox1.Image = ContrastImage;
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (ContrastImage == null) return;

            int tb = trackBar1.Value;

            // 對比係數：0=全灰，128=原圖，255=極限黑白
            float factor;
            if (tb <= 128)
            {
                // 0 ~ 128 → 從全灰慢慢恢復原圖
                factor = tb / 128f;
            }
            else
            {
                // 128 ~ 255 → 從原圖拉到極限黑白
                factor = 1.0f + ((tb - 128) / 127f) * 10.0f;
            }

            Bitmap output = new Bitmap(ContrastImage.Width, ContrastImage.Height);

            unsafe
            {
                Rectangle rect = new Rectangle(0, 0, ContrastImage.Width, ContrastImage.Height);
                BitmapData srcData = ContrastImage.LockBits(rect, ImageLockMode.ReadOnly, ContrastImage.PixelFormat);
                byte* src = (byte*)srcData.Scan0;

                BitmapData dstData = output.LockBits(rect, ImageLockMode.WriteOnly, ContrastImage.PixelFormat);
                byte* dst = (byte*)dstData.Scan0;

                int stride = srcData.Stride;
                int bytesPerPixel = Image.GetPixelFormatSize(ContrastImage.PixelFormat) / 8;
                int h = ContrastImage.Height;
                int w = ContrastImage.Width;

                for (int y = 0; y < h; y++)
                {
                    for (int x = 0; x < w; x++)
                    {
                        int idx = y * stride + x * bytesPerPixel;

                        int b = src[idx + 0];
                        int g = src[idx + 1];
                        int r = src[idx + 2];

                        // 標準對比公式
                        b = Clamp((int)(128 + (b - 128) * factor));
                        g = Clamp((int)(128 + (g - 128) * factor));
                        r = Clamp((int)(128 + (r - 128) * factor));

                        dst[idx + 0] = (byte)b;
                        dst[idx + 1] = (byte)g;
                        dst[idx + 2] = (byte)r;

                        if (bytesPerPixel >= 4)
                            dst[idx + 3] = src[idx + 3];
                    }
                }

                ContrastImage.UnlockBits(srcData);
                output.UnlockBits(dstData);
            }



            pictureBox2.Image = output;
        }
        private byte Clamp(int value)
        {
            if (value < 0) return 0;
            if (value > 255) return 255;
            return (byte)value;
        }
    }
}
