#pragma once

#define TITLE_ID 0x415608C3 // Black Ops II

#include <string>
#include <vector>
#include <cstdint>
#include <cwctype>
#include <algorithm>
#include <iostream>
#include <ctime>
#include <cstdio>
#include <unordered_map>
#include <fstream>

#include <xtl.h>
#include <stdio.h> // for sprintf_s
#include <stdint.h>
#include <ppcintrinsics.h>
#include <direct.h>

#include <xtl.h>
#include <xboxmath.h>

// xbox kernel stuff
extern "C"
{
	NTSYSAPI DWORD NTAPI XamGetCurrentTitleId(VOID);
	NTSYSAPI VOID NTAPI DbgPrint(const char* s, ...);
}
