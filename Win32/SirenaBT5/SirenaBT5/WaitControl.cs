using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace SirenaBT5
{
	public partial class WaitControl : UserControl
	{
		public delegate void EventHandler();

		public event EventHandler EventShow;
		public event EventHandler EventHide;
		
		public WaitControl()
		{
			InitializeComponent();
			Hide();
		}

		new public void Show()
		{
			Visible = true;
			if (EventShow != null) EventShow();
		}
		new public void Hide()
		{
			Visible = false;
			status.Visible = false;
			progress.Visible = false;
			if (EventHide != null) EventHide();
		}

		public void ShowStatus(string text)
		{
			status.Text = text;
			if (!status.Visible)
			{
				status.Visible = true;
				progress.Visible = false;
				Wait_Resize(null, null);
				Show();
			}
		}

		public void ShowProgress(int value)
		{
			progress.Value = value;
			if (!progress.Visible)
			{
				progress.Visible = true;
				Wait_Resize(null, null);
				Show();
			}
		}

		private void Wait_Resize(object sender, EventArgs e)
		{
			progress.Width = Width - 100;
			if (status.Visible && progress.Visible)
			{
				status.Location = new Point((Width - status.Width) / 2, (Height - (status.Height + progress.Height)) / 2);
				progress.Location = new Point((Width - progress.Width) / 2, status.Location.Y + status.Height + 10);
			}
			else
			{
				SetCenter(status);
				SetCenter(progress);
			}
		}

		private void SetCenter(object sender)
		{
			Control c = (Control)sender;
			if (c.Visible)
			{
				c.Location = new Point((Width - c.Width) / 2, (Height - c.Height) / 2);
			}
		}

		private void status_Paint(object sender, PaintEventArgs e)
		{
			Wait_Resize(null, null);
		}
	}
}
