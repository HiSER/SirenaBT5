package com.hiserlitvin.sirenabt5;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.provider.Settings;
import android.util.Base64;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLConnection;
import java.net.URLEncoder;
import java.security.SecureRandom;
import java.security.cert.Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.Set;
import java.util.UUID;
import java.lang.String;

import javax.net.ssl.*;
import java.security.cert.X509Certificate;


public abstract class SirenaBT
{
	
	/* BT Events */
	public abstract void onAdapterNotFound();
	public abstract void onAdapterOff();
	public abstract void onAdapterOn();
	public abstract void onDeviceNotFound();
	public abstract void onDeviceConnected();
	public abstract void onDeviceError();
	public abstract void onWait();
	public abstract void onStartUpdate(int length);
	public abstract void onProgressUpdate(int index);
	public abstract void onSerialError();
	public abstract void onInternetError();
	public abstract void onBuildError();
	
	private String apiUrl = "https://artemlitvin.com/app/beephorn/5.0/api/";
	
	public static class Wave
	{
		public Valid valid;
		public String name;
		public int samples;
		public int length;
		public int offset;
		public byte[] buffer;
		public enum Valid
		{
			Sussed,
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
		
		public Wave(String filename)
		{
			File file = new File(filename);
			name = file.getName();
			int pos = name.lastIndexOf('.');
			if (pos > 0) name = name.substring(0, pos);
			if (name.length() > 256) name = name.substring(0, 256);
			name = new String(Base64.encode(name.getBytes(), Base64.NO_WRAP));
			buffer = loadFile(filename);
			valid = Valid.ErrorUnknown;
			if (buffer != null)
			{
				Chunk riff;
				Chunk fmt;
				Chunk data;
				riff = findChunk("RIFF", null, 4);
				riff.offset = 4;
				fmt = findChunk( "fmt ", riff, 16);
				data = findChunk( "data", riff, 0);
				if (riff.start != -1 && (new String(riff.buffer)).equals("WAVE") && fmt.start != -1 && data.start != -1)
				{
					int format = ((0xFF & fmt.buffer[1]) << 8) | (0xFF & fmt.buffer[0]);
					int channels = ((0xFF & fmt.buffer[3]) << 8) | (0xFF & fmt.buffer[2]);
					int bits = fmt.buffer[14];
					samples = ((0xFF & fmt.buffer[7]) << 24) | ((0xFF & fmt.buffer[6]) << 16) | ((0xFF & fmt.buffer[5]) << 8) | (0xFF & fmt.buffer[4]);
					this.length = data.length;
					if (format == 1 && channels == 1 && bits == 8 && samples >= 8000 && samples <= 48000 && length > 0 && length <= (1024 * 1024))
					{
						if ((data.start + data.length) <= buffer.length)
						{
							byte[] tmp = Arrays.copyOfRange(buffer, data.start, data.start + data.length);
							buffer = tmp;
							offset = 0;
							valid = Valid.Sussed;
						}
						else
						{
							buffer = null;
							valid = Valid.ErrorFormat;
						}
					}
					else
					{
						buffer = null;
						valid = Valid.ErrorFormat;
					}
				}
				else
				{
					buffer = null;
					valid = Valid.ErrorFile;
				}
			}
			else
			{
				valid = Valid.NotFound;
			}
		}
		
		private Chunk findChunk(String chunkName, Chunk area, int load)
		{
			Chunk chunk = new Chunk();
			int Position = 0;
			int EndPosition = buffer.length;
			byte[] fragment;
			int length;
			if (area != null)
			{
				Position = area.start + area.offset;
				EndPosition = area.start + area.length;
				if (EndPosition > buffer.length) EndPosition = buffer.length;
			}
			while ((Position + 8) <= EndPosition)
			{
				fragment = Arrays.copyOfRange(buffer, Position, Position + 8);
				Position += 8;
				length = ((0xFF & fragment[7]) << 24) | ((0xFF & fragment[6]) << 16) | ((0xFF & fragment[5]) << 8) | (0xFF & fragment[4]);
				if ((new String(fragment, 0, 4)).equals(chunkName))
				{
					chunk.start = Position;
					chunk.length = length;
					if (load != 0 && load <= length)
					{
						if (load < 0) load = length;
						if ((Position + load) <= EndPosition)
						{
							chunk.buffer = Arrays.copyOfRange(buffer, Position, Position + load);
							Position += load;
						}
					}
					break;
				}
				else
				{
					Position += length;
				}
			}
			return chunk;
		}
		
		private byte[] loadFile(String filename)
		{
			byte[] buffer = null;
			
			if (filename.toLowerCase().matches("http(.*)"))
			{
				try
				{
					int readed = 0;
					int length;
					InputStream stream;
					URL url = new URL(filename);
					URLConnection connection = url.openConnection();
					connection.connect();
					length = connection.getContentLength();
					if (length > 0)
					{
						stream = connection.getInputStream();
						buffer = new byte[length];
						while (readed < length) readed += stream.read(buffer, readed, length - readed);
						stream.close();
					}
				}
				catch (Exception e)
				{
					buffer = null;
					debug.p("loadFileURL " + e.getMessage());
				}
			}
			else
			{
				try
				{
					File file = new File(filename);
					if (file.exists())
					{
						int readed = 0;
						int length = (int)file.length();
						FileInputStream stream = new FileInputStream(file.getAbsoluteFile());
						buffer = new byte[length];
						while (readed < length) readed += stream.read(buffer, readed, length - readed);
						stream.close();
					}
				}
				catch (Exception e)
				{
					buffer = null;
					debug.p("loadFile " + e.getMessage());
				}
			}
			
			return buffer;
		}
	}
	
	/* Sirena Events */
	
	public static class Channel
	{
		int index = -1;
		int pressMax = 0;
		int idleMax = 0;
		int pulsesMax = 0;
		int deviation = 0;
		boolean button = false;
		boolean measure = false;
		boolean playOnce = false;
		boolean playToEnd = false;
		boolean noWidth = false;
		boolean noBounceHigh = true;
		boolean standard = false;
		
