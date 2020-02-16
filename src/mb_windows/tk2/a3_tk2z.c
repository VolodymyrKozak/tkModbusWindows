/*
 * a3_tk2z.c
 *
 *  Created on: 2 ��. 2020 �.
 *      Author: KozakVF
 */

#include "a3_tk2z.h"
#include "../../wConfig.h"
#include "a00_tk2Config.h"
#include <stdio.h>
#include <stdbool.h>
#include "../../mb_funcs/vk_wqueue.h"
#include <commctrl.h>
#include <winuser.h>
#include "../a3_tkTxRx.h"
#include "a4_tk2log.h"

/* ���������, ��� ������� �����
 * ���������� ���� ��������� ������, �� ��������
 * ��������� ����� � ���������� ������, ���� �� �����
 * �������� ���������� ����� ���������� �� ��2*/
extern   queue_t tk2Queue;
extern FILE *fp_tkLogFile;


// 0x0205	������� ������ �� ������� ����������, �	10.0 �  600.0	I�	arr[2]
extern float2_t Itk2_Max;
//0x0206	������� ���������� ����������� ���� (����������), � 	10.0�600.0 I��	arr[3]
extern float2_t Itk2_Min;

/* ��������� ��������� ���������� ����������� */
extern d_type_t q;
extern tk2_fs_t fs;
uint8_t tk2_Addr=0x02;


char user_msg112[256]={0};

/* ������ ���� �� ���������� ��� ������:
 *   0 - ����� �� ����� ��� ���������� �������
 *   1 - �� ���� ���� ������ ������������� ���������
 *       ��� ����������� ����������� ������ �� ����������
 *   2 - �� ���� ���� ������ � ���������� ��'���� PC_DESC <-> ���������
 *       ��� ����������� ����������� ������ �� ����������
 *   -1  ���� ������ ����������� � ��������, ������� ������� ������� */
int tk2ProcessState = 0;

/* ˳������� ��������� �� ��2 ����������-������.
 * �������������� �������� f_tk2QPWD_RgAnswer(), ���� 'a3_tk2z.h'
 * ����������, ��� ��������� ����������� ���������� ���� ���� ���,
 * ���� ������� ���� ����������� */
uint64_t tk2ProcessCntr=0;


uint16_t tk2_PasswpedFlag = 0;
uint16_t typorozmir = 0xFFFF;
FunctionalState us_access_status = DISABLE;
FunctionalState us_access_statusOld = DISABLE;
//RG_R_ACCESS_STATUS

/***************************************************************************************************************************************
ANSWER                                                                                                    ANSWER ANSWER ANSWER ANSWER
*****************************************************************************************************************************************
*/
static uint32_t AnswerOkCntr = 0;
/* ������� - �������� ������ �� ��2
 * ��������� �� ����, � ����� ����������� �����������.*/
/* ��������� - ���������, ������� �� ��2
 * ��������� - �������� ������� ������� (��� ���������� ���������� ��� ������� ������)*/
