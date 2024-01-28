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
	public partial class LogControl : UserControl
	{
		public delegate void EventHandlerAddEvent(SirenaChannelEvent e);

		public EventHandlerAddEvent EventAddEvent;

		private SirenaChannelEvent chEvent;

		public LogControl(int index, int widthMax, int idleMin)
		{
			InitializeComponent();
			mCH.Text = index.ToString();
			if (widthMax < 1 && idleMin < 1)
			{
				mPress.Text = "Ошибка";
				mIdle.Text = "-";
			}
			else
			{
				int widthCenter = (idleMin - widthMax) / 2 + widthMax;
				widthMax += 50;
				if (widthMax > widthCenter) widthMax = widthCenter;
				idleMin += 100;
				if (idleMin > 2000) idleMin = 2000;
				if (widthMax > idleMin) widthMax = idleMin;
				mPress.Text = widthMax.ToString();
				mIdle.Text = idleMin.ToString();
			}
			chEvent = null;
			ePanel.Visible = false;
			mPanel.Visible = true;
			mPanel.Location = new Point(3, 3);
			Width = mPanel.Width + mPanel.Location.X * 2;
			Height = mPanel.Height + mPanel.Location.Y * 2;
			BackColor = Color.LightCoral;
			BorderStyle = BorderStyle.None;
		}
		public LogControl(SirenaChannelEvent e)
		{
			InitializeComponent();
			chEvent = e;
			eCH.Text = e.index.ToString();
			ePulses.Visible = false;
			ePanel.Height = 58;
			switch (e.type)
			{
				case SirenaEvent.PulseType.Press:
					eType.Text = "Удержание";
					break;
				case SirenaEvent.PulseType.Cyclic:
					eType.Text = "Циклический";
					break;
				case SirenaEvent.PulseType.Release:
					ePanel.Height = 27;
					eType.Text = "Не активен";
					break;
				case SirenaEvent.PulseType.Pulse:
					eType.Text = "Импульсы";
					ePulses.Text = "";
					foreach (var p in e.pulse)
					{
						if (ePulses.Text != "") ePulses.Text += ", ";
						ePulses.Text += p.width.ToString() + ":" + p.count.ToString();
					}
					ePulses.Visible = true;
					break;
			}
			mPanel.Visible = false;
			ePanel.Visible = true;
			ePanel.Location = new Point(3, 3);
			Width = ePanel.Width + ePanel.Location.X * 2;
			Height = ePanel.Height + ePanel.Location.Y * 2;
			BackColor = Color.LightCoral;
			BorderStyle = BorderStyle.FixedSingle;
		}

		private void addEvent_Click(object sender, EventArgs e)
		{
			if (EventAddEvent != null && chEvent != null)
			{
				EventAddEvent(chEvent);
			}
		}

		private void timerBG_Tick(object sender, EventArgs e)
		{
			timerBG.Stop();
			BackColor = ((chEvent == null) ? SystemColors.ControlLight : SystemColors.ControlLightLight);
		}

		private void LogControl_Load(object sender, EventArgs e)
		{
			timerBG.Start();
		}
	}
}
