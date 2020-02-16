/*
 * a3_TK112TxRx.c
 *
 *  Created on: 31 груд. 2019 р.
 *      Author: KozakVF
 */

#include "a3_tkTxRx.h"
#include "../wConfig.h"
//#include "a3_tk2z.h"
#include <windows.h>
#include <string.h>
#include <stdio.h>
//#include "../mb_funcs/wk_wqueue.h"

#include "../mb_funcs/tkCRC.h"
#include <process.h>    /* _beginthread, _endthread */

vk_software_t vk_software = TK2_DISTANCE;
static HWND hWndTxRx=NULL;


/* Глобальні змінні, передаються на обробку в вікно, яке повідомлення кидало в чергу */
modbus_master_rx_msg_t mIn_rx_msg={0};
/* Глобальні змінні, передаються на обробку в вікно, яке повідомлення кидало в чергу */
modbus_master_tx_msg_t mOut_tx_msg={0};
/* Глобальні змінні, передаються на обробку в вікно, яке повідомлення кидало в чергу */
modbus_status_t RxMasterModbusStatus=MODBUS_UNKNOWN_ERROR;


static ULONGLONG  TxRxSessionBusyEtimeSum =0;
static ULONGLONG  TxRxSessionBusyIddleEtimeSum  =0;
//typedef struct{
	/* Стан обробника черги:
	 * ProcessIddle - черга пуста,
	 * ProcessBusy  - черга чимось заповнена, і
	 * триває сесій Модбас*/
	static ProcessState_t   RS232State = ProcessIddle;
	/* Глобальна змінна -  підсумковий процент зайнятості каналу
	 * за весь час дистанційного управління  */
	procentf_t gTxRxBusyAveDegreef          =0;
	/* Глобальна змінна - поточний процент зайнятості каналу
	 * зглажений з постійною часу - п'ять останніх сеансів */
	procentf_t gTxRxBusyDegreef             =0;
	/* Поточний лічильник сесій*/
	ULONGLONG  gTxRxSessionStartCntr        =0;
	/* Поточний лічильний сесій, що завершилися без помилок */
	ULONGLONG  gTxRxSessionOkCntr           =0;
	/* Поточний процент сесій, що завершилися без помилок   */
	procentf_t gTxRxSessionSuccessDegreef   =0;
	/* Середній за весь час дистанційного управління сесій, що завершилися без помилок */
	procentf_t gTxRxSessionAVESuccessDegreef = 0;
//}tXrX_session_duration_t;

/* Тривилість часу Iddle and Busy
 * 1) починаємо рахувати після натискання кнопки
 *    старту дистанційного управління, коли починається періодичне опитування регістра
 * 2) перериваємо після входу в вікно налаштуваня користувача,
 * 3) відразу ж відновлюємо у прив'язцs до початку періодичного питування
 * 	  стану доступу
 * 	  Це опитування не припиняється при вході користувача в режим налаштувань виробника
 * 4) перериваємо при виході з вікна налаштувань користувача
 * 5) відновлюємо при виході з налаштувань користувача у прив'язці до
 *    відновлення опитування стану тк2
 * 6) Остаточно припиняємо при виході з програми..
 * */
static ProcessState_t TxRxBusyDegreeCalc = ProcessIddle;
static ProcessState_t TxRxBusyDegreeCalcOld = ProcessIddle;
static ULONGLONG  TxRxStartTime         =0;
/* 'a3_TxRx.c/.h, Тривилість часу Iddle and Busy
 * 1) починаємо рахувати після натискання кнопки
 *    старту дистанційного управління, коли починається періодичне опитування регістра
 * 2) перериваємо після входу в вікно налаштуваня користувача,
 * 3) відразу ж відновлюємо у прив'язцs до початку періодичного питування
 * 	  стану доступу
 * 	  Це опитування не припиняється при вході користувача в режим налаштувань виробника
 * 4) перериваємо при виході з вікна налаштувань користувача
 * 5) відновлюємо при виході з налаштувань користувача у прив'язці до
 *    відновлення опитування стану тк2
 * 6) Остаточно припиняємо при виході з програми..
 * */
void f_TxRxBusyDegreeCalcStart(void){
	TxRxBusyDegreeCalc=ProcessBusy;
	TxRxStartTime=GetTickCount();
}
/* Тривилість часу Iddle and Busy
 * 1) починаємо рахувати після натискання кнопки
 *    старту дистанційного управління, коли починається періодичне опитування регістра
 * 2) перериваємо після входу в вікно налаштуваня користувача,
 * 3) відразу ж відновлюємо у прив'язцs до початку періодичного питування
 * 	  стану доступу
 * 	  Це опитування не припиняється при вході користувача в режим налаштувань виробника
 * 4) перериваємо при виході з вікна налаштувань користувача
 * 5) відновлюємо при виході з налаштувань користувача у прив'язці до
 *    відновлення опитування стану тк2
 * 6) Остаточно припиняємо при виході з програми..
 * */
