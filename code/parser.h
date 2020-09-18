#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdint.h>

// definicja typu strukturalnego
typedef struct {
	char komenda[12];
	uint8_t (* cmd_service)(char * params);
} TCMD;

void parse_cmd(char * pBuf);

#endif
