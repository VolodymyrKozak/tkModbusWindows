/*
 * a3_tk2z.c
 *
 *  Created on: 2 січ. 2020 р.
 *      Author: KozakVF
 */

#include "a3_tk5z.h"
//#include "windows.h"
#include "../../wConfig.h"
//#include "a00_tk2Config.h"
#include <stdio.h>
#include <stdbool.h>
#include "../../mb_funcs/vk_wqueue.h"
#include "../a3_tkTxRx.h"
#include <commctrl.h>
#include "a4_tk5log.h"
/* Структура, яка визначає чергу
 * Оголошення цієї структури означає, що створена
 * конкретна черга з конкретним іменем, поки що пуста
 * Створено дескриптор черги повідомлень до Тк2*/
extern   queue_t tk5Queue;
extern FILE *fp_tk5LogFile;

extern HWND hWndtk5;
extern tk5user_settings_t Tk5us;
extern tk5fs_t tk5fs;
/* Структура параметрів заводських налаштувань */
extern d_type_t q;
//extern tk2_fs_t fs;
extern uint8_t tk5_Addr;
grid_t grid={0};
char user_msg5[256]={0};

/* Статус типу та завершення сесії Модбас:
 *   0 - нічого не знаємо про завершення процесу
 *   1 - це була сесія Модбас дистанційного управління
 *       яка завершилася поверненням відповіді від контролера
 *   2 - це була сесія Модбас з тестування зв'язку PC_DESC <-> контролер
 *       яка завершилася поверненням відповіді від контролера
 *   -1  сесія Модбас завершилася з помилкою, потрібна обробка помилки */
int tk5ProcessState = 0;

/* Лічильник надісланих до ТК2 повідомлень-запитів.
 * Інкрементується функцією f_tk2QPWD_RgAnswer(), файл 'a3_tk2z.h'
 * Необхідний, щоб перевіряти необхідність обновлення вікна лише тоді,
 * коли надійшло нове повідомлення */
uint64_t tk5ProcessCntr=0;


uint16_t tk5_PasswpedFlag = 0;

FunctionalState us5_access_status = DISABLE;
FunctionalState us5_access_statusOld = DISABLE;
//RG_R_ACCESS_STATUS
static int test_message_cntr_debug =0;
/***************************************************************************************************************************************
ANSWER                                                                                                    ANSWER ANSWER ANSWER ANSWER
*****************************************************************************************************************************************
*/
uint16_t grid_m16[0x40]={0};
static uint32_t AnswerOkCntr = 0;
/* Функція - обробник відповіді від ТК2
 * незалежно від вікна, з якого надсилалося повідомлення.*/
/* Результат - параметри, отримані від ТК2
 * Повертаємо - лічильник успішних обробок (без врахування повідомлень про помилки Модбас)*/
extern uint16_t DebugMode;
extern uint16_t Tk4screenMode;
extern uint16_t AmperageAmplitude;
extern uint16_t BAmperageAmplitude;
extern uint16_t CosFi;
int32_t f_tk5QPWD_RgAnswer(
		modbus_status_t ms,
		modbus_master_tx_msg_t *mb_tx_msg,
		modbus_master_rx_msg_t *mb_rx_msg,
		int incase){

    char user_msga[256]={0};
	memset(user_msga, 0,sizeof(user_msga));
	tk5ProcessCntr++;
	if (ms!=MODBUS_SUCCESS){
		/* Усі повідомлення, крім повідомлення про неналаштований СОМ-порт блокуємо.*/
		/* Вони корисні при відлагодженні, але шкідливі при роботі */
		/* При помилці Модбас користуємося низьким показником успіху Модбас    */
		switch(ms){
		/* Zкщо була виявлена помилка сеансу RTC  */
		case MODBUS_ERR_TIMEOUT:{
//			 snprintf (user_msga,USER_SESSAGE_SIZE," 0x%02X 0x%04X Не отримано відповіді за обумовлений час",
//					 mb_tx_msg->msg[1], mb_tx_msg->msg[2]*0x100+ mb_tx_msg->msg[3]);
			 break;
		}
		case MODBUS_COMPORT_ERR:{
			 snprintf (user_msga,USER_SESSAGE_SIZE," COM-порт належним чином не налаштовано   ");
			 break;
		}
		case MODBUS_ERR_RX:{
//			snprintf (user_msga,USER_SESSAGE_SIZE,"Message wasn't receive");
			break;
		}
		case MODBUS_ERR_TX:{
//			snprintf (user_msga,USER_SESSAGE_SIZE,"Message wasn't sent");
			break;
		}
		case MODBUS_ERR_SLAVE_ADDR:{
								  //123456789112345678921234567893123456789412345678951234567896123456789712345
//			snprintf (user_msga,USER_SESSAGE_SIZE,"Transmitted slave address doesn't correspond to the received slave address");
			break;
		}
		case MODBUS_ERR_CRC:{
//			snprintf (user_msga,USER_SESSAGE_SIZE,"Помилка CRC ");
			break;
		}
		case MODBUS_ERR_FC:{
//			snprintf (user_msga,USER_SESSAGE_SIZE,"Transmitted function code doesn't correspond to the received function code");
			break;
		}
		default:{
//			snprintf (user_msga,USER_SESSAGE_SIZE," Невизначена помилка RTU Modbus            ");
		}
		}
		tk5ProcessState=-1;
  	  char str[256]={0};
  	  snprintf (str,sizeof(str)," a3_tk5z.c Отримано повідомлення про помилку Модбас %d", ms);
  	  f_tk5Logging(str, sizeof(str));
	}
	/* Якщо помилки виявлено не було */
	while(ms==MODBUS_SUCCESS){
		/* Інкрементуємо лічильник успішних сесій */
		AnswerOkCntr++;
		/* Якщо не співпадає код команди */
		uint8_t MbCode=mb_rx_msg->msg[1];
		switch(mb_tx_msg->msg[1]){
		case 0x03:{
		/* ТК2 виявив помилку читання регістри і ми отримали повідомлення про помилку */
		if((mb_tx_msg->msg[1]==0x03)&&(MbCode==0x83)){
			if(mb_rx_msg->msg[2]==0x02){
				snprintf (user_msga,USER_SESSAGE_SIZE," Зчитування цього регістра не дозолено");
			}
			else{
				snprintf (user_msga,USER_SESSAGE_SIZE," Помилка протоколу Модбас        ");
			}
			tk5ProcessState=-1;
		}
		/* Це інформаційне повідомлення */
		else if((mb_tx_msg->msg[1]==0x03)&&(MbCode==0x03)){
        	uint16_t value=mb_rx_msg->msg[3]*0x100 + mb_rx_msg->msg[4];
        	/* Результат зчитування блоку регістрів */
        	if(incase==0x100){

    			uint8_t NBytes=0;
    			NBytes=mb_rx_msg->msg[2];
    			if(NBytes>1){
    				for(uint8_t i=0;i<NBytes/2;i++){
    					grid_m16[i] = mb_rx_msg->msg[3+2*i]*0x100+mb_rx_msg->msg[3+2*i+1];
    				}
    			}
        	}
        	else if(incase== 0xEEEE){
        		Tk4screenMode	=mb_rx_msg->msg[3]*0x100+mb_rx_msg->msg[4];
        		DebugMode		=mb_rx_msg->msg[5]*0x100+mb_rx_msg->msg[6];
        		AmperageAmplitude	=mb_rx_msg->msg[7]*0x100+mb_rx_msg->msg[8];
        		BAmperageAmplitude		=mb_rx_msg->msg[9]*0x100+mb_rx_msg->msg[10];
        		CosFi	=mb_rx_msg->msg[11]*0x100+mb_rx_msg->msg[12];
        	}
        	/************************************************************************************************/
        	/* Зчитування заводських налаштувань                                                            */
        	/************************************************************************************************/
        	//ПОСТІЙНА ЧАСУ НАГРІВАННЯ 														0Х0209	0Х1004	90
        	//#define ID_EDIT_HEATINGTIMECONSTANT						150
        	else if (incase==0x0209){ //             10        20        30        40        50        60        70        80        90
        		if(value<13){         //     1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
        			snprintf (user_msga,USER_SESSAGE_SIZE," Отримане від ТК2 значення ПОСТІЙНОЇ НАГРІВУ надто мале.\nМожливе часте безпідставне відключення");
        		}
        		else if(value>230){
        			snprintf (user_msga,USER_SESSAGE_SIZE," Отримане від ТК2 значення ПОСТІЙНОЇ НАГРІВУ надто велике\nДвигун недозахищений від перегріву");
        		}
        		else{
//        			fs.fsHeatingTimeConstant=value;
        		}
        	}
        	//ГРАНИЧНА КІЛЬКІСТЬ ПУСКІВ ЕД ПРОТЯГОМ 10ХВ									0X0216	0X1014   5
        	//#define ID_CMB_NUMBERSTARTLIMIT						    175
        	else if (incase==0x0216){
        		if((value>0)&&(value<10)){
 //       			fs.fsNumberStartLimit=value;
        		}
        		else{
        			snprintf (user_msga,USER_SESSAGE_SIZE," Отримане від ТК2 значення ЧИСЛА СТАРТІВ за 10хв підозріле");
        		}
        	}
        	/* якщо читали налаштування користувача з памяті контролера */
        	else if(incase==IDB_tk5READ_USERSETTING_FROM_MEMORY){
    			uint16_t value_i=0;
    			value_i = mb_rx_msg->msg[3+0x00] * 0x100 + mb_rx_msg->msg[3+0x01];
    			Tk5us.oper_mode = (oper_mode_t) value_i;
    			/* i тут підуть усі інші налаштування */
        	}
        	else if(incase==IDB_tk5READ_FACILITYSETTING_FROM_MEMORY){
    			uint16_t value_i=0;

    			/* i тут підуть усі інші налаштування */
    			uint16_t dec=0;
    			uint16_t fra=0;
//    			float amperageAclbr; 	// 0х2000
    			dec=mb_rx_msg->msg[3+0x00]*0x100+mb_rx_msg->msg[3+ 0x01];
    			fra=mb_rx_msg->msg[3+0x02]*0x100+mb_rx_msg->msg[3+ 0x03];
    			tk5fs.amperageAclbr=(float)dec+(float)fra/10000.f;

//    			float clbr_iA;			// 0х2002
    			dec=mb_rx_msg->msg[3+0x04]*0x100+mb_rx_msg->msg[3+ 0x05];
    			fra=mb_rx_msg->msg[3+0x06]*0x100+mb_rx_msg->msg[3+ 0x07];
    			tk5fs.clbr_iA=(float)dec+(float)fra/10000.f;

//    			float amperageBclbr; 	// 0х2004
    			dec=mb_rx_msg->msg[3+0x08]*0x100+mb_rx_msg->msg[3+ 0x09];
    			fra=mb_rx_msg->msg[3+0x0A]*0x100+mb_rx_msg->msg[3+ 0x0B];
    			tk5fs.amperageBclbr=(float)dec+(float)fra/10000.f;

//    			float clbr_iB;			// 0х2006
    			dec=mb_rx_msg->msg[3+0x0C]*0x100+mb_rx_msg->msg[3+ 0x0D];
    			fra=mb_rx_msg->msg[3+0x0E]*0x100+mb_rx_msg->msg[3+ 0x0F];
    			tk5fs.clbr_iB=(float)dec+(float)fra/10000.f;

//    			float amperageCclbr; 	// 0х2008
    			dec=mb_rx_msg->msg[3+0x10]*0x100+mb_rx_msg->msg[3+ 0x11];
    			fra=mb_rx_msg->msg[3+0x12]*0x100+mb_rx_msg->msg[3+ 0x13];
    			tk5fs.amperageCclbr=(float)dec+(float)fra/10000.f;

//    			float clbr_iC;			// 0х200A
    			dec=mb_rx_msg->msg[3+0x14]*0x100+mb_rx_msg->msg[3+ 0x15];
    			fra=mb_rx_msg->msg[3+0x16]*0x100+mb_rx_msg->msg[3+ 0x17];
    			tk5fs.clbr_iC=(float)dec+(float)fra/10000.f;

//    			fSet.voltageCclbr		//0x200C
    			dec=mb_rx_msg->msg[3+0x18]*0x100+mb_rx_msg->msg[3+ 0x19];
    			fra=mb_rx_msg->msg[3+0x1A]*0x100+mb_rx_msg->msg[3+ 0x1B];
    			tk5fs.voltageCclbr=(float)dec+(float)fra/10000.f;

//    			float clbr_uC;			// 0х200E
    			dec=mb_rx_msg->msg[3+0x1C]*0x100+mb_rx_msg->msg[3+ 0x1D];
    			fra=mb_rx_msg->msg[3+0x1E]*0x100+mb_rx_msg->msg[3+ 0x1F];
    			tk5fs.clbr_uC=(float)dec+(float)fra/10000.f;

//    			uint16_t DeviceMode;     //0x1010 Типорозмір контролера 1...5
    			uint16_t value =0;
    			value=mb_rx_msg->msg[3+0x20]*0x100+mb_rx_msg->msg[3+0x21];
    			tk5fs.DeviceMode   =value;

//    			uint16_t Faznost;		 //0x1011
    			value=mb_rx_msg->msg[3+0x22]*0x100+mb_rx_msg->msg[3+0x23];
    			tk5fs.Faznost   =value;



        	}


        	else{}
        	tk5ProcessState=1;
		}
		/* Не 0х03 і не 0х83 */
		else{
			snprintf (user_msga,USER_SESSAGE_SIZE," Невизначена помилка RTU Modbus            ");
        }



		}
		break;
		case 0x06:{
	    if((mb_tx_msg->msg[1]==0x06)&&(mb_rx_msg->msg[1]==0x86)){
	    	if(mb_rx_msg->msg[2]==0x02){
	    		snprintf (user_msga,USER_SESSAGE_SIZE," Запис регістра не підтримується ");
	    		tk5ProcessState=-1;
	    	}
	    	else{
	    		snprintf (user_msga,USER_SESSAGE_SIZE," Помилка протоколу Модбас              ");
	    		tk5ProcessState=-1;
	    	}

	    }
	    /* Якщо це був запис регістра */
	    else if ((mb_tx_msg->msg[1]==0x06)&&(mb_rx_msg->msg[1]==0x06)){
				uint16_t regNumber_Tx = mb_tx_msg->msg[2]*0x100 + mb_tx_msg->msg[3];
				uint16_t regValue_Tx = mb_tx_msg->msg[4]*0x100 + mb_tx_msg->msg[5];
				uint16_t regNumber_Rx = mb_rx_msg->msg[2]*0x100 + mb_rx_msg->msg[3];
				uint16_t regValue_Rx = mb_rx_msg->msg[4]*0x100 + mb_rx_msg->msg[5];
				if((regNumber_Tx==regNumber_Rx)&&(regValue_Tx==regValue_Rx)){
					tk5ProcessState=1;

				}
				else{
					snprintf (user_msga,USER_SESSAGE_SIZE," Помилка Модбас запису регістра RTx");
					tk5ProcessState=-1;
				}
	    }

        else{
        	snprintf (user_msga,USER_SESSAGE_SIZE," Відповідь не відповідає формату Модбас");
        	tk5ProcessState=-1;
		}
		}//case 0x06:{
		break;
		case 0x10:{
		    if((mb_tx_msg->msg[1]==0x10)&&(mb_rx_msg->msg[1]==0x90)){
		    	if(mb_rx_msg->msg[2]==0x02){
		    		snprintf (user_msga,USER_SESSAGE_SIZE," Запис регістрiв не підтримується ");
		    		tk5ProcessState=-1;
		    	}
		    	else{
		    		snprintf (user_msga,USER_SESSAGE_SIZE," Помилка протоколу Модбас              ");
		    		tk5ProcessState=-1;
		    	}

		    }
		    /* Якщо це був запис регістрів блоком  */
		    else if ((mb_tx_msg->msg[1]==0x10)&&(mb_rx_msg->msg[1]==0x10)){
				    uint16_t txStartingAddress = mb_tx_msg->msg[2]*0x100 + mb_tx_msg->msg[2];
				    uint16_t rxStartingAddress = mb_rx_msg->msg[2]*0x100 + mb_rx_msg->msg[2];
					uint16_t txQuantityOfRegisters = mb_tx_msg->msg[4]*0x100 + mb_tx_msg->msg[5];
					uint16_t rxQuantityOfRegisters = mb_rx_msg->msg[4]*0x100 + mb_rx_msg->msg[5];


					if((txStartingAddress == rxStartingAddress)&&
							(txQuantityOfRegisters == rxQuantityOfRegisters)){
						/* Все Ок       */
						tk5ProcessState=1;
					}
					else{
						snprintf (user_msga,USER_SESSAGE_SIZE," Помилка Модбас запису регістрів блоком RTx");
						tk5ProcessState=-1;
					}
		    }

	        else{
	        	snprintf (user_msga,USER_SESSAGE_SIZE," Відповідь не відповідає формату Модбас");
	        	tk5ProcessState=-1;
			}
		}
		break;

		case 0x11:{
			memset(&user_msga,0,sizeof(user_msg5));
			  tk5ProcessState=2;
	//        Server ID device specific
	    	  user_msga[0]=mb_rx_msg->msg[3];
	    	  user_msga[1]=mb_rx_msg->msg[4];
	    	  user_msga[2]=mb_rx_msg->msg[5];
	    	  user_msga[3]=mb_rx_msg->msg[6];

	    	  test_message_cntr_debug++;
	    	  char str[256]={0};
	    	  snprintf (str,sizeof(str)," a3_tk5z.c: Отримано тестову відповідь %d від ТК5", test_message_cntr_debug);
	    	  f_tk5Logging(str, sizeof(str));

	//        Run Indicator Status 1 Byte 0x00 = OFF, 0xFF = ON
	//        Additional Data
		}
		break;

		case 0x91:{
	    	  char str[256]={0};
	    	  snprintf (str,sizeof(str)," a3_tk5z.c: Отримано повідомлення про помилку від сервера");
	    	  f_tk5Logging(str, sizeof(str));
		}
		break;
		default:{}
		}
        /* Вихід з блоку */
        ms=MODBUS_UNKNOWN_ERROR;/* Вихід з WHILE */



	}//MODBUS_SUCCESS

    if(  strlen(user_msga) != 0){
    	memcpy(user_msg5,user_msga,5);
    }
    /* Це - розблокування роботи черги, раком поки що..*/
    mb_rx_msg->length=0;
    /* Повертаємо стан обробки відповіді від контролера*/
    return tk5ProcessState;
}





int f_Set5TxReadReg(uint16_t addr,modbus_master_tx_msg_t *tx){
	tk5_Addr= f_GetTk5ModbussAddress(ID_tk5_EDIT_ADDRESS);if(tk5_Addr<0){DestroyWindow(hWndTk5);}
	memset(tx,0,sizeof(modbus_master_tx_msg_t));
	tx->msg[0]=tk5_Addr;
	tx->msg[1]=0x03;
	tx->msg[2]=addr/0x100;
	tx->msg[3]=addr%0x100;//
	tx->msg[4]=0x00;
	tx->msg[5]=0x01;
	tx->length=6;
	return tx->length;
}
int f_Set5TxWriteReg(uint16_t addr, uint16_t value,modbus_master_tx_msg_t *tx){
	tk5_Addr= f_GetTk5ModbussAddress(ID_tk5_EDIT_ADDRESS);if(tk5_Addr<0){DestroyWindow(hWndTk5);}
	memset(tx,0,sizeof(modbus_master_tx_msg_t));
	tx->msg[0]=tk5_Addr;
	tx->msg[1]=0x06;
	tx->msg[2]=addr/0x100;
	tx->msg[3]=addr%0x100;//
	tx->msg[4]=value/0x100;
	tx->msg[5]=value%0x100;
	tx->length=6;
	return tx->length;
}

int16_t f_GetTk5ModbussAddress(int ID_EditBox){
	memset(&user_msg5,0,sizeof(user_msg5));
	char inaddr=0;
	/* Зчитуємо регістр адреси, це завжди HEX*/
	float2_t fl={0};
	int err = f_checkEdit(ID_EditBox,hWndtk5,&fl);
	if(err != 0){f_valueEditMessageBox(err);}
	inaddr=fl.d;
	return inaddr;
}


/* Ця функція перевіряє відповідність адреси Модбас
 * поточним вимогам ТОВ"Техноконт"*/
BOOL f_Get_tk5ModbussAddressOk(uint8_t mb_addr){
	BOOL condition = ((mb_addr==1)||(mb_addr==2));
	return condition;
}




void f_tk5Logging(char *str, size_t n_str){
	/* Якщо була подія, і було сформовано повідомлення для логу -
	* записуємо його в файл */
	if((str != NULL)&&(strlen(str) != 0)){
		/* Перевіряємо, чи змінилася дата, і якщо змінилася -
		* змінюємо ім'я файла, а отже i його хендлер*/
		fp_tk5LogFile= f_tk5check_logFilename(fp_tk5LogFile);
		SYSTEMTIME  SystemTime ={0};
		LPSYSTEMTIME lpSystemTime=&SystemTime;
		GetSystemTime(lpSystemTime);

		char wr[512+256]={0};
		snprintf(wr,23,"%02d:%02d:%02d     ",lpSystemTime->wHour+2,lpSystemTime->wMinute,lpSystemTime->wSecond);
		/* Отримуємо повне ім'я файла для запису логів  */
		strcat(wr,str);
		fprintf(fp_tk5LogFile,"\n%s",wr);
		memset(str,0,sizeof(n_str));

	}

}

