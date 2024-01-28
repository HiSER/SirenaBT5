using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Management;
using System.IO;
using System.IO.Ports;
using System.Threading;
using System.Text.RegularExpressions;

namespace SirenaBT5
{
	public class JSON
	{
		public static int GetInt(string name, string json)
		{
			Match m = (new Regex("\"" + name + "\":([0-9-]+)")).Match(json);
			string value = m.Groups[1].ToString();
			return Convert.ToInt32(value);
		}
		public static double GetDouble(string name, string json)
		{
			Match m = (new Regex("\"" + name + "\":([0-9-\\.]+)")).Match(json);
			string value = m.Groups[1].ToString();
			//return Convert.ToDouble(value);
			return double.Parse(value, System.Globalization.CultureInfo.InvariantCulture);
		}
		public static string GetString(string name, string json)
		{
			Match m = (new Regex("\"" + name + "\":\"([0-9a-zA-Z+/=]*)\"")).Match(json);
			string value = m.Groups[1].ToString();
			return value;
		}
		public static string GetStringBase64(string name, string json)
		{
			byte[] b = Convert.FromBase64String(GetString(name, json));
 			string value = Encoding.UTF8.GetString(b);
			return value;
		}
		public static bool GetBool(string name, string json)
		{
			Match m = (new Regex("\"" + name + "\":(true|false)")).Match(json);
			string value = m.Groups[1].ToString();
			return (value == "true");
		}
		public static string GetArray(string name, string json)
		{
			Match m = (new Regex("\"" + name + "\":\\[([^\\]]*)\\]")).Match(json);
			string value = m.Groups[1].ToString();
			return value;
		}
		public static List<int> GetArrayIntItems(string array)
		{
			List<int> result = new List<int>();
			foreach (Match m in (new Regex(@"\d+")).Matches(array))
			{
				result.Add(Convert.ToInt32(m.ToString()));
			}
			return result;
		}
		public static List<string> GetArrayObjectItems(string array)
		{
			List<string> result = new List<string>();
			foreach (Match m in (new Regex(@"\{([^\}]+)\}")).Matches(array))
			{
				result.Add(m.ToString());
			}
			return result;
		}
	}

	public class Wave
	{
		public Valid valid;
		public string name;
		public int samples;
		public int length;
		public int offset;
		public byte[] buffer;
		public enum Valid : int
		{
			Sussed = 0,
			NotFound,
			ErrorFile,
			ErrorFormat,
			ErrorUnknown
		}
		private class Chunk
		{
			public int start;
			public int length;
			public int offset;
			public byte[] buffer;
			public Chunk()
			{
				start = -1;
				length = -1;
				offset = 0;
				buffer = null;
			}
		}

		public Wave(string filename)
		{
			byte[] b = Encoding.UTF8.GetBytes(Path.GetFileNameWithoutExtension(filename));
			name = Convert.ToBase64String(b, 0, ((b.Length > 512) ? 512 : b.Length));
			valid = Valid.ErrorUnknown;
			if (File.Exists(filename))
			{
				Chunk riff;
				Chunk fmt;
				Chunk data;
				riff = findChunk(filename, "RIFF", null, 4);
				riff.offset = 4;
				fmt = findChunk(filename, "fmt ", riff, 16);
				data = findChunk(filename, "data", riff);
				if (riff.start != -1 && Encoding.ASCII.GetString(riff.buffer) == "WAVE" && fmt.start != -1 && data.start != -1)
				{
					int format = (fmt.buffer[1] << 8) | fmt.buffer[0];
					int channels = (fmt.buffer[3] << 8) | fmt.buffer[2];
					int bits = fmt.buffer[14];
					samples = (fmt.buffer[7] << 24) | (fmt.buffer[6] << 16) | (fmt.buffer[5] << 8) | fmt.buffer[4];
					this.length = data.length;
					if (format == 1 && channels == 1 && bits == 8 && samples >= 8000 && samples <= 48000 && length > 0 && length <= (1024 * 1024))
					{
						FileStream stream = File.OpenRead(filename);
						if (stream.Length >= (riff.length + 8))
						{
							buffer = new byte[data.length];
							stream.Position = data.start;
							stream.Read(buffer, 0, data.length);
							offset = 0;
							valid = Valid.Sussed;
						}
						else
						{
							valid = Valid.ErrorFile;
						}
						stream.Close();
					}
					else
					{
						valid = Valid.ErrorFormat;
					}
				}
				else
				{
					valid = Valid.ErrorFile;
				}
			}
			else
			{
				valid = Valid.NotFound;
			}
		}

