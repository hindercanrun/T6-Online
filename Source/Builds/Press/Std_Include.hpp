#pragma once

#define TITLE_ID 0x415608C3 // Black Ops 2

#include <xtl.h>
#include <stdint.h>

extern "C"
{
	NTSYSAPI DWORD NTAPI XamGetCurrentTitleId(VOID);
}
