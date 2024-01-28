package com.hiserlitvin.sirenabt5;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.os.Environment;
import android.os.Process;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public abstract class MicRecord implements Runnable
{
	public abstract void onStart();
	public abstract void onError();
	public abstract void onEnd(byte[] buffer);
	
	private int TimeMaximum = 5000;
	
	private MainActivity ma;
	private int samples;
	private byte[] buffer;
	private Boolean isRecord;
	
	public MicRecord(MainActivity mainActivity, int samples)
	{
		ma = mainActivity;
		this.samples = samples;
	}
	
	public void stop()
	{
		isRecord = false;
	}
	
	private class RecordEnd implements Runnable
	{
		private byte[] buffer;
		public RecordEnd(byte[] buffer)
		{
			this.buffer = buffer;
		}
		public void volume()
		{
			int i, s;
			double sample, error, level;
			double blur = 0;
			double peak = 0;
			double peakRaw = 0;
			for (i = 0; i < buffer.length; i++)
			{
				sample = (128.0 - (buffer[i] & 0xFF)) / 128;
				error = sample - blur;
				blur += error / 2;
				peak = Math.max(peak, Math.abs(blur));
				peakRaw = Math.max(peakRaw, Math.abs(sample));
			}
			level = 1 / peak;
			if (level < 0.2) level = 0.2;
			if (level > 15) level = 15;
			for (i = 0; i < buffer.length; i++)
			{
				sample = 128 - (buffer[i] & 0xFF);
				sample *= level;
				if (sample > 128) sample = 128;
				if (sample < -127) sample = -127;
				buffer[i] = (byte)(128 - sample);
			}
			debug.p("level: " + level + " (" + (1 / peak) + " / " + (1 / peakRaw) + ")");
			debug.p("peak: " + peak + " / " + peakRaw);
		}
		@Override
		public void run()
		{
			onEnd(buffer);
		}
	}
	
	@Override
	public void run()
	{
		//int samples = 8000;
		int channels = AudioFormat.CHANNEL_IN_MONO;
		int format = AudioFormat.ENCODING_PCM_8BIT;
		int bufferSize = (int)(samples / 12.5) /*AudioRecord.getMinBufferSize(samples, channels, format)*/;
		//debug.p("MicRecord bufferSize:" + bufferSize);
		Process.setThreadPriority(Process.THREAD_PRIORITY_URGENT_AUDIO);
		AudioRecord record = new AudioRecord(MediaRecorder.AudioSource.MIC, samples, channels, format, bufferSize);
		if (record.getState() == AudioRecord.STATE_INITIALIZED)
		{
			record.startRecording();
			ma.runOnUiThread(new Runnable()
			{
				@Override
				public void run() { onStart(); }
			});
			isRecord = true;
			buffer = new byte[0];
			while (isRecord)
			{
				byte[] buff = new byte[bufferSize];
				int bytes = record.read(buff, 0 , buff.length);
				byte[] tmp = new byte[buffer.length + bytes];
				System.arraycopy(buffer, 0, tmp, 0, buffer.length);
				System.arraycopy(buff, 0, tmp, buffer.length, bytes);
				buffer = tmp;
				if (buffer.length >= (samples * TimeMaximum / 1000))
				{
					isRecord = false;
				}
			}
			record.stop();
			record.release();
			RecordEnd re = new RecordEnd(buffer);
			re.volume();
			ma.runOnUiThread(re);
		}
		else
		{
			ma.runOnUiThread(new Runnable()
			{
				@Override
				public void run() { onError(); }
			});
		}
	}
}

