namespace SirenaBT5
{
	partial class EventForm
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.CHIndex = new System.Windows.Forms.ComboBox();
			this.Type = new System.Windows.Forms.ComboBox();
			this.Melody = new System.Windows.Forms.ComboBox();
			this.label1 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.label3 = new System.Windows.Forms.Label();
			this.PowerOff = new System.Windows.Forms.CheckBox();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.label10 = new System.Windows.Forms.Label();
			this.label8 = new System.Windows.Forms.Label();
			this.Count3 = new System.Windows.Forms.NumericUpDown();
			this.Width3 = new System.Windows.Forms.NumericUpDown();
			this.label9 = new System.Windows.Forms.Label();
			this.Impulse3 = new System.Windows.Forms.CheckBox();
			this.label6 = new System.Windows.Forms.Label();
			this.Count2 = new System.Windows.Forms.NumericUpDown();
			this.Width2 = new System.Windows.Forms.NumericUpDown();
			this.label7 = new System.Windows.Forms.Label();
			this.Impulse2 = new System.Windows.Forms.CheckBox();
			this.label5 = new System.Windows.Forms.Label();
			this.Count1 = new System.Windows.Forms.NumericUpDown();
			this.Width1 = new System.Windows.Forms.NumericUpDown();
			this.label4 = new System.Windows.Forms.Label();
			this.Apply = new System.Windows.Forms.Button();
			this.groupBox1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.Count3)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.Width3)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.Count2)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.Width2)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.Count1)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.Width1)).BeginInit();
			this.SuspendLayout();
			// 
			// CHIndex
			// 
			this.CHIndex.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.CHIndex.FormattingEnabled = true;
			this.CHIndex.Location = new System.Drawing.Point(72, 12);
			this.CHIndex.Name = "CHIndex";
			this.CHIndex.Size = new System.Drawing.Size(94, 21);
			this.CHIndex.TabIndex = 0;
			this.CHIndex.SelectedIndexChanged += new System.EventHandler(this.CHIndex_SelectedIndexChanged);
			// 
			// Type
			// 
			this.Type.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.Type.FormattingEnabled = true;
			this.Type.Location = new System.Drawing.Point(266, 12);
			this.Type.Name = "Type";
			this.Type.Size = new System.Drawing.Size(94, 21);
			this.Type.TabIndex = 1;
			this.Type.SelectedIndexChanged += new System.EventHandler(this.Type_SelectedIndexChanged);
			// 
			// Melody
			// 
			this.Melody.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.Melody.FormattingEnabled = true;
			this.Melody.Location = new System.Drawing.Point(73, 39);
			this.Melody.Name = "Melody";
			this.Melody.Size = new System.Drawing.Size(532, 21);
			this.Melody.TabIndex = 2;
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(12, 20);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(41, 13);
			this.label1.TabIndex = 3;
			this.label1.Text = "Канал:";
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(185, 20);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(75, 13);
			this.label2.TabIndex = 4;
			this.label2.Text = "Тип события:";
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(12, 47);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(55, 13);
			this.label3.TabIndex = 5;
			this.label3.Text = "Мелодия:";
			// 
			// PowerOff
			// 
			this.PowerOff.AutoSize = true;
			this.PowerOff.Location = new System.Drawing.Point(407, 16);
			this.PowerOff.Name = "PowerOff";
			this.PowerOff.Size = new System.Drawing.Size(198, 17);
			this.PowerOff.TabIndex = 6;
			this.PowerOff.Text = "Сразу уходить в дежурный режим";
			this.PowerOff.UseVisualStyleBackColor = true;
			this.PowerOff.Click += new System.EventHandler(this.PowerOff_Click);
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.label10);
			this.groupBox1.Controls.Add(this.label8);
			this.groupBox1.Controls.Add(this.Count3);
			this.groupBox1.Controls.Add(this.Width3);
			this.groupBox1.Controls.Add(this.label9);
			this.groupBox1.Controls.Add(this.Impulse3);
			this.groupBox1.Controls.Add(this.label6);
			this.groupBox1.Controls.Add(this.Count2);
			this.groupBox1.Controls.Add(this.Width2);
			this.groupBox1.Controls.Add(this.label7);
			this.groupBox1.Controls.Add(this.Impulse2);
			this.groupBox1.Controls.Add(this.label5);
			this.groupBox1.Controls.Add(this.Count1);
			this.groupBox1.Controls.Add(this.Width1);
			this.groupBox1.Controls.Add(this.label4);
			this.groupBox1.Location = new System.Drawing.Point(11, 76);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(594, 106);
			this.groupBox1.TabIndex = 7;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Импульсы";
			// 
			// label10
			// 
			this.label10.AutoSize = true;
			this.label10.Location = new System.Drawing.Point(13, 20);
			this.label10.Name = "label10";
			this.label10.Size = new System.Drawing.Size(92, 13);
			this.label10.TabIndex = 15;
			this.label10.Text = "Вид имульса №1";
			// 
			// label8
			// 
			this.label8.AutoSize = true;
			this.label8.Location = new System.Drawing.Point(338, 75);
			this.label8.Name = "label8";
			this.label8.Size = new System.Drawing.Size(69, 13);
			this.label8.TabIndex = 14;
			this.label8.Text = "Количество:";
			// 
			// Count3
			// 
			this.Count3.Location = new System.Drawing.Point(413, 68);
			this.Count3.Maximum = new decimal(new int[] {
            20,
            0,
            0,
            0});
			this.Count3.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.Count3.Name = "Count3";
			this.Count3.Size = new System.Drawing.Size(53, 20);
			this.Count3.TabIndex = 13;
			this.Count3.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
			// 
			// Width3
			// 
			this.Width3.Location = new System.Drawing.Point(413, 42);
			this.Width3.Maximum = new decimal(new int[] {
            2000,
            0,
            0,
            0});
			this.Width3.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.Width3.Name = "Width3";
			this.Width3.Size = new System.Drawing.Size(53, 20);
			this.Width3.TabIndex = 12;
			this.Width3.Value = new decimal(new int[] {
            10,
            0,
            0,
            0});
			// 
			// label9
			// 
			this.label9.AutoSize = true;
			this.label9.Location = new System.Drawing.Point(338, 49);
			this.label9.Name = "label9";
			this.label9.Size = new System.Drawing.Size(49, 13);
			this.label9.TabIndex = 11;
			this.label9.Text = "Ширина:";
			// 
			// Impulse3
			// 
			this.Impulse3.AutoSize = true;
			this.Impulse3.Location = new System.Drawing.Point(341, 19);
			this.Impulse3.Name = "Impulse3";
			this.Impulse3.Size = new System.Drawing.Size(117, 17);
			this.Impulse3.TabIndex = 10;
			this.Impulse3.Text = "Вид импульса №3";
			this.Impulse3.UseVisualStyleBackColor = true;
			this.Impulse3.CheckedChanged += new System.EventHandler(this.Impulse3_CheckedChanged);
			// 
			// label6
			// 
			this.label6.AutoSize = true;
			this.label6.Location = new System.Drawing.Point(174, 75);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(69, 13);
			this.label6.TabIndex = 9;
			this.label6.Text = "Количество:";
			// 
			// Count2
			// 
			this.Count2.Location = new System.Drawing.Point(249, 68);
			this.Count2.Maximum = new decimal(new int[] {
            20,
            0,
            0,
            0});
			this.Count2.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.Count2.Name = "Count2";
			this.Count2.Size = new System.Drawing.Size(53, 20);
			this.Count2.TabIndex = 8;
			this.Count2.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
			// 
			// Width2
			// 
			this.Width2.Location = new System.Drawing.Point(249, 42);
			this.Width2.Maximum = new decimal(new int[] {
            2000,
            0,
            0,
            0});
			this.Width2.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.Width2.Name = "Width2";
			this.Width2.Size = new System.Drawing.Size(53, 20);
			this.Width2.TabIndex = 7;
			this.Width2.Value = new decimal(new int[] {
            600,
            0,
            0,
            0});
			// 
			// label7
			// 
			this.label7.AutoSize = true;
			this.label7.Location = new System.Drawing.Point(174, 49);
			this.label7.Name = "label7";
			this.label7.Size = new System.Drawing.Size(49, 13);
			this.label7.TabIndex = 6;
			this.label7.Text = "Ширина:";
			// 
			// Impulse2
			// 
			this.Impulse2.AutoSize = true;
			this.Impulse2.Location = new System.Drawing.Point(177, 19);
			this.Impulse2.Name = "Impulse2";
			this.Impulse2.Size = new System.Drawing.Size(117, 17);
			this.Impulse2.TabIndex = 5;
			this.Impulse2.Text = "Вид импульса №2";
			this.Impulse2.UseVisualStyleBackColor = true;
			this.Impulse2.CheckedChanged += new System.EventHandler(this.Impulse2_CheckedChanged);
			// 
			// label5
			// 
			this.label5.AutoSize = true;
			this.label5.Location = new System.Drawing.Point(13, 75);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(69, 13);
			this.label5.TabIndex = 4;
			this.label5.Text = "Количество:";
			// 
			// Count1
			// 
			this.Count1.Location = new System.Drawing.Point(88, 68);
			this.Count1.Maximum = new decimal(new int[] {
            20,
            0,
            0,
            0});
			this.Count1.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.Count1.Name = "Count1";
			this.Count1.Size = new System.Drawing.Size(53, 20);
			this.Count1.TabIndex = 3;
			this.Count1.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
			// 
			// Width1
			// 
			this.Width1.Location = new System.Drawing.Point(88, 42);
			this.Width1.Maximum = new decimal(new int[] {
            2000,
            0,
            0,
            0});
			this.Width1.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.Width1.Name = "Width1";
			this.Width1.Size = new System.Drawing.Size(53, 20);
			this.Width1.TabIndex = 2;
			this.Width1.Value = new decimal(new int[] {
            300,
            0,
            0,
            0});
			// 
			// label4
			// 
			this.label4.AutoSize = true;
			this.label4.Location = new System.Drawing.Point(13, 49);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(49, 13);
			this.label4.TabIndex = 1;
			this.label4.Text = "Ширина:";
			// 
			// Apply
			// 
			this.Apply.Location = new System.Drawing.Point(513, 197);
			this.Apply.Name = "Apply";
			this.Apply.Size = new System.Drawing.Size(93, 34);
			this.Apply.TabIndex = 8;
			this.Apply.Text = "Применить";
			this.Apply.UseVisualStyleBackColor = true;
			this.Apply.Click += new System.EventHandler(this.Apply_Click);
			// 
			// EventForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(618, 242);
			this.Controls.Add(this.Apply);
			this.Controls.Add(this.groupBox1);
			this.Controls.Add(this.PowerOff);
			this.Controls.Add(this.label3);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.Melody);
			this.Controls.Add(this.Type);
			this.Controls.Add(this.CHIndex);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.MaximizeBox = false;
			this.Name = "EventForm";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Событие";
			this.groupBox1.ResumeLayout(false);
			this.groupBox1.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.Count3)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.Width3)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.Count2)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.Width2)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.Count1)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.Width1)).EndInit();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.ComboBox CHIndex;
		private System.Windows.Forms.ComboBox Type;
		private System.Windows.Forms.ComboBox Melody;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.CheckBox PowerOff;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.Label label8;
		private System.Windows.Forms.NumericUpDown Count3;
		private System.Windows.Forms.NumericUpDown Width3;
		private System.Windows.Forms.Label label9;
		private System.Windows.Forms.CheckBox Impulse3;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.NumericUpDown Count2;
		private System.Windows.Forms.NumericUpDown Width2;
		private System.Windows.Forms.Label label7;
		private System.Windows.Forms.CheckBox Impulse2;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.NumericUpDown Count1;
		private System.Windows.Forms.NumericUpDown Width1;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.Button Apply;
		private System.Windows.Forms.Label label10;
	}
}