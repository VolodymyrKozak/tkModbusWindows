/*
 * demo_smi2.c
 *
 *  Created on: 10 вер. 2019 р.
 *      Author: KozakVF
 */
#include "demo_smi2.h"
#include "windows.h"

float float_a = (float)(40000);




/* Опитали всі можливі теоретично Модбас-адреси,
 * отримали перелік адрес Модбас, які відкликаються на команду 17*/
uint8_t smi2_addresses[256]={0};




void f_start_test_smi2(void){
	uint8_t j=0;
	for(uint8_t i=0;i<0x100;i++){
		oven_session_status_t oss = oven_UnknownERR;
		oss= f_ovenSMI2_TASKS_17 (i);
		if(oss!=oven_WrongDevice){
			smi2_addresses[j]=i;
			j++;
			if(i==0x20){
				float_a=0;
			}
		}
		oss = oven_UnknownERR;
	}
}
/* Виявили дві адреси, що відкликаються.. Зафіксуємо..  */
#define SMI1_DEFAULT_MODBUSS_ADRESS_1 0x0F
#define SMI1_DEFAULT_MODBUSS_ADRESS_2 0x10

void f_demo_smi(void){
	oven_session_status_t demo_status = oven_UnknownERR;
	while(demo_status == oven_UnknownERR){
//        Sleep(2500);
        uint16_t smi15_r[40]={0};
        uint16_t smi16_r[40]={0};
        for(uint8_t i=29;i<33;i++){
        	demo_status = f_ovenSMI2_TASKS_03_1 (SMI1_DEFAULT_MODBUSS_ADRESS_1, i, &smi15_r[i]);
        	demo_status = f_ovenSMI2_TASKS_03_1 (SMI1_DEFAULT_MODBUSS_ADRESS_2, i, &smi16_r[i]);
        }
//        demo_status = f_ovenSMI2_TASKS_06 (SMI1_DEFAULT_MODBUSS_ADRESS_1, 16, 1000);
//        demo_status = f_ovenSMI2_TASKS_06 (SMI1_DEFAULT_MODBUSS_ADRESS_2, 16, 1000);
//
//        demo_status = f_ovenSMI2_TASKS_03 (SMI1_DEFAULT_MODBUSS_ADRESS_1, 16, &smi15_r16);
//        demo_status = f_ovenSMI2_TASKS_03 (SMI1_DEFAULT_MODBUSS_ADRESS_2, 16, &smi16_r16);

	    demo_status=f_ovenSMI2_Init (SMI1_DEFAULT_MODBUSS_ADRESS_1);
	    if(demo_status!=oven_OK){break;}
	    demo_status=f_ovenSMI2_Init (SMI1_DEFAULT_MODBUSS_ADRESS_2);
	    if(demo_status!=oven_OK){break;}
//	    f_ovenSMI2_Init_String (SMI1_DEFAULT_MODBUSS_ADRESS_1);
//	    oven_session_status_t f_ovenSMI2_TASKS_16 (
//	    		uint8_t ovenSMI_ADDRESS,  /* Модбас-адреса прилада*/
//				uint8_t register_to_BEGIN,/* Регістр, з якого починати писати */
//	    		uint8_t N_of_registers,   /* Кількість регістрів, які треба писати */
//	    		uint8_t N2_of_bytes,      /* Кількість байт, як треба писати      */
//				uint8_t *str_to_oven);    /* Масив байт або (uint8_t*) з масивом 16-значних числел*/
        /*                     I    d    L    E*/
//	    uint8_t cmi_str[4]={"0x31,0x64,0x6C,0x65"};
//	    uint8_t cmi_str1[4]={0x30,0x31,0x32,0x33,0x20,0x20,0x20,0x20};
	    uint8_t cmi_str1[8]={"9876."};
	    uint8_t cmi_str2[8]={0x33,0x35,0x36,0x37,0x20,0x20,0x20,0x20};
	    uint8_t cmi_strSet[8]= {"Set ."};
	    uint8_t cmi_strIdle[8]={"1dLE."};
	    uint8_t cmi_strZero[8]={0};
	    uint8_t cmi_str_[8]={0x2d,0x2d,0x2d,0x2d,0x20,0x20,0x20,0x20};
	    Sleep(200);
	    f_ovenSMI2_DisplayString8 (SMI1_DEFAULT_MODBUSS_ADRESS_1, cmi_strZero);
	    Sleep(200);
	    f_ovenSMI2_DisplayString8 (SMI1_DEFAULT_MODBUSS_ADRESS_2, cmi_strZero);
	    Sleep(200);
        for(uint8_t i=29;i<33;i++){
        	demo_status = f_ovenSMI2_TASKS_03_1 (SMI1_DEFAULT_MODBUSS_ADRESS_1, i, &smi15_r[i]);
        	demo_status = f_ovenSMI2_TASKS_03_1 (SMI1_DEFAULT_MODBUSS_ADRESS_2, i, &smi16_r[i]);
        }
        Sleep(200);
		f_ovenSMI2_DisplayString8 (SMI1_DEFAULT_MODBUSS_ADRESS_1, cmi_str_);
		Sleep(200);
        for(uint8_t i=29;i<33;i++){
        	demo_status = f_ovenSMI2_TASKS_03_1 (SMI1_DEFAULT_MODBUSS_ADRESS_1, i, &smi15_r[i]);
        	demo_status = f_ovenSMI2_TASKS_03_1 (SMI1_DEFAULT_MODBUSS_ADRESS_2, i, &smi16_r[i]);
        }
        Sleep(200);
		f_ovenSMI2_DisplayString8 (SMI1_DEFAULT_MODBUSS_ADRESS_2, cmi_str_);
		Sleep(200);
	    f_ovenSMI2_DisplayString4 (SMI1_DEFAULT_MODBUSS_ADRESS_1, cmi_strSet);
	    Sleep(200);
		f_ovenSMI2_DisplayString4 (SMI1_DEFAULT_MODBUSS_ADRESS_2, cmi_strIdle);
		Sleep(200);
		f_ovenSMI2_DisplayString8 (SMI1_DEFAULT_MODBUSS_ADRESS_1, cmi_str_);
		Sleep(200);
		        for(uint8_t i=29;i<33;i++){
		        	demo_status = f_ovenSMI2_TASKS_03_1 (SMI1_DEFAULT_MODBUSS_ADRESS_1, i, &smi15_r[i]);
		        	demo_status = f_ovenSMI2_TASKS_03_1 (SMI1_DEFAULT_MODBUSS_ADRESS_2, i, &smi16_r[i]);
		        }

		Sleep(200);
		f_ovenSMI2_DisplayString8 (SMI1_DEFAULT_MODBUSS_ADRESS_2, cmi_str_);
		Sleep(200);


	    f_ovenSMI2_DisplayString4 (SMI1_DEFAULT_MODBUSS_ADRESS_1, cmi_str2);
	    Sleep(200);
	    f_ovenSMI2_DisplayString4 (SMI1_DEFAULT_MODBUSS_ADRESS_2, cmi_str1);
	    Sleep(200);
	    f_ovenSMI2_DisplayString8 (SMI1_DEFAULT_MODBUSS_ADRESS_1, cmi_str1);
	    Sleep(200);
	    f_ovenSMI2_DisplayString8 (SMI1_DEFAULT_MODBUSS_ADRESS_2, cmi_str2);
	    Sleep(200);
	    f_ovenSMI2_DisplayString8 (SMI1_DEFAULT_MODBUSS_ADRESS_1, cmi_strSet);
	    Sleep(200);
	    f_ovenSMI2_DisplayString8 (SMI1_DEFAULT_MODBUSS_ADRESS_2, cmi_strIdle);
	    Sleep(200);
	   while(1){
        f_oventest(0);
	   }

	    demo_status = oven_OK;
	}
}

