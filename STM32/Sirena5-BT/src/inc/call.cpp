/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 24 февр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#include <call.hpp>

CALL::tProc CALL::stack[];
uint16 CALL::in;
uint16 CALL::out;
uint16 CALL::length;
uint16 CALL::lengthMax;

void CALL::Init(void)
{
	memset(&stack[0], 0, sizeof(tProc) * CallStackSize);
	in = 0;
	out = 0;
	length = 0;
	lengthMax = 0;
}

void CALL::DeInit(void)
{

}

void CALL::__fatal(const char* text)
{
	DBGPN_FATAL("CALL: ");
	DBGP_FATAL(text);
	LOOP
	{
		IWDG_ReloadCounter();
	}
}

void CALL::Thread(void)
{
	CALL::tProc* proc;
	if (length != 0)
	{
		do
		{
			proc = &stack[out++];
			if (out == CallStackSize) out = 0;
		}
		while (!proc->queue || !(proc->time == NULL || proc->time->interval()));
		proc->callback(proc->param);
		if (proc->queue)
		{
			if (!proc->interval)
			{
				proc->queue = false;
				if (proc->time != NULL) delete(proc->time);
				length--;
			}
		}
	}
	else
	{
		__fatal("empty stack");
	}
}

int CALL::setInterval(tCallback callback, int milli, void* param)
{
	CALL::tProc* proc;
	int handle;
	if (length == CallStackSize) __fatal("overflow setInterval");
	do
	{
		proc = &stack[in];
		handle = in;
		in++;
		if (in == CallStackSize) in = 0;
	}
	while (proc->queue);
	proc->callback = callback;
	proc->param = param;
	proc->time = new TIMER(milli);
	proc->interval = true;
	proc->queue = true;
	length++;
	if (length > lengthMax) lengthMax = length;
	return handle;
}

int CALL::setTimeout(tCallback callback, int milli, void* param)
{
	CALL::tProc* proc;
	int handle;
	if (length == CallStackSize) __fatal("overflow setTimeout");
	do
	{
		proc = &stack[in];
		handle = in;
		in++;
		if (in == CallStackSize) in = 0;
	}
	while (proc->queue);
	proc->callback = callback;
	proc->param = param;
	proc->time = new TIMER(milli);
	proc->interval = false;
	proc->queue = true;
	length++;
	if (length > lengthMax) lengthMax = length;
	return handle;
}

int CALL::setImmediate(tCallback callback, void* param)
{
	CALL::tProc* proc;
	int handle;
	if (length == CallStackSize) __fatal("overflow setImmediate");
	do
	{
		proc = &stack[in];
		handle = in;
		in++;
		if (in == CallStackSize) in = 0;
	}
	while (proc->queue);
	proc->callback = callback;
	proc->param = param;
	proc->time = NULL;
	proc->interval = false;
	proc->queue = true;
	length++;
	if (length > lengthMax) lengthMax = length;
	return handle;
}

void CALL::clear(int handle)
{
	if (handle >= 0 && handle < CallStackSize && length > 0)
	{
		CALL::tProc* proc = &stack[handle];
		if (proc->queue)
		{
			proc->queue = false;
			if (proc->time != NULL) delete(proc->time);
			length--;
		}
	}
	else
	{
		DBGF_WARNING("CALL: no clear %i", handle);
	}
}

uint16 CALL::getCalls(void)
{
	return length;
}

uint16 CALL::getMaximumCalls(void)
{
	return lengthMax;
}
