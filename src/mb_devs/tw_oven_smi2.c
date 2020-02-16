/*
 * tk_oven_smi2.c
 *
 *  Created on: 23 трав. 2018 р.
 *      Author: KozakVF
 */

#include "tw_oven_smi2.h"

#include <stdio.h>
#include "stdint.h"
#include "windows.h"

#include "../mb_funcs/tkCRC.h"
//#include "../mb_funcs/tw_mb.h"
//#ifdef   WINDOWS_MODBUS_MASTER_for_OVEN_INDICATOR
//https://owen.ua/uploads/104/re_smi2_2-ru-54088-1.2_a4.pdf
/*********************************************************************************************************************
 *  ОГОЛОШЕННЯ СТАТИЧНИХ ФУНКЦІЙ
 *********************************************************************************************************************/

static oven_session_status_t f_ovenSMI2_FloatToString (float value, oven_smi2_string_t *ps);

/*********************************************************************************************************************
 *  Ф У Н К Ц І Ї
 *********************************************************************************************************************/
oven_session_status_t f_ovenSMI2_Init (uint8_t ovenSMI_ADDRESS){

	oven_session_status_t InitStatus = oven_UnknownERR;
	/* задержка ответа - 48 мс		*/
	InitStatus=f_ovenSMI2_TASKS_06(ovenSMI_ADDRESS, 8, 48);
	if(InitStatus!=oven_OK){return InitStatus;}
	Sleep (20);
	/* тип переменной - "строка" 	*/
	InitStatus=f_ovenSMI2_TASKS_06(ovenSMI_ADDRESS, 17, 3);
	if(InitStatus!=oven_OK){return InitStatus;}
	Sleep (20);
	/* выключить мигание			*/
	InitStatus=f_ovenSMI2_TASKS_06(ovenSMI_ADDRESS, 0x0023, 0x0000);
	Sleep (20);
	return InitStatus;
}


/* тип переменной - "строка" */
oven_session_status_t f_ovenSMI2_StringSet(uint8_t ovenSMI_ADDRESS){
	return (f_ovenSMI2_TASKS_06(ovenSMI_ADDRESS, 17, 3));
}

/**********************************************************************************************************************
 * ШИРОКОВЕЩАТЕЛЬНАЯ ЗАПИСЬ ОПЕРАТИВНЫХ ПАРАМЕТРОВ
 * 1. Модбас-адреса нескольких (до 31, по моему, индикаторов СМИ2 должны быть подряд,
 * например 0х20, 0х21, ... , 0х51. Указывается лишь адрес первого регистра и количчество регистров.
 * 2. Массив плавающих чисел должен быть объявлен и определен.
 *
 **********************************************************************************************************************/
oven_session_status_t f_ovenSMI2_DisplayBroadCast (	uint8_t first_modbass_address,
													uint8_t number_of_devices,
													float *pvalue){
	oven_session_status_t oven_dysplay_status = oven_UnknownERR;
	if (number_of_devices>31){
		oven_dysplay_status = oven_WrongeTask; return oven_dysplay_status;
	}
	if (pvalue==NULL){
		oven_dysplay_status = oven_NULL_POINTER; return oven_dysplay_status;
	}
	modbus_master_tx_msg_t w_fl = {0};

		w_fl.msg [0]= 0x00; /* modbuss broudcact address */
		w_fl.msg [1]= 0x10; /* function code*/
		uint16_t ADAD_ADDR = 0x03e8 + first_modbass_address;
		w_fl.msg [2]= ADAD_ADDR/0x100; /* register address to begin writing, fist byte of uint16_t */
		w_fl.msg [3]= ADAD_ADDR%0x100; /* register address to begin writing, last byte of uint16_t*/
		uint16_t number_of_16bit_words = number_of_devices*4;
		w_fl.msg [4]= number_of_16bit_words/0x100; /* number of 16bit words, fist byte of uint16_t */
		w_fl.msg [5]= number_of_16bit_words%0x100; /* number of 16bit words, last byte of uint16_t */
		w_fl.msg [6]= number_of_devices*8; /* number of bytes*/
		w_fl.length=7+w_fl.msg [6]+2;

		for (uint8_t i=0;i<number_of_devices;i++){
			oven_smi2_string_t tmp_str={0};
			f_ovenSMI2_FloatToString (pvalue[i], &tmp_str);

			w_fl.msg[7+i*8+0]=tmp_str.smbl[0];
			w_fl.msg[7+i*8+1]=tmp_str.smbl[1];
			w_fl.msg[7+i*8+2]=tmp_str.smbl[2];
			w_fl.msg[7+i*8+3]=tmp_str.smbl[3];
			w_fl.msg[7+i*8+4]=tmp_str.smbl[4];
			w_fl.msg[7+i*8+5]=tmp_str.smbl[5];
			w_fl.msg[7+i*8+6]=tmp_str.smbl[6];
			w_fl.msg[7+i*8+7]=tmp_str.smbl[7];
		}

	uint16_t crc_calc_tx16 = 0;
	crc_calc_tx16 = CRC_16x (w_fl.msg, 			    		    	/* 2. Обчислюємо 16-бітний CRC   									*/
			        w_fl.length -2);
		w_fl.msg[w_fl.length-1]	= crc_calc_tx16 / 0x100;			/* 3. Старший байт CRC засилаємо в останній байт повідомлення 		*/
		w_fl.msg[w_fl.length-2]	= crc_calc_tx16 % 0x100;			/* 4. Молодший байт СRC засилаэмо в передостанный байт повыдомлення */

																	/* 5. Задаємо час очікування відповіді в мілісекундах згідно з параметрами прикладого алгоритму */
		modbus_master_rx_msg_t w16responce = {0};

			/* Тільки циркулярна передача. Відповіді не чекаємо        */
		if (tw_txModbus_Session (cport_nr, &w_fl)!= MODBUS_SUCCESS){

//			    tw_txrxModbus_Session (cport_nr, &w_fl, &w16responce, 400) != MODBUS_SUCCESS){
				oven_dysplay_status=oven_Modbus_ERR;
			}
		else{
			oven_dysplay_status=oven_OK;
		}

		return oven_dysplay_status;

}

/* Виведення на СМИ2 рядка 4 символи (символи - перші чотири)                               */
oven_session_status_t f_ovenSMI2_DisplayString4 (uint8_t ovenSMI_ADDRESS, uint8_t *str){
	oven_session_status_t df1 = f_ovenSMI2_TASKS_16(
			ovenSMI_ADDRESS,  /* Модбас-адреса прилада                                       */
			0x1D,             /* Регістр, з якого починати писати 0x0000..0xFFFF             */
			2,                /* Кількість регістрів, які треба писати 0x0001 to 0x007B(123) */
			4,                /* Кількість байт, як треба писати   2*  0x01  to 0x7B(246)    */
			str               /* Масив байт або (uint8_t*) з масивом 16-значних числел       */
			);
	return df1;
}

/* Виведення на СМИ2 рядка 8 символів (Наприклад)                               */

oven_session_status_t f_ovenSMI2_DisplayString8 (uint8_t ovenSMI_ADDRESS, uint8_t *str){
	oven_session_status_t df1 = f_ovenSMI2_TASKS_16(
			ovenSMI_ADDRESS,  /* Модбас-адреса прилада                                       */
			0x1D,             /* Регістр, з якого починати писати 0x0000..0xFFFF             */
			4,                /* Кількість регістрів, які треба писати 0x0001 to 0x007B(123) */
			8,                /* Кількість байт, як треба писати   2*  0x01  to 0x7B(246)    */
			str               /* Масив байт або (uint8_t*) з масивом 16-значних числел       */
			);
	return df1;
}

oven_session_status_t f_ovenSMI2_DisplayFloat1 (uint8_t ovenSMI_ADDRESS, float value){
	oven_smi2_string_t st = {0};
	f_ovenSMI2_FloatToString (value, &st);
	oven_session_status_t df1 = f_ovenSMI2_TASKS_16 (
			ovenSMI_ADDRESS,  /* Модбас-адреса прилада                                       */
			0x001D,           /* Регістр, з якого починати писати 0x0000..0xFFFF             */
			4,                /* Кількість регістрів, які треба писати 0x0001 to 0x007B(123) */
			8,                /* Кількість байт, як треба писати   2*  0x01  to 0x7B(246)    */
			st.smbl         /* Масив байт або (uint8_t*) з масивом 16-значних числел       */
			);
	return df1;
}


