/*
 * tk_oven_smi2.h
 *
 *  Created on: 23 ����. 2018 �.
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
	uint16_t oNoFF;            /* 0 - 1=���/0=���� */
	uint16_t FaultCode;        /* 1 - 0=Ok/ 2*CODE-3 �������� ���� ���� ��2*/
	uint16_t Reserve2;         /* 2 - */
	uint16_t PumpingInOut;     /* 3 - 0- ������, 1 - ������� */
	uint16_t LiqudeLevel;      /* 4 - 0..0xFFFF  */
	uint16_t Reserve5;         /* 5 - */
	uint16_t Reserve6;         /* 6 - */
	uint16_t Reserve7;         /* 7 - */
	uint16_t Reserve8;         /* 8 - */
	uint16_t GoToScreen;       /* 9 -  ���������� ������� �� ����� */
	uint16_t GetScreen;        /* 10 - �������� �����  */
	uint16_t Time320;          /* 11 - ��� �� ��320, ������ */
	uint16_t Reserve12;        /* 12 - */
	uint16_t Reserve13;        /* 13 - */
	uint16_t Reserve14;        /* 14 - */
	uint16_t GetButton15;      /* 15 - 0x0001=�� �������� ����� ��������� ������ SET 240 coil
									   0x0100=�� �������� ����� ��������� ������ ESC 248 coil*/
	uint16_t SetMaxAmperage;   /* 16 - ������� ����� ���� */
	uint16_t SetMinAmperage;   /* 17 - ������� ����� ��  */
	uint16_t Mode;             /* 18 - ����� ������ 0/1/2 ���/�������/����*/
	uint16_t PumpingMode;      /* 19 - 0 ������, 1 �������*/
	uint16_t Reserve20;        /* 20 - 0- ������, 1 - ������� */
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
� 0x01 (Read Coils Status)
� 0x05 (Write Single Coil)
� 0x0F (Write Multiple Coils)
� 0x03 (Read Holding Registers)
� 0x06 (Write Single Register)
� 0x10 (Write Multiple Registers)
*/
//#endif /*  MODBUS_MASTER_for_OVEN_INDICATOR */
#endif /* TK_OVEN_SMI2_H_ 					*/
