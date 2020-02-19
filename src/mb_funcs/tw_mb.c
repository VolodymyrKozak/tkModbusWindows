/*
 * mb.c
 *
 *  Created on: 30 жовт. 2017 р.
 *      Author: Volodymyr.Kozak
 */
#include "../mb_funcs/tw_mb.h"

#include "../mb_funcs/RS_232/rs232.h"

extern int bdrate;       /* 9600 baud */
extern char mode[];
static modbus_status_t f_RS232_readMbMsg(
		int n, 	/* Очікувана кількість байт на вході RS232*/
		int comport_number,
		modbus_master_tx_msg_t *p_tx_msg,
		modbus_master_rx_msg_t *p_rx_msg,
		int msg_timeout);

/* Ця функція на основі вихідного повідомлення знаходить очікувану кількість байт
 * у вхідному повідомленні - відповіді Модбас */
static uint16_t  f_tw_txWaitedByteNumber(modbus_master_tx_msg_t *pmb_tx_msg);



modbus_status_t tw_txModbus_Session (
		int comport_number,
		modbus_master_tx_msg_t *p_tx_msg
		){

	volatile modbus_status_t mb_session_status = MODBUS_UNKNOWN_ERROR;
	while (mb_session_status == MODBUS_UNKNOWN_ERROR) {
	/* ВІДКРИТТЯ СОМ-порту */
		int ocom = RS232_OpenComportWrite(comport_number, bdrate, mode);
		if (ocom !=0){
			mb_session_status=MODBUS_COMPORT_ERR;
			break;
		}
	/* ВІДПРАВКА ПОВІДОМЛЕННЯ */
		int jn=0;
		jn=RS232_SendBuf(comport_number, p_tx_msg->msg, p_tx_msg->length);
//		Sleep(47);
		if (jn==p_tx_msg->length){

			mb_session_status=MODBUS_SUCCESS;
		}
		else{
			mb_session_status=MODBUS_ERR_TX;
			break;
		}
		/* Закриття СОМ-порту						*/
		RS232_CloseComport(comport_number);
		mb_session_status=MODBUS_SUCCESS;
	}
	return mb_session_status;
}



/***************************************************************************
 * Ця функціє виконує сесію RTS Modbus Master з Windows-комп'ютера.
 * Здійснюється:
 *  відкриття заданого COM-порту на передачу,
 *  відправка Модбас повідомлення,
 *  закриття заданого COM-порту на передачу
 *  відкриття заданого COM-порту на прийом
 *  в разі надходження відповіді від Slave - прийом відповіді
 *  закриття заданого СОМ-порту
 *  */
modbus_status_t tw_txrxModbus_Session (
		int comport_number,
		modbus_master_tx_msg_t *p_tx_msg,
		modbus_master_rx_msg_t *p_rx_msg,
		uint32_t msg_timeout
		){

	volatile modbus_status_t mb_session_status = MODBUS_UNKNOWN_ERROR;

	while (mb_session_status == MODBUS_UNKNOWN_ERROR) {
	/* ВІДКРИТТЯ СОМ-порту */
//		ocom = RS232_OpenComportWrite(comport_number, bdrate, mode);
		int ocom = -1;
		ocom = RS232_OpenComport(comport_number, bdrate, mode);
		//ocom = RS232_OpenComportWrite(comport_number, bdrate, mode);
		if (ocom !=0){
			/* Якщо не вдалося відкрити порт пробуємо його закрити*/
			BOOL ccom=RS232_vkCloseComport(comport_number);

			if(ccom==0){/* якщо не вдалося*/
				mb_session_status=MODBUS_COMPORT_ERR;
				break;
			}
			else{ /* якщо вдалося закрити -*/
				  /* попробувати ще раз відкрити */
				int ocom2 = RS232_OpenComport(comport_number, bdrate, mode);
				//int ocom2 = RS232_OpenComportWrite(comport_number, bdrate, mode);
				if(ocom2!=0){
					mb_session_status=MODBUS_COMPORT_ERR;
					break;
				}
			}

		}
	/* ВІДПРАВКА ПОВІДОМЛЕННЯ */
		int jn=0;
		jn=RS232_SendBuf(comport_number, p_tx_msg->msg, p_tx_msg->length);
		/* Закрити СОМ-порт, відкритий на передачу      */
		//RS232_CloseComport(comport_number);
		if (jn==p_tx_msg->length){
			mb_session_status=MODBUS_SUCCESS;
		}
		else{
			mb_session_status=MODBUS_ERR_TX;
			break;
		}

	    /* Отримання-очікування відповіді                        */
		/* Відкрити СОМ-порт на прийом                           */


		//ocom = RS232_OpenComportRead(comport_number, bdrate, mode);
		//if (ocom !=0){mb_session_status=MODBUS_COMPORT_ERR;break;}
//		Sleep(msg_timeout);
//
//		int n = RS232_PollComport(comport_number, p_rx_msg->msg, RX_MAX_MASTER_MSG_LENGTH);
//
//		if (n==0){
//			BOOL ccom=RS232_vkCloseComport(comport_number);
//			//RS232_CloseComport(comport_number);
//			mb_session_status=MODBUS_ERR_TIMEOUT;
//			break;
//		}
//		p_rx_msg->length=n;
//		mb_session_status=MODBUS_SUCCESS;

		/*************************************************************/
		int nbyteRx =  f_tw_txWaitedByteNumber(p_tx_msg);
		mb_session_status=f_RS232_readMbMsg(
				nbyteRx, /* number bytes to read from RS232*/
				comport_number,
				p_tx_msg,
				p_rx_msg,
				msg_timeout
				);
				/*******************************************************/
		/* Закриття СОМ-порту*/
		BOOL ccom=RS232_vkCloseComport(comport_number);
		if(ccom==0){/* якщо не вдалося*/
			mb_session_status=MODBUS_COMPORT_ERR;
			break;
		}
		//RS232_CloseComport(comport_number);
	}/*Вихід з WHILE*/


	return mb_session_status;
}

extern void f_tk5Logging(char *str, size_t n_str);
extern char tk5LogStr[512];
#define TK5_MODBUSRxQUANTITY_MS 3
static int msg_time_ms=0; /* Поточний час очікування/прийому повідомлення */
static char buff[300]={0};/* Буфер для накопичення повідомлення */

/* Ця функція - альтернатива отримання відповіді від сервера (slave)Модбас
 * на надісланий запит способом:
 * 		Sleep(msg_timeout);
 *  	int n = RS232_PollComport(comport_number, p_rx_msg->msg, RX_MAX_MASTER_MSG_LENGTH);
 *  	if (n==0){mb_session_status=MODBUS_ERR_TIMEOUT;break;},
 *  	який працює чудово, але вимагає значних витрат часу на очікування відповіді
 * Ця функція завершує роботу відразу по отриманню останніх байт повідомлення, що
 * разів в п'ять швидче*/
