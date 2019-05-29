/*
 * geiger.c
 *
 * Created: 2014-04-20 12:01:47
 *  Author: Marek
 */ 

#include <stdlib.h>
#include "geiger.h"

#define PRESCALER 57		//For STS-5/SBM-20/BOI-33 sensor tube 0.0057 * 10000 to avoid float

volatile uint16_t geiger_pulses[60];

void geiger_init (void) {
    uint8_t i;
    for (i=0; i<60; i++) {
        geiger_pulses[i] = 0;
    }
    // Initialize counter here
}

uint16_t cpm (void) {
	uint16_t result = 0;
	uint8_t i;
	
	for (i=0; i < 60; i++) {
		result += geiger_pulses[i];
	}
	
	return result;
}

uint32_t cpm2sievert (uint16_t cpm) {
	return (PRESCALER * cpm);
}
