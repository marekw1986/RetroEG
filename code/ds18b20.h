#ifndef DS18B20_H
#define DS18B20_H

#include <stdint.h>

/*
 * Reads a DS18B20 on the bus and reports its temperature in
 * milli-degrees Celsius (e.g. 23456 = 23.456C, -5250 = -5.250C).
 * Returns 1 on success, 0 if nothing answered the bus reset.
 *
 * Assumes: a single DS18B20 on the bus (uses Skip ROM - more than
 * one device would collide during the scratchpad read), and an
 * externally-powered sensor, not parasite-powered off the data
 * line - see the comment above the conversion-wait loop in
 * ds18b20.c if that doesn't match your wiring.
 */
uint8_t ds18b20_read_temp(int32_t *millic);

#endif