//void f_demo_smi2(void){
//
//
//		vect_float.value[0] = float_a;
//		vect_float.value[1] = float_a*0.5;
//		f_ovenSMI2_DisplayBroadCast (	0x20,
//										2,
//										&vect_float
//									);
//	if (float_a > 0){
//		if (float_a > 0.0001){
//			float_a= float_a*((float)(0.7));
//		}
//		else {
//			(float_a = float_a *(-1.0));
//		}
//	}
//	if(float_a<0){
//		if (float_a>-10999){
//		float_a= float_a*((float)(1.3));
//		}
//		else {
//			(float_a = 10000);
//
//			f_ovenSMI2_TASKS_06(0x20, 0x0023, 0x00BB); /* включить мигание		*/
//		}
//	}
//
//
//		uint16_t value_from_register = 0;
//		oven_session_status_t oven_session_status = oven_UnknownERR;
//		oven_session_status = f_ovenSMI2_TASKS_17 (0x20);
//		oven_session_status = oven_UnknownERR;
//		oven_session_status = f_ovenSMI2_TASKS_03 (0x20, 19, &value_from_register);
//		oven_session_status = oven_UnknownERR;
//		oven_session_status = f_ovenSMI2_TASKS_03 (0x20, 8, &value_from_register);
//}
//
//
//#include <stdio.h>
//#include "stdint.h"
//#include <string.h>
//#include <stdlib.h>
//#ifdef MODBUS_MASTER_for_OVEN_INDICATOR
///* Тут писати функції конкретних проектів з використанням
// * OVEN_INDICATOR
// * */
//
//#ifdef MODBUS_MASTER_for_OVEN_INDICATOR_TESTING
//	static float k[11]  = {10, 1, 0.1, 0.01, 0.001,0.0001, -0.0001,-0.001,-0.01,-0.1, -1};
//	static uint8_t tmpov_str[17]={0};
//
//	static void ctr_oprint(uint8_t *str, float float_a);


	/* Демонстрація управління контролером роботою ОВЕН СМИ
	 * Ты самі числа виводяться і на ОВЕН СМИ і на дисплей.
	*/