int f_read_facilitysetting_from_tk5memory(HWND hwnd){
	modbus_master_tx_msg_t mbTxMsg={0};
	mbTxMsg.msg[0]=0x01;
	mbTxMsg.msg[1]=0x03;
	//(rAddress==0x2020)&&(NumberOfRegs==0x20)){
	mbTxMsg.msg[2]=0x20;
	mbTxMsg.msg[3]=0x20;
	mbTxMsg.msg[4]=0x00;
	mbTxMsg.msg[5]=0x20;
	mbTxMsg.length=6;
	/* Ставимо повідомлення в чергу на обробку   */
	f_set_tkqueue(
			&tk5Queue,
			DISABLE,
			hwnd,
			IDB_tk5READ_FACILITYSETTING_FROM_MEMORY,
			&mbTxMsg,
			1000
		);
}

int f_read_usersetting_from_tk5memory(HWND hwnd){
	modbus_master_tx_msg_t mbTxMsg={0};
	mbTxMsg.msg[0]=0x01;
	mbTxMsg.msg[1]=0x03;
	//(rAddress==0x2020)&&(NumberOfRegs==0x20)){
	mbTxMsg.msg[2]=0x20;
	mbTxMsg.msg[3]=0x00;
	mbTxMsg.msg[4]=0x00;
	mbTxMsg.msg[5]=0x20;
	mbTxMsg.length=6;
	/* Ставимо повідомлення в чергу на обробку   */
	f_set_tkqueue(
			&tk5Queue,
			DISABLE,
			hwnd,
			IDB_tk5READ_USERSETTING_FROM_MEMORY,
			&mbTxMsg,
			1000
		);
}