		public Channel(JSONObject json, int index)
		{
			this.index = index;
			try
			{
				pressMax = json.getInt("pressMax");
				idleMax = json.getInt("idleMax");
				pulsesMax = json.getInt("pulsesMax");
				deviation = json.getInt("deviation");
				button = json.getBoolean("button");
				measure = json.getBoolean("measure");
				/* 1331 */
				playOnce = json.getBoolean("playOnce");
				playToEnd = json.getBoolean("playToEnd");
				noWidth = json.getBoolean("noWidth");
				/* 1513 */
				noBounceHigh = json.getBoolean("noBounceHigh");
				/* 1778 */
				standard = json.getBoolean("standard");
			}
			catch (JSONException e)
			{
				debug.p("Channel " + index + " " + e.getMessage());
			}
		}
	}
	
	public static class Melody
	{
		public int index = -1;
		public String name = "";
		
		public Melody(JSONObject json)
		{
			try
			{
				index = json.getInt("index");
				name = json.getString("name");
				name = new String(Base64.decode(name, Base64.DEFAULT));
			}
			catch (JSONException e)
			{
				debug.p("Melody " + index + " " + e.getMessage());
			}
			catch (IllegalArgumentException e)
			{
				name = "Error name melody " + index;
				debug.p("Melody " + index + " name " + e.getMessage());
			}
		}
	}
	
	public enum PulseType
	{
		Null,
		Press,
		Release,
		Cyclic,
		Pulse,
		Measure,
		MeasureError
	}
	
	public static class SirenaPulse
	{
		public int width;
		public int count;
		
		public SirenaPulse(int width, int count)
		{
			this.width = width;
			this.count = count;
		}
	}
	
	public static class Event
	{
		public int index = -1;
		public int chIndex = 1;
		public int melodyIndex = -1;
		public boolean poweroff = false;
		public int volume = 10;
		public int noPlayAfter = -1;
		public PulseType type = PulseType.Null;
		public List<SirenaPulse> pulse = new ArrayList();
		
		public Event(JSONObject json)
		{
			try
			{
				index = json.getInt("index");
				chIndex = json.getInt("chIndex");
				melodyIndex = json.getInt("melodyIndex");
				poweroff = json.getBoolean("poweroff");
				String pulseType = json.getString("type");
				if (pulseType.equalsIgnoreCase("press"))
				{
					type = PulseType.Press;
				}
				else if (pulseType.equalsIgnoreCase("cyclic"))
				{
					type = PulseType.Cyclic;
				}
				else if (pulseType.equalsIgnoreCase("pulse"))
				{
					type = PulseType.Pulse;
					JSONArray pulsesArray = json.getJSONArray("pulse");
					JSONObject pulseObj;
					int i;
					for (i = 0; i < pulsesArray.length(); i++)
					{
						pulseObj = pulsesArray.getJSONObject(i);
						pulse.add(new SirenaPulse(pulseObj.getInt("width"), pulseObj.getInt("count")));
					}
				}
				/* 1513 */
				volume = json.getInt("volume");
				/* 1590 */
				noPlayAfter = json.getInt("noPlayAfter");
			}
			catch (JSONException e)
			{
				debug.p("Event " + index + " " + e.getMessage());
			}
		}
	}
	
	public static class Config
	{
		int speakSamples = 8000;
		int build = 0;
		double version = 0;
		String pincode = "";
		int melodyMax = 0;
		int eventMax = 0;
		int volume = 0;
		int volumeBeep = 0;
		boolean sleep = false;
		String serial = "SBT5";
		public List<Integer> melodyIndexs;
		public List<Integer> eventIndexs;
		public List<Channel> channels;
		public List<Melody> melodys;
		public List<Event> events;
		
		public Config(JSONObject json)
		{
			try
			{
				speakSamples = json.getInt("speakSamples");
			}
			catch (JSONException e)
			{
				debug.p("INFO: error speakSamples");
			}
			try
			{
				build = json.getInt("build");
				version = json.getDouble("version");
				pincode = json.getString("pincode");
				serial = json.getString("serial");
			}
			catch (JSONException e)
			{
				debug.p("Config " + e.getMessage());
			}
		}
		
		public void fromList(JSONObject json)
		{
			melodyIndexs = new ArrayList<>();
			eventIndexs = new ArrayList<>();
			melodys = new ArrayList<>();
			events = new ArrayList<>();
			JSONArray arr;
			int index;
			int i;
			try
			{
				melodyMax = json.getInt("melodyMax");
				eventMax = json.getInt("eventMax");
				arr = json.getJSONArray("melody");
				for (i = 0; i < arr.length(); i++)
				{
					index = arr.getInt(i);
					melodyIndexs.add(index);
				}
				arr = json.getJSONArray("event");
				for (i = 0; i < arr.length(); i++)
				{
					index = arr.getInt(i);
					eventIndexs.add(index);
				}
			}
			catch (JSONException e)
			{
				debug.p("fromList " + e.getMessage());
			}
		}
		
		public void fromGetConfig(JSONArray json)
		{
			int index;
			JSONObject cfg;
			channels = new ArrayList<>();
			for (index = 0; index < 3 && index < json.length(); index++)
			{
				try
				{
					cfg = json.getJSONObject(index);
					channels.add(new Channel(cfg, index));
				}
				catch (JSONException e)
				{
					debug.p("fromGetConfig " + index + " " + e.getMessage());
				}
			}
			/* 1722 */
			try
			{
				cfg = json.getJSONObject(3);
				volume = cfg.getInt("volume");
				volumeBeep = cfg.getInt("volumeBeep");
				sleep = cfg.getBoolean("sleep");
			}
			catch (JSONException e)
			{
				debug.p("fromGetConfig 3 " + e.getMessage());
			}
		}
		
		private boolean isMelodyEventLoaded()
		{
			return ((melodyIndexs.size() == melodys.size()) && (eventIndexs.size() == events.size()));
		}
		
		public boolean fromMelody(JSONObject json)
		{
			if (melodys != null) melodys.add(new Melody(json));
			return isMelodyEventLoaded();
		}
		