		public static byte[] encode7(byte[] buffer)
		{
			byte[] encode = new byte[buffer.Length];
			int i;
			for (i = 0; i < buffer.Length; i++)
			{
				encode[i] = toChar(buffer[i] >> 1);
			}
			return encode;
		}

		private static byte toChar(int c)
		{
			c = (byte)(c & 0x7F);
			if (c >= 0 && c < 26) return (byte)(65 + c); /* A-Z */
			if (c >= 26 && c < 52) return (byte)(97 + (c - 26)); /* a-z */
			if (c >= 52 && c < 62) return (byte)(48 + (c - 52)); /* 0-9 */
			if (c >= 62 && c < 126) return (byte)(192 + (c - 62)); /* а-я А-Я */
			if (c == 126) return 43; /* + */
			if (c == 127) return 47; /* / */
			return 0;
		}

		private Chunk findChunk(string filename, string chunkName, Chunk area = null, int load = 0)
		{
			Chunk chunk = new Chunk();
			FileStream stream = File.OpenRead(filename);
			byte[] buffer = new byte[8];
			int EndPosition = (int)stream.Length;
			if (area != null)
			{
				stream.Position = area.start + area.offset;
				EndPosition = area.start + area.length;
				if (EndPosition > stream.Length) EndPosition = (int)stream.Length;
			}
			while (stream.Position < EndPosition)
			{
				if (stream.Read(buffer, 0, 8) != 8) break;
				if (Encoding.ASCII.GetString(buffer, 0, 4) == chunkName)
				{
					int start = (int)stream.Position;
					int length = (buffer[7] << 24) | (buffer[6] << 16) | (buffer[5] << 8) | buffer[4];
					if (load == 0)
					{
						chunk.start = start;
						chunk.length = length;
					}
					else
					{
						if (load <= length)
						{
							if (load < 0) load = length;
							chunk.buffer = new byte[load];
							if (stream.Read(chunk.buffer, 0, load) == load)
							{
								chunk.start = start;
								chunk.length = length;
							}
							else
							{
								chunk.buffer = null;
							}
						}
					}
					break;
				}
				else
				{
					stream.Position += (buffer[7] << 24) | (buffer[6] << 16) | (buffer[5] << 8) | buffer[4];
				}
			}
			stream.Close();
			return chunk;
		}

	}

	public class SirenaMelody
	{
		public int index;
		public string name;
	}
	public class SirenaPulse
	{
		public int width;
		public int count;
	}
	public class SirenaEvent
	{
		public int index;
		public PulseType type;
		public int chIndex;
		public int melodyIndex;
		public bool poweroff;
		public List<SirenaPulse> pulse;

		public enum PulseType : int
		{
			Null = 0,
			Press,
			Release,
			Cyclic,
			Pulse,
			Measure,
			MeasureError
		}

		public SirenaEvent()
		{
			pulse = new List<SirenaPulse>();
		}
	}
	public class SirenaTables
	{

		public List<SirenaMelody> melodys;
		public List<SirenaEvent> events;
		public int melodysMaximum;
		public int eventsMaximum;

		private int melodysLoaded;
		private int eventsLoaded;

		public SirenaTables(string json)
		{
			melodysMaximum = JSON.GetInt("melodyMax", json);
			eventsMaximum = JSON.GetInt("eventMax", json);
			melodys = new List<SirenaMelody>();
			foreach (int value in JSON.GetArrayIntItems(JSON.GetArray("melody", json)))
			{
				melodys.Add(new SirenaMelody() { index = value });
			}
			events = new List<SirenaEvent>();
			foreach (int value in JSON.GetArrayIntItems(JSON.GetArray("event", json)))
			{
				events.Add(new SirenaEvent() { index = value });
			}
			melodysLoaded = 0;
			eventsLoaded = 0;
		}