void f_TxRxBusyDegreeCalcStop(char *log, size_t log_size){
	TxRxBusyDegreeCalc=ProcessIddle;
	uint32_t DistCntrDuration_ms=(uint32_t)GetTickCount()-TxRxStartTime;
	uint32_t DistCntrDuration_s=DistCntrDuration_ms/1000;
	uint32_t DistCntrDuration_m=DistCntrDuration_s/60;
	uint32_t DistCntrDuration_h=DistCntrDuration_m/60;
			  DistCntrDuration_m=DistCntrDuration_m%60;
	uint32_t DistCntrDuration_days=DistCntrDuration_h/24;
	          DistCntrDuration_h=DistCntrDuration_h%24;
    if(log!=NULL){
    	snprintf(log,log_size,"  :  :       %d діб(доби) %d годин %d хвилин - тривалість дистанційного управління\n",
			(int)DistCntrDuration_days, (int)DistCntrDuration_h, (int)DistCntrDuration_m);
    }
}
/* Тривилість часу Iddle and Busy
 * 1) починаємо рахувати після натискання кнопки
 *    старту дистанційного управління, коли починається періодичне опитування регістра
 * 2) перериваємо після входу в вікно налаштуваня користувача,
 * 3) відразу ж відновлюємо у прив'язцs до початку періодичного питування
 * 	  стану доступу
 * 	  Це опитування не припиняється при вході користувача в режим налаштувань виробника
 * 4) перериваємо при виході з вікна налаштувань користувача
 * 5) відновлюємо при виході з налаштувань користувача у прив'язці до
 *    відновлення опитування стану тк2
 * 6) Остаточно припиняємо при виході з програми..
 * */
static modbus_status_t f_tkTxRx(
		modbus_master_tx_msg_t *pmb_tx_msg,
		modbus_master_rx_msg_t *pmb_rx_msg,
		int timeout
		);

/* Це функція - обробник черги яка:
 * забирає дані з голови черги в тимчасові змінні
 * стирає голову черги
 * надсилає повідомлення з параметрами, заданими в тілі вузла черги  */