		public boolean fromEvent(JSONObject json)
		{
			if (events != null) events.add(new Event(json));
			return isMelodyEventLoaded();
		}
	};
	
	public abstract void onConfig(Config config);
	
	public enum SirenaSpeak
	{
		End
	}
	public abstract void  onSirenaSpeak(SirenaSpeak type);
	
	public enum SirenaPlay
	{
		Play,
		Stop
	}
	public abstract void onSirenaPlay(SirenaPlay type, int index);
	
	public enum SirenaFS
	{
		Ready,
		Busy,
		Save,
		SaveError
	}
	public abstract void onSirenaFS(SirenaFS type);
	
	public enum SirenaPin
	{
		Change,
		Request,
		Error
	}
	public abstract void onSirenaPin(SirenaPin type, String pin);
	
	public enum SirenaConfigChange
	{
		Change,
		ChangeButton,
		ChangeMeasure,
		ChangePressMaximum,
		ChangeIdleMaximum,
		ChangePulsesMaximum,
		ChangeDeviation,
		ChangePlayOnce,
		ChangePlayToEnd,
		ChangeNoWidth,
		ChangeNoBounceHigh,
		ChangeStandard,
		ChangeVolume,
		ChangeVolumeBeep,
		ChangeSleep,
		Saved
	}
	public abstract void onSirenaConfigChange(SirenaConfigChange type, int index);
	
	public enum SirenaMelody
	{
		Start,
		Progress,
		End,
		Full,
		Delete,
		Clear,
		Error
	};
	
	public abstract void onSirenaMelodyChange(SirenaMelody type, int value);
	
	public enum SirenaEvent
	{
		Change,
		Delete,
		Clear
	};
	
	public abstract void onSirenaEventChange(SirenaEvent type, int value);
	
	public static class SirenaMeasure
	{
		public int width;
		public int idle;
		
		public SirenaMeasure(int width, int idle)
		{
			this.width = width;
			this.idle = idle;
		}
	}
	
	public static class SirenaLogEvent
	{
		public int chIndex;
		public PulseType type;
		public List<SirenaPulse> pulse = null;
		public SirenaMeasure measure = null;
		
		public SirenaLogEvent(JSONObject json)
		{
			try
			{
				chIndex = json.getInt("index");
				String pulseType = json.getString("type");
				if (pulseType.equalsIgnoreCase("press"))
				{
					type = PulseType.Press;
				}
				else if (pulseType.equalsIgnoreCase("release"))
				{
					type = PulseType.Release;
				}
				else if (pulseType.equalsIgnoreCase("cyclic"))
				{
					type = PulseType.Cyclic;
				}
				else if (pulseType.equalsIgnoreCase("pulse"))
				{
					type = PulseType.Pulse;
					pulse = new ArrayList<>();
					JSONArray pulsesArray = json.getJSONArray("pulse");
					JSONObject pulseObj;
					int i;
					for (i = 0; i < pulsesArray.length(); i++)
					{
						pulseObj = pulsesArray.getJSONObject(i);
						pulse.add(new SirenaPulse(pulseObj.getInt("width"), pulseObj.getInt("count")));
					}
				}
				else if (pulseType.equalsIgnoreCase("measure"))
				{
					type = PulseType.Measure;
					JSONObject measureObj = json.getJSONObject("measure");
					measure = new SirenaMeasure(measureObj.getInt("width"), measureObj.getInt("idle"));
				}
				else if (pulseType.equalsIgnoreCase("errMeasure"))
				{
					type = PulseType.MeasureError;
				}
			}
			catch (JSONException e)
			{
				debug.p("LogEvent " + e.getMessage());
			}
		}
	}
	
	public abstract void onSirenaLogEvent(SirenaLogEvent event);
	
	/* Sirena connect options */
	private String DeviceName = "SirenaBT";
	private UUID DeviceUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
	private int SpeakSendBytes = 2048;
	
	/* Sirena class */
	private BluetoothAdapter adapter;
	private BluetoothDevice device;
	private BluetoothSocket socket;
	private MainActivity ma;
	private InputStream input;
	private OutputStream output;
	private Thread receiver;
	
	private Boolean isSpeak;
	private byte[] speakBuffer;
	private int speakBufferPosition;
	private Config config;
	private Wave wave;
	
	private List<String> updatePacket;
	private int updateIndex;
	
	private int _serialCode = 0;
	//private Thread checkTimeout = null;
	
	public SirenaBT(MainActivity mainActivity)
	{
		ma = mainActivity;
		updateIndex = -1;
		updatePacket = new ArrayList();
		adapter = BluetoothAdapter.getDefaultAdapter();
		device = null;
		socket = null;
		if (adapter == null)
		{
			debug.p("BT adapter not found");
			onAdapterNotFound();
		}
		else
		{
			BroadcastReceiver broadcast = new BroadcastReceiver()
			{
				@Override
				public void onReceive(Context context, Intent intent)
				{
					switch (intent.getAction())
					{
						case BluetoothAdapter.ACTION_STATE_CHANGED:
							switch (adapter.getState())
							{
								case BluetoothAdapter.STATE_ON:
									AdapterOn();
									break;
								
								case BluetoothAdapter.STATE_OFF:
									AdapterOff();
									break;
							}
							break;
						
						case BluetoothDevice.ACTION_BOND_STATE_CHANGED:
							DeviceSearch();
							break;
					}
				}
			};
			IntentFilter filter = new IntentFilter();
			filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
			filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
			ma.registerReceiver(broadcast, filter);
			
			if (!adapter.isEnabled())
			{
				AdapterOff();
			}
			else
			{
				AdapterOn();
			}
		}
		
	}
	
	public void BTSettingsOpen()
	{
		Intent i = new Intent(Settings.ACTION_BLUETOOTH_SETTINGS);
		ma.startActivity(i);
	}
	
	public void reset()
	{
		new Thread(new Runnable()
		{
			@Override
			public void run()
			{
				DeviceDisconnect();
				DeviceSearch();
			}
		}).start();
	}
	
