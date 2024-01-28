namespace SirenaBT5
{
	partial class WaitControl
	{
		/// <summary> 
		/// Обязательная переменная конструктора.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary> 
		/// Освободить все используемые ресурсы.
		/// </summary>
		/// <param name="disposing">истинно, если управляемый ресурс должен быть удален; иначе ложно.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Код, автоматически созданный конструктором компонентов

		/// <summary> 
		/// Требуемый метод для поддержки конструктора — не изменяйте 
		/// содержимое этого метода с помощью редактора кода.
		/// </summary>
		private void InitializeComponent()
		{
			this.status = new System.Windows.Forms.Label();
			this.progress = new System.Windows.Forms.ProgressBar();
			this.SuspendLayout();
			// 
			// status
			// 
			this.status.AutoSize = true;
			this.status.Font = new System.Drawing.Font("Microsoft Sans Serif", 11.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
			this.status.ForeColor = System.Drawing.Color.White;
			this.status.Location = new System.Drawing.Point(78, 87);
			this.status.Name = "status";
			this.status.Size = new System.Drawing.Size(93, 18);
			this.status.TabIndex = 0;
			this.status.Text = "Status Text";
			this.status.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			this.status.Paint += new System.Windows.Forms.PaintEventHandler(this.status_Paint);
			// 
			// progress
			// 
			this.progress.Location = new System.Drawing.Point(81, 188);
			this.progress.Name = "progress";
			this.progress.Size = new System.Drawing.Size(163, 47);
			this.progress.TabIndex = 1;
			this.progress.Value = 50;
			// 
			// WaitControl
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
			this.Controls.Add(this.progress);
			this.Controls.Add(this.status);
			this.Name = "WaitControl";
			this.Size = new System.Drawing.Size(631, 482);
			this.Resize += new System.EventHandler(this.Wait_Resize);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Label status;
		private System.Windows.Forms.ProgressBar progress;
	}
}