		public bool SetName(string json)
		{
			int index = JSON.GetInt("index", json);
			SirenaMelody melodyFind = melodys.Find(m => m.index == index);
			if (melodyFind != null)
			{
				melodyFind.name = JSON.GetStringBase64("name", json);
			}
			melodysLoaded++;
			return IsComplete();
		}

		public bool SetEvent(string json)
		{
			int index = JSON.GetInt("index", json);
			SirenaEvent eventFind = events.Find(e => e.index == index);
			if (eventFind != null)
			{
				eventFind.chIndex = JSON.GetInt("chIndex", json);
				eventFind.melodyIndex = JSON.GetInt("melodyIndex", json);
				eventFind.poweroff = JSON.GetBool("poweroff", json);
				switch (JSON.GetString("type", json))
				{
					case "press":
						eventFind.type = SirenaEvent.PulseType.Press;
						break;
					case "cyclic":
						eventFind.type = SirenaEvent.PulseType.Cyclic;
						break;
					case "pulse":
						eventFind.type = SirenaEvent.PulseType.Pulse;
						foreach (string o in JSON.GetArrayObjectItems(JSON.GetArray("pulse", json)))
						{
							eventFind.pulse.Add(new SirenaPulse() { width = JSON.GetInt("width", o), count = JSON.GetInt("count", o) });
						}
						break;
				}
			}
			eventsLoaded++;
			return IsComplete();
		}

		private bool IsComplete()
		{
			return (melodysLoaded == melodys.Count && eventsLoaded == events.Count);
		}
	}
	public class SirenaConfig
	{
		public int pressMax;
		public int idleMax;
		public int pulsesMax;
		public int deviation;
		public bool button;
		public bool measure;

		public SirenaConfig(string json = "")
		{
			if (json != "")
			{
				pressMax = JSON.GetInt("pressMax", json);
				idleMax = JSON.GetInt("idleMax", json);
				pulsesMax = JSON.GetInt("pulsesMax", json);
				deviation = JSON.GetInt("deviation", json);
				button = JSON.GetBool("button", json);
				measure = JSON.GetBool("measure", json);
			}
		}
	}
	public class SirenaChannelEvent
	{
		public int index;
		public SirenaEvent.PulseType type;
		public List<SirenaPulse> pulse;

		public SirenaChannelEvent(string json)
		{
			index = JSON.GetInt("index", json);
			pulse = new List<SirenaPulse>();
			switch (JSON.GetString("type", json))
			{
				case "press":
					type = SirenaEvent.PulseType.Press;
					break;
				case "cyclic":
					type = SirenaEvent.PulseType.Cyclic;
					break;
				case "release":
					type = SirenaEvent.PulseType.Release;
					break;
				case "pulse":
					type = SirenaEvent.PulseType.Pulse;
					foreach (string o in JSON.GetArrayObjectItems(JSON.GetArray("pulse", json)))
					{
						pulse.Add(new SirenaPulse() { width = JSON.GetInt("width", o), count = JSON.GetInt("count", o) });
					}
					break;
			}
		}
	}

	public class Sirena
	{
		public delegate void EventHandlerError(string description);
		public delegate void EventHandlerDevices(List<string> ports);
		public delegate void EventHandlerFS(FSChange type);
		public delegate void EventHandlerTables(SirenaTables tables);
		public delegate void EventHandlerConfig(List<SirenaConfig> config);
		public delegate void EventHandlerConfigChange(ConfigChange type, int index);
		public delegate void EventHandlerMelodyChange(MelodyChange type, int value);
		public delegate void EventHandlerEventChange(EventChange type, int index);
		public delegate void EventHandlerPinChange(PinChange type, string pin);
		public delegate void EventHandlerPlayChange(PlayChange type, int index);
		public delegate void EventHandlerChannel(SirenaChannelEvent e);
		public delegate void EventHandlerMeasure(int index, int widthMax, int idleMin);
		public delegate void EventHandlerInfo(double version, int build);