void f_desktop_tk_session (
		queue_t *htkQ           /* показчик на структуру - дескриптор черги*/
	){
	int status = 0;
	static uint32_t TxRxSessionStartEtime;
	static uint32_t TxRxSessionStopEtime;
	static uint32_t TxRxSessionDurEtime;
	static uint32_t TxRxIddleDurEtime;
	while(status>=0){
        if(htkQ==NULL){status=-1;break;}
        /* Якщо в черзі є хоч одне повідомлення
			 * поки що використаємо глобальну змінну mIn_rx_msg для передачі
			 *  прапора про закінчення обробки попереднього повідоблення
//			 *  Це пов'язане з тим, що вдповідь обробляється в викликаючому вікні,
 * 			    і поки повідомлення не оброблене, або хоча б не скопійоване -
 * 			    наступне повинно блокуватися */
		if ((htkQ->node_number>0)&&(mIn_rx_msg.length==0)){

			/* Поточний лічильник сесій */
			gTxRxSessionStartCntr++;
			/* Домоміжні обчислення для обчислення
			 * проценту зайнятості каналу
			 *                    */
			char str[256]={0};
			snprintf (str,sizeof(str),"a3TxRx.c Старт сесії %d", gTxRxSessionStartCntr);
			f_tk5Logging(str, sizeof(str));


			TxRxSessionStartEtime = (uint32_t)GetTickCount();
			if (TxRxSessionStartEtime>TxRxSessionStopEtime){
				TxRxIddleDurEtime=TxRxSessionStartEtime-TxRxSessionStopEtime;
			}
			else{
				TxRxIddleDurEtime=10;
			}

			RS232State=ProcessBusy;
			tk_session_data_t *htks=htkQ->first;  /* показчик на структуру - дескриптр тіла вузла черги,
			                                         тобто на голову черги    */
			/* Все з голови забираємо на обробку..
			 * і буфер, який оголошено публічним і до якого мають доступ вікна,
			 * з яких повідомлення кидалося в чергу */
			memcpy(&mOut_tx_msg,&(htks->mb_tx_msg),sizeof(modbus_master_tx_msg_t));
			int in_timeout = htks-> timeout;
			int in_case = htks->in_case;
			HWND in_hwnd = htks->hwnd;
			snprintf (str,sizeof(str),"a3TxRx.c Варіант сесії %d", in_case);
			f_tk5Logging(str, sizeof(str));
			/* Видаляємо голову, бо за час сесії Модбас
			 * в голову черги можуть прийти інші повідомлення,
			 * і тоді конфлікт.. */
			/* видалення голови */
			deletehead(
					sizeof(tk_session_data_t),/* размер головы */
					htkQ /* Ссылка на обработчик очереди */
					);
			/* Власне сесія Модбас */
			memset(&mIn_rx_msg,0,sizeof(modbus_master_rx_msg_t));
 			RxMasterModbusStatus=f_tkTxRx(
					&mOut_tx_msg,
					&mIn_rx_msg,
					in_timeout
					);
			/* Якщо сесія завершилась Ок -
			 * отримаємо повідомлення для обробки in_rx_msg,
			 * якщо з помилкою - обробляємо код помилки */
			/* Нагадуємо вікнові, з якого надсилалося повідомлення,
			 * що відповідь отримано, необхідна реакція вікна    */
 			snprintf (str,sizeof(str),"a3TxRx.c Сесію %d %d  завершено. Код %d",
 					(int)gTxRxSessionStartCntr, in_case, (int)RxMasterModbusStatus);
 			f_tk5Logging(str, sizeof(str));


			if(IsWindow(in_hwnd)){
					SendMessage(
						in_hwnd,
						VK_UPDATE_WINDOW,
						in_case,/* wParam */
						0
			    		);
				}
			if(in_case==0xEEEE){
				gTxRxSessionAVESuccessDegreef=0;
			}

			/* Середній процент сесій, що завершилися без помилок   */
			gTxRxSessionAVESuccessDegreef  =	(float)(gTxRxSessionOkCntr*10000/gTxRxSessionStartCntr) / 100.0f;
			/* Поточний згладжений процес сесій, щозавершилися без помилок */
			procentf_t tmpgTxRxSessionSuccessDegreef=gTxRxSessionSuccessDegreef;
			procentf_t tmpthissessionOk =0;
			if(RxMasterModbusStatus==MODBUS_SUCCESS){tmpthissessionOk=100.0f;}else{tmpthissessionOk=0.0f;}
			float alphams=0.9;
			gTxRxSessionSuccessDegreef = tmpgTxRxSessionSuccessDegreef*alphams + tmpthissessionOk*(1.0f-alphams);

			/* Обчислення проценту поточної зайнятості каналу */
			RS232State=ProcessIddle;
			TxRxSessionStopEtime=GetTickCount();
			if (TxRxSessionStopEtime>TxRxSessionStartEtime){
				TxRxSessionDurEtime=TxRxSessionStopEtime-TxRxSessionStartEtime;
			}
			else{
				TxRxSessionDurEtime=10;
			}
			procentf_t lTxRxBusyDegreef= (float)(TxRxSessionDurEtime*100) /(float)(TxRxSessionDurEtime+TxRxIddleDurEtime);
			procentf_t TxRxBusyDegreef=gTxRxBusyDegreef ;
			float alpha=0.2;
			gTxRxBusyDegreef=lTxRxBusyDegreef*alpha+TxRxBusyDegreef*(1.0f-alpha);

			/* Обчислення середнього проценту зайнятості каналу  за весь час дистанційного управління*/
			if((TxRxBusyDegreeCalcOld==ProcessIddle)&&(TxRxBusyDegreeCalc==ProcessBusy)){
				TxRxSessionBusyEtimeSum += TxRxSessionDurEtime;
			}
			if((TxRxBusyDegreeCalcOld==ProcessBusy)&&(TxRxBusyDegreeCalc==ProcessBusy)){
				/* Обчислення середнього проценту зайнятості каналу  за весь час дистанційного управління*/
				TxRxSessionBusyEtimeSum += TxRxSessionDurEtime;
				TxRxSessionBusyIddleEtimeSum  += TxRxSessionDurEtime;
				TxRxSessionBusyIddleEtimeSum  += (TxRxSessionDurEtime+TxRxIddleDurEtime);
				ULONGLONG pro_x_10000 = (TxRxSessionBusyEtimeSum*10000)/TxRxSessionBusyIddleEtimeSum;
				gTxRxBusyAveDegreef=(float)pro_x_10000 /100.0f;
			}
			else{

			}
			TxRxBusyDegreeCalcOld=TxRxBusyDegreeCalc;
		}
		/* Примусова перерва 10мс */
		Sleep(10);

	}
	_endthread();
}

/* Ця функція додає нове повідомлення в чергу
 * повертає довжину черги */
