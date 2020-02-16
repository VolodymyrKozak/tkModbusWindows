/*
 * a3_TK112TxRx.c
 *
 *  Created on: 31 ����. 2019 �.
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


/* �������� ����, ����������� �� ������� � ����, ��� ����������� ������ � ����� */
modbus_master_rx_msg_t mIn_rx_msg={0};
/* �������� ����, ����������� �� ������� � ����, ��� ����������� ������ � ����� */
modbus_master_tx_msg_t mOut_tx_msg={0};
/* �������� ����, ����������� �� ������� � ����, ��� ����������� ������ � ����� */
modbus_status_t RxMasterModbusStatus=MODBUS_UNKNOWN_ERROR;


static ULONGLONG  TxRxSessionBusyEtimeSum =0;
static ULONGLONG  TxRxSessionBusyIddleEtimeSum  =0;
//typedef struct{
	/* ���� ��������� �����:
	 * ProcessIddle - ����� �����,
	 * ProcessBusy  - ����� ������ ���������, �
	 * ����� ���� ������*/
	static ProcessState_t   RS232State = ProcessIddle;
	/* ��������� ����� -  ���������� ������� ��������� ������
	 * �� ���� ��� ������������� ���������  */
	procentf_t gTxRxBusyAveDegreef          =0;
	/* ��������� ����� - �������� ������� ��������� ������
	 * ��������� � �������� ���� - �'��� ������� ������ */
	procentf_t gTxRxBusyDegreef             =0;
	/* �������� �������� ����*/
	ULONGLONG  gTxRxSessionStartCntr        =0;
	/* �������� �������� ����, �� ����������� ��� ������� */
	ULONGLONG  gTxRxSessionOkCntr           =0;
	/* �������� ������� ����, �� ����������� ��� �������   */
	procentf_t gTxRxSessionSuccessDegreef   =0;
	/* ������� �� ���� ��� ������������� ��������� ����, �� ����������� ��� ������� */
	procentf_t gTxRxSessionAVESuccessDegreef = 0;
//}tXrX_session_duration_t;

/* ��������� ���� Iddle and Busy
 * 1) �������� �������� ���� ���������� ������
 *    ������ ������������� ���������, ���� ���������� ��������� ���������� �������
 * 2) ���������� ���� ����� � ���� ����������� �����������,
 * 3) ������ � ���������� � ����'���s �� ������� ����������� ���������
 * 	  ����� �������
 * 	  �� ���������� �� ������������ ��� ���� ����������� � ����� ����������� ���������
 * 4) ���������� ��� ����� � ���� ����������� �����������
 * 5) ���������� ��� ����� � ����������� ����������� � ����'���� ��
 *    ���������� ���������� ����� ��2
 * 6) ��������� ���������� ��� ����� � ��������..
 * */
static ProcessState_t TxRxBusyDegreeCalc = ProcessIddle;
static ProcessState_t TxRxBusyDegreeCalcOld = ProcessIddle;
static ULONGLONG  TxRxStartTime         =0;
/* 'a3_TxRx.c/.h, ��������� ���� Iddle and Busy
 * 1) �������� �������� ���� ���������� ������
 *    ������ ������������� ���������, ���� ���������� ��������� ���������� �������
 * 2) ���������� ���� ����� � ���� ����������� �����������,
 * 3) ������ � ���������� � ����'���s �� ������� ����������� ���������
 * 	  ����� �������
 * 	  �� ���������� �� ������������ ��� ���� ����������� � ����� ����������� ���������
 * 4) ���������� ��� ����� � ���� ����������� �����������
 * 5) ���������� ��� ����� � ����������� ����������� � ����'���� ��
 *    ���������� ���������� ����� ��2
 * 6) ��������� ���������� ��� ����� � ��������..
 * */
void f_TxRxBusyDegreeCalcStart(void){
	TxRxBusyDegreeCalc=ProcessBusy;
	TxRxStartTime=GetTickCount();
}
/* ��������� ���� Iddle and Busy
 * 1) �������� �������� ���� ���������� ������
 *    ������ ������������� ���������, ���� ���������� ��������� ���������� �������
 * 2) ���������� ���� ����� � ���� ����������� �����������,
 * 3) ������ � ���������� � ����'���s �� ������� ����������� ���������
 * 	  ����� �������
 * 	  �� ���������� �� ������������ ��� ���� ����������� � ����� ����������� ���������
 * 4) ���������� ��� ����� � ���� ����������� �����������
 * 5) ���������� ��� ����� � ����������� ����������� � ����'���� ��
 *    ���������� ���������� ����� ��2
 * 6) ��������� ���������� ��� ����� � ��������..
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
    	snprintf(log,log_size,"  :  :       %d ��(����) %d ����� %d ������ - ��������� ������������� ���������\n",
			(int)DistCntrDuration_days, (int)DistCntrDuration_h, (int)DistCntrDuration_m);
    }
}
/* ��������� ���� Iddle and Busy
 * 1) �������� �������� ���� ���������� ������
 *    ������ ������������� ���������, ���� ���������� ��������� ���������� �������
 * 2) ���������� ���� ����� � ���� ����������� �����������,
 * 3) ������ � ���������� � ����'���s �� ������� ����������� ���������
 * 	  ����� �������
 * 	  �� ���������� �� ������������ ��� ���� ����������� � ����� ����������� ���������
 * 4) ���������� ��� ����� � ���� ����������� �����������
 * 5) ���������� ��� ����� � ����������� ����������� � ����'���� ��
 *    ���������� ���������� ����� ��2
 * 6) ��������� ���������� ��� ����� � ��������..
 * */
