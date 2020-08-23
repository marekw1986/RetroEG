#include "m6242.h"
#include "delay.h"
#include "io.h"

#define M6242_1_SEC_REG   (*(volatile uint8_t*)0x6400)
#define M6242_10_SEC_REG  (*(volatile uint8_t*)0x6401)
#define M6242_1_MIN_REG   (*(volatile uint8_t*)0x6402)
#define M6242_10_MIN_REG  (*(volatile uint8_t*)0x6403)
#define M6242_1_HOUR_REG  (*(volatile uint8_t*)0x6404)
#define M6242_10_HOUR_REG (*(volatile uint8_t*)0x6405)
#define M6242_1_DAY_REG   (*(volatile uint8_t*)0x6406)
#define M6242_10_DAY_REG  (*(volatile uint8_t*)0x6407)
#define M6242_1_MON_REG   (*(volatile uint8_t*)0x6408)
#define M6242_10_MON_REG  (*(volatile uint8_t*)0x6409)
#define M6242_1_YEAR_REG  (*(volatile uint8_t*)0x640A)
#define M6242_10_YEAR_REG (*(volatile uint8_t*)0x640B)
#define M6242_WEEK_REG    (*(volatile uint8_t*)0x640C)
#define M6242_CTRLD_REG   (*(volatile uint8_t*)0x640D)
#define M6242_CTRLE_REG   (*(volatile uint8_t*)0x640E)
#define M6242_CTRLF_REG   (*(volatile uint8_t*)0x640F)

extern const uint8_t digits[16];			// USUN TO POTEM

//uint8_t dot = 0;

uint8_t m6242_buf[10];
struct tm current_time;
time_t timestamp;

void m6242_init (void) {                  
	M6242_CTRLD_REG = RTCD_IRQ_FLAG;								//0x04 (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 0)              
	M6242_CTRLE_REG = (RTCE_INTRT | RTCE_STDP_EN | RTCE_TM_1S);		//0x06 (Innterrupt mode, STD.P enabled, 1 s.)
	M6242_CTRLF_REG = RTCF_RESET;									//RESET needs to be 1 to set 24H mode.
	M6242_CTRLF_REG = (RTCF_TEST_DIS | RTCF_24H);					//0x04 (TEST = 0, 24h mode, STOP = 0, RESET = 0) 
}


char* m6242_read_time_str (void) {
	M6242_CTRLD_REG = RTCD_HOLD | RTCD_IRQ_FLAG;		//Set HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 1)
	while(M6242_CTRLD_REG & RTCD_BUSY) {
		M6242_CTRLD_REG = RTCD_IRQ_FLAG;				//Clear HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 0)
		delay_16us(5);									//Required idle time
		M6242_CTRLD_REG = RTCD_HOLD | RTCD_IRQ_FLAG;	//Set HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 1)
	}  	
    m6242_buf[0] = (M6242_10_HOUR_REG & 0x03) + '0';
    m6242_buf[1] = (M6242_1_HOUR_REG & 0x0F) + '0';
    m6242_buf[2] = ':';
    m6242_buf[3] = (M6242_10_MIN_REG & 0x0F) + '0';
    m6242_buf[4] = (M6242_1_MIN_REG & 0x0F) + '0';
    m6242_buf[5] = ':';
    m6242_buf[6] = (M6242_10_SEC_REG & 0x0F) + '0';
    m6242_buf[7] = (M6242_1_SEC_REG & 0x0F) + '0';
    m6242_buf[8] = '\0';
    M6242_CTRLD_REG = RTCD_IRQ_FLAG;					//Clear HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 0)
    return m6242_buf;
}


char* m6242_read_date_str (void) {
	M6242_CTRLD_REG = RTCD_HOLD | RTCD_IRQ_FLAG;		//Set HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 1)
	while(M6242_CTRLD_REG & RTCD_BUSY) {
		M6242_CTRLD_REG = RTCD_IRQ_FLAG;				//Clear HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 0)
		delay_16us(5);									//Required idle time
		M6242_CTRLD_REG = RTCD_HOLD | RTCD_IRQ_FLAG;	//Set HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 1)
	}  	
    m6242_buf[0] = (M6242_10_DAY_REG & 0x0F) + '0';
    m6242_buf[1] = (M6242_1_DAY_REG & 0x0F) + '0';
    m6242_buf[2] = '-';
    m6242_buf[3] = (M6242_10_MON_REG & 0x0F) + '0';
    m6242_buf[4] = (M6242_1_MON_REG & 0x0F) + '0';
    m6242_buf[5] = '-';
    m6242_buf[6] = (M6242_10_YEAR_REG & 0x0F) + '0';
    m6242_buf[7] = (M6242_1_YEAR_REG & 0x0F) + '0';
    m6242_buf[8] = '\0';
    M6242_CTRLD_REG = RTCD_IRQ_FLAG;					//Clear HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 0)
    return m6242_buf;
}


