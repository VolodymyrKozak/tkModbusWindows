/*
 * a3_tk2z.c
 *
 *  Created on: 2 ��. 2020 �.
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
/* ���������, ��� ������� �����
 * ���������� ���� ��������� ������, �� ��������
 * ��������� ����� � ���������� ������, ���� �� �����
 * �������� ���������� ����� ���������� �� ��2*/
extern   queue_t tk5Queue;
extern FILE *fp_tk5LogFile;

extern HWND hWndtk5;
// 0x0205	������� ������ �� ������� ����������, �	10.0 �  600.0	I�	arr[2]
extern float2_t Itk5_Max;
//0x0206	������� ���������� ����������� ���� (����������), � 	10.0�600.0 I��	arr[3]
extern float2_t Itk5_Min;

/* ��������� ��������� ���������� ����������� */
extern d_type_t q;
//extern tk2_fs_t fs;
extern uint8_t tk5_Addr;
grid_t grid={0};
char user_msg5[256]={0};

/* ������ ���� �� ���������� ��� ������:
 *   0 - ����� �� ����� ��� ���������� �������
 *   1 - �� ���� ���� ������ ������������� ���������
 *       ��� ����������� ����������� ������ �� ����������
 *   2 - �� ���� ���� ������ � ���������� ��'���� PC_DESC <-> ���������
 *       ��� ����������� ����������� ������ �� ����������
 *   -1  ���� ������ ����������� � ��������, ������� ������� ������� */
int tk5ProcessState = 0;

/* ˳������� ��������� �� ��2 ����������-������.
 * �������������� �������� f_tk2QPWD_RgAnswer(), ���� 'a3_tk2z.h'
 * ����������, ��� ��������� ����������� ���������� ���� ���� ���,
 * ���� ������� ���� ����������� */
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
/* ������� - �������� ������ �� ��2
 * ��������� �� ����, � ����� ����������� �����������.*/
/* ��������� - ���������, ������� �� ��2
 * ��������� - �������� ������� ������� (��� ���������� ���������� ��� ������� ������)*/
