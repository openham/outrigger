#include <Windows.h>

#include "datetime.h"

uint64_t ms_ticks(void)
{
	return GetTickCount64();
}

void ms_sleep(unsigned msecs)
{
	Sleep(msecs);
}