		public event EventHandlerError EventError;
		public event EventHandlerDevices EventDevices;
		public event EventHandlerFS EventFS;
		public event EventHandlerTables EventTables;
		public event EventHandlerConfig EventConfig;
		public event EventHandlerConfigChange EventConfigChange;
		public event EventHandlerMelodyChange EventMelodyChange;
		public event EventHandlerEventChange EventEventChange;
		public event EventHandlerPinChange EventPinChange;
		public event EventHandlerPlayChange EventPlayChange;
		public event EventHandlerChannel EventChannel;
		public event EventHandlerMeasure EventMeasure;
		public event EventHandlerInfo EventInfo;

		public System.Windows.Forms.Form owner;
		public bool Closed;

		public enum FSChange : int
		{
			Ready = 0,
			Busy,
			Save,
			SaveError
		}
		public enum ConfigChange : int
		{
			Change = 0,
			ChangeButton,
			ChangeMeasure,
			ChangePressMaximum,
			ChangeIdleMaximum,
			ChangePulsesMaximum,
			ChangeDeviation,
			Saved
		}
		public enum MelodyChange : int
		{
			Start = 0,
			Progress,
			End,
			Full,
			Delete,
			Clear,
			Error
		}
		public enum EventChange : int
		{
			Change = 0,
			Delete,
			Clear
		}
		public enum PinChange : int
		{
			Change = 0,
			Request,
			Error
		}
		public enum PlayChange : int
		{
			Play = 0,
			Stop
		}

		private SerialPort serial;
		private string dataRAW;
		private int waitAnswers;
		private System.Timers.Timer waitAnswersTimer;
		private SirenaTables tables;
		private Wave wave;

		public Sirena()
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
			waitAnswersTimer = new System.Timers.Timer(3000);
			waitAnswersTimer.AutoReset = false;
			waitAnswersTimer.Elapsed += WaitTimer_Elapsed;
			owner = null;
			wave = null;
			tables = null;
			Closed = false;
		}

		public void GetDevices(int sleep = 0, string forDebug = null)
		{
			if (forDebug == null)
			{
				 new Thread(this.__GetDevices).Start(sleep);
			}
			else
			{
				new Timer((object state) => { if (Closed) return; invokeEventDevices(new List<string>() { forDebug }); }, null, sleep, -1);
			}
		}
		private void __GetDevices(object sleep)
		{
			if (Closed) return;
			Thread.Sleep((int)sleep);
			List<string> result = new List<string>();
			ManagementObjectCollection search = new System.Management.ManagementObjectSearcher("SELECT * FROM Win32_SerialPort WHERE PNPDeviceID LIKE 'BTHENUM\\\\{00001101-0000-1000-8000-00805F9B34FB}_LOCALMFG&%' AND NOT PNPDeviceID LIKE '%_LOCALMFG&0000%'").Get();
			try
			{
				foreach (ManagementObject device in search)
				{

					result.Add(device["DeviceID"].ToString());
					/*Console.WriteLine("--- Sirena.GetDevices -------------");
					Console.WriteLine(device["Caption"].ToString());
					Console.WriteLine(device["DeviceID"].ToString());
					Console.WriteLine(device["PNPDeviceID"].ToString());
					Console.WriteLine("-----------------------------------");*/
				}
			}
			catch (Exception e)
			{
				/*Console.WriteLine("--- Sirena.GetDevices -------------");
				Console.WriteLine(e.ToString());
				Console.WriteLine("-----------------------------------");*/
				invokeEventError("Не удалось получить список портов!\r\n" + e.ToString());
			}
			invokeEventDevices(result);
		}

		public void Open(string port)
		{
			disable();
			serial.PortName = port;
			new Thread(this.__Open).Start();
		}
		private void __Open()
		{
			if (Closed) return;
			dataRAW = "";
			waitAnswers = 0;
			try
			{
				serial.Open();
				sendRAW("INFO");
			}
			catch (Exception e)
			{
				/*Console.WriteLine("--- Sirena.Open -------------");
				Console.WriteLine(e.ToString());
				Console.WriteLine("-----------------------------------");*/
				switch (System.Runtime.InteropServices.Marshal.GetHRForException(e))
				{
					case -2147024775:
						invokeEventError("Не удалось подключится!\r\nПопробуйте вывести сирену из дежурного режима.");
						break;
					/*case -2147024891:
						invokeEventError("Не удалось подключится!\r\nПорт занят другим приложением.");
						break;*/
					default:
						invokeEventError("Не удалось открыть порт!\n\r" + e.Message);
						break;
				}
			}
		}
		public void Close()
		{
			if (!Closed)
			{
				Closed = true;
				new Thread(this.__Close).Start();
			}
		}
		private void __Close()
		{
			disable();
			invokeEventError("объект закрыт");
		}

