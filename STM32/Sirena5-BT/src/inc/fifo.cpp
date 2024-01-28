/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 22 февр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#include <fifo.hpp>

FIFO8::FIFO8(int size)
{
	_buffer = (uint8*)malloc(size);
	_size = size;
	clear();
}

FIFO8::~FIFO8()
{
	free(_buffer);
}

void FIFO8::push(uint8 data)
{
	if (_length >= _size) return;
	_buffer[_in++] = data;
	if (_in == _size) _in = 0;
	_length++;
}

uint8 FIFO8::pop(void)
{
	if (_length == 0) return 0;
	uint8 data;
	data = _buffer[_out++];
	if (_out == _size) _out = 0;
	_length--;
	return data;
}

int FIFO8::write(const void* src, int length)
{
	if (length < 0)
	{
		length = _length;
	}
	else
	{
		if (src == NULL) return 0;
		uint8* data = (uint8*)src;
		int free = (_size - _length);
		int len = ((length > free) ? free : length);
		length = len;
		_length += len;
		free = _size - _in;
		if (len > free)
		{
			memcpy(&_buffer[_in], data, free);
			_in = 0;
			data += free;
			len -= free;
		}
		memcpy(&_buffer[_in], data, len);
		_in += len;
		if (_in == _size) _in = 0;
	}
	return length;
}

int FIFO8::read(void* dst, int length)
{
	if (length < 0)
	{
		length = _length;
	}
	else
	{
		if (dst == NULL) return 0;
		uint8* data = (uint8*)dst;
		int avail;
		int len = ((length > _length) ? _length : length);
		length = len;
		_length -= len;
		avail = _size - _out;
		if (len > avail)
		{
			memcpy(data, &_buffer[_out], avail);
			_out = 0;
			data += avail;
			len -= avail;
		}
		memcpy(data, &_buffer[_out], len);
		_out += len;
		if (_out == _size) _out = 0;
	}
	return length;
}

bool FIFO8::empty(void)
{
	return (_length == 0);
}

bool FIFO8::full(void)
{
	return (_length == _size);
}

void FIFO8::clear(void)
{
	_in = 0;
	_out = 0;
	_length = 0;
}
