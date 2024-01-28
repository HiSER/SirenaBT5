package com.hiserlitvin.sirenabt5;

import android.Manifest;
import android.content.Context;
import android.content.Entity;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
//import android.support.v7.app.AppCompatActivity;
import androidx.appcompat.app.AppCompatActivity;

import android.webkit.WebView;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.io.File;

public class MainActivity extends AppCompatActivity
{
	private static final int TRY_CONNECT	= 3;
	
	private MainActivity ma;
	private SirenaBT sirena;
	private int speakSamples;
	private MicRecord record;
	private WebView w;
	private SharedPreferences settings;
	private int tryConnect;
	
	private class uiJSI
	{
		public class Buttons
		{
			public List<Integer> emoji;
			public List<Integer> mld;
			
			public Buttons()
			{
				emoji = new ArrayList<>();
				mld = new ArrayList<>();
				for (int i = 0; i <= 7; i++)
				{
					emoji.add(i);
					mld.add(-1);
				}
			}
		};
		
		public SirenaBT.Config config;
		public SirenaBT.SirenaLogEvent logEvent;
		public Buttons buttons;
		public HashMap<String, String> eventName;
		
		public uiJSI()
		{
			buttons = new Buttons();
			eventName = new HashMap<>();
		}
		
		public int getSirenaPulse(SirenaBT.PulseType pulseType)
		{
			if (pulseType == SirenaBT.PulseType.Null)
			{
				return 0;
			}
			else if (pulseType == SirenaBT.PulseType.Press)
			{
				return 1;
			}
			else if (pulseType == SirenaBT.PulseType.Release)
			{
				return 2;
			}
			else if (pulseType == SirenaBT.PulseType.Cyclic)
			{
				return 3;
			}
			else if (pulseType == SirenaBT.PulseType.Pulse)
			{
				return 4;
			}
			else if (pulseType == SirenaBT.PulseType.Measure)
			{
				return 5;
			}
			else if (pulseType == SirenaBT.PulseType.MeasureError)
			{
				return 6;
			}
			else
			{
				return 0;
			}
		}
		
		public SirenaBT.PulseType getSirenaPulse(int pulseType)
		{
			switch (pulseType)
			{
				case 1:		return SirenaBT.PulseType.Press;
				case 2:		return SirenaBT.PulseType.Release;
				case 3:		return SirenaBT.PulseType.Cyclic;
				case 4:		return SirenaBT.PulseType.Pulse;
				case 5:		return SirenaBT.PulseType.Measure;
				case 6:		return SirenaBT.PulseType.MeasureError;
				default:	return SirenaBT.PulseType.Null;
			}
		}
		
		@android.webkit.JavascriptInterface
		public String getLang()
		{
			return Resources.getSystem().getConfiguration().locale.getLanguage();
		}
		
