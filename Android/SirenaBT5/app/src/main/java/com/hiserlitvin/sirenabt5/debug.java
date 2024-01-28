package com.hiserlitvin.sirenabt5;

import android.util.Log;

public final class debug
{
	static private String TAG = "SirenaBT5";
	
	static final public void p(String msg)
	{
		Log.d(TAG, msg);
	}
}
