/*
 * 1main_paint.c

 *
 *  Created on: 26 ���. 2019 �.
 *      Author: KozakVF
 */
#include "stdio.h"
#include "a3_monitoring.h"
#include "../mb_funcs/tkRS232rx.h"

  extern HWND hwndMain;
  extern HWND hStatus;
  extern mb_mon_t mb_mon;
  extern int iVscrollPos;

   /* ʳ������ �������������� ��'����/�������� � �����*/
  extern uint8_t nBase;
	static RECT rect_Mn={0};
	static HDC HDC_Mn;
//  HDC hDC; // ������ ���������� ���������� ������ �� ������
//  extern PAINTSTRUCT ps; // ���������, ���-��� ���������� � ���������� ������� (�������, ���� � ��)
//  RECT rect; // ���-��, ������������ ������ ���������� �������
  COLORREF colorText = RGB(0, 255, 0); // ����� ���� ������
  CHAR db_str[3]={0x31,0x32,','};
  SYSTEMTIME SystemTime={0};
  extern SYSTEMTIME start_time;	  /* ������� ����������  */
  extern char StatusStr [100];
  extern SYSTEMTIME last_msg_time;/* ������ ����������� */
 /* �������� ��������� ���������� ������, �������������� � ���� md_base.c */
 /* ���� ������� ���������� ��������, �� ����*/
 static int old_msg_count=0;
 int db_istr=0;

 static void f_printMBmsg(RECT *prectX, SYSTEMTIME *systime, uint8_t *msg, uint8_t msg_size);


/* ϳ��� ������� ������������ ��������� ���������� ����� ������ (������ ����� ��������� ����)
 * �� ����������� Windows-������ ����������� �� ������� f_main_paint(), � ���:
 *
 * 1. ³���������� �������� ����� ���-���� f_OpenComport_forMB(), ���� 'tkRS232rx.c/h'
 * 2. ϳ��� ���������� ������ ���-����� f_rs232rx_buf_poll()
 * 2.1. ����� ����������� ������ ���������� � ���� ����� ����������,
 *      �� ��� ����� ������ ������ ���������� ������ ������� ����������
 *      f_crc_search() ���� 'tkRS232rx.c/h' ->  f_write_to_passport(), ���� mb_base.c/h
 * 2.2. �� ������� ���� ���������� ������� ������ �����������
 * 		������� �� ������ ����������� �� ������� �������� Slave
 * 		� ������� ��������� ������ Slave
 * 		f_paint_modbuss_msg()
 * 2.3.	� ����� ���� � ����� ��� ���� ���������� ���������� ��������� �����,
 * 		�� �� ���������� (�� ���� ���������� ������� ������)
 *      f_paint_modbuss_msg() -> check_coincidental_psp(), ���� mb_base.c/h
 *                                         |
 *                                         V � ���������� � ��� ����� ��������
 *                               f_printColorByte2(),��� ����
 * ����������� Windows-����� ������������ ��� ����� ������ ����� ����, ��� ��� ������� ����.
 * ��� ����� ����������� ���-����
 * */
void f_main_paint(void){

	f_OpenComport_forMB();
	GetSystemTime(&start_time);
    snprintf(StatusStr,100, "�����: %d.%d.%d   %d:%d ���������� 0",
    		start_time.wDay,
			start_time.wMonth,
			start_time.wYear,
			start_time.wHour+3,
			start_time.wMinute
			);
    SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)StatusStr);
 while (1){
	// �������� ������ � ������ ������� ��� ���������
	HDC_Mn= GetDC(hwndMain);
	GetClientRect(hwndMain, &rect_Mn);
	if (mb_mon==mb_mon_stopped){
		/* ��� ��������� ������ ���� ������� ����    */
		FillRect(HDC_Mn, &rect_Mn, (HBRUSH)(COLOR_WINDOW+1));
		ReleaseDC(hwndMain, HDC_Mn);
		f_main_paint_clouse();
//		mb_mon=mb_mon_starting;
		_endthread();
	}
	else if (mb_mon == mb_mon_poused){
		f_rs232rx_buf_poll();
	}
    else if(mb_mon == mb_mon_started){
    	/* ���������� ���� ������ ���-�����, �� ����� ����������� �� ������� ��� */
    	f_rs232rx_buf_poll();
    	/* ���� �������� ���� ����������� */
    	if (msg_count !=old_msg_count){
    		/* ������������ ������� �� �����*/
    		int numlines=f_tm();
    		SetScrollRange(hwndMain,SB_VERT,0,numlines-1,TRUE);
    		SetScrollPos (hwndMain, SB_VERT, iVscrollPos, TRUE) ;
    		old_msg_count=msg_count;
    		/*������� ������*/
    		FillRect(HDC_Mn, &rect_Mn, (HBRUSH)(COLOR_WINDOW+1));
    		f_paint_modbuss_msg(iVscrollPos);
    		ReleaseDC(hwndMain, HDC_Mn);

    		/* ���������� ������ ��� */
    		snprintf(StatusStr,100, "�����: %2d.%2d.%4d   %2d:%2d   ���������� %d    ������� %d   ������ ����������� %2d.%2d.%4d   %2d:%2d:%2d",
    			    		start_time.wDay,
    						start_time.wMonth,
    						start_time.wYear,
    						start_time.wHour+3,
    						start_time.wMinute,

    						(int)msg_count,
    						(int)(nBase-1),

    						last_msg_time.wDay,
    						last_msg_time.wMonth,
    						last_msg_time.wYear,
    						last_msg_time.wHour+3,
    						last_msg_time.wMinute,
    						last_msg_time.wSecond

    						);
    			    SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)StatusStr);
    	}
    }
    else{}
 } //while (1){
}


void f_main_paint_clouse(void){
	f_rs232rx_buf_poll_deInit();
}

void f_paint_modbuss_msg(int istr_file){

    	uint8_t n=4; /* ʳ������ ����������, �� ����������� � ������ ������� */
//    	int istr_scr =0; /* ����� ����� �� ����� */
//    	int istr_file=0;
    	int yBase=0; /* */
    	int yStr=0;
    	yBase = istr_file / (n+1);
    	yStr  = istr_file % (n+1);
//    	DrawText(hDC, sdate, 12, &rect, DT_SINGLELINE|DT_LEFT|DT_TOP);
        /* ��� ������� �������� */
    	RECT rectI = rect_Mn;
    	rectI.top=rectI.top + 100;
    	for (uint8_t iBase=0;iBase<nBase;iBase++){
    	 for(uint8_t jmsg=0;jmsg<n;jmsg++){
    		if (mb_passport[iBase].MSG[jmsg].n_msg!=0){
    			if (((iBase==yBase)&&(jmsg>=yStr))||(iBase>yBase)){
    				f_printMBmsg(
    						&rectI,
							&mb_passport[iBase].MSG[jmsg].last_activity_time,
							mb_passport[iBase].MSG[jmsg].last_msg,
							mb_passport[iBase].MSG[jmsg].n_msg
						);
    				rectI.top=rectI.top + Y_STRING;
    			}
    		}
    	 }
    	 if (iBase>=yBase){
    		 rectI.top=rectI.top + Y_STRING;
    	 }
    	}
    	/* ϳ��� ����, �� ��� ������ �� ���� - �������� ����� � ������������
    	 * � ��������� �������
    	 * ������� �������
    	 * � ��������� ����� ������� ������                                  */
    	for (int iPasp=0;iPasp<nBase;iPasp++){
    		check_coincidental_psp(iPasp);
    	}
}


/* ���� ������ �����������
  */
static void f_printMBmsg(
		RECT *prectX,        /* top ������������ ������, �� ������� �������� */
		SYSTEMTIME *systime,
		uint8_t *msg,
		uint8_t msg_size
		){

	RECT Rect0={0};
	memcpy (&Rect0, prectX, sizeof(RECT));
	Rect0.top=prectX->top-5;
	Rect0.bottom=Rect0.top+Y_STRING+5;
	FillRect(HDC_Mn, &Rect0, (HBRUSH)(COLOR_WINDOW+1));
	Rect0.top=prectX->top;
	/* ���� ���� 									  */
	char strTime[12]={0};
    sprintf(strTime,"%02d:%02d:%02d.%03d",
    		systime->wHour,
			systime->wMinute,
			systime->wSecond,
			systime->wMilliseconds
		);
    DrawText(HDC_Mn, strTime, 12, &Rect0, DT_SINGLELINE|DT_LEFT|DT_TOP);

    Rect0.left=prectX->left + X_TIME;
    /* ���� ����������� ���� �� ������ */
    for (uint16_t i=0;i<msg_size;i++){
	 Rect0.left=Rect0.left + X_BYTE;
	 char l3[3]={0};
	 l3[0]= f_xprint(msg[i]/0x10);
	 l3[1] =f_xprint(msg[i]%0x10);
	 l3[2] =',';
	 DrawText(HDC_Mn, l3, 3, &Rect0, DT_SINGLELINE|DT_LEFT|DT_TOP);
    }
    char msg_sizeCh[12]={0};
    sprintf (msg_sizeCh, "  size= %d", msg_size);
    Rect0.left=Rect0.left + X_BYTE;
    DrawText(HDC_Mn, msg_sizeCh, 12, &Rect0, DT_SINGLELINE|DT_LEFT|DT_TOP);
}