static modbus_status_t f_RS232_readMbMsg(
		int n, /* number bytes to read from RS232*/
		int comport_number,
		modbus_master_tx_msg_t *p_tx_msg,
		modbus_master_rx_msg_t *p_rx_msg,
		int msg_timeout){
	modbus_status_t chi=MODBUS_UNKNOWN_ERROR;
    if((n==0)||(p_tx_msg==NULL)||(p_rx_msg==NULL)){return MODBUS_WRONG_ARG;}

	char buff0[256]={0}; /* Буфер для прийому і тимчасового зберігання куска повідомлення Модбас */
	int l = n;           /* Очікувана кількість байт */
	int len=0;           /* Сумарно прийнята кількість байт */
	int kOld=0;          /* Кількість байт, прийнята за попередній крок */
	/* Допоки не прийняли передочікувану кількість байт */
	while (len<l){
		/* Кожні десять мікросекунд */
		Sleep(TK5_MODBUSRxQUANTITY_MS);
		/* Сумуємо час, щоб мати можливість вийти за таймаутом, якщо не дочекаємося */
		msg_time_ms+=TK5_MODBUSRxQUANTITY_MS;
		/* Опитуємо порт */
		int k = RS232_PollComport(comport_number, (uint8_t*)buff0, 256);
		/* Якщо отримали якусь кількість байт */
		if (k>0){
//			DbgCntr++;
//
//			if(DbgCntr==2){
//				DbgCntr=0;
//			}
			 /* Сумуємо байти всього */
			len+=k;
			/* Переписуємо отриману порцію байт в кінець загального буфера */
			memcpy(buff+kOld, buff0,k);
			/* Буфер для прийому куска повідомлення стираємо для прийому наступного куска */
			memset(buff0,0,sizeof(buff0));
			/* Перевіряємо, якщо це повідомлення сервера про помилку*/
			if(k>2){
				if(l==n){
					if((buff[1] | 0x80)==0x80){
						/* Очікуємо не інформаційну відповідь, а
						 * повідомлення про помилку, 5 байт у стандарті Модбас */
						l=5;
					}
				}
    		}
			/* Кількість байт, прийнята за попередній крок додаємо до поточної прийнятої кількості байт */
			kOld+=k;
    	}
		/* Контролюємо таймаут */
		if(msg_time_ms>msg_timeout){
			chi=MODBUS_ERR_TIMEOUT;
			break;}
		chi=MODBUS_SUCCESS;
    }   /* Вихід з циклу, якщо отримали очікуване число повідомлень */
	if(chi==MODBUS_SUCCESS){
		p_rx_msg->length=l;
		memcpy(p_rx_msg->msg,buff,l+1);

	}
	else{
		len=0;
	}
//	msg_time_ms=0; /* Поточний час очікування/прийому повідомлення */
	memset(&buff,0,sizeof(buff));/* Буфер для накопичення повідомлення */

//	/* Це все для комфорту відлагоджування */
	uint16_t Cmd = p_tx_msg->msg[1];
	uint16_t RegTx = p_tx_msg->msg[2]*0x100+p_tx_msg->msg[3];

	uint8_t ValNumTx=0;
	uint16_t RegRx=0;
	if(Cmd==0x03){/* Якщо команда 0х03 */

		uint16_t Quantity_of_RegistersTx=p_tx_msg->msg[5];
		/* Регістр, який або з якого починати читати */
		RegRx=p_rx_msg->msg[3]*0x100+p_rx_msg->msg[4];
		/* Кількість регістрів */
		ValNumTx=Quantity_of_RegistersTx;


	}
	/* Якщо 0х06 */

	if(Cmd==0x06){
		/* Куди писали */
		RegRx=p_tx_msg->msg[2]*0x100+p_tx_msg->msg[3];
		/* Що писали   */
		ValNumTx=p_tx_msg->msg[4]*0x100+p_tx_msg->msg[5];

	}
	snprintf(tk5LogStr,100,"0x%1X 0x%04X  %d ms, err %d",
			Cmd, RegRx, /*ValNumTx, RegRx,*/ msg_time_ms, (int)chi);
	f_tk5Logging(tk5LogStr, sizeof(tk5LogStr));
	msg_time_ms=0;
	memset(buff,0,sizeof(buff));
    return chi;




}


/* Ця функція на основі вихідного повідомлення
 * знаходить очікувану кількість байт у вхідному повідомленні
 * -відповіді Модбас */
static uint16_t  f_tw_txWaitedByteNumber(
		modbus_master_tx_msg_t *pmb_tx_msg
		){
	uint8_t MB_Cmnd = pmb_tx_msg->msg[1];
	int n=0;
	switch(MB_Cmnd){
	case 0x03:{
		uint8_t MB03NReg=pmb_tx_msg->msg[5];
		if(MB03NReg==1){
			n=7;
		}
		else{
			n=5+MB03NReg*2;
		}
	}
	break;
	case 0x06:{
		n=8;
	}
	break;
	case 0x10:{
		n=8;
	}
	break;
	case 0x11:{

		n=10;/* Для ТК5 */
	}
	break;
	default:{}
	}
	return n;
}
