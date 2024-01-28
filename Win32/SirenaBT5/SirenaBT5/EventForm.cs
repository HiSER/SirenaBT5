using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace SirenaBT5
{
	public partial class EventForm : Form
	{
		private ListViewItem eventItem;
		private bool edit;

		public EventForm(ListView listMelodys, ListViewItem eventItem, bool edit = false, SirenaChannelEvent ech = null)
		{
			InitializeComponent();
			this.eventItem = eventItem;
			this.edit = edit;
			InitElements(listMelodys, ech);
		}

		private void InitElements(ListView listMelodys, SirenaChannelEvent ech)
		{
			Text += ": " + ((SirenaEvent)eventItem.Tag).index.ToString();
			DialogResult = DialogResult.Cancel;

			CHIndex.Items.Add(new ComboItem(1, "№1"));
			CHIndex.Items.Add(new ComboItem(2, "№2"));
			CHIndex.Items.Add(new ComboItem(3, "№3"));

			Type.Items.Add(new ComboItem(SirenaEvent.PulseType.Press, "Удержание"));
			Type.Items.Add(new ComboItem(SirenaEvent.PulseType.Cyclic, "Циклический"));
			Type.Items.Add(new ComboItem(SirenaEvent.PulseType.Pulse, "Импульсы"));

			if (edit) Melody.Items.Add(new ComboItem(-2, "не менять"));
			Melody.Items.Add(new ComboItem(-1, "не установлена"));
			if (listMelodys != null)
			{
				foreach (ListViewItem i in listMelodys.Items)
				{
					Melody.Items.Add(new ComboItem(i.Tag, i.SubItems[1].Text));
				}
			}

			Impulse3.Checked = false;
			Impulse2.Checked = false;
			if (edit)
			{
				SirenaEvent e = (SirenaEvent)eventItem.Tag;
				PowerOff.Checked = e.poweroff;
				PowerOff.Tag = false;
				ComboSelect(Type, e.type);
				ComboSelect(CHIndex, e.chIndex);
				ComboSelect(Melody, -2);
				if (e.type == SirenaEvent.PulseType.Pulse)
				{
					Width1.Value = e.pulse[0].width;
					Count1.Value = e.pulse[0].count;
					if (e.pulse.Count > 1)
					{
						Impulse2.Checked = true;
						Width2.Value = e.pulse[1].width;
						Count2.Value = e.pulse[1].count;
					}
					if (e.pulse.Count > 2)
					{
						Impulse3.Checked = true;
						Width3.Value = e.pulse[2].width;
						Count3.Value = e.pulse[2].count;
					}
				}
			}
			else
			{
				PowerOff.Checked = true;
				PowerOff.Tag = true;
				ComboSelect(Type, SirenaEvent.PulseType.Press);
				ComboSelect(CHIndex, 1);
				ComboSelect(Melody, -1);
			}

			if (ech != null)
			{
				ComboSelect(Type, ech.type);
				ComboSelect(CHIndex, ech.index);
				if (ech.type == SirenaEvent.PulseType.Pulse)
				{
					Width1.Value = ech.pulse[0].width;
					Count1.Value = ech.pulse[0].count;
					if (ech.pulse.Count > 1)
					{
						Impulse2.Checked = true;
						Width2.Value = ech.pulse[1].width;
						Count2.Value = ech.pulse[1].count;
					}
					if (ech.pulse.Count > 2)
					{
						Impulse3.Checked = true;
						Width3.Value = ech.pulse[2].width;
						Count3.Value = ech.pulse[2].count;
					}
				}
			}
		} 

		private class ComboItem
		{
			public object key;
			public string text;

			public ComboItem(object key, string text = "")
			{
				this.key = key;
				this.text = text;
			}
			public override string ToString()
			{
				return text;
			}
		}
		private void ComboSelect(ComboBox combo, object key)
		{
			foreach (ComboItem i in combo.Items)
			{
				if (((int)i.key) == ((int)key))
				{
					combo.SelectedItem = i;
					break;
				}
			}
		}
		private object ComboValue(ComboBox combo)
		{
			return ((ComboItem)combo.SelectedItem).key;
		}

		private void Impulse3_CheckedChanged(object sender, EventArgs e)
		{
			bool flag = (Impulse3.Checked && Impulse2.Checked && Width1.Enabled && Type.Enabled);
			Width3.Enabled = flag;
			Count3.Enabled = flag;
		}
		private void Impulse2_CheckedChanged(object sender, EventArgs e)
		{
			bool flag = (Impulse2.Checked && Width1.Enabled && Type.Enabled);
			Impulse3.Enabled = flag;
			Width2.Enabled = flag;
			Count2.Enabled = flag;
			Impulse3_CheckedChanged(null, null);
		}
		private void Type_SelectedIndexChanged(object sender, EventArgs e)
		{
			bool flag = (Type.Enabled && (SirenaEvent.PulseType)ComboValue(Type) == SirenaEvent.PulseType.Pulse);
			Impulse2.Enabled = flag;
			Width1.Enabled = flag;
			Count1.Enabled = flag;
			Impulse2_CheckedChanged(null, null);
		}
		private void CHIndex_SelectedIndexChanged(object sender, EventArgs e)
		{
			Type.Enabled = ((int)ComboValue(CHIndex) == 1);
			if ((bool)PowerOff.Tag) PowerOff.Checked = Type.Enabled;
			Type_SelectedIndexChanged(null, null);
		}

		private void Apply_Click(object sender, EventArgs e)
		{
			SirenaEvent ev = (SirenaEvent)eventItem.Tag;
			ev.chIndex = (int)ComboValue(CHIndex);
			if ((int)ComboValue(Melody) > -2)
			{
				ev.melodyIndex = (int)ComboValue(Melody);
			}
			ev.poweroff = PowerOff.Checked;
			ev.pulse.Clear();
			if (ev.chIndex == 1)
			{
				ev.type = (SirenaEvent.PulseType)ComboValue(Type);
				if (ev.type == SirenaEvent.PulseType.Pulse)
				{
					int i;
					int count = 1;
					if (Impulse2.Checked)
					{
						count++;
						if (Impulse3.Checked) count++;
					}
					for (i = 0; i < count; i++)
					{
						switch (i)
						{
							case 0:
								ev.pulse.Add(new SirenaPulse() { width = (int)Width1.Value, count = (int)Count1.Value });
								break;
							case 1:
								ev.pulse.Add(new SirenaPulse() { width = (int)Width2.Value, count = (int)Count2.Value });
								break;
							case 2:
								ev.pulse.Add(new SirenaPulse() { width = (int)Width3.Value, count = (int)Count3.Value });
								break;
						}
					}
				}
			}
			else
			{
				ev.type = SirenaEvent.PulseType.Press;
			}
			DialogResult = DialogResult.OK;
		}

		private void PowerOff_Click(object sender, EventArgs e)
		{
			PowerOff.Tag = false;
		}
	}
}
