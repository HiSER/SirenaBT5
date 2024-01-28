/**
 * HiSER (c)2019 Litvin Artem Vasilyevich
 * Date 24 февр. 2019 г.
 * sirenabt5@artemlitvin.com
 */

#include <bt.hpp>
#include <call.hpp>
#include <encoders.hpp>
#include <remapVect.h>
#include <aes.h>
#include <hash.hpp>

#define FLASH_PAGE_SIZE		2048UL
#define FLASH_APP_START		0x08008000UL
#define FLASH_APP_END		0x08040000UL

#if ((FLASH_PAGE_SIZE + 6) % AES_BLOCKLEN) == 0
#define PACKET_RESERVE_SIZE 0
#else
#define PACKET_RESERVE_SIZE (AES_BLOCKLEN - ((FLASH_PAGE_SIZE + 6) % AES_BLOCKLEN))
#endif

typedef struct __PACKED
{
	uint32 address;
	uint8 buffer[FLASH_PAGE_SIZE];
	uint8 reserve[PACKET_RESERVE_SIZE];
	uint16 crc;
}
tPacket;

const uint8 aesKey[] =
	{
		0x5C, 0x6C, 0x6E, 0x3D, 0x79, 0x7A, 0x7B, 0x64,
		0x4E, 0x72, 0x59, 0x4C, 0x4D, 0x29, 0x5E, 0x43,
		0x52, 0x67, 0x6B, 0x21, 0x5F, 0x24, 0x4F, 0x2C,
		0x32, 0x4F, 0x4D, 0x3B, 0x6B, 0x46, 0x28, 0x33
	};

ATP(PACKET)
{
	char* tmp = (char*)malloc(32);

	if (at->argc == 2 && at->argv[0]->isNumber && at->argv[1]->length == (sizeof(tPacket) * 4 / 3))
	{
		int len;
		tPacket* packet;
		struct AES_ctx* ctx;

		len = at->argv[1]->length;
		packet = (tPacket*)BASE64::decode(at->argv[1]->data, &len);

		ctx = (struct AES_ctx*)malloc(sizeof(struct AES_ctx));
		AES_init_ctx_iv(ctx, aesKey, aesKey);
		AES_CBC_decrypt_buffer(ctx, (uint8*)packet, len);

		if (packet->crc == CRC16::calc(packet, sizeof(tPacket) - 2))
		{
			if (packet->address >= FLASH_APP_START && packet->address < FLASH_APP_END && (packet->address & (FLASH_PAGE_SIZE - 1)) == 0)
			{
				uint32 end = packet->address + FLASH_PAGE_SIZE;
				uint32* data = (uint32*)&packet->buffer[0];
				FLASH_Unlock();
				FLASH_ErasePage(packet->address);
				for (; packet->address < end; packet->address += 4)
				{
					FLASH_ProgramWord(packet->address, *data++);
				}
				FLASH_Lock();
				sprintf(tmp, "OK;%i", (int)at->argv[0]->sNumber);

			}
			else
			{
				sprintf(tmp, "ERROR;%i;ADDRESS", (int)at->argv[0]->sNumber);
			}
		}
		else
		{
			sprintf(tmp, "ERROR;%i;CRC", (int)at->argv[0]->sNumber);
		}

		free(ctx);
		free(packet);
	}
	else
	{
		sprintf(tmp, "ERROR");
	}

	ANSWER(tmp);
	free(tmp);
}

ATP(BOOT)
{
	if (FLASH_OB_GetRDP() == RESET)
	{
		ANSWER("READY;RDP");
	}
	else
	{
		ANSWER("READY;OK");
	}
}

extern void callMainCallback(__UNUSED void* param);
ATP(MAIN)
{
	if (testMain())
	{
		CALL::setInterval(callMainCallback, 500);
		ANSWER("OK");
	}
	else
	{
		ANSWER("ERROR");
	}
}

/* no param */
ATP(EMPTY)
{
	ANSWER(ANSWERBOOL(at->argc == 0));
}

/* no param */
ATP(ATI)
{
	ANSWER("Device;" BUILD);
	ANSWER("Build;" ADDQUOTES(VERSION) ";" ADDQUOTES(__BUILD__));
	ANSWER("Manufacturer;BeepHorn");
	ANSWER("Author;Artem Litvin;sirenabt5@artemlitvin.com");
}

/* no param */
ATP(MEM)
{
	char* tmp = (char*)malloc(32);
	sprintf
		(
			tmp, "%u;%u;%u;%u",
			getFreeHeap(), getMinimumEverFreeHeap(),
			CALL::getCalls(), CALL::getMaximumCalls()
		);
	ANSWER(tmp);
	free(tmp);
}

const BT::tListAT BT::listAT[] =
	{
		AT("^BOOT",			BOOT)
		AT("^MAIN",			MAIN)
		AT("PACKET", 		PACKET)

		/* BT answers */
		ATI("+READY",		BT::readyCmd)
		ATI("+VERSION",		BT::testAT)
		ATI("CONNECTED",	BT::connectedCmd)
		ATI("+DISC",		BT::disconnectedCmd)

		/* diagnostics */
		AT("AT",			EMPTY)
		AT("ATI",			ATI)
		AT("MEM",			MEM)

		/* BT answers no used */
		AT("+READY",		NULL)
		AT("+CONNECTING",	NULL)
		AT("+PAIRABLE",		NULL)
		AT("+ROLE",			NULL)
		AT("+PIN",			NULL)
		AT("+NAME",			NULL)
		AT("+BAUD",			NULL)
		AT("OK",			NULL)
		AT("+OK",			NULL)
		AT("ERROR",			NULL)
		AT("",				NULL)

		AT_NULL
	};