oven_session_status_t f_oventest(uint32_t ov_counter){


		oven_session_status_t fSessionStatus=oven_UnknownERR;
		while(fSessionStatus==oven_UnknownERR){
			   float a1=10000.0f;
		       float a2=0;
		       while (a1>0.001){
		    	    fSessionStatus=f_ovenSMI2_DisplayFloat1(SMI1_DEFAULT_MODBUSS_ADRESS_1,a1);
					fSessionStatus=f_ovenSMI2_DisplayFloat(SMI1_DEFAULT_MODBUSS_ADRESS_1,a1);
					if(!((fSessionStatus==oven_OK)||(fSessionStatus==oven_answerTimeout))){break;}
					a2=10.0/a1;
					fSessionStatus = f_ovenSMI2_DisplayFloat1(SMI1_DEFAULT_MODBUSS_ADRESS_2,a2);
					fSessionStatus = f_ovenSMI2_DisplayFloat(SMI1_DEFAULT_MODBUSS_ADRESS_2,a2);
					//ctr_oprint(tmpov_str,a1);

				    a1=a1*0.99f;
				    if(!((fSessionStatus==oven_OK)||(fSessionStatus==oven_answerTimeout))){break;}
		       }

		       a1=-0.01;
		       while (a1>-999){
		       		fSessionStatus=f_ovenSMI2_DisplayFloat (SMI1_DEFAULT_MODBUSS_ADRESS_1,a1);
		       		if(!((fSessionStatus==oven_OK)||(fSessionStatus==oven_answerTimeout))){break;}
		       			a2=10.0/a1;

		       		fSessionStatus = f_ovenSMI2_DisplayFloat (SMI1_DEFAULT_MODBUSS_ADRESS_2,a2);
		       					//ctr_oprint(tmpov_str,a1);

		       		a1=a1*1.1f;
		       		if(!((fSessionStatus==oven_OK)||(fSessionStatus==oven_answerTimeout))){break;}
		       	}
		       /* ест широкоформатного виводу масиву чисел з плаваючою тчкою */
		       float A[2]={1000.0f, 0};
		       /* виводимо додатні числа */
		       while (A[0]>0.001){
		    	   A[1]=10.0f/A[0];
		    	   fSessionStatus = f_ovenSMI2_DisplayBroadCast (
		    			   SMI1_DEFAULT_MODBUSS_ADRESS_1,/* Модбас адреса першого девайса */
						   2,                            /* Кількість девайсів            */
						   A                            /* показчик на масив чисел float */
						   );
					if(fSessionStatus!=oven_OK){break;}
//					uint16_t idelay = (uint16_t)A[0];
//					Sleep(idelay);
				    A[0]=A[0]*0.99f;

		       }
		       /* Виводимо відємні числа */
		       A[0]=-0.001;
		       while (A[0]>-999){
		    	   A[1]=10.0f/A[0];
		    	   fSessionStatus = f_ovenSMI2_DisplayBroadCast (
		    			   SMI1_DEFAULT_MODBUSS_ADRESS_1,/* Модбас адреса першого девайса */
						   2,                            /* Кількість девайсів            */
						   A                            /* показчик на масив чисел float */
						   );
					if(fSessionStatus!=oven_OK){break;}
					A[0]=A[0]*1.01f;

		       	}
		       if(!((fSessionStatus==oven_OK)||(fSessionStatus==oven_answerTimeout))){break;}
		}

		return fSessionStatus;
	}