uint32_t f_tk2QPWD_RgAnswer(
		modbus_status_t ms,
		modbus_master_tx_msg_t *mb_tx_msg,
		modbus_master_rx_msg_t *mb_rx_msg,
		int incase){

    char user_msga[256]={0};
	memset(user_msga, 0,sizeof(user_msga));
	tk2ProcessCntr++;
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
		tk2ProcessState=-1;
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
			tk2ProcessState=-1;
		}
		/* �� ������������ ����������� */
		else if((mb_tx_msg->msg[1]==0x03)&&(MbCode==0x03)){
        	uint16_t value=mb_rx_msg->msg[3]*0x100 + mb_rx_msg->msg[4];
        	/* ��������� ���������� ����� ������� */
        	if(incase==ID_TK2_POOL){
        		/* */
//        		uint16_t value0 = mb_rx_msg->msg[3]*0x100 + mb_rx_msg->msg[4];
////        		�������� ����� ������	0�0
//        		q.d_regime=(control_type_t)value0;
//        		����� ����������� ������ �� ��������� (���-1, ����.0)	0�1
//        		uint16_t value1 = mb_rx_msg->msg[5]*0x100 + mb_rx_msg->msg[6];
//        		q.d_AutoCtrl=(on_off_t)value1;
//        		���� �������������� ���-1, ����-0	0�2
        		uint16_t value2 = mb_rx_msg->msg[7]*0x100 + mb_rx_msg->msg[8];
        		q.d_MotorState=(on_off_t)value2;
//        		��� ����	0�3
        		uint16_t value3 = mb_rx_msg->msg[9]*0x100 + mb_rx_msg->msg[10];
        		q.d_motorfault=(on_off_t)value3;
//        		г���� ����� � ���������/������ ���� ����� �� ������ ����
        		uint16_t value4 = mb_rx_msg->msg[11]*0x100 + mb_rx_msg->msg[12];
        	    uint8_t cx = value4 % 2;
				q.d_CX=(on_off_t)cx;
        	    uint8_t nu = (value4 >> 1)% 2;
        	   	q.d_NU=(on_off_t)nu;
        	    uint8_t vu = (value4 >> 2)% 2;
        	   	q.d_VU=(on_off_t)vu;
//        	   	����� �������  	0�5
        	   	tk2_PasswpedFlag = mb_rx_msg->msg[13]*0x100 + mb_rx_msg->msg[14];
        	   	//        		�������� ����� ���� �	0�6	0.0..600.0�            (�������� �������  /10)

        	   	uint16_t value6 =  mb_rx_msg->msg[15]*0x100 + mb_rx_msg->msg[16];
        	   	q.d_aAmperage=(amperf_t)value6 /10.0f;
        	   	//        		�������� ����� ���� B	0�7
        	   	uint16_t value7 =  mb_rx_msg->msg[17]*0x100 + mb_rx_msg->msg[18];
        	    q.d_bAmperage=(amperf_t)value7 /10.0f;
        	    //        		�������� ����� ���� C 	0�8
                uint16_t value8 =  mb_rx_msg->msg[19]*0x100 + mb_rx_msg->msg[20];
                q.d_cAmperage=(amperf_t)value8 /10.0f;
//              ������ �������� ���������� ��������� ����� ��������	0�9
                uint16_t value9 =  mb_rx_msg->msg[21]*0x100 + mb_rx_msg->msg[22];
                q.d_InsulationResistance=(float)value9;   /* 100..2500 */
//              �������� ������	0��
                uint16_t value10 = mb_rx_msg->msg[23]*0x100 + mb_rx_msg->msg[24];
                q.d_AmperageUnbalance=(float)value10;      /*0..90% */
//        		�������� ����� ������� ������� ������ ����	0�B	100..2500	����� ���
                q.d_DryModeADC=mb_rx_msg->msg[25]*0x100 + mb_rx_msg->msg[26];
//              �������� ����� ������� ������� �������� ���� 	0��
                q.d_LowLevelADC=mb_rx_msg->msg[27]*0x100 + mb_rx_msg->msg[28];
//              �������� ����� ������� ������� ��������� ����	0�D
                q.d_HighLevelADC=mb_rx_msg->msg[29]*0x100 + mb_rx_msg->msg[30];
                /* ˳������� ����� */
//              uint16_t valueE = mb_rx_msg->msg[31]*0x100 + mb_rx_msg->msg[32];
                tk2ProcessState=1;
        	}
        	else if(incase==ID_TK2_POOL_MODE){

        		if((value>0)&&(value<6)){
        			q.d_regime=(control_type_t)value;
        			tk2ProcessState=1;
        		}
        		else{
        			tk2ProcessState=-1;
        			snprintf (user_msga,USER_SESSAGE_SIZE," �������� �� ��2 �������� ������ �� �����������");
        		}
        	}
        	else if((incase==ID_TK2_POOL_PUMPING_MODE)||(incase==ID_READ_US_PUMPING_MODE)){
        		if((value>0)&&(value<6)){
        			q.tk2_PumpingMode=value;
        			tk2ProcessState=1;
        		}
        		else{
        			tk2ProcessState=-1;
        			snprintf (user_msga,USER_SESSAGE_SIZE," �������� �� ��2 �������� ���� ������ �� �����������");
        		}
        	}
        	else if(incase==RG_R_TK2_SELFSTART){//||(incase==ID_READ_US_SELF_START)){
        		if(value<3){
        			q.d_FS=(FunctionalState)value;
        			tk2ProcessState=1;
        		}
        		else{
        			tk2ProcessState=-1;
        			snprintf (user_msga,USER_SESSAGE_SIZE," �������� �� ��2 �������� ����������� �� �����������");
        		}

        	}
        	else if((incase==ID_TK2_POOL_MODBUSS_ADDRESS)||(incase==ID_READ_US_MODBUS_ADDR)){
        		if (f_Get_tk2ModbussAddressOk(value)==true){
        			tk2_Addr=(control_type_t)value;
        			tk2ProcessState=1;
        		}
        		else{
        			tk2ProcessState=-1;
        			snprintf (user_msga,USER_SESSAGE_SIZE," �������� �� ��2 �������� ������ ������ �� �����������");
        		}
        	}
        	else if(incase==IDB_TEST_DISTANCE_CTRL){
        		//                      0123456789012
        		snprintf (user_msga,USER_SESSAGE_SIZE,"TK112-H1-��/");

        		user_msga[12]=mb_rx_msg->msg[4]+0x30;
        		tk2ProcessState=2;
        	}
        	else if(incase==ID_READ_US_MAX_AMPERAGE){
        		 Itk2_Max.d=value/10;
        		 Itk2_Max.f=value%10;
        		 Itk2_Max.floatf=(float)value/10.0f;
        		tk2ProcessState=1;
        	}
        	else if(incase==ID_READ_US_MIN_AMPERAGE){
       		    Itk2_Min.d=value/10;
       		    Itk2_Min.f=value%10;
       		    Itk2_Min.floatf=(float)value/10.0f;
        		tk2ProcessState=1;
        	}
        	else if (incase==RG_R_TK2_TYPOROZMIR){
        		if((value>0)&&(value<9)){
        			fs.fsTyporozmir=value;
        			typorozmir=value;
        			tk2ProcessState=1;
        		}
        		else{
        			tk2ProcessState=-1;
        			snprintf (user_msga,USER_SESSAGE_SIZE," �������� �� ��2 �������� �������̲�� �� �����������");
        		}
        	}
        	else if (incase==RG_R_TK2_ACCESS_STATUS){
        		if(value<3){
        			us_access_status = (FunctionalState)(value);
        			tk2ProcessState=1;
        		}
        		else{
        			tk2ProcessState=-1;
        			snprintf (user_msga,USER_SESSAGE_SIZE," �������� �� ��2 �������� ������� ������� �� �����������");
        		}
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
        			fs.fsHeatingTimeConstant=value;
        		}
        	}
        	//�������� ʲ��ʲ��� ���ʲ� �� �������� 10��									0X0216	0X1014   5
        	//#define ID_CMB_NUMBERSTARTLIMIT						    175
        	else if (incase==0x0216){
        		if((value>0)&&(value<10)){
        			fs.fsNumberStartLimit=value;
        		}
        		else{
        			snprintf (user_msga,USER_SESSAGE_SIZE," �������� �� ��2 �������� ����� ����Ҳ� �� 10�� �������");
        		}
        	}
        	//�������� ����� �� �� в���													0�0214	0�1006   4..
        	//#define ID_EDIT_ONSENSORMOTORDELAY					    225
        	else if (incase==0x0214){fs.fsOnSensorMotorDelay=value;}
        	//�������� ������� �� �� в���													0�0215	0�1007   12
        	//#define ID_EDIT_OFFSENSORMOTORDELAY						250
        	else if (incase==0x0215){fs.fsOffSensorMotorDelay=value;}
        	//�������� ²��������� ������ ������� ������� ���ӻ ϲ��� ���в� ������� ���ӻ	0�0217	0�1012   13
        	//#define ID_EDIT_AFTRERDRYMOVEDELAY						275
        	else if (incase==0x0217){fs.fsAftrerDryMoveDelay=value;}
        	//����� ����� ����� �����ֲ�/��.����                             ARR[26]		0X0219	0X101A	 0/1?
        	//#define ID_CMB_INSUL_SOFTSTART_OPTION					325
        	else if (incase==0x0219){fs.fsInsul_SoftStart_Option=value;}
        	//������� ��� �������� �������� ���������
        	//��� ���������� �������� �����  											    0X020D	0�1015   381
        	//#define ID_EDIT_INSUL_WITHOUTSOFTSTART					350
        	else if (incase==0x020D){fs.fsInsul_WithoutSoftStart=value;}
        	//������� ������ ������������ ������� ������  					arr21		    0�0213	0�1016   1001
        	//#define ID_EDIT_LEVELSENSORLIMIT						425
        	else if (incase==0x0213){fs.fsLevelSensorLimit=value;}
        	//������� ��� ������� ��������� � ������� ������                arr[28]         0x021A  0x101C    700 ��� ���
        	//#define ID_EDIT_SOFTSTART_PRM							450
        	else if (incase==0x021A){fs.fsSoftStart_prm=value;}
        	//�������̲� ����ֲ�															0X0208	0�1008    3
        	//#define ID_CMB_TYPOROZMIR								475
//        	else if (incase==0x0208){fs.fsTyporozmir=value;} ��������� ����
        	//���Ͳ���																		0�020�	0X101E    3
        	//#define ID_CMB_PHASNOST								500
        	else if (incase==0x020A){
        		fs.fsPhasnost=value;}
        	else if (incase==0x021d){
        		/* � ������������� ����� ������ �� �������� ��������/��������� */
        		q.d_AutoCtrl=(on_off_t)value;
        	}
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
	    		tk2ProcessState=-1;
	    	}
	    	else{
	    		snprintf (user_msga,USER_SESSAGE_SIZE," ������� ��������� ������              ");
	    		tk2ProcessState=-1;
	    	}

	    }
	    /* ���� �� ��� ����� ������� */
	    else if ((mb_tx_msg->msg[1]==0x06)&&(mb_rx_msg->msg[1]==0x06)){
				uint16_t regNumber_Tx = mb_tx_msg->msg[2]*0x100 + mb_tx_msg->msg[3];
				uint16_t regValue_Tx = mb_tx_msg->msg[4]*0x100 + mb_tx_msg->msg[5];
				uint16_t regNumber_Rx = mb_rx_msg->msg[2]*0x100 + mb_rx_msg->msg[3];
				uint16_t regValue_Rx = mb_rx_msg->msg[4]*0x100 + mb_rx_msg->msg[5];
				if((regNumber_Tx==regNumber_Rx)&&(regValue_Tx==regValue_Rx)){
					tk2ProcessState=1;
				}
				else{
					snprintf (user_msga,USER_SESSAGE_SIZE," ������� ������ ������ ������� RTx");
					tk2ProcessState=-1;
				}
	    }
        else{
        	snprintf (user_msga,USER_SESSAGE_SIZE," ³������ �� ������� ������� ������");
        	tk2ProcessState=-1;
		}
		}//case 0x06:{
		break;
		default:{}
		}
        /* ����� � ����� */
        ms=MODBUS_UNKNOWN_ERROR;/* ����� � WHILE */



	}//MODBUS_SUCCESS

    if(  strlen(user_msga) != 0){
    	memcpy(user_msg112,user_msga,256);
    }
    mb_rx_msg->length=0;
    return AnswerOkCntr;
}

