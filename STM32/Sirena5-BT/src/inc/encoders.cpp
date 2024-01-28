/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 24 февр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#include <encoders.hpp>

/* CHECK */

uint8* CHECK::encode(const char* serial, uint8 code)
{
	uint8* result = (uint8*)malloc(64);
	uint8 i, n;
	srand(TIMER::getTick());
	for (i = 0, n = 0; i < 62; i += 2, n++)
	{
		result[i] = (uint8)serial[n];
		result[i + 1] = (uint8)(rand() & 0xFF);
	}
	do
	{
		for (i = 0; i < 61; i++)
		{
			result[i] ^= result[i + 1];
		}
		result[i] ^= result[0];
	}
	while (code-- > 0);
	*((uint16*)&result[62]) = CRC16::calc(result, 62);
	return result;
}

bool CHECK::decode(uint8* src, const char* serial, uint8 code)
{
	bool result = false;
	uint16 crc = CRC16::calc(src, 62);
	uint16 crc2 = *((uint16*)&src[62]);
	if (crc == crc2)
	{
		sint8 i, n;
		do
		{
			i = 61;
			src[i--] ^= src[0];
			for (; i >= 0; i--)
			{
				src[i] ^= src[i + 1];
			}
		}
		while (code-- > 0);
		result = true;
		for (i = 0, n = 0; i < 62; i += 2, n++)
		{
			if (src[i] != (uint8)serial[n])
			{
				result = false;
				break;
			}
		}
	}
	else
	{
		//DBGF("CHECK:: error CRC %04X / %04X", crc, crc2);
	}
	return result;
}

uint8* CHECK::encodeRandom()
{
	uint8* result = (uint8*)malloc(64);
	uint8 i;
	srand(TIMER::getTick());
	for (i = 0; i < 64; i++)
	{
		result[i] = (uint8)(rand() & 0xFF);
	}
	return result;
}

/* HEX */

char* HEX::encode(const uint8* src, int* length)
{
	char* dst = NULL;
	if (src != NULL && length != NULL)
	{
		uint16 i = 0;
		uint16 len = ((*length < 0) ? strlen((char*)src) : *length);
		uint16 mlen = len * 2 + 1;
		//DBGF("HEX::E %u/%u", len, mlen);
		dst = (char*)malloc(mlen);
		while (len-- > 0)
		{
			dst[i++] = toChar(*src >> 4);
			dst[i++] = toChar(*src++);
		}
		dst[i] = '\0';
		*length = i;
	}
	return dst;
}

uint8* HEX::decode(const char* src, int* length)
{
	uint8* dst = NULL;
	if (src != NULL && length != NULL)
	{
		uint16 i = 0;
		uint16 len = ((*length < 0) ? strlen(src) : *length);
		uint16 mlen = len / 2 + 1;
		uint8 tmp[2];
		uint8 c = 0;
		//DBGF("HEX::D %u/%u", len, mlen);
		dst = (uint8*)malloc(mlen);
		while (len-- > 0)
		{
			tmp[c++] = toBIN(*src++);
			if (c == 2)
			{
				c = 0;
				dst[i++] = (tmp[0] << 4) | tmp[1];
			}
		}
		dst[i] = '\0';
		*length = i;
	}
	return dst;
}

char HEX::toChar(char c)
{
	c = c & 0x0F;
	return c + ((c < 10) ? '0' : ('A' - 10));
}

char HEX::toBIN(char c)
{
	return (c - ((c < 'A') ? '0' : ('A' - 10))) & 0x0F;
}



/* BASE64 */

