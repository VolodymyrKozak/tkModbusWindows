/*
 * tw_mbAppl.h
 *
 *  Created on: 10 ����. 2019 �.
 *      Author: KozakVF
 */

#ifndef SRC_MB_DEVS_TW_MBAPPL_H_
#define SRC_MB_DEVS_TW_MBAPPL_H_

#include "../mb_funcs/tw_mb.h"
#include "../wConfig.h"
typedef enum {
	oven_UnknownERR,
	oven_OK,
	oven_Modbus_ERR,
	oven_answerTimeout,
	oven_WrongDevice,
	oven_WrongeFunction,
	oven_Wronge_Nbytes,
	oven_WrongeRegime,
	oven_WrongeTask,
	oven_NULL_POINTER,
	oven_AnswerLOST,

	oven_NO_REQUEST,
	oven_ResponseSent
} oven_session_status_t;

/****************************************************************************************************************************
* PRESET_MULTIPLE_REGISTERS
* ***************************************************************************************************************************
*/
oven_session_status_t f_ovenSMI2_TASKS_16 (
		uint8_t ovenSMI_ADDRESS,   /* ������-������ �������                                       */
		uint16_t register_to_BEGIN,/* ������, � ����� �������� ������ 0x00..0xFF
		                             �������� - 0�0000 .. 0�FFFF                                  */
		uint16_t N_of_registers,   /* ʳ������ �������, �� ����� ������ 0x0001 to 0x007B(123) */
		uint8_t  N2_of_bytes,      /* ʳ������ ����, �� ����� ������   2*  0x01  to 0x7B(246)    */
		uint16_t *i16words);       /* ����� 16-������� ������       */

oven_session_status_t f_ovenSMI2_TASKS_06 (
		uint8_t ovenSMI_ADDRESS,
		uint16_t register_to_PRESET,
		uint16_t register_value
		);
/*Read Holding Registers - ONE register */
oven_session_status_t f_ovenSMI2_TASKS_03_1 (
		uint8_t ovenSMI_ADDRESS,
		uint16_t register_to_READ,
		uint16_t *ptr_received_values
		);
/****************************************************************************************************************************
* read_SINGLE_REGISTER
* ������� ����������� ��� ������� ,�������� �������
* ***************************************************************************************************************************
*/
oven_session_status_t f_ovenSMI2_TASKS_03 (
		uint8_t ovenSMI_ADDRESS,      /* ������-������ �������    */
		uint16_t register_to_READ,    /* ������ ����������� ������� ������   */
		uint16_t number_of_registers,
		uint16_t *ptr_received_values /* ������ � ������� ����� */
		);
#endif /* SRC_MB_DEVS_TW_MBAPPL_H_ */