		public void GetTables()
		{
			sendRAW("LIST");
		}
		private void LoadListData()
		{
			if (tables != null)
			{
				new Thread(this.__LoadListData).Start();
			}
		}
		private void __LoadListData()
		{
			foreach (SirenaMelody m in tables.melodys)
			{
				sendRAW("GETMELODY=" + m.index.ToString());
				Thread.Sleep(50);
			}
			if (tables != null)
			{
				foreach (SirenaEvent e in tables.events)
				{
					sendRAW("GETEVENT=" + e.index.ToString());
					Thread.Sleep(50);
				}
			}
		}
		public Wave.Valid SetMelody(string filename)
		{
			sendRAW("ENDMELODY");
			wave = new Wave(filename);
			if (wave.valid == Wave.Valid.Sussed)
			{
				sendRAW("SETMELODY=" + wave.length.ToString() + ";" + wave.samples.ToString() + ";" + wave.name);
			}
			return wave.valid;
		}
		public void DeleteMelody(int index)
		{
			sendRAW("DELETEMELODY=" + index.ToString());
		}
		public void ClearMelody()
		{
			sendRAW("CLEARMELODY");
		}
		public void SetEvent(SirenaEvent e)
		{
			string cmd = "SETEVENT=" + e.index.ToString();
			cmd += ";" + e.melodyIndex.ToString();
			cmd += ";" + ((e.poweroff) ? "1" : "0");
			cmd += ";" + e.chIndex.ToString();
			switch (e.type)
			{
				case SirenaEvent.PulseType.Press:
					cmd += ";press";
					break;
				case SirenaEvent.PulseType.Cyclic:
					cmd += ";cyclic";
					break;
				case SirenaEvent.PulseType.Pulse:
					cmd += ";" + e.pulse.Count.ToString();
					foreach (var p in e.pulse)
					{
						cmd += ";" + p.width.ToString();
						cmd += ";" + p.count.ToString();
					}
					break;
			}
			sendRAW(cmd);
		}
		public void DeleteEvent(int index)
		{
			sendRAW("DELETEEVENT=" + index.ToString());
		}
		public void ClearEvent()
		{
			sendRAW("CLEAREVENT");
		}
		public void GetConfig()
		{
			sendRAW("GETCONFIG");
		}
		public void SetConfig(int index, SirenaConfig config)
		{
			string cmd = "SETCONFIG=" + index.ToString();
			cmd += ";" + config.pressMax.ToString();
			cmd += ";" + config.idleMax.ToString();
			cmd += ";" + config.pulsesMax.ToString();
			cmd += ";" + config.deviation.ToString();
			cmd += ";" + ((config.button) ? "1" : "0");
			cmd += ";" + ((config.measure) ? "1" : "0");
			sendRAW(cmd);
		}
		public void SetButton(int index, bool button)
		{
			sendRAW("SETBUTTON=" + index.ToString() + ";" + ((button) ? "1" : "0"));
		}
		public void SetMeasure(int index, bool measure)
		{
			sendRAW("SETMEASURE=" + index.ToString() + ";" + ((measure) ? "1" : "0"));
		}
		public void SetPressMaximum(int index, int value)
		{
			sendRAW("SETPRESSMAX=" + index.ToString() + ";" + value.ToString());
		}
		public void SetIdleMaximum(int index, int value)
		{
			sendRAW("SETIDLEMAX=" + index.ToString() + ";" + value.ToString());
		}
		public void SetPulsesMaximum(int index, int value)
		{
			sendRAW("SETPULSESMAX=" + index.ToString() + ";" + value.ToString());
		}
		public void SetDeviation(int index, int value)
		{
			sendRAW("SETDEVIATION=" + index.ToString() + ";" + value.ToString());
		}
		public void Play(int index)
		{
			sendRAW("PLAY=" + index.ToString());
		}
		public void PlayStop()
		{
			sendRAW("PLAYSTOP");
		}
		public void SaveFS()
		{
			sendRAW("SAVEFS");
		}
		public void SaveConfig()
		{
			sendRAW("SAVECONFIG");
		}
		public void GetPinCode()
		{
			sendRAW("PINCODE");
		}