	public void loadWave()
	{
		onSirenaMelodyChange(SirenaMelody.Progress, wave.offset * 100 / wave.length);
		int length = wave.length - wave.offset;
		if (length > 512) length = 512;
		if (length > 0)
		{
			sendRAWData(Base64.encode(wave.buffer, wave.offset, length, Base64.NO_WRAP));
			wave.offset += length;
		}
	}
	
	public void speak(byte[] buffer)
	{
		if (isSpeak)
		{
			debug.p("Speak collision");
		}
		else
		{
			isSpeak = true;
			speakBufferPosition = 0;
			speakBuffer = buffer;
			speakNext();
		}
		
	}
	
	private void speakNext()
	{
		int sendBytes = speakBuffer.length - speakBufferPosition;
		if (sendBytes > SpeakSendBytes) sendBytes = SpeakSendBytes;
		byte[] data = encodeWAVE7(Arrays.copyOfRange(speakBuffer, speakBufferPosition, speakBufferPosition + sendBytes));
		sendRAWSpeak(data);
	}
	
	private byte[] encodeWAVE7(byte[] buffer)
	{
		for (int i = 0; i < buffer.length; i++)
		{
			buffer[i] = encodeByteWAVE7((byte)(buffer[i] >> 1));
		}
		return buffer;
	}
	
	private byte encodeByteWAVE7(byte value)
	{
		value = (byte)(0x7F & value);
		if (value >= 0 && value < 26) return (byte)(0xFF & ('A' + value));			/* 26*/
		if (value >= 26 && value < 52) return (byte)(0xFF & ('a' + (value - 26)));	/* 26 */
		if (value >= 52 && value < 62) return (byte)(0xFF & ('0' + (value - 52)));	/* 10 */
		if (value >= 62 && value < 126) return (byte)(0xFF & (192 + (value - 62)));	/* 64 */
		if (value == 126) return '+';						/* 1 */
		if (value == 127) return '/';						/* 1 */
		return '\0';
	}
	
	public void play(int index)
	{
		sendRAW("PLAY=" + index);
	}
	
	public void stop()
	{
		sendRAW("PLAYSTOP");
	}
	
	private void AdapterOff()
	{
		debug.p("BT adapter disabled");
		onAdapterOff();
		Intent i = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
		ma.startActivity(i);
	}
	
	private void AdapterOn()
	{
		debug.p("BT adapter enabled");
		onAdapterOn();
		DeviceSearch();
	}
	
	private void DeviceSearch()
	{
		Set<BluetoothDevice> devices = adapter.getBondedDevices();
		BluetoothDevice found = null;
		ma.runOnUiThread(new Runnable()
		{
			@Override
			public void run()
			{
				onWait();
			}
		});
		for (BluetoothDevice d : devices)
		{
			if (d.getName().matches("^" + DeviceName + ".+"))
			{
				debug.p("Device found: " + d.getName());
				found = d;
				break;
			}
		}
		if (found == null)
		{
			debug.p("Device '" + DeviceName + "*' not found");
			DeviceDisconnect();
			ma.runOnUiThread(new Runnable()
			{
				@Override
				public void run()
				{
					onDeviceNotFound();
				}
			});
		}
		else
		{
			try
			{
				Thread.sleep(1000);
			}
			catch (InterruptedException ex)
			{
			
			}
			DeviceConnect(found);
		}
	}
	
	private void DeviceConnect(BluetoothDevice d)
	{
		if (device == null)
		{
			isSpeak = false;
			device = d;
			new Thread(new Runnable()
			{
				@Override
				public void run()
				{
					try
					{
						socket = device.createRfcommSocketToServiceRecord(DeviceUUID);
						socket.connect();
						input = socket.getInputStream();
						output = socket.getOutputStream();
						debug.p("Device open socket");
						ma.runOnUiThread(new Runnable()
						{
							@Override
							public void run()
							{
								onDeviceConnected();
							}
						});
					}
					catch (IOException e)
					{
						debug.p("Rfcomm error " + e.getMessage());
						DeviceDisconnect();
						_onDeviceError();
					}
					if (socket != null && socket.isConnected())
					{
						receiver = new Thread(new Runnable()
						{
							@Override
							public void run()
							{
								debug.p("Receiver start");
								String dataRAW = "";
								while (socket != null)
								{
									try
									{
										byte[] buffer = new byte[2048];
										int bytes = input.read(buffer, 0, buffer.length);
										dataRAW += new String(buffer, 0, bytes);
										int firstCmdEnd;
										while ((firstCmdEnd = dataRAW.indexOf("\r\n")) != -1)
										{
											if (dataRAW.substring(0, 1).equalsIgnoreCase("+"))
											{
												String answer = dataRAW.substring(1, firstCmdEnd);
												int argvStart = answer.indexOf(':');
												if (argvStart == -1)
												{
													ma.runOnUiThread(new recvRAWOnUi(answer, null));
												}
												else
												{
													String command = answer.substring(0, argvStart);
													String argvRAW = answer.substring(argvStart + 1);
													List<String> argv = new ArrayList<String>();
													int argvDelimiter;
													while ((argvDelimiter = argvRAW.indexOf(';')) != -1)
													{
														argv.add(argvRAW.substring(0, argvDelimiter));
														argvRAW = argvRAW.substring(argvDelimiter + 1);
													}
													if (argvRAW.length() != 0)
													{
														argv.add(argvRAW);
													}
													if (command.equalsIgnoreCase("SPEAK"))
													{
														try
														{
															JSONObject json = new JSONObject(argv.get(0));
															speakBufferPosition += json.getInt("writed");
															if (speakBufferPosition >= speakBuffer.length)
															{
																ma.runOnUiThread(new Runnable()
																{
																	@Override
																	public void run()
																	{
																		onSirenaSpeak(SirenaSpeak.End);
																	}
																});
																isSpeak = false;
															}
															else
															{
																speakNext();
															}
														}
														catch (JSONException e)
														{
															debug.p("Speak JSON " + e.getMessage());
															isSpeak = false;
														}
													}
													else
													{
														ma.runOnUiThread(new recvRAWOnUi(command, argv));
													}
												}
											}
											dataRAW = dataRAW.substring(firstCmdEnd + 2);
										}
									}
									catch (IOException e)
									{
										debug.p("Receiver read error " + e.getMessage());
										DeviceDisconnect();
										_onDeviceError();
									}
								}
								input = null;
								output = null;
								receiver = null;
							}
						});
						receiver.start();
						if (updateIndex >= 0)
						{
							debug.p("Init from Boot");
							sendRAW("^BOOT");
						}
						else
						{
							debug.p("Init Normal");
							sendRAW("^MAIN");
							sendRAW("ENDMELODY");
							sendRAW("INFO");
						}
					}
				}
			}).start();
		}
		else
		{
			debug.p("DeviceConnect collision");
		}
	}
	
	public void SetUpdate(String fw)
	{
		String[] lines = fw.split("\r\n");
		int index = 0;
		for (String line : lines)
		{
			line = line.trim();
			if (line != "")
			{
				updatePacket.add(index, "PACKET=" + index + ";" + line);
				index++;
			}
		}
		sendRAW("^BOOT");
	}
	
	private void SendUpdate()
	{
		sendRAW(updatePacket.get(updateIndex));
	}
	
	private void DeviceDisconnect()
	{
		if (device != null)
		{
			if (socket != null)
			{
				try
				{
					socket.close();
					debug.p("Device close socket");
				}
				catch (IOException e)
				{
					debug.p("Device close socket error " + e.getMessage());
				}
				socket = null;
			}
			device = null;
		}
	}
	
	private void sendRAW(String cmd)
	{
		if (socket != null && socket.isConnected())
		{
			byte[] b = (cmd + "\r\n").getBytes();
			try
			{
				output.write(b);
			}
			catch (IOException e)
			{
				debug.p("sendRAW error " + e.getMessage());
				_onDeviceError();
			}
		}
	}
	
	private void sendRAWSpeak(byte[] data)
	{
		if (socket != null && socket.isConnected())
		{
			byte[] cmd = ("SPEAK=").getBytes();
			byte[] crlf = ("\r\n").getBytes();
			try
			{
				output.write(cmd);
				output.write(data);
				output.write(crlf);
			}
			catch (IOException e)
			{
				debug.p("sendRAWSpeak error " + e.getMessage());
				_onDeviceError();
			}
		}
	}
	
	private void sendRAWData(byte[] data)
	{
		if (socket != null && socket.isConnected())
		{
			byte[] cmd = ("DATAMELODY=").getBytes();
			byte[] crlf = ("\r\n").getBytes();
			try
			{
				output.write(cmd);
				output.write(data);
				//debug.p("sendRAWData " + data.length);
				//debug.p("sendRAWData " + Base64.decode(data, Base64.DEFAULT).length);
				output.write(crlf);
			}
			catch (IOException e)
			{
				debug.p("sendRAWData error " + e.getMessage());
				_onDeviceError();
			}
		}
	}
	
	private class recvRAWOnUi implements Runnable
	{
		private String cmd;
		private List<String> value;
		
		public recvRAWOnUi(String cmd, List<String> value)
		{
			this.cmd = cmd;
			this.value = value;
		}
		
