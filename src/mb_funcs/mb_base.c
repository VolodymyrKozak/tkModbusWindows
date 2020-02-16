/*
 * mb_base.c
 *
 *  Created on: 24 ���. 2019 �.
 *      Author: KozakVF
 */

#include "mb_base.h"

#include <stdbool.h>


mb_passport_t mb_passport[256]={0};
uint8_t nBase=1;      		 /* ʳ������ �������������� ��'����/�������� � �����*/
uint32_t msg_count=0; 		 	 /* ʳ������ ����������, ��������� �� ��� ����������  */
SYSTEMTIME start_time={0};	 /* ������� ����������  */
SYSTEMTIME last_msg_time={0};/* ������ ����������� */
//uint8_t address;
//uint32_t intensity;
//SYSTEMTIME last_activity_time;
//int8_t last_msg0[256];
//uint8_t n_msg0;
//int8_t last_msg1[256];
//uint8_t n_msg1;
//int8_t last_msg2[256];
//uint8_t n_msg2;
//int8_t last_msg3[256];
//uint8_t n_msg3;
static int16_t look_for_slave(uint8_t addr);
static uint8_t f_rankmb_passport(void);

char StatusStr [100]={0};
//SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"Hi there :)");
int16_t f_passportInit(void){

	memset (&mb_passport,0,sizeof(mb_passport));
	nBase=1;
	return 0;
}

uint8_t iMBP=0;
int16_t f_write_to_passport (
					uint8_t *mb_msg,
					uint8_t l_msg){
	msg_count++;
	if (msg_count>0xFFFFFFF0){msg_count=0;}
	uint8_t saddr=mb_msg[0];
	int16_t baseindex=look_for_slave(saddr);
	if (baseindex<0){return -1;}

	memcpy (&mb_passport[baseindex].MSG[3],&mb_passport[baseindex].MSG[2], sizeof(msg_info_t));
	memcpy (&mb_passport[baseindex].MSG[2],&mb_passport[baseindex].MSG[1], sizeof(msg_info_t));
	memcpy (&mb_passport[baseindex].MSG[1],&mb_passport[baseindex].MSG[0], sizeof(msg_info_t));

	memset (&mb_passport[baseindex].MSG[0],0,sizeof(msg_info_t));
	memcpy (&mb_passport[baseindex].MSG[0].last_msg,mb_msg,l_msg);
	         mb_passport[baseindex].MSG[0].n_msg=l_msg;
	         mb_passport[baseindex].intensity++;
	GetSystemTime(&(mb_passport[baseindex].MSG[0].last_activity_time));
	last_msg_time=mb_passport[baseindex].MSG[0].last_activity_time;

	/* ���������� ���������� � ��������� */
//	uint8_t nBase=1;      		 /* ʳ������ �������������� ��'����/�������� � �����*/
//	uint32_t msg_count=0; 		 /* ʳ������ ����������, ��������� �� ��� ����������  */
//	SYSTEMTIME start_time={0};	 /* ������� ����������  */
//	SYSTEMTIME last_msg_time={0};/* ������ ����������� */
	return 0;
}




static int16_t look_for_slave(uint8_t addr){
	int16_t ind =0x0FFF;
	while (ind == 0x0FFF){
		/* ���� �� �������������� ����������� - ����� ������ */
		if (addr==0x00){
			ind= 0;
			break;
		}
		/* ���� � �a� � ������� ��� ���� ������ ������ - ������ ���� */
		bool inbase = false;
		for (uint8_t i=0;i<nBase;i++){
			if (mb_passport[i].address==addr){
				ind=(int16_t)i;
				inbase=true;
				break;
			}
		}
		if (inbase==true){
			break;
		}
		/* ���� � ��� �� �� �������� ��� ���� ������-������, �������� ����..*/
		nBase++;
		if (nBase == 2){
			ind++;
		}
		mb_passport[nBase-1].address=addr;
		/* �������� ������ � ��� �� ���������� ������� ������*/
		f_rankmb_passport();
		/* ������ ������ �������� � ������������� ����� ������� */
		for (uint8_t i=0;i<nBase;i++){
			if (mb_passport[i].address==addr){
				ind=i;
				break;
				break;
			}
		}
	}
	return ind;
}


static uint8_t f_rankmb_passport(void){
	if (nBase==1){return -1;}
	bool goon = false;
	do {
		goon = false;
		for (uint8_t i=1;i<nBase;i++){
//			if (line[i-1]>line[i]){
			if (mb_passport[i-1].address>mb_passport[i].address){
				mb_passport_t tmp_passport = mb_passport[i-1];
				mb_passport[i-1]=mb_passport[i];
				mb_passport[i]= tmp_passport;
				goon=true;
			}
		}
	}while(goon==true);
	return 0;
}
//static void f_rank8(uint8_t line[], uint8_t n){
//	bool goon = false;
//	do {
//		goon = false;
//		uint8_t i=0;
//		for (i=1;i<n;i++){
//			if (line[i-1]>line[i]){
//				uint8_t b8 = line[i-1];
//				line[i-1]=line[i];
//				line[i]=b8;
//				goon=true;
//			}
//		}
//	}while(goon==true);
//}
void check_coincidental_psp(int psp_index){

	int n=4;/* ʳ������ �����������, �� ����������� � ����� ��������*/
	/* ��������� ����� �����  iStrOne � ���������� 			*/
	for (uint8_t iStrOne=0;iStrOne<n-1;iStrOne++){
		/* ������ ������ �� ������ jStrTwo �� ������ iStrOne 	*/
//		for (uint8_t jStrTwo=iStrOne+1;jStrTwo<n;jStrTwo++){
		uint8_t jStrTwo=iStrOne+1;
			/* ���� ������� ������� ���������� � ���� �� ������� �� ����� 5 	*/
			if  ((mb_passport[psp_index].MSG[iStrOne].n_msg >5)&&
				 (mb_passport[psp_index].MSG[iStrOne].n_msg==mb_passport[psp_index].MSG[jStrTwo].n_msg)){
				/* ���� ���������� ���� ������� 		*/
				if (mb_passport[psp_index].MSG[iStrOne].last_msg[1]==mb_passport[psp_index].MSG[jStrTwo].last_msg[1]){
                    /* ��������� ����������� �������� */
					for (uint8_t ibyte=3;ibyte<mb_passport[psp_index].MSG[iStrOne].n_msg-2;ibyte++){
						/* ������ �� ������� ������������ ����� */
						char chONE=mb_passport[psp_index].MSG[iStrOne].last_msg[ibyte];
						char chTWO=mb_passport[psp_index].MSG[jStrTwo].last_msg[ibyte];
						if (chONE!=chTWO){
							/* ��� ������� ���� ����� msg1, msg2, i*/
							f_printColorByte2(
									psp_index,/* iPassport*/
									iStrOne,
									ibyte,
									chONE,
									(HBRUSH)4
									);
							f_printColorByte2(
									psp_index,/* iPassport*/
									jStrTwo,
									ibyte,
									chTWO,
									(HBRUSH)4
									);
						}
					}


				}
//			}
		}
	}
}

