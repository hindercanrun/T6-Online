#pragma once

namespace Symbols
{
	// Functions
	typedef void (*Com_Printf_t)(int channel, const char* fmt, ...);
	extern Com_Printf_t Com_Printf;
}