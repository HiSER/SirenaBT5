/**
 * Add library AltSoftSerial
 * TX		9
 * RX		8
 * WakeUP	7
 * LED		13
 * Debug baud 115200
 */

#define WAKEUP		7
#define LED			13


#define DEBUG		0
#define NOTICE		1
#define DEBUG_BAUD	115200



/* Play */

#include <AltSoftSerial.h>

AltSoftSerial playSerial;

#define PLAY_BAUD			19200
#define MLD_MIN				0
#define MLD_MAX				39
#define UART_BUFFER_SIZE	126

byte playIndex;
bool isAnswer;
bool isPlay;
bool isRepeat;
char playBuffer[UART_BUFFER_SIZE + 1];
byte playLength;

void playWakeUp()
{
	digitalWrite(WAKEUP, HIGH);
	delay(10);
	digitalWrite(WAKEUP, LOW);
}

void playStart(byte index, bool repeat = false)
{
	if (index < MLD_MIN || index > MLD_MAX)
	{
		#if NOTICE == 1
		Serial.print("playStart: error index ");
		Serial.println(index);
		#endif
		return;
	}
	if (isPlay)
	{
		#if NOTICE == 1
		Serial.println("playStart collision");
		#endif
		return;
	}
	playWakeUp();
	#if NOTICE == 1
	Serial.print("playMelody: ");
	Serial.println(index);
	#endif
	playIndex = index;
	isRepeat = repeat;
	playSerial.print("PLAY=");
	playSerial.println(playIndex);
	isPlay = true;
	digitalWrite(LED, HIGH);
}

void playStop()
{
	isPlay = false;
	isRepeat = false;
}

void getPlayMelody()
{
	playWakeUp();
	#if NOTICE == 1
	Serial.println("Query melody");
	#endif
	playSerial.println("MELODY");
}

void playInit()
{
	playSerial.begin(PLAY_BAUD);
	pinMode(WAKEUP, OUTPUT);
	digitalWrite(WAKEUP, LOW);
	pinMode(LED, OUTPUT);
	digitalWrite(LED, LOW);
	isAnswer = false;
	isRepeat = false;
	isPlay = false;
	playLength = 0;
	playBuffer[0] = '\0';
}

void playAnswer(char* cmd, char* value)
{
	if (strcmp(cmd, "PLAY") == 0)
	{
		#if NOTICE == 1
		Serial.print("Play started: ");
		Serial.println(value);
		#endif
	}
	else if (strcmp(cmd, "PLAYSTOP") == 0)
	{
		isPlay = false;
		digitalWrite(LED, LOW);
		if (strcmp(value, "OK") != 0)
		{
			#if NOTICE == 1
			Serial.print("Play stoped: ");
			Serial.println(value);
			#endif
			if (isRepeat)
			{
				#if NOTICE == 1
				Serial.print("Play repeat: ");
				Serial.println(value);
				#endif
				playSerial.print("PLAY=");
				playSerial.println(value);
			}
		}
	}
	else
	{
		#if NOTICE == 1
		Serial.print("playAnswer: ");
		Serial.print(cmd);
		Serial.print(" = ");
		Serial.println(value);
		#endif		
	}
}

void playRoutine()
{
	byte data;
	if (isAnswer)
	{
		while (playSerial.available())
		{
			data = playSerial.read();
			if (data == '\n')
			{
				if (playLength > 0)
				{
					if (playBuffer[playLength - 1] == '\r')
					{
						playLength--;
						playBuffer[playLength] = '\0';
					}
					if (playLength > 0)
					{
						char* cmd = playBuffer;
						char* value = strstr(playBuffer, ":");
						if (value == NULL)
						{
							char nullValue = 0;
							value = &nullValue;
						}
						else
						{
							(*value++) = '\0';
						}
						playAnswer(cmd, value);
					}
				}
				isAnswer = false;
				playLength = 0;
				playBuffer[0] = '\0';
			}
			else if (playLength < UART_BUFFER_SIZE)
			{
				playBuffer[playLength++] = data;
				playBuffer[playLength] = '\0';
			}
			else
			{
				#if DEBUG == 1
				Serial.println("Play overflow buffer");
				#endif
			}
		}
	}
	else
	{
		data = playSerial.read();
		if (data == '+')
		{
			isAnswer = true;
			#if DEBUG == 1
			Serial.println("Play detected answer");
			#endif
		}
	}
}

/* ------------------------------ */



/* Key */

byte keyOld;
byte melodyHigh;
byte melodyRepeat;

void setMelodyLow(byte low)
{
	#if DEBUG == 1
	Serial.print("setMelodyLow: ");
	Serial.println(low);
	#endif
	playStart(melodyHigh + low, melodyRepeat);
}

void setMelodyHigh(byte high)
{
	melodyHigh = high * 10;
	#if DEBUG == 1
	Serial.print("setMelodyHigh: ");
	Serial.println(melodyHigh);
	#endif
}

void setMelodyRepeat(bool repeat)
{
	#if NOTICE == 1
	Serial.print("setMelodyRepeat: ");
	Serial.println(repeat);
	#endif
	melodyRepeat = repeat;
}

void keyInit()
{
	keyOld = 255;
	melodyHigh = 0;
	melodyRepeat = false;
}

void keyRoutine()
{
	byte key = Serial.read();
	if (key != keyOld)
	{
		if (key >= '0' && key <= '9')
		{
			setMelodyLow(key - '0');
		}
		else
		{
			switch (key)
			{
				case 'A':
				case 'a':
					setMelodyHigh(0);
					break;
				case 'B':
				case 'b':
					setMelodyHigh(1);
					break;
				case 'C':
				case 'c':
					setMelodyHigh(2);
					break;
				case 'D':
				case 'd':
					setMelodyHigh(3);
					break;
				case 'M':
				case 'm':
					getPlayMelody();
					break;
				case 'R':
				case 'r':
					setMelodyRepeat(true);
					break;
				case 'N':
				case 'n':
					setMelodyRepeat(false);
					break;
				case 'S':
				case 's':
					playStop();
					break;
			}
		}
		keyOld = key;
	}
}

/* ------------------------------ */



/* Main */

void setup()
{
	Serial.begin(DEBUG_BAUD);
	playInit();
	keyInit();
}

void loop()
{
	keyRoutine();
	playRoutine();
}

/* ------------------------------ */
