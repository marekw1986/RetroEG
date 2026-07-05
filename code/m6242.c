#include "m6242.h"
#include "config.h"
#include "delay.h"
#include "io.h"
#include "ff.h"

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

#define TZ_OFFSET	        EEConfig.timezone
#define DAYLIGHT_SAVING     EEConfig.dst
//#define TZ_OFFSET	1

extern const uint8_t digits[16];			// USUN TO POTEM

//uint8_t dot = 0;

uint8_t m6242_buf[10];
struct tm current_time;
struct tm tz_time;
time_t timestamp;

struct tm* rtc_local_tm(time_t utc);

struct tm* __fastcall__ gmtime (const time_t* timep);
time_t __fastcall__ mktime (struct tm* timep);

static void epoch_to_tm_tz(time_t t, struct tm *tm);
#if defined(DAYLIGHT_SAVING)
static unsigned char is_dst_now(void);
#endif;

void __fastcall__ m6242_init (void) {                  
	M6242_CTRLD_REG = RTCD_IRQ_FLAG;								//0x04 (30 AJD = 0, IRQ FLAG = 1 (required), BUSY = 0(?), HOLD = 0)              
	M6242_CTRLE_REG = (RTCE_INTRT | RTCE_STDP_EN | RTCE_TM_1S);		//0x06 (Innterrupt mode, STD.P enabled, 1 s.)
	M6242_CTRLF_REG = RTCF_RESET;									//RESET needs to be 1 to set 24H mode.
	M6242_CTRLF_REG = (RTCF_TEST_DIS | RTCF_24H);					//0x04 (TEST = 0, 24h mode, STOP = 0, RESET = 0) 
}

/*
char* __fastcall__ m6242_read_time_str (void) {
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


char* __fastcall__ m6242_read_date_str (void) {
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
*/

void __fastcall__ m6242_read_time_raw (uint8_t* data) {	
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


void __fastcall__ m6242_read_date_raw (uint8_t* data) {
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


void __fastcall__ m6242_read_tm (void) {
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

time_t* __fastcall__ m6242_read_timestamp (void) {
    m6242_read_tm();
    timestamp = mktime(&current_time);
    m6242_update_tz_time();
    return &timestamp;
}


void __fastcall__ m6242_settime (uint8_t h, uint8_t m, uint8_t s) {
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

void __fastcall__ m6242_setdate (uint8_t d, uint8_t m, uint8_t y) {
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

char* __fastcall__ m6242_read_time_str (void) {  	
    m6242_buf[0] = (current_time.tm_hour / 10) + '0';
    m6242_buf[1] = (current_time.tm_hour % 10) + '0';
    m6242_buf[2] = ':';
    m6242_buf[3] = (current_time.tm_min / 10) + '0';
    m6242_buf[4] = (current_time.tm_min % 10) + '0';
    m6242_buf[5] = ':';
    m6242_buf[6] = (current_time.tm_sec / 10) + '0';
    m6242_buf[7] = (current_time.tm_sec % 10) + '0';
    m6242_buf[8] = '\0';

    return m6242_buf;
}


char* __fastcall__ m6242_read_date_str (void) {
    m6242_buf[0] = (current_time.tm_mday / 10) + '0';
    m6242_buf[1] = (current_time.tm_mday % 10) + '0';
    m6242_buf[2] = '-';
    m6242_buf[3] = ((current_time.tm_mon + 1) / 10) + '0';
    m6242_buf[4] = ((current_time.tm_mon + 1) % 10) + '0';
    m6242_buf[5] = '-';
    m6242_buf[6] = ((current_time.tm_year % 100) / 10) + '0';
    m6242_buf[7] = ((current_time.tm_year % 100) % 10) + '0';
    m6242_buf[8] = '\0';

    return m6242_buf;
}

char* __fastcall__ m6242_read_time_str_tz(void) {
    m6242_buf[0] = (tz_time.tm_hour / 10) + '0';
    m6242_buf[1] = (tz_time.tm_hour % 10) + '0';
    m6242_buf[2] = ':';
    m6242_buf[3] = (tz_time.tm_min / 10) + '0';
    m6242_buf[4] = (tz_time.tm_min % 10) + '0';
    m6242_buf[5] = ':';
    m6242_buf[6] = (tz_time.tm_sec / 10) + '0';
    m6242_buf[7] = (tz_time.tm_sec % 10) + '0';
    m6242_buf[8] = '\0';
    return m6242_buf;
}

char* __fastcall__ m6242_read_date_str_tz(void) {
    m6242_buf[0] = (tz_time.tm_mday / 10) + '0';
    m6242_buf[1] = (tz_time.tm_mday % 10) + '0';
    m6242_buf[2] = '-';
    m6242_buf[3] = ((tz_time.tm_mon + 1) / 10) + '0';
    m6242_buf[4] = ((tz_time.tm_mon + 1) % 10) + '0';
    m6242_buf[5] = '-';
    m6242_buf[6] = ((tz_time.tm_year % 100) / 10) + '0';
    m6242_buf[7] = ((tz_time.tm_year % 100) % 10) + '0';
    m6242_buf[8] = '\0';
    return m6242_buf;
}

DWORD get_fattime (void)
{
    return ((DWORD)(current_time.tm_year - 80) << 25)
         | ((DWORD)(current_time.tm_mon + 1) << 21)
         | ((DWORD)current_time.tm_mday << 16)
         | ((DWORD)current_time.tm_hour << 11)
         | ((DWORD)current_time.tm_min << 5)
         | ((DWORD)current_time.tm_sec >> 1);
}

void m6242_update_tz_time(void) {
    epoch_to_tm_tz(timestamp, &tz_time);
}

static unsigned char day_of_week(unsigned int year, unsigned char month, unsigned char day)
{
    /* Zeller's congruence (Gregorian). Returns 0=Sunday .. 6=Saturday. */
    unsigned int y = year;
    unsigned char m = month;   /* 1=Jan .. 12=Dec */
    unsigned int K, J;
    long h;

    if (m < 3) {
        m = (unsigned char)(m + 12);
        y -= 1;
    }
    K = y % 100;
    J = y / 100;
    h = (day + (13L * (m + 1)) / 5 + K + K / 4 + J / 4 + 5 * J) % 7;
    /* h: 0=Saturday, 1=Sunday, ... 6=Friday -> convert to 0=Sunday..6=Saturday */
    return (unsigned char)((h + 6) % 7);
}

#if defined(DAYLIGHT_SAVING)
static unsigned char is_dst_now(void)
{
    unsigned int year;
    unsigned char last_sun_mar;
    unsigned char last_sun_oct;

    if (!DAYLIGHT_SAVING) {
        return 0;
    }

    year = current_time.tm_year + 1900;

    if (current_time.tm_mon < 2 || current_time.tm_mon > 9) {
        return 0;   /* Jan, Feb, Nov, Dec: never DST */
    }
    if (current_time.tm_mon > 2 && current_time.tm_mon < 9) {
        return 1;   /* Apr..Sep: always DST */
    }

    if (current_time.tm_mon == 2) {   /* March */
        last_sun_mar = (unsigned char)(31 - day_of_week(year, 3, 31));
        if (current_time.tm_mday > last_sun_mar) return 1;
        if (current_time.tm_mday == last_sun_mar && current_time.tm_hour >= 1) return 1;
        return 0;
    }

    /* current_time.tm_mon == 9, October */
    last_sun_oct = (unsigned char)(31 - day_of_week(year, 10, 31));
    if (current_time.tm_mday < last_sun_oct) return 1;
    if (current_time.tm_mday == last_sun_oct && current_time.tm_hour < 1) return 1;
    return 0;
}
#endif

static void epoch_to_tm_tz(time_t t, struct tm *tm) {
    unsigned long days;
    unsigned long secs_of_day;
    unsigned int y;
    unsigned int ylen;
    unsigned char mo;
    unsigned char dim;
    unsigned char leap;
    static const unsigned char mdays[] = {31,28,31,30,31,30,31,31,30,31,30,31};

    t += (time_t)(TZ_OFFSET) * 3600L;
    
#if defined(DAYLIGHT_SAVING)
    if (is_dst_now()) {
        t += (time_t)3600L;
    }
#endif

    days = (unsigned long)t / 86400UL;
    secs_of_day = (unsigned long)t % 86400UL;

    tm->tm_hour = (unsigned char)(secs_of_day / 3600UL);
    tm->tm_min  = (unsigned char)((secs_of_day % 3600UL) / 60UL);
    tm->tm_sec  = (unsigned char)(secs_of_day % 60UL);

    /* day of week: Jan 1 1970 was a Thursday (tm_wday = 4) */
    tm->tm_wday = (unsigned char)((days + 4) % 7);

    y = 1970;
    for (;;) {
        leap = (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) ? 1 : 0;
        ylen = leap ? 366 : 365;
        if (days < ylen) break;
        days -= ylen;
        y++;
    }
    tm->tm_year = y - 1900;

    leap = (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) ? 1 : 0;
    for (mo = 0; mo < 12; mo++) {
        dim = mdays[mo] + ((mo == 1 && leap) ? 1 : 0);
        if (days < dim) break;
        days -= dim;
    }
    tm->tm_mon = mo;                       /* 0-11, matches struct tm convention */
    tm->tm_mday = (unsigned char)(days + 1);
}