		@Override
		public void run()
		{
			JSONObject json = null;
			try
			{
				String value0 = value.get(0);
				if
				(
						value0.substring(0, 1).equalsIgnoreCase("{")
								&& value0.substring(value0.length() - 1).equalsIgnoreCase("}")
				)
				{
					json = new JSONObject(value0);
				}
				else if
				(
						value0.substring(0, 1).equalsIgnoreCase("[")
								&& value0.substring(value0.length() - 1).equalsIgnoreCase("]")
				)
				{
					json = new JSONObject("{\"array\":" + value0 + "}");
				}
				
				switch (cmd)
				{
					case "PLAY":
						onSirenaPlay(SirenaPlay.Play, json.getInt("index"));
						break;
					
					case "PLAYSTOP":
						if (value0.equalsIgnoreCase("OK"))
						{
							onSirenaPlay(SirenaPlay.Stop, -1);
						}
						else
						{
							onSirenaPlay(SirenaPlay.Stop, json.getInt("index"));
						}
						break;
					
					case "INFO":
						if (json == null)
						{
							onDeviceError();
						}
						else
						{
							if (json.getBoolean("fsReady"))
							{
								onSirenaFS(SirenaFS.Ready);
								config = new Config(json);
								initOnFirstConnect();
								//sendRAW("GETCONFIG");
								//if (config.build >= 1777) setCheckTimeout();
							}
							else
							{
								//clearCheckTimeout();
								onSirenaFS(SirenaFS.Busy);
							}
						}
						break;
					
					case "FS":
						if (value0.equalsIgnoreCase("READY"))
						{
							onSirenaFS(SirenaFS.Ready);
						}
						else
						{
							//clearCheckTimeout();
							onSirenaFS(SirenaFS.Busy);
						}
						break;
					
					case "SAVEFS":
						if (config.build < 1778)
						{
							onSirenaFS(value0.equalsIgnoreCase("OK") ? SirenaFS.Save : SirenaFS.SaveError);
						}
						else
						{
							debug.p("SAVEFS ignore");
						}
						break;
					
					case "LIST":
						if (json == null)
						{
							onDeviceError();
						}
						else
						{
							config.fromList(json);
							new Thread(new Runnable()
							{
								@Override
								public void run()
								{
									try
									{
										int m = 0;
										int e = 0;
										for (int index : config.melodyIndexs)
										{
											m++;
											sendRAW("GETMELODY=" + index);
											Thread.sleep(50);
										}
										for (int index : config.eventIndexs)
										{
											e++;
											sendRAW("GETEVENT=" + index);
											Thread.sleep(50);
										}
										if (m == 0 && e == 0)
										{
											ma.runOnUiThread(new Runnable()
											{
												@Override
												public void run()
												{
													onConfig(config);
												}
											});
										}
									}
									catch (InterruptedException e)
									{
										debug.p("LIST sleep " + e.getMessage());
									}
								}
							}).start();
						}
						break;
					
					case "GETMELODY":
						if (json == null)
						{
							onDeviceError();
						}
						else
						{
							if (config.fromMelody(json))
							{
								onConfig(config);
							}
						}
						break;
					
					case "SETMELODY":
						if (value0.equalsIgnoreCase("ERROR"))
						{
							if (value0.equalsIgnoreCase("FULL"))
							{
								onSirenaMelodyChange(SirenaMelody.Full, -1);
							}
							else
							{
								onSirenaMelodyChange(SirenaMelody.Error, -1);
							}
						}
						else
						{
							onSirenaMelodyChange(SirenaMelody.Start, json.getInt("index"));
							loadWave();
						}
						break;
					
					case "DATAMELODY":
						if (value0.equalsIgnoreCase("OK"))
						{
							loadWave();
						}
						else if (value0.equalsIgnoreCase("END"))
						{
							sendRAW("ENDMELODY");
							wave = null;
						}
						else
						{
							onSirenaMelodyChange(SirenaMelody.Error, -1);
						}
						break;
					
					case "ENDMELODY":
						onSirenaMelodyChange(SirenaMelody.End, -1);
						break;
					
					case "DELETEMELODY":
						onSirenaMelodyChange(SirenaMelody.Delete, json.getInt("index"));
						break;
					
					case "CLEARMELODY":
						onSirenaMelodyChange(SirenaMelody.Clear, -1);
						break;
					
					case "GETEVENT":
						if (json == null)
						{
							onDeviceError();
						}
						else
						{
							if (config.fromEvent(json))
							{
								onConfig(config);
							}
						}
						break;
					
					case "SETEVENT":
						if (json == null)
						{
							debug.p("SETEVENT: " + value0);
						}
						else
						{
							onSirenaEventChange(SirenaEvent.Change, json.getInt("index"));
						}
						break;
					
					case "DELETEEVENT":
						onSirenaEventChange(SirenaEvent.Delete, json.getInt("index"));
						break;
					
					case "CLEAREVENT":
						onSirenaEventChange(SirenaEvent.Clear, 0);
						break;
					
					case "GETCONFIG":
						//config.fromGetConfig(json.getJSONArray("array"));
						//sendRAW("LIST");
						if (config != null)
						{
							config.fromGetConfig(json.getJSONArray("array"));
							sendRAW("LIST");
						}
						break;
					
					case "SETCONFIG":
						onSirenaConfigChange(SirenaConfigChange.Change, json.getInt("index"));
						break;
					
					case "SETBUTTON":
						onSirenaConfigChange(SirenaConfigChange.ChangeButton, json.getInt("index"));
						break;
					
					case "SETMEASURE":
						onSirenaConfigChange(SirenaConfigChange.ChangeMeasure, json.getInt("index"));
						break;
					
					case "SETPRESSMAX":
						onSirenaConfigChange(SirenaConfigChange.ChangePressMaximum, json.getInt("index"));
						break;
					
					case "SETIDLEMAX":
						onSirenaConfigChange(SirenaConfigChange.ChangeIdleMaximum, json.getInt("index"));
						break;
					
					case "SETPULSESMAX":
						onSirenaConfigChange(SirenaConfigChange.ChangePulsesMaximum, json.getInt("index"));
						break;
					
					case "SETDEVIATION":
						onSirenaConfigChange(SirenaConfigChange.ChangeDeviation, json.getInt("index"));
						break;
					
					case "SETPLAYONCE":
						onSirenaConfigChange(SirenaConfigChange.ChangePlayOnce, json.getInt("index"));
						break;
					
					case "SETPLAYTOEND":
						onSirenaConfigChange(SirenaConfigChange.ChangePlayToEnd, json.getInt("index"));
						break;
					
					case "SETNOWIDTH":
						onSirenaConfigChange(SirenaConfigChange.ChangeNoWidth, json.getInt("index"));
						break;
						
					case "SETNOBOUNCEHIGH":
						onSirenaConfigChange(SirenaConfigChange.ChangeNoBounceHigh, json.getInt("index"));
						break;
					
					case "SETSTANDARD":
						onSirenaConfigChange(SirenaConfigChange.ChangeStandard, json.getInt("index"));
						break;
					
					case "SETVOLUME":
						onSirenaConfigChange(SirenaConfigChange.ChangeVolume, json.getInt("index"));
						break;
					
					case "SETVOLUMEBEEP":
						onSirenaConfigChange(SirenaConfigChange.ChangeVolumeBeep, json.getInt("index"));
						break;
					
					case "SETSLEEP":
						onSirenaConfigChange(SirenaConfigChange.ChangeSleep, json.getInt("index"));
						break;
					
					case "SAVECONFIG":
						onSirenaConfigChange(SirenaConfigChange.Saved, 0);
						break;
					
					case "PINCODE":
						switch (value0)
						{
							case "OK":		onSirenaPin(SirenaPin.Change, null);break;
							case "ERROR":	onSirenaPin(SirenaPin.Error, null);break;
							default:		onSirenaPin(SirenaPin.Request, value0);break;
						}
						break;
					
					case "EVENT":
						onSirenaLogEvent(new SirenaLogEvent(json));
						break;
						
					case "^BOOT":
						switch (value0)
						{
							case "OK":
								debug.p("Switch to Boot");
								updateIndex = 0;
								try
								{
									socket.close();
								}
								catch (IOException ex)
								{
									debug.p("^BOOT " + ex.getMessage());
								}
								onWait();
								break;
							case "READY":
								debug.p("Start update");
								onStartUpdate(updatePacket.size());
								SendUpdate();
								break;
						}
						break;
						
					case "PACKET":
						if (value0.equalsIgnoreCase("OK"))
						{
							onProgressUpdate(updateIndex);
							if (updateIndex >= (updatePacket.size() - 1))
							{
								sendRAW("^MAIN");
							}
							else
							{
								updateIndex++;
								SendUpdate();
							}
						}
						else
						{
							SendUpdate();
						}
						break;
						
					case "^MAIN":
						if (value0.equalsIgnoreCase("OK"))
						{
							debug.p("Switch to Main");
							updateIndex = -1;
							try
							{
								socket.close();
							}
							catch (IOException ex)
							{
								debug.p("^MAIN " + ex.getMessage());
							}
							onWait();
						}
						break;
						
					case "CHECK":
						if (value0.equalsIgnoreCase("ERROR"))
						{
							setSerialError();
						}
						else
						{
							if (testSerialCheck(value0))
							{
								//clearCheckTimeout();
								sendRAW("GETCONFIG");
							}
							else
							{
								setSerialError();
							}
						}
						break;
				}
			}
			catch (JSONException e)
			{
				debug.p("Answer '" + cmd + "' JSON " + e.getMessage());
				reset();
			}
		}
	}
	
	public void DeleteMelody(int melody)
	{
		sendRAW("DELETEMELODY=" + melody);
	}
	
	public void ClearMelody()
	{
		sendRAW("CLEARMELODY");
	}
	
	public void DeleteEvent(int event)
	{
		sendRAW("DELETEEVENT=" + event);
	}
	
	public void ClearEvent()
	{
		sendRAW("CLEAREVENT");
	}
	
	public void SetButton(int index, boolean enable)
	{
		sendRAW("SETBUTTON=" + index + ";" + ((enable) ? 1 : 0));
	}
	
	public void SetMeasure(int index, boolean enable)
	{
		sendRAW("SETMEASURE=" + index + ";" + ((enable) ? 1 : 0));
	}
	
	public void SetPressMax(int index, int value)
	{
		sendRAW("SETPRESSMAX=" + index + ";" + value);
	}
	
	public void SetIdleMax(int index, int value)
	{
		sendRAW("SETIDLEMAX=" + index + ";" + value);
	}
	
	public void SetPulsesMax(int index, int value)
	{
		sendRAW("SETPULSESMAX=" + index + ";" + value);
	}
	
	public void SetDeviation(int index, int value)
	{
		sendRAW("SETDEVIATION=" + index + ";" + value);
	}
	
	public void SetPlayOnce(int index, boolean enable)
	{
		sendRAW("SETPLAYONCE=" + index + ";" + ((enable) ? 1 : 0));
	}
	
	public void SetPlayToEnd(int index, boolean enable)
	{
		sendRAW("SETPLAYTOEND=" + index + ";" + ((enable) ? 1 : 0));
	}
	
	public void SetNoWidth(int index, boolean enable)
	{
		sendRAW("SETNOWIDTH=" + index + ";" + ((enable) ? 1 : 0));
	}
	
	public void SetNoBounceHigh(int index, boolean enable)
	{
		sendRAW("SETNOBOUNCEHIGH=" + index + ";" + ((enable) ? 1 : 0));
	}
	
	public void SetStandard(int index, boolean enable)
	{
		sendRAW("SETSTANDARD=" + index + ";" + ((enable) ? 1 : 0));
	}
	
	public void SetVolume(int value)
	{
		sendRAW("SETVOLUME=" + value);
	}
	
	public void SetVolumeBeep(int value)
	{
		sendRAW("SETVOLUMEBEEP=" + value);
	}
	
	public void SetSleep(boolean enable)
	{
		sendRAW("SETSLEEP=" + ((enable) ? 1 : 0));
	}
	
	public void SetPINCode(String pin)
	{
		sendRAW("PINCODE=" + pin);
	}
	
	public void SaveFS()
	{
		sendRAW("SAVEFS");
	}
	
	public void SaveConfig()
	{
		sendRAW("SAVECONFIG");
	}
	
	public static class SetEventValue
	{
		public int index;
		public int chIndex;
		public int melodyIndex;
		public boolean poweroff;
		public int volume;
		public int noPlayAfter;
		public PulseType type;
		public List<SirenaPulse> pulse = null;
		
		public SetEventValue(int index, int chIndex, int melodyIndex, boolean poweroff, int volume, int noPlayAfter, PulseType type)
		{
			this.index = index;
			this.chIndex = chIndex;
			this.melodyIndex = melodyIndex;
			this.poweroff = poweroff;
			this.volume = volume;
			this.noPlayAfter = noPlayAfter;
			this.type = type;
			if (isPulse())
			{
				pulse = new ArrayList<>();
			}
		}
		
		public boolean isPulse()
		{
			return (this.type == PulseType.Pulse);
		}
		
		public void addPulse(int width, int count)
		{
			if (pulse != null)
			{
				SirenaPulse p = new SirenaPulse(width, count);
				pulse.add(p);
			}
		}
	}
	
	public void checkSerial(String hash)
	{
		try
		{
			JSONObject json = new JSONObject(hash);
			_serialCode = json.getInt("id");
			String data = json.getString("data");
			sendRAW("CHECK=" + data);
		}
		catch (JSONException ex)
		{
			debug.p("checkSerial: " + hash + ", " + ex.getMessage());
			setSerialError();
		}
	}
	
	private void setSerialError()
	{
		ma.runOnUiThread(new Runnable()
		{
			@Override
			public void run()
			{
				onSerialError();
			}
		});
	}
	
	private void setInternetError()
	{
		ma.runOnUiThread(new Runnable()
		{
			@Override
			public void run()
			{
				onInternetError();
			}
		});
	}
	
	private void setBuildError()
	{
		ma.runOnUiThread(new Runnable()
		{
			@Override
			public void run()
			{
				onBuildError();
			}
		});
	}
	
	private byte HEXtoByte(byte hex)
	{
		if (hex >= '0' && hex <= '9') return (byte)(0xFF & (hex - '0'));
		if (hex >= 'A' && hex <= 'F') return (byte)(0xFF & (hex - 'A' + 10));
		if (hex >= 'a' && hex <= 'f') return (byte)(0xFF & (hex - 'a' + 10));
		return 0;
	}
	
	private byte[] HEXtoBytes(String data)
	{
		int i, n;
		int l = data.length();
		byte[] _data = data.getBytes();
		byte[] result = new byte[l / 2];
		for (i = 0, n = 0; (i + 1) < l; i += 2, n++)
		{
			result[n] = (byte)(0xFF & ((HEXtoByte(_data[i]) << 4) | HEXtoByte(_data[i + 1])));
		}
		return result;
	}
	
