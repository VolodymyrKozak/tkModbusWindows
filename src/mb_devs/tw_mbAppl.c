/*
 * tw_mbAppl.c
 *
 *  Created on: 10 жовт. 2019 р.
 *      Author: KozakVF
 */

#include "tw_mbAppl.h"
#include "../mb_funcs/tkCRC.h"
#include "string.h"
/****************************************************************************************************************************
* PRESET_MULTIPLE_REGISTERS
* ***************************************************************************************************************************
*/
oven_session_status_t f_ovenSMI2_TASKS_16 (
		uint8_t ovenSMI_ADDRESS,   /* Модбас-адреса прилада                                       */
		uint16_t register_to_BEGIN,/* Регістр, з якого починати писати 0x00..0xFF
		                             стандарт - 0х0000 .. 0хFFFF                                  */
		uint16_t N_of_registers,   /* Кількість регістрів, які треба писати 0x0001 to 0x007B(123) */
		uint8_t N2_of_bytes,       /* Кількість байт, як треба писати   2*  0x01  to 0x7B(246)    */
		uint16_t *i16words){       /* Масив   16-значних числел       */
	if((N_of_registers>0x007B)||((((uint16_t)N_of_registers)*2) != N2_of_bytes)){
		return oven_Wronge_Nbytes;
	}
	oven_session_status_t session_status16 = oven_UnknownERR;
	modbus_master_tx_msg_t w16 = {0};
//	sprv_t sprv16 = {0};
	uint16_t crc_calc_tx16 = 0;
	w16.msg [0]= ovenSMI_ADDRESS;
	w16.msg [1]= 0x10; /* function */
	/* register address to begin writing */
	w16.msg [2]= register_to_BEGIN / 0x100;
	w16.msg [3]= register_to_BEGIN % 0x100;
	/* number of 16bit words, fist byte of uint16_t */
	w16.msg [4]= N_of_registers / 0x100;
	w16.msg [5]= N_of_registers % 0x100;
	/* number of bytes*/
	w16.msg [6]= N2_of_bytes;
	/* перевантажуємо байти */
	uint8_t *bytes=NULL;
//	bytes = (uint8_t*)i16words;
//	memcpy(w16.msg+7,bytes,N2_of_bytes);/* помилка? */
	for(uint8_t i=0;i<N_of_registers;i++){
		uint16_t i16=(*(i16words+i));
		w16.msg[7+2*i]=i16 / 0x100;
		w16.msg[7+2*i+1]=i16 % 0x100;
	}
	w16.length=N2_of_bytes+7+2;

	crc_calc_tx16 = CRC_16x (w16.msg, 			    			/* 2. Обчислюємо 16-бітний CRC   									*/
		        w16.length -2);
	w16.msg[w16.length-1]	= crc_calc_tx16 / 0x100;			/* 3. Старший байт CRC засилаємо в останній байт повідомлення 		*/
	w16.msg[w16.length-2]	= crc_calc_tx16 % 0x100;			/* 4. Молодший байт СRC засилаэмо в передостанный байт повыдомлення */

																/* 5. Задаємо час очікування відповіді в мілісекундах згідно з параметрами прикладого алгоритму */
	modbus_master_rx_msg_t w16responce = {0};
	uint8_t w16responcelength=8;

	while (session_status16 == oven_UnknownERR) {
		modbus_status_t mbb=MODBUS_UNKNOWN_ERROR;
		mbb=tw_txrxModbus_Session (cport_nr, &w16, &w16responce, 50+w16.length*6);
		if (mbb != MODBUS_SUCCESS){
			if(mbb==MODBUS_ERR_TIMEOUT){session_status16=oven_answerTimeout;break;}
			else                       {session_status16=oven_Modbus_ERR;   break;}
		}
		uint16_t crc_calc_rx=0;
		crc_calc_rx = CRC_16x (w16responce.msg, 			    		/* 2. Обчислюємо 16-бітний CRC   									*/
				w16responce.length -2);

		if (
			 (w16responce.msg[w16responce.length-1] != crc_calc_rx / 0x100) ||/* якщо CRC у повідомленні правильний*/
			 (w16responce.msg[w16responce.length-2] != crc_calc_rx % 0x100)
		   ){
			session_status16=oven_AnswerLOST;break;
		}
		if (w16responce.msg[0] != w16.msg[0]){
			session_status16=oven_WrongDevice;break; 		/* address ok*/
		}
		if (w16responce.msg[1] != w16.msg[1]){
			session_status16=oven_WrongeFunction;break;		/* function ok*/
		}
		if(w16responcelength != w16responce.length){
			session_status16=oven_AnswerLOST;break;		/* function ok*/
		}
		session_status16=oven_OK;
	}
	return session_status16;
}





/****************************************************************************************************************************
* PRESET_SINGLE_REGISTER *
* відлагоджено і чудово працює
* ***************************************************************************************************************************
*/
oven_session_status_t f_ovenSMI2_TASKS_06 (
		uint8_t ovenSMI_ADDRESS,
		uint16_t register_to_PRESET,
		uint16_t register_value
		){
	oven_session_status_t session_status06 = oven_UnknownERR;
	modbus_master_tx_msg_t w06 = {0};
//	sprv_t sprv06 = {0};
	uint16_t crc_calc_tx06 = 0;
	w06.length = 8;
	w06.msg[0] = ovenSMI_ADDRESS;
	w06.msg[1] = 0x06;  /* function */
	w06.msg[2] = register_to_PRESET / 0x100;
	w06.msg[3] = register_to_PRESET % 0x100;

	w06.msg[4] = register_value / 0x100; /* quantity of registers */
	w06.msg[5] = register_value % 0x100; /* quantity of registers */

	crc_calc_tx06 = CRC_16x (w06.msg, 			    		    /* 2. Обчислюємо 16-бітний CRC   									*/
		        w06.length -2);
	w06.msg[w06.length-1]	= crc_calc_tx06 / 0x100;			/* 3. Старший байт CRC засилаємо в останній байт повідомлення 		*/
	w06.msg[w06.length-2]	= crc_calc_tx06 % 0x100;			/* 4. Молодший байт СRC засилаэмо в передостанный байт повыдомлення */

																/* 5. Задаємо час очікування відповіді в мілісекундах згідно з параметрами прикладого алгоритму */
	modbus_master_rx_msg_t w06responce = {0};
	w06responce.length=8;

	while (session_status06 != oven_OK) {
		if (tw_txrxModbus_Session (cport_nr, &w06, &w06responce, 400) != MODBUS_SUCCESS){
			session_status06=oven_Modbus_ERR;break;
		}
		uint16_t crc_calc_rx=0;
		crc_calc_rx = CRC_16x (w06responce.msg, 			    		/* 2. Обчислюємо 16-бітний CRC   									*/
				w06responce.length -2);

		if (
			 (w06responce.msg[w06responce.length-1] != crc_calc_rx / 0x100) ||/* якщо CRC у повідомленні правильний*/
			 (w06responce.msg[w06responce.length-2] != crc_calc_rx % 0x100)
		   ){
			session_status06=oven_AnswerLOST;break;
		}
		if (w06responce.msg[0] != w06.msg[0]){
			session_status06=oven_WrongDevice;break; 		/* address ok*/
		}
		if (w06responce.msg[1] != w06.msg[1]){
			session_status06=oven_WrongeFunction;break;		/* function ok*/
		}

		session_status06=oven_OK;
	}
	return session_status06;
}
/****************************************************************************************************************************
* read_SINGLE_REGISTER
* Функція реалізованна для читання одного регістра
* ***************************************************************************************************************************
*/
oven_session_status_t f_ovenSMI2_TASKS_03_1 (
		uint8_t ovenSMI_ADDRESS,      /* Модбас-адреса прилада    */
		uint16_t register_to_READ,    /* Адреса регістра Модбас   */
		uint16_t *ptr_received_values /* зчитате з регістра число */
		){
	oven_session_status_t session_status03 = oven_UnknownERR;
	modbus_master_tx_msg_t w03 = {0};
//	sprv_t sprv03 = {0};
	uint16_t crc_calc_tx03 = 0;
	w03.length = 8;
	w03.msg[0] = ovenSMI_ADDRESS;
	w03.msg[1] = 0x03;  /* function */
	w03.msg[2] = register_to_READ / 0x100;
	w03.msg[3] = register_to_READ % 0x100;

	w03.msg[4] = 0; /* quantity of registers */
	w03.msg[5] = 1; /* quantity of registers */

	crc_calc_tx03 = CRC_16x (w03.msg, 			    			/* 2. Обчислюємо 16-бітний CRC   									*/
		        w03.length -2);
	w03.msg[w03.length-1]	= crc_calc_tx03 / 0x100;			/* 3. Старший байт CRC засилаємо в останній байт повідомлення 		*/
	w03.msg[w03.length-2]	= crc_calc_tx03 % 0x100;			/* 4. Молодший байт СRC засилаэмо в передостанный байт повыдомлення */

																/* 5. Задаємо час очікування відповіді в мілісекундах згідно з параметрами прикладого алгоритму */
	modbus_master_rx_msg_t w03responce = {0};
	w03responce.length=7;

	while (session_status03 != oven_OK) {
		if (tw_txrxModbus_Session (cport_nr, &w03, &w03responce, 400) != MODBUS_SUCCESS){
			session_status03=oven_Modbus_ERR;break;
		}
		uint16_t crc_calc_rx=0;
		crc_calc_rx = CRC_16x (w03responce.msg, 			    		/* 2. Обчислюємо 16-бітний CRC   									*/
				w03responce.length -2);

		if (
			 (w03responce.msg[w03responce.length-1] != crc_calc_rx / 0x100) ||/* якщо CRC у повідомленні правильний*/
			 (w03responce.msg[w03responce.length-2] != crc_calc_rx % 0x100)
		   ){
			session_status03=oven_AnswerLOST;break;
		}
		if (w03responce.msg[0] != w03.msg[0]){
			session_status03=oven_WrongDevice;break; 		/* address ok*/
		}
		if (w03responce.msg[1] != w03.msg[1]){
			session_status03=oven_WrongeFunction;break;		/* function ok*/
		}
		if (w03responce.msg[2] != w03.msg[5]*2){
			session_status03=oven_Wronge_Nbytes;break; 		/* address ok*/
		}

		uint16_t read_values=    w03responce.msg[3]*0x100 + w03responce.msg[4];
		(*ptr_received_values) = read_values;
		session_status03=oven_OK;
	}
	return session_status03;
}