oven_session_status_t f_ovenSMI2_DisplayFloat (uint8_t ovenSMI_ADDRESS, float value){
	oven_session_status_t oven_dysplay_status = oven_UnknownERR;
	modbus_master_tx_msg_t w_fl = {0};
	/* По суті це та сама команда Модбас 16, лише запис конкретних заданих регістрів
	 * 0х10    код команди
	 * 0х001D  перший регістр, куди писати
	 * 0х04    кількість регістрів, куди писати
	 * */
		w_fl.msg [0]= ovenSMI_ADDRESS;
		w_fl.msg [1]= 0x10; /* function */

		w_fl.msg [2]= 0x00; /* register address to begin writing, fist byte of uint16_t */
		w_fl.msg [3]= 0x1D; /* register address to begin writing, last byte of uint16_t*/

		w_fl.msg [4]= 0x00; /* number of 16bit words, fist byte of uint16_t */
		w_fl.msg [5]= 0x04; /* number of 16bit words, last byte of uint16_t */

		w_fl.msg [6]= 0x08; /* number of bytes*/

		oven_smi2_string_t st = {0};
		f_ovenSMI2_FloatToString (value, &st);

		w_fl.length=17;
		/* байти 7..14 - рядок на дисплей */
		w_fl.msg [7]  = st.smbl[0];
		w_fl.msg [8]  = st.smbl[1];
		w_fl.msg [9]  = st.smbl[2];
		w_fl.msg [10] = st.smbl[3];
		w_fl.msg [11] = st.smbl[4];
		w_fl.msg [12] = st.smbl[5];
		w_fl.msg [13] = st.smbl[6];
		w_fl.msg [14]=  st.smbl[7];

	uint16_t crc_calc_tx16 = 0;
	crc_calc_tx16 = CRC_16x (w_fl.msg, 			    		/* 2. Обчислюємо 16-бітний CRC   									*/
			        w_fl.length -2);
		w_fl.msg[w_fl.length-1]	= crc_calc_tx16 / 0x100;			/* 3. Старший байт CRC засилаємо в останній байт повідомлення 		*/
		w_fl.msg[w_fl.length-2]	= crc_calc_tx16 % 0x100;			/* 4. Молодший байт СRC засилаэмо в передостанный байт повыдомлення */

																	/* 5. Задаємо час очікування відповіді в мілісекундах згідно з параметрами прикладого алгоритму */
		modbus_master_rx_msg_t w16responce = {0};
		w16responce.length=8;

		while (oven_dysplay_status != oven_OK) {
			modbus_status_t mbb=MODBUS_UNKNOWN_ERROR;
			mbb=tw_txrxModbus_Session (cport_nr, &w_fl, &w16responce, 100);
			if (mbb != MODBUS_SUCCESS){
				if(mbb==MODBUS_ERR_TIMEOUT){oven_dysplay_status=oven_answerTimeout;break;}
				else 		               {oven_dysplay_status=oven_Modbus_ERR;   break;}
			}
			uint16_t crc_calc_rx=0;
			crc_calc_rx = CRC_16x (w16responce.msg, 			    		/* 2. Обчислюємо 16-бітний CRC   									*/
					w16responce.length -2);

			if (
				 (w16responce.msg[w16responce.length-1] != crc_calc_rx / 0x100) ||/* якщо CRC у повідомленні правильний*/
				 (w16responce.msg[w16responce.length-2] != crc_calc_rx % 0x100)
			   ){
				oven_dysplay_status=oven_AnswerLOST;break;
			}
			if (w16responce.msg[0] != w_fl.msg[0]){
				oven_dysplay_status=oven_WrongDevice;break; 		/* address ok*/
			}
			if (w16responce.msg[1] != w_fl.msg[1]){
				oven_dysplay_status=oven_WrongeFunction;break;		/* function ok*/
			}

			oven_dysplay_status=oven_OK;
		}
		return oven_dysplay_status;



}

static oven_session_status_t f_ovenSMI2_FloatToString (float value, oven_smi2_string_t *ps){
	oven_session_status_t oven_dysplay_status = oven_UnknownERR;
	uint32_t d_int = 0;
	uint8_t  dig   =0;
	if (value >9999.0) {

				ps->smbl [0]   = 0x45; /*    E            */
				ps->smbl [1]   = 0x72; /*    r            */
				ps->smbl [2]   = 0x72; /*    r            */
				ps->smbl [3]   = 0x20; /*                 */

				ps->smbl [4]   = 0x20;  /*                 */
				ps->smbl [5]   = 0x20;  /*                 */
				ps->smbl [6]   = 0x20;  /*                 */
				ps->smbl [7]   = 0x20;  /*                 */
	}
	else if ((value >=1000.0)&&(value<10000.0)) {
		d_int=(uint32_t)value;
		dig=d_int/1000;
					ps->smbl [0]   = 0x30+dig;
		d_int=d_int-dig*1000;
		dig=d_int/100;
					ps->smbl [1]   = 0x30+dig;
		d_int=d_int-dig*100;
		dig=d_int/10;
					ps->smbl [2]   = 0x30+dig;
		d_int=d_int-dig*10;
		dig=d_int;
					ps->smbl [3]   = 0x30+dig;

					ps->smbl [4]   = 0x20;
					ps->smbl [5]   = 0x20;
					ps->smbl [6]   = 0x20;
					ps->smbl [7]   = 0x20;
		}
	else if ((value >=100.0)&&(value<1000.0)) {
		d_int=(uint32_t)(value*10.0);
		dig=d_int/1000;
					ps->smbl [0]   = 0x30+dig;
		d_int=d_int-dig*1000;
		dig=d_int/100;
					ps->smbl [1]   = 0x30+dig;
		d_int=d_int-dig*100;
		dig=d_int/10;
					ps->smbl [2]   = 0x30+dig;

					ps->smbl [3]   = 0x2E;
		d_int=d_int-dig*10;
		dig=d_int;
					ps->smbl [4]   = 0x30+dig;


					ps->smbl [5]   = 0x20;
					ps->smbl [6]   = 0x20;
					ps->smbl [7]   = 0x20;
	}
	else if ((value >=10.0)&&(value<100.0)) {
		d_int=(uint32_t)(value*100.0);
		dig=d_int/1000;
					ps->smbl [0]   = 0x30+dig;
		d_int=d_int-dig*1000;
		dig=d_int/100;
					ps->smbl [1]   = 0x30+dig;
					ps->smbl [2]   = 0x2E;

		d_int=d_int-dig*100;
		dig=d_int/10;
					ps->smbl [3]   = 0x30+dig;
		d_int=d_int-dig*10;
		dig=d_int;
					ps->smbl [4]   = 0x30+dig;
					ps->smbl [5]   = 0x20;
					ps->smbl [6]   = 0x20;
					ps->smbl [7]   = 0x20;
	}
	else if ((value >=0.001)&&(value<10.0)) {
		d_int=(uint32_t)(value*1000.0);
		dig=d_int/1000;
					ps->smbl [0]   = 0x30+dig;
					ps->smbl [1]   = 0x2E;
		d_int=d_int-dig*1000;
		dig=d_int/100;
					ps->smbl [2]   = 0x30+dig;


		d_int=d_int-dig*100;
		dig=d_int/10;
					ps->smbl [3]   = 0x30+dig;
		d_int=d_int-dig*10;
		dig=d_int;
					ps->smbl [4]   = 0x30+dig;
					ps->smbl [5]   = 0x20;
					ps->smbl [6]   = 0x20;
					ps->smbl [7]   = 0x20;
	}
	else if ((value >-0.01)&&(value<0.001)) {

					ps->smbl [0]   = 0x30;
					ps->smbl [1]   = 0x30;
					ps->smbl [2]   = 0x30;
					ps->smbl [3]   = 0x30;
					ps->smbl [4]   = 0x20;
					ps->smbl [5]   = 0x20;
					ps->smbl [6]   = 0x20;
					ps->smbl [7]   = 0x20;
	}
	else if ((value >-10.0)&&(value<=-0.01)) {

		    ps->smbl[0]   = 0x2D;

			d_int=(uint32_t)(value*100.0*(-1.0));
			dig=d_int/100;
					ps->smbl[1]   = 0x30+dig;
					ps->smbl[2]   = 0x2E;
			d_int=d_int-dig*100;
			dig=d_int/10;

					ps->smbl[3]   = 0x30+dig;

			d_int=d_int-dig*10;
			dig=d_int;
						ps->smbl[4]   = 0x30+dig;

						ps->smbl[5]   = 0x20;
						ps->smbl[6]   = 0x20;
						ps->smbl[7]   = 0x20;

		}

	else if ((value >-100.0)&&(value<=-10.0)) {

		    ps->smbl[0]   = 0x2D;

			d_int=(uint32_t)(value*10.0*(-1.0));
			dig=d_int/100;
					ps->smbl[1]   = 0x30+dig;

			d_int=d_int-dig*100;
			dig=d_int/10;

					ps->smbl[2]   = 0x30+dig;
					ps->smbl[3]   = 0x2E;
			d_int=d_int-dig*10;
			dig=d_int;
						ps->smbl[4]   = 0x30+dig;

						ps->smbl[5]   = 0x20;
						ps->smbl[6]   = 0x20;
						ps->smbl[7]   = 0x20;
		}
	else if ((value >-1000.0)&&(value<=-100.0)) {

			    ps->smbl[0]   = 0x2D;

				d_int=(uint32_t)(value*(-1.0));
				dig=d_int/100;
						ps->smbl[1]   = 0x30+dig;

				d_int=d_int-dig*100;
				dig=d_int/10;

						ps->smbl[2]   = 0x30+dig;

				d_int=d_int-dig*10;
				dig=d_int;
				ps->smbl [3]   = 0x30+dig;

				ps->smbl [4]   = 0x20;
				ps->smbl [5]   = 0x20;
				ps->smbl [6]   = 0x20;
				ps->smbl [7]   = 0x20;
			}
	if (value <-999.0) {

					ps->smbl [0]   = 0x2D; /*    -             */
					ps->smbl [1]   = 0x45; /*    E            */
					ps->smbl [2]   = 0x72; /*    r            */
					ps->smbl [3]   = 0x72; /*    r            */


					ps->smbl [4]   = 0x20;  /*                 */
					ps->smbl [5]   = 0x20;  /*                 */
					ps->smbl [6]   = 0x20;  /*                 */
					ps->smbl [7]   = 0x20;  /*                 */
		}
  else {}
	oven_dysplay_status=oven_OK;
	return oven_dysplay_status;
}
oven_session_status_t f_ovenSMI2_Init_String (uint8_t ovenSMI_ADDRESS){
	oven_session_status_t oven_init_status = oven_UnknownERR;

	f_ovenSMI2_TASKS_06(ovenSMI_ADDRESS, 8, 48);
	f_ovenSMI2_TASKS_06(ovenSMI_ADDRESS, 17, 3);

	uint8_t test_str[4]={'0','1','2','3'};
	f_ovenSMI2_TASKS_16 (0x20, /* Адресс прибора 		*/
						 0x1D, /* Номер первого регистра*/
						 0x02, /* Количество регистров	*/
						 0x04, /* Количество байт		*/
						 test_str
						);
	return oven_init_status;
}







