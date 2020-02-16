/*
 * mb.h
 *
 *  Created on: 24 жовт. 2017 р.
 *      Author: Volodymyr.Kozak
 */
/*Библиотека MODBUS (MB)
 *
 * Файл mb - пільний для Modbus Master і Modbus Slave,
 * mb_slave5, mb_master5 	- реалізація верхнього рівня протоколу Модбас, фактично - логіки модбас для клієнта/сервера відповідно
 *
 * mb_slave,mb_master		- реалізація протоколу RTU
 *
 *
 * */
#ifndef MB_H_
#define MB_H_
#include <stdint.h>
extern int cport_nr;

#define HUART4_RxTx_SWITCH GPIOA,GPIO_PIN_15
#define HUART5_RxTx_SWITCH GPIOB,GPIO_PIN_3
#define SLAVE_BROADCAST_ADDRESS 0x00

/* закоментувати один дефайн або жодного дефайна  */
#define MODBUS_RTU_MASTER
#define MODBUS_RTU_SLAVE


#ifdef MODBUS_RTU_MASTER

#define MASTER_HUART huart5
#define MASTER_HUART_RxTx_SWITCH HUART5_RxTx_SWITCH

#endif

#ifdef MODBUS_RTU_SLAVE

#define SLAVE_NODE_ADDRESS      0x01
#define SLAVE_HUART  			huart4
#define SLAVE_HUART_RxTx_SWITCH HUART4_RxTx_SWITCH

#endif




#define RX_MAX_SLAVE_MSG_LENGTH 125
#define TX_MAX_SLAVE_MSG_LENGTH 255

#define TX_MAX_MASTER_MSG_LENGTH 125
#define RX_MAX_MASTER_MSG_LENGTH 255

extern uint8_t volatile m_msg_received[TX_MAX_SLAVE_MSG_LENGTH];
extern uint8_t volatile m_msg_received_length;

typedef enum{
  TW_OK       = 0x00U,
  TW_ERROR    = 0x01U,
  TW_BUSY     = 0x02U,
  TW_TIMEOUT  = 0x03U
} TW_StatusTypeDef;



TW_StatusTypeDef UartHandle;
/*
 *  Created on: 23 вер. 2017 р.
 *  	http://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf
 *  	http://www.modbus.org/docs/Modbus_over_serial_line_V1_02.pdf
 *      http://www.simplymodbus.ca/Enron.htm
    	http://libmodbus.org/docs/v3.0.6/

*/
/*
Supported Register Function codes:
===============================================================
Function Code  Action	         Table Name
===============================================================
04 (0x04 hex)	   Read	             Analog Input Registers					later
***********************************************************************************************
03 (0x03 hex)	   Read	Analog       Output Holding Registers				under realization
06 (0x06 hex)	   Write single	     Analog Output Holding Register			under realization
***********************************************************************************************
16 (0x10 hex)	   Write multiple	 Analog Output Holding Registers		later
================================================================

Supported Coil Function codes:
================================================================
Function Code	Action			Table Name
02 (0x02 hex)		Read			Discrete Input Contacts					later
01 (0x01 hex)		Read			Discrete Output Coils					later
***********************************************************************************************
05 (0x05 hex)		Write single	Discrete Output Coil					under realization
***********************************************************************************************
15 (0x0F hex)		Write multiple	Discrete Output Coils					later

Diagnostic
================================================================
***********************************************************************************************
08 (0x08 hex)		Diagnostic 												under realization
***********************************************************************************************
*/


#define READ_HOLDING_REGISTERS    0x03
#define WRITE_SINGLE_COIL         0x05
#define WRITE_SINGLE_REGISTER     0x06
#define DIAGNOSTIC                0x08
#define WRITE_MULTIPLE_REGISTERS  0x10
#define tk4forPC_Master			  0x77

//#define MODBUS_FC_READ_EXCEPTION_STATUS     0x07
//#define MODBUS_FC_WRITE_MULTIPLE_COILS      0x0F
//#define MODBUS_FC_WRITE_MULTIPLE_REGISTERS  0x10
//#define MODBUS_FC_REPORT_SLAVE_ID           0x11
//#define MODBUS_FC_MASK_WRITE_REGISTER       0x16
//#define MODBUS_FC_WRITE_AND_READ_REGISTERS  0x17



/* Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 1 page 12)
 * Quantity of Coils to read (2 bytes): 1 to 2000 (0x7D0)
 * (chapter 6 section 11 page 29)
 * Quantity of Coils to write (2 bytes): 1 to 1968 (0x7B0)
 */
//#define MODBUS_MAX_READ_BITS              2000
//#define MODBUS_MAX_WRITE_BITS             1968

/* Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 3 page 15)
 * Quantity of Registers to read (2 bytes): 1 to 125 (0x7D)
 * (chapter 6 section 12 page 31)
 * Quantity of Registers to write (2 bytes) 1 to 123 (0x7B)
 * (chapter 6 section 17 page 38)
 * Quantity of Registers to write in R/W registers (2 bytes) 1 to 121 (0x79)
 */
#define MODBUS_MAX_READ_REGISTERS          125
#define MODBUS_MAX_WRITE_REGISTERS         123
#define MODBUS_MAX_WR_WRITE_REGISTERS      121
#define MODBUS_MAX_WR_READ_REGISTERS       125

/* The size of the MODBUS PDU is limited by the size constraint inherited from
 * the first MODBUS implementation on Serial Line network (max. RS485 ADU = 256
 * bytes). Therefore, MODBUS PDU for serial line communication = 256 - Server
 * address (1 byte) - CRC (2 bytes) = 253 bytes.
 */
#define MODBUS_MAX_PDU_LENGTH              253

/* Consequently:
 * - RTU MODBUS ADU = 253 bytes + Server address (1 byte) + CRC (2 bytes) = 256
 *   bytes.
 * - TCP MODBUS ADU = 253 bytes + MBAP (7 bytes) = 260 bytes.
 * so the maximum of both backend in 260 bytes. This size can used to allocate
 * an array of bytes to store responses and it will be compatible with the two
 * backends.
 */
#define MODBUS_MAX_ADU_LENGTH              260

/* Random number to avoid errno conflicts */
#define MODBUS_ENOBASE 112345678

/* Повідомлення для Modbus на передачу   */
typedef struct  {
	/* кількість байтів в повідоленні    */
	uint8_t length;
	/* послідовність байтів повідомлення */
	uint8_t msg [TX_MAX_MASTER_MSG_LENGTH+1];
}modbus_master_tx_msg_t;

/* Повідомлення для Modbus на прийом     */
typedef struct  {
	/* кількість байтів в повідоленні    */
	uint8_t length;
	/* послідовність байтів повідомлення */
	uint8_t msg [RX_MAX_MASTER_MSG_LENGTH+1];
}modbus_master_rx_msg_t;
typedef enum { /* це статус, що повертається функцією*/

	MODBUS_UNKNOWN_ERROR,
	MODBUS_SUCCESS,	   		/*	if the call was successful.	  											*/
	MODBUS_ERR_RX,          /*  if the message wasn't receive                                        	*/
	MODBUS_ERR_TX,			/*  if the message wasn't sent			                                	*/
	MODBUS_ERR_TIMEOUT,		/*	if a response was not received from the slave within the timeout specified for this channel (see MB_CfgCh()).	*/
	MODBUS_ERR_SLAVE_ADDR,	/*  If the transmitted slave address doesn't correspond to the received slave address								*/
	MODBUS_ERR_CRC,
	MODBUS_ERR_FC,			/*  If the transmitted function code doesn't correspond to the received function code								*/
	MODBUS_ERR_BYTE_COUNT,	/*  If the expected number of bytes to receive doesn't correspond to the number of bytes received					*/
	MODBUS_ERR_DIAG,		/*  If you specified an invalid diagnostic function code (i.e. not one of the function described in  argument).		*/
	MODBUS_ZERO_POINTER,
	MODBUS_MASTER_BUSY,
	MODBUS_WRONG_ARG,
	MODBUS_ERR_UNKNOWN,
	MODBUS_ERR_fromSLAVE,

	MODBUS_COMPORT_ERR
} modbus_status_t;

modbus_status_t tw_txModbus_Session (
		int comport_number,
		modbus_master_tx_msg_t *p_tx_msg
		);
/* Ця функціє виконує сесію RTS Modbus Master з Windows-комп'ютера.
* Здійснюється:
*  відкриття заданого COM-порту на передачу,
*  відправка Модбас повідомлення,
*  закриття заданого COM-порту на передачу
*  відкриття заданого COM-порту на прийом
*  очікування протягом заданого часу
*  в разі надходження відповіді від Slave - прийом відповіді
*  закриття заданого СОМ-порту
*  */
modbus_status_t tw_txrxModbus_Session (
		int comport_number,
		modbus_master_tx_msg_t *p_tx_msg,
		modbus_master_rx_msg_t *p_rx_msg,
		uint32_t msg_timeout
		);



typedef enum {
    MODBUS_F_SUCCESS,
/* Protocol exceptions, згідно з стандартом протоколу MODBUS Application Protocol Specification */
/*
MODBUS Exception Codes

Code 	Name 						Meaning*/
	MODBUS_EXC_ILLEGAL_FUNCTION, /*
01 		ILLEGAL FUNCTION 			The function code received in the query is not an
									allowable action for the server. This may be
									because the function code is only applicable to
									newer devices, and was not implemented in the
									unit selected. It could also indicate that the server
									is in the wrong state to process a request of this
									type, for example because it is unconfigured and
									is being asked to return register values.*/
    MODBUS_EXT_ILLEGAL_DATA_ADDRESS, /*
02 		ILLEGAL DATA ADDRESS 		The data address received in the query is not an
									allowable address for the server. More
									specifically, the combination of reference number
									and transfer length is invalid. For a controller with
									100 registers, the PDU addresses the first register
									as 0, and the last one as 99. If a request is
									submitted with a starting register address of 96
									and a quantity of registers of 4, then this request
									will successfully operate (address-wise at least)
									on registers 96, 97, 98, 99. If a request is
									submitted with a starting register address of 96
									and a quantity of registers of 5, then this request
									will fail with Exception Code 0x02 “Illegal Data
									Address” since it attempts to operate on registers
									96, 97, 98, 99 and 100, and there is no register
									with address 100.*/
    MODBUS_EXC_ILLEGAL_DATA_VALUE,/*
03 		ILLEGAL DATA VALUE 			A value contained in the query data field is not an
									allowable value for server. This indicates a fault in
									the structure of the remainder of a complex
									request, such as that the implied length is
									incorrect. It specifically does NOT mean that a
									data item submitted for storage in a register has a
									value outside the expectation of the application
									program, since the MODBUS protocol is unaware
									of the significance of any particular value of any
									particular register.*/
    MODBUS_EXC_SLAVE_OR_SERVER_FAILURE,/*
04 		SERVER DEVICE FAILURE 		An unrecoverable error occurred while the server
									was attempting to perform the requested action.*/
    MODBUS_EXC_ACKNOWLEDGE,/*
05 		ACKNOWLEDGE 				Specialized use in conjunction with programming
									commands.
									The server has accepted the request and is
									processing it, but a long duration of time will be
									required to do so. This response is returned to
									prevent a timeout error from occurring in the
									client. The client can next issue a Poll Program
									Complete message to determine if processing is
									completed.*/
    MODBUS_EXC_SLAVE_OR_SERVER_BUSY,/*
06 		SERVER DEVICE BUSY 			Specialized use in conjunction with programming
									commands.
									The server is engaged in processing a long–
									duration program command. The client should
									retransmit the message later when the server is
									free.*/
	   MODBUS_EXC_NEGATIVE_ACKNOWLEDGE,
	   MODBUS_EXC_MEMORY_PARITY,/*
08 		MEMORY PARITY ERROR 		Specialized use in conjunction with function codes
									20 and 21 and reference type 6, to indicate that
									the extended file area failed to pass a consistency
									check.
									The server attempted to read record file, but
									detected a parity error in the memory. The client
									can retry the request, but service may be required*/
	    MODBUS_EXC_NOT_DEFINED,
	    MODBUS_EXC_GATEWAY_PATH,/*
0A 		GATEWAY PATH UNAVAILABLE 	Specialized use in conjunction with gateways,
									indicates that the gateway was unable to allocate
									an internal communication path from the input port
									to the output port for processing the request.
									Usually means that the gateway is misconfigured
									or overloaded.*/
		MODBUS_EXC_GATEWAY_TARGET,/*
0B 		GATEWAY TARGET DEVICE		Specialized use in conjunction with gateways,
FAILED TO RESPOND					indicates that no response was obtained from the
									target device. Usually means that the device is not
									present on the network.*/
    MODBUS_EXC_MAX
} modbus_exception_t;


/************************************************************************************************************************************************
 *           					     V A R I A B L E S
 * **********************************************************************************************************************************************
 */
typedef struct {
	uint32_t timeout;                         	/* час очікування клієнтом відповіді сервера													*/
	uint8_t  waited_normal_response_length;		/* очікувана довжина в байтах нормальної віповіді сервера на запит клієнта						*/
	uint8_t  waited_error_response_length;		/* очікувана довжина в байтах відповіді сервера про помилку при обробці повідомлення 			*/
	uint8_t  waited_slave_response_address;		/* адреса сервера: 0x00 - циркулярний, для функцій запису даних в сервер, 0х00..0хFF-вибірковий */
	uint8_t  waited_slave_response_function;	/* код функції 0x03,0x05,0x06,0x08															    */
	uint32_t transmitting_began_at;				/* початок відліку часу виконання функції Modbus												*/
	modbus_exception_t exception;				/* код помилки, виявленої сервером									*/
	modbus_master_tx_msg_t *ptr_tx;				/* показчик на адресу пам'яті буфера повідомлення, що передається	*/
	modbus_master_rx_msg_t *ptr_rx;				/* показчик на адресу пам'яті буфера повідомлення, що приймається   */
}sprv_t ;



/*
 *
 *
 *
https://doc.micrium.com/pages/viewpage.action?pageId=10753125&preview=/10753125/11013986/100-uC-Modbus-001.pdf


Швидксть передачі і таймаути:
При швидкості 9600 біт/сек, придблизно передається 1 байт в мс, при швидкості 19.2 кбіт/сек - 2 байти в м/с
Стандарт modbus RTU допускає затримку не більше 1.5 часу передачі байта між байтами та передбачає 3.5 м.с перед початком
та закінченням передачі повідомлення.

Тобто затримка при передачі повідомлення може розраховуватися, взявши за основу у час передачі байта - 1мс (1 байтосекунда):
     (кількість байтів напередачу * 2.5) байтосек + 7байтосек, все це варто помножити на коефіцієнт, наприклад 2.0
На прийом - так же
Очікування відповіді на прикладному рівні.
час передачі + час прийому * 2.., буде досить. Час обробки - можна ігнорувати.

*/






#endif /* MB_H_ */