//	oven_session_status_t f_oventest1(void){
////		volatile uint32_t dursession=0;
////		uint32_t ticksess=HAL_GetTick();
////		for (uint8_t i=0;i<10;i++){
////		    SYSTEMTIME SystemTime={0};
////		    GetSystemTime(&SystemTime);
////			uint32_t seed = SystemTime.wMilliseconds;
////			srand (seed);
////			uint32_t randseedh = rand()%10000;
////			uint8_t rowh=rand()%11;
////			float a1 = (float)(randseedh)*k[rowh];
//
//       }
//	}
//
//	vector_of_float_t vect = {{0.001, 0.002, 0.003, 0.004}};
//	void f_oventest2(void){
//		f_ovenSMI2_DisplayBroadCast (	16,
//										3,
//										&vect);
//		ctr_oprint(tmpov_str,vect.value[0]);
////		WH1602b3_WriteHighString(tmpov_str);
//		ctr_oprint(tmpov_str,vect.value[2]);
////		WH1602b3_WriteLowString(tmpov_str);
//		vect.value[0]=vect.value[0]+0.001;
//		vect.value[2]=vect.value[2]+0.001;
//
//	}
//
//	void f_oventest3(void){
//		SYSTEMTIME SystemTime={0};
//		GetSystemTime(&SystemTime);
//		uint32_t seed = SystemTime.wMilliseconds % 0x100000000;
//
//		srand (seed);
//		uint32_t randseedh = rand()%10000;
//		uint8_t rowh=rand()%11;
//		volatile float a1 = (float)(randseedh)*k[rowh];
//		f_ovenSMI2_DisplayFloat (15,a1);
//		ctr_oprint(tmpov_str,a1);
////		WH1602b3_WriteHighString(tmpov_str);
//		Sleep(20);
//
//		uint32_t randseedl = rand()%10000;
//		uint8_t rowl=rand()%11;
//		volatile float a2 = (float)(randseedl)*k[rowl];
//		f_ovenSMI2_DisplayFloat (16,a2);
//		ctr_oprint(tmpov_str,a2);
////		WH1602b3_WriteLowString(tmpov_str);
//		Sleep(20);
//	}
//
//
//
//static void ctr_oprint(uint8_t *str, float float_a){
//	volatile uint32_t int_ttt=0;
//	volatile float fl = 0;
//	volatile uint32_t fra_ttt=0;
//	if (float_a>=0){
//		int_ttt=(uint32_t)float_a;
//		fl = (float_a-int_ttt)*1000;
//		fra_ttt=(uint32_t)(fl);
//		memset (&str, 0, sizeof(str));
//		snprintf((char*)(str), 17, "t = %4d.%03d ",int_ttt, fra_ttt);/*#include "string.h"*/
//
//
//	}
//	else {
//
//		float ttt=-float_a;
//		int_ttt=(uint32_t)ttt;
//		fl = (ttt-int_ttt)*1000;
//		fra_ttt=(uint32_t)(fl);
//	}
//	snprintf((char*)(&tmpov_str), 17, "t =-%4d.%03d ",int_ttt, fra_ttt);/*#include "string.h"*/
//}
//#endif // MODBUS_MASTER_for_OVEN_INDICATOR_TESTING
//#endif // MODBUS_MASTER_for_OVEN_INDICATOR
//
