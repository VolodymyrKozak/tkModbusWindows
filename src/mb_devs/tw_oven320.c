/*
 * tk_oven_smi2.c
 *
 *  Created on: 23 трав. 2018 р.
 *      Author: KozakVF
 */

#include "tw_oven_smi2.h"
#include "tw_oven320.h"
#include <stdio.h>
#include "stdint.h"
#include "windows.h"
#include "../mb_funcs/tkRS232rx.h"
#include "../mb_funcs/tw_mb.h"
//#include "../mb_funcs/tw_mb.h"
#ifdef   WINDOWS_MODBUS_MASTER_for_OVEN_INDICATOR


#define MB_320_ADDRESS 01



	  extern int cport_nr;
//https://owen.ua/uploads/104/re_smi2_2-ru-54088-1.2_a4.pdf
/*********************************************************************************************************************
 *  ОГОЛОШЕННЯ СТАТИЧНИХ ФУНКЦІЙ
 *********************************************************************************************************************/
uint32_t debugbbb=0;
uint16_t registers320[300]={0};
//uint8_t mbRxMsg[256]={0};
int8_t nRxMsg=0;
modbus_master_tx_msg_t RspTxMsg = {8,{0x01,0x03,0x00,0x03,0x00,0x01}};
modbus_master_rx_msg_t  RspRxMsg = {0};

static SYSTEMTIME SystemTime={0};
static uint16_t st[200]={0};
static uint32_t ist=0;
static uint16_t AMPERAGE_MAX=(uint16_t)(20.0f*10.0f);
static uint16_t AMPERAGE_MIN=(uint16_t)(2.0f*10.0f);
op320_registers_t rr={0};
uint16_t WriteSet[5]={100, /* Макс уставка струму з флеш * 10 */
		              20,  /* Мін уставка струму з флеш  * 10 */
					  1,   /* Авт + Самозапуск */
					  0    /* Подача           */
};
uint16_t BuffSet[5]={0};   /* Макс уставка струму з флеш * 10 */
						   /* Мін уставка струму з флеш  * 10 */
					       /* Авт + Самозапуск */
					       /* Подача           */

uint16_t Screen_1[5]={0,   /*  0х0000 - выкл, 0х0001 - вкл */
		              0,   /*  Код аварії тк2  */
					  1,   /*  0-подача, 1 відкачка  */
					  0    /*  Рівень рідини 0х0000 = 0%, 0хFFFF 100%          */
};

/* Iніціалізація ор320,
 * введення в регістри op320 уставок з флеш ТК2*/
