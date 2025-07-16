#pragma once

#define TITLE_ID 0x415688E7 // Phoenix

#include <xtl.h>
#include <stdint.h>

extern "C"
{
	NTSYSAPI DWORD NTAPI XamGetCurrentTitleId(VOID);
}