		@android.webkit.JavascriptInterface
		public String getConfig()
		{
			String s = null;
			if (config != null)
			{
				boolean flag;
				s = "{\"pincode\":\"" + config.pincode + "\",";
				s += "\"build\":" + config.build + ",";
				s += "\"version\":" + config.version + ",";
				s += "\"melodyMax\":" + config.melodyMax + ",";
				s += "\"eventMax\":" + config.eventMax + ",";
				s += "\"volume\":" + config.volume + ",";
				s += "\"volumeBeep\":" + config.volumeBeep + ",";
				s += "\"sleep\":" + ((config.sleep) ? "true" : "false") + ",";
				s += "\"serial\":\"" + config.serial + "\",";
				s += "\"channels\":[";
				flag = false;
				for (SirenaBT.Channel ch : config.channels)
				{
					if (flag) s += ","; else flag = true;
					s += "{\"index\":" + ch.index + ",";
					s += "\"pressMax\":" + ch.pressMax + ",";
					s += "\"idleMax\":" + ch.idleMax + ",";
					s += "\"pulsesMax\":" + ch.pulsesMax + ",";
					s += "\"deviation\":" + ch.deviation + ",";
					s += "\"button\":" + ((ch.button) ? "true" : "false") + ",";
					s += "\"measure\":" + ((ch.measure) ? "true" : "false") + ",";
					s += "\"playOnce\":" + ((ch.playOnce) ? "true" : "false") + ",";
					s += "\"playToEnd\":" + ((ch.playToEnd) ? "true" : "false") + ",";
					s += "\"noWidth\":" + ((ch.noWidth) ? "true" : "false") + ",";
					s += "\"noBounceHigh\":" + ((ch.noBounceHigh) ? "true" : "false") + ",";
					s += "\"standard\":" + ((ch.standard) ? "true" : "false") + "}";
				}
				s += "],\"melodys\":[";
				flag = false;
				for (SirenaBT.Melody mld : config.melodys)
				{
					if (flag) s += ","; else flag = true;
					s += "{\"index\":" + mld.index + ",";
					s += "\"name\":\"" + mld.name + "\"}";
				}
				s += "],\"events\":[";
				flag = false;
				for (SirenaBT.Event evt : config.events)
				{
					if (flag) s += ","; else flag = true;
					s += "{\"index\":" + evt.index + ",";
					s += "\"chIndex\":" + evt.chIndex + ",";
					s += "\"melodyIndex\":" + evt.melodyIndex + ",";
					s += "\"poweroff\":" + ((evt.poweroff) ? true : false) + ",";
					s += "\"volume\":" + evt.volume + ",";
					s += "\"noPlayAfter\":" + evt.noPlayAfter + ",";
					s += "\"type\":" + getSirenaPulse(evt.type) + ",";
					s += "\"pulse\":[";
					boolean flag2 = false;
					for (SirenaBT.SirenaPulse pulse : evt.pulse)
					{
						if (flag2) s += ","; else flag2 = true;
						s += "{\"width\":" + pulse.width + ",";
						s += "\"count\":" + pulse.count + "}";
					}
					s += "]}";
				}
				s += "]}";
				config = null;
			}
			return s;
		}
		
		@android.webkit.JavascriptInterface
		public String getLogEvent()
		{
			String s = null;
			if (logEvent != null)
			{
				s = "{\"chIndex\":" + logEvent.chIndex + ",";
				s += "\"type\":" + getSirenaPulse(logEvent.type);
				if (logEvent.type == SirenaBT.PulseType.Pulse)
				{
					s += ",\"pulse\":[";
					boolean flag = false;
					for (SirenaBT.SirenaPulse pulse : logEvent.pulse)
					{
						if (flag) s += ","; else flag = true;
						s += "{\"width\":" + pulse.width + ",";
						s += "\"count\":" + pulse.count + "}";
					}
					s += "]";
				}
				else if (logEvent.type == SirenaBT.PulseType.Measure)
				{
					s += ",\"measure\":{\"width\":" + logEvent.measure.width + ",";
					s += "\"idle\":" + logEvent.measure.idle + "}";
				}
				s += "}";
			}
			return s;
		}
		
		@android.webkit.JavascriptInterface
		public void play(int melody)
		{
			sirena.play(melody);
		}
		
		@android.webkit.JavascriptInterface
		public void stop()
		{
			sirena.stop();
		}
		
		@android.webkit.JavascriptInterface
		public void speakStart()
		{
			record = new MicRecord(ma, ma.speakSamples)
			{
				@Override
				public void onStart()
				{
					w.loadUrl("javascript:ui.cbSpeak(true)");
				}
				
				@Override
				public void onEnd(byte[] buffer)
				{
					sirena.speak(buffer);
					record = null;
				}
				
				@Override
				public void onError()
				{
					debug.p("MicRecord Error");
					w.loadUrl("javascript:ui.cbSpeak(false)");
				}
			};
			new Thread(record).start();
		}
		
		@android.webkit.JavascriptInterface
		public void speakStop()
		{
			if (record != null)
			{
				record.stop();
			}
		}
		
		@android.webkit.JavascriptInterface
		public void reconnect()
		{
			sirena.reset();
		}
		
		@android.webkit.JavascriptInterface
		public void btSettings()
		{
			sirena.BTSettingsOpen();
		}
		
		@android.webkit.JavascriptInterface
		public void deleteMelody(int melody)
		{
			sirena.DeleteMelody(melody);
		}
		
		@android.webkit.JavascriptInterface
		public void setButton(int index, boolean enable)
		{
			sirena.SetButton(index, enable);
		}
		
		@android.webkit.JavascriptInterface
		public void setMeasure(int index, boolean enable)
		{
			sirena.SetMeasure(index, enable);
		}
		
