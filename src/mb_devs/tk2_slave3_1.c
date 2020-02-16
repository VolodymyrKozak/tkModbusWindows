/*
 * tk2_slave3_1.c
 *
 *  Created on: 12 лист. 2019 р.
 *      Author: KozakVF
 */
#include "stdint.h"
#include "tk2_slave3_1.h"
//uint8_t aRX[255]={0};
//uint8_t iRx=0;

/* Карта регистров */
#define iR0000 5
uint16_t R0000[iR0000]={0};
#define iR0100 12
uint16_t R0100[iR0100]={0};
#define iR1000 1000
uint16_t R1000[iR1000]={0};
/* ответ */
uint8_t TxRsp[125]={0};
uint8_t iTxRsp=0;

/* Read Holding Registers - читати регістри */
static int8_t mb_in03(uint8_t *aRx,uint8_t iRx);

/* Write Single Register писати один регістр */
static int8_t mb_in06(uint8_t *aRx,uint8_t iRx);

/* Write Multiple registers  писати регістри */
static int8_t mb_in16(uint8_t *aRx,uint8_t iRx);
static void mb_in03_dop(
		uint16_t StartingAddress,
		uint16_t QuantityOfRegisters,
		uint8_t *aRx,
		uint8_t iRx,
		uint16_t *pRegs
		);
static void mb_in16_dop(
		uint16_t StartingAddress,
		uint16_t QuantityOfRegisters,
		uint8_t *aRx,
		uint8_t iRx,
		uint16_t *pRegR
		);


/* Функція - розводящий :)*/
int8_t mb_in(uint8_t *aRx, uint8_t iRx){

	if(aRx[0]!=(uint8_t)R0100[1]){
		return -2;/* не наш модбас-адрес */
	}
	/* Якщо модбас-адреса наша - перевіряємо код команди */
	int8_t status=-1;
	TxRsp[0]=aRx[0];/* адреса отримувача відповіді - наша адреса*/
	switch(aRx[1]){
	/* Read Holding Registers - читати регістри */
	case 0x03:{
		status=mb_in03(aRx,iRx);
	}
	break;
	/* Write Single Register писати один регістр */
	case 0x06:{
		status=mb_in06(aRx,iRx);
	}
	break;
	/* Write Multiple registers  писати регістри */
	case 0x10:{

		uint16_t QuantityOfRegisters=aRx[5]*0x100+aRx[6];
		if ((uint8_t)QuantityOfRegisters != aRx[7]){
			/* Код команди - якщо помилка +0х80 */
			TxRsp[1]= 0x80+aRx[1];
			TxRsp[2]= 0x03;/* Код помилки */
			iTxRsp=5;
		}
		else{
			status=mb_in16(aRx,iRx);
		}
	}
	break;
	case 0x11:{
		TxRsp[1]=aRx[1];/* 0х11 */
		TxRsp[2]=10; /* Кількість байт в відповіді */
		TxRsp[3]='T';
		TxRsp[4]='K';
		TxRsp[5]='-';
		TxRsp[6]='2';
		TxRsp[7]= (uint8_t)R0100[1];  /* Стан двигуна Off/On , регістр 0х0101 */
		iTxRsp=10;
		status=0;
	}
	break;
	default:{
		/* Код команди - якщо помилка +0х80 */
		TxRsp[1]= 0x80+aRx[1];
		TxRsp[2]= 0x01;
		iTxRsp=5;
		status=0;
		}

	}
	if(status==0){
	/* Додаємо CRC
	 * відправляємо відповідь */
	}
	return status;
}
/* Read Holding Registers - читати регістри */
static int8_t mb_in03(uint8_t *aRx,uint8_t iRx){
	int8_t status03=-1;
	uint16_t StartingAddress = aRx[3]*0x100+aRx[4];
	uint16_t QuantityOfRegisters=aRx[5]*0x100+aRx[6];
	/* Якщо початкова адреса і кількість регістрів коректна */
	if(((StartingAddress>=0x0100)&&(StartingAddress<0x0100+iR0100))){
		mb_in03_dop(
				StartingAddress,
				QuantityOfRegisters,
				aRx,
				iRx,
				R0100
				);
		status03=0;
	}
	else if(((StartingAddress>=0x1000)&&(StartingAddress<0x1000+iR0100))){
		mb_in03_dop(
				StartingAddress,
				QuantityOfRegisters,
				aRx,
				iRx,
				R1000
				);
		status03=0;
	}
	/*  Якщо початкова адреса регістрів коректна */
	else{
		/* Код команди - якщо помилка +0х80 */
				TxRsp[1]= 0x80+aRx[1];
				TxRsp[2]= 0x02;/* Не підтримуються така адреса регістра */
				iTxRsp=5;
				status03=0;
	}
	return status03;
}
/* Write Single Register писати один регістр */
static int8_t mb_in06(uint8_t *aRx,uint8_t iRx){

	uint16_t StartingAddress = aRx[3]*0x100+aRx[4];
	uint16_t value = aRx[5]*0x100+aRx[6];
	uint16_t i16=0;
		/* Якщо початкова адреса  коректна */
		if((StartingAddress>=0x0000)&&(StartingAddress<(0x0000+iR0000))){
			i16=StartingAddress-0x0000;
			switch(StartingAddress){
			case 0x0001:{
				value | 0x00FF;
				if((value | 0x00FF)==0x00FF){
					/* команда включить двигатель                  */
					/* задание такущее <- задание новое r 0x0101 = */
					R0100[1]=R0100[1] | 0xFF00;
				}
				if((value | 0xFF00)==0xFF00){

				}
				}
			}
			R0000[i16]=value;
		}
		else if(((StartingAddress>=0x1000)&&(StartingAddress<(0x1000+iR0100)))){
			i16=StartingAddress-0x1000;
			R1000[i16]=value;
		}
		/*  Якщо початкова адреса регістрів коректна */
		else{
			/* Код команди - якщо помилка +0х80 */
					TxRsp[1]= 0x80+aRx[1];
					TxRsp[2]= 0x02;/* Не підтримуються така адреса регістра */
					iTxRsp=5;
		}
	return 0;
}
/* Write Multiple registers  писати регістри
 * працює лиша для уставок */
