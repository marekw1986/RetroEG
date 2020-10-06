#ifndef _MC6840_H_
#define _MC6840_H_

#include <inttypes.h>

#define MC6840_CON13 	(*(volatile uint8_t*)0x6480)
#define MC6840_CON2 	(*(volatile uint8_t*)0x6481)
#define MC6840_STATUS 	(*(volatile uint8_t*)0x6481)
#define MC6840_TIMER1 	(*(volatile uint16_t*)0x6482)
#define MC6840_TIMER2 	(*(volatile uint16_t*)0x6484)
#define MC6840_TIMER3 	(*(volatile uint16_t*)0x6486)

#define Swap2Bytes(val) \
 ( (((val) >> 8) & 0x00FF) | (((val) << 8) & 0xFF00) )

#define TM_COUNTER_OUTPUT_ENABLE		0x80
#define TM_COUNTER_OUTPUT_DISABLE		0x00

#define TM_INTERUPT_ENABLE				0x40
#define TM_INTERUPT_DISABLE				0x00

#define TM_CONT_OP_MODE1				0x00
#define TM_FREQ_COMP_MODE1				0x20
#define TM_CONT_OP_MODE2				0x10
#define TM_PULSE_WIDTH_COMP_MODE1		0x30
#define TM_SINGLE_SHOT_MODE1			0x08
#define TM_FREQ_COMP_MODE2				0x28
#define TM_SINGLE_SHOT_MODE2			0x18
#define TM_PULSE_WIDTH_COMP_MODE2		0x38

#define TM_NORMAL_16BIT					0x00
#define TM_DUAL_8BIT					0x04

#define TM_EXT_CLK						0x00
#define TM_SYS_CLK						0x02

#define TMCR1_ALL_TIMERS_ALLOWED		0x00
#define TMCR1_ALL_TIMERS_PRESET			0x01

#define TMCR2_WRITE_CR3					0x00
#define TMCR2_WRITE_CR1					0x01

#define TMCR3_T3_CLK_NO_PRESCALER		0x00
#define TMCR3_T3_CLK_DIV8_PRESCALER		0x01

#define STS20_PRESCALER 57		//For STS-5/SBM-20/BOI-33 sensor tube 0.0057 * 10000 to avoid float
#define GEIGER_USV(cpm)		STS20_PRESCALER * cpm;


void mc6840_init (void);
uint8_t millis(void);
uint32_t uptime(void);
uint16_t get_geiger_pulses (void);
char* get_usiv_str (uint16_t cpmin, char * des);
void set_sound_frequency (uint16_t freq);

#endif //_MC6840_H_
