#pragma once

#define TITLE_ID 0x415608C3 // COD: Black Ops II

#define IS_XENIA TRUE // Temp value for now.

#include <xtl.h>
#include <stdint.h>

extern "C"
{
	NTSYSAPI DWORD NTAPI XamGetCurrentTitleId(VOID);
}