extern uint16_t DebugMode;
extern uint16_t Tk4screenMode;
extern uint16_t AmperageAmplitude;
extern uint16_t BAmperageAmplitude;
extern uint16_t CosFi;
uint32_t f_tk5QPWD_RgAnswer(
		modbus_status_t ms,
		modbus_master_tx_msg_t *mb_tx_msg,
		modbus_master_rx_msg_t *mb_rx_msg,
		int incase){

    char user_msga[256]={0};
	memset(user_msga, 0,sizeof(user_msga));
	tk5ProcessCntr++;
	if (ms!=MODBUS_SUCCESS){
		/* �� �����������, ��� ����������� ��� �������������� ���-���� �������.*/
		/* ���� ������ ��� �����������, ��� ������ ��� ����� */
		/* ��� ������� ������ ����������� ������� ���������� ����� ������    */
		switch(ms){
		/* Z��� ���� �������� ������� ������ RTC  */
		case MODBUS_ERR_TIMEOUT:{
//			 snprintf (user_msga,USER_SESSAGE_SIZE," 0x%02X 0x%04X �� �������� ������ �� ����������� ���",
//					 mb_tx_msg->msg[1], mb_tx_msg->msg[2]*0x100+ mb_tx_msg->msg[3]);
			 break;
		}
		case MODBUS_COMPORT_ERR:{
			 snprintf (user_msga,USER_SESSAGE_SIZE," COM-���� �������� ����� �� �����������   ");
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
//			snprintf (user_msga,USER_SESSAGE_SIZE,"������� CRC ");
			break;
		}
		case MODBUS_ERR_FC:{
//			snprintf (user_msga,USER_SESSAGE_SIZE,"Transmitted function code doesn't correspond to the received function code");
			break;
		}
		default:{
//			snprintf (user_msga,USER_SESSAGE_SIZE," ����������� ������� RTU Modbus            ");
		}
		}
		tk5ProcessState=-1;
  	  char str[256]={0};
  	  snprintf (str,sizeof(str)," a3_tk5z.c �������� ����������� ��� ������� ������ %d", ms);
  	  f_tk5Logging(str, sizeof(str));
	}
	/* ���� ������� �������� �� ���� */
	while(ms==MODBUS_SUCCESS){
		/* ������������ �������� ������� ���� */
		AnswerOkCntr++;
		/* ���� �� ������� ��� ������� */
		uint8_t MbCode=mb_rx_msg->msg[1];
		switch(mb_tx_msg->msg[1]){
		case 0x03:{
		/* ��2 ������ ������� ������� ������� � �� �������� ����������� ��� ������� */
		if((mb_tx_msg->msg[1]==0x03)&&(MbCode==0x83)){
			if(mb_rx_msg->msg[2]==0x02){
				snprintf (user_msga,USER_SESSAGE_SIZE," ���������� ����� ������� �� ��������");
			}
			else{
				snprintf (user_msga,USER_SESSAGE_SIZE," ������� ��������� ������        ");
			}
			tk5ProcessState=-1;
		}
		/* �� ������������ ����������� */
		else if((mb_tx_msg->msg[1]==0x03)&&(MbCode==0x03)){
        	uint16_t value=mb_rx_msg->msg[3]*0x100 + mb_rx_msg->msg[4];
        	/* ��������� ���������� ����� ������� */
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
        	/* ���������� ���������� �����������                                                            */
        	/************************************************************************************************/
        	//���Ҳ��� ���� ���в����� 														0�0209	0�1004	90
        	//#define ID_EDIT_HEATINGTIMECONSTANT						150
        	else if (incase==0x0209){ //             10        20        30        40        50        60        70        80        90
        		if(value<13){         //     1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
        			snprintf (user_msga,USER_SESSAGE_SIZE," �������� �� ��2 �������� ���Ҳ��ί ���в�� ����� ����.\n������� ����� ����������� ����������");
        		}
        		else if(value>230){
        			snprintf (user_msga,USER_SESSAGE_SIZE," �������� �� ��2 �������� ���Ҳ��ί ���в�� ����� ������\n������ ������������� �� ��������");
        		}
        		else{
//        			fs.fsHeatingTimeConstant=value;
        		}
        	}
        	//�������� ʲ��ʲ��� ���ʲ� �� �������� 10��									0X0216	0X1014   5
        	//#define ID_CMB_NUMBERSTARTLIMIT						    175
        	else if (incase==0x0216){
        		if((value>0)&&(value<10)){
 //       			fs.fsNumberStartLimit=value;
        		}
        		else{
        			snprintf (user_msga,USER_SESSAGE_SIZE," �������� �� ��2 �������� ����� ����Ҳ� �� 10�� �������");
        		}
        	}
//        	//�������� ����� �� �� в���													0�0214	0�1006   4..
//        	//#define ID_EDIT_ONSENSORMOTORDELAY					    225
//        	else if (incase==0x0214){fs.fsOnSensorMotorDelay=value;}
//        	//�������� ������� �� �� в���													0�0215	0�1007   12
//        	//#define ID_EDIT_OFFSENSORMOTORDELAY						250
//        	else if (incase==0x0215){fs.fsOffSensorMotorDelay=value;}
//        	//�������� ²��������� ������ ������� ������� ���ӻ ϲ��� ���в� ������� ���ӻ	0�0217	0�1012   13
//        	//#define ID_EDIT_AFTRERDRYMOVEDELAY						275
//        	else if (incase==0x0217){fs.fsAftrerDryMoveDelay=value;}
//        	//����� ����� ����� �����ֲ�/��.����                             ARR[26]		0X0219	0X101A	 0/1?
//        	//#define ID_CMB_INSUL_SOFTSTART_OPTION					325
//        	else if (incase==0x0219){fs.fsInsul_SoftStart_Option=value;}
//        	//������� ��� �������� �������� ���������
//        	//��� ���������� �������� �����  											    0X020D	0�1015   381
//        	//#define ID_EDIT_INSUL_WITHOUTSOFTSTART					350
//        	else if (incase==0x020D){fs.fsInsul_WithoutSoftStart=value;}
//        	//������� ������ ������������ ������� ������  					arr21		    0�0213	0�1016   1001
//        	//#define ID_EDIT_LEVELSENSORLIMIT						425
//        	else if (incase==0x0213){fs.fsLevelSensorLimit=value;}
//        	//������� ��� ������� ��������� � ������� ������                arr[28]         0x021A  0x101C    700 ��� ���
//        	//#define ID_EDIT_SOFTSTART_PRM							450
//        	else if (incase==0x021A){fs.fsSoftStart_prm=value;}
//        	//�������̲� ����ֲ�															0X0208	0�1008    3
//        	//#define ID_CMB_TYPOROZMIR								475
////        	else if (incase==0x0208){fs.fsTyporozmir=value;} ��������� ����
//        	//���Ͳ���																		0�020�	0X101E    3
//        	//#define ID_CMB_PHASNOST								500
//        	else if (incase==0x020A){
//        		fs.fsPhasnost=value;}
//

        	else{}
		}
		/* �� 0�03 � �� 0�83 */
		else{
			snprintf (user_msga,USER_SESSAGE_SIZE," ����������� ������� RTU Modbus            ");
        }



		}
		break;
		case 0x06:{
	    if((mb_tx_msg->msg[1]==0x06)&&(mb_rx_msg->msg[1]==0x86)){
	    	if(mb_rx_msg->msg[2]==0x02){
	    		snprintf (user_msga,USER_SESSAGE_SIZE," ����� ������� �� ����������� ");
	    		tk5ProcessState=-1;
	    	}
	    	else{
	    		snprintf (user_msga,USER_SESSAGE_SIZE," ������� ��������� ������              ");
	    		tk5ProcessState=-1;
	    	}

	    }
	    /* ���� �� ��� ����� ������� */
	    else if ((mb_tx_msg->msg[1]==0x06)&&(mb_rx_msg->msg[1]==0x06)){
				uint16_t regNumber_Tx = mb_tx_msg->msg[2]*0x100 + mb_tx_msg->msg[3];
				uint16_t regValue_Tx = mb_tx_msg->msg[4]*0x100 + mb_tx_msg->msg[5];
				uint16_t regNumber_Rx = mb_rx_msg->msg[2]*0x100 + mb_rx_msg->msg[3];
				uint16_t regValue_Rx = mb_rx_msg->msg[4]*0x100 + mb_rx_msg->msg[5];
				if((regNumber_Tx==regNumber_Rx)&&(regValue_Tx==regValue_Rx)){
					tk5ProcessState=1;
				}
				else{
					snprintf (user_msga,USER_SESSAGE_SIZE," ������� ������ ������ ������� RTx");
					tk5ProcessState=-1;
				}
	    }
        else{
        	snprintf (user_msga,USER_SESSAGE_SIZE," ³������ �� ������� ������� ������");
        	tk5ProcessState=-1;
		}
		}//case 0x06:{
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
	    	  snprintf (str,sizeof(str)," a3_tk5z.c: �������� ������� ������� %d �� ��5", test_message_cntr_debug);
	    	  f_tk5Logging(str, sizeof(str));

	//        Run Indicator Status 1 Byte 0x00 = OFF, 0xFF = ON
	//        Additional Data
		}
		break;
		case 0x91:{
	    	  char str[256]={0};
	    	  snprintf (str,sizeof(str)," a3_tk5z.c: �������� ����������� ��� ������� �� �������");
	    	  f_tk5Logging(str, sizeof(str));
		}
		break;
		default:{}
		}
        /* ����� � ����� */
        ms=MODBUS_UNKNOWN_ERROR;/* ����� � WHILE */



	}//MODBUS_SUCCESS

    if(  strlen(user_msga) != 0){
    	memcpy(user_msg5,user_msga,5);
    }
    /* �� - ������������� ������ �����, ����� ���� ��..*/
    mb_rx_msg->length=0;
    return AnswerOkCntr;
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
	/* ������� ������ ������, �� ������ HEX*/
	float2_t fl={0};
	int err = f_checkEdit(ID_EditBox,hWndtk5,&fl);
	if(err != 0){f_valueEditMessageBox(err);}
	inaddr=fl.d;
	return inaddr;
}


