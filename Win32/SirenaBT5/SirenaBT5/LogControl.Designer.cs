namespace SirenaBT5
{
	partial class LogControl
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
			this.components = new System.ComponentModel.Container();
			this.label1 = new System.Windows.Forms.Label();
			this.mCH = new System.Windows.Forms.TextBox();
			this.mPanel = new System.Windows.Forms.Panel();
			this.mPress = new System.Windows.Forms.TextBox();
			this.mIdle = new System.Windows.Forms.TextBox();
			this.label2 = new System.Windows.Forms.Label();
			this.label3 = new System.Windows.Forms.Label();
			this.ePanel = new System.Windows.Forms.Panel();
			this.eCH = new System.Windows.Forms.TextBox();
			this.label4 = new System.Windows.Forms.Label();
			this.label5 = new System.Windows.Forms.Label();
			this.eType = new System.Windows.Forms.TextBox();
			this.ePulses = new System.Windows.Forms.TextBox();
			this.addEvent = new System.Windows.Forms.Button();
			this.timerBG = new System.Windows.Forms.Timer(this.components);
			this.mPanel.SuspendLayout();
			this.ePanel.SuspendLayout();
			this.SuspendLayout();
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(3, 6);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(55, 13);
			this.label1.TabIndex = 0;
			this.label1.Text = "Канал №:";
			// 
			// mCH
			// 
			this.mCH.Location = new System.Drawing.Point(64, 3);
			this.mCH.Name = "mCH";
			this.mCH.ReadOnly = true;
			this.mCH.Size = new System.Drawing.Size(18, 20);
			this.mCH.TabIndex = 1;
			// 
			// mPanel
			// 
			this.mPanel.Controls.Add(this.label3);
			this.mPanel.Controls.Add(this.label2);
			this.mPanel.Controls.Add(this.mIdle);
			this.mPanel.Controls.Add(this.mPress);
			this.mPanel.Controls.Add(this.label1);
			this.mPanel.Controls.Add(this.mCH);
			this.mPanel.Location = new System.Drawing.Point(34, 116);
			this.mPanel.Name = "mPanel";
			this.mPanel.Size = new System.Drawing.Size(398, 27);
			this.mPanel.TabIndex = 2;
			// 
			// mPress
			// 
			this.mPress.Location = new System.Drawing.Point(171, 3);
			this.mPress.Name = "mPress";
			this.mPress.ReadOnly = true;
			this.mPress.Size = new System.Drawing.Size(56, 20);
			this.mPress.TabIndex = 2;
			// 
			// mIdle
			// 
			this.mIdle.Location = new System.Drawing.Point(337, 3);
			this.mIdle.Name = "mIdle";
			this.mIdle.ReadOnly = true;
			this.mIdle.Size = new System.Drawing.Size(56, 20);
			this.mIdle.TabIndex = 3;
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(97, 6);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(68, 13);
			this.label2.TabIndex = 4;
			this.label2.Text = "Удержание:";
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(246, 6);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(85, 13);
			this.label3.TabIndex = 5;
			this.label3.Text = "Не активность:";
			// 
			// ePanel
			// 
			this.ePanel.Controls.Add(this.addEvent);
			this.ePanel.Controls.Add(this.ePulses);
			this.ePanel.Controls.Add(this.eType);
			this.ePanel.Controls.Add(this.label5);
			this.ePanel.Controls.Add(this.label4);
			this.ePanel.Controls.Add(this.eCH);
			this.ePanel.Location = new System.Drawing.Point(34, 30);
			this.ePanel.Name = "ePanel";
			this.ePanel.Size = new System.Drawing.Size(398, 59);
			this.ePanel.TabIndex = 3;
			// 
			// eCH
			// 
			this.eCH.Location = new System.Drawing.Point(64, 3);
			this.eCH.Name = "eCH";
			this.eCH.ReadOnly = true;
			this.eCH.Size = new System.Drawing.Size(18, 20);
			this.eCH.TabIndex = 2;
			// 
			// label4
			// 
			this.label4.AutoSize = true;
			this.label4.Location = new System.Drawing.Point(3, 6);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(55, 13);
			this.label4.TabIndex = 6;
			this.label4.Text = "Канал №:";
			// 
			// label5
			// 
			this.label5.AutoSize = true;
			this.label5.Location = new System.Drawing.Point(98, 6);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(41, 13);
			this.label5.TabIndex = 7;
			this.label5.Text = "Канал:";
			// 
			// eType
			// 
			this.eType.Location = new System.Drawing.Point(145, 3);
			this.eType.Name = "eType";
			this.eType.ReadOnly = true;
			this.eType.Size = new System.Drawing.Size(83, 20);
			this.eType.TabIndex = 6;
			// 
			// ePulses
			// 
			this.ePulses.Location = new System.Drawing.Point(234, 3);
			this.ePulses.Name = "ePulses";
			this.ePulses.ReadOnly = true;
			this.ePulses.Size = new System.Drawing.Size(159, 20);
			this.ePulses.TabIndex = 8;
			// 
			// addEvent
			// 
			this.addEvent.Location = new System.Drawing.Point(134, 29);
			this.addEvent.Name = "addEvent";
			this.addEvent.Size = new System.Drawing.Size(127, 26);
			this.addEvent.TabIndex = 9;
			this.addEvent.Text = "Добавить событие";
			this.addEvent.UseVisualStyleBackColor = true;
			this.addEvent.Click += new System.EventHandler(this.addEvent_Click);
			// 
			// timerBG
			// 
			this.timerBG.Interval = 800;
			this.timerBG.Tick += new System.EventHandler(this.timerBG_Tick);
			// 
			// LogControl
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackColor = System.Drawing.SystemColors.Control;
			this.Controls.Add(this.ePanel);
			this.Controls.Add(this.mPanel);
			this.Name = "LogControl";
			this.Size = new System.Drawing.Size(481, 251);
			this.Load += new System.EventHandler(this.LogControl_Load);
			this.mPanel.ResumeLayout(false);
			this.mPanel.PerformLayout();
			this.ePanel.ResumeLayout(false);
			this.ePanel.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TextBox mCH;
		private System.Windows.Forms.Panel mPanel;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TextBox mIdle;
		private System.Windows.Forms.TextBox mPress;
		private System.Windows.Forms.Panel ePanel;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.TextBox eCH;
		private System.Windows.Forms.TextBox eType;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.TextBox ePulses;
		private System.Windows.Forms.Button addEvent;
		private System.Windows.Forms.Timer timerBG;
	}
}
