/*
 * tk_oven_smi2.h
 *
 *  Created on: 23 трав. 2018 р.
 *      Author: KozakVF
 */

#ifndef TK_OVEN_320_H_
#define TK_OVEN_320_H_
#include <stdio.h>
#include "stdint.h"
#include "tw_mbAppl.h"
#include "../wConfig.h"

//#define WINDOWS_MODBUS_MASTER_for_OVEN_INDICATOR
//
//#ifdef   WINDOWS_MODBUS_MASTER_for_OVEN_INDICATOR

typedef struct {
	uint16_t oNoFF;            /* 0 - 1=¬кл/0=¬икл */
	uint16_t FaultCode;        /* 1 - 0=Ok/ 2*CODE-3 «наченн€ коду авар≥њ “ 2*/
	uint16_t Reserve2;         /* 2 - */
	uint16_t PumpingInOut;     /* 3 - 0- подача, 1 - откачка */
	uint16_t LiqudeLevel;      /* 4 - 0..0xFFFF  */
	uint16_t Reserve5;         /* 5 - */
	uint16_t Reserve6;         /* 6 - */
	uint16_t Reserve7;         /* 7 - */
	uint16_t Reserve8;         /* 8 - */
	uint16_t GoToScreen;       /* 9 -  ѕримусовий перех≥д на екран */
	uint16_t GetScreen;        /* 10 - ѕоточний екран  */
	uint16_t Time320;          /* 11 - „ас в≥д ор320, резерв */
	uint16_t Reserve12;        /* 12 - */
	uint16_t Reserve13;        /* 13 - */
	uint16_t Reserve14;        /* 14 - */
	uint16_t GetButton15;      /* 15 - 0x0001=на дес€тому екран≥ натиснута клав≥ша SET 240 coil
									   0x0100=на дес€тому екран≥ натиснута клав≥ша ESC 248 coil*/
	uint16_t SetMaxAmperage;   /* 16 - ”ставка струм макс */
	uint16_t SetMinAmperage;   /* 17 - ”ставка струм м≥н  */
	uint16_t Mode;             /* 18 - –ежим роботи 0/1/2 јвт/јвт—м«п/–учн*/
	uint16_t PumpingMode;      /* 19 - 0 подача, 1 откачка*/
	uint16_t Reserve20;        /* 20 - 0- подача, 1 - откачка */
	uint16_t Reserve21;        /* 21 - 0..0xFFFF  */
	uint16_t Reserve22;        /* 22 - */
	uint16_t Reserve23;        /* 23 - */
	uint16_t Reserve24;        /* 24 - */
}op320_registers_t;






//typedef struct {
//	uint8_t  symbol[8];
//}oven_smi2_string_t;





oven_session_status_t f_oven320_Tasks (void);
oven_session_status_t f_oven320_Init(void);
oven_session_status_t f_oven320_TASKS_17 (
		uint8_t ovenSMI_ADDRESS
		);




/*
Х 0x01 (Read Coils Status)
Х 0x05 (Write Single Coil)
Х 0x0F (Write Multiple Coils)
Х 0x03 (Read Holding Registers)
Х 0x06 (Write Single Register)
Х 0x10 (Write Multiple Registers)
*/
//#endif /*  MODBUS_MASTER_for_OVEN_INDICATOR */
#endif /* TK_OVEN_SMI2_H_ 					*/