oven_session_status_t f_ovenSMI2_TASKS_17 (uint8_t ovenSMI_ADDRESS){
	oven_session_status_t session_status17 = oven_UnknownERR;
	modbus_master_tx_msg_t w17 = {0};
//	sprv_t sprv17 = {0};
	uint16_t crc_calc_tx17 = 0;
	w17.length = 4;
	w17.msg[0] = ovenSMI_ADDRESS;
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
		if(w17responce.length>3){
			if (
					(w17responce.msg[w17responce.length-1] != crc_calc_rx / 0x100) ||/* якщо CRC у повідомленні правильний*/
					(w17responce.msg[w17responce.length-2] != crc_calc_rx % 0x100)
		     ){
			session_status17=oven_AnswerLOST;break;
			}
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

/* ВКЛЮЧИТЬ МИГАНИЕ */
oven_session_status_t f_ovenSMI2_DisplayBlinkingON (uint8_t ovenSMI_ADDRESS){
	volatile uint16_t inDm=0xFF;
	f_ovenSMI2_TASKS_06(ovenSMI_ADDRESS, 0x23, 0xBB); 		/* выключить мигание			*/
	Sleep (20);
	return oven_OK;
}

/* ВЫКЛЮЧИТЬ МИГАНИЕ */
oven_session_status_t f_ovenSMI2_DisplayBlinkingOFF (uint8_t ovenSMI_ADDRESS){

	uint16_t inDm=0xFE;
	f_ovenSMI2_TASKS_03_1( ovenSMI_ADDRESS,0x23,&inDm);
	Sleep(20);

	f_ovenSMI2_TASKS_06(ovenSMI_ADDRESS, 0x23, 0x00); 		/* выключить мигание			*/
	Sleep(20);

	f_ovenSMI2_TASKS_03_1( ovenSMI_ADDRESS,0x23,&inDm);
	Sleep(20);
	return oven_OK;
}

//#endif   /*MODBUS_MASTER_for_OVEN_INDICATOR*/
