#ifndef _M6242_H_
#define _M6242_H_
#include <time.h>
#include <stdint.h>
#include <time.h>

#define RTCD_HOLD		0x01
#define RTCD_BUSY		0x02
#define RTCD_IRQ_FLAG	0x04
#define RTCD_30ADJ		0x08
#define RTCE_STDP_DIS	0x01
#define RTCE_STDP_EN	0x00
#define RTCE_INTRT		0x02
#define RTCE_STDP		0x00
#define RTCE_TM_1_64S	0x00
#define RTCE_TM_1S		0x04
#define RTCE_TM_1M		0x08
#define RTCE_TM_1H		0x0C
#define RTCF_RESET		0x01
#define RTCF_STOP		0x02
#define RTCF_24H		0x04
#define RTCF_12H		0x00
#define RTCF_TEST_EN	0x08
#define RTCF_TEST_DIS	0x00

void m6242_init (void);
char* m6242_read_time_str (void);
char* m6242_read_date_str (void);
void m6242_read_time_raw (uint8_t* data);
void m6242_read_date_raw (uint8_t* data);
void m6242_read_tm (void);
time_t* m6242_read_timestamp (void);
void m6242_settime (uint8_t h, uint8_t m, uint8_t s);
void m6242_setdate (uint8_t d, uint8_t m, uint8_t y);

#endif //_M6242_H_
