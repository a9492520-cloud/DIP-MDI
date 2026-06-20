using System;
using System.Drawing;
using System.Windows.Forms;
using System.Windows.Forms.DataVisualization.Charting;

namespace DIP
{
    public partial class Form5 : Form
    {
        Bitmap originalImage;

        public Form5(Bitmap img)
        {
            InitializeComponent();

            originalImage = new Bitmap(img);
            pictureBox1.Image = new Bitmap(img);

            
        }

        private void Form5_Load(object sender, EventArgs e)
        {
            if (originalImage != null)
            {
                DrawHistogram(originalImage);
            }
        }
        private void DrawHistogram(Bitmap bmp)
        {
            // 1. 統計 0~255 灰階數量
            int[] hist = new int[256];

            for (int y = 0; y < bmp.Height; y++)
            {
                for (int x = 0; x < bmp.Width; x++)
                {
                    Color c = bmp.GetPixel(x, y);
                    int gray = c.R; // 灰階圖 R=G=B
                    hist[gray]++;
                }
            }

            // 2. 清空 chart1
            chart1.Series.Clear();
            chart1.Titles.Clear();
            chart1.Titles.Add("灰階直方圖");

            // 3. 建立直方圖樣式
            Series series = new Series("灰階分布");
            series.ChartType = SeriesChartType.Column;
            series.Color = Color.Black;
            series.BorderWidth = 0;

            // 4. 把 0~255 丟進去
            for (int i = 0; i < 256; i++)
            {
                series.Points.AddXY(i, hist[i]);
            }

            // 5. 顯示
            chart1.Series.Add(series);
            chart1.ChartAreas[0].AxisX.Minimum = 0;
            chart1.ChartAreas[0].AxisX.Maximum = 255;
            chart1.ChartAreas[0].AxisX.Title = "灰階值 (0~255)";
            chart1.ChartAreas[0].AxisY.Title = "像素數量";
        }


        private void btnChange_Click(object sender, EventArgs e)
        {
            DIPSample main = (DIPSample)this.MdiParent;

            if (main.ActiveMdiChild == null) return;

            PictureBox pb = main.ActiveMdiChild.Controls[0] as PictureBox;

            if (pb != null)
            {
                pb.Image = new Bitmap(pictureBox1.Image);
            }

            this.Close();
        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {

        }

        
    }
}