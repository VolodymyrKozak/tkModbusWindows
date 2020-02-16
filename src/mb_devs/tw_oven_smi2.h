/*
 * tk_oven_smi2.h
 *
 *  Created on: 23 трав. 2018 р.
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

/* ¬ Ћё„»“№ ћ»√јЌ»≈ */
oven_session_status_t f_ovenSMI2_DisplayBlinkingON (uint8_t ovenSMI_ADDRESS);
/* ¬џ Ћё„»“№ ћ»√јЌ»≈ */
oven_session_status_t f_ovenSMI2_DisplayBlinkingOFF (uint8_t ovenSMI_ADDRESS);
/* тип переменной - "строка" */
oven_session_status_t f_ovenSMI2_StringSet(uint8_t ovenSMI_ADDRESS);

oven_session_status_t f_ovenSMI2_DisplayString4 (uint8_t ovenSMI_ADDRESS, uint8_t *str);
oven_session_status_t f_ovenSMI2_DisplayString8 (uint8_t ovenSMI_ADDRESS, uint8_t *str);
oven_session_status_t f_ovenSMI2_DisplayFloat1 (uint8_t ovenSMI_ADDRESS, float value);
/* ¬иведенн€ на ≥ндикатор —ћ»-2
 * числа з плаваючою комою  в 9999.. 0.001 та -0.01..-999
 * ѕопередньо становити тип зм≥нноњ - р€докf_ovenSMI2_StringSet(uint8_t ovenSMI_ADDRESS)*/
oven_session_status_t f_ovenSMI2_DisplayFloat (
		uint8_t ovenSMI_ADDRESS, /* јдреса —ћ»-2 ћодбас */
		float value              /* „исло з плаваючою комою  в 9999.. 0.001 та -0.01..-999*/
		);                       /* якщо число м≥ж -0.01..0.001 -> 0000                   */
                                 /* >9999 or <-999 -> Err                                 */

/**********************************************************************************************************************
 * Ў»–ќ ќ¬≈ўј“≈Ћ№Ќјя «јѕ»—№ ќѕ≈–ј“»¬Ќџ’ ѕј–јћ≈“–ќ¬
 * 1. ћодбас-адреса нескольких (до 31, по моему, индикаторов —ћ»2 должны быть подр€д,
 * например 0х20, 0х21, ... , 0х51. ”казываетс€ лишь адрес первого —ћ»2 и их количчество.
 * 2. ћассив плавающих чисел должен быть объ€влен и определен через vector_of_float_t */
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
Х 0x01 (Read Coils Status)
Х 0x05 (Write Single Coil)
Х 0x0F (Write Multiple Coils)
Х 0x03 (Read Holding Registers)
Х 0x06 (Write Single Register)
Х 0x10 (Write Multiple Registers)
*/
#endif /*  MODBUS_MASTER_for_OVEN_INDICATOR */
#endif /* TK_OVEN_SMI2_H_ 					*/