int f_Set112TxReadReg(uint16_t addr,modbus_master_tx_msg_t *tx){
	tk2_Addr= f_GetModbussAddress(ID_TK2_EDIT_ADDRESS);if(tk2_Addr<0){DestroyWindow(hWndTk2);}
	memset(tx,0,sizeof(modbus_master_tx_msg_t));
	tx->msg[0]=tk2_Addr;
	tx->msg[1]=0x03;
	tx->msg[2]=addr/0x100;
	tx->msg[3]=addr%0x100;//
	tx->msg[4]=0x00;
	tx->msg[5]=0x01;
	tx->length=6;
	return tx->length;
}
int f_Set112TxWriteReg(uint16_t addr, uint16_t value,modbus_master_tx_msg_t *tx){
	tk2_Addr= f_GetModbussAddress(ID_TK2_EDIT_ADDRESS);if(tk2_Addr<0){DestroyWindow(hWndTk2);}
	memset(tx,0,sizeof(modbus_master_tx_msg_t));
	tx->msg[0]=tk2_Addr;
	tx->msg[1]=0x06;
	tx->msg[2]=addr/0x100;
	tx->msg[3]=addr%0x100;//
	tx->msg[4]=value/0x100;
	tx->msg[5]=value%0x100;
	tx->length=6;
	return tx->length;
}





int16_t f_GetModbussAddress(int ID_EditBox){
	memset(&user_msg112,0,sizeof(user_msg112));
	char inaddr=0;
	/* ������� ������ ������, �� ������ HEX*/
	float2_t fl={0};
	int err = f_checkEdit(ID_EditBox,hWndTk2,&fl);
	if(err != 0){f_valueEditMessageBox(err);}
	inaddr=fl.d;
	return inaddr;
}

/* �� ������� �������� ���������� ������ ������
 * �������� ������� ���"���������"*/
BOOL f_Get_tk2ModbussAddressOk(uint8_t mb_addr){
	BOOL condition = ((mb_addr==1)||(mb_addr==2));
	return condition;
}

/* �� ������� ����������� � 10� ������� � �������� �� �� ��2 ������� �����������, ��� ������ ���������
 *  */
int f_tk2UpdateStat(
		HWND hwnd,
		HWND hProcesBar,
		UINT_PTR pTimer,
		int timer_ID
		){
	uint16_t x_right_panel 				= 839;
	char pm[80]={0};
	/* ����� �������� ��� ��������� ������ � ����*/
	HDC hDCr; // ������ ���������� ���������� ������ �� ������
	RECT rect; // ���-��, ������������ ������ ���������� �������
	/*������� ������*/
	hDCr= GetDC(hwnd);
	GetClientRect(hwnd, &rect);  	// �������� ������ � ������ ������� ��� ���������
	   /***********************************************************************************************************
	   * ������ � ������ ��²������� ������
	   ***********************************************************************************************************/
	    //* �������� ������� �����, ��������� ��������� RS232*/
		uint16_t iQ= f_get_tkrs232_length(&tk2Queue);
	//	uint16_t iQ=f_get_tk2queue_length();
		int32_t pbQ=10000-10000/(iQ+1);
	//	if(iQ!=iQold){iQold=iQ;}
		float alpha=0.95;
		float MbQ=0;
		if(iQ>0){MbQ=(float)pbQ;}
		else{MbQ=MbQ*alpha;}
		/* ³���������� ������ ��� ��������� ����� ���������� ������*/

	//	snprintf(tk2LogStr,80,"MbQ = %f", MbQ);
	//	f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));

		SendMessage(hProcesBar, PBM_SETPOS,
				    (int)MbQ, //Signed integer that becomes the new position.
					0);   //Must be zero
		/* Գ����� ������� ����� ������ �� ����� */


		/* ���������� */

		memset(&pm,0,sizeof(pm));
		/* ���������� */
		snprintf(pm,80,"%9d", (uint32_t)gTxRxSessionStartCntr);
		RECT rect_av = {rect.left+x_right_panel+100, rect.top+585, rect.left+x_right_panel+170+100, rect.top+660+20};
		DrawText(hDCr, pm, 9, &rect_av, DT_SINGLELINE|DT_LEFT|DT_TOP);
		memset(&pm,0,sizeof(pm));
		/* ���� */
		uint32_t d2=(uint32_t)((gTxRxSessionSuccessDegreef+0.001)*100.0f)/100;
		uint32_t f2=(uint32_t)((gTxRxSessionSuccessDegreef+0.001)*100.0f)%100;
		snprintf(pm,80,"  %4d.%02d", d2,f2);
		RECT rect_a = {rect.left+x_right_panel+100, rect.top+610, rect.left+x_right_panel+170+100, rect.top+660+20};
		DrawText(hDCr, pm, 9, &rect_a, DT_SINGLELINE|DT_LEFT|DT_TOP);
		memset(&pm,0,sizeof(pm));
		/* ��������� */
		uint32_t d1=(uint32_t)(gTxRxBusyDegreef*100.0f)/100;
		uint32_t f1=(uint32_t)(gTxRxBusyDegreef*100.0f)%100;
		snprintf(pm,80,"    %02d.%02d", d1,f1);
		RECT rect_b = {rect.left+x_right_panel+100, rect.top+635, rect.left+x_right_panel+170+100, rect.top+660+20};
		DrawText(hDCr, pm, 9, &rect_b, DT_SINGLELINE|DT_LEFT|DT_TOP);
		memset(&pm,0,sizeof(pm));
		/* ����� */
		snprintf(pm,80,"      %2d", iQ);
		RECT rect_E = {rect.left+x_right_panel+100, rect.top+660, rect.left+x_right_panel+170+100, rect.top+660+20};
		DrawText(hDCr, pm, 9, &rect_E, DT_SINGLELINE|DT_LEFT|DT_TOP);
		memset(pm,0,sizeof(pm));


		if(iQ>40){
			KillTimer(hwnd, pTimer);

			const int result = MessageBox(NULL,
					"����� ���������� ��� ������ �����������\n��������� �����?",
					"Modbus Error",
			 MB_YESNO);
			switch (result){
			case IDYES:{
				f_clear_tkqueue(&tk2Queue);
			   /* ������������� �������� ������ */
	//		   p10Timer=SetTimer(
	//				hwnd,                // handle to main window
	//				IDT_TIMER_TK2_10MS,  // timer identifier
	//				TIMER_MAIN_MS,                  // msecond interval
	//				(TIMERPROC) NULL     // no timer callback
	//				);

			  pTimer=SetTimer(
					hwnd,                // handle to main window
					timer_ID,// timer identifier
					TIMER_PERIOD_TK2_1000_MS,                // msecond interval
					(TIMERPROC) NULL     // no timer callback
					);
			}
			break;
			case IDNO:{
					// Do something
			} //case IDT_TIMER_TK2_1333MS:
			break;
			default:{}
			}

	//		EnumChildWindows(hWndTk2 /* parent hwnd*/, UpdateQeueChildProcessBarS, 0);
		}
	ReleaseDC(hwnd, hDCr);
	return 0;
}


void f_tk2Logging(char *str, size_t n_str){
	/* ���� ���� ����, � ���� ���������� ����������� ��� ���� -
	* �������� ���� � ���� */
	if((str != NULL)&&(strlen(str) != 0)){
		/* ����������, �� �������� ����, � ���� �������� -
		* ������� ��'� �����, � ���� i ���� �������*/
		fp_tkLogFile= f_tk2check_logFilename(fp_tkLogFile);
		SYSTEMTIME  SystemTime ={0};
		LPSYSTEMTIME lpSystemTime=&SystemTime;
		GetSystemTime(lpSystemTime);

		char wr[512+256]={0};
		snprintf(wr,23,"%02d:%02d:%02d     ",lpSystemTime->wHour+2,lpSystemTime->wMinute,lpSystemTime->wSecond);
		/* �������� ����� ��'� ����� ��� ������ ����  */
		strcat(wr,str);
		fprintf(fp_tkLogFile,"\n%s",wr);
		memset(str,0,sizeof(n_str));
	}

}




