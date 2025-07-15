#include "std_include.hpp"

// xbox kernel stuff
extern "C"
{
	void* ExAllocatePoolWithTag(DWORD poolType, SIZE_T size, DWORD tag);
	void KeDelayExecutionThread(int WaitMode, bool Alertable, PLARGE_INTEGER Interval);
	void DbgPrint(const char* fmt, ...);
}