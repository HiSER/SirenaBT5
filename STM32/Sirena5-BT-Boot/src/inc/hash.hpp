/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 13 мар. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#ifndef INC_HASH_HPP_
#define INC_HASH_HPP_ 1

#include <main.hpp>

class CRC8
{
public:

	static uint8 byte(uint8 data, uint8 oldCRC);
	static uint8 calc(const void *data, uint16 length);

private:

	static const uint8 InitValue		= 0xFF;

	static const uint8 table[];

};

class CRC16
{
public:

	static uint16 byte(uint8 data, uint16 oldCRC);
	static uint16 calc(const void *data, uint16 length);

private:

	static const uint16 InitValue		= 0xFFFF;

	static const uint16 table[];

};

#endif
