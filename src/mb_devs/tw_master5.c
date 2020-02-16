/*
 * mb_master5.c
 *
 *  Created on: 30 ����. 2017 �.
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
/* 0�03 	R E A D    H O L D I N G    R E G I S T E R S  																				   */
/*******************************************************************************************************************************************/

/* �� ������� ����������� �� ������ �볺�������� (Master) ������������,��� ������� � ���'�� ������� ������ ������ 16-����� �������
 * ��������� � ������ ������ ������� ������ ������� (Slave) ��������� ��� ��������
 * */
modbus_status_t MB_Read_Holding_Registers (	uint8_t slave_address,
											uint16_t starting_address,
											uint16_t quantity_of_registers,
											uint16_t *ptr_received_values,
											modbus_exception_t *ptr_WSR_exception) {
/* ��:
 * slave_adress - ���� ������ �������;
 * starting_address         - 16-���� ������ ���'�� �������, ��������� � ��� �� ������� �������; 0�0000..0�FFFF, ��������� ����� � ������������� �������
 * quantity_of_registers    - 16-���� �������� ������� ������, �� �� ��������� ���������; 0�0000..007D (125),  ��������� ����� � ������������� �������
 * ptr_received_values		is a pointer to an array of unsigned 16 bit values that will receive the value of all the registers you are reading.
				The size of the array needs to be at least nbr_regs. Note that you can �cast� the unsigned values to signed values.
				As far as the Modbus protocol is concerned, it sends and receives 16 bit values and the interpretation of what these values mean
				is application specific.
 * *ptr_WSR_exception - �������� �� ������ ���� �������, ���� ���� ���� �������� ��� ������� � ��� ���������� ��������� ������� ��������.
 * � ��� �������� ���������� ������ �������� - ����������� MODBUS_SUCCESS, � ��� ������� - ����������� ��� �������, ���.������ mb.h
 *
 * */
	modbus_status_t mb_f_status = MODBUS_ERR_UNKNOWN;
	modbus_master_tx_msg_t w03 = {0};
	modbus_master_rx_msg_t rx03={0};
	sprv_t sprv03 = {0};
	uint16_t crc_calc_tx03 = 0;										/* 1. ������� ����� �����������: */
	w03.msg[0] = slave_address;
	w03.msg[1] = READ_HOLDING_REGISTERS;
	w03.msg[2] = starting_address / 0x100;
	w03.msg[3] = starting_address % 0x100;
	w03.msg[4] = 0x00;
	w03.msg[5] = quantity_of_registers % 0x100;
	w03.length = 8;
	crc_calc_tx03 = CRC_16x (w03.msg, 			    					/* 2. ���������� 16-����� CRC   									*/
				        w03.length -2*(sizeof(uint8_t)));
	w03.msg[w03.length-1]	= crc_calc_tx03 / 0x100;					/* 3. ������� ���� CRC �������� � ������� ���� ����������� 		*/
	w03.msg[w03.length-2]	= crc_calc_tx03 % 0x100;					/* 4. �������� ���� �RC ��������� � ������������� ���� ������������ */

	sprv03.timeout = 100; 												/* 5. ������ ��� ���������� ������ � ���������� ����� � ����������� ���������� ��������� */
	sprv03.waited_normal_response_length=5+quantity_of_registers*2;  	/* 6. ������ ������� ��������� ������ ������ */
	sprv03.waited_error_response_length=5;								/* 7. ������ ������� ��������� ������ ��� �������, ������� �������� */
	sprv03.waited_slave_response_address=slave_address;
	sprv03.waited_slave_response_function=READ_HOLDING_REGISTERS;
//	sprv03.transmitting_began_at = HAL_GetTick();						/* ������� ������ �������� �����������   */

	sprv03.ptr_tx = &w03;
	sprv03.ptr_rx = &rx03;
    //	MB_Master_Transmit ();											/* 10. ³���������� �����������  */
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
		mb_f_status = MB_Master_Supervisor (&sprv03);					/* 11 ��������� ������� ������� � ������� ������ 				*/
	}
	if ((mb_f_status==MODBUS_SUCCESS)&&((sprv03.exception)==MODBUS_F_SUCCESS)){	/* 12 ���� ������� ���������� ��� �������, � ���� �� ������� �� 	*/


		for (uint8_t i03j=0;i03j<quantity_of_registers;++i03j){			/*    �������� ���� �������, � �� ��� �������                	*/
			(*(ptr_received_values+i03j*sizeof(uint16_t)))=				/*    �������� ���� ������� � ����������� �� ������� 			*/
					rx03.msg[i03j+3]*0x100 +							/*    � ���'��� �� �������, ������� ���������� ���������			*/
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
	return (mb_f_status);/* 12 ���� mb_f_status = MODBUS_SUCCESS ��:
	 	 	 	 	 	 ���� (*ptr_WSR_exception) = 0x00 �� ����� � ������� ������ �������� ������ ��������
	 	 	 	 	 	 ������ = ���. �������� modbus_exception_t , ������ mb.h
	 	 	 	 	 	       ���� mb_f_status != MODBUS_SUCCESS,���. �������� modbus_status_t, ������ mb.h */
}

/**************************************************************************************************************************************
* 0�05  W R I T E _  S I N G L E _  C O I L
*************************************************************************************************************************************/



/* This function code is used to write a single output to either ON or OFF in a remote device */

modbus_status_t MB_Write_Syngle_Coil (	    uint8_t  slave_address,
											uint16_t output_address,
											uint16_t output_value,
											modbus_exception_t *ptr_WSR_exception) {
/* ��:
 * slave_adress - ���� ������ �������;
 * reg_address  - 16-���� ������ ���'�� �������, ���� �� �������� ���������� ��� ��������;
 * reg_value    - 16-���� �������� ��������� � ��������� ������, ��� �� �������� � ������;
 * *ptr_WSR_exception - �������� �� ������ ���� �������, ���� ���� ���� �������� ��� ������� � ��� ���������� ��������� ������� ��������.
 * � ��� �������� ���������� ������ �������� - ����������� MODBUS_SUCCESS, � ��� ������� - ����������� ��� �������, ���.������ mb.h
 *
 * */
	modbus_status_t mb_f_status = MODBUS_ERR_UNKNOWN;
	modbus_master_tx_msg_t w05 = {0};
	modbus_master_rx_msg_t rx05={0};
	sprv_t sprv05 = {0};
	uint16_t crc_calc_tx05 = 0;
	(*ptr_WSR_exception) = 0;											/* 1. ������� ����� �����������: 									*/
	w05.msg[0] = slave_address;											/* 0x00 ���� ���������� �����������, 0�01..0�FF*/
	w05.msg[1] = WRITE_SINGLE_COIL;
	w05.msg[2] = output_address / 0x100;                                /* 0x0000..0xFFFF													*/
	w05.msg[3] = output_address % 0x100;
	w05.msg[4] = output_value   / 0x100;								/* 0x0000 - Off, 0xFF00 - On									    */
	w05.msg[5] = output_value   % 0x100;
	w05.length = 8;
	crc_calc_tx05 = CRC_16x (w05.msg, 			    					/* 2. ���������� 16-����� CRC   									*/
				        w05.length -2*(sizeof(uint8_t)));
	w05.msg[w05.length-1]	= crc_calc_tx05 / 0x100;					/* 3. ������� ���� CRC �������� � ������� ���� ����������� 		*/
	w05.msg[w05.length-2]	= crc_calc_tx05 % 0x100;					/* 4. �������� ���� �RC ��������� � ������������� ���� ������������ */

	if (slave_address==0x00){
		sprv05.timeout = 50; 											/* 5. ������ ��� ���������� ������ � ���������� ����� � ����������� ���������� ��������� */
	}
	else {
		sprv05.timeout = 100;
	}
	sprv05.waited_normal_response_length=8;  							/* 6. ������ ������� ��������� ������ ������ */
	sprv05.waited_error_response_length=5;								/* 7. ������ ������� ��������� ������ ��� �������, ������� �������� */
	sprv05.waited_slave_response_address=slave_address;
	sprv05.waited_slave_response_function=WRITE_SINGLE_COIL;
//	sprv05.transmitting_began_at = HAL_GetTick();						/* ������� ������ �������� �����������   */
    sprv05.exception=(*ptr_WSR_exception);								/* ������ ��������� ��� ��������� ���� ������� �� �� ������� */
	sprv05.ptr_tx = &w05;
	sprv05.ptr_rx = &rx05;
    //	MB_Master_Transmit ();											/* 10. ³���������� �����������  */
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
		/* 11 ��������� ������� ������� � ������� ������ */
		mb_f_status = MB_Master_Supervisor(&sprv05);					/* 11 ��������� ������� ������� � ������� ������ */
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
	return (mb_f_status);/* 12 ���� mb_f_status = MODBUS_SUCCESS ��:
	 	 	 	 	 	 ���� (*ptr_WSR_exception) = 0x00 �� ����� � ������� ������ �������� ������ ��������
	 	 	 	 	 	 ������ = ���. �������� modbus_exception_t , ������ mb.h
	 	 	 	 	 	       ���� mb_f_status != MODBUS_SUCCESS,���. �������� modbus_status_t, ������ mb.h */




}
/*******************************************************************************************************************************************/
/* 0�06     W R I T E    S I N G L E    R E G I S T E R 																				   */
/*******************************************************************************************************************************************/
/* �� ������� ����������� �� ������ ���������� (Master) ������������,��� �������� � ������� ������ ������� (Slave) ��������� ��� ��������
 * */
modbus_status_t MB_Write_Single_Register (	uint8_t slave_address,
											uint16_t reg_address,
											uint16_t reg_value,
											modbus_exception_t *ptr_WSR_exception) {
/* ��:
 * slave_adress - ���� ������ �������;
 * reg_address  - 16-���� ������ ���'�� �������, ���� �� �������� ���������� ��� ��������;
 * reg_value    - 16-���� �������� ��������� � ��������� ������, ��� �� �������� � ������;
 * *ptr_WSR_exception - �������� �� ������ ���� �������, ���� ���� ���� �������� ��� ������� � ��� ���������� ��������� ������� ��������.
 * � ��� �������� ���������� ������ �������� - ����������� MODBUS_SUCCESS, � ��� ������� - ����������� ��� �������, ���.������ mb.h
 *
 * */
	modbus_status_t mb_f_status = MODBUS_ERR_UNKNOWN;
	modbus_master_tx_msg_t w06 = {0};
	modbus_master_rx_msg_t rx06= {0};
	sprv_t sprv06 = {0};
	uint16_t crc_calc_tx06 = 0;
	(*ptr_WSR_exception) = 0;											/* 1. ������� ����� �����������: 									*/
	w06.msg[0] = slave_address;											/* 0x00 - ���������� �����������, 0�01..0�FF					    */
	w06.msg[1] = WRITE_SINGLE_REGISTER;
	w06.msg[2] = reg_address / 0x100;									/* 0x0000 ... 0xFFFF												*/
	w06.msg[3] = reg_address % 0x100;
	w06.msg[4] = reg_value   / 0x100;									/* 0x0000 ... 0xFFFF 											    */
	w06.msg[5] = reg_value   % 0x100;
	w06.length = 8;
	crc_calc_tx06 = CRC_16x (w06.msg, 			    				/* 2. ���������� 16-����� CRC   									*/
				        w06.length -2*(sizeof(uint8_t)));
	w06.msg[w06.length-1]	= crc_calc_tx06 / 0x100;					/* 3. ������� ���� CRC �������� � ������� ���� ����������� 		*/
	w06.msg[w06.length-2]	= crc_calc_tx06 % 0x100;					/* 4. �������� ���� �RC ��������� � ������������� ���� ������������ */
	if (slave_address==0x00){
		sprv06.timeout = 50;
	}
	else {
		sprv06.timeout = 100; 											/* 5. ������ ��� ���������� ������ � ���������� ����� � ����������� ���������� ��������� */
	}
	sprv06.waited_normal_response_length=8;  								/* 6. ������ ������� ��������� ������ ������ 						*/
	sprv06.waited_error_response_length=5;								/* 7. ������ ������� ��������� ������ ��� �������, ������� �������� 	*/
	sprv06.waited_slave_response_address=slave_address;
	sprv06.waited_slave_response_function=WRITE_SINGLE_REGISTER;
//	sprv06.transmitting_began_at = HAL_GetTick();							/* ������� ������ �������� �����������  								*/
    sprv06.exception=(*ptr_WSR_exception);								/* ������ ��������� ��� ��������� ���� ������� �� �� �������				*/
	sprv06.ptr_tx = &w06;
	sprv06.ptr_rx = &rx06;
    //	MB_Master_Transmit ();											/* 10. ³���������� ����������� 											*/
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
		mb_f_status = MB_Master_Supervisor (&sprv06);						/* 11 ��������� ������� ������� � ������� ������ 						*/
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
	return (mb_f_status);/* 12 ���� mb_f_status = MODBUS_SUCCESS ��:
	 	 	 	 	 	 ���� (*ptr_WSR_exception) = 0x00 �� ����� � ������� ������ �������� ������ ��������
	 	 	 	 	 	 ������ = ���. �������� modbus_exception_t , ������ mb.h
	 	 	 	 	 	 ���� mb_f_status != MODBUS_SUCCESS,���. �������� modbus_status_t, ������ mb.h */
}


/*******************************************************************************************************************************************/
/* 0�08 � � � � � � � � � � � */
/*******************************************************************************************************************************************/
/* �� ������� ����������� �� ������ ��i��������� (Master) ������������, ��� ��������� ������������� ���������� �볺��-������
 * MODBUS function code 08 provides a series of tests for checking the communication system between a client device and a server,
 * or for checking various internal error conditions within a server.
The function uses a two�byte sub-function code field in the query to define the type of test to be performed.
The server echoes both the function code and sub-function code in a normal responce.

In general, issuing a diagnostic function to a remote device does not affect the running of the user program in the remote device.
 *
 *
 *  � ������� ������ ������� (Slave) ��������� ��� ��������
 * */
modbus_status_t MB_Diagnostic (	uint8_t slave_address,
								modbus_exception_t *ptr_WSR_exception) {
/* ��:
 * slave_adress - ���� ������ �������;
 * *ptr_WSR_exception - �������� �� ������ ���� �������, ���� ���� ���� �������� ��� ������� � ��� ���������� ��������� ������� ��������.
 * � ��� �������� ���������� ������ ������� - ����������� MODBUS_SUCCESS, � ��� ������� - ����������� ��� �������, ���.������ mb.h
 *
 * */
	volatile modbus_status_t mb_f08_status = MODBUS_ERR_UNKNOWN;
	modbus_master_tx_msg_t w08 = {0};
	modbus_master_rx_msg_t rx08= {0};
	sprv_t sprv08 = {0};
	uint16_t crc_calc_tx08 = 0;
	(*ptr_WSR_exception) = 0;											/* 1. ������� ����� �����������: */
	w08.msg[0] = slave_address;	//w08.msg[0]=0x06;
	w08.msg[1] = DIAGNOSTIC;
	w08.msg[2] = 0x00;                                                  /* ��� ����� 0x00 0x00 - ��� ��������� ������� ����������, �� ��������� */
	w08.msg[3] = 0x00;													/* ��������� ���� ����� � ���������� ���� ���� ���������*/

	w08.length = 6;

	crc_calc_tx08 = CRC_16x (w08.msg, 			    					/* 2. ���������� 16-����� CRC   									*/
				        w08.length -2*(sizeof(uint8_t)));
	w08.msg[w08.length-1]	= crc_calc_tx08 / 0x100;					/* 3. ������� ���� CRC �������� � ������� ���� ����������� 		*/
	w08.msg[w08.length-2]	= crc_calc_tx08 % 0x100;					/* 4. �������� ���� �RC ��������� � ������������� ���� ������������ */

	sprv08.timeout = 100; 												/* 5. ������ ��� ���������� ������ � ���������� ����� � ����������� ���������� ��������� */
	sprv08.waited_normal_response_length=6;  							/* 6. ������ ������� ��������� ������ ������ */
	sprv08.waited_error_response_length=5;								/* 7. ������ ������� ��������� ������ ��� �������, ������� �������� */
	sprv08.waited_slave_response_address=slave_address;
	sprv08.waited_slave_response_function=DIAGNOSTIC;
//	sprv08.transmitting_began_at = HAL_GetTick();						/* ������� ������ �������� �����������   */
	sprv08.ptr_tx = &w08;
    //	MB_Master_Transmit ();											/* 10. ³���������� �����������  */
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
		mb_f08_status = MB_Master_Supervisor (&sprv08);					/* 11 ��������� ������� ������� � ������� ������ */
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
//LCD_Printf ("\n\n");												/*    � ���������� � ����� ���������� �������� ������� */
	master_state_5=Idle;
	return (mb_f08_status);/* 12 ���� mb_f_status = MODBUS_SUCCESS ��:
	 	 	 	 	 	 ���� (*ptr_WSR_exception) = 0x00 �� ��'���� Master->Slave->Master - �������,
	 	 	 	 	 	 ������ = ���. �������� modbus_exception_t , ������ mb.h
	 	 	 	 	 	       ���� mb_f_status != MODBUS_SUCCESS,���. �������� modbus_status_t, ������ mb.h */
}
/*******************************************************************************************************************************************/
/* 0�10 (Dec 16) 	W R I T E   M U L T I P L E    R E G I S T E R S Write Multiple Registers 																				   */
/*******************************************************************************************************************************************/
/*This function code is used to write a block of contiguous registers (1 to 123 registers) in a remote device.
* The requested written values are specified in the request data field. Data is packed as two bytes per register.
* The normal response returns the function code, starting address, and quantity of registers written.
*
*  �� ������� ����������� �� ������ ���������� (Master) ������������,��� �������� � ���'���  � ���'�� �������
 * ������ ������ 16-����� �������, ��������� � ������ ������ � ����� ������� ������� (Slave) �������� ��� ��������
 * */
modbus_status_t MB_Write_Multiple_Registers (	uint8_t slave_address,
												uint16_t starting_address,
												uint16_t quantity_of_registers,						/* 2 bytes,  N							*/
												uint8_t byte_count, 								/* 1 byte,   2 x N*           			*/
												uint16_t *ptr_registers_value,						/* 2*N bytes							*/
												modbus_exception_t *ptr_WSR_exception) {
/* ��:
 * slave_adress - ���� ������ �������;
 * starting_address         - 16-���� ������ ���'�� �������, ��������� � ��� �� �������� �������; 0�0000..0�FFFF, ��������� ����� � ������������� �������
 * quantity_of_registers    - 16-���� �������� ������� ������, �� �� ��������� ����������; 0�0000..007B (123),  ��������� ����� � ������������� �������
 * ptr_received_values		is a pointer to an array of unsigned 16 bit values that will be sent to be written into server memory
				The size of the array needs to be at least nbr_regs. Note that you can �cast� the unsigned values to signed values.
				As far as the Modbus protocol is concerned, it sends and receives 16 bit values and the interpretation of what these values mean
				is application specific.
 * *ptr_WSR_exception - �������� �� ������ ���� �������, ���� ���� ���� �������� ��� ������� � ��� ���������� ��������� ������� ��������.
 * � ��� �������� ���������� ������ �������� - ����������� MODBUS_SUCCESS, � ��� ������� - ����������� ��� �������, ���.������ mb.h
 *
 * */
	modbus_status_t mb_f_status = MODBUS_ERR_UNKNOWN;
	modbus_master_tx_msg_t w10 = {0};
	modbus_master_rx_msg_t rx10= {0};
	sprv_t sprv10 = {0};
	uint16_t crc_calc_tx10 = 0;
	(*ptr_WSR_exception) = 0;											/* 1. ������� ����� �����������: */
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
	crc_calc_tx10 = CRC_16x (w10.msg, 			    					/* 2. ���������� 16-����� CRC   									*/
				        w10.length -2*(sizeof(uint8_t)));
	w10.msg[w10.length-1]	= crc_calc_tx10 / 0x100;					/* 3. ������� ���� CRC �������� � ������� ���� ����������� 		*/
	w10.msg[w10.length-2]	= crc_calc_tx10 % 0x100;					/* 4. �������� ���� �RC ��������� � ������������� ���� ������������ */

	sprv10.timeout = 500; 												/* 5. ������ ��� ���������� ������ � ���������� ����� � ����������� ���������� ��������� */
	sprv10.waited_normal_response_length=8;  							/* 6. ������ ������� ��������� ������ ������ */
	sprv10.waited_error_response_length=5;								/* 7. ������ ������� ��������� ������ ��� �������, ������� �������� */
	sprv10.waited_slave_response_address=slave_address;
	sprv10.waited_slave_response_function=WRITE_MULTIPLE_REGISTERS;
	sprv10.ptr_tx = &w10;
    //	MB_Master_Transmit ();											/* 10. ³���������� �����������  */
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
		mb_f_status = MB_Master_Supervisor (&sprv10);					/* 11 ��������� ������� ������� � ������� ������ 				*/
	}
	if (mb_f_status==MODBUS_SUCCESS){									/* 12 ���� ������� ���������� ��� �������, � ���� �� ������� �� 	*/
																		/*    �������� ��������� �������, � �� ��� �������                	*/
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
	memset (&rx_msg,0,sizeof(rx_msg));          /* 12 ��������� ����� ������� ������ ������� �  */
												/*    � ���������� � ����� ���������� �������� ������� */
	master_state_5=Idle;
	return (mb_f_status);/* 12 ���� mb_f_status = MODBUS_SUCCESS ��:
	 	 	 	 	 	 ���� (*ptr_WSR_exception) = 0x00 �� ����� � ������� ������ �������� ������ ��������
	 	 	 	 	 	 ������ = ���. �������� modbus_exception_t , ������ mb.h
	 	 	 	 	 	       ���� mb_f_status != MODBUS_SUCCESS,���. �������� modbus_status_t, ������ mb.h */
}

/*********************************************************************************************************************************************
 *  								 P R I V A T E      F U N C T I O N S
**********************************************************************************************************************************************/

/*http://www.modbus.org/docs/Modbus_over_serial_line_V1_02.pdf
State "Idle" = no pending request. This is the initial state after power-up. A request can only be sent in "Idle" state. After sending
a request, the Master leaves the "Idle" state, and cannot send a second request at the same time
 When a unicast request is sent to a slave, the master goes into "Waiting for reply" state, and a �Response Time-out� is started. It
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
//	Idle,						/* ���������� ���������� ��������	*/
//	m_msg_transmitting,
//  m_msg_transmitting_ERROR,
//	m_msg_transmitting_Cplt,
//	Waiting_turnaround_delay,   /* ���������� ���� slaves ��������� �������� ������������ �����������, �������� ����� ���������� �� ����������� 	*/
//	Waiting_for_reply,			/* ���������� ������ �� �������� ����������� 			*/
//  m_msg_receiving,
//	m_msg_recived_RxIdle,
//	Processing_reply,			/* ������� ������				*/
//	Processing_error			/* ������� �������/����			*/
//};


static modbus_status_t MB_Master_Supervisor (sprv_t *obj) {
	volatile modbus_status_t MB_sv_status = MODBUS_ERR_UNKNOWN;

	/*
	 * ���� ��� �������� ������� ������ ���� ����������� ��������� ��������� ������.���������� master-5 � ���� ���������� ������
	 * ������� tw_txrxModbus_Session() ��������� �������� ����������� ������ ����� ���-����, ������������ ���-����� � ����� �������,
	 * ������ �����������, �������� ��� ����������� ��� ����� �� ��������

	 *
	 * ���� �������� ��� ������ �������:
	 * 		����� �� �����,
	 * 		����� - �� �������
	 * 		����� - �� ����������� ��� �������
	 *
	 * ������� ����������� � �� ������� MODBUS.
	 *   �������� ����������� � �������� ���������
	 *   struct sprv_t {										��� ���ί ����ֲ� ������
						uint8_t  waited_normal_response_length;       - ��������� ��������� ��������� ������ �볺��� (Master) �� ������� (Slave)
						uint8_t  waited_error_response_length;		  - ��������� ������� ����������� ��� ������� ��� �������� �� �������
						uint8_t  waited_slave_response_address;		  - ��������� ������ �������
						uint8_t  waited_slave_response_function;      - ���������� ��� ������� �� �������
						uint32_t transmitting_began_at;				  - ����� ����� ���� � ���������� �� ������ ������, �� ���������������
						modbus_exception_t *ptr_exception;			  - �������� �� ������ ���������� ���� ����������� ��� ������� �� �������

						};
//	 *������ ���������� �� ����, ���������� � ����� ����������� �������� ����������� �������� ������ ������ ������� obj->timeout �� ����� �볺���
//	 *		� �����䳿 � ��������� ������� ���������� �� ���� �볺��:
//	 *		void HAL_UART_RxIdleCallback(UART_HandleTypeDef *UartHandle)  ������� ����������� IDLE, ���� ��������� ������� ����������� �� �������
//			void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)  Set transmission flag: transfer complete - �� ���������������, ������
//			void HAL_UART_TxCpltCallback (UART_HandleTypeDef *UartHandle) ������� ����������� TxCplt ���� ���������� �������� ����������� �� �볺��� �������
//			void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)   ������� ����������� ��� ��������� ������ UART
//	 *
	 * */
	while(MB_sv_status == MODBUS_ERR_UNKNOWN){
		modbus_master_tx_msg_t *htx=NULL;
		modbus_master_rx_msg_t *hrx=NULL;
	    htx=obj->ptr_tx;
        hrx=obj->ptr_rx;

		/* 1. ���� ������� ���������� ����������� �� ������� ����������� �����������
		 * �� ����������� ����������� ��� �������
		 * �� ����� */
		if (!((hrx->length==obj->waited_error_response_length)||(hrx->length==obj->waited_normal_response_length))){
			MB_sv_status=MODBUS_ERR_BYTE_COUNT;
			 master_state_5=Idle;
			break;
		}
		/* 2. ���������� 16-����� CRC   									*/
		uint16_t crc_calc_rx = CRC_16x(hrx->msg,hrx->length -2);
		/* ���� CRC � ���������� �� ����������*/
		if (
			(hrx->msg[hrx->length-1] != crc_calc_rx / 0x100) ||
			(hrx->msg[hrx->length-2] != crc_calc_rx % 0x100)
		   ){
		   MB_sv_status = MODBUS_ERR_CRC;
		   master_state_5=Idle;
		   break;
		}
		/* 3. ���� ������ ��������� ����������� �� ������� ����� �������� ����������� */
		if(htx->msg[0]!=hrx->msg[0]){
			MB_sv_status=MODBUS_ERR_SLAVE_ADDR;
			break;
		}
		/* ���� �� ����������� ��� ������� (Exception), ��� �� �������
		 * ��� ����������� � ������ �����������
		 * ��������� ��� ������� (Exception)  */
		if((htx->msg[1]==(0x80+hrx->msg[1])) || (hrx->length==obj->waited_error_response_length)){
			MB_sv_status=MODBUS_ERR_fromSLAVE;
			obj->exception=hrx->msg[2];
			break;
		}
		/* ��� �� �� ��� ������� � �� ��� ������� */
		else if(((htx->msg[1]!=(hrx->msg[1])) || (hrx->length==obj->waited_normal_response_length))){
			MB_sv_status=MODBUS_ERR_FC;
			break;
		}
		/* �� ��������� ��������� �������*/
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
//		/* 0�03 	R E A D    H O L D I N G    R E G I S T E R S  																				   */
//		/*******************************************************************************************************************************************/
//		if (m_msg_received[1]==READ_HOLDING_REGISTERS){
//			if (m_msg_received_length == 8){
//				uint16_t Starting_Address = m_msg_received[2] * 0x100 + msg_received[3];
//				uint16_t Quantity_of_Registers=m_msg_received[4] * 0x100 + msg_received[5];
//				if (Quantity_of_Registers < 125){							/* 0x01, 0x03, 0xII, 0xII, 0x00, 0x07 0xCRCry 0xCRCrh*/
//					m_msg_transmitted [1]=READ_HOLDING_REGISTERS;             /* ������� ��� ������� �� ���������, �������� � ���� 1*/
//					uint8_t ByteCount=Quantity_of_Registers * 2;			/* ���������� ������� ����, �� ������ Master */
//					m_msg_transmitted [2] = ByteCount % 0x100;				/* �������� ������� ������������� ����, �������� � ���� 2 */
//							for (uint8_t ibc=3; ibc<=ByteCount+3; ++ibc){			/* � ����� ������� �������� ������� ����������� ������� */
//								m_msg_transmitted [ibc] = ibc;                        /* �������� ��� ��������� ������� !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
//							}
//					m_msg_transmitted_length=ByteCount+5;						/* ���������� ������� ���� � ����������-������ */
//				}															/* 0x01, 0x03, 0x0e, 0x03, 0x04, ..., 0x0d, 0x0e, 0x0f, 0x10, 0x11 0xCRC, 0xCRC */
//				else { /* Quantity of Registers is WRONG */
//					msg_transmitted[1] = READ_HOLDING_REGISTERS | 0x80;		/* ������� ���-������� exception-function_code �� ��������� + 0x80  */
//					msg_transmitted[2]	=  MODBUS_EXC_ILLEGAL_DATA_VALUE; 	/* �������� � ��������� ���� ��� ������� ����� � ���������� Modbus*/
//																			/* ��������� exception-function_code,  ���� ������� ����������� */
//					msg_transmitted_length=5;								/* ������������ ������� ����������� ��� �������		*/
//				}
//			}
//			else {/* msg_received_length is WRONG */
//				msg_transmitted[1] = READ_HOLDING_REGISTERS | 0x80;		/* ������� ���-������� exception-function_code �� ��������� + 0x80  */
//				msg_transmitted[2]	=  MODBUS_EXC_ILLEGAL_DATA_VALUE; 	/* �������� � ��������� ���� ��� ������� ����� � ���������� Modbus*/
//																		/* ��������� exception-function_code,  ���� ������� ����������� */
//				msg_transmitted_length=5;								/* ������������ ������� ����������� ��� �������		*/
//			}
//		} /* end of READ_HOLDING_REGISTERS*/
//		/*******************************************************************************************************************************************/
//		/* 0�06     W R I T E    S I N G L E    R E G I S T E R 																				   */
//		/*******************************************************************************************************************************************/
//		else if (m_msg_received[1]==WRITE_SINGLE_REGISTER){
//			if (m_msg_received_length == 8){
//
//				uint16_t Register_Address = m_msg_received[2] * 0x100 + msg_received[3];
//				uint16_t Register_Value   = m_msg_received[4] * 0x100 + msg_received[5];
//
//
//				/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
//				msg_transmitted [1]=WRITE_SINGLE_REGISTER;             /* ������� ��� ������� �� ���������, �������� � ���� 1*/
//
//				msg_transmitted [2] = Register_Address / 0x100;			/* �������� � ����� 2 ������� ����  										*/
//				msg_transmitted [3] = Register_Address % 0x100;			/* �������� � ����� 3 �������� ����  										*/
//				msg_transmitted [4] = Register_Value / 0x100;			/* �������� � ����� 4 ������� ����  										*/
//				msg_transmitted [5] = Register_Value % 0x100;			/* �������� � ����� 5 �������� ����  										*/
//
//			msg_transmitted_length=8;						/* ���������� ������� ���� � ����������-������ 									*/
//			}
//			else {/* msg_received_length is WRONG */
//				msg_transmitted[1] = WRITE_SINGLE_REGISTER | 0x80;		/* ������� ���-������� exception-function_code �� ��������� + 0x80  		*/
//				msg_transmitted[2]	=  MODBUS_EXC_ILLEGAL_DATA_VALUE; 	/* �������� � ��������� ���� ��� ������� ����� � ���������� Modbus		*/
//																		/* ��������� exception-function_code,  ���� ������� ����������� 			*/
//				msg_transmitted_length=5;								/* ������������ ������� ����������� ��� �������							*/
//			}
//		}
//		/*******************************************************************************************************************************************/
//		/* 0�08 � � � � � � � � � � � */
//		/*******************************************************************************************************************************************/
//		else if (msg_received[1]==DIAGNOSTIC ){                       					/* ���� �������� Modbus-������� 0�08 ĳ��������� 					*/
////			if 		(
////					((msg_received_length>=6) && (msg_received_length<125)) &&
////					(msg_received_length % 2 == 0) &&									/*   {0x01, 0x88, 0x00, 0x00, 0xCRCty, 0xCRCth}*/
////					((msg_received [2]==0x00) && (msg_received[3]==0x00))
////					) { 	/* � ���� �������� ��������� 0�0000 - Return Query Data  			*/																										/*   {0x01, 0x08, 0x00, 0x00, 0xCRCry, 0xCRCrh}*/
////						uint8_t N_TestData=(msg_received_length-6)/2;
////						msg_transmitted_length=6+(N_TestData*2);
////						msg_transmitted[1]=DIAGNOSTIC;									/*  ��,    ��������� �������� �����������, ���� ��������� �������  */
////						msg_transmitted[2]=0x00;										/*   {0x01, 0x88, 0x00, 0x00, 0xCRCty, 0xCRCth}*/
////						msg_transmitted[3]=0x00;
////						msg_transmitted_length=4+N_TestData*2+2;
////						if (N_TestData !=0){
////							for (uint8_t i_dgn=0;i_dgn<N_TestData*2;++i_dgn){
////								msg_transmitted[4+i_dgn]=msg_received[4+i_dgn];
////							}
////						}
////			}
////			else{																/* ���� �i������ ���� � ������������ ����� �����						*/
////																				/* ��� ���� ��������� �� 0000, � � ��� ���� �� ����������				*/
////																				/* ��� ����������� ������ ������� �����                  				*/
////						msg_transmitted[1] = DIAGNOSTIC | 0x80;		    		/* ������� ���-������� exception-function_code �� ��������� + 0�80  	*/
////						msg_transmitted[2]	=  MODBUS_EXC_ILLEGAL_DATA_VALUE; 	/* �������� � ��������� ���� ��� ������� ����� � ���������� Modbus	*/
////																				/* ��������� exception-function_code,  ���� ������� ����������� 		*/
////						msg_transmitted_length=5;								/* ������������ ������� ����������� ��� �������						*/
////					}
//
//		}
//		/*******************************************************************************************************************************************
//		 *  � � � � � � � � � �      � � �     � � � � � � �      � � � � � �
//		 ******************************************************************************************************************************************/
//		else {   /* ������� ���� ������� */                            /* ���� �� �������� ���������� ��� ������� Modbus: 					*/
////			msg_transmitted[1]=msg_received[1] | 0x80;				   /* ������� ���-������� exception-function_code �� ��������� + 0�80  */
////			msg_transmitted[2]	= MODBUS_EXC_ILLEGAL_FUNCTION;		   /* �������� � ��������� ���� ��� ������� ����� � ���������� Modbus*/
////			msg_transmitted_length=5;								   /* ������������ ������� ����������� ��� �������						*/
////			uint16_t crc_calc_tx = CRC_16x (msg_transmitted, 			    /* 2. ���������� 16-����� CRC   									*/
////						        msg_transmitted_length -2*(sizeof(uint8_t)));
////				msg_transmitted[msg_transmitted_length-1]	= crc_calc_tx / 0x100;	/* 3. ������� ���� CRC �������� � ������� ���� ����������� 		*/
////				msg_transmitted[msg_transmitted_length-2]	= crc_calc_tx % 0x100;	/* 4. �������� ���� �RC ��������� � ������������� ���� ������������ */
////				modbus_slave_state=answer_transmitting;
////				MB_Slave_Transmit ();											/* 5. ³���������� �����������                                     */
//		}
//		/************************************************************************************************************************************
//		 *  � � � � � � � � � �     � � � � � � � � �  ��  �����������, ���������� ��������� ���
//		 ************************************************************************************************************************************/
//
//
//		for (uint8_t izb=0;izb<msg_received_length;++izb){				/* ��������� ����� �������� ����������� */
////LCD_Printf ("0x%X%X ", 	msg_received [izb]/0x10, msg_received [izb]%0x10 );	/* debug only */
////			msg_received[izb]=0x00;
////		}
////																		/* ��������� ����� �������� ������������ */
////																		/* ������� ����������� - ������� ��� Master   					*/
////
////		msg_transmitted[0]=SLAVE_NODE_ADDRESS;                          /* 1. � ������/������� ���� - ������ Master - 0x00 					*/
////		uint16_t crc_calc_tx = CRC_16x (msg_transmitted, 			    /* 2. ���������� 16-����� CRC   									*/
////				        msg_transmitted_length -2*(sizeof(uint8_t)));
////		msg_transmitted[msg_transmitted_length-1]	= crc_calc_tx / 0x100;	/* 3. ������� ���� CRC �������� � ������� ���� ����������� 		*/
////		msg_transmitted[msg_transmitted_length-2]	= crc_calc_tx % 0x100;	/* 4. �������� ���� �RC ��������� � ������������� ���� ������������ */
////		modbus_slave_state=answer_transmitting;
////		MB_Slave_Transmit ();											/* 5. ³���������� �����������                                     */
////		/************************************************************************************************************************************
//		 * **********************************************************************************************************************************
//		 */
//	}   /*  � � � � � � �       � �     � � � � � � � � � �  � � � � � � � � � � �    */
//	/*****************************************************************************************************************************************/
//	else if (modbus_slave_state == broadcast_message ) {
//LCD_Printf("modbus_slave_state == broadcast_message\n");
//		/**************************************************************************************************************************************
//		 * 0�05  W R I T E _  S I N G L E _  C O I L
//		 *************************************************************************************************************************************/
//		if (msg_received[1]==WRITE_SINGLE_COIL){
//
//			/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
//LCD_Printf ("WRITE_SINGLE_COIL\n");
//			modbus_slave_state = no_message_for_us;
//		}
//		/*************************************************************************************************************************************/
//		else { 	/* ������� ���� ������� */                             /* ������� ������ ���� � �������� ����������, ��� ������������   */
//LCD_Printf ("FUNCTION CODE WRONG\n");								   /* ������ ���������� �� ��������� ������� � �����					 */
//			msg_received_length=0;
//HAL_Delay (5000);
//			modbus_slave_state=no_message_for_us;
//		}
//	/****************************************************************************************************************************************/
//		for (uint8_t izb=0;izb<msg_received_length;++izb){				/* ��������� ����� �������� ����������� */
//			msg_received[izb]=0;
//		}
//	}
//	else {}
//}




