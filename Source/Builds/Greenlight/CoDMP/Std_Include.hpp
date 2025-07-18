#pragma once

#define TITLE_ID 0x415608C3 // New Moon

#include <xtl.h>
#include <stdint.h>

extern "C"
{
	NTSYSAPI DWORD NTAPI XamGetCurrentTitleId(VOID);
}