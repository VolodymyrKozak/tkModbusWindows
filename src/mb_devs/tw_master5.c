/*
 * mb_master5.c
 *
 *  Created on: 30 жовт. 2017 р.
 *      Author: Volodymyr.Kozak
 */

#include "tw_master5.h"

#include "string.h"

#include "../mb_funcs/tkCRC.h"
#include "../mb_funcs/tw_mb.h"
master_state_5_t master_state_5=Idle;
extern int cport_nr;

/*********************************************************************************************************************************************
 *    P R I V A T E      F U N C T I O N S  DECLARATION
**********************************************************************************************************************************************/
static modbus_status_t MB_Master_Supervisor (sprv_t *obj);
/*****************************************************************************************************************************************
 *****************************************************************************************************************************************
 *    P U B L I C S     F U N C T I O N S
 *****************************************************************************************************************************************
 ******************************************************************************************************************************************/



/*******************************************************************************************************************************************/
/* 0х03 	R E A D    H O L D I N G    R E G I S T E R S  																				   */
/*******************************************************************************************************************************************/

/* Ця функція викликається із вашого клієнтського (Master) застосування,щоб зчитати з пам'яті сервера задану кількіть 16-бітних регістрів
 * починаючи з заданої адреси заданий регістр сервера (Slave) необхідне вам значення
 * */
modbus_status_t MB_Read_Holding_Registers (	uint8_t slave_address,
											uint16_t starting_address,
											uint16_t quantity_of_registers,
											uint16_t *ptr_received_values,
											modbus_exception_t *ptr_WSR_exception) {
/* де:
 * slave_adress - байт адреси сервера;
 * starting_address         - 16-бітна адреса пам'яті сервера, починаючи з якої ви зчитуєте регістри; 0х0000..0хFFFF, обмеження згідно з документацією сервера
 * quantity_of_registers    - 16-бітне значення кількості регістів, які ви збираєтесь зчитувати; 0х0000..007D (125),  обмеження згідно з документацією сервера
 * ptr_received_values		is a pointer to an array of unsigned 16 bit values that will receive the value of all the registers you are reading.
				The size of the array needs to be at least nbr_regs. Note that you can ‘cast’ the unsigned values to signed values.
				As far as the Modbus protocol is concerned, it sends and receives 16 bit values and the interpretation of what these values mean
				is application specific.
 * *ptr_WSR_exception - показчик на адресу коду помилки, куди може бути записано код помилки в разі неуспішного виконання команди сервером.
 * В разі успішного завершення роботи фуункції - повертається MODBUS_SUCCESS, в разі помилки - повертається код помилки, див.модуль mb.h
 *
 * */
	modbus_status_t mb_f_status = MODBUS_ERR_UNKNOWN;
	modbus_master_tx_msg_t w03 = {0};
	modbus_master_rx_msg_t rx03={0};
	sprv_t sprv03 = {0};
	uint16_t crc_calc_tx03 = 0;										/* 1. Формуємо байти повідомлення: */
	w03.msg[0] = slave_address;
	w03.msg[1] = READ_HOLDING_REGISTERS;
	w03.msg[2] = starting_address / 0x100;
	w03.msg[3] = starting_address % 0x100;
	w03.msg[4] = 0x00;
	w03.msg[5] = quantity_of_registers % 0x100;
	w03.length = 8;
	crc_calc_tx03 = CRC_16x (w03.msg, 			    					/* 2. Обчислюємо 16-бітний CRC   									*/
				        w03.length -2*(sizeof(uint8_t)));
	w03.msg[w03.length-1]	= crc_calc_tx03 / 0x100;					/* 3. Старший байт CRC засилаємо в останній байт повідомлення 		*/
	w03.msg[w03.length-2]	= crc_calc_tx03 % 0x100;					/* 4. Молодший байт СRC засилаэмо в передостанный байт повыдомлення */

	sprv03.timeout = 100; 												/* 5. Задаємо час очікування відповіді в мілісекундах згідно з параметрами прикладого алгоритму */
	sprv03.waited_normal_response_length=5+quantity_of_registers*2;  	/* 6. Задаємо довжину очікуваної успішної відповіді */
	sprv03.waited_error_response_length=5;								/* 7. Задаємо довжину очікуваної відповіді про помилку, віявлену сервером */
	sprv03.waited_slave_response_address=slave_address;
	sprv03.waited_slave_response_function=READ_HOLDING_REGISTERS;
//	sprv03.transmitting_began_at = HAL_GetTick();						/* Засікаємо момент відправки повідомлення   */

	sprv03.ptr_tx = &w03;
	sprv03.ptr_rx = &rx03;
    //	MB_Master_Transmit ();											/* 10. Відправляємо повідомлення  */
    master_state_5 = m_msg_transmitting;

    modbus_status_t TxIt = tw_txrxModbus_Session (
    		cport_nr,
    		&w03,
    		&rx03,
			sprv03.timeout
    		);
//	HAL_StatusTypeDef TxIt =HAL_UART_Transmit_IT(&huart4, &w03.msg, w03.length/* RX_MAX_SLAVE_MSG_LENGTH*/);
	if (TxIt != MODBUS_SUCCESS) {
		return TxIt;
	}
	else {
		mb_f_status = MB_Master_Supervisor (&sprv03);					/* 11 Викликаємо функцію прийому і обробки відповіді 				*/
	}
	if ((mb_f_status==MODBUS_SUCCESS)&&((sprv03.exception)==MODBUS_F_SUCCESS)){	/* 12 Якщо функція виконалась без помилок, і якщо від сервера ми 	*/


		for (uint8_t i03j=0;i03j<quantity_of_registers;++i03j){			/*    отримали вміст регістрів, а не код помилки                	*/
			(*(ptr_received_values+i03j*sizeof(uint16_t)))=				/*    записуємо вміст регістрів з повідомлення від сервера 			*/
					rx03.msg[i03j+3]*0x100 +							/*    в пам'ять за адресою, заданою прикладною програмою			*/
					rx03.msg[i03j+4];
		}
		(*ptr_WSR_exception)=MODBUS_F_SUCCESS;
	}
	else {
		(*ptr_WSR_exception)=mb_f_status;
	}
//	LCD_Printf ("MB_Read_Holding_Registers SENT:\n");
//
//	for (uint8_t i08i=0;i08i<w03.length;++i08i){
//		LCD_Printf ("0x%X%X, ",w03.msg[i08i]/0x10,w03.msg[i08i]%0x10);
//	}
//	LCD_Printf ("\n");
//
//	LCD_Printf ("Responce from Slave:\n");
//	for (uint8_t i08r=0;i08r<rx_msg.length;++i08r){
//		LCD_Printf ("0x%X%X, ",modbus_master_rx.msg[i08r]/0x10, modbus_master_rx.msg[i08r]%0x10);
//	}
//LCD_Printf ("\n\n");

	master_state_5=Idle;
	return (mb_f_status);/* 12 Якщо mb_f_status = MODBUS_SUCCESS то:
	 	 	 	 	 	 якщо (*ptr_WSR_exception) = 0x00 то запис у заданий регістр серевера успішно виконано
	 	 	 	 	 	 інакше = див. значення modbus_exception_t , модуль mb.h
	 	 	 	 	 	       Якщо mb_f_status != MODBUS_SUCCESS,див. значення modbus_status_t, модуль mb.h */
}

/**************************************************************************************************************************************
* 0х05  W R I T E _  S I N G L E _  C O I L
*************************************************************************************************************************************/



/* This function code is used to write a single output to either ON or OFF in a remote device */

modbus_status_t MB_Write_Syngle_Coil (	    uint8_t  slave_address,
											uint16_t output_address,
											uint16_t output_value,
											modbus_exception_t *ptr_WSR_exception) {
/* де:
 * slave_adress - байт адреси сервера;
 * reg_address  - 16-бітна адреса пам'яті сервера, куди ви записуєте необхідний вам параметр;
 * reg_value    - 16-бітне значення параметра у двійковому форматі, яке ви записуєте у сервер;
 * *ptr_WSR_exception - показчик на адресу коду помилки, куди може бути записано код помилки в разі неуспішного виконання команди сервером.
 * В разі успішного завершення роботи фуункції - повертається MODBUS_SUCCESS, в разі помилки - повертається код помилки, див.модуль mb.h
 *
 * */
	modbus_status_t mb_f_status = MODBUS_ERR_UNKNOWN;
	modbus_master_tx_msg_t w05 = {0};
	modbus_master_rx_msg_t rx05={0};
	sprv_t sprv05 = {0};
	uint16_t crc_calc_tx05 = 0;
	(*ptr_WSR_exception) = 0;											/* 1. Формуємо байти повідомлення: 									*/
	w05.msg[0] = slave_address;											/* 0x00 якщо циркулярне повідомлення, 0х01..0хFF*/
	w05.msg[1] = WRITE_SINGLE_COIL;
	w05.msg[2] = output_address / 0x100;                                /* 0x0000..0xFFFF													*/
	w05.msg[3] = output_address % 0x100;
	w05.msg[4] = output_value   / 0x100;								/* 0x0000 - Off, 0xFF00 - On									    */
	w05.msg[5] = output_value   % 0x100;
	w05.length = 8;
	crc_calc_tx05 = CRC_16x (w05.msg, 			    					/* 2. Обчислюємо 16-бітний CRC   									*/
				        w05.length -2*(sizeof(uint8_t)));
	w05.msg[w05.length-1]	= crc_calc_tx05 / 0x100;					/* 3. Старший байт CRC засилаємо в останній байт повідомлення 		*/
	w05.msg[w05.length-2]	= crc_calc_tx05 % 0x100;					/* 4. Молодший байт СRC засилаэмо в передостанный байт повыдомлення */

	if (slave_address==0x00){
		sprv05.timeout = 50; 											/* 5. Задаємо час очікування відповіді в мілісекундах згідно з параметрами прикладого алгоритму */
	}
	else {
		sprv05.timeout = 100;
	}
	sprv05.waited_normal_response_length=8;  							/* 6. Задаємо довжину очікуваної успішної відповіді */
	sprv05.waited_error_response_length=5;								/* 7. Задаємо довжину очікуваної відповіді про помилку, віявлену сервером */
	sprv05.waited_slave_response_address=slave_address;
	sprv05.waited_slave_response_function=WRITE_SINGLE_COIL;
//	sprv05.transmitting_began_at = HAL_GetTick();						/* Засікаємо момент відправки повідомлення   */
    sprv05.exception=(*ptr_WSR_exception);								/* задаємо посилання для отримання коду помилки від від сервера */
	sprv05.ptr_tx = &w05;
	sprv05.ptr_rx = &rx05;
    //	MB_Master_Transmit ();											/* 10. Відправляємо повідомлення  */
    master_state_5 = m_msg_transmitting;

    modbus_status_t TxIt = tw_txrxModbus_Session(
    		cport_nr,
    		&w05,
    		&rx05,
			sprv05.timeout
    		);

//	HAL_StatusTypeDef TxIt =HAL_UART_Transmit_IT(&huart4, &w05.msg, w05.length/* RX_MAX_SLAVE_MSG_LENGTH*/);
	if (TxIt != MODBUS_SUCCESS) {
		return TxIt;
	}
	else {
		/* 11 Викликаємо функцію прийому і обробки відповіді */
		mb_f_status = MB_Master_Supervisor(&sprv05);					/* 11 Викликаємо функцію прийому і обробки відповіді */
	}
//LCD_Printf ("MB_Write_Syngle_Coil SENT:\n");
//	for (uint8_t i08i=0;i08i<w05.length;++i08i){
//		   		LCD_Printf ("0x%X%X ",w05.msg[i08i]/0x10,w05.msg[i08i]%0x10);
//	}
//LCD_Printf ("\n");
//LCD_Printf ("Responce from Slave:\n");
//	for (uint8_t i08r=0;i08r<modbus_master_rx.length;++i08r){
//		   		LCD_Printf ("0x%X%X ",modbus_master_rx.msg[i08r]/0x10, modbus_master_rx.msg[i08r]%0x10);
//	}
//LCD_Printf ("\n\n");


	master_state_5=Idle;
	return (mb_f_status);/* 12 Якщо mb_f_status = MODBUS_SUCCESS то:
	 	 	 	 	 	 якщо (*ptr_WSR_exception) = 0x00 то запис у заданий регістр серевера успішно виконано
	 	 	 	 	 	 інакше = див. значення modbus_exception_t , модуль mb.h
	 	 	 	 	 	       Якщо mb_f_status != MODBUS_SUCCESS,див. значення modbus_status_t, модуль mb.h */




}
/*******************************************************************************************************************************************/
/* 0х06     W R I T E    S I N G L E    R E G I S T E R 																				   */
/*******************************************************************************************************************************************/
/* Ця функція викликається із вашого клєнтського (Master) застосування,щоб записати у заданий регістр сервера (Slave) необхідне вам значення
 * */
modbus_status_t MB_Write_Single_Register (	uint8_t slave_address,
											uint16_t reg_address,
											uint16_t reg_value,
											modbus_exception_t *ptr_WSR_exception) {
/* де:
 * slave_adress - байт адреси сервера;
 * reg_address  - 16-бітна адреса пам'яті сервера, куди ви записуєте необхідний вам параметр;
 * reg_value    - 16-бітне значення параметра у двійковому форматі, яке ви записуєте у сервер;
 * *ptr_WSR_exception - показчик на адресу коду помилки, куди може бути записано код помилки в разі неуспішного виконання команди сервером.
 * В разі успішного завершення роботи фуункції - повертається MODBUS_SUCCESS, в разі помилки - повертається код помилки, див.модуль mb.h
 *
 * */
	modbus_status_t mb_f_status = MODBUS_ERR_UNKNOWN;
	modbus_master_tx_msg_t w06 = {0};
	modbus_master_rx_msg_t rx06= {0};
	sprv_t sprv06 = {0};
	uint16_t crc_calc_tx06 = 0;
	(*ptr_WSR_exception) = 0;											/* 1. Формуємо байти повідомлення: 									*/
	w06.msg[0] = slave_address;											/* 0x00 - циркулярне повідомлення, 0х01..0хFF					    */
	w06.msg[1] = WRITE_SINGLE_REGISTER;
	w06.msg[2] = reg_address / 0x100;									/* 0x0000 ... 0xFFFF												*/
	w06.msg[3] = reg_address % 0x100;
	w06.msg[4] = reg_value   / 0x100;									/* 0x0000 ... 0xFFFF 											    */
	w06.msg[5] = reg_value   % 0x100;
	w06.length = 8;
	crc_calc_tx06 = CRC_16x (w06.msg, 			    				/* 2. Обчислюємо 16-бітний CRC   									*/
				        w06.length -2*(sizeof(uint8_t)));
	w06.msg[w06.length-1]	= crc_calc_tx06 / 0x100;					/* 3. Старший байт CRC засилаємо в останній байт повідомлення 		*/
	w06.msg[w06.length-2]	= crc_calc_tx06 % 0x100;					/* 4. Молодший байт СRC засилаэмо в передостанный байт повыдомлення */
	if (slave_address==0x00){
		sprv06.timeout = 50;
	}
	else {
		sprv06.timeout = 100; 											/* 5. Задаємо час очікування відповіді в мілісекундах згідно з параметрами прикладого алгоритму */
	}
	sprv06.waited_normal_response_length=8;  								/* 6. Задаємо довжину очікуваної успішної відповіді 						*/
	sprv06.waited_error_response_length=5;								/* 7. Задаємо довжину очікуваної відповіді про помилку, віявлену сервером 	*/
	sprv06.waited_slave_response_address=slave_address;
	sprv06.waited_slave_response_function=WRITE_SINGLE_REGISTER;
//	sprv06.transmitting_began_at = HAL_GetTick();							/* Засікаємо момент відправки повідомлення  								*/
    sprv06.exception=(*ptr_WSR_exception);								/* задаємо посилання для отримання коду помилки від від сервера				*/
	sprv06.ptr_tx = &w06;
	sprv06.ptr_rx = &rx06;
    //	MB_Master_Transmit ();											/* 10. Відправляємо повідомлення 											*/
    master_state_5 = m_msg_transmitting;

    modbus_status_t TxIt = tw_txrxModbus_Session(
    		cport_nr,
    		&w06,
    		&rx06,
			sprv06.timeout
    		);

//	HAL_StatusTypeDef TxIt =HAL_UART_Transmit_IT(&huart4, &w06.msg, w06.length/* RX_MAX_SLAVE_MSG_LENGTH*/);
	if (TxIt != MODBUS_SUCCESS) {
		return TxIt;
	}
	else {
		mb_f_status = MB_Master_Supervisor (&sprv06);						/* 11 Викликаємо функцію прийому і обробки відповіді 						*/
	}
//LCD_Printf ("MB_Read_Holding_Registers SENT:\n");
//			for (uint8_t i08i=0;i08i<w06.length;++i08i){
//				   		LCD_Printf ("0x%X%X ",w06.msg[i08i]/0x10,w06.msg[i08i]%0x10);
//			}
//LCD_Printf ("\n");
//LCD_Printf ("Responce from Slave:\n");
//			for (uint8_t i08r=0;i08r<modbus_master_rx.length;++i08r){
//				   		LCD_Printf ("0x%X%X ",modbus_master_rx.msg[i08r]/0x10, modbus_master_rx.msg[i08r]%0x10);
//			}
//LCD_Printf ("\n\n");

	master_state_5=Idle;
	return (mb_f_status);/* 12 Якщо mb_f_status = MODBUS_SUCCESS то:
	 	 	 	 	 	 якщо (*ptr_WSR_exception) = 0x00 то запис у заданий регістр серевера успішно виконано
	 	 	 	 	 	 інакше = див. значення modbus_exception_t , модуль mb.h
	 	 	 	 	 	 Якщо mb_f_status != MODBUS_SUCCESS,див. значення modbus_status_t, модуль mb.h */
}


/*******************************************************************************************************************************************/
/* 0х08 Д І А Г Н О С Т И К А */
/*******************************************************************************************************************************************/
/* Ця функція викликається із вашого клiєнтського (Master) застосування, щоб перевірити працездатність комунікації клієнт-сервер
 * MODBUS function code 08 provides a series of tests for checking the communication system between a client device and a server,
 * or for checking various internal error conditions within a server.
The function uses a two–byte sub-function code field in the query to define the type of test to be performed.
The server echoes both the function code and sub-function code in a normal responce.

In general, issuing a diagnostic function to a remote device does not affect the running of the user program in the remote device.
 *
 *
 *  у заданий регістр сервера (Slave) необхідне вам значення
 * */
modbus_status_t MB_Diagnostic (	uint8_t slave_address,
								modbus_exception_t *ptr_WSR_exception) {
/* де:
 * slave_adress - байт адреси сервера;
 * *ptr_WSR_exception - показчик на адресу коду помилки, куди може бути записано код помилки в разі неуспішного виконання команди сервером.
 * В разі успішного завершення роботи функції - повертається MODBUS_SUCCESS, в разі помилки - повертається код помилки, див.модуль mb.h
 *
 * */
	volatile modbus_status_t mb_f08_status = MODBUS_ERR_UNKNOWN;
	modbus_master_tx_msg_t w08 = {0};
	modbus_master_rx_msg_t rx08= {0};
	sprv_t sprv08 = {0};
	uint16_t crc_calc_tx08 = 0;
	(*ptr_WSR_exception) = 0;											/* 1. Формуємо байти повідомлення: */
	w08.msg[0] = slave_address;	//w08.msg[0]=0x06;
	w08.msg[1] = DIAGNOSTIC;
	w08.msg[2] = 0x00;                                                  /* Два байти 0x00 0x00 - код підфункції функції діагностики, що передбачає */
	w08.msg[3] = 0x00;													/* відсутність поля даних і повернення лише коду підфункції*/

	w08.length = 6;

	crc_calc_tx08 = CRC_16x (w08.msg, 			    					/* 2. Обчислюємо 16-бітний CRC   									*/
				        w08.length -2*(sizeof(uint8_t)));
	w08.msg[w08.length-1]	= crc_calc_tx08 / 0x100;					/* 3. Старший байт CRC засилаємо в останній байт повідомлення 		*/
	w08.msg[w08.length-2]	= crc_calc_tx08 % 0x100;					/* 4. Молодший байт СRC засилаэмо в передостанный байт повыдомлення */

	sprv08.timeout = 100; 												/* 5. Задаємо час очікування відповіді в мілісекундах згідно з параметрами прикладого алгоритму */
	sprv08.waited_normal_response_length=6;  							/* 6. Задаємо довжину очікуваної успішної відповіді */
	sprv08.waited_error_response_length=5;								/* 7. Задаємо довжину очікуваної відповіді про помилку, віявлену сервером */
	sprv08.waited_slave_response_address=slave_address;
	sprv08.waited_slave_response_function=DIAGNOSTIC;
//	sprv08.transmitting_began_at = HAL_GetTick();						/* Засікаємо момент відправки повідомлення   */
	sprv08.ptr_tx = &w08;
    //	MB_Master_Transmit ();											/* 10. Відправляємо повідомлення  */
    master_state_5 = m_msg_transmitting;
    modbus_master_rx_msg_t rx_msg={0};
    modbus_status_t TxIt = tw_txrxModbus_Session (
    		cport_nr,
    		&w08,
    		&rx08,
			sprv08.timeout
    		);
//	HAL_StatusTypeDef TxIt =HAL_UART_Transmit_IT(&huart4, &(w08.msg), w08.length/* RX_MAX_SLAVE_MSG_LENGTH*/);
	if (TxIt != MODBUS_SUCCESS) {
		return TxIt;
	}
	else {
		mb_f08_status = MB_Master_Supervisor (&sprv08);					/* 11 Викликаємо функцію прийому і обробки відповіді */
	}

//LCD_Printf ("Diagnistic Message SENT:\n");
//for (uint8_t i08i=0;i08i<w08.length;++i08i){
//	   		LCD_Printf ("0x%X%X ",w08.msg[i08i]/0x10,w08.msg[i08i]%0x10);
//}
//LCD_Printf ("\n");
//LCD_Printf ("Responce from Slave:\n");
//for (uint8_t i08r=0;i08r<modbus_master_rx.length;++i08r){
//	   		LCD_Printf ("0x%X%X ",modbus_master_rx.msg[i08r]/0x10, modbus_master_rx.msg[i08r]%0x10);
//}
//LCD_Printf ("\n\n");												/*    і переходимо в режим очікування наступної команди */
	master_state_5=Idle;
	return (mb_f08_status);/* 12 Якщо mb_f_status = MODBUS_SUCCESS то:
	 	 	 	 	 	 якщо (*ptr_WSR_exception) = 0x00 то зв'язок Master->Slave->Master - успішний,
	 	 	 	 	 	 інакше = див. значення modbus_exception_t , модуль mb.h
	 	 	 	 	 	       Якщо mb_f_status != MODBUS_SUCCESS,див. значення modbus_status_t, модуль mb.h */
}
/*******************************************************************************************************************************************/
/* 0х10 (Dec 16) 	W R I T E   M U L T I P L E    R E G I S T E R S Write Multiple Registers 																				   */
/*******************************************************************************************************************************************/
/*This function code is used to write a block of contiguous registers (1 to 123 registers) in a remote device.
* The requested written values are specified in the request data field. Data is packed as two bytes per register.
* The normal response returns the function code, starting address, and quantity of registers written.
*
*  Ця функція викликається із вашого клєнтського (Master) застосування,щоб записати у пам'ять  з пам'яті сервера
 * задану кількіть 16-бітних регістрів, починаючи з заданої адреси у задані регістри сервера (Slave) необхідні вам значення
 * */
modbus_status_t MB_Write_Multiple_Registers (	uint8_t slave_address,
												uint16_t starting_address,
												uint16_t quantity_of_registers,						/* 2 bytes,  N							*/
												uint8_t byte_count, 								/* 1 byte,   2 x N*           			*/
												uint16_t *ptr_registers_value,						/* 2*N bytes							*/
												modbus_exception_t *ptr_WSR_exception) {
/* де:
 * slave_adress - байт адреси сервера;
 * starting_address         - 16-бітна адреса пам'яті сервера, починаючи з якої ви записуєте регістри; 0х0000..0хFFFF, обмеження згідно з документацією сервера
 * quantity_of_registers    - 16-бітне значення кількості регістів, які ви збираєтесь записувати; 0х0000..007B (123),  обмеження згідно з документацією сервера
 * ptr_received_values		is a pointer to an array of unsigned 16 bit values that will be sent to be written into server memory
				The size of the array needs to be at least nbr_regs. Note that you can ‘cast’ the unsigned values to signed values.
				As far as the Modbus protocol is concerned, it sends and receives 16 bit values and the interpretation of what these values mean
				is application specific.
 * *ptr_WSR_exception - показчик на адресу коду помилки, куди може бути записано код помилки в разі неуспішного виконання команди сервером.
 * В разі успішного завершення роботи фуункції - повертається MODBUS_SUCCESS, в разі помилки - повертається код помилки, див.модуль mb.h
 *
 * */
	modbus_status_t mb_f_status = MODBUS_ERR_UNKNOWN;
	modbus_master_tx_msg_t w10 = {0};
	modbus_master_rx_msg_t rx10= {0};
	sprv_t sprv10 = {0};
	uint16_t crc_calc_tx10 = 0;
	(*ptr_WSR_exception) = 0;											/* 1. Формуємо байти повідомлення: */
	w10.msg[0] = slave_address;
	w10.msg[1] = WRITE_MULTIPLE_REGISTERS;
	w10.msg[2] = starting_address / 0x100;
	w10.msg[3] = starting_address % 0x100;
	w10.msg[4] = 0x00;
	w10.msg[5] = (quantity_of_registers*2) % 0x100;
	for (uint8_t i_10=0;i_10<(quantity_of_registers*2);i_10=i_10+2){
		w10.msg[i_10+5]=*(ptr_registers_value+(i_10*sizeof(uint16_t))) / 0x100;
		w10.msg[i_10+6]=*(ptr_registers_value+(i_10*sizeof(uint16_t))) % 0x100;
	}
	w10.length = 5+(quantity_of_registers*2)+2;
	crc_calc_tx10 = CRC_16x (w10.msg, 			    					/* 2. Обчислюємо 16-бітний CRC   									*/
				        w10.length -2*(sizeof(uint8_t)));
	w10.msg[w10.length-1]	= crc_calc_tx10 / 0x100;					/* 3. Старший байт CRC засилаємо в останній байт повідомлення 		*/
	w10.msg[w10.length-2]	= crc_calc_tx10 % 0x100;					/* 4. Молодший байт СRC засилаэмо в передостанный байт повыдомлення */

	sprv10.timeout = 500; 												/* 5. Задаємо час очікування відповіді в мілісекундах згідно з параметрами прикладого алгоритму */
	sprv10.waited_normal_response_length=8;  							/* 6. Задаємо довжину очікуваної успішної відповіді */
	sprv10.waited_error_response_length=5;								/* 7. Задаємо довжину очікуваної відповіді про помилку, віявлену сервером */
	sprv10.waited_slave_response_address=slave_address;
	sprv10.waited_slave_response_function=WRITE_MULTIPLE_REGISTERS;
	sprv10.ptr_tx = &w10;
    //	MB_Master_Transmit ();											/* 10. Відправляємо повідомлення  */
    master_state_5 = m_msg_transmitting;
    modbus_master_rx_msg_t rx_msg={0};
    modbus_status_t TxIt = tw_txrxModbus_Session (
    		cport_nr,
    		&w10,
    		&rx10,
			sprv10.timeout
    		);
//	HAL_StatusTypeDef TxIt =HAL_UART_Transmit_IT(&huart4, &w10.msg, w10.length/* RX_MAX_SLAVE_MSG_LENGTH*/);
	if (TxIt != MODBUS_SUCCESS) {
		return TxIt;
	}
	else {
		mb_f_status = MB_Master_Supervisor (&sprv10);					/* 11 Викликаємо функцію прийому і обробки відповіді 				*/
	}
	if (mb_f_status==MODBUS_SUCCESS){									/* 12 Якщо функція виконалась без помилок, і якщо від сервера ми 	*/
																		/*    отримали нормальну відповідь, а не код помилки                	*/
			/* do nothing*/


	}
//LCD_Printf ("MB_Write_Multiple_Registers SENT:\n");
//		for (uint8_t i08i=0;i08i<w10.length;++i08i){
//			   		LCD_Printf ("0x%X%X ",w10.msg[i08i]/0x10,w10.msg[i08i]%0x10);
//		}
//LCD_Printf ("\n");
//LCD_Printf ("Responce from Slave:\n");
//		for (uint8_t i08r=0;i08r<modbus_master_rx.length;++i08r){
//			   		LCD_Printf ("0x%X%X ",modbus_master_rx.msg[i08r]/0x10, modbus_master_rx.msg[i08r]%0x10);
//		}
//LCD_Printf ("\n\n");
	memset (&rx_msg,0,sizeof(rx_msg));          /* 12 Обнуляємо буфер прийому відповіді сервера і  */
												/*    і переходимо в режим очікування наступної команди */
	master_state_5=Idle;
	return (mb_f_status);/* 12 Якщо mb_f_status = MODBUS_SUCCESS то:
	 	 	 	 	 	 якщо (*ptr_WSR_exception) = 0x00 то запис у заданий регістр серевера успішно виконано
	 	 	 	 	 	 інакше = див. значення modbus_exception_t , модуль mb.h
	 	 	 	 	 	       Якщо mb_f_status != MODBUS_SUCCESS,див. значення modbus_status_t, модуль mb.h */
}

/*********************************************************************************************************************************************
 *  								 P R I V A T E      F U N C T I O N S
**********************************************************************************************************************************************/

/*http://www.modbus.org/docs/Modbus_over_serial_line_V1_02.pdf
State "Idle" = no pending request. This is the initial state after power-up. A request can only be sent in "Idle" state. After sending
a request, the Master leaves the "Idle" state, and cannot send a second request at the same time
 When a unicast request is sent to a slave, the master goes into "Waiting for reply" state, and a “Response Time-out” is started. It
prevents the Master from staying indefinitely in "Waiting for reply" state. Value of the Response time-out is application
dependant.
 When a reply is received, the Master checks the reply before starting the data processing. The checking may result in an error,
for example a reply from an unexpected slave, or an error in the received frame. In case of a reply received from an unexpected
slave, the Response time-out is kept running. In case of an error detected on the frame, a retry may be performed.
 If no reply is received, the Response time-out expires, and an error is generated. Then the Master goes into "Idle" state, enabling
a retry of the request. The maximum number of retries depends on the master set-up.
MODBUS over serial line specification and implementation guide V1.02 Modbus.org
Modbus.org http://www.modbus.org/ 10/44
Dec 20, 2006
 When a broadcast request is sent on the serial bus, no response is returned from the slaves. Nevertheless a delay is respected
by the Master in order to allow any slave to process the current request before sending a new one. This delay is called
"Turnaround delay". Therefore the master goes into "Waiting Turnaround delay" state before going back in "idle" state and before
being able to send another request.
 In unicast the Response time out must be set long enough for any slave to process the request and return the response, in
broadcast the Turnaround delay must be long enough for any slave to process only the request and be able to receive a new one.
Therefore the Turnaround delay should be shorter than the Response time-out. Typically the Response time-out is from 1s to
several second at 9600 bps; and the Turnaround delay is from 100 ms to 200ms.
 Frame error consists of : 1) Parity checking applied to each character; 2) Redundancy checking applied to the entire frame
*/
//enum master_state_5_t {
//	Idle,						/* очікування наступного завдання	*/
//	m_msg_transmitting,
//  m_msg_transmitting_ERROR,
//	m_msg_transmitting_Cplt,
//	Waiting_turnaround_delay,   /* очікування поки slaves оброблять завдання циркулярного повідомлення, передача нових повідомлень не допускається 	*/
//	Waiting_for_reply,			/* очікування відповіді на вибіркове повідомлення 			*/
//  m_msg_receiving,
//	m_msg_recived_RxIdle,
//	Processing_reply,			/* обробка відповіді				*/
//	Processing_error			/* обробка помилки/аварії			*/
//};


static modbus_status_t MB_Master_Supervisor (sprv_t *obj) {
	volatile modbus_status_t MB_sv_status = MODBUS_ERR_UNKNOWN;

	/*
	 * Будь яка вибіркова команда модбас може передбачати конкретну структуру відповіді.переводимо master-5 в стан очікування відповіді
	 * Функція tw_txrxModbus_Session() передбачає передачу повідомлення Модбас через СОМ-порт, переключення СОМ-порту в режим прийому,
	 * прийом повдомлення, отримумо або повідомлення або аварію по таймауту

	 *
	 * Якщо отримуємо яку небудь відповідь:
	 * 		версія це сміття,
	 * 		версія - це відповідь
	 * 		версія - це повідомлення про помилку
	 *
	 * Функція викликається у тілі функцій MODBUS.
	 *   вхідними параметрами є приватна структура
	 *   struct sprv_t {										ДЛЯ ДАНОЇ ФУНКЦІЇ МОДБУС
						uint8_t  waited_normal_response_length;       - очікувана тривалість нормальної відповіді клієнту (Master) від сервера (Slave)
						uint8_t  waited_error_response_length;		  - очікувана довжина повідомлення про помилку при звертанні до сервера
						uint8_t  waited_slave_response_address;		  - очікувана адреса сервера
						uint8_t  waited_slave_response_function;      - очікуваний код функції від сервера
						uint32_t transmitting_began_at;				  - точка відліку часу в мілісекундах від старту запиту, не використовується
						modbus_exception_t *ptr_exception;			  - показник на адресу повернення коду повідомлення про помилку від сервера

						};
//	 *Фунція реалізована як цикл, обчислення в якому здійснюються протягом очікуваного заданого терміну відповіді сервера obj->timeout на запит клієнта
//	 *		у взаємодії з функціями обробки переривань по уарт клієта:
//	 *		void HAL_UART_RxIdleCallback(UART_HandleTypeDef *UartHandle)  Обробка переривання IDLE, подія закінчення прийому повідомлення від сервера
//			void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)  Set transmission flag: transfer complete - не використовується, резерв
//			void HAL_UART_TxCpltCallback (UART_HandleTypeDef *UartHandle) Обробка переривання TxCplt подія завершення передачі повідомлення від клієнта серверу
//			void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)   Обробка повідомлення про помилкову роботу UART
//	 *
	 * */
	while(MB_sv_status == MODBUS_ERR_UNKNOWN){
		modbus_master_tx_msg_t *htx=NULL;
		modbus_master_rx_msg_t *hrx=NULL;
	    htx=obj->ptr_tx;
        hrx=obj->ptr_rx;

		/* 1. Якщо довжина отриманого повідомлення не відповідає очікуваному повідомленню
		 * чи очікуваному повідомлення про помилку
		 * це сміття */
		if (!((hrx->length==obj->waited_error_response_length)||(hrx->length==obj->waited_normal_response_length))){
			MB_sv_status=MODBUS_ERR_BYTE_COUNT;
			 master_state_5=Idle;
			break;
		}
		/* 2. Обчислюємо 16-бітний CRC   									*/
		uint16_t crc_calc_rx = CRC_16x(hrx->msg,hrx->length -2);
		/* якщо CRC у повідомленні не правильний*/
		if (
			(hrx->msg[hrx->length-1] != crc_calc_rx / 0x100) ||
			(hrx->msg[hrx->length-2] != crc_calc_rx % 0x100)
		   ){
		   MB_sv_status = MODBUS_ERR_CRC;
		   master_state_5=Idle;
		   break;
		}
		/* 3. якщо адреса вихідного повідомлення не відповідає адресі вхідного повідомлення */
		if(htx->msg[0]!=hrx->msg[0]){
			MB_sv_status=MODBUS_ERR_SLAVE_ADDR;
			break;
		}
		/* Якщо це повідомлення про помилку (Exception), про що свідчить
		 * код повідомлення і дожина повідомлення
		 * повертаємо код помилки (Exception)  */
		if((htx->msg[1]==(0x80+hrx->msg[1])) || (hrx->length==obj->waited_error_response_length)){
			MB_sv_status=MODBUS_ERR_fromSLAVE;
			obj->exception=hrx->msg[2];
			break;
		}
		/* кщо це не код помилки і не код функції */
		else if(((htx->msg[1]!=(hrx->msg[1])) || (hrx->length==obj->waited_normal_response_length))){
			MB_sv_status=MODBUS_ERR_FC;
			break;
		}
		/* це нормальна очікувана відповідь*/
		else{
			obj->exception=MODBUS_F_SUCCESS;
			MB_sv_status=MODBUS_SUCCESS;
		}
	}
    return (MB_sv_status);
}
/*********************************************************************************************************************************************
 * *******************************************************************************************************************************************
 * ********************************************************************************************************************************************
 */












