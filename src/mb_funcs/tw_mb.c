/*
 * mb.c
 *
 *  Created on: 30 ����. 2017 �.
 *      Author: Volodymyr.Kozak
 */
#include "../mb_funcs/tw_mb.h"

#include "../mb_funcs/RS_232/rs232.h"

extern int bdrate;       /* 9600 baud */
extern char mode[];
static modbus_status_t f_RS232_readMbMsg(
		int n, 	/* ��������� ������� ���� �� ���� RS232*/
		int comport_number,
		modbus_master_tx_msg_t *p_tx_msg,
		modbus_master_rx_msg_t *p_rx_msg,
		int msg_timeout);

/* �� ������� �� ����� ��������� ����������� ��������� ��������� ������� ����
 * � �������� ���������� - ������ ������ */
static uint16_t  f_tw_txWaitedByteNumber(modbus_master_tx_msg_t *pmb_tx_msg);



modbus_status_t tw_txModbus_Session (
		int comport_number,
		modbus_master_tx_msg_t *p_tx_msg
		){

	volatile modbus_status_t mb_session_status = MODBUS_UNKNOWN_ERROR;
	while (mb_session_status == MODBUS_UNKNOWN_ERROR) {
	/* ²������� ���-����� */
		int ocom = RS232_OpenComportWrite(comport_number, bdrate, mode);
		if (ocom !=0){
			mb_session_status=MODBUS_COMPORT_ERR;
			break;
		}
	/* ²������� ��²�������� */
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
		/* �������� ���-�����						*/
		RS232_CloseComport(comport_number);
		mb_session_status=MODBUS_SUCCESS;
	}
	return mb_session_status;
}



/***************************************************************************
 * �� ������� ������ ���� RTS Modbus Master � Windows-����'�����.
 * �����������:
 *  �������� �������� COM-����� �� ��������,
 *  �������� ������ �����������,
 *  �������� �������� COM-����� �� ��������
 *  �������� �������� COM-����� �� ������
 *  � ��� ����������� ������ �� Slave - ������ ������
 *  �������� �������� ���-�����
 *  */
modbus_status_t tw_txrxModbus_Session (
		int comport_number,
		modbus_master_tx_msg_t *p_tx_msg,
		modbus_master_rx_msg_t *p_rx_msg,
		uint32_t msg_timeout
		){

	volatile modbus_status_t mb_session_status = MODBUS_UNKNOWN_ERROR;

	while (mb_session_status == MODBUS_UNKNOWN_ERROR) {
	/* ²������� ���-����� */
//		ocom = RS232_OpenComportWrite(comport_number, bdrate, mode);
		int ocom = -1;
		ocom = RS232_OpenComport(comport_number, bdrate, mode);
		//ocom = RS232_OpenComportWrite(comport_number, bdrate, mode);
		if (ocom !=0){
			/* ���� �� ������� ������� ���� ������� ���� �������*/
			BOOL ccom=RS232_vkCloseComport(comport_number);

			if(ccom==0){/* ���� �� �������*/
				mb_session_status=MODBUS_COMPORT_ERR;
				break;
			}
			else{ /* ���� ������� ������� -*/
				  /* ����������� �� ��� ������� */
				int ocom2 = RS232_OpenComport(comport_number, bdrate, mode);
				//int ocom2 = RS232_OpenComportWrite(comport_number, bdrate, mode);
				if(ocom2!=0){
					mb_session_status=MODBUS_COMPORT_ERR;
					break;
				}
			}

		}
	/* ²������� ��²�������� */
		int jn=0;
		jn=RS232_SendBuf(comport_number, p_tx_msg->msg, p_tx_msg->length);
		/* ������� ���-����, �������� �� ��������      */
		//RS232_CloseComport(comport_number);
		if (jn==p_tx_msg->length){
			mb_session_status=MODBUS_SUCCESS;
		}
		else{
			mb_session_status=MODBUS_ERR_TX;
			break;
		}

	    /* ���������-���������� ������                        */
		/* ³������ ���-���� �� ������                           */


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
		/* �������� ���-�����*/
		BOOL ccom=RS232_vkCloseComport(comport_number);
		if(ccom==0){/* ���� �� �������*/
			mb_session_status=MODBUS_COMPORT_ERR;
			break;
		}
		//RS232_CloseComport(comport_number);
	}/*����� � WHILE*/


	return mb_session_status;
}

extern void f_tk5Logging(char *str, size_t n_str);
extern char tk5LogStr[512];
#define TK5_MODBUSRxQUANTITY_MS 3
static int msg_time_ms=0; /* �������� ��� ����������/������� ����������� */
static char buff[300]={0};/* ����� ��� ����������� ����������� */

