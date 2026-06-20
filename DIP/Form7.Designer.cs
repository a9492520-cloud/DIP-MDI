namespace DIP
{
    partial class ResizeForm
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
            this.label1 = new System.Windows.Forms.Label();
            this.numericUpDown1 = new System.Windows.Forms.NumericUpDown();
            this.button1 = new System.Windows.Forms.Button();
            this.radioButton1 = new System.Windows.Forms.RadioButton();
            this.radioButton2 = new System.Windows.Forms.RadioButton();
            this.radioButton3 = new System.Windows.Forms.RadioButton();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).BeginInit();
            this.SuspendLayout();

            this.pictureBox1.Location = new System.Drawing.Point(12, 12);
            this.pictureBox1.Size = new System.Drawing.Size(350, 350);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pictureBox1.TabIndex = 0;
            this.pictureBox1.TabStop = false;

            this.pictureBox2.Location = new System.Drawing.Point(420, 12);
            this.pictureBox2.Size = new System.Drawing.Size(350, 350);
            this.pictureBox2.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pictureBox2.TabIndex = 1;
            this.pictureBox2.TabStop = false;

            this.label1.Text = "Scale Factor:";
            this.label1.Location = new System.Drawing.Point(30, 380);
            this.label1.Size = new System.Drawing.Size(90, 23);

            this.numericUpDown1.DecimalPlaces = 1;
            this.numericUpDown1.Increment = 0.1m;
            this.numericUpDown1.Location = new System.Drawing.Point(120, 378);
            this.numericUpDown1.Minimum = 0.1m;
            this.numericUpDown1.Maximum = 10m;
            this.numericUpDown1.Value = 1.0m;
            this.numericUpDown1.Width = 80;

            this.radioButton1.Text = "Nearest";
            this.radioButton1.Location = new System.Drawing.Point(30, 415);
            this.radioButton1.Size = new System.Drawing.Size(90, 25);

            this.radioButton2.Text = "Bilinear";
            this.radioButton2.Location = new System.Drawing.Point(130, 415);
            this.radioButton2.Size = new System.Drawing.Size(90, 25);
            this.radioButton2.Checked = true;

            this.radioButton3.Text = "Bicubic";
            this.radioButton3.Location = new System.Drawing.Point(230, 415);
            this.radioButton3.Size = new System.Drawing.Size(90, 25);

            this.button1.Font = new System.Drawing.Font("微軟正黑體", 14F);
            this.button1.Location = new System.Drawing.Point(350, 415);
            this.button1.Size = new System.Drawing.Size(120, 40);
            this.button1.Text = "Apply";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);

            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(782, 483);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.pictureBox2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.numericUpDown1);
            this.Controls.Add(this.radioButton1);
            this.Controls.Add(this.radioButton2);
            this.Controls.Add(this.radioButton3);
            this.Controls.Add(this.button1);
            this.Name = "ResizeForm";
            this.Text = "放大縮小";
            this.Load += new System.EventHandler(this.ResizeForm_Load);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();
        }

        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.PictureBox pictureBox2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.NumericUpDown numericUpDown1;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.RadioButton radioButton1;
        private System.Windows.Forms.RadioButton radioButton2;
        private System.Windows.Forms.RadioButton radioButton3;
    }
}