		@android.webkit.JavascriptInterface
		public void setPressMax(int index, int value)
		{
			sirena.SetPressMax(index, value);
		}
		
		@android.webkit.JavascriptInterface
		public void setIdleMax(int index, int value)
		{
			sirena.SetIdleMax(index, value);
		}
		
		@android.webkit.JavascriptInterface
		public void setPulsesMax(int index, int value)
		{
			sirena.SetPulsesMax(index, value);
		}
		
		@android.webkit.JavascriptInterface
		public void setDeviation(int index, int value)
		{
			sirena.SetDeviation(index, value);
		}
		
		@android.webkit.JavascriptInterface
		public void setPlayOnce(int index, boolean enable)
		{
			sirena.SetPlayOnce(index, enable);
		}
		
		@android.webkit.JavascriptInterface
		public void setPlayToEnd(int index, boolean enable)
		{
			sirena.SetPlayToEnd(index, enable);
		}
		
		@android.webkit.JavascriptInterface
		public void setNoWidth(int index, boolean enable)
		{
			sirena.SetNoWidth(index, enable);
		}
		
		@android.webkit.JavascriptInterface
		public void setNoBounceHigh(int index, boolean enable)
		{
			sirena.SetNoBounceHigh(index, enable);
		}
		
		@android.webkit.JavascriptInterface
		public void setStandard(int index, boolean enable)
		{
			sirena.SetStandard(index, enable);
		}
		
		@android.webkit.JavascriptInterface
		public void setVolume(int value)
		{
			sirena.SetVolume(value);
		}
		
		@android.webkit.JavascriptInterface
		public void setVolumeBeep(int value)
		{
			sirena.SetVolumeBeep(value);
		}
		
		@android.webkit.JavascriptInterface
		public void setSleep(boolean enable)
		{
			sirena.SetSleep(enable);
		}
		
		@android.webkit.JavascriptInterface
		public void setPINCode(String pin)
		{
			sirena.SetPINCode(pin);
		}
		
		@android.webkit.JavascriptInterface
		public void saveFS()
		{
			sirena.SaveFS();
		}
		
		@android.webkit.JavascriptInterface
		public void saveConfig()
		{
			sirena.SaveConfig();
		}
		
		@android.webkit.JavascriptInterface
		public void setEvent(String event)
		{
			SirenaBT.SetEventValue ev;
			try
			{
				JSONObject json = new JSONObject(event);
				ev = new SirenaBT.SetEventValue(
					json.getInt("index"),
					json.getInt("chIndex"),
					json.getInt("melodyIndex"),
					json.getBoolean("poweroff"),
					json.getInt("volume"),
					json.getInt("noPlayAfter"),
					getSirenaPulse(json.getInt("type")));
				if (ev.isPulse())
				{
					JSONArray arr = json.getJSONArray("pulse");
					for (int i = 0; i < arr.length(); i++)
					{
						JSONObject pulse = arr.getJSONObject(i);
						ev.addPulse(pulse.getInt("width"), pulse.getInt("count"));
					}
				}
				sirena.SetEvent(ev);
			}
			catch (JSONException e)
			{
				debug.p("JSI.setEvent " + e.getMessage());
			}
		}
		
		@android.webkit.JavascriptInterface
		public void deleteEvent(int event)
		{
			sirena.DeleteEvent(event);
		}
		
		@android.webkit.JavascriptInterface
		public void setButtons(String b)
		{
			try
			{
				JSONObject json = new JSONObject(b);
				JSONArray arr;
				arr = json.getJSONArray("emoji");
				for (int i = 0; i <= 7 && i < arr.length(); i++)
				{
					buttons.emoji.set(i, arr.getInt(i));
				}
				arr = json.getJSONArray("mld");
				for (int i = 0; i <= 7 && i < arr.length(); i++)
				{
					buttons.mld.set(i, arr.getInt(i));
				}
			}
			catch (JSONException e)
			{
				debug.p("setButtons " + e.getMessage());
			}
		}
		
		@android.webkit.JavascriptInterface
		public String getButtons()
		{
			String s = "{";
			boolean flag = false;
			s += "\"emoji\":[";
			for (int i : buttons.emoji)
			{
				if (flag) s += ","; else flag = true;
				s += i;
			}
			s += "],\"mld\":[";
			flag = false;
			for (int i : buttons.mld)
			{
				if (flag) s += ","; else flag = true;
				s += i;
			}
			s += "]}";
			return s;
		}
		