//void MB_Master_Handling (void) {
//LCD_Printf ("Slave_Handling\n");
//	if (modbus_master_state == new_message_for_us ){
//
//		/*******************************************************************************************************************************************/
//		/* 0х03 	R E A D    H O L D I N G    R E G I S T E R S  																				   */
//		/*******************************************************************************************************************************************/
//		if (m_msg_received[1]==READ_HOLDING_REGISTERS){
//			if (m_msg_received_length == 8){
//				uint16_t Starting_Address = m_msg_received[2] * 0x100 + msg_received[3];
//				uint16_t Quantity_of_Registers=m_msg_received[4] * 0x100 + msg_received[5];
//				if (Quantity_of_Registers < 125){							/* 0x01, 0x03, 0xII, 0xII, 0x00, 0x07 0xCRCry 0xCRCrh*/
//					m_msg_transmitted [1]=READ_HOLDING_REGISTERS;             /* формуємо код функції як отриманий, засилаємо в байт 1*/
//					uint8_t ByteCount=Quantity_of_Registers * 2;			/* обчислюємо кількість байт, що вимагає Master */
//					m_msg_transmitted [2] = ByteCount % 0x100;				/* засилаємо кількість інформаційних байт, засилаємо в байт 2 */
//							for (uint8_t ibc=3; ibc<=ByteCount+3; ++ibc){			/* в байти наступні засилаємо відповідь прикладного процесу */
//								m_msg_transmitted [ibc] = ibc;                        /* заглушка для прикладної функції !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
//							}
//					m_msg_transmitted_length=ByteCount+5;						/* обчислюємо кількість байт у повідомленні-відповіді */
//				}															/* 0x01, 0x03, 0x0e, 0x03, 0x04, ..., 0x0d, 0x0e, 0x0f, 0x10, 0x11 0xCRC, 0xCRC */
//				else { /* Quantity of Registers is WRONG */
//					msg_transmitted[1] = READ_HOLDING_REGISTERS | 0x80;		/* формуємо код-функції exception-function_code як отриманий + 0x80  */
//					msg_transmitted[2]	=  MODBUS_EXC_ILLEGAL_DATA_VALUE; 	/* засилаємо в наступний байт код помилки згідно зі стандартом Modbus*/
//																			/* повертаємо exception-function_code,  якщо довжина неправильна */
//					msg_transmitted_length=5;								/* встановлюємо довжину повідомлення про помилку		*/
//				}
//			}
//			else {/* msg_received_length is WRONG */
//				msg_transmitted[1] = READ_HOLDING_REGISTERS | 0x80;		/* формуємо код-функції exception-function_code як отриманий + 0x80  */
//				msg_transmitted[2]	=  MODBUS_EXC_ILLEGAL_DATA_VALUE; 	/* засилаємо в наступний байт код помилки згідно зі стандартом Modbus*/
//																		/* повертаємо exception-function_code,  якщо довжина неправильна */
//				msg_transmitted_length=5;								/* встановлюємо довжину повідомлення про помилку		*/
//			}
//		} /* end of READ_HOLDING_REGISTERS*/
//		/*******************************************************************************************************************************************/
//		/* 0х06     W R I T E    S I N G L E    R E G I S T E R 																				   */
//		/*******************************************************************************************************************************************/
//		else if (m_msg_received[1]==WRITE_SINGLE_REGISTER){
//			if (m_msg_received_length == 8){
//
//				uint16_t Register_Address = m_msg_received[2] * 0x100 + msg_received[3];
//				uint16_t Register_Value   = m_msg_received[4] * 0x100 + msg_received[5];
//
//
//				/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
//				msg_transmitted [1]=WRITE_SINGLE_REGISTER;             /* формуємо код функції як отриманий, засилаємо в байт 1*/
//
//				msg_transmitted [2] = Register_Address / 0x100;			/* засилаємо в регіст 2 старший байт  										*/
//				msg_transmitted [3] = Register_Address % 0x100;			/* засилаємо в регіст 3 молодший байт  										*/
//				msg_transmitted [4] = Register_Value / 0x100;			/* засилаємо в регіст 4 старший байт  										*/
//				msg_transmitted [5] = Register_Value % 0x100;			/* засилаємо в регіст 5 молодший байт  										*/
//
//			msg_transmitted_length=8;						/* обчислюємо кількість байт у повідомленні-відповіді 									*/
//			}
//			else {/* msg_received_length is WRONG */
//				msg_transmitted[1] = WRITE_SINGLE_REGISTER | 0x80;		/* формуємо код-функції exception-function_code як отриманий + 0x80  		*/
//				msg_transmitted[2]	=  MODBUS_EXC_ILLEGAL_DATA_VALUE; 	/* засилаємо в наступний байт код помилки згідно зі стандартом Modbus		*/
//																		/* повертаємо exception-function_code,  якщо довжина неправильна 			*/
//				msg_transmitted_length=5;								/* встановлюємо довжину повідомлення про помилку							*/
//			}
//		}
//		/*******************************************************************************************************************************************/
//		/* 0х08 Д І А Г Н О С Т И К А */
//		/*******************************************************************************************************************************************/
//		else if (msg_received[1]==DIAGNOSTIC ){                       					/* Якщо отримана Modbus-функція 0х08 Діагностика 					*/
////			if 		(
////					((msg_received_length>=6) && (msg_received_length<125)) &&
////					(msg_received_length % 2 == 0) &&									/*   {0x01, 0x88, 0x00, 0x00, 0xCRCty, 0xCRCth}*/
////					((msg_received [2]==0x00) && (msg_received[3]==0x00))
////					) { 	/* і якщо отримана підфункція 0х0000 - Return Query Data  			*/																										/*   {0x01, 0x08, 0x00, 0x00, 0xCRCry, 0xCRCrh}*/
////						uint8_t N_TestData=(msg_received_length-6)/2;
////						msg_transmitted_length=6+(N_TestData*2);
////						msg_transmitted[1]=DIAGNOSTIC;									/*  ОК,    повертаємо отримане повідомлення, якщо правильна довжина  */
////						msg_transmitted[2]=0x00;										/*   {0x01, 0x88, 0x00, 0x00, 0xCRCty, 0xCRCth}*/
////						msg_transmitted[3]=0x00;
////						msg_transmitted_length=4+N_TestData*2+2;
////						if (N_TestData !=0){
////							for (uint8_t i_dgn=0;i_dgn<N_TestData*2;++i_dgn){
////								msg_transmitted[4+i_dgn]=msg_received[4+i_dgn];
////							}
////						}
////			}
////			else{																/* якщо кiлькість байт у повыдомленны менше шести						*/
////																				/* або якщо підфункція не 0000, і у нас вона не реалізована				*/
////																				/* або неправильно задана кількість даних                  				*/
////						msg_transmitted[1] = DIAGNOSTIC | 0x80;		    		/* формуємо код-команди exception-function_code як отриманий + 0ч80  	*/
////						msg_transmitted[2]	=  MODBUS_EXC_ILLEGAL_DATA_VALUE; 	/* засилаємо в наступний байт код помилки згідно зі стандартом Modbus	*/
////																				/* повертаємо exception-function_code,  якщо довжина неправильна 		*/
////						msg_transmitted_length=5;								/* встановлюємо довжину повідомлення про помилку						*/
////					}
//
//		}
//		/*******************************************************************************************************************************************
//		 *  П О М И Л К О В И Й      К О Д     Ф У Н К Ц І Ї      М о д б у с
//		 ******************************************************************************************************************************************/
//		else {   /* помилка кода команди */                            /* Якщо ми отримали помилковий код команди Modbus: 					*/
////			msg_transmitted[1]=msg_received[1] | 0x80;				   /* формуємо код-команди exception-function_code як отриманий + 0ч80  */
////			msg_transmitted[2]	= MODBUS_EXC_ILLEGAL_FUNCTION;		   /* засилаємо в наступний байт код помилки згідно зі стандартом Modbus*/
////			msg_transmitted_length=5;								   /* встановлюємо довжину повідомлення про помилку						*/
////			uint16_t crc_calc_tx = CRC_16x (msg_transmitted, 			    /* 2. Обчислюэмо 16-бітний CRC   									*/
////						        msg_transmitted_length -2*(sizeof(uint8_t)));
////				msg_transmitted[msg_transmitted_length-1]	= crc_calc_tx / 0x100;	/* 3. Старший байт CRC засилаємо в останній байт повідомлення 		*/
////				msg_transmitted[msg_transmitted_length-2]	= crc_calc_tx % 0x100;	/* 4. Молодший байт СRC засилаэмо в передостанный байт повыдомлення */
////				modbus_slave_state=answer_transmitting;
////				MB_Slave_Transmit ();											/* 5. Відправляємо повідомлення                                     */
//		}
//		/************************************************************************************************************************************
//		 *  Ф О Р М У В А Н Н Я     В І Д П О В І Д І  НА  повідомлення, адресоване конкретно нам
//		 ************************************************************************************************************************************/
//
//
//		for (uint8_t izb=0;izb<msg_received_length;++izb){				/* Обнуляэмо буфер вхідного повідомлення */
////LCD_Printf ("0x%X%X ", 	msg_received [izb]/0x10, msg_received [izb]%0x10 );	/* debug only */
////			msg_received[izb]=0x00;
////		}
////																		/* Обнуляэмо буфер вхыдного повыдомлення */
////																		/* Формуємо повідомлення - відповідь для Master   					*/
////
////		msg_transmitted[0]=SLAVE_NODE_ADDRESS;                          /* 1. В перший/нульвий байт - адресу Master - 0x00 					*/
////		uint16_t crc_calc_tx = CRC_16x (msg_transmitted, 			    /* 2. Обчислюэмо 16-бітний CRC   									*/
////				        msg_transmitted_length -2*(sizeof(uint8_t)));
////		msg_transmitted[msg_transmitted_length-1]	= crc_calc_tx / 0x100;	/* 3. Старший байт CRC засилаємо в останній байт повідомлення 		*/
////		msg_transmitted[msg_transmitted_length-2]	= crc_calc_tx % 0x100;	/* 4. Молодший байт СRC засилаэмо в передостанный байт повыдомлення */
////		modbus_slave_state=answer_transmitting;
////		MB_Slave_Transmit ();											/* 5. Відправляємо повідомлення                                     */
////		/************************************************************************************************************************************
//		 * **********************************************************************************************************************************
//		 */
//	}   /*  Р Е А К Ц І Я       Н А     Ц И Р К У Л Я Р Н Е  П О В І Д О М Л Е Н Я    */
//	/*****************************************************************************************************************************************/
//	else if (modbus_slave_state == broadcast_message ) {
//LCD_Printf("modbus_slave_state == broadcast_message\n");
//		/**************************************************************************************************************************************
//		 * 0х05  W R I T E _  S I N G L E _  C O I L
//		 *************************************************************************************************************************************/
//		if (msg_received[1]==WRITE_SINGLE_COIL){
//
//			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
//LCD_Printf ("WRITE_SINGLE_COIL\n");
//			modbus_slave_state = no_message_for_us;
//		}
//		/*************************************************************************************************************************************/
//		else { 	/* помилка кода команди */                             /* скидаємо кількісь байт у вхідному повідомленні, чим уможливлюємо   */
//LCD_Printf ("FUNCTION CODE WRONG\n");								   /* роботу переривань по відсутності сигнала в мережі					 */
//			msg_received_length=0;
//HAL_Delay (5000);
//			modbus_slave_state=no_message_for_us;
//		}
//	/****************************************************************************************************************************************/
//		for (uint8_t izb=0;izb<msg_received_length;++izb){				/* Обнуляэмо буфер вхідного повідомлення */
//			msg_received[izb]=0;
//		}
//	}
//	else {}
//}