		public void SetPinCode(string pincode)
		{
			sendRAW("PINCODE=" + pincode);
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
							recvRAW(answer, null);
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
							recvRAW(command, argv);
						}
					}
					dataRAW = dataRAW.Substring(firstCmdEnd + 2, dataRAW.Length - firstCmdEnd - 2);
				}
			}
		}
		private void WaitTimer_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
		{
			disable();
			invokeEventError("Таймаут ответа");
		}
		private void recvRAW(string command, List<string> argv)
		{
			if (waitAnswers > 0)
			{
				waitAnswers--;
				waitAnswersTimer.Stop();
			}
			/*Console.WriteLine("--------------");
			Console.WriteLine(command);
			foreach (string arg in argv)
			{
				Console.WriteLine('\t' + arg);
			}*/
			switch (command)
			{
				case "SPEAK":

					break;
				case "PLAY":
					invokeEventPlayChange(PlayChange.Play, JSON.GetInt("index", argv[0]));
					break;
				case "PLAYSTOP":
					if (argv[0] == "OK")
					{
						invokeEventPlayChange(PlayChange.Stop, -1);
					}
					else
					{
						invokeEventPlayChange(PlayChange.Stop, JSON.GetInt("index", argv[0]));
					}
					break;
				case "INFO":
					if (JSON.GetBool("fsReady", argv[0]))
					{
						invokeEventFS(FSChange.Ready);
						invokeEventInfo(JSON.GetDouble("version", argv[0]), JSON.GetInt("build", argv[0]));
						invokeEventPinChange(PinChange.Request, JSON.GetString("pincode", argv[0]));
					}
					else
					{
						disable();
						invokeEventFS(FSChange.Busy);
					}
					break;
				case "FS":
					if (argv[0] == "READY")
					{
						invokeEventFS(FSChange.Ready);
					}
					else
					{
						disable();
						invokeEventFS(FSChange.Busy);
					}
					break;
				case "SAVEFS":
					invokeEventFS((argv[0] == "OK") ? FSChange.Save : FSChange.SaveError);
					break;
				case "LIST":
					tables = new SirenaTables(argv[0]);
					if (tables.melodys.Count == 0 && tables.events.Count == 0)
					{
						invokeEventTables(tables);
					}
					else
					{
						LoadListData();
					}
					break;
				case "GETMELODY":
					if (tables.SetName(argv[0]))
					{
						invokeEventTables(tables);
						tables = null;
					}
					break;
				case "SETMELODY":
					if (argv[0] == "ERROR")
					{
						switch (argv[1])
						{
							case "FULL":
								invokeEventMelodyChange(MelodyChange.Full);
								break;
							default:
								invokeEventMelodyChange(MelodyChange.Error);
								break;
						}
					}
					else
					{
						invokeEventMelodyChange(MelodyChange.Start, JSON.GetInt("index", argv[0]));
						loadWave();
					}
					break;
				case "DATAMELODY":
					switch (argv[0])
					{
						case "OK":
							loadWave();
							break;
						case "END":
							sendRAW("ENDMELODY");
							wave = null;
							break;
						default:
							invokeEventMelodyChange(MelodyChange.Error);
							break;
					}
					break;
				case "ENDMELODY":
					invokeEventMelodyChange(MelodyChange.End);
					break;
				case "DELETEMELODY":
					invokeEventMelodyChange(MelodyChange.Delete, JSON.GetInt("index", argv[0]));
					break;
				case "CLEARMELODY":
					invokeEventMelodyChange(MelodyChange.Clear);
					break;
				case "GETEVENT":
					if (tables.SetEvent(argv[0]))
					{
						invokeEventTables(tables);
						tables = null;
					}
					break;
				case "SETEVENT":
					invokeEventEventChange(EventChange.Change, JSON.GetInt("index", argv[0]));
					break;
				case "DELETEEVENT":
					invokeEventEventChange(EventChange.Delete, JSON.GetInt("index", argv[0]));
					break;
				case "CLEAREVENT":
					invokeEventEventChange(EventChange.Clear);
					break;
				case "GETCONFIG":
					List<SirenaConfig> config = new List<SirenaConfig>();
					foreach (string o in JSON.GetArrayObjectItems(argv[0]))
					{
						config.Add(new SirenaConfig(o));
					}
					invokeEventConfig(config);
					break;
				case "SETCONFIG":
					invokeEventConfigChange(ConfigChange.Change, JSON.GetInt("index", argv[0]));
					break;
				case "SETBUTTON":
					invokeEventConfigChange(ConfigChange.ChangeButton, JSON.GetInt("index", argv[0]));
					break;
				case "SETMEASURE":
					invokeEventConfigChange(ConfigChange.ChangeMeasure, JSON.GetInt("index", argv[0]));
					break;
				case "SETPRESSMAX":
					invokeEventConfigChange(ConfigChange.ChangePressMaximum, JSON.GetInt("index", argv[0]));
					break;
				case "SETIDLEMAX":
					invokeEventConfigChange(ConfigChange.ChangeIdleMaximum, JSON.GetInt("index", argv[0]));
					break;
				case "SETPULSESMAX":
					invokeEventConfigChange(ConfigChange.ChangePulsesMaximum, JSON.GetInt("index", argv[0]));
					break;
				case "SETDEVIATION":
					invokeEventConfigChange(ConfigChange.ChangeDeviation, JSON.GetInt("index", argv[0]));
					break;
				case "SAVECONFIG":
					invokeEventConfigChange(ConfigChange.Saved);
					break;
				case "PINCODE":
					switch (argv[0])
					{
						case "OK":
							invokeEventPinChange(PinChange.Change);
							break;
						case "ERROR":
							invokeEventPinChange(PinChange.Error);
							break;
						default:
							invokeEventPinChange(PinChange.Request, argv[0]);
							break;
					}
					break;
				case "EVENT":
					switch (JSON.GetString("type", argv[0]))
					{
						case "measure":
							invokeEventMeasure(JSON.GetInt("index", argv[0]), JSON.GetInt("width", argv[0]), JSON.GetInt("idle", argv[0]));
							break;
						case "errMeasure":
							invokeEventMeasure(JSON.GetInt("index", argv[0]), -1, -1);
							break;
						default:
							invokeEventChannel(new SirenaChannelEvent(argv[0]));
							break;
					}
					break;
			}
			if (waitAnswers > 0)
			{
				waitAnswersTimer.Start();
			}
		}
		private void sendRAW(string command)
		{
			if (serial.IsOpen)
			{
				try
				{
					waitAnswers += ((command == "ATI") ? 4 : 1);
					serial.WriteLine(command);
					waitAnswersTimer.Start();
				}
				catch (Exception e)
				{
					disable();
					/*Console.WriteLine("--- Sirena.sendRAW -------------");
					Console.WriteLine(e.ToString());
					Console.WriteLine("-----------------------------------");*/
					invokeEventError("Не удалось отправить данные!\r\n" + e.Message);
				}
			}
		}

		private void disable()
		{
			waitAnswersTimer.Stop();
			if (serial.IsOpen)
			{
				serial.DiscardInBuffer();
				serial.DiscardOutBuffer();
				serial.Close();
			}
		}

		private void loadWave()
		{
			invokeEventMelodyChange(MelodyChange.Progress, wave.offset * 100 / wave.length);
			int length = wave.length - wave.offset;
			if (length > 512) length = 512;
			if (length > 0)
			{
				sendRAW("DATAMELODY=" + Convert.ToBase64String(wave.buffer, wave.offset, length));
				wave.offset += length;
			}
		}

		private void invokeEventError(string description)
		{
			if (EventError != null)
			{
				if (owner != null && owner.InvokeRequired)
				{
					owner.Invoke(new EventHandlerError(EventError), new object[] { description });
				}
				else
				{
					EventError(description);
				}
			}
		}
		private void invokeEventDevices(List<string> ports)
		{
			if (EventDevices != null)
			{
				if (owner != null && owner.InvokeRequired)
				{
					owner.Invoke(new EventHandlerDevices(EventDevices), new object[] { ports });
				}
				else
				{
					EventDevices(ports);
				}
			}
		}
		private void invokeEventFS(FSChange type)
		{
			if (EventFS != null)
			{
				if (owner != null && owner.InvokeRequired)
				{
					owner.Invoke(new EventHandlerFS(EventFS), new object[] { type });
				}
				else
				{
					EventFS(type);
				}
			}
		}
		private void invokeEventTables(SirenaTables tables)
		{
			if (EventTables != null)
			{
				if (owner != null && owner.InvokeRequired)
				{
					owner.Invoke(new EventHandlerTables(EventTables), new object[] { tables });
				}
				else
				{
					EventTables(tables);
				}
			}
		}
		private void invokeEventConfig(List<SirenaConfig> config)
		{
			if (EventConfig != null)
			{
				if (owner != null && owner.InvokeRequired)
				{
					owner.Invoke(new EventHandlerConfig(EventConfig), new object[] { config });
				}
				else
				{
					EventConfig(config);
				}
			}
		}
		private void invokeEventConfigChange(ConfigChange type, int index = 0)
		{
			if (EventConfigChange != null)
			{
				if (owner != null && owner.InvokeRequired)
				{
					owner.Invoke(new EventHandlerConfigChange(EventConfigChange), new object[] { type, index });
				}
				else
				{
					EventConfigChange(type, index);
				}
			}
		}
		private void invokeEventMelodyChange(MelodyChange type, int value = 0)
		{
			if (EventMelodyChange != null)
			{
				if (owner != null && owner.InvokeRequired)
				{
					owner.Invoke(new EventHandlerMelodyChange(EventMelodyChange), new object[] { type, value });
				}
				else
				{
					EventMelodyChange(type, value);
				}
			}
		}
		private void invokeEventEventChange(EventChange type, int index = 0)
		{
			if (EventEventChange != null)
			{
				if (owner != null && owner.InvokeRequired)
				{
					owner.Invoke(new EventHandlerEventChange(EventEventChange), new object[] { type, index });
				}
				else
				{
					EventEventChange(type, index);
				}
			}
		}
		private void invokeEventPinChange(PinChange type, string pin = "")
		{
			if (EventPinChange != null)
			{
				if (owner != null && owner.InvokeRequired)
				{
					owner.Invoke(new EventHandlerPinChange(EventPinChange), new object[] { type, pin });
				}
				else
				{
					EventPinChange(type, pin);
				}
			}
		}
		private void invokeEventPlayChange(PlayChange type, int index = 0)
		{
			if (EventPlayChange != null)
			{
				if (owner != null && owner.InvokeRequired)
				{
					owner.Invoke(new EventHandlerPlayChange(EventPlayChange), new object[] { type, index });
				}
				else
				{
					EventPlayChange(type, index);
				}
			}
		}
		private void invokeEventChannel(SirenaChannelEvent e)
		{
			if (EventChannel != null)
			{
				if (owner != null && owner.InvokeRequired)
				{
					owner.Invoke(new EventHandlerChannel(EventChannel), new object[] { e });
				}
				else
				{
					EventChannel(e);
				}
			}
		}
		private void invokeEventMeasure(int index, int widthMax, int idleMin)
		{
			if (EventMeasure != null)
			{
				if (owner != null && owner.InvokeRequired)
				{
					owner.Invoke(new EventHandlerMeasure(EventMeasure), new object[] { index, widthMax, idleMin });
				}
				else
				{
					EventMeasure(index, widthMax, idleMin);
				}
			}
		}
		private void invokeEventInfo(double version, int build)
		{
			if (EventInfo != null)
			{
				if (owner != null && owner.InvokeRequired)
				{
					owner.Invoke(new EventHandlerInfo(EventInfo), new object[] { version, build });
				}
				else
				{
					EventInfo(version, build);
				}
			}
		}
	}

}