int f_set_tkqueue(
		queue_t *htkQ,           /* показчик на структуру - дескриптор черги*/
		FunctionalState lifo, /* ENABLE  - додати в голову черги
		                       * DISABLE - додати у хвіст черги */
		HWND hwnd,            /* вікно, з якого надійшло повідомлення в чергу */
		int in_case,          /* ідентифікатор у вікні, за яким буде визначатися обробка відповіді */
		modbus_master_tx_msg_t *pmb_tx_msg,
							  /* повідомлення Модбас*/
		int timeout           /* таймаут для модбас-повідомлення */
		){
	tk_session_data_t tks={0};
	tks.hwnd=hwnd;
	tks.in_case=in_case;
	memcpy(&tks.mb_tx_msg, pmb_tx_msg, sizeof(modbus_master_tx_msg_t));
	tks.timeout=timeout;

	if (htkQ->node_number<80){
		if(lifo==DISABLE){
			addtail(
				(void*)&tks,   /* Ссылка на тело элемента в вызывающей программе
								  для перезаписи его в очередь */
				sizeof(tk_session_data_t),/*Размер тела элемента списка */
				htkQ /*Ссылка на обработчик очереди */
				);
		}
		else if(lifo==ENABLE){
			addhead(
				(void*)&tks,   /* Ссылка на тело элемента в вызывающей программе
								  для перезаписи его в очередь */
				sizeof(tk_session_data_t),/*Размер тела элемента списка */
				htkQ /*Ссылка на обработчик очереди */
				);
		}
		else{}
	}
	return htkQ->node_number;
}

/* Отримання довжини черги. Повідомлення яке обробляється
 * в черзі вже не стоїть..
 * При відсутності черги, тобто повідомлення прийшло й тут же передано в
 * в процес сесії Модбас складно зафіксувати наявність черги,
 * тому більш доцільно використовувати функцію f_get_tk2rs232_length() */
int f_get_tkqueue_length(queue_t *htkQ){
	return htkQ->node_number;
}

/* Отримання довжини черги з урахуванням повідомлення,яке знаходиться у стані обробки*/
int f_get_tkrs232_length(queue_t *htkQ){
	return htkQ->node_number+(int)RS232State;
}
/* Вилучення голови. При цьому визначається наступна голова. */
/* Видалення голови в циклі допоки голів не залишится        */
int f_clear_tkqueue(queue_t *htkQ){
	while (htkQ->node_number!=0){
	      deletehead(
			sizeof(tk_session_data_t),/* размер головы */
			htkQ /* Ссылка на обработчик очереди */
			);
	}
	return htkQ->node_number;
}

void f_Stop_tk_session (queue_t *htkQ){
	f_clear_tkqueue(htkQ);
	_endthread();

}

HWND f_SetTxRxHWND(HWND hwnd){
	hWndTxRx=hwnd;
	return hwnd;
}

/* Це функція - обробник сесії Модбас на рівні черги повідомлень.
 * До вхідного повідомлення (без CRC додається CRC),
 * викликається сесія Модбас нижчого рівня (функція tw_txrxModbus_Session(), файл tw_mb.c/h),
 * якщо сесія Модбас - Ок - до повідомлення-відповіді Модбас додається СRC, повертається MODBUS_SUCCESS
 * додатково перевіряється адреса вх/вих повідомлень
 * якщо помилка - повертається код помилки */
static modbus_status_t f_tkTxRx(
		modbus_master_tx_msg_t *pmb_tx_msg,
		modbus_master_rx_msg_t *pmb_rx_msg,
		int timeout
		){
	modbus_status_t ms=MODBUS_UNKNOWN_ERROR;
//	modbus_status_t BTH_TxTx_status = MODBUS_UNKNOWN_ERROR;
//	modbus_master_tx_msg_t BTH_TxMsg={0};
//	modbus_master_rx_msg_t BTH_RxMsg={0};
//	char BTH_User_Msg[256]={0};
	if((pmb_tx_msg->length != 0)&&(timeout!=0)){
		/* Додаємо CRC*/
		ms = f_add_CRC(pmb_tx_msg);
		/* Запускаємо сесію	*/
		ms=tw_txrxModbus_Session (
				cport_nr,
				pmb_tx_msg,
				pmb_rx_msg,
				timeout
		);
	}
    if(ms == MODBUS_SUCCESS){
	    /* Якщо сесія RTC завершилася успішно, перевіряємо CRC */
	    ms = f_check_CRC(pmb_rx_msg);
	    if (ms != MODBUS_SUCCESS){
	    	memset(&pmb_rx_msg,0,sizeof(modbus_master_rx_msg_t));
	    }
	    else{
	    	/* якщо вхідна адреса не рівна вихідній адресі - це помилка*/
	    	if(pmb_rx_msg->msg[0] != pmb_tx_msg->msg[0]){
	    		ms=MODBUS_ERR_SLAVE_ADDR;	/*  If the transmitted slave address doesn't correspond to the received slave address	*/
	    		memset(&pmb_rx_msg,0,sizeof(modbus_master_rx_msg_t));
	    	}
	    }

    }

	/* Завершення txrx сеансу */
	return ms;
}

//https://stackoverflow.com/questions/16506291/remove-button-from-a-window

