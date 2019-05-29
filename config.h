#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>

#define EEPROM 0x6000
#define EEConfig (*(volatile cfg_t*)0x6000)

typedef struct cfg_t {
	uint8_t element1;
	uint8_t element2;
} cfg_t;

#endif
