/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 24 февр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#ifndef INC_ENCODERS_HPP_
#define INC_ENCODERS_HPP_ 1

#include <main.hpp>
#include <hash.hpp>
#include <dbg.hpp>

/**
 * Example
 *
 * include <encoders.hpp>
 *
 * void main(void)
 * {
 *  	int length;
 *  	char* buff1;
 *  	char* buff2;
 *
 *  	buff1 = (char*)malloc(64);
 *  	sprintf(buff1, "Hello world!");
 *
 *  	length = -1;									// strlen(buff1)
 *  	buff2 = HEX::encode((uint8*)buff1, &length);	// buff2 = "HEX\0", length = count HEX symbols
 *  	printf("%i '%s'", length, buff2);
 *
 *  	free(buff1);
 *
 *  	length = -1										// strlen(buff2)
 *  	buff1 = (char*)HEX::decode(buff2, &length);		// buff1 = "Hello world!\0", length = count chars
 *  	printf("%i '%s'", length, buff1);
 *
 *  	free(buff1);
 *  	free(buff2);
 * }
 */

class CHECK
{
public:

	static uint8* encode(const char* serial, uint8 code); /* result 64 bytes, serial 31 bytes */
	static bool decode(uint8* src, const char* serial, uint8 code); /* src 64, serial 31 bytes */
	static uint8* encodeRandom(); /* result 64 bytes */

};

class HEX
{
public:

	static char* encode(const uint8* src, int* length);
	static uint8* decode(const char* src, int* length);

private:

	static inline char toChar(char c);
	static char toBIN(char c);

};

class BASE64
{
public:

	static char* encode(const uint8* src, int* length);
	static uint8* decode(const char* src, int* length);

private:

	static char toChar(char c);
	static char toBIN(char c);

};

class BASE128
{
public:

	static char* encode(const uint8* src, int* length);
	static uint8* decode(const char* src, int* length);

private:

	static char toChar(char c);
	static char toBIN(char c);

};

class WAVE7
{
public:

	static char* encode(const uint8* src, int* length);
	static uint8* decode(const char* src, int* length);

private:

	static char toChar(char c);
	static char toBIN(char c);

};

#endif
