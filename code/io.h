#ifndef _IO_H_
#define _IO_H_

#include <stdint.h>

#define PORT (*(volatile uint8_t*)0x7C00)
#define BTNS (*(volatile uint8_t*)0x6300)

#define PA_8255 (*(volatile uint8_t*)0x6600)
#define PB_8255	(*(volatile uint8_t*)0x6601)
#define PC_8255	(*(volatile uint8_t*)0x6602)
#define CONF_8255 (*(volatile uint8_t*)0x6603)

#define BTN_UP	0x01
#define BTN_DN	0x02
#define BTN0	0x04
#define BTN1	0x08
#define BTN2	0x10
#define BTN3	0x20
#define BTN4	0x40
#define BTN5	0x80

#define RS485_PIN	  0x20
#define BACKLIGHT_PIN 0x10

#define SHORT_WAIT 5    // 5 * 20ms = 100 ms.

typedef struct key {
    uint8_t pin;
    uint8_t last_state;
	uint8_t timer;
	void (*push_proc)(void);
} key_t;

void __fastcall__ key_init (key_t *key, uint8_t pin, void (*push_proc)(void));
void __fastcall__ key_update (key_t *key);
void __fastcall__ port_write (uint8_t data);
void __fastcall__ port_set (uint8_t data);
void __fastcall__ port_clr (uint8_t data);
void __fastcall__ port_tgl (uint8_t data);

#endif