		@android.webkit.JavascriptInterface
		public void setEventName(String en)
		{
			try
			{
				JSONObject json = new JSONObject(en);
				JSONArray keys = json.names();
				eventName.clear();
				for (int i = 0; i < keys.length(); i++)
				{
					String key = keys.getString(i);
					eventName.put(key, json.getString(key));
				}
			}
			catch (JSONException e)
			{
				debug.p("setEventName " + e.getMessage());
			}
		}
		
		@android.webkit.JavascriptInterface
		public String getEventName()
		{
			String s = "{";
			boolean flag = false;
			for (HashMap.Entry e : eventName.entrySet())
			{
				if (flag) s += ","; else flag = true;
				s += "\"" + e.getKey() + "\":";
				s += "\"" + e.getValue() + "\"";
			}
			s += "}";
			return s;
		}
		
		@android.webkit.JavascriptInterface
		public String getDownloadFiles()
		{
			String s = "{";
			File dir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
			s += "\"catalog\":\"" + dir.getAbsolutePath() + "\"";
			File[] files = dir.listFiles();
			s += ",\"files\":[";
			boolean flag = false;
			if (files != null)
			{
				for (File file : files)
				{
					String name = file.getName();
					if (name.toLowerCase().matches("(.*).wav"))
					{
						if (flag) s += ","; else flag = true;
						s += "\"" + name + "\"";
					}
				}
			}
			s += "]}";
			return s;
		}
		
		@android.webkit.JavascriptInterface
		public boolean startLoadMelody(String filename)
		{
			SirenaBT.Wave.Valid valid = sirena.SetMelody(filename);
			return !(valid == SirenaBT.Wave.Valid.ErrorFile || valid == SirenaBT.Wave.Valid.ErrorFormat);
		}
		
		@android.webkit.JavascriptInterface
		public boolean setUpdate(String fw, String hash)
		{
			sirena.SetUpdate(fw);
			return true;
		}

		/*@android.webkit.JavascriptInterface
		public void checkSerial(String hash)
		{
			sirena.checkSerial(hash);
		}*/
		
		@android.webkit.JavascriptInterface
		public int serialCode()
		{
			return sirena.serialCode();
		}
	}
	
	private uiJSI jsi;
	
