#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdint.h>

enum modbus_input_idx {
	MODBUS_INPUT_CPM=0,
	MODBUS_INPUT_SIVERT_INT,
	MODBUS_INPUT_SIVERT_FRACT,
    MODBUS_INPUT_DSTEMPH,
    MODBUS_INPUT_DSTEMPL,
    MODBUS_INPUT_DSTEMP_TIMEH,
    MODBUS_INPUT_DSTEMP_TIMEL,
	MODBUS_INPUT_TIMEH,
	MODBUS_INPUT_TIMEL,
	MODBUS_INPUT_UPTIMEH,
	MODBUS_INPUT_UPTIMEL,
    MODBUS_INPUT_LASTCFH,
    MODBUS_INPUT_LASTCFL,
	MODBUS_INPUT_CFRES
};
	
enum modbus_holding_idx {
	MODBUS_HOLDING_CMD=0,
	MODBUS_H1,
	MODBUS_H2,
	MODBUS_H3,
	MODBUS_H4,
	MODBUS_H5,
	MODBUS_H6
};

#define MODBUS_CMD_SET_TIME 		0x01
#define MODBUS_CMD_SET_DATE 		0x02
#define MODBUS_CMD_SET_DATETIME 	0x03
#define MODBUS_CMD_SET_TIMEZONE		0x04
#define MODBUS_CMD_SET_DST          0x05

void modbus_init(void) ;
void modbus_process_frame(void);
void modbus_set_cpm(void);
uint16_t modbus_get_cpm(void);
void modbus_set_sivert(void);
volatile uint16_t* modbus_get_sivert(void);
void modbus_set_ds18b20_temp(int32_t val);
volatile uint16_t* modbus_ds18b20_temp(void);
void modbus_set_time(void);
void modbus_set_uptime(void);
void modbus_set_cf_time();
void modbus_set_cf_result(uint8_t res);

#endif
