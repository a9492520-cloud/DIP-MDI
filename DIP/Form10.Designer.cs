namespace DIP
{
    partial class EdgeDefectForm
    {
        private System.ComponentModel.IContainer components = null;
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null)) components.Dispose();
            base.Dispose(disposing);
        }

        private void InitializeComponent()
        {
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.pictureBox2 = new System.Windows.Forms.PictureBox();
            this.pictureBox3 = new System.Windows.Forms.PictureBox();
            this.pictureBox4 = new System.Windows.Forms.PictureBox();
            this.pictureBox5 = new System.Windows.Forms.PictureBox();
            this.btnDetect = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.thresholdTrackBar = new System.Windows.Forms.TrackBar();
            this.lblThreshold = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox3)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox4)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox5)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.thresholdTrackBar)).BeginInit();
            this.SuspendLayout();

            this.pictureBox1.Location = new System.Drawing.Point(12, 28);
            this.pictureBox1.Size = new System.Drawing.Size(280, 210);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pictureBox1.TabIndex = 0;
            this.pictureBox1.TabStop = false;

            this.pictureBox2.Location = new System.Drawing.Point(308, 28);
            this.pictureBox2.Size = new System.Drawing.Size(280, 210);
            this.pictureBox2.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pictureBox2.TabIndex = 1;
            this.pictureBox2.TabStop = false;

            this.pictureBox3.Location = new System.Drawing.Point(12, 278);
            this.pictureBox3.Size = new System.Drawing.Size(190, 140);
            this.pictureBox3.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pictureBox3.TabIndex = 2;
            this.pictureBox3.TabStop = false;

            this.pictureBox4.Location = new System.Drawing.Point(208, 278);
            this.pictureBox4.Size = new System.Drawing.Size(190, 140);
            this.pictureBox4.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pictureBox4.TabIndex = 3;
            this.pictureBox4.TabStop = false;

            this.pictureBox5.Location = new System.Drawing.Point(404, 278);
            this.pictureBox5.Size = new System.Drawing.Size(190, 140);
            this.pictureBox5.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pictureBox5.TabIndex = 4;
            this.pictureBox5.TabStop = false;

            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("微軟正黑體", 9F);
            this.label1.Location = new System.Drawing.Point(12, 10);
            this.label1.Size = new System.Drawing.Size(62, 16);
            this.label1.Text = "1. Original";

            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("微軟正黑體", 9F);
            this.label2.Location = new System.Drawing.Point(308, 10);
            this.label2.Size = new System.Drawing.Size(95, 16);
            this.label2.Text = "5. Final Result";

            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("微軟正黑體", 9F);
            this.label3.Location = new System.Drawing.Point(12, 260);
            this.label3.Size = new System.Drawing.Size(107, 16);
            this.label3.Text = "2. Binary Threshold";

            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("微軟正黑體", 9F);
            this.label4.Location = new System.Drawing.Point(208, 260);
            this.label4.Size = new System.Drawing.Size(93, 16);
            this.label4.Text = "3. Closing Result";

            this.label5.AutoSize = true;
            this.label5.Font = new System.Drawing.Font("微軟正黑體", 9F);
            this.label5.Location = new System.Drawing.Point(404, 260);
            this.label5.Size = new System.Drawing.Size(112, 16);
            this.label5.Text = "4. Defect Region";

            this.btnDetect.Font = new System.Drawing.Font("微軟正黑體", 12F);
            this.btnDetect.Location = new System.Drawing.Point(232, 430);
            this.btnDetect.Size = new System.Drawing.Size(140, 36);
            this.btnDetect.Text = "邊緣缺陷偵測";
            this.btnDetect.UseVisualStyleBackColor = true;
            this.btnDetect.Click += new System.EventHandler(this.btnDetect_Click);

            this.thresholdTrackBar.Location = new System.Drawing.Point(12, 472);
            this.thresholdTrackBar.Minimum = 0;
            this.thresholdTrackBar.Maximum = 255;
            this.thresholdTrackBar.TickFrequency = 16;
            this.thresholdTrackBar.Size = new System.Drawing.Size(500, 45);
            this.thresholdTrackBar.TabIndex = 5;
            this.thresholdTrackBar.Enabled = false;
            this.thresholdTrackBar.Scroll += new System.EventHandler(this.thresholdTrackBar_Scroll);

            this.lblThreshold.AutoSize = true;
            this.lblThreshold.Font = new System.Drawing.Font("微軟正黑體", 10F);
            this.lblThreshold.Location = new System.Drawing.Point(520, 476);
            this.lblThreshold.Size = new System.Drawing.Size(80, 17);
            this.lblThreshold.Text = "門檻值: 0";

            this.kernelTrackBar = new System.Windows.Forms.TrackBar();
            this.lblKernel = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.kernelTrackBar)).BeginInit();

            this.kernelTrackBar.Location = new System.Drawing.Point(12, 510);
            this.kernelTrackBar.Minimum = 1;
            this.kernelTrackBar.Maximum = 200;
            this.kernelTrackBar.Value = 120;
            this.kernelTrackBar.TickFrequency = 10;
            this.kernelTrackBar.Size = new System.Drawing.Size(500, 45);
            this.kernelTrackBar.TabIndex = 6;
            this.kernelTrackBar.Scroll += new System.EventHandler(this.kernelTrackBar_Scroll);

            this.lblKernel.AutoSize = true;
            this.lblKernel.Font = new System.Drawing.Font("微軟正黑體", 10F);
            this.lblKernel.Location = new System.Drawing.Point(520, 514);
            this.lblKernel.Size = new System.Drawing.Size(85, 17);
            this.lblKernel.Text = "核心半徑: 120";

            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(614, 560);
            this.Controls.Add(this.thresholdTrackBar);
            this.Controls.Add(this.lblThreshold);
            this.Controls.Add(this.kernelTrackBar);
            this.Controls.Add(this.lblKernel);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.pictureBox2);
            this.Controls.Add(this.pictureBox3);
            this.Controls.Add(this.pictureBox4);
            this.Controls.Add(this.pictureBox5);
            this.Controls.Add(this.btnDetect);
            this.Name = "EdgeDefectForm";
            this.Text = "邊緣缺陷檢測";
            this.Load += new System.EventHandler(this.EdgeDefectForm_Load);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox3)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox4)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox5)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.thresholdTrackBar)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.kernelTrackBar)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();
        }

        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.PictureBox pictureBox2;
        private System.Windows.Forms.PictureBox pictureBox3;
        private System.Windows.Forms.PictureBox pictureBox4;
        private System.Windows.Forms.PictureBox pictureBox5;
        private System.Windows.Forms.Button btnDetect;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TrackBar thresholdTrackBar;
        private System.Windows.Forms.Label lblThreshold;
        private System.Windows.Forms.TrackBar kernelTrackBar;
        private System.Windows.Forms.Label lblKernel;
    }
}
