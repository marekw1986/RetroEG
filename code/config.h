#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>

#define EEPROM_ADDRESS 0x4000
#define EEConfig (*(volatile cfg_t*)EEPROM_ADDRESS)

typedef struct cfg_t {
	uint8_t element1;
	uint8_t element2;
} cfg_t;

#endif