	private void createSirena()
	{
		sirena = new SirenaBT(this)
		{
			@Override
			public void onWait()
			{
				jsi.config = null;
				w.loadUrl("file:///android_asset/wait.htm");
			}
			
			@Override
			public void onAdapterNotFound()
			{
				w.loadUrl("file:///android_asset/adapternotfound.htm");
			}
			
			@Override
			public void onAdapterOn() {}
			
			@Override
			public void onAdapterOff()
			{
				w.loadUrl("file:///android_asset/adapteroff.htm");
			}
			
			@Override
			public void onDeviceNotFound()
			{
				w.loadUrl("file:///android_asset/devicenotfound.htm");
			}
			
			@Override
			public void onDeviceConnected()
			{
				tryConnect = TRY_CONNECT;
			}
			
			@Override
			public void onDeviceError()
			{
				if (tryConnect > 0)
				{
					tryConnect--;
					debug.p("Try connect " + tryConnect);
					sirena.reset();
				}
				else
				{
					w.loadUrl("file:///android_asset/deviceerror.htm");
				}
			}
			
			@Override
			public void onSirenaSpeak(SirenaSpeak type)
			{
				if (type == SirenaSpeak.End) w.loadUrl("javascript:ui.cbSpeak(false)");
			}
			
			@Override
			public void onSirenaPlay(SirenaPlay type, int index)
			{
				//debug.p("Play: " + type + ", " + index);
				if (type == SirenaPlay.Play)
				{
					w.loadUrl("javascript:ui.cbPlay(true, " + index + ")");
				}
				else
				{
					w.loadUrl("javascript:ui.cbPlay(false, " + index + ")");
				}
			}
			
			@Override
			public void onSirenaFS(SirenaFS type)
			{
				debug.p("FS: " + type);
				if (type == SirenaFS.Ready)
				{
					w.loadUrl("file:///android_asset/main.htm");
				}
				else if (type == SirenaFS.Save)
				{
					w.loadUrl("javascript:ui.cbSaveFS()");
				}
				else
				{
					w.loadUrl("file:///android_asset/fserror.htm");
				}
			}
			
			@Override
			public void onSerialError()
			{
				w.loadUrl("file:///android_asset/serialerror.htm");
			}
			
			@Override
			public void onInternetError()
			{
				w.loadUrl("file:///android_asset/interneterror.htm");
			}
			
			@Override
			public void onBuildError()
			{
				w.loadUrl("file:///android_asset/builderror.htm");
			}
			
			@Override
			public void onConfig(Config config)
			{
				speakSamples = config.speakSamples;
				jsi.config = config;
			}
			
			@Override
			public void onSirenaPin(SirenaPin type, String pin)
			{
				if (type == SirenaPin.Change)
				{
					w.loadUrl("javascript:ui.cbSetPINCode(false)");
				}
				else if (type == SirenaPin.Error)
				{
					w.loadUrl("javascript:ui.cbSetPINCode(true)");
				}
			}
			
			@Override
			public void onSirenaConfigChange(SirenaConfigChange type, int index)
			{
				if (type == SirenaConfigChange.ChangeButton)
				{
					w.loadUrl("javascript:ui.cbSetButton(" + index + ")");
				}
				else if (type == SirenaConfigChange.ChangeMeasure)
				{
					w.loadUrl("javascript:ui.cbSetMeasure(" + index + ")");
				}
				else if (type == SirenaConfigChange.ChangePressMaximum)
				{
					w.loadUrl("javascript:ui.cbSetPressMax(" + index + ")");
				}
				else if (type == SirenaConfigChange.ChangeIdleMaximum)
				{
					w.loadUrl("javascript:ui.cbSetIdleMax(" + index + ")");
				}
				else if (type == SirenaConfigChange.ChangePulsesMaximum)
				{
					w.loadUrl("javascript:ui.cbSetPulsesMax(" + index + ")");
				}
				else if (type == SirenaConfigChange.ChangeDeviation)
				{
					w.loadUrl("javascript:ui.cbSetDeviation(" + index + ")");
				}
				else if (type == SirenaConfigChange.ChangePlayOnce)
				{
					w.loadUrl("javascript:ui.cbSetPlayOnce(" + index + ")");
				}
				else if (type == SirenaConfigChange.ChangePlayToEnd)
				{
					w.loadUrl("javascript:ui.cbSetPlayToEnd(" + index + ")");
				}
				else if (type == SirenaConfigChange.ChangeNoWidth)
				{
					w.loadUrl("javascript:ui.cbSetNoWidth(" + index + ")");
				}
				else if (type == SirenaConfigChange.ChangeNoBounceHigh)
				{
					w.loadUrl("javascript:ui.cbSetNoBounceHigh(" + index + ")");
				}
				else if (type == SirenaConfigChange.ChangeStandard)
				{
					w.loadUrl("javascript:ui.cbSetStandard(" + index + ")");
				}
				else if (type == SirenaConfigChange.ChangeVolume)
				{
					w.loadUrl("javascript:ui.cbSetVolume()");
				}
				else if (type == SirenaConfigChange.ChangeVolumeBeep)
				{
					w.loadUrl("javascript:ui.cbSetVolumeBeep()");
				}
				else if (type == SirenaConfigChange.ChangeSleep)
				{
					w.loadUrl("javascript:ui.cbSetSleep()");
				}
				else if (type == SirenaConfigChange.Saved)
				{
					w.loadUrl("javascript:ui.cbSaveConfig()");
				}
			}
			
			@Override
			public void onSirenaMelodyChange(SirenaMelody type, int value)
			{
				if (type == SirenaMelody.Start)
				{
					w.loadUrl("javascript:ui.cbSetMelody(" + value + ")");
				}
				else if (type == SirenaMelody.Progress)
				{
					w.loadUrl("javascript:ui.cbProgressMelody(" + value + ")");
				}
				else if (type == SirenaMelody.End)
				{
					w.loadUrl("javascript:ui.cbEndMelody()");
				}
				else if (type == SirenaMelody.Delete)
				{
					w.loadUrl("javascript:ui.cbDeleteMelody(" + value + ")");
				}
				else if (type == SirenaMelody.Clear)
				{
					w.loadUrl("javascript:ui.cbDeleteMelody(-1)");
				}
				else if (type == SirenaMelody.Error || type == SirenaMelody.Full)
				{
					w.loadUrl("javascript:ui.cbErrorMelody()");
				}
			}
			
			@Override
			public void onSirenaEventChange(SirenaEvent type, int value)
			{
				if (type == SirenaEvent.Change)
				{
					w.loadUrl("javascript:ui.cbChangeEvent(" + value + ")");
				}
				else if (type == SirenaEvent.Delete)
				{
					w.loadUrl("javascript:ui.cbDeleteEvent(" + value + ")");
				}
				else if (type == SirenaEvent.Clear)
				{
					w.loadUrl("javascript:ui.cbDeleteEvent(-1)");
				}
			}
			
			@Override
			public void onSirenaLogEvent(SirenaLogEvent event)
			{
				jsi.logEvent = event;
				w.loadUrl("javascript:ui.cbEvent()");
			}
			
			@Override
			public void onStartUpdate(int length)
			{
				w.loadUrl("javascript:ui.cbStartUpdate(" + length + ")");
			}
			
			@Override
			public void onProgressUpdate(int index)
			{
				w.loadUrl("javascript:ui.cbProgressUpdate(" + index + ")");
			}
		};
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		ma = this;
		tryConnect = TRY_CONNECT;
		
		setContentView(R.layout.layout);
		
		w = findViewById(R.id.webView);
		
		w.getSettings().setJavaScriptEnabled(true);
		jsi = new uiJSI();
		w.addJavascriptInterface(jsi, "jsi");
		
		
		settings = getSharedPreferences(getString(R.string.app_name), Context.MODE_PRIVATE);
		
//		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)
//		{
			if
			(
					checkSelfPermission(Manifest.permission.BLUETOOTH) != PackageManager.PERMISSION_GRANTED
					|| checkSelfPermission(Manifest.permission.BLUETOOTH_ADMIN) != PackageManager.PERMISSION_GRANTED
					|| checkSelfPermission(Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED
					|| checkSelfPermission(Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED
					|| checkSelfPermission(Manifest.permission.BLUETOOTH_ADVERTISE) != PackageManager.PERMISSION_GRANTED
					|| checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED
					|| checkSelfPermission(Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED
			)
			{
				requestPermissions(new String[] {
						Manifest.permission.BLUETOOTH,
						Manifest.permission.BLUETOOTH_ADMIN,
						Manifest.permission.BLUETOOTH_CONNECT,
						Manifest.permission.BLUETOOTH_SCAN,
						Manifest.permission.BLUETOOTH_ADVERTISE,
						Manifest.permission.READ_EXTERNAL_STORAGE,
						Manifest.permission.RECORD_AUDIO
				}, 1);
			}
//		}
		
		createSirena();
	}
	
	@Override
	protected void onPause()
	{
		super.onPause();
		SharedPreferences.Editor edit = settings.edit();
		int n = 0;
		for (int i : jsi.buttons.emoji)
		{
			edit.putInt("emoji" + n, i);
			n++;
		}
		n = 0;
		for (int i : jsi.buttons.mld)
		{
			edit.putInt("mld" + n, i);
			n++;
		}
		n = -1;
		for (HashMap.Entry e : jsi.eventName.entrySet())
		{
			String key = e.getKey().toString();
			int i = Integer.parseInt(key);
			if (i > n) n = i;
			edit.putString("event" + key, e.getValue().toString());
		}
		edit.putInt("eventEnd", n);
		edit.apply();
	}
	
	@Override
	protected void onResume()
	{
		super.onResume();
		for (int i = 0; i <= 7; i++)
		{
			if (settings.contains("emoji" + i))
			{
				jsi.buttons.emoji.set(i, settings.getInt("emoji" + i, i));
			}
			if (settings.contains("mld" + i))
			{
				jsi.buttons.mld.set(i, settings.getInt("mld" + i, -1));
			}
		}
		int n = settings.getInt("eventEnd", -1);
		jsi.eventName.clear();
		for (int i = 0; i <= n; i++)
		{
			String key = String.valueOf(i);
			String value =  settings.getString("event" + i, "");
			jsi.eventName.put(key, value);
		}
		w.loadUrl("javascript:if (window.ui) {ui.cbButtons();ui.cbEventName();}");
	}
}
