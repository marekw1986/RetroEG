#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>

#define EEPROM_ADDRESS 0x4000
#define EEConfig (*(volatile cfg_t*)EEPROM_ADDRESS)

typedef struct cfg_t {
	int8_t	timezone;
	uint8_t dst;
	uint8_t element2;
	uint8_t element3;
} cfg_t;

#endif