/****************************************************************************************************************************
* read_SINGLE_REGISTER
* Функція реалізованна для читання ,багатьох регістрів
* ***************************************************************************************************************************
*/
oven_session_status_t f_ovenSMI2_TASKS_03 (
		uint8_t ovenSMI_ADDRESS,      /* Модбас-адреса прилада    */
		uint16_t register_to_READ,    /* Адреса початкового регістра Модбас   */
		uint16_t number_of_registers,
		uint16_t *ptr_received_values /* зчитані з регістрів число */
		){
	oven_session_status_t session_status03 = oven_UnknownERR;
	modbus_master_tx_msg_t w03 = {0};
//	sprv_t sprv03 = {0};
	uint16_t crc_calc_tx03 = 0;
	w03.length = 8;
	w03.msg[0] = ovenSMI_ADDRESS;
	w03.msg[1] = 0x03;  /* function */
	w03.msg[2] = register_to_READ / 0x100;
	w03.msg[3] = register_to_READ % 0x100;

	w03.msg[4] = number_of_registers / 0x100;/* quantity of registers */
	w03.msg[5] = number_of_registers % 0x100;/* quantity of registers */


	crc_calc_tx03 = CRC_16x (w03.msg, 			    			/* 2. Обчислюємо 16-бітний CRC   									*/
		        w03.length -2);
	w03.msg[w03.length-1]	= crc_calc_tx03 / 0x100;			/* 3. Старший байт CRC засилаємо в останній байт повідомлення 		*/
	w03.msg[w03.length-2]	= crc_calc_tx03 % 0x100;			/* 4. Молодший байт СRC засилаэмо в передостанный байт повыдомлення */

																/* 5. Задаємо час очікування відповіді в мілісекундах згідно з параметрами прикладого алгоритму */
	modbus_master_rx_msg_t w03responce = {0};
	w03responce.length=7;

	while (session_status03 != oven_OK) {
		if (tw_txrxModbus_Session (cport_nr, &w03, &w03responce, 400) != MODBUS_SUCCESS){
			session_status03=oven_Modbus_ERR;break;
		}
		uint16_t crc_calc_rx=0;
		crc_calc_rx = CRC_16x (w03responce.msg, 			    		/* 2. Обчислюємо 16-бітний CRC   									*/
				w03responce.length -2);

		if (
			 (w03responce.msg[w03responce.length-1] != crc_calc_rx / 0x100) ||/* якщо CRC у повідомленні правильний*/
			 (w03responce.msg[w03responce.length-2] != crc_calc_rx % 0x100)
		   ){
			session_status03=oven_AnswerLOST;break;
		}
		if (w03responce.msg[0] != w03.msg[0]){
			session_status03=oven_WrongDevice;break; 		/* address ok*/
		}
		if (w03responce.msg[1] != w03.msg[1]){
			session_status03=oven_WrongeFunction;break;		/* function ok*/
		}
		if (w03responce.msg[2] != w03.msg[5]*2){
			session_status03=oven_Wronge_Nbytes;break; 		/* address ok*/
		}
		uint16_t *ptr16=ptr_received_values;
		for (uint8_t i=0;i<number_of_registers;i++){
		uint16_t read_values=    w03responce.msg[3+2*i]*0x100 + w03responce.msg[3+2*i+1];

		(*ptr16) = read_values;
		ptr16++;

		}
		session_status03=oven_OK;
	}
	return session_status03;
}


/*Coils 0..32 */