char* BASE64::encode(const uint8* src, int* length)
{
	char* dst = NULL;
	if (src != NULL && length != NULL)
	{
		uint16 i = 0;
		uint16 len = ((*length < 0) ? strlen((char*)src) : *length);
		uint16 mlen = len * 4 / 3 + 4;
		uint8 tmp[3];
		uint8 c = 0;
		//DBGF("BASE64::E %u/%u", len, mlen);
		dst = (char*)malloc(mlen);
		while (len-- > 0)
		{
			tmp[c++] = *src++;
			if (c == 3)
			{
				c = 0;
				dst[i++] = toChar(tmp[0] >> 2);
				dst[i++] = toChar((tmp[0] << 4) | (tmp[1] >> 4));
				dst[i++] = toChar((tmp[1] << 2) | (tmp[2] >> 6));
				dst[i++] = toChar(tmp[2]);
			}
		}
		if (c > 0)
		{
			dst[i++] = toChar(tmp[0] >> 2);
			if (c == 1)
			{
				dst[i++] = toChar(tmp[0] << 4);
				dst[i++] = '=';
			}
			else
			{
				dst[i++] = toChar((tmp[0] << 4) | (tmp[1] >> 4));
				dst[i++] = toChar(tmp[1] << 2);
			}
			dst[i++] = '=';
		}
		dst[i] = '\0';
		*length = i;
	}
	return dst;
}

uint8* BASE64::decode(const char* src, int* length)
{
	uint8* dst = NULL;
	if (src != NULL && length != NULL)
	{
		uint16 i = 0;
		uint16 len = ((*length < 0) ? strlen(src) : *length);
		uint16 mlen = len * 3 / 4 + 1;
		uint8 tmp[4];
		uint8 c = 0;
		//DBGF("BASE64::D %u/%u", len, mlen);
		dst = (uint8*)malloc(mlen);
		while (len-- > 0)
		{
			tmp[c++] = toBIN(*src++);
			if (c == 4)
			{
				c = 0;
								 dst[i++] = (tmp[0] << 2) | (tmp[1] >> 4);
				if (tmp[2] < 64) dst[i++] = (tmp[1] << 4) | (tmp[2] >> 2);
				if (tmp[3] < 64) dst[i++] = (tmp[2] << 6) | tmp[3];
			}
		}
		dst[i] = '\0';
		*length = i;
	}
	return dst;
}

char BASE64::toChar(char c)
{
	c = c & 0x3F;
	if (c >= 0 && c < 26) return 'A' + c;			/* 26*/
	if (c >= 26 && c < 52) return 'a' + (c - 26);	/* 26 */
	if (c >= 52 && c < 62) return '0' + (c - 52);	/* 10 */
	if (c == 62) return '+';						/* 1 */
	if (c == 63) return '/';						/* 1 */
	return '\0';
}

char BASE64::toBIN(char c)
{
	if (c >= 'A' && c <= 'Z') return c - 'A';			/* 26 */
	if (c >= 'a' && c <= 'z') return c - ('a' - 26);	/* 26 */
	if (c >= '0' && c <= '9') return c - ('0' - 52);	/* 10 */
	if (c == '+') return 62;							/* 1 */
	if (c == '/') return 63;							/* 1 */
	return 64;
}



/* BASE128 */

char* BASE128::encode(const uint8* src, int* length)
{
	char* dst = NULL;
	if (src != NULL && length != NULL)
	{
		uint16 i = 0;
		uint16 len = ((*length < 0) ? strlen((char*)src) : *length);
		uint16 mlen = len * 8 / 7 + 8;
		uint8 tmp[7];
		uint8 c = 0;
		uint8 n;
		//DBGF("BASE128::E %u/%u", len, mlen);
		dst = (char*)malloc(mlen);
		while (len-- > 0)
		{
			tmp[c++] = *src++;
			if (c == 7)
			{
				c = 0;
				dst[i++] = toChar(tmp[0] >> 1);
				for (n = 1; n < 7; n++)
				{
					dst[i++] = toChar((tmp[n - 1] << (7 - n)) | (tmp[n] >> (1 + n)));
				}
				dst[i++] = toChar(tmp[6]);
			}
		}
		if (c > 0)
		{
			dst[i++] = toChar(tmp[0] >> 1);
			for (n = 1; n < (c + 1); n++)
			{
				dst[i++] = toChar((tmp[n - 1] << (7 - n)) | (tmp[n] >> (1 + n)));
			}
			for (n = 7 - c; n > 0; n--)
			{
				dst[i++] = '=';
			}
		}
		dst[i] = '\0';
		*length = i;
	}
	return dst;
}