oven_session_status_t f_oven320_Init(void){
	/* Це болванка, взяти після читання флеш                      */
	oven_session_status_t ot=oven_UnknownERR;
	ot= f_ovenSMI2_TASKS_16 (
			MB_320_ADDRESS,   		   /* Модбас-адреса прилада                                       */
			16,						   /* Регістр, з якого починати писати - 0х0000 .. 0хFFFF         */
			5,   					   /* Кількість регістрів, які треба писати 0x0001 to 0x007B(123) */
			10,                        /* Кількість байт, як треба писати   2*  0x01  to 0x7B(246)    */
			WriteSet         /* Масив байт або (uint8_t*) з масивом 16-значних числел       */
	    );

	/* Скидання прапорів натискання клавіш */
	ot=f_ovenSMI2_TASKS_06 (
			MB_320_ADDRESS,
			15,               /* register */
			0                /* value    */
			);
	/* Примусовий перехід на головний екран */
	ot=f_ovenSMI2_TASKS_06 (
			MB_320_ADDRESS,
			9,               /* register */
			1                /* value    */
			);
	return oven_ResponseSent;
}
static uint16_t ScreenN=0;
static uint16_t OldScreen=0;
static uint16_t FlagFixSet=0;
oven_session_status_t f_oven320_Loop(void){
	oven_session_status_t ot=oven_UnknownERR;
	/* Читаємо номерер екрану */

	ot = f_ovenSMI2_TASKS_03_1 (
			MB_320_ADDRESS,      /* Модбас-адреса прилада    */
			0x0A,    /* Адреса регістра Модбас   */
			&ScreenN /* зчитате з регістра число */
			);

	switch(ScreenN){
			case 1:{ /* Головний екран */

				/* Постійно або якщо щось в масиві змінилося обновляємо  стан екрана  */

				ot= f_ovenSMI2_TASKS_16 (
						MB_320_ADDRESS,   		   /* Модбас-адреса прилада                                       */
						0,						   /* Регістр, з якого починати писати - 0х0000 .. 0хFFFF         */
						5,   					   /* Кількість регістрів, які треба писати 0x0001 to 0x007B(123) */
						10,                        /* Кількість байт, як треба писати   2*  0x01  to 0x7B(246)    */
						Screen_1         /* Масив байт або (uint8_t*) з масивом 16-значних числел       */
				    );
				break;
			}
			case 2:{
				/* Це екран-диспетчер, нічого не робимо */
			}
			break;
			case 3:{
				oven_session_status_t rs=f_ovenSMI2_TASKS_03 (
						MB_320_ADDRESS,      /* Модбас-адреса прилада    */
						16,    /* Адреса початкового регістра Модбас   */
						2,
						&BuffSet /* зчитані з регістрів число */
						);
				/* Якщо струм вище верхнього граничного - примосуво записуємо в регістр граничний */
				/* Примусовий перехід на головний екран */
				if(BuffSet[0]>AMPERAGE_MAX){
					ot=f_ovenSMI2_TASKS_06 (
						MB_320_ADDRESS,
						16,               /* register */
						AMPERAGE_MAX                /* value    */
						);
				}
				/* Якщо струм нище нижнього граничного - примосуво записуємо в регістр граничний */
				if(BuffSet[1]<AMPERAGE_MIN){
					ot=f_ovenSMI2_TASKS_06 (
						MB_320_ADDRESS,
						17,               /* register */
						AMPERAGE_MIN                /* value    */
						);
				}
			}
			case 4:
			case 5:{
				/* Користувач копирсається в налаштуваннях, зчитуємо все, що він напрацював в буфер */

				oven_session_status_t rs=f_ovenSMI2_TASKS_03 (
						MB_320_ADDRESS,      /* Модбас-адреса прилада    */
						16,    /* Адреса початкового регістра Модбас   */
						5,
						&BuffSet /* зчитані з регістрів число */
						);

			}
			break;
			case 10:{
				/* Користувач вирішує, що робити з налаштуваннями */
				uint16_t Esc_or_Ent={0};
				ot = f_ovenSMI2_TASKS_03_1 (
						MB_320_ADDRESS,      /* Модбас-адреса прилада    */
						0x0F,    /* Адреса регістра Модбас   */
						&Esc_or_Ent /* зчитате з регістра число */
						);
				if(Esc_or_Ent==0x0001){/* Enter */
					/* Якщо це не повтор */
//					if (OldScreen!=ScreenN){
						if(FlagFixSet==0){
						/* Остаточно зчитуємо все, що там налаштував користувач,
						 *  зчитуємо все, що він напрацював в буфер */

						oven_session_status_t rs=f_ovenSMI2_TASKS_03 (
								MB_320_ADDRESS,      /* Модбас-адреса прилада    */
								16,    /* Адреса початкового регістра Модбас   */
								5,
								&BuffSet /* зчитані з регістрів число */
								);
						/* Користувач вирішив записати нові налаштування-
						 * переписуємо їх з буфера до основного масиву,
						 * в реальній системі - записуємо основний масив на ФЛЕШ*/
						memcpy(&WriteSet,BuffSet,sizeof(WriteSet));
						/* скидаємо в ноль 0 регістр 16, де прапори натискання користувачем клавіш*/
						/* Фіксуємо, що налаштування вже записали */
						oven_session_status_t os=f_ovenSMI2_TASKS_06 (
							MB_320_ADDRESS,
							0x0F, /* регістр */
							0x0000/* в ноль  */
							);
						/* Примусовий перехід на головний екран */
						ot=f_ovenSMI2_TASKS_06 (
								MB_320_ADDRESS,
								9,               /* register */
								1                /* value    */
								);
						/*Користувач ВЖЕ натиснув ENTER, блокуємо на якийсь час */
						FlagFixSet=1;
						}
//					}
				}
				else if(Esc_or_Ent==0x0100){/* ESC */
					oven_session_status_t os=f_ovenSMI2_TASKS_06 (
						MB_320_ADDRESS,
						0x0F, /* регістр */
						0x0000/* в ноль  */
						);

					os=f_ovenSMI2_TASKS_06 (
						MB_320_ADDRESS,
						0x0F, /* регістр */
						0x0000/* в ноль  */
						);
					/* Примусовий перехід на головний екран */
					ot=f_ovenSMI2_TASKS_06 (
							MB_320_ADDRESS,
							9,               /* register */
							1                /* value    */
							);
				}
				else{}
				break;
			}
			case 14:{

				break;
			}
			case MODBUS_COMPORT_ERR:{
				15;
			}
			break;
			default:{
			}

		}
		OldScreen=ScreenN;
		/* Скидаэмо прапор, можна знову писати налаштування */
		if(ScreenN!=10){FlagFixSet=0;}

}


oven_session_status_t f_oven320_Tasks (void){

	f_oven320_Init();
	while (1){
		f_oven320_Loop();
	}
//
////	https://forum.arduino.cc/index.php?topic=176142.0
////	https://www.revolvy.com/page/POWER-LINE-COMMUNICATION-(PLC)-USING-MODBUS-PROTOCOL?stype=videos&cmd=list&sml=zz1YJXZAB2c&cr=1
	uint16_t u16[100]={0xFF};
	oven_session_status_t y7g=f_ovenSMI2_TASKS_06 (
			MB_320_ADDRESS,
			5,
			12
			);
	oven_session_status_t yt=f_ovenSMI2_TASKS_03(
			MB_320_ADDRESS, /* Модбас-адреса прилада                */
			0x05,           /* Адреса початкового регістра Модбас   */
			70,
			&u16            /* зчитані з регістрів число            */
			);

	uint16_t r05=0;
	oven_session_status_t yy = f_ovenSMI2_TASKS_03_1 (
			MB_320_ADDRESS,      /* Модбас-адреса прилада    */
			0x05,    /* Адреса регістра Модбас   */
			&r05 /* зчитате з регістра число */
			);
//	for(uint16_t i=0;i<300;i++){
//		f_ovenSMI2_TASKS_03 (
//				MB_320_ADDRESS,      /* Модбас-адреса прилада    */
//				i,    /* Адреса регістра Модбас   */
//				registers320[i] /* зчитате з регістра число */
//				);
//	}
	return oven_ResponseSent;
}

oven_session_status_t f_oven320isMaster(void){
	/* Нескінчений цикл з очікування повідомлень від Мастер, їх обробка та
	 * надання відповідей */
	while(1){
		nRxMsg=0;
		memset( &RspRxMsg,0,sizeof(RspRxMsg));
		/* Якщо знайшли на вході повідомлення Модбас */
		crc_search_t csst =f_AskMasterMB(&RspRxMsg);
	//	nRxMsg=f_AskMasterMB(mbRxMsg);
		uint16_t crc_calc_tx16 = 0;
		if(!(
		   (csst==crcs_full_found)||
		   (csst==	crcs_crc_and_wrongbytes_found)
		   )){
			return oven_NO_REQUEST;
		}

		uint8_t MB_FunctionCode = RspRxMsg.msg[1];
		memset( &RspTxMsg,0,sizeof(RspTxMsg));
		switch (MB_FunctionCode){
			case 0x01:{
				RspTxMsg.length=6;
				RspTxMsg.msg[0]=RspRxMsg.msg[0];/* Address */
				RspTxMsg.msg[1]=RspRxMsg.msg[1];/* Function Code */
				RspTxMsg.msg[2]=0x01;/* Byte count */
				RspTxMsg.msg[3]=0x01;/* CoilsValues */
			}

			break;
			case 0x03:{
				GetSystemTime(&SystemTime);
				st[ist]=(uint16_t)SystemTime.wMilliseconds;
				ist++;
				if(ist==10){
					ist=0;
				}
				uint8_t N = RspRxMsg.msg[5];
				RspTxMsg.length=5+2*N;
				RspTxMsg.msg[0]=RspRxMsg.msg[0];/* Address */
				RspTxMsg.msg[1]=RspRxMsg.msg[1];/* Function Code */
				RspTxMsg.msg[2]=2*N;/* Byte count */
				/* RegisterssValues */
				for(uint8_t i=0;i<2*N;i++){RspTxMsg.msg[3+i]=i+1;}
			}
			break;

		}
		crc_calc_tx16 = CRC_16x (RspTxMsg.msg, 			    			/* 2. Обчислюємо 16-бітний CRC   									*/
				RspTxMsg.length -2);
		RspTxMsg.msg[RspTxMsg.length-1]	= crc_calc_tx16 / 0x100;		/* 3. Старший байт CRC засилаємо в останній байт повідомлення 		*/
		RspTxMsg.msg[RspTxMsg.length-2]	= crc_calc_tx16 % 0x100;		/* 4. Молодший байт СRC засилаэмо в передостанный байт повыдомлення */
		modbus_status_t ms = tw_txModbus_Session (
				cport_nr,
				&RspTxMsg
				);
	}
}







