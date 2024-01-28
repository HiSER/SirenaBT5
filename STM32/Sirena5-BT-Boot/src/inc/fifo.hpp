/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 22 февр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#ifndef INC_FIFO_HPP_
#define INC_FIFO_HPP_ 1

#include <main.hpp>

/**
 * Example
 *
 * include <fifo.hpp>
 *
 * void main(void)
 * {
 *  	FIFO8* fifo = new FIFO8(16); // FIFO 16 bytes
 *
 *  	fifo->push('A');
 *
 *  	int w = fifo->write("0123456789", 10);
 *  	//w = 10
 *
 *  	fifo->push('B');
 *
 *  	while (!fifo->empty())
 *  	{
 *  		putchar(fifo->pop());
 *  	}
 *
 *  	//putchar -> A0123456789B
 * }
 */

class FIFO8
{
public:

	FIFO8(int size);			/* up to 65535 bytes */
	~FIFO8();

	void push(uint8 data);		/* no length check, use full() */
	uint8 pop(void);			/* no length check, use empty() */

	int write(const void* src, int length);
	int read(void* dst, int length);

	bool empty(void);
	bool full(void);

	void clear(void);

protected:

	uint8* _buffer;

	uint16 _in;
	uint16 _out;

	uint16 _size;
	uint16 _length;

};

#endif