void m6242_read_time_raw (uint8_t* data) {	
	M6242_CTRLD_REG = RTCD_HOLD | RTCD_IRQ_FLAG;		//Set HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 1)
	while(M6242_CTRLD_REG & RTCD_BUSY) {
		M6242_CTRLD_REG = RTCD_IRQ_FLAG;				//Clear HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 0)
		delay_16us(5);									//Required idle time
		M6242_CTRLD_REG = RTCD_HOLD | RTCD_IRQ_FLAG;	//Set HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 1)
	}   
	data[0] = (M6242_10_HOUR_REG & 0x03);
    data[1] = (M6242_1_HOUR_REG & 0x0F);
    data[2] = (M6242_10_MIN_REG & 0x0F);
    data[3] = (M6242_1_MIN_REG & 0x0F);
    data[4] = (M6242_10_SEC_REG & 0x0F);
    data[5] = (M6242_1_SEC_REG & 0x0F);
    M6242_CTRLD_REG = RTCD_IRQ_FLAG;					//Clear HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 0)	 	    
}


void m6242_read_date_raw (uint8_t* data) {
	M6242_CTRLD_REG = RTCD_HOLD | RTCD_IRQ_FLAG;		//Set HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 1)
	while(M6242_CTRLD_REG & RTCD_BUSY) {
		M6242_CTRLD_REG = RTCD_IRQ_FLAG;				//Clear HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 0)
		delay_16us(5);									//Required idle time
		M6242_CTRLD_REG = RTCD_HOLD | RTCD_IRQ_FLAG;	//Set HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 1)
	}
    data[0] = (M6242_10_DAY_REG & 0x03);	  
    data[1] = (M6242_1_DAY_REG & 0x0F);
    data[2] = (M6242_10_MON_REG & 0x0F);
    data[3] = (M6242_1_MON_REG & 0x0F);
    data[4] = (M6242_10_YEAR_REG & 0x0F);
    data[5] = (M6242_1_YEAR_REG & 0x0F);
    M6242_CTRLD_REG = RTCD_IRQ_FLAG;					//Clear HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 0)            
  	 	    
}


void m6242_read_tm (void) {
	M6242_CTRLD_REG = RTCD_HOLD | RTCD_IRQ_FLAG;		//Set HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 1)
	while(M6242_CTRLD_REG & RTCD_BUSY) {
		M6242_CTRLD_REG = RTCD_IRQ_FLAG;				//Clear HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 0)
		delay_16us(5);									//Required idle time
		M6242_CTRLD_REG = RTCD_HOLD | RTCD_IRQ_FLAG;	//Set HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 1)
	}  	
    current_time.tm_sec = (10 * (M6242_10_SEC_REG & 0x0F)) + (M6242_1_SEC_REG & 0x0F);
    current_time.tm_min = (10 * (M6242_10_MIN_REG & 0x0F)) + (M6242_1_MIN_REG & 0x0F);
    current_time.tm_hour = (10 * (M6242_10_HOUR_REG & 0x03)) + (M6242_1_HOUR_REG & 0x0F);
    current_time.tm_mday = (10 * (M6242_10_DAY_REG & 0x0F)) + (M6242_1_DAY_REG & 0x0F);
    current_time.tm_mon = (10 * (M6242_10_MON_REG & 0x0F)) + (M6242_1_MON_REG & 0x0F) - 1;
    current_time.tm_year = (10 * (M6242_10_YEAR_REG & 0x0F)) + (M6242_1_YEAR_REG & 0x0F) + 100;
    current_time.tm_wday = (M6242_WEEK_REG & 0x0F);
    M6242_CTRLD_REG = RTCD_IRQ_FLAG;					//Clear HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 0)            
}

time_t* m6242_read_timestamp (void) {
    m6242_read_tm();
    timestamp = mktime(&current_time);
    return &timestamp;
}


void m6242_settime (uint8_t h, uint8_t m, uint8_t s) {
	M6242_CTRLD_REG = RTCD_HOLD | RTCD_IRQ_FLAG;		//Set HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 1)
	while(M6242_CTRLD_REG & RTCD_BUSY) {
		M6242_CTRLD_REG = RTCD_IRQ_FLAG;				//Clear HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 0)
		delay_16us(5);									//Required idle time
		M6242_CTRLD_REG = RTCD_HOLD | RTCD_IRQ_FLAG;	//Set HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 1)
	}
	// Write registers here
	M6242_1_SEC_REG = (s % 10);
	M6242_10_SEC_REG = (s / 10);
	M6242_1_MIN_REG = (m % 10);
	M6242_10_MIN_REG = (m / 10);
	M6242_1_HOUR_REG = (h % 10);
	M6242_10_HOUR_REG = (h / 10);	
	M6242_CTRLD_REG = RTCD_IRQ_FLAG;					//Clear HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 0)
}

void m6242_setdate (uint8_t d, uint8_t m, uint8_t y) {
	M6242_CTRLD_REG = RTCD_HOLD | RTCD_IRQ_FLAG;		//Set HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 1)
	while(M6242_CTRLD_REG & RTCD_BUSY) {
		M6242_CTRLD_REG = RTCD_IRQ_FLAG;				//Clear HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 0)
		delay_16us(5);									//Required idle time
		M6242_CTRLD_REG = RTCD_HOLD | RTCD_IRQ_FLAG;	//Set HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 1)
	}
	// Write registers here
	M6242_1_DAY_REG = (d % 10);
	M6242_10_DAY_REG = (d / 10);
	M6242_1_MON_REG = (m % 10);
	M6242_10_MON_REG = (m / 10);
	M6242_1_YEAR_REG = (y % 10);
	M6242_10_YEAR_REG = (y / 10);		
	M6242_CTRLD_REG = RTCD_IRQ_FLAG;					//Clear HOLD bit (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 0)
}
