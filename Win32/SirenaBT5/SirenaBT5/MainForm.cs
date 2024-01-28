using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.IO;
using System.Windows.Forms;

namespace SirenaBT5
{
	public partial class MainForm : Form
	{
		Sirena sirena;
		string newMelodyName;
		EventForm eventForm;
		private string version;
		private string debug;

		public MainForm(string debugPort)
		{
			InitializeComponent();
			debug = debugPort;
			sirena = new Sirena();
			sirena.owner = this;
			sirena.EventError += Sirena_EventError;
			sirena.EventDevices += Sirena_EventDevices;
			sirena.EventFS += Sirena_EventFS;
			sirena.EventConfig += Sirena_EventConfig;
			sirena.EventTables += Sirena_EventTables;
			sirena.EventConfigChange += Sirena_EventConfigChange;
			sirena.EventEventChange += Sirena_EventEventChange;
			sirena.EventMelodyChange += Sirena_EventMelodyChange;
			sirena.EventPlayChange += Sirena_EventPlayChange;
			sirena.EventPinChange += Sirena_EventPinChange;
			sirena.EventMeasure += Sirena_EventMeasure;
			sirena.EventChannel += Sirena_EventChannel;
			sirena.EventInfo += Sirena_EventInfo;
			eventForm = null;
			FindSirena();
		}

		private void Sirena_EventInfo(double version, int build)
		{
			this.version = version.ToString("0.0") + " build " + build.ToString(); 
		}

		private void Sirena_EventChannel(SirenaChannelEvent e)
		{
			addLogControl(new LogControl(e));
		}

		private void Sirena_EventMeasure(int index, int widthMax, int idleMin)
		{
			addLogControl(new LogControl(index, widthMax, idleMin));
		}