//void f_printColorByte(
//		int iPassport, /* ������ ��������, 0..127              */
//		int iMSG,      /* ������ ����������� � �������, 0..3 */
//		int iByte,     /* ������ �����, ���� ����� ��������  */
//		char Byte,     /* �������� �����, ���� ����� ��������*/
//		HBRUSH hbr     /* ���� �������                      */
//		){
//	RECT rectCB={0};
//
//
//	rectCB.top=Y_HEADER+iPassport*Y_PASSPORT+iMSG*Y_STRING+Y_STRING-5;
//	rectCB.left=X_TIME +iByte*X_BYTE+X_BYTE-5;
//	rectCB.right=rectCB.left+X_BYTE+5;
//	rectCB.bottom=rectCB.top+Y_STRING;
//	FillRect(hDC, &rectCB, (HBRUSH)(COLOR_WINDOW+6));
//	char l2[2]={0};
//	l2[0]= f_xprint(Byte/0x10);
//    l2[1] =f_xprint(Byte%0x10);
//    RECT rectCBT=rectCB;
//    rectCBT.top+=5;
//    rectCBT.left+=5;
//    DrawText(hDC, l2, 2, &rectCBT, DT_SINGLELINE|DT_LEFT|DT_TOP);
//}


void f_printColorByte2(
		int iPassport, /* ������ ��������, 0..127              */
		int iMSG,      /* ������ ����������� � �������, 0..3 */
		int iByte,     /* ������ �����, ���� ����� ��������  */
		char Byte,     /* �������� �����, ���� ����� ��������*/
		HBRUSH hbr     /* ���� �������                      */
		){
	uint8_t n=4; /* ʳ������ ����������, �� ����������� � ������ ������� */
//	int istr_scr =0; /* ����� ����� �� ����� */
//    	int istr_file=0;
	int yBase=0;
	int yStr=0;
	yBase = iVscrollPos / (n+1);
	yStr  = iVscrollPos % (n+1);
//    	DrawText(hDC, sdate, 12, &rect, DT_SINGLELINE|DT_LEFT|DT_TOP);
    /* ��� ������� �������� */
	HDC hDC;
	hDC= GetDC(hwndMain);
	RECT rectI = rect_Mn;
	rectI.top=rectI.top + 100;
	for (int iBase=0;iBase<nBase;iBase++){
	 for(int jmsg=0;jmsg<n;jmsg++){
		if (mb_passport[iBase].MSG[jmsg].n_msg!=0){
			if (((iBase==yBase)&&(jmsg>=yStr))||(iBase>yBase)){
				if (iPassport==iBase){
				 if(iMSG==jmsg) {
				    RECT rectCB={0};
					rectCB.top=rectI.top-5;//Y_HEADER+iPassport*Y_PASSPORT+iMSG*Y_STRING+Y_STRING-5;
					rectCB.left=X_TIME +iByte*X_BYTE+X_BYTE-5;
					rectCB.right=rectCB.left+X_BYTE+5;
					rectCB.bottom=rectCB.top+Y_STRING;
					FillRect(hDC, &rectCB, (HBRUSH)(COLOR_WINDOW+6));
					char l2[2]={0};
					l2[0]= f_xprint(Byte/0x10);
				    l2[1] =f_xprint(Byte%0x10);
				    RECT rectCBT=rectCB;
				    rectCBT.top+=5;
				    rectCBT.left+=5;
				    DrawText(hDC, l2, 2, &rectCBT, DT_SINGLELINE|DT_LEFT|DT_TOP);
				    return;
				 }
				}
				rectI.top=rectI.top + Y_STRING;
			}
		}
	 } //for(int jmsg=0;jmsg<n;jmsg++){
	 if (iBase>=yBase){
		 rectI.top=rectI.top + Y_STRING;
	 }
	}//for (int iBase=0;iBase<nBase;iBase++){
}
