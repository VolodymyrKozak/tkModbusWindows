/*
 * mb_master5.h
 *
 *  Created on: 30 ����. 2017 �.
 *      Author: Volodymyr.Kozak
 */

#ifndef MB_MASTER5_H_
#define MB_MASTER5_H_
#include <stdint.h>
#include "../wConfig.h"
#include "../mb_funcs/tw_mb.h"



typedef enum  {
	Idle,						/* ���������� ���������� ��������	*/
	m_msg_transmitting,
	m_msg_transmitting_ERROR,
	m_msg_transmitting_Cplt,
	Waiting_turnaround_delay,   /* ���������� ���� slaves ��������� �������� ������������ �����������, �������� ����� ���������� �� ����������� 	*/
	Waiting_for_reply,			/* ���������� ������ �� �������� ����������� 			*/
	m_msg_receiving,
	m_msg_receiving_ERROR,
	m_msg_recived_RxIdle,
	Processing_reply,			/* ������� ������				*/
	Processing_error			/* ������� �������/����			*/
}master_state_5_t;

/*****************************************************************************************************************************************
 *****************************************************************************************************************************************
 *    P U B L I C S     F U N C T I O N S   P R O T O T Y P E S
 *****************************************************************************************************************************************
 ******************************************************************************************************************************************/


/*******************************************************************************************************************************************/
/* 0�06     W R I T E    S I N G L E    R E G I S T E R 																				   */
/*******************************************************************************************************************************************/
/* �� ������� ����������� �� ������ ���������� (Master) ������������,��� �������� � ������� ������ ������� (Slave) ��������� ��� ��������
 * */
modbus_status_t MB_Write_Single_Register (	uint8_t slave_address,
											uint16_t reg_address,
											uint16_t reg_value,
											modbus_exception_t *ptr_WSR_exception);
/* ��:
 * slave_adress - ���� ������ �������;
 * reg_address  - 16-���� ������ ���'�� �������, ���� �� �������� ���������� ��� ��������;
 * reg_value    - 16-���� �������� ��������� � ��������� ������, ��� �� �������� � ������;
 * *ptr_WSR_exception - �������� �� ������ ���� �������, ���� ���� ���� �������� ��� ������� � ��� ���������� ��������� ������� ��������.
 * � ��� �������� ���������� ������ �������� - ����������� MODBUS_SUCCESS, � ��� ������� - ����������� ��� �������, ���.������ mb.h
 *
 * */
/* 12 ���� mb_f_status = MODBUS_SUCCESS ��:
	 	 	 	 	 	 ���� (*ptr_WSR_exception) = 0x00 �� ����� � ������� ������ �������� ������ ��������
	 	 	 	 	 	 ������ = ���. �������� modbus_exception_t , ������ mb.h
	 	 	 	 	 	       ���� mb_f_status != MODBUS_SUCCESS,���. �������� modbus_status_t, ������ mb.h */

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
								modbus_exception_t *ptr_WSR_exception);
/* ��:
 * slave_adress - ���� ������ �������;
 * *ptr_WSR_exception - �������� �� ������ ���� �������, ���� ���� ���� �������� ��� ������� � ��� ���������� ��������� ������� ��������.
 * � ��� �������� ���������� ������ ������� - ����������� MODBUS_SUCCESS, � ��� ������� - ����������� ��� �������, ���.������ mb.h
 *
 * */

/* 12 ���� mb_f_status = MODBUS_SUCCESS ��:
	 	 	 	 	 	 ���� (*ptr_WSR_exception) = 0x00 �� ��'���� Master->Slave->Master - �������,
	 	 	 	 	 	 ������ = ���. �������� modbus_exception_t , ������ mb.h
	 	 	 	 	 	       ���� mb_f_status != MODBUS_SUCCESS,���. �������� modbus_status_t, ������ mb.h */


/*******************************************************************************************************************************************/
/* 0�03 	R E A D    H O L D I N G    R E G I S T E R S  																				   */
/*******************************************************************************************************************************************/

/* �� ������� ����������� �� ������ ���������� (Master) ������������,��� ������� � ���'�� ������� ������ ������ 16-����� �������
 * ��������� � ������ ������� ������� ������ ������� (Slave) ��������� ��� ��������
 * */
modbus_status_t MB_Read_Holding_Registers (	uint8_t slave_address,
											uint16_t starting_address,
											uint16_t quantity_of_registers,
											uint16_t *ptr_received_values,
											modbus_exception_t *ptr_WSR_exception);
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
/* 12 ���� mb_f_status = MODBUS_SUCCESS ��:
	 	 	 	 	 	 ���� (*ptr_WSR_exception) = 0x00 �� ����� � ������� ������ �������� ������ ��������
	 	 	 	 	 	 ������ = ���. �������� modbus_exception_t , ������ mb.h
	 	 	 	 	 	       ���� mb_f_status != MODBUS_SUCCESS,���. �������� modbus_status_t, ������ mb.h */


/**************************************************************************************************************************************
* 0�05  W R I T E _  S I N G L E _  C O I L
*************************************************************************************************************************************/



/* This function code is used to write a single output to either ON or OFF in a remote device */

modbus_status_t MB_Write_Syngle_Coil (	    uint8_t slave_address,
											uint16_t output_address,
											uint16_t output_value,
											modbus_exception_t *ptr_WSR_exception);
/* ��:
 * slave_adress - ���� ������ �������;
 * reg_address  - 16-���� ������ ���'�� �������, ���� �� �������� ���������� ��� ��������;
 * reg_value    - 16-���� �������� ��������� � ��������� ������, ��� �� �������� � ������;
 * *ptr_WSR_exception - �������� �� ������ ���� �������, ���� ���� ���� �������� ��� ������� � ��� ���������� ��������� ������� ��������.
 * � ��� �������� ���������� ������ �������� - ����������� MODBUS_SUCCESS, � ��� ������� - ����������� ��� �������, ���.������ mb.h
 *
 * */
/* 12 ���� mb_f_status = MODBUS_SUCCESS ��:
	 	 	 	 	 	 ���� (*ptr_WSR_exception) = 0x00 �� ����� � ������� ������ �������� ������ ��������
	 	 	 	 	 	 ������ = ���. �������� modbus_exception_t , ������ mb.h
	 	 	 	 	 	       ���� mb_f_status != MODBUS_SUCCESS,���. �������� modbus_status_t, ������ mb.h */

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


/*********************************************************************************************************************************************
 *  								 P R I V A T E      F U N C T I O N S
**********************************************************************************************************************************************/

//static modbus_status_t MB_Master_Supervisor (sprv_t *obj);

	/*
	 * ϳ��� �������� �������� ������� (���� ��������� HAL_UART_TxCpltCallback) ���������� master-5 � ���� ���������� ������
	 * � �������� ���� ���� ���������� ������
	 *
	 * ���� �������� ��� ������ ������� (�������� ������: ���� ��������� � HAL_UART_RxIdleCallback
	 * 	��������� ��������� �������, ���������� �������� �������
	 *
	 * ����� �� �����,
	 *
	 * ����� - �� �������
	 *
	 * ����� - �� ����������� ��� �������
	 *
	 * ������� ����������� � �� ������� MODBUS.
	 *   �������� ����������� � �������� ���������
	 *   struct sprv_t {										��� ���ί ����ֲ� ������
						uint8_t  waited_normal_response_length;       - ��������� ��������� ��������� ������ �볺��� (Master) �� ������� (Slave)
						uint8_t  waited_error_response_length;		  - ��������� ������� ����������� ��� ������� ��� �������� �� �������
						uint8_t  waited_slave_response_address;		  - ��������� ������ �������
						uint8_t  waited_slave_response_function;      - ���������� ��� ������� �� �������
						uint32_t transmitting_began_at;				  - ����� ����� ���� � ���������� �� ������ ������
						modbus_exception_t *ptr_exception;			  - �������� �� ������ ���������� ���� ����������� ��� ������� �� �������

						};
	 *������ ���������� �� ����, ���������� � ����� ����������� �������� ����������� �������� ������ ������ ������� obj->timeout �� ����� �볺���
	 *		� �����䳿 � ��������� ������� ���������� �� ���� �볺��:
	 *		void HAL_UART_RxIdleCallback(UART_HandleTypeDef *UartHandle)  ������� ����������� IDLE, ���� ��������� ������� ����������� �� �������
			void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)  Set transmission flag: transfer complete - �� ���������������, ������
			void HAL_UART_TxCpltCallback (UART_HandleTypeDef *UartHandle) ������� ����������� TxCplt ���� ���������� �������� ����������� �� �볺��� �������
			void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)   ������� ����������� ��� ��������� ������ UART
	 *
	 * */

/*********************************************************************************************************************************************
 * *******************************************************************************************************************************************
 * ********************************************************************************************************************************************
 */
/*
� 0x01 (Read Coils Status)
� 0x05 (Write Single Coil)
� 0x03 (Read Holding Registers)
� 0x06 (Write Single Register)
� 0x10 (Write Multiple Registers)
*/




#endif /* MB_MASTER5_H_ */