static int8_t mb_in16(uint8_t *aRx,uint8_t iRx){
	uint16_t StartingAddress = aRx[3]*0x100+aRx[4];
	uint16_t QuantityOfRegisters=aRx[5]*0x100+aRx[6];

	/* Якщо початкова адреса і кількість регістрів коректна
	 * запис параметрів, які не вимагають паролю         */
	if((StartingAddress>=0x1000)&&(StartingAddress<0x1005)){
		if((StartingAddress+QuantityOfRegisters-1)<0x1005){
				mb_in16_dop(
						StartingAddress,
						QuantityOfRegisters,
						aRx,
						iRx,
						R1000
						);
		}
		else{
			TxRsp[1]= 0x80+aRx[1];
			TxRsp[2]= 0x02;/* Не підтримуються така адреса регістра */
			iTxRsp=5;
		}
	}
	/* Запис параметрів які вимагають паролю */
	else if (StartingAddress>=0x1005){
			/* Перевіряємо чи був представлений пароль */
			if(R0000[2]==0xFFFF){
				if(StartingAddress<0x1009){
					if((StartingAddress+QuantityOfRegisters-1)<0x100B){
						mb_in16_dop(
								StartingAddress,
								QuantityOfRegisters,
								aRx,
								iRx,
								R1000
								);
					}
				}
				/* більше arr9	*/
				else if((StartingAddress>=0x1014)&&(StartingAddress<0x101F)){
					if((StartingAddress+QuantityOfRegisters-1)<0x101F){
							mb_in16_dop(
									StartingAddress,
									QuantityOfRegisters,
									aRx,
									iRx,
									R1000
									);
					}
				}
				else{

					TxRsp[1]= 0x80+aRx[1];
					TxRsp[2]= 0x02;/* Не підтримуються така адреса регістра */
					iTxRsp=5;

				}

			}
			/* Пароль не було введено, повідомлення про помилку*/
			else{
				/* Код команди - якщо помилка +0х80 */
				TxRsp[1]= 0x80+aRx[1];
				TxRsp[2]= 0x06;/* Помилка коду дуступа. Не введено пароль */
				iTxRsp=5;
			}
		}
		/**/
		else{
			TxRsp[1]= 0x80+aRx[1];
			TxRsp[2]= 0x02;/* Не підтримуються така адреса регістра */
			iTxRsp=5;
		}


	return 0;
}

static void mb_in03_dop(
		uint16_t StartingAddress,
		uint16_t QuantityOfRegisters,
		uint8_t *aRx,
		uint8_t iRx,
		uint16_t *pRegs
		){
	TxRsp[1]= aRx[1];
	TxRsp[2]= (uint8_t)(QuantityOfRegisters*2);
	for(uint8_t i=0;i<QuantityOfRegisters;i++){
		uint16_t r=pRegs[StartingAddress+i];
		TxRsp[3+2*i]=r/0x100;
		TxRsp[3+2*i+1]=r%0x100;
	}
	/* Розмір відповіді і байтах   */
	iTxRsp=2*QuantityOfRegisters+3+2;

}

static void mb_in16_dop(
		uint16_t StartingAddress,
		uint16_t QuantityOfRegisters,
		uint8_t *aRx,
		uint8_t iRx,
		uint16_t *pRegR
		){
	/* записуємо регістри */
	for(uint8_t i=0;i<QuantityOfRegisters;i++){
		uint16_t r=		aRx[3+2*i]*0x100*aRx[3+2*i+1];
		pRegR[StartingAddress+i]=r;
		/* Записуємо з Модбас-регістрів в масив arr         */
		/* Чи відразу й на флеш записуємо - треба подумати  */

	}
	/* формуємо відповідь*/
	TxRsp[1]= aRx[1];
	TxRsp[2]= StartingAddress/0x100;
	TxRsp[3]= StartingAddress%0x100;
	TxRsp[4]= QuantityOfRegisters/0x100;
	TxRsp[5]= QuantityOfRegisters%0x100;
	/* Розмір відповіді і байтах   */
	iTxRsp=7;

}
