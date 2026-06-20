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
    public partial class BrightFrom : Form
    {
        internal Bitmap BrightImage;

        public BrightFrom()
        {
            InitializeComponent();
        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {

        }

        private void BrightFrom_Load(object sender, EventArgs e)
        {
            if (BrightImage != null)
            {
                pictureBox1.Image = BrightImage;
            }
        }

        private byte Clamp(int value)
        {
            if (value < 0) return 0;
            if (value > 255) return 255;
            return (byte)value;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (BrightImage == null) return;

            Bitmap src = new Bitmap(BrightImage);
            Bitmap output = new Bitmap(src.Width, src.Height);

            // trackBar1: -155 ~ 355
            //   -155 → delta = -255 (all black)
            //    100 → delta = 0   (original)
            //    355 → delta = 255 (all white)
            int delta = trackBar1.Value - 100;

            for (int y = 0; y < src.Height; y++)
            {
                for (int x = 0; x < src.Width; x++)
                {
                    Color c = src.GetPixel(x, y);

                    
                    int r = Clamp(c.R + delta);
                    int g = Clamp(c.G + delta);
                    int b = Clamp(c.B + delta);

                    output.SetPixel(x, y, Color.FromArgb(r, g, b));
                }
            }

            pictureBox2.Image = output;
        }


    }
}