oven_session_status_t f_oven320_TASKS_17 (uint8_t oven320_ADDRESS){
	oven_session_status_t session_status17 = oven_UnknownERR;
	modbus_master_tx_msg_t w17 = {0};
//	sprv_t sprv17 = {0};
	uint16_t crc_calc_tx17 = 0;
	w17.length = 4;
	w17.msg[0] = oven320_ADDRESS;
	w17.msg[1] = 17;

	crc_calc_tx17 = CRC_16x (w17.msg, 			    			/* 2. Обчислюємо 16-бітний CRC   									*/
		        w17.length -2);
	w17.msg[w17.length-1]	= crc_calc_tx17 / 0x100;			/* 3. Старший байт CRC засилаємо в останній байт повідомлення 		*/
	w17.msg[w17.length-2]	= crc_calc_tx17 % 0x100;			/* 4. Молодший байт СRC засилаэмо в передостанный байт повыдомлення */

																/* 5. Задаємо час очікування відповіді в мілісекундах згідно з параметрами прикладого алгоритму */
	modbus_master_rx_msg_t w17responce = {0};
	//w17responce.length=19;

	while (session_status17 != oven_OK) {
		modbus_status_t modbus_status17=MODBUS_UNKNOWN_ERROR;
		modbus_status17=tw_txrxModbus_Session (cport_nr, &w17, &w17responce, 400);
		if(modbus_status17 != MODBUS_SUCCESS){
			/* Якщо в мережі відсутній прилад з такою адресою */
			if(modbus_status17 == MODBUS_ERR_TIMEOUT){session_status17=oven_WrongDevice;}
			/* якщо інші помилки сесії */
			else session_status17=oven_Modbus_ERR;
			break;
		}
		/* Якщо в мережі відсутній прилад з такою адресою */
		/* якщо отримали відгук */
		uint16_t crc_calc_rx=0;
		crc_calc_rx = CRC_16x (w17responce.msg, 			    		/* 2. Обчислюємо 16-бітний CRC   									*/
				w17responce.length -2);
		if (
			 (w17responce.msg[w17responce.length-1] != crc_calc_rx / 0x100) ||/* якщо CRC у повідомленні правильний*/
			 (w17responce.msg[w17responce.length-2] != crc_calc_rx % 0x100)
		   ){
			session_status17=oven_AnswerLOST;break;
		}
		if (w17responce.msg[0] != w17.msg[0]){
			session_status17=oven_WrongDevice;break; 		/* address ok*/
		}
		if (w17responce.msg[1] != w17.msg[1]){
			session_status17=oven_WrongeFunction;break;		/* function ok*/
		}
		session_status17=oven_OK;
	}
	return session_status17;
}




#endif   /*MODBUS_MASTER_for_OVEN_INDICATOR*/
