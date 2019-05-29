#include <string.h>
#include "config.h"

cfg_t Config;

void* __fastcall__ memset (void* p, int val, size_t count);
void* __fastcall__ memcpy (void* dest, const void* src, size_t count);


void load_config (void) {
	memcpy(&Config, &EEConfig, sizeof(cfg_t));
}

void save_config (void) {
	memcpy(&EEConfig, &Config, sizeof(cfg_t));
}

void restore_default_config (void) {
	memset(&EEConfig, 0x00, sizeof(cfg_t));
	//Set default values
	save_config();
}
