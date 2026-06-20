namespace DIP
{
    partial class DIPSample
    {
        private System.ComponentModel.IContainer components = null;
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null)) components.Dispose();
            base.Dispose(disposing);
        }

        private void InitializeComponent()
        {
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.stStripLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.iPToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.rGBtoGrayToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fffToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem3 = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem4 = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem5 = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem6 = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem7 = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem8 = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem9 = new System.Windows.Forms.ToolStripMenuItem();
            this.gggToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.hggggToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.contrastToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.直圖圖轉換與直圖等化ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.transformationToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.equalizationToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.filterToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.resizeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.rotateToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.otsuToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.lineDetectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.metalCharToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.oFileDlg = new System.Windows.Forms.OpenFileDialog();
            this.saveFileDialog1 = new System.Windows.Forms.SaveFileDialog();
            this.statusStrip1.SuspendLayout();
            this.menuStrip1.SuspendLayout();
            this.SuspendLayout();

            this.statusStrip1.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] { this.stStripLabel });
            this.statusStrip1.Location = new System.Drawing.Point(0, 357);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Padding = new System.Windows.Forms.Padding(2, 0, 14, 0);
            this.statusStrip1.Size = new System.Drawing.Size(657, 22);
            this.statusStrip1.TabIndex = 0;
            this.statusStrip1.Text = "statusStrip1";

            this.stStripLabel.Name = "stStripLabel";
            this.stStripLabel.Size = new System.Drawing.Size(128, 17);
            this.stStripLabel.Text = "toolStripStatusLabel1";

            this.menuStrip1.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
                this.fileToolStripMenuItem,
                this.iPToolStripMenuItem,
                this.gggToolStripMenuItem,
                this.直圖圖轉換與直圖等化ToolStripMenuItem,
                this.filterToolStripMenuItem,
                this.resizeToolStripMenuItem,
                this.rotateToolStripMenuItem,
                this.otsuToolStripMenuItem,
                this.lineDetectToolStripMenuItem,
                this.metalCharToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Padding = new System.Windows.Forms.Padding(4, 2, 0, 2);
            this.menuStrip1.Size = new System.Drawing.Size(657, 24);
            this.menuStrip1.TabIndex = 1;
            this.menuStrip1.Text = "menuStrip1";

            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] { this.openToolStripMenuItem });
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(38, 20);
            this.fileToolStripMenuItem.Text = "&File";
            this.openToolStripMenuItem.Name = "openToolStripMenuItem";
            this.openToolStripMenuItem.Size = new System.Drawing.Size(106, 22);
            this.openToolStripMenuItem.Text = "&Open";
            this.openToolStripMenuItem.Click += new System.EventHandler(this.openToolStripMenuItem_Click);

            this.iPToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
                this.rGBtoGrayToolStripMenuItem, this.fffToolStripMenuItem});
            this.iPToolStripMenuItem.Name = "iPToolStripMenuItem";
            this.iPToolStripMenuItem.Size = new System.Drawing.Size(38, 20);
            this.iPToolStripMenuItem.Text = "I&GS";
            this.rGBtoGrayToolStripMenuItem.Name = "rGBtoGrayToolStripMenuItem";
            this.rGBtoGrayToolStripMenuItem.Size = new System.Drawing.Size(136, 22);
            this.rGBtoGrayToolStripMenuItem.Text = "RGBtoGray";
            this.rGBtoGrayToolStripMenuItem.Click += new System.EventHandler(this.RGBtoGrayToolStripMenuItem_Click);

            this.fffToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
                this.toolStripMenuItem2, this.toolStripMenuItem3, this.toolStripMenuItem4,
                this.toolStripMenuItem5, this.toolStripMenuItem6, this.toolStripMenuItem7,
                this.toolStripMenuItem8, this.toolStripMenuItem9});
            this.fffToolStripMenuItem.Name = "fffToolStripMenuItem";
            this.fffToolStripMenuItem.Size = new System.Drawing.Size(136, 22);
            this.fffToolStripMenuItem.Text = "Bit Section";

            this.toolStripMenuItem2.Name = "toolStripMenuItem2"; this.toolStripMenuItem2.Size = new System.Drawing.Size(81, 22); this.toolStripMenuItem2.Text = "0";
            this.toolStripMenuItem2.Click += new System.EventHandler(this.toolStripMenuItem2_Click);
            this.toolStripMenuItem3.Name = "toolStripMenuItem3"; this.toolStripMenuItem3.Size = new System.Drawing.Size(81, 22); this.toolStripMenuItem3.Text = "1";
            this.toolStripMenuItem3.Click += new System.EventHandler(this.toolStripMenuItem3_Click);
            this.toolStripMenuItem4.Name = "toolStripMenuItem4"; this.toolStripMenuItem4.Size = new System.Drawing.Size(81, 22); this.toolStripMenuItem4.Text = "2";
            this.toolStripMenuItem4.Click += new System.EventHandler(this.toolStripMenuItem4_Click);
            this.toolStripMenuItem5.Name = "toolStripMenuItem5"; this.toolStripMenuItem5.Size = new System.Drawing.Size(81, 22); this.toolStripMenuItem5.Text = "3";
            this.toolStripMenuItem5.Click += new System.EventHandler(this.toolStripMenuItem5_Click);
            this.toolStripMenuItem6.Name = "toolStripMenuItem6"; this.toolStripMenuItem6.Size = new System.Drawing.Size(81, 22); this.toolStripMenuItem6.Text = "4";
            this.toolStripMenuItem6.Click += new System.EventHandler(this.toolStripMenuItem6_Click);
            this.toolStripMenuItem7.Name = "toolStripMenuItem7"; this.toolStripMenuItem7.Size = new System.Drawing.Size(81, 22); this.toolStripMenuItem7.Text = "5";
            this.toolStripMenuItem7.Click += new System.EventHandler(this.toolStripMenuItem7_Click);
            this.toolStripMenuItem8.Name = "toolStripMenuItem8"; this.toolStripMenuItem8.Size = new System.Drawing.Size(81, 22); this.toolStripMenuItem8.Text = "6";
            this.toolStripMenuItem8.Click += new System.EventHandler(this.toolStripMenuItem8_Click);
            this.toolStripMenuItem9.Name = "toolStripMenuItem9"; this.toolStripMenuItem9.Size = new System.Drawing.Size(81, 22); this.toolStripMenuItem9.Text = "7";
            this.toolStripMenuItem9.Click += new System.EventHandler(this.toolStripMenuItem9_Click);

            this.gggToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
                this.hggggToolStripMenuItem, this.contrastToolStripMenuItem});
            this.gggToolStripMenuItem.Name = "gggToolStripMenuItem";
            this.gggToolStripMenuItem.Size = new System.Drawing.Size(47, 20);
            this.gggToolStripMenuItem.Text = "Bt_Ct";
            this.hggggToolStripMenuItem.Name = "hggggToolStripMenuItem";
            this.hggggToolStripMenuItem.Size = new System.Drawing.Size(131, 22);
            this.hggggToolStripMenuItem.Text = "Brightness";
            this.hggggToolStripMenuItem.Click += new System.EventHandler(this.hggggToolStripMenuItem_Click);
            this.contrastToolStripMenuItem.Name = "contrastToolStripMenuItem";
            this.contrastToolStripMenuItem.Size = new System.Drawing.Size(131, 22);
            this.contrastToolStripMenuItem.Text = "Contrast";
            this.contrastToolStripMenuItem.Click += new System.EventHandler(this.contrastToolStripMenuItem_Click);

            this.直圖圖轉換與直圖等化ToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
                this.transformationToolStripMenuItem, this.equalizationToolStripMenuItem});
            this.直圖圖轉換與直圖等化ToolStripMenuItem.Name = "直圖圖轉換與直圖等化ToolStripMenuItem";
            this.直圖圖轉換與直圖等化ToolStripMenuItem.Size = new System.Drawing.Size(78, 20);
            this.直圖圖轉換與直圖等化ToolStripMenuItem.Text = "Histogram";
            this.transformationToolStripMenuItem.Name = "transformationToolStripMenuItem";
            this.transformationToolStripMenuItem.Size = new System.Drawing.Size(180, 50);
            this.transformationToolStripMenuItem.Text = "transformation";
            this.transformationToolStripMenuItem.Click += new System.EventHandler(this.transformationToolStripMenuItem_Click);
            this.equalizationToolStripMenuItem.Name = "equalizationToolStripMenuItem";
            this.equalizationToolStripMenuItem.Size = new System.Drawing.Size(180, 50);
            this.equalizationToolStripMenuItem.Text = "equalization";
            this.equalizationToolStripMenuItem.Click += new System.EventHandler(this.equalizationToolStripMenuItem_Click);

            this.filterToolStripMenuItem.Name = "filterToolStripMenuItem";
            this.filterToolStripMenuItem.Size = new System.Drawing.Size(47, 20);
            this.filterToolStripMenuItem.Text = "濾波器";
            this.filterToolStripMenuItem.Click += new System.EventHandler(this.filterToolStripMenuItem_Click);

            this.resizeToolStripMenuItem.Name = "resizeToolStripMenuItem";
            this.resizeToolStripMenuItem.Size = new System.Drawing.Size(67, 20);
            this.resizeToolStripMenuItem.Text = "放大縮小";
            this.resizeToolStripMenuItem.Click += new System.EventHandler(this.resizeToolStripMenuItem_Click);

            this.rotateToolStripMenuItem.Name = "rotateToolStripMenuItem";
            this.rotateToolStripMenuItem.Size = new System.Drawing.Size(47, 20);
            this.rotateToolStripMenuItem.Text = "旋轉";
            this.rotateToolStripMenuItem.Click += new System.EventHandler(this.rotateToolStripMenuItem_Click);

            this.otsuToolStripMenuItem.Name = "otsuToolStripMenuItem";
            this.otsuToolStripMenuItem.Size = new System.Drawing.Size(87, 20);
            this.otsuToolStripMenuItem.Text = "Otsu分割";
            this.otsuToolStripMenuItem.Click += new System.EventHandler(this.otsuToolStripMenuItem_Click);

            this.lineDetectToolStripMenuItem.Name = "lineDetectToolStripMenuItem";
            this.lineDetectToolStripMenuItem.Size = new System.Drawing.Size(57, 20);
            this.lineDetectToolStripMenuItem.Text = "線偵測";
            this.lineDetectToolStripMenuItem.Click += new System.EventHandler(this.lineDetectToolStripMenuItem_Click);

            this.metalCharToolStripMenuItem.Name = "metalCharToolStripMenuItem";
            this.metalCharToolStripMenuItem.Size = new System.Drawing.Size(87, 20);
            this.metalCharToolStripMenuItem.Text = "金屬字元辨識";
            this.metalCharToolStripMenuItem.Click += new System.EventHandler(this.metalCharToolStripMenuItem_Click);

            this.oFileDlg.FileName = "openFileDialog1";

            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(657, 379);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.menuStrip1);
            this.MainMenuStrip = this.menuStrip1;
            this.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.Name = "DIPSample";
            this.Text = "DIPSample";
            this.Load += new System.EventHandler(this.DIPSample_Load);
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();
        }

        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel stStripLabel;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem iPToolStripMenuItem;
        private System.Windows.Forms.OpenFileDialog oFileDlg;
        private System.Windows.Forms.SaveFileDialog saveFileDialog1;
        private System.Windows.Forms.ToolStripMenuItem rGBtoGrayToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fffToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem gggToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem hggggToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem2;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem3;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem4;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem5;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem6;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem7;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem8;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem9;
        private System.Windows.Forms.ToolStripMenuItem contrastToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 直圖圖轉換與直圖等化ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem transformationToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem equalizationToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem filterToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem resizeToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem rotateToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem otsuToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem lineDetectToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem metalCharToolStripMenuItem;
    }
}
