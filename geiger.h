/*
 * geiger.h
 *
 * Created: 2014-04-20 12:01:23
 *  Author: Marek
 */ 


#ifndef GEIGER_H_
#define GEIGER_H_

#include <stdlib.h>
#include <inttypes.h>

// Deklaracje zmiennych zewnêtrznych
extern volatile uint16_t geiger_pulses[];


//Deklaracje funkcji
void geiger_init (void);
uint16_t cpm (void);
uint32_t cpm2sievert (uint16_t cpm);



#endif /* GEIGER_H_ */
