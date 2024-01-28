using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Management;
using System.IO;
using System.IO.Ports;
using System.Threading;
using System.Text.RegularExpressions;

namespace SirenaBT5Updater
{

	public partial class MainForm : Form
	{
		private string usePort = null;
		string dataRAW = null;
		bool test = false;
		SerialPort serial;
		string[] packet;
		int packetIndex;
		public MainForm(string port = null)
		{
			InitializeComponent();
			usePort = port;
		}

		private void MainForm_Load(object sender, EventArgs e)
		{
			serial = new SerialPort();
			serial.BaudRate = 9600; /* для BT порта не важна скорость */
			serial.Parity = Parity.None;
			serial.ReadBufferSize = 2048;
			serial.WriteBufferSize = 2048;
			serial.WriteTimeout = 1000;
			serial.ReadTimeout = 1000;
			serial.ReceivedBytesThreshold = 1;
			serial.StopBits = StopBits.One;
			serial.RtsEnable = false;
			serial.Handshake = Handshake.None;
			serial.NewLine = "\r\n";
			serial.DataReceived += Serial_DataReceived;
			if (usePort == null)
			{
				AddStatus("Поиск сирены ...");
				Show();
				ManagementObjectCollection search = new System.Management.ManagementObjectSearcher("SELECT * FROM Win32_SerialPort WHERE PNPDeviceID LIKE 'BTHENUM\\\\{00001101-0000-1000-8000-00805F9B34FB}_LOCALMFG&%' AND NOT PNPDeviceID LIKE '%_LOCALMFG&0000%'").Get();
				List<string> result = new List<string>();
				try
				{
					foreach (ManagementObject device in search)
					{

						result.Add(device["DeviceID"].ToString());
					}
				}
				catch (Exception err)
				{
					AddStatus("!!!FATAL Не удалось получить список портов! " + err.ToString());
				}
				if (result.Count == 0)
				{
					AddStatus("Порты не найдены, сопрягите сирену!");
				}
				else if (result.Count > 1)
				{
					AddStatus("Найдено несколько портов, запустите программу с параметром!");
					AddStatus("\tSirenaBT5Updater.exe COMn");
					foreach (string p in result)
					{
						AddStatus("\t\tПорт '" + p + "'");
					}
				}
				else
				{
					open(result[0]);
				}
			}
			else
			{
				Show();
				open(usePort);
			}
		}

		private void open(string port, int delay = 0)
		{
			usePort = port;
			if (delay > 0)
			{
				AddStatus("Подключаемся '" + port + "' через " + delay + " секунд ...");
				new Thread(_open).Start(delay);
			}
			else
			{
				AddStatus("Подключаемся '" + port + "' ...");
				_open(0);
			}
		}

		private void _open(object delay)
		{
			Thread.Sleep((int)delay);
			dataRAW = "";
			serial.PortName = usePort;
			try
			{
				serial.Open();
				AddStatus("Подключились.");
				send((test) ? "^MAIN" : "^BOOT");
			}
			catch (Exception err)
			{
				switch (System.Runtime.InteropServices.Marshal.GetHRForException(err))
				{
					case -2147024775:
						AddStatus("!!!ERR Не удалось подключится!" /*+ err.ToString()*/);
						break;
					default:
						AddStatus("!!!ERR Не удалось открыть порт! " /*+ err.ToString()*/);
						break;
				}
				open(usePort, 3);
			}
		}

		private void close()
		{
			if (serial.IsOpen)
			{
				serial.DiscardInBuffer();
				serial.DiscardOutBuffer();
				new Thread(this._close).Start();
			}
		}
		private void _close()
		{
			serial.Close();
		}

		private void Serial_DataReceived(object sender, SerialDataReceivedEventArgs e)
		{
			if (serial.IsOpen)
			{
				int bytes = serial.BytesToRead;
				if (bytes > 0)
				{
					byte[] buffer = new byte[bytes];
					serial.Read(buffer, 0, bytes);
					dataRAW += Encoding.ASCII.GetString(buffer);
				}
				int firstCmdEnd;
				while ((firstCmdEnd = dataRAW.IndexOf("\r\n")) != -1)
				{
					if (dataRAW[0] == '+')
					{
						string answer = dataRAW.Substring(1, firstCmdEnd - 1);
						int argvStart = answer.IndexOf(':');
						if (argvStart == -1)
						{
							recv(answer, null);
						}
						else
						{
							string command = answer.Substring(0, argvStart);
							string argvRAW = answer.Substring(argvStart + 1, answer.Length - argvStart - 1);
							List<string> argv = new List<string>();
							int argvDelimiter;
							while ((argvDelimiter = argvRAW.IndexOf(';')) != -1)
							{
								argv.Add(argvRAW.Substring(0, argvDelimiter));
								argvRAW = argvRAW.Substring(argvDelimiter + 1, argvRAW.Length - argvDelimiter - 1);
							}
							if (argvRAW.Length != 0)
							{
								argv.Add(argvRAW);
							}
							recv(command, argv);
						}
					}
					dataRAW = dataRAW.Substring(firstCmdEnd + 2, dataRAW.Length - firstCmdEnd - 2);
				}
			}
		}

		private void fwInit()
		{
			packet = Regex.Split(Properties.Resources.firmware, "\r\n");
			packetIndex = 0;
		}

		private void fwPacket()
		{
			send("PACKET=" + packetIndex + ";" + packet[packetIndex++]);
		}
		private bool fwEnd()
		{
			return (packetIndex >= packet.Length || packet[packetIndex] == "");
		}

		private void recv(string command, List<string> argv)
		{
			switch (command)
			{
				case "PACKET":
					if (argv[0] == "OK")
					{
						AddStatus("Записано " + argv[1]);
						if (fwEnd())
						{
							test = true;
							send("^MAIN");
						}
						else
						{
							fwPacket();
						}
					}
					else
					{
						close();
						AddStatus("!!!FATAL Не удалось записать!\r\n\tПопробуйте перезапустить программу.");
					}
					break;
				case "^BOOT":
					if (argv[0] == "READY")
					{
						AddStatus("Начинаем обновление ...");
						fwInit();
						fwPacket();
					}
					else
					{
						close();
						AddStatus("Переключаем сирену в режим загрузки ...");
						open(usePort, 15);
					}
					break;
				case "^MAIN":
					if (argv[0] == "READY")
					{
						send("INFO");
					}
					else
					{
						close();
						AddStatus("Переключаем сирену в нормальный режим ...");
						open(usePort, 15);
					}
					break;
				case "INFO":
					AddStatus("Установленное ПО: " + JSON.GetDouble("version", argv[0]) +  " сборка " + JSON.GetInt("build", argv[0]));
					break;
			}
		}

		private void send(string command)
		{
			if (serial.IsOpen)
			{
				try
				{
					serial.WriteLine(command);
				}
				catch (Exception err)
				{
					close();
					AddStatus("!!!FATAL Не удалось отправить данные! " /*+ err.ToString()*/);
				}
			}
		}

		private delegate void EventHandlerAddStatus(string text);
		private void AddStatus(string text)
		{
			if (InvokeRequired)
			{
				Invoke(new EventHandlerAddStatus(_AddStatus), new object[] { text });
			}
			else
			{
				_AddStatus(text);
			}
		}
		private void _AddStatus(string text)
		{
			status.Text += text + "\r\n";
			Application.DoEvents();
		}
	}
}