/* �� ������� - ������������ ��������� ������ �� ������� (slave)������
 * �� ��������� ����� ��������:
 * 		Sleep(msg_timeout);
 *  	int n = RS232_PollComport(comport_number, p_rx_msg->msg, RX_MAX_MASTER_MSG_LENGTH);
 *  	if (n==0){mb_session_status=MODBUS_ERR_TIMEOUT;break;},
 *  	���� ������ ������, ��� ������ ������� ������ ���� �� ���������� ������
 * �� ������� ������� ������ ������ �� ��������� ������� ���� �����������, ��
 * ���� � �'��� ������*/
static modbus_status_t f_RS232_readMbMsg(
		int n, /* number bytes to read from RS232*/
		int comport_number,
		modbus_master_tx_msg_t *p_tx_msg,
		modbus_master_rx_msg_t *p_rx_msg,
		int msg_timeout){
	modbus_status_t chi=MODBUS_UNKNOWN_ERROR;
    if((n==0)||(p_tx_msg==NULL)||(p_rx_msg==NULL)){return MODBUS_WRONG_ARG;}

	char buff0[256]={0}; /* ����� ��� ������� � ����������� ��������� ����� ����������� ������ */
	int l = n;           /* ��������� ������� ���� */
	int len=0;           /* ������� �������� ������� ���� */
	int kOld=0;          /* ʳ������ ����, �������� �� ��������� ���� */
	/* ������ �� �������� �������������� ������� ���� */
	while (len<l){
		/* ���� ������ ���������� */
		Sleep(TK5_MODBUSRxQUANTITY_MS);
		/* ������ ���, ��� ���� ��������� ����� �� ���������, ���� �� ���������� */
		msg_time_ms+=TK5_MODBUSRxQUANTITY_MS;
		/* ������� ���� */
		int k = RS232_PollComport(comport_number, (uint8_t*)buff0, 256);
		/* ���� �������� ����� ������� ���� */
		if (k>0){
//			DbgCntr++;
//
//			if(DbgCntr==2){
//				DbgCntr=0;
//			}
			 /* ������ ����� ������ */
			len+=k;
			/* ���������� �������� ������ ���� � ����� ���������� ������ */
			memcpy(buff+kOld, buff0,k);
			/* ����� ��� ������� ����� ����������� ������� ��� ������� ���������� ����� */
			memset(buff0,0,sizeof(buff0));
			/* ����������, ���� �� ����������� ������� ��� �������*/
			if(k>2){
				if(l==n){
					if((buff[1] | 0x80)==0x80){
						/* ������� �� ������������ �������, �
						 * ����������� ��� �������, 5 ���� � �������� ������ */
						l=5;
					}
				}
    		}
			/* ʳ������ ����, �������� �� ��������� ���� ������ �� ������� �������� ������� ���� */
			kOld+=k;
    	}
		/* ����������� ������� */
		if(msg_time_ms>msg_timeout){
			chi=MODBUS_ERR_TIMEOUT;
			break;}
		chi=MODBUS_SUCCESS;
    }   /* ����� � �����, ���� �������� ��������� ����� ���������� */
	if(chi==MODBUS_SUCCESS){
		p_rx_msg->length=l;
		memcpy(p_rx_msg->msg,buff,l+1);

	}
	else{
		len=0;
	}
//	msg_time_ms=0; /* �������� ��� ����������/������� ����������� */
	memset(&buff,0,sizeof(buff));/* ����� ��� ����������� ����������� */

//	/* �� ��� ��� �������� �������������� */
	uint16_t Cmd = p_tx_msg->msg[1];
	uint16_t RegTx = p_tx_msg->msg[2]*0x100+p_tx_msg->msg[3];

	uint8_t ValNumTx=0;
	uint16_t RegRx=0;
	if(Cmd==0x03){/* ���� ������� 0�03 */

		uint16_t Quantity_of_RegistersTx=p_tx_msg->msg[5];
		/* ������, ���� ��� � ����� �������� ������ */
		RegRx=p_rx_msg->msg[3]*0x100+p_rx_msg->msg[4];
		/* ʳ������ ������� */
		ValNumTx=Quantity_of_RegistersTx;


	}
	/* ���� 0�06 */

	if(Cmd==0x06){
		/* ���� ������ */
		RegRx=p_tx_msg->msg[2]*0x100+p_tx_msg->msg[3];
		/* �� ������   */
		ValNumTx=p_tx_msg->msg[4]*0x100+p_tx_msg->msg[5];

	}
	snprintf(tk5LogStr,100,"0x%1X 0x%04X  %d ms, err %d",
			Cmd, RegRx, /*ValNumTx, RegRx,*/ msg_time_ms, (int)chi);
	f_tk5Logging(tk5LogStr, sizeof(tk5LogStr));
	msg_time_ms=0;
	memset(buff,0,sizeof(buff));
    return chi;




}


/* �� ������� �� ����� ��������� �����������
 * ��������� ��������� ������� ���� � �������� ����������
 * -������ ������ */
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

		n=10;/* ��� ��5 */
	}
	break;
	default:{}
	}
	return n;
}