static modbus_status_t f_tkTxRx(
		modbus_master_tx_msg_t *pmb_tx_msg,
		modbus_master_rx_msg_t *pmb_rx_msg,
		int timeout
		);

/* �� ������� - �������� ����� ���:
 * ������ ��� � ������ ����� � �������� ����
 * ����� ������ �����
 * ������� ����������� � �����������, �������� � �� ����� �����  */
void f_desktop_tk_session (
		queue_t *htkQ           /* �������� �� ��������� - ���������� �����*/
	){
	int status = 0;
	static uint32_t TxRxSessionStartEtime;
	static uint32_t TxRxSessionStopEtime;
	static uint32_t TxRxSessionDurEtime;
	static uint32_t TxRxIddleDurEtime;
	while(status>=0){
        if(htkQ==NULL){status=-1;break;}
        /* ���� � ���� � ��� ���� �����������
			 * ���� �� ����������� ��������� ����� mIn_rx_msg ��� ��������
			 *  ������� ��� ��������� ������� ������������ �����������
//			 *  �� ���'����� � ���, �� ������� ������������ � ������������ ���,
 * 			    � ���� ����������� �� ���������, ��� ���� � �� ���������� -
 * 			    �������� ������� ����������� */
		if ((htkQ->node_number>0)&&(mIn_rx_msg.length==0)){

			/* �������� �������� ���� */
			gTxRxSessionStartCntr++;
			/* ������� ���������� ��� ����������
			 * �������� ��������� ������
			 *                    */
			char str[256]={0};
			snprintf (str,sizeof(str),"a3TxRx.c ����� ��� %d", gTxRxSessionStartCntr);
			f_tk5Logging(str, sizeof(str));


			TxRxSessionStartEtime = (uint32_t)GetTickCount();
			if (TxRxSessionStartEtime>TxRxSessionStopEtime){
				TxRxIddleDurEtime=TxRxSessionStartEtime-TxRxSessionStopEtime;
			}
			else{
				TxRxIddleDurEtime=10;
			}

			RS232State=ProcessBusy;
			tk_session_data_t *htks=htkQ->first;  /* �������� �� ��������� - ��������� ��� ����� �����,
			                                         ����� �� ������ �����    */
			/* ��� � ������ �������� �� �������..
			 * � �����, ���� ��������� �������� � �� ����� ����� ������ ����,
			 * � ���� ����������� �������� � ����� */
			memcpy(&mOut_tx_msg,&(htks->mb_tx_msg),sizeof(modbus_master_tx_msg_t));
			int in_timeout = htks-> timeout;
			int in_case = htks->in_case;
			HWND in_hwnd = htks->hwnd;
			snprintf (str,sizeof(str),"a3TxRx.c ������ ��� %d", in_case);
			f_tk5Logging(str, sizeof(str));
			/* ��������� ������, �� �� ��� ��� ������
			 * � ������ ����� ������ ������ ���� �����������,
			 * � ��� �������.. */
			/* ��������� ������ */
			deletehead(
					sizeof(tk_session_data_t),/* ������ ������ */
					htkQ /* ������ �� ���������� ������� */
					);
			/* ������ ���� ������ */
			memset(&mIn_rx_msg,0,sizeof(modbus_master_rx_msg_t));
 			RxMasterModbusStatus=f_tkTxRx(
					&mOut_tx_msg,
					&mIn_rx_msg,
					in_timeout
					);
			/* ���� ���� ����������� �� -
			 * �������� ����������� ��� ������� in_rx_msg,
			 * ���� � �������� - ���������� ��� ������� */
			/* �������� �����, � ����� ����������� �����������,
			 * �� ������� ��������, ��������� ������� ����    */
 			snprintf (str,sizeof(str),"a3TxRx.c ���� %d %d  ���������. ��� %d",
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

			/* ������� ������� ����, �� ����������� ��� �������   */
			gTxRxSessionAVESuccessDegreef  =	(float)(gTxRxSessionOkCntr*10000/gTxRxSessionStartCntr) / 100.0f;
			/* �������� ���������� ������ ����, ������������� ��� ������� */
			procentf_t tmpgTxRxSessionSuccessDegreef=gTxRxSessionSuccessDegreef;
			procentf_t tmpthissessionOk =0;
			if(RxMasterModbusStatus==MODBUS_SUCCESS){tmpthissessionOk=100.0f;}else{tmpthissessionOk=0.0f;}
			float alphams=0.9;
			gTxRxSessionSuccessDegreef = tmpgTxRxSessionSuccessDegreef*alphams + tmpthissessionOk*(1.0f-alphams);

			/* ���������� �������� ������� ��������� ������ */
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

			/* ���������� ���������� �������� ��������� ������  �� ���� ��� ������������� ���������*/
			if((TxRxBusyDegreeCalcOld==ProcessIddle)&&(TxRxBusyDegreeCalc==ProcessBusy)){
				TxRxSessionBusyEtimeSum += TxRxSessionDurEtime;
			}
			if((TxRxBusyDegreeCalcOld==ProcessBusy)&&(TxRxBusyDegreeCalc==ProcessBusy)){
				/* ���������� ���������� �������� ��������� ������  �� ���� ��� ������������� ���������*/
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
		/* ��������� ������� 10�� */
		Sleep(10);

	}
	_endthread();
}

/* �� ������� ���� ���� ����������� � �����
 * ������� ������� ����� */
int f_set_tkqueue(
		queue_t *htkQ,           /* �������� �� ��������� - ���������� �����*/
		FunctionalState lifo, /* ENABLE  - ������ � ������ �����
		                       * DISABLE - ������ � ���� ����� */
		HWND hwnd,            /* ����, � ����� ������� ����������� � ����� */
		int in_case,          /* ������������� � ���, �� ���� ���� ����������� ������� ������ */
		modbus_master_tx_msg_t *pmb_tx_msg,
							  /* ����������� ������*/
		int timeout           /* ������� ��� ������-����������� */
		){
	tk_session_data_t tks={0};
	tks.hwnd=hwnd;
	tks.in_case=in_case;
	memcpy(&tks.mb_tx_msg, pmb_tx_msg, sizeof(modbus_master_tx_msg_t));
	tks.timeout=timeout;

	if (htkQ->node_number<80){
		if(lifo==DISABLE){
			addtail(
				(void*)&tks,   /* ������ �� ���� �������� � ���������� ���������
								  ��� ���������� ��� � ������� */
				sizeof(tk_session_data_t),/*������ ���� �������� ������ */
				htkQ /*������ �� ���������� ������� */
				);
		}
		else if(lifo==ENABLE){
			addhead(
				(void*)&tks,   /* ������ �� ���� �������� � ���������� ���������
								  ��� ���������� ��� � ������� */
				sizeof(tk_session_data_t),/*������ ���� �������� ������ */
				htkQ /*������ �� ���������� ������� */
				);
		}
		else{}
	}
	return htkQ->node_number;
}

/* ��������� ������� �����. ����������� ��� ������������
 * � ���� ��� �� �����..
 * ��� ��������� �����, ����� ����������� ������� � ��� �� �������� �
 * � ������ ��� ������ ������� ����������� �������� �����,
 * ���� ���� �������� ��������������� ������� f_get_tk2rs232_length() */
int f_get_tkqueue_length(queue_t *htkQ){
	return htkQ->node_number;
}

/* ��������� ������� ����� � ����������� �����������,��� ����������� � ���� �������*/
int f_get_tkrs232_length(queue_t *htkQ){
	return htkQ->node_number+(int)RS232State;
}
/* ��������� ������. ��� ����� ����������� �������� ������. */
/* ��������� ������ � ���� ������ ���� �� ���������        */
int f_clear_tkqueue(queue_t *htkQ){
	while (htkQ->node_number!=0){
	      deletehead(
			sizeof(tk_session_data_t),/* ������ ������ */
			htkQ /* ������ �� ���������� ������� */
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

/* �� ������� - �������� ��� ������ �� ��� ����� ����������.
 * �� �������� ����������� (��� CRC �������� CRC),
 * ����������� ���� ������ ������� ���� (������� tw_txrxModbus_Session(), ���� tw_mb.c/h),
 * ���� ���� ������ - �� - �� �����������-������ ������ �������� �RC, ����������� MODBUS_SUCCESS
 * ��������� ������������ ������ ��/��� ����������
 * ���� ������� - ����������� ��� ������� */
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
		/* ������ CRC*/
		ms = f_add_CRC(pmb_tx_msg);
		/* ��������� ����	*/
		ms=tw_txrxModbus_Session (
				cport_nr,
				pmb_tx_msg,
				pmb_rx_msg,
				timeout
		);
	}
    if(ms == MODBUS_SUCCESS){
	    /* ���� ���� RTC ����������� ������, ���������� CRC */
	    ms = f_check_CRC(pmb_rx_msg);
	    if (ms != MODBUS_SUCCESS){
	    	memset(&pmb_rx_msg,0,sizeof(modbus_master_rx_msg_t));
	    }
	    else{
	    	/* ���� ������ ������ �� ���� ������� ����� - �� �������*/
	    	if(pmb_rx_msg->msg[0] != pmb_tx_msg->msg[0]){
	    		ms=MODBUS_ERR_SLAVE_ADDR;	/*  If the transmitted slave address doesn't correspond to the received slave address	*/
	    		memset(&pmb_rx_msg,0,sizeof(modbus_master_rx_msg_t));
	    	}
	    }

    }

	/* ���������� txrx ������ */
	return ms;
}

//https://stackoverflow.com/questions/16506291/remove-button-from-a-window

