#include "time.h"
#include "ff.h"

volatile uint32_t milliseconds = 0;
volatile uint32_t uptime_value = 0;

DWORD get_fattime (void)
{
	return(0);
}

uint32_t millis(void) {
	return milliseconds;
}

uint32_t uptime (void) {
	return uptime_value;
}