	private int[] crc16Table =
			{
				0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
				0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
				0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
				0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
				0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
				0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
				0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
				0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
				0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
				0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
				0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
				0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
				0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
				0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
				0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
				0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
				0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
				0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
				0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
				0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
				0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
				0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
				0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
				0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
				0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
				0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
				0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
				0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
				0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
				0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
				0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
				0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
			};
	
	private int crc16(byte[] buffer, int length)
	{
		int crc = 0xFFFF;
		byte b;
		int i;
		if (buffer.length < length) length = buffer.length;
		for (i = 0; i < length; i++)
		{
			b = buffer[i];
			crc = 0xFFFF & (((crc << 8) & 0xFF00) ^ crc16Table[((crc >> 8) ^ (0xFF & b)) & 0x00FF]);
		}
		return 0xFFFF & crc;
	}
	
	private Boolean testSerialCheck(String data)
	{
		Boolean result = false;
		byte[] d = HEXtoBytes(data);
		if (d.length == 64)
		{
			int crc = crc16(d, 62);
			int crc2 = 0xFFFF & (((0xFF & d[63]) << 8) | (0xFF & d[62]));
			//debug.p("data " + data);
			//debug.p("_data " + d[62] + ", " + d[63]);
			//debug.p("CRC " + crc + " / " + crc2);
			if (crc == crc2)
			{
				int c = 13;
				int i, n;
				byte[] s = new byte[31];
				while (c-- > 0)
				{
					i = 61;
					d[i] = (byte)(0xFF & (d[i--] ^ d[0]));
					for (; i >= 0; i--)
					{
						d[i] = (byte)(0xFF & (d[i] ^ d[i + 1]));
					}
				}
				for (i = 0, n = 0; n < 31; i += 2, n++)
				{
					s[n] = d[i];
				}
				result = config.serial.equalsIgnoreCase(new String(s));
			}
		}
		return result;
	}
	
	public int serialCode()
	{
		return _serialCode;
	}
	
	/*private void setCheckTimeout()
	{
		clearCheckTimeout();
		checkTimeout = new Thread(new Runnable()
		{
			@Override
			public void run()
			{
				//debug.p("CheckTimeout on");
				try
				{
					Thread.sleep(5000);
					//debug.p("CheckTimeout lost");
					setSerialError();
				}
				catch (InterruptedException ex)
				{
					//debug.p("CheckTimeout off");
				}
				checkTimeout = null;
			}
		});
		checkTimeout.start();
	}
	
	private void clearCheckTimeout()
	{
		if (checkTimeout != null)
		{
			checkTimeout.interrupt();
			checkTimeout = null;
		}
	}*/
	
	void initOnFirstConnect()
	{
		if (/*config.build < 1777*/ true)
		{
			//setBuildError();
			sendRAW("GETCONFIG");
		}
		else
		{
			new Thread(new Runnable()
			{
				@Override
				public void run()
				{
					try
					{
						/*SSLContext sc = SSLContext.getInstance("SSL");
						TrustManager[] trustAllCerts = new TrustManager[] {
								new X509TrustManager() {
									public X509Certificate[] getAcceptedIssuers() {
										return new X509Certificate[0];
									}
									public void checkClientTrusted(X509Certificate[] certs, String authType) {}
									public void checkServerTrusted(X509Certificate[] certs, String authType) {}
								}};
						sc.init(null, trustAllCerts, new SecureRandom());
						HostnameVerifier hv = new HostnameVerifier() {
							public boolean verify(String hostname, SSLSession session) { return true; }
						};
						HttpsURLConnection.setDefaultSSLSocketFactory(sc.getSocketFactory());
						HttpsURLConnection.setDefaultHostnameVerifier(hv);*/
						
						HttpsURLConnection conn = (HttpsURLConnection)(new URL(apiUrl + "update/serial/")).openConnection();
						conn.setDoOutput(true);
						conn.setChunkedStreamingMode(0);
						conn.setRequestMethod("POST");
						
						BufferedOutputStream out = new BufferedOutputStream(conn.getOutputStream());
						out.write(("serial=" + URLEncoder.encode(config.serial, "utf8")).getBytes());
						out.flush();
						
						InputStream in = new BufferedInputStream(conn.getInputStream());
						byte[] dataBinary = new byte[384];
						int readed = in.read(dataBinary);
						
						conn.disconnect();
						
						if (readed > 0)
						{
							String data = new String(dataBinary);
							data = data.substring(0, readed);
							checkSerial(data);
						}
						else
						{
							setInternetError();
						}
					}
					catch (Exception ex)
					{
						debug.p("initOnFirstConnect: " + ex.toString());
						setInternetError();
					}
				}
			}).start();
		}
	}
	
	public void SetEvent(SetEventValue event)
	{
		String s = "SETEVENT=" + event.index + ";" + event.melodyIndex + ";" + ((event.poweroff) ? 1 : 0) + ";" + event.chIndex + ";";
		if (config.build >= 1513)
		{
			s += event.volume + ";";
		}
		if (config.build >= 1590)
		{
			s += event.noPlayAfter + ";";
		}
		if (event.isPulse())
		{
			int count = 0;
			String t = "";
			for (SirenaPulse p : event.pulse)
			{
				count++;
				t += ";" + p.width + ";" + p.count;
			}
			s += count + t;
		}
		else
		{
			if (event.type == PulseType.Press)
			{
				s += "press";
			}
			else if (event.type == PulseType.Cyclic)
			{
				s += "cyclic";
			}
		}
		sendRAW(s);
	}
	
	public Wave.Valid SetMelody(String filename)
	{
		sendRAW("ENDMELODY");
		wave = new Wave(filename);
		if (wave.valid == Wave.Valid.Sussed)
		{
			sendRAW("SETMELODY=" + wave.length+ ";" + wave.samples + ";" + wave.name);
		}
		return wave.valid;
	}
	
	private void _onDeviceError()
	{
		ma.runOnUiThread(new Runnable()
		{
			@Override
			public void run()
			{
				onDeviceError();
			}
		});
	}
}