		private void addLogControl(LogControl c)
		{
			c.EventAddEvent += addEventItem;
			flowLog.Controls.Add(c);
			flowLog.Controls.SetChildIndex(c, 0);
			while (flowLog.Controls.Count > 20)
			{
				flowLog.Controls.RemoveAt(flowLog.Controls.Count - 1);
			}
		}
		private void Sirena_EventPinChange(Sirena.PinChange type, string pin)
		{
			switch (type)
			{
				case Sirena.PinChange.Request:
					PinCode.Text = pin;
					break;
				case Sirena.PinChange.Change:
					MessageBox.Show("PIN код изменён.\r\nДля вступления изменений в силу,\r\nнеобходимо сохнранить конфигурации и перезагрузить сирену.", "PIN код", MessageBoxButtons.OK, MessageBoxIcon.Information);
					break;
				default:
					MessageBox.Show("Не удалось изменить PIN!", null, MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
					break;
			}
			changePinCode.Enabled = true;
			PinCode.Enabled = true;
		}

		private void Sirena_EventPlayChange(Sirena.PlayChange type, int index)
		{
			switch (type)
			{
				case Sirena.PlayChange.Play:
					playMelody.Visible = false;
					stopMelody.Visible = true;
					playMelody.Enabled = true;
					break;
				case Sirena.PlayChange.Stop:
					if (listMelodys.SelectedItems.Count > 0) playMelody.Visible = true;
					stopMelody.Visible = false;
					stopMelody.Enabled = true;
					break;
			}
		}

		private void Sirena_EventMelodyChange(Sirena.MelodyChange type, int value)
		{
			switch (type)
			{
				case Sirena.MelodyChange.Progress:
					waitScreen.ShowProgress(value);
					break;

				case Sirena.MelodyChange.Start:
					addMelodyItem(new SirenaMelody() { index = value, name = newMelodyName });
					waitScreen.ShowStatus("Загрузка мелодии " + value.ToString());
					break;

				case Sirena.MelodyChange.End:
					addMelody.Enabled = true;
					waitScreen.Hide();
					break;

				case Sirena.MelodyChange.Delete:
					foreach (ListViewItem e in listMelodys.Items)
					{
						if ((int)e.Tag == value)
						{
							e.Remove();
							break;
						}
					}
					foreach (ListViewItem i in listEvents.Items)
					{
						if (((SirenaEvent)i.Tag).melodyIndex == value)
						{
							listEventUpdateItem(i);
						}
					}
					deleteMelody.Enabled = true;
					break;
				case Sirena.MelodyChange.Clear:
					listMelodys.Items.Clear();
					listMelodys_SelectedIndexChanged(listMelodys, null);
					foreach (ListViewItem i in listEvents.Items)
					{
						listEventUpdateItem(i);
					}
					clearMelodys.Enabled = true;
					break;

				case Sirena.MelodyChange.Error:
					FindSirena("Ошибка загрузки мелодии.");
					break;
			}
		}

		private void Sirena_EventEventChange(Sirena.EventChange type, int index)
		{
			switch (type)
			{
				case Sirena.EventChange.Change:
					addEvent.Enabled = true;
					editEvent.Enabled = true;
					break;
				case Sirena.EventChange.Delete:
					foreach (ListViewItem e in listEvents.Items)
					{
						if (((SirenaEvent)e.Tag).index == index)
						{
							e.Remove();
							break;
						}
					}
					deleteEvent.Enabled = true;
					break;
				case Sirena.EventChange.Clear:
					listEvents.Items.Clear();
					listEvents_SelectedIndexChanged(listEvents, null);
					clearEvents.Enabled = true;
					break;
			}
		}

		private void Sirena_EventConfigChange(Sirena.ConfigChange type, int index)
		{
			switch (type)
			{
				case Sirena.ConfigChange.Change:
					if (index == 0)
					{
						config0PressMax.Enabled = true;
						config0IdleMax.Enabled = true;
						config0PulsesMax.Enabled = true;
						config0Deviation.Enabled = true;
						config0Button.Enabled = true;
						config0Measure.Enabled = true;
					}
					break;
				case Sirena.ConfigChange.ChangeButton:
					if (index == 0)
					{
						config0Measure.Checked = (bool)config0Measure.Tag;
						config0Measure.Enabled = true;
						config0Button.Enabled = true;
					}
					break;
				case Sirena.ConfigChange.ChangeMeasure:
					if (index == 0)
					{
						config0Button.Checked = (bool)config0Button.Tag;
						config0Button.Enabled = true;
						config0Measure.Enabled = true;
					}
					break;
				case Sirena.ConfigChange.ChangePressMaximum:
					if (index == 0) config0PressMax.Enabled = true;
					break;
				case Sirena.ConfigChange.ChangeIdleMaximum:
					if (index == 0) config0IdleMax.Enabled = true;
					break;
				case Sirena.ConfigChange.ChangePulsesMaximum:
					if (index == 0) config0PulsesMax.Enabled = true;
					break;
				case Sirena.ConfigChange.ChangeDeviation:
					if (index == 0) config0Deviation.Enabled = true;
					break;
				case Sirena.ConfigChange.Saved:
					MessageBox.Show("Конфигурации сохранены.", "Конфигурации", MessageBoxButtons.OK, MessageBoxIcon.Information);
					saveConfig.Enabled = true;
					break;
			}
		}

		private void Sirena_EventConfig(List<SirenaConfig> config)
		{
			config0PressMax.Enabled = false;
			config0IdleMax.Enabled = false;
			config0PulsesMax.Enabled = false;
			config0Deviation.Enabled = false;
			config0Button.Enabled = false;
			config0Measure.Enabled = false;
			config0PressMax.Value = config[0].pressMax;
			config0IdleMax.Value = config[0].idleMax;
			config0PulsesMax.Value = config[0].pulsesMax;
			config0Deviation.Value = config[0].deviation;
			config0Button.Checked = config[0].button;
			config0Measure.Checked = config[0].measure;
			config0PressMax.Enabled = true;
			config0IdleMax.Enabled = true;
			config0PulsesMax.Enabled = true;
			config0Deviation.Enabled = true;
			config0Button.Enabled = true;
			config0Measure.Enabled = true;
			sirena.GetTables();
		}

		private void Sirena_EventTables(SirenaTables tables)
		{
			listEvents.Tag = tables.eventsMaximum;
			listMelodys.Tag = tables.melodysMaximum;
			listEvents.Items.Clear();
			listMelodys.Items.Clear();
			foreach (var m in tables.melodys) addMelodyItem(m);
			foreach (var e in tables.events) listEventUpdateItem(addEventItem(e));
			DataLoaded();
		}

		private void DataLoaded()
		{
			PinCode.Enabled = true;
			changePinCode.Enabled = true;
			addMelody.Enabled = true;
			addEvent.Enabled = true;
			clearMelodys.Enabled = true;
			deleteMelody.Enabled = true;
			playMelody.Enabled = true;
			stopMelody.Enabled = true;
			playMelody.Visible = false;
			stopMelody.Visible = false;
			clearEvents.Enabled = true;
			deleteEvent.Enabled = true;
			editEvent.Enabled = true;
			saveFS.Enabled = true;
			saveConfig.Enabled = true;
			config0PressMax.Enabled = true;
			config0IdleMax.Enabled = true;
			config0PulsesMax.Enabled = true;
			config0Deviation.Enabled = true;
			config0Button.Enabled = true;
			config0Measure.Enabled = true;
			waitScreen.Hide();
		}

		private void addMelodyItem(SirenaMelody m)
		{
			listMelodys.Items.Add(new ListViewItem()
			{
				Tag = m.index,
				Text = m.index.ToString(),
				SubItems = { m.name }
			});
		}

		private ListViewItem addEventItem(SirenaEvent e)
		{
			return listEvents.Items.Add(new ListViewItem()
			{
				Tag = e,
				Text = e.index.ToString(),
				SubItems = { "", "", "", "", "" }
			});
		}

		private void listEventUpdateItem(ListViewItem item)
		{
			SirenaEvent e = (SirenaEvent)item.Tag;
			item.SubItems[1].Text = ((e.poweroff) ? "*" : "");
			item.SubItems[2].Text = e.chIndex.ToString();
			string type = "-";
			switch (e.type)
			{
				case SirenaEvent.PulseType.Press:
					item.SubItems[3].Text = "удержание";
					break;
				case SirenaEvent.PulseType.Cyclic:
					item.SubItems[3].Text = "циклический";
					break;
				default:
					item.SubItems[3].Text = "импульсы";
					type = "";
					foreach (var p in e.pulse)
					{
						if (type != "") type += ", ";
						type += p.width.ToString() + ":" + p.count.ToString();
					}
					break;
			}
			item.SubItems[4].Text = type;
			if (e.melodyIndex < 0)
			{
				item.SubItems[5].Text = "не установлена";
			}
			else
			{
				ListViewItem i = null;
				foreach (ListViewItem f in listMelodys.Items)
				{
					if ((int)f.Tag == e.melodyIndex)
					{
						i = f;
						break;
					}
				}
				if (i == null)
				{
					item.SubItems[5].Text = "отсутствует";
				}
				else
				{
					item.SubItems[5].Text = i.SubItems[0].Text + ": " + i.SubItems[1].Text;
				}
			}
		}

		private void Sirena_EventDevices(List<string> ports)
		{
			switch (ports.Count)
			{
				case 0:
					waitScreen.ShowStatus("Ни одного устройства не найдено.");
					break;
				case 1:
					waitScreen.ShowStatus("*** Подключаемся ***");
					sirena.Open(ports[0]);
					break;
				default:
					waitScreen.ShowStatus("Подключите одно устройство и перезапустите программу!");
					break;
			}
		}

		private void Sirena_EventFS(Sirena.FSChange type)
		{
			switch (type)
			{
				case Sirena.FSChange.Ready:
					waitScreen.ShowStatus("*** Загружаем данные ***");
					//sirena.GetPinCode();
					sirena.GetConfig();
					break;
				case Sirena.FSChange.Busy:
					FindSirena("Файловая система не инициализировалась.");
					break;
				case Sirena.FSChange.Save:
					MessageBox.Show("Мелодии и события сохранены", null, MessageBoxButtons.OK, MessageBoxIcon.Information);
					saveFS.Enabled = true;
					break;
				case Sirena.FSChange.SaveError:
					FindSirena("Ошибка сохранения, сбой файловой системы");
					break;
			}
		}

		private void FindSirena(string error = null)
		{
			if (eventForm != null) eventForm.Close();
			string msg = "*** Поиск SirenaBT5 ***";
			waitScreen.ShowStatus((error == null) ? msg : error + "\r\n" + msg);
			sirena.GetDevices(((error == null) ? 0 : 3000), debug);
		}

		private void Sirena_EventError(string description)
		{
			if (sirena.Closed)
			{
				this.Close();
			}
			else
			{
				FindSirena(description);
			}
		}

		private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
		{
			if (!sirena.Closed)
			{
				e.Cancel = true;
				sirena.Close();
			}
		}

		private void waitScreen_EventShow()
		{
			waitScreen.BringToFront();
		}

		private void listEvents_SelectedIndexChanged(object sender, EventArgs e)
		{
			if (listEvents.SelectedItems.Count == 0)
			{
				deleteEvent.Visible = false;
				editEvent.Visible = false;
			}
			else
			{
				deleteEvent.Visible = true;
				editEvent.Visible = true;
			}
		}

		private void deleteEvent_Click(object sender, EventArgs e)
		{
			deleteEvent.Enabled = false;
			SirenaEvent ev = (SirenaEvent)listEvents.SelectedItems[0].Tag;
			sirena.DeleteEvent(ev.index);
		}

		private void clearEvents_Click(object sender, EventArgs e)
		{
			clearEvents.Enabled = false;
			sirena.ClearEvent();
		}

		private void listMelodys_SelectedIndexChanged(object sender, EventArgs e)
		{
			if (listMelodys.SelectedItems.Count == 0)
			{
				deleteMelody.Visible = false;
				if (!stopMelody.Visible) playMelody.Visible = false;
			}
			else
			{
				deleteMelody.Visible = true;
				if (!stopMelody.Visible) playMelody.Visible = true;
			}
		}

		private void deleteMelody_Click(object sender, EventArgs e)
		{
			sirena.DeleteMelody((int)listMelodys.SelectedItems[0].Tag);
		}

		private void playMelody_Click(object sender, EventArgs e)
		{
			playMelody.Enabled = false;
			sirena.Play((int)listMelodys.SelectedItems[0].Tag);
		}

		private void stopMelody_Click(object sender, EventArgs e)
		{
			stopMelody.Enabled = false;
			sirena.PlayStop();
		}

		private void config0Button_CheckedChanged(object sender, EventArgs e)
		{
			if (config0Button.Enabled)
			{
				config0Button.Enabled = false;
				config0Measure.Enabled = false;
				config0Measure.Tag = ((config0Button.Checked) ? false : config0Measure.Checked);
				sirena.SetButton(0, config0Button.Checked);
			}
		}

		private void config0Measure_CheckedChanged(object sender, EventArgs e)
		{
			if (config0Measure.Enabled)
			{
				config0Measure.Enabled = false;
				config0Button.Enabled = false;
				config0Button.Tag = ((config0Measure.Checked) ? false : config0Button.Checked);
				sirena.SetMeasure(0, config0Measure.Checked);
			}
		}

		private void saveFS_Click(object sender, EventArgs e)
		{
			saveFS.Enabled = false;
			sirena.SaveFS();
		}

		private void saveConfig_Click(object sender, EventArgs e)
		{
			saveConfig.Enabled = false;
			sirena.SaveConfig();
		}

		private void clearMelodys_Click(object sender, EventArgs e)
		{
			clearMelodys.Enabled = false;
			sirena.ClearMelody();
		}

		private void addMelody_Click(object sender, EventArgs e)
		{
			addMelody.Enabled = false;
			if (listMelodys.Items.Count < (int)listMelodys.Tag)
			{
				string melodyPath = Application.StartupPath + "\\melody";
				if (!Directory.Exists(melodyPath)) melodyPath = Application.StartupPath;
				openFile.InitialDirectory = melodyPath;
				openFile.Multiselect = false;
				openFile.Filter = "Microsoft WAVE|*.wav";
				if (openFile.ShowDialog() != DialogResult.OK)
				{
					addMelody.Enabled = true;
				}
				else
				{
					if (sirena.SetMelody(openFile.FileName) == Wave.Valid.Sussed)
					{
						newMelodyName = Path.GetFileNameWithoutExtension(openFile.FileName);
					}
					else
					{
						MessageBox.Show("Файл испорчен или имеет неизвестный формат!", null, MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
						addMelody.Enabled = true;
					}
				}
			}
			else
			{
				MessageBox.Show("Превышено максимальное количество мелодий!", null, MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
				addMelody.Enabled = true;
			}
		}

		private void editEvent_Click(object sender, EventArgs e)
		{
			editEvent.Enabled = false;
			ListViewItem item = listEvents.SelectedItems[0];
			eventForm = new EventForm(listMelodys, item, true);
			if (eventForm.ShowDialog() == DialogResult.Cancel)
			{
				editEvent.Enabled = true;
			}
			else
			{
				listEventUpdateItem(item);
				sirena.SetEvent((SirenaEvent)item.Tag);
			}
			eventForm = null;
		}

		private void addEvent_Click(object sender, EventArgs e)
		{
			addEventItem();
		}

		private void addEventItem(SirenaChannelEvent e = null)
		{
			addEvent.Enabled = false;
			if (listEvents.Items.Count < (int)listEvents.Tag)
			{
				int index = 0;
				bool flag;
				do
				{
					flag = false;
					foreach (ListViewItem i in listEvents.Items)
					{
						if (((SirenaEvent)i.Tag).index == index)
						{
							index++;
							flag = true;
							break;
						}
					}
				}
				while (flag);
				ListViewItem item = addEventItem(new SirenaEvent() { index = index });
				eventForm = new EventForm(listMelodys, item, false, e);
				if (eventForm.ShowDialog() == DialogResult.Cancel)
				{
					item.Remove();
					addEvent.Enabled = true;
				}
				else
				{
					listEventUpdateItem(item);
					sirena.SetEvent((SirenaEvent)item.Tag);
				}
				eventForm = null;
			}
			else
			{
				MessageBox.Show("Превышено максимальное количество событий!", null, MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
				addEvent.Enabled = true;
			}
		}

		private void changePinCode_Click(object sender, EventArgs e)
		{
			if (PinCode.Text.Length == 4)
			{
				changePinCode.Enabled = false;
				PinCode.Enabled = false;
				sirena.SetPinCode(PinCode.Text);
			}
			else
			{
				MessageBox.Show("Введите 4 цифры!", null, MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
			}
		}

		private void config0PressMax_ValueChanged(object sender, EventArgs e)
		{
			if (config0PressMax.Enabled)
			{
				config0PressMax.Enabled = false;
				sirena.SetPressMaximum(0, (int)config0PressMax.Value);
			}
		}

		private void config0IdleMax_ValueChanged(object sender, EventArgs e)
		{
			if (config0IdleMax.Enabled)
			{
				config0IdleMax.Enabled = false;
				sirena.SetIdleMaximum(0, (int)config0IdleMax.Value);
			}
		}

		private void config0PulsesMax_ValueChanged(object sender, EventArgs e)
		{
			if (config0PulsesMax.Enabled)
			{
				config0PulsesMax.Enabled = false;
				sirena.SetPulsesMaximum(0, (int)config0PulsesMax.Value);
			}
		}

		private void config0Deviation_ValueChanged(object sender, EventArgs e)
		{
			if (config0Deviation.Enabled)
			{
				config0Deviation.Enabled = false;
				sirena.SetDeviation(0, (int)config0Deviation.Value);
			}
		}

		private void clearLog_Click(object sender, EventArgs e)
		{
			flowLog.Controls.Clear();
		}

		private void aboutMessage_Click(object sender, EventArgs e)
		{
			MessageBox.Show("Версия ПО: " + System.Reflection.Assembly.GetExecutingAssembly().GetName().Version.ToString() + "\r\nВерсия встроенного ПО: " + version + "\r\n\r\nnatalia@beephorn.com", "О программе", MessageBoxButtons.OK, MessageBoxIcon.Information);
		}

		private void toLink(string URI)
		{ 
			System.Diagnostics.Process.Start(URI);
		}

		private void linkYoutube_Click(object sender, EventArgs e)
		{
			toLink("https://www.youtube.com/channel/UCaenJ8IjDf8UTXYKDcMIung");
		}

		private void linkDrive2_Click(object sender, EventArgs e)
		{
			toLink("https://www.drive2.ru/users/hiser/");
		}

		private void linkVK_Click(object sender, EventArgs e)
		{
			toLink("https://vk.com/beephorn");
		}

		private void linkSite_Click(object sender, EventArgs e)
		{
			toLink("https://autobzik.com/");
		}

		private void linkBeepHorn_Click(object sender, EventArgs e)
		{
			toLink("https://beephorn.com/");
		}

		private void linkInstagram_Click(object sender, EventArgs e)
		{
			toLink("https://www.instagram.com/hiser_litvin/");
		}
	}
}
