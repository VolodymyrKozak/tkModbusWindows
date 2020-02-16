/*
 * tk_oven_smi2.h
 *
 *  Created on: 23 ����. 2018 �.
 *      Author: KozakVF
 */

#ifndef TK_OVEN_SMI2_H_
#define TK_OVEN_SMI2_H_
#include <stdio.h>
#include "stdint.h"
#include "tw_mbAppl.h"
#include "../wConfig.h"

#define WINDOWS_MODBUS_MASTER_for_OVEN_INDICATOR

#ifdef   WINDOWS_MODBUS_MASTER_for_OVEN_INDICATOR



typedef struct {
	uint8_t  smbl[8];
}oven_smi2_string_t;






oven_session_status_t f_ovenSMI2_Init (uint8_t ovenSMI_ADDRESS);

/* �������� ������� */
oven_session_status_t f_ovenSMI2_DisplayBlinkingON (uint8_t ovenSMI_ADDRESS);
/* ��������� ������� */
oven_session_status_t f_ovenSMI2_DisplayBlinkingOFF (uint8_t ovenSMI_ADDRESS);
/* ��� ���������� - "������" */
oven_session_status_t f_ovenSMI2_StringSet(uint8_t ovenSMI_ADDRESS);

oven_session_status_t f_ovenSMI2_DisplayString4 (uint8_t ovenSMI_ADDRESS, uint8_t *str);
oven_session_status_t f_ovenSMI2_DisplayString8 (uint8_t ovenSMI_ADDRESS, uint8_t *str);
oven_session_status_t f_ovenSMI2_DisplayFloat1 (uint8_t ovenSMI_ADDRESS, float value);
/* ��������� �� ��������� ���-2
 * ����� � ��������� �����  � 9999.. 0.001 �� -0.01..-999
 * ���������� ��������� ��� ����� - �����f_ovenSMI2_StringSet(uint8_t ovenSMI_ADDRESS)*/
oven_session_status_t f_ovenSMI2_DisplayFloat (
		uint8_t ovenSMI_ADDRESS, /* ������ ���-2 ������ */
		float value              /* ����� � ��������� �����  � 9999.. 0.001 �� -0.01..-999*/
		);                       /* ���� ����� �� -0.01..0.001 -> 0000                   */
                                 /* >9999 or <-999 -> Err                                 */

/**********************************************************************************************************************
 * ����������������� ������ ����������� ����������
 * 1. ������-������ ���������� (�� 31, �� �����, ����������� ���2 ������ ���� ������,
 * �������� 0�20, 0�21, ... , 0�51. ����������� ���� ����� ������� ���2 � �� �����������.
 * 2. ������ ��������� ����� ������ ���� �������� � ��������� ����� vector_of_float_t */
oven_session_status_t f_ovenSMI2_DisplayBroadCast (
		uint8_t first_modbass_address,
		uint8_t number_of_devices,
		float *pvalue);

//
//oven_session_status_t f_ovenSMI2_TASKS_06 (
//		uint8_t ovenSMI_ADDRESS,
//		uint16_t register_to_PRESET,
//		uint16_t register_value
//		);
//oven_session_status_t f_ovenSMI2_TASKS_03 (
//		uint8_t ovenSMI_ADDRESS,
//		uint16_t register_to_READ,
//		uint16_t *ptr_received_values
//		);

oven_session_status_t f_ovenSMI2_TASKS_17 (
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
#endif /*  MODBUS_MASTER_for_OVEN_INDICATOR */
#endif /* TK_OVEN_SMI2_H_ 					*/