/* �� ������� �������� ���������� ������ ������
 * �������� ������� ���"���������"*/
BOOL f_Get_tk5ModbussAddressOk(uint8_t mb_addr){
	BOOL condition = ((mb_addr==1)||(mb_addr==2));
	return condition;
}




void f_tk5Logging(char *str, size_t n_str){
	/* ���� ���� ����, � ���� ���������� ����������� ��� ���� -
	* �������� ���� � ���� */
	if((str != NULL)&&(strlen(str) != 0)){
		/* ����������, �� �������� ����, � ���� �������� -
		* ������� ��'� �����, � ���� i ���� �������*/
		fp_tk5LogFile= f_tk5check_logFilename(fp_tk5LogFile);
		SYSTEMTIME  SystemTime ={0};
		LPSYSTEMTIME lpSystemTime=&SystemTime;
		GetSystemTime(lpSystemTime);

		char wr[512+256]={0};
		snprintf(wr,23,"%02d:%02d:%02d     ",lpSystemTime->wHour+2,lpSystemTime->wMinute,lpSystemTime->wSecond);
		/* �������� ����� ��'� ����� ��� ������ ����  */
		strcat(wr,str);
		fprintf(fp_tk5LogFile,"\n%s",wr);
		memset(str,0,sizeof(n_str));

	}

}




