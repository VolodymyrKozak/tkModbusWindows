/*
 * mb_RS232rxSlave.c
 *
 *  Created on: 23 ����. 2019 �.
 *      Author: KozakVF
 */
#include "tkRS232rx.h"
/* ��� ������� ��������� ������ ����������� �� PC, �� ����� �� ����������
 * ������ � �� ��������� ����������� �� �������� �������� ���� �������
 * �� ��������� ������ */
	  extern mb_buf[256];
	  extern unsigned char buf[4096];
	  extern uint16_t n;
	  extern int bsize;
	  extern int cport_nr;
      extern int bdrate;       /* 9600 baud */
	  extern char mode[];
//	  extern FILE *fp;

	  uint16_t dbI;
	  uint16_t dbY;

/* �������� �������� ����� �� ���� ������ � ������ ��� ����������� */
crc_search_t f_AskMasterMB(modbus_master_rx_msg_t *prx_msg){
	/* ³�������� ������� �� ������� */
	int16_t ocom = RS232_OpenComportRead(cport_nr, bdrate, mode);
	crc_search_t CRCSS = crcs_Unknown;
	//f_rs232rx_mbSlave(modbus_master_rx_msg_t *rx_msg);
	/* ��������� ���� ��� ���� �� ������� ����� �� ����������� */
	while (CRCSS == crcs_Unknown){
		CRCSS =  f_rs232rx_mbSlave(prx_msg);
	}
    /* ��������� ������� � ��������� ����������� �� ������� */
	RS232_CloseComport(cport_nr);
	return CRCSS;

}

crc_search_t f_rs232rx_mbSlave(modbus_master_rx_msg_t *rx_msg){
	/* ²������� ���-����� */
	 crc_search_t crcss = crcs_Unknown;
//	  Sleep(200);
	  int8_t output=0;
	  /* �������� ���� � �����  */
	  n = RS232_PollComport(cport_nr, buf+bsize, 4095-bsize);

//		  n= 30;
//		  n = n_residue+RS232_PollComport(cport_nr, buf+n_residue, 4095-n_residue);
	  int m=0;
	  if (n!=0){
		 m=n+bsize;
	  }
	  buf[m] = 0;   /* always put a "null" at the end of a string! 			*/
	  if((m) > 0){
		  bsize=0;
		  /* ������ � ����� ����������� ������ */
		  if ((m)>2){
			  int16_t bm=0; /* ����������� ������� ����������� ������ */
			  int16_t eM=2; /* ����������� ����� ����������� ������  */
			  /* ������� ���� - ����� ����������� ������ - �� ����� ����������� */

			  while ((eM<m)){
				  /* ���������� ������ �� ��, � ����� ������ � ����������� ������
				   * � ������� ������������ �� �������:
				   * ����� ����� - ����������, ��
				   * ������� - �����, �� ���� �� bm                                */

				  crcss=f_crc_searchSlave (
						  buf,
						  bm,
						  eM,
						  rx_msg
						  );
				  /* ���� ���� � ��������� ������� */
				  if (crcss==crcs_Error){
					  rx_msg->length=0xFF; break;
				  }
				  /* ���� ����� ����� ������ �� �� ������ ����������� ������ -
				   * ������������ ����� ����� ������*/
				  else if (crcss==crcs_not_found){
					  eM++;
				  }
				  /* ���� ����� ����� ������ ������ ����������� ������ ���
				   * ����������� ������ � ������ ������ ������ ��������  -
				  	 ���������� �� ���������� ����� ������       				   */
				  else if ((crcss==crcs_full_found)||
						  (crcss==crcs_crc_and_wrongbytes_found)){

					  bm=eM+1;
					  eM=eM+3;
				  }
				  else{}

			  }//while
			  if (crcss==crcs_not_found){
				  /* ���� ����� �� ���� ������  � �����, � �� ������� �����������,
				     ���������� ��� ����� �� ������� ������, ����
				     ������� �� �� ������� ���������� �����������, ������ ������� �����
				     ���������� � ���������� ����� ������, ���������� � ���-�����   * */
				  dbY++;
				  bsize=eM-bm;
				  if (bsize>256){
					  memcpy (mb_buf, buf+eM-256, 256);
//					  f_mbPrint(buf+bm, bsize-256);
//					  f_mbPrint(mb_buf, 256);
				  }
				  else{
					   memcpy (mb_buf, buf+bm, bsize);
//					   f_mbPrint(mb_buf, bsize);
				  }
				  memset (buf, 0, sizeof(buf));
				  memcpy (buf, mb_buf, bsize);
				  memset (mb_buf, 0, sizeof(mb_buf));
			  }
			  else {
				  memset (buf, 0, sizeof(buf));
			  }

		  } // if (n>2){
	} ///if(n > 0){

	return crcss;
}
//typedef enum {
//	crcs_Error,
//	crcs_Unknown,
//	crcs_not_found,
//	crcs_full_found,
//	crcs_crc_and_wrongbytes_found,
//}crc_serch_t;
//crc_search_t crc_search_state = crcs_Unknown;
crc_search_t f_crc_searchSlave (
		uint8_t *buf,
		int16_t bi,
		int16_t ei,
		modbus_master_rx_msg_t *msg
		){
	uint16_t msg_size=0;
	if (ei-bi<2){return crcs_Error;}
	crc_search_t cs = crcs_not_found;
	for (int16_t i=ei-2;i>=bi;i--){
	  int8_t iYes = false;
	  iYes =  f_crcOK (
				buf,
				i,
				ei
				);
	       if (iYes== -1    ){cs=crcs_Error;break;	}
	  else if (iYes==  false){/*�����*/}
	  else if (iYes==  true ){
		  /* ���� ������� � ����� ����������� ������ - ������������ ����*/
		  if (i==bi){
			  cs=crcs_full_found;
			  /* �������������� ������-����������� */
			  msg_size = ei-bi+1;
//			  f_mbPrint(buf+bi, msg_size);
			  memcpy(msg->msg,buf+bi,msg_size);
			  msg->length=msg_size;
			  if (msg_size>256){
				  cs = crcs_not_found;
			  }
		  }
		  else if (i>bi){
			  cs=crcs_crc_and_wrongbytes_found;
			  /* ������� ���� - �� ������������� �� ������ �����������*/
//			  uint16_t umsg_size = i-bi+1;
//			  f_mbPrint(buf+bi,  umsg_size);
			  /* �������������� ������-����������� */
			  uint16_t imsg_size = ei-i+1;
//			  f_mbPrint(buf+bi, imsg_size);
			  if (imsg_size>256){
			 	  cs = crcs_not_found;
			  }
			  else {
//				  f_mbPrint(buf+i, ei-i+1);
//				  f_write_to_passport (buf+i,ei-i+1);
				  msg_size=ei-i+1;
				  memcpy(msg->msg,buf+bi,msg_size);
				  msg->length=msg_size;
			  }
		  }
		  else{}
	  }
	}
	return cs;
}