uint8* BASE128::decode(const char* src, int* length)
{
	uint8* dst = NULL;
	if (src != NULL && length != NULL)
	{
		uint16 i = 0;
		uint16 len = ((*length < 0) ? strlen(src) : *length);
		uint16 mlen = len * 7 / 8 + 1;
		uint8 tmp[8];
		uint8 c = 0;
		uint8 n;
		//DBGF("BASE128::E %u/%u", len, mlen);
		dst = (uint8*)malloc(mlen);
		while (len-- > 0)
		{
			tmp[c++] = toBIN(*src++);
			if (c == 8)
			{
				c = 0;
				for (n = 0; n < 7; n++)
				{
					if (tmp[n + 1] < 128) dst[i++] = (tmp[n] << (n + 1)) | (tmp[n + 1] >> (6 - n));
				}
			}
		}
		dst[i] = '\0';
		*length = i;
	}
	return dst;
}

char BASE128::toChar(char c)
{
	c = c & 0x7F;
	if (c >= 0 && c < 26) return 'A' + c;			/* 26*/
	if (c >= 26 && c < 52) return 'a' + (c - 26);	/* 26 */
	if (c >= 52 && c < 62) return '0' + (c - 52);	/* 10 */
	if (c >= 62 && c < 126) return 192 + (c - 62);	/* 64 */
	if (c == 126) return '+';						/* 1 */
	if (c == 127) return '/';						/* 1 */
	return '\0';
}

char BASE128::toBIN(char c)
{
	if (c >= 'A' && c <= 'Z') return c - 'A';			/* 26 */
	if (c >= 'a' && c <= 'z') return c - ('a' - 26);	/* 26 */
	if (c >= '0' && c <= '9') return c - ('0' - 52);	/* 10 */
	if ((unsigned char)c >= 192) return c - (192 - 62);	/* 64 */
	if (c == '+') return 126;							/* 1 */
	if (c == '/') return 127;							/* 1 */
	return 128;
}



/* WAVE7 */

char* WAVE7::encode(const uint8* src, int* length)
{
	char* dst = NULL;
	if (src != NULL && length != NULL)
	{
		uint16 i = 0;
		uint16 len = ((*length < 0) ? strlen((char*)src) : *length);
		uint8 tmp;
		dst = (char*)malloc(len);
		while (len-- > 0)
		{
			tmp = *src++;
			dst[i++] = toChar(tmp >> 1);
		}
		dst[i] = '\0';
		*length = i;
	}
	return dst;
}

uint8* WAVE7::decode(const char* src, int* length)
{
	uint8* dst = NULL;
	if (src != NULL && length != NULL)
	{
		uint16 i = 0;
		uint16 len = ((*length < 0) ? strlen(src) : *length);
		uint8 tmp;
		dst = (uint8*)malloc(len);
		while (len-- > 0)
		{
			tmp = toBIN(*src++);
			dst[i++] = tmp << 1;
		}
		dst[i] = '\0';
		*length = i;
	}
	return dst;
}

char WAVE7::toChar(char c)
{
	c = c & 0x7F;
	if (c >= 0 && c < 26) return 'A' + c;			/* 26*/
	if (c >= 26 && c < 52) return 'a' + (c - 26);	/* 26 */
	if (c >= 52 && c < 62) return '0' + (c - 52);	/* 10 */
	if (c >= 62 && c < 126) return 192 + (c - 62);	/* 64 */
	if (c == 126) return '+';						/* 1 */
	if (c == 127) return '/';						/* 1 */
	return '\0';
}

char WAVE7::toBIN(char c)
{
	if (c >= 'A' && c <= 'Z') return c - 'A';			/* 26 */
	if (c >= 'a' && c <= 'z') return c - ('a' - 26);	/* 26 */
	if (c >= '0' && c <= '9') return c - ('0' - 52);	/* 10 */
	if ((unsigned char)c >= 192) return c - (192 - 62);	/* 64 */
	if (c == '+') return 126;							/* 1 */
	if (c == '/') return 127;							/* 1 */
	return 128;
}
