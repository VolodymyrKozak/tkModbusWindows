
#include "math.h"
#include <process.h>    /* _beginthread, _endthread */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <winbase.h>
#include <windef.h>
#include <wingdi.h>
#include <winuser.h>
#include <float.h> //
#include "../../mb_funcs/tw_mb.h"
#include "../../mb_funcs/tx_manual.h"
#include "../a0_win_control.h"
#include "../a2_setting.h"
#include "a5_tk2levels.h"
#include "a20_tk2.h"
#include "a21_tk2us.h"
#include "a3_tk2z.h"
#include "../../mb_devs/tk2_dist.h"
#include "../e1_passwordEdit.h"
#include "a22_tk2fs.h"
#include "../../mb_devs/tk2/tkThermal.h"
#include "../a3_tkTxRx.h"
#include "a4_tk2log.h"


/********************************************************************************************************************************
 *  �̲�Ͳ �� ����ֲ� �ղ����� (���������) ²��� ������ֲ����� �����˲��� PC_DESK <-> TK2
 * ******************************************************************************************************************************
 * */


/* ���������, ��� ������� �����
 * ���������� ���� ��������� ������, �� ��������
 * ��������� ����� � ���������� ������, ���� �� �����
 * ��������� ��������� - ���������� ����� ���������� �� ��2,
 * ������ � ������ ����������� ��������� ����� 'a3_tk2TxRx.c/h' */
   queue_t tk2Queue = {0};

   /* ��������� �������� �� ���������� �����, ���� ������ ������������ ����
    * ��'� ����� ����������� �����������,
    * ������ � ������ ����������� ��������� �����  a4_tk2log.c/h*/
   FILE *fp_tkLogFile=NULL;
   /* �����, ���� ���������� ����� ����. ������������ ��������� � ������ 10��,
    * ���� �� ������, ����� ���������� � ����.
    * ���� ����� ������ - ����� ������ �������� � ����, ���. ������� f_tk2Logging(), ���� a3_tk2z.c/h */
   char tk2LogStr[512]={0};

/* ��� ��������� ����, ������ �� ���� �� �� ����� ������ ������,
 * ������������ ���� � �������, �� � �������� ��� ��� ������ ������
 *
 * ��� ��������� ������ '����.����.���������' ��������� ����� �� ���������� ���������� �������
 * �������� ���� ������������ �������� ������ ������ � TK2
 * ����� ������ ����������.
 * ��� ��������� ������ '����� ����.���������'
 * ����������� ������ ����� ������������� PC_DESK <-> TK2,
 * � ����� �� ���������� "���� ����.���"*/
ProcessState_t tk2Process=ProcessIddle;
 /* �� ������ ���������� ������� ������������� ���������.
  * ��� ���������  ������ '����� ����.���������' tk2Process=ProcessBusy
  * ��� ���������   '����� ����.���������' tk2Process=ProcessIddle
  * */

/* ���������� �� ������ ������������ ����� ����
 * ��������, ������ �� ���������� ������� ���������� �� ������*/
static FunctionalState tk2btn_access = ENABLE;
/* ˳������� ��������*/
static uint32_t tk2btnSleepCntr = 0;
/* ��������� �������� */
#define  TK2_BTN_BLOKING_MS         400


/* ��������� ���������, �� ������ ���������� ��������� ������
 * ������ ��2 - ��������� �� ������� �������������/��������������*/
d_type_t q={0};
/* �������� �������� ��������� ��� ������ � ���� ���� ���, �� ��������*/
static d_type_t qOld={0};
/* ������ � ��������� � ������-����� ���������� ���������� */
extern brl_t brl;

/* ���������� ����� ��� ������ ������-���� ������� ����� �������������� */
extern float2_t Itk2_Max;

HWND hWndTk2 = NULL;
HINSTANCE hinst_tk2;
static  HFONT hfDefault;
const char g_szClassNameTK112[] = "TK112_WndClass";


/* �������� ����� ��� ���������� �������� ������� ���� ������*/
static event_t  met_zero_MBsuccess = NO_EVENT;
/* ˳������� ���� � ������ 100�� ��� �������� ����� ���������� � ����� ��������*/
static uint32_t tk2TimerCntr = 0;


/* �����p ���� �������, ��������� �������,
 * � ��� ��� ���������� ���� ��������
 * �� ������� �� ������� � �� ������� �� �������*/
/* � ����� ���� ����������� ������� �������, ��� ������������� ������ */
int RW_case=0;
/* �������� ����, ����������� �� ������� � ����, ��� ����������� ������ � ����� */
extern modbus_master_rx_msg_t mIn_rx_msg;
/* �������� ����, ����������� �� ������� � ����, ��� ����������� ������ � ����� */
extern modbus_master_tx_msg_t mOut_tx_msg;
/* �������� ����, ����������� �� ������� � ����, ��� ����������� ������ � ����� */
extern modbus_status_t RxMasterModbusStatus;


/* ���������� ��� ��������� ���� */
/* ������� �������� ���������� Y ����� ������������� */
static uint32_t y_LeftRadio = 300;
/* ��� ������ ����� ���� 1024 �� ������ 1009 */
static uint16_t x_left_panel  				= 20;
static uint16_t x_left_right_width_panel	= 150;
static uint16_t x_centr_panel   			= 190;
static uint16_t x_centr_width 				= 629;
static uint16_t x_right_panel 				= 839;
/* ����/����������� �� ������������ ������������ �������,
 * ����������� � ���� PPB ������������ ������� */
static uint16_t yMode 					= 80;
static uint16_t yPumpingMode 			= 105;
static uint16_t ySelfStart     			= 180;
static uint16_t y_AutoCtrl				= 205;
static uint16_t y_motorfault			= 255;
static uint16_t y_aAmperage				= 305;
static uint16_t y_bAmperage             = 330;
static uint16_t y_cAmperage				= 355;
static uint16_t y_InsulationResistance  = 380;
static uint16_t y_AmperageUnbalance     = 405;
static uint16_t y_DryModeADC            = 430;
static uint16_t y_LowLevelADC           = 455;
static uint16_t y_HighLevelADC          = 480;
static uint16_t y_PBBarrel_left         = 445;

/* ��� ������� - ������, �������, ���-����, ���������� � �.�.*/
extern HWND comWndChild;
static HWND  hEdit;
static HWND  hBtnViewAutoctrl;
static HWND  hWndViewBtnMotorState;
static HWND  hBtnViewCX;
static HWND  hWndProgressBar_Barrel;
static HWND  hWndProgressBar_ThermalModel;
static HWND  hWndProgressBar_Queue;
static HWND  hButtonSend;
static HWND  hEdit_TK2_ADDR;
static HWND  hABtnHighPanel;
static HWND  hABtnStart;
static HWND  hABtnStop;
static HWND  hABtnBuster;
static HWND  hABtnLowPanel;
static HWND  hABtnToR;

static HWND  hRBtnHighPanel;
static HWND  hRBtnStart;
static HWND  hRBtnStop;
static HWND  hRBtnLowPanel;
static HWND  hRBtnToA;

UINT_PTR  p1000Timer;
static UINT_PTR  p1333Timer;
static UINT_PTR  p10Timer;
static UINT_PTR  p2000Timer;
static UINT_PTR  p50Timer;
/* �� ������� ����������� ����� ��� ��� �������� ����������� � VK_UPDATE_WINDOW
 * ��� ���������� ��� ������, ���������� � ��������� ����*/
static int f_tk2UpdateWindow(HWND hwnd);
/* �� �������-�������� ������� ������������ ������� */
static void f_desktop_tk2_session(void);
/* �����������, ��� �������� ��������� ������� ��������� ��� ����������� ����������� */
static void f_InitTK2_Drow(void);

/* ҳ���� ��� ������� ��������� ������ ��� ��� ������ ������ ���� <-> ���.���� */
static BOOL CALLBACK DestoryChildCallbackAR(/* ���->����*/
  HWND   hwnd,
  LPARAM lParam
);
static BOOL CALLBACK DestoryChildCallbackRA(/* ����->���*/
  HWND   hwnd,
  LPARAM lParam
);


#ifndef		TK2_DISTANCE_WND
/* �� �������� ���� ����.�����˲��� �� ��������� � �������� �������,
���� ���� ���������� ��������� ��� */
LPCSTR f_RegisterTK112WndClass(HINSTANCE hinst){
	hinst_tk2=hinst;
	WNDCLASS w;
		memset(&w,0,sizeof(WNDCLASS));
		w.lpfnWndProc = WndProcTK112;
		w.hInstance = hinst;
		w.style = CS_HREDRAW | CS_VREDRAW;
		w.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
		w.lpszClassName = g_szClassNameTK112;
		w.hIcon	 =
		CreateIcon(
			hinst,       	// application instance
			32,             // icon width
			32,             // icon height
			1,              // number of XOR planes
			1,              // number of bits per pixel
			ANDmaskTK_Icon, // AND bitmask
			XORmaskTK_Icon  // XOR bitmask
			);
		w.hCursor=LoadCursor(NULL, IDC_ARROW);
//		RegisterClassA(&w);
//		volatile int yjt=GetLastError();
		if(!RegisterClassA(&w))
		{
//		int Error = GetLastError();
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);

		}
		return w.lpszClassName;
}
int WINAPI f_CreateTK112Wnd(HWND hh1, HINSTANCE hInstance){
	fp_tkLogFile=f_tk2log_Init();
	HWND child112;
	MSG Msg;
	child112 = CreateWindowEx(
		0,
		g_szClassNameTK112,
		"��2.����������� ���������",

//        WS_CHILD
//		| (WS_OVERLAPPEDWINDOW&(~(WS_MAXIMIZEBOX|WS_MINIMIZEBOX)))
//		|WS_BORDER
//		|WS_VISIBLE
//
//		,

		(WS_OVERLAPPEDWINDOW&(~(WS_MAXIMIZEBOX|WS_MINIMIZEBOX)))|WS_BORDER|WS_VISIBLE
		,
		CW_USEDEFAULT, CW_USEDEFAULT, 1024, 800,
		NULL, NULL, hinst_tk2, NULL
		);


	if(child112 == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	/* ��������� ����� ����, �� ������������ ������������
	 * ������ ������������ ������ */
	f_RegisterTK112USWndClass(hInstance);
	f_RegisterTk2fsWndClass(hInstance);
	hWndTk2=child112;

	HMENU hMenubar = CreateMenu();
	HMENU hMenu = CreateMenu();

	AppendMenu(hMenu, MF_STRING, IDM_COMPORT_SETTING, "&������������ ���-�����");
	AppendMenu(hMenu, MF_STRING, IDM_USER_SETTINGS, "&������������ �����������");
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenu, MF_STRING, IDM_QUIT, "&�����");

	AppendMenu(hMenubar, MF_POPUP, (UINT_PTR) hMenu, "&������������");

	SetMenu(child112, hMenubar);



	ShowWindow(child112, SW_NORMAL);
	UpdateWindow(child112);

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}
#endif
#ifdef		TK2_DISTANCE_WND



/* ����� ����� � Windows-���������, ��������� ��������� ���� */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow){
	/* ³�������� ���������� ���� ��� ���� */
	fp_tkLogFile=f_tk2log_Init();
	/* ����� ��������� ���� */
	hinst_tk2 =hInstance;
	WNDCLASSEX wc={0};
	MSG Msg;
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.style		 = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc	 = WndProcTK112;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInstance;
	wc.hIcon		 =
//			LoadIcon(NULL, IDI_APPLICATION);
	CreateIcon(
				hInstance,       // application instance
				32,              // icon width
				32,              // icon height
				1,               // number of XOR planes
				1,               // number of bits per pixel
				ANDmaskTK_Icon, // AND bitmask
				XORmaskTK_Icon  // XOR bitmask
				);
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = g_szClassNameTK112;

	if(!RegisterClassEx(&wc)){
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	/* ����������� :) */
//	int width      = GetSystemMetrics(SM_CXSCREEN);
//	screen_heigh   = GetSystemMetrics(SM_CYSCREEN);

	hWndTk2 = CreateWindowEx (
    		0,
			g_szClassNameTK112,
    		TEXT ("��2.����������� ���������"),
//            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN |WS_VSCROLL | WS_HSCROLL,
			(WS_OVERLAPPEDWINDOW&(~(WS_MAXIMIZEBOX|WS_MINIMIZEBOX)))|WS_BORDER|WS_VISIBLE,
//            CW_USEDEFAULT, CW_USEDEFAULT,
//            CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT, 1024, 800,

            NULL, NULL, hInstance, NULL
			);

	if(hWndTk2 == NULL){
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	/* ��������� ����� ����, �� ������������ ������������
	 * ������ ������������ ������ */
	f_Register_PasswordWndClass(hInstance);
	f_RegisterSWndClass(hInstance);

	f_RegisterTK112USWndClass(hInstance);
	f_RegisterTk2fsWndClass(hInstance);

	HMENU hMenubar = CreateMenu();
	HMENU hMenu = CreateMenu();

	AppendMenu(hMenu, MF_STRING, IDM_COMPORT_SETTING, "&������������ ���-�����");
	AppendMenu(hMenu, MF_STRING, IDM_USER_SETTINGS, "&������������ �����������");
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenu, MF_STRING, IDM_QUIT, "&�����");

	AppendMenu(hMenubar, MF_POPUP, (UINT_PTR) hMenu, "&������������");

	SetMenu(hWndTk2, hMenubar);
	SetMenu(hWndTk2, hMenu);
	ShowWindow(hWndTk2, SW_NORMAL);
//	ShowWindow(hwndm, nCmdShow);
	UpdateWindow(hWndTk2);

	while(GetMessage(&Msg, NULL, 0, 0) > 0){
					TranslateMessage(&Msg);
					DispatchMessage(&Msg);
	}
	return Msg.wParam;
}
#endif		//TK2_DISTANCE_WND

LRESULT CALLBACK WndProcTK112(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	modbus_master_tx_msg_t mbTxMsg={0};
	switch(msg){

		/**************************************************************************************************************
		 *      CREATE 																						     CREATE
		 **************************************************************************************************************/
		case WM_CREATE:{
		  /* ����� ��� ��������� ������� */
		  CreateWindowW(L"Button", L"",
                WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				x_centr_panel, 10,  x_centr_width, 40, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
		  /* ���������� ����� ��� ��������� */
          CreateWindowW(L"Button", L"",
                WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				x_centr_panel, 60,  x_centr_width, 640, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
          //http://zetcode.com/gui/winapi/controlsIII/

          /* ˳�� ����� ��� ������ ���������  */
	   	  CreateWindowW(L"Button", L"", WS_CHILD /*| WS_VISIBLE */| BS_GROUPBOX| WS_GROUP,
	   			 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 290, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

	   	  hWndViewBtnMotorState= CreateWindow("button", "������ ��" , WS_CHILD |
	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
	   			 x_left_panel, y_LeftRadio, x_left_right_width_panel , 30, hwnd,(HMENU) IDB_SHOW_MOTOR_OPERATING, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

	   	  /* ˳��� ��������� ������ ���� */
	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

		  hBtnViewCX= CreateWindow("button", "����� ���" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +50, x_left_right_width_panel , 30, hwnd,(HMENU) IDB_SHOW_CX, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

          /* ����� ����� ����� ��� ������ ���������  */
	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE |*/ BS_GROUPBOX| WS_GROUP,
	   			x_right_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 290, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);


//			/* ���������� �������-��� ��� ��������� �������������� */
			 hWndProgressBar_ThermalModel = CreateWindowEx(
			 				             0,
			 				             PROGRESS_CLASS,
			 				             (LPSTR)NULL,
			 				             WS_VISIBLE | WS_CHILD /*| PBS_VERTICAL*/,
			 				             x_left_panel,
			 							 660,
			 				             150,
			 				             x_left_panel,
			 				             hwnd,
			 				             (HMENU)IDPB_PROGRESS_BAR_ThermalModelBehavior,
			 				             (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
			 				             NULL
			 							 );
			 if (!hWndProgressBar_ThermalModel){
			 		MessageBox(NULL, "Progress Bar Faileq.", "Error", MB_OK | MB_ICONERROR);
			 }
			 /* ����� 120% - ������ � ��������� */
			 SendMessage(hWndProgressBar_ThermalModel, PBM_SETRANGE, 0, MAKELPARAM(
			 		0, //minimum range value
			 		120  //maximum range value
			 		));

			 SendMessage(hWndProgressBar_ThermalModel, PBM_SETPOS,
			 		0, //Signed integer that becomes the new position.
			 		0  //Must be zero
			 		);


	          /* ����� ����� ��� ������ ���������  */
		   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE| */BS_GROUPBOX| WS_GROUP,
		   			x_right_panel, 60, 150, 640, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);


	          /* EditBox ��� ������-������ */
	          char addr_s[3]={0};
	          hEdit_TK2_ADDR = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", addr_s,
					WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
					, x_right_panel, 80, 40, 25,
					hwnd, (HMENU)ID_TK2_EDIT_ADDRESS , GetModuleHandle(NULL), NULL);
			  SendMessage(hEdit_TK2_ADDR, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

			  snprintf (addr_s,20,"%X",tk2_Addr);
			  SetWindowText(hEdit_TK2_ADDR,(LPCTSTR)addr_s);

	          CreateWindow("BUTTON", "���� ����.���.",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_right_panel, 105, 150, 20, hwnd, (HMENU)IDB_TEST_DISTANCE_CTRL,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
	          CreateWindow("BUTTON", "����� ����.���.",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_right_panel, 130, 150, 20, hwnd, (HMENU)IDB_START_DISTANCE_CTRL,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
	          CreateWindow("BUTTON", "����  ����.���.",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_right_panel, 155, 150, 20, hwnd, (HMENU)IDB_STOP_DISTANCE_CTRL,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);

	          CreateWindow("BUTTON", "����.�������",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_right_panel+30, 205, 150-30, 20, hwnd, (HMENU)IDB_RESET_MOTOR_PROTECTION_1,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
	          CreateWindow("BUTTON", "����.������� 2",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_right_panel, 230, 150, 20, hwnd, (HMENU)IDB_RESET_MOTOR_PROTECTION_2,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);


	          CreateWindow("BUTTON", "�����",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_right_panel, 300, 150, 20, hwnd, (HMENU)IDB_TK2DIST_EXIT,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);


			  /* ���������� �������-��� ��� ����� ���������� */
			  hWndProgressBar_Queue = CreateWindowEx(
				 	0,
				 	PROGRESS_CLASS,
				 	(LPSTR)NULL,
				 	WS_VISIBLE | WS_CHILD /*| PBS_VERTICAL*/,
					x_right_panel, 680, 150, 20,
					//910, 620, 150, 640
				 	hwnd,
				 	(HMENU)IDPB_QUEUE_MSG ,
				 	(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
				 	NULL
				 	);
			  if (!hWndProgressBar_Queue){
				 	MessageBox(NULL, "Progress Bar Faileq.", "Error", MB_OK | MB_ICONERROR);
			  }
			  /* ����� 0x10000 */
			  SendMessage(hWndProgressBar_Queue, PBM_SETRANGE, 0, MAKELPARAM(
							  0,       //minimum range value
							  10000  //maximum range value
							  )
			  );
			  SendMessage(hWndProgressBar_Queue, PBM_SETPOS,
				 			  0, //Signed integer that becomes the new position.
				 			  0  //Must be zero
			  );
			  /* ������ ��� ������ � ������-����� ����� ���������� */
			  p10Timer=SetTimer(
						hwnd,                // handle to main window
						IDT_TIMER_TK2_10MS,  // timer identifier
						TIMER_MAIN_MS,                  // msecond interval
					    (TIMERPROC) NULL     // no timer callback
						);

			  p50Timer=SetTimer(
						hwnd,                // handle to main window
						IDT_TIMER_TK2_50MS,  // timer identifier
						TIMER_MAIN_MS,                  // msecond interval
					    (TIMERPROC) NULL     // no timer callback
						);

			  f_InitTK2_Drow();

			  _beginthread((void*)f_desktop_tk2_session, 0, NULL);
		}
		break;
		/***************************************************************************************************************************************
	    *      PAINT 																						      						PAINT
		***************************************************************************************************************************************/
		case WM_PAINT: {// ���� ����� ����������, ��:

			HDC hDC; // ������ ���������� ���������� ������ �� ������
    	    RECT rect; // ���-��, ������������ ������ ���������� �������
	    	PAINTSTRUCT ps; // ���������, ���-��� ���������� � ���������� ������� (�������, ���� � ��)

	    	hDC = BeginPaint(hwnd, &ps); 	// �������������� �������� ����������
	    	GetClientRect(hwnd, &rect);  	// �������� ������ � ������ ������� ��� ���������

	    	RECT rectL =    {rect.left+210,rect.top+25, rect.left+1000,rect.top+50};
	    	DrawText(hDC, "������ ��������� ��������������        ", 40, &rectL, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	//����������� ������� ������ ������ ����
	    	RECT rectblue = {rect.left+450,rect.top+70,rect.left+819,rect.top+700};
	    	FillRect(hDC, &rectblue, (HBRUSH)29);

	    	RECT rect0 = {rect.left+210, rect.top+yMode,rect.left+1000,rect.top+yMode+25};
	    	DrawText(hDC, "����� ������ ", 14, &rect0, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	RECT rectmf = {rect.left+210, rect.top+y_motorfault,rect.left+1000,rect.top+y_motorfault+25};
	    	DrawText(hDC, "������ �������������� ", 23, &rectmf, DT_SINGLELINE|DT_LEFT|DT_TOP);

			RECT rectsz = {rect.left+210, rect.top+ySelfStart,rect.left+1000,rect.top+ySelfStart+25};
			DrawText(hDC, "����������", 11, &rectsz, DT_SINGLELINE|DT_LEFT|DT_TOP);

//	    	/* ����� ���� � */
	    	RECT rectfa = {rect.left+210, rect.top+y_aAmperage,rect.left+1000,rect.top+y_aAmperage+25};
    		DrawText(hDC, "�����, ���� �", 14, &rectfa, DT_SINGLELINE|DT_LEFT|DT_TOP);

//
//	    	/* ����� ���� B */
    		RECT rectfb = {rect.left+210, rect.top+y_bAmperage,rect.left+1000,rect.top+y_bAmperage+25};
    		DrawText(hDC, "�����, ���� �", 14, &rectfb, DT_SINGLELINE|DT_LEFT|DT_TOP);

//
//	    	/* ����� ���� C */
    		RECT rectfc = {rect.left+210, rect.top+y_cAmperage,rect.left+1000,rect.top+y_cAmperage+25};
    		DrawText(hDC, "�����, ���� C", 14, &rectfc, DT_SINGLELINE|DT_LEFT|DT_TOP);

//	    	/* ������г� ������  */
    		RECT rectab = {rect.left+210, rect.top+y_AmperageUnbalance,rect.left+1000,rect.top+y_AmperageUnbalance+25};
    		DrawText(hDC, "�������� ������ ", 18, &rectab, DT_SINGLELINE|DT_LEFT|DT_TOP);
//
	    	if ((q.d_regime==remote_automatic_mode)||(q.d_regime==automatic_mode)){
	    		/* ������ �� ����� ��������i� ������������� ������*/
	    	  	RECT rectpm1 = {rect.left+210, rect.top+yPumpingMode,rect.left+1000,rect.top+yPumpingMode+25};
	    		DrawText(hDC, "��� ������", 11, &rectpm1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  	RECT rectra = {rect.left+210, rect.top+y_AutoCtrl,rect.left+1000,rect.top+y_AutoCtrl+25};
	    		DrawText(hDC, "������ � ������������� �����", 30, &rectra, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    		RECT rectdm = {rect.left+210, rect.top+y_DryModeADC,rect.left+1000,rect.top+y_DryModeADC+25};
	    		DrawText(hDC, "����� ���,     ���", 19, &rectdm, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    		RECT rectll = {rect.left+210, rect.top+y_LowLevelADC,rect.left+1000,rect.top+y_LowLevelADC+25};
	    		DrawText(hDC, "����� �����, ���", 19, &rectll, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    		RECT recthl = {rect.left+210, rect.top+y_HighLevelADC,rect.left+1000,rect.top+y_HighLevelADC+25};
	    		DrawText(hDC, "������ �����,���", 19, &recthl, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	}/* K����� ��� ������������ ������*/
	    	////


			//��� ��������, ���
    		RECT rectir = {rect.left+210, rect.top+y_InsulationResistance,rect.left+1000,rect.top+y_InsulationResistance+25};
			DrawText(hDC, "��� �������� ", 15, &rectir, DT_SINGLELINE|DT_LEFT|DT_TOP);

//	    	/* ��������������� ���������� ���� ����� - ��������� �������������� */
	    	RECT rectblueHeating = {rect.left+20,rect.top+640,rect.left+170,rect.top+660};
	    	DrawText(hDC, "���������", 11, &rectblueHeating, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	RECT rectblueHeating1 = {rect.left+20+112,rect.top+640,rect.left+170,rect.top+660};
	    	FillRect(hDC, &rectblueHeating1, (HBRUSH)26);

	    	RECT rectblueHeating2 = {rect.left+20+112,rect.top+640,rect.left+20+115,rect.top+700};
	    	FillRect(hDC, &rectblueHeating2, (HBRUSH)24);
	    	//RECT rect_testbrush ={rect.left+20,rect.top+660,rect.left+20+116,rect.top+680};

	    	RECT rectblueHeating3 = {rect.left+20+115,rect.top+680,rect.left+190,rect.top+700};
	    	DrawText(hDC, ">100%", 6, &rectblueHeating3, DT_SINGLELINE|DT_LEFT|DT_TOP);

			/* ����� ��� ������ ������ ������  */

	    	RECT rect_EditAddress = {rect.left+x_right_panel+50, rect.top+75, rect.left+x_right_panel+50+60, rect.top+75+25};
	    	DrawText(hDC, "������", 7, &rect_EditAddress, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	RECT rect_EditAddress1 = {rect.left+x_right_panel+50, rect.top+90, rect.left+x_right_panel+50+60, rect.top+90+25};
	    	DrawText(hDC, "������", 8, &rect_EditAddress1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	/* ��������������� ���������� ������ ����� - ��������� �������������� */
	    	//910, 680, 150, 20,
	    	RECT recttr = {rect.left+x_right_panel, rect.top+535, rect.left+x_right_panel+150, rect.top+660+20};
	    	DrawText(hDC, "������:", 7, &recttr, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	RECT rectt9 = {rect.left+x_right_panel, rect.top+560, rect.left+x_right_panel+150, rect.top+660+20};
	    	DrawText(hDC, "����������:", 12, &rectt9, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	RECT recttg = {rect.left+x_right_panel, rect.top+610, rect.left+x_right_panel+150, rect.top+660+20};
	    	DrawText(hDC, "����:", 7, &recttg, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	RECT recttj = {rect.left+x_right_panel, rect.top+635, rect.left+x_right_panel+150, rect.top+660+20};
	    	DrawText(hDC, "���������:", 12, &recttj, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	RECT rectty = {rect.left+x_right_panel, rect.top+660, rect.left+x_right_panel+150, rect.top+660+20};
	    	DrawText(hDC, "�����:", 7, &rectty, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	EndPaint(hwnd, &ps);

		}//end WM_PAINT
		break;
		/**************************************************************************************************************
	    *      SIZE 																						      SIZE
		**************************************************************************************************************/
		case WM_SIZE:{
			RECT rcClient;

			GetClientRect(hwnd, &rcClient);
			hEdit = GetDlgItem(hwnd, IDC_MAIN_EDIT);
			SetWindowPos(hEdit,        NULL, 0,                 800+100, rcClient.right-110,  25, SWP_NOZORDER);
			SetWindowPos(hButtonSend,  NULL, rcClient.right-110,800+100, 100,                 25, SWP_NOZORDER);

//			SetWindowPos(hButtonSTART, NULL, 10,					 75,  110,                 25, SWP_NOZORDER);
//			SetWindowPos(hButtonSTOP,  NULL, 10,					200,  110,                 25, SWP_NOZORDER);

		//	https://studfiles.net/preview/1410070/page:13/
		}

		break;
		/*************************************************************************************************************************************
	    *      COMMAND 																						    		COMMAND
		**************************************************************************************************************************************/
		/* ������ֲ� */
		case WM_COMMAND:{
			char wmc[256]={0};
			switch(wParam) {

			case IDB_BtnSTART:{
				 if(tk2Process==ProcessBusy){
				   if(tk2btn_access==ENABLE){
					 tk2btn_access=DISABLE;
					 /* ������� ����������� ��� ������ ������� */
					 f_Set112TxWriteReg(RG_W_TK2_MOTOR_STARTSTOP,RG_V_TK2_MOTOR_START,&mbTxMsg);
					 /* ������� ����������� � ����� �� �������   */
					  f_set_tkqueue(
							&tk2Queue,
							ENABLE,
							hwnd,
							RG_W_TK2_MOTOR_STARTSTOP,
							&mbTxMsg,
							600
							);
					 snprintf(tk2LogStr,511,"��: ���� ��");
				   }else{snprintf (wmc,255," �� ���������� ��� ����� �� ������! ");}
				 }else{snprintf (wmc,255," ������ ������������� ��������� �� �� �������� ");}
			}
			break;
			case IDB_BtnSTOP:{
				 if(tk2Process==ProcessBusy){
				  if(tk2btn_access==ENABLE){
					 tk2btn_access=DISABLE;
					 /* ������� ����������� ��� ������ ������� */
					 f_Set112TxWriteReg(RG_W_TK2_MOTOR_STARTSTOP,RG_V_TK2_MOTOR_STOP,&mbTxMsg);
					 /* ������� ����������� � ����� �� �������   */
					 f_set_tkqueue(
							&tk2Queue,
							ENABLE,
							hwnd,
							IDB_Btn_MANUALE_DISTANCE_MODE,
							&mbTxMsg,
							600
							);

					 snprintf(tk2LogStr,511,"��: ���� ��");
				  }else{snprintf (wmc,255," �� ���������� ��� ����� �� ������! ");}
				 }else{snprintf (wmc,255," ������ ������������� ��������� �� �� �������� ");}

			}
			break;
			case IDB_BtnOnAUTO:{
				 if(tk2Process==ProcessBusy){
				  if(tk2btn_access==ENABLE){
					 tk2btn_access=DISABLE;

				  /* ������� ����������� ��� ������ ������� */
				  f_Set112TxWriteReg(RG_W_TK2_AUTO_STARTSTOP,RG_V_TK2_AUTO_START,&mbTxMsg);
				  /* ������� ����������� � ����� �� �������   */
				  f_set_tkqueue(
						    &tk2Queue,
							ENABLE,
							hwnd,
							RG_W_TK2_AUTO_STARTSTOP,
							&mbTxMsg,
							600
							);
				  snprintf(tk2LogStr,511,"��: ����� ������ � ���.���.");
				 }else{snprintf (wmc,255," �� ���������� ��� ����� �� ������! ");}
				 }else{snprintf (wmc,255," ������ ������������� ��������� �� �� �������� ");}
			}
			break;
			case IDB_BtnOffAUTO:{
				 if(tk2Process==ProcessBusy){
				  if(tk2btn_access==ENABLE){
					 tk2btn_access=DISABLE;
				  /* ������� ����������� ��� ������ ������� */
				  f_Set112TxWriteReg(RG_W_TK2_AUTO_STARTSTOP,RG_V_TK2_AUTO_STOP,&mbTxMsg);
				  /* ������� ����������� � ����� �� �������   */
				  f_set_tkqueue(
						    &tk2Queue,
							ENABLE,
							hwnd,
							IDB_Btn_MANUALE_DISTANCE_MODE,
							&mbTxMsg,
							600
							);
				  }else{snprintf (wmc,255," �� ���������� ��� ����� �� ������! ");}
				 }else{snprintf (wmc,255," ������ ������������� ��������� �� �� �������� ");}

			}
			break;
			case IDB_StartWhenAuto:{
				 if(tk2Process==ProcessBusy){
				  if(tk2btn_access==ENABLE){
					 tk2btn_access=DISABLE;
				  /* ������� ����������� ��� ������ ������� */
				  f_Set112TxWriteReg(RG_W_TK2_MOTOR_STARTSTOP,RG_V_TK2_MOTOR_START,&mbTxMsg);
				  /* ������� ����������� � ����� �� �������   */
				  f_set_tkqueue(
						    &tk2Queue,
							ENABLE,
							hwnd,
							RG_W_TK2_MOTOR_STARTSTOP,
							&mbTxMsg,
							600
							);
				  snprintf(tk2LogStr,511,"��: ������");
				  }else{snprintf (wmc,255," �� ���������� ��� ����� �� ������! ");}
				 }else{snprintf (wmc,255," ������ ������������� ��������� �� �� �������� ");}
			}
			break;
			case IDB_Btn_MANUALE_DISTANCE_MODE:{
				if(tk2Process==ProcessBusy){
				 if(tk2btn_access==ENABLE){
					 tk2btn_access=DISABLE;

				  /* ������� ����������� ��� ������ ������� */
				  f_Set112TxWriteReg(RG_W_TK2_MODE,0x0003,&mbTxMsg);
				  /* ������� ����������� � ����� �� �������   */
				  f_set_tkqueue(
						    &tk2Queue,
							ENABLE,
							hwnd,
							IDB_Btn_MANUALE_DISTANCE_MODE,
							&mbTxMsg,
							600
							);
				  snprintf(tk2LogStr,511,"��: ������� � ����� ������.");
				 }else{snprintf (wmc,255," �� ���������� ��� ����� �� ������! ");}
				}else{snprintf (wmc,255," ������ ������������� ��������� �� �� �������� ");}
			}
			break;
			case IDB_Btn_AUTOMATICAL_DIST_MODE:{
				if(tk2Process==ProcessBusy){
				 if(tk2btn_access==ENABLE){
					tk2btn_access=DISABLE;
				  /* ������� ����������� ��� ������ ������� */
				  f_Set112TxWriteReg(RG_W_TK2_MODE,0x0004,&mbTxMsg);
				  /* ������� ����������� � ����� �� �������   */
				  f_set_tkqueue(
						    &tk2Queue,
							ENABLE,
							hwnd,
							IDB_Btn_MANUALE_DISTANCE_MODE,
							&mbTxMsg,
							600
							);
				  snprintf(tk2LogStr,511,"��: ������� � ����� ������������");
				 }else{snprintf (wmc,255," �� ���������� ��� ����� �� ������! ");}
				}else{snprintf (wmc,255," ������ ������������� ��������� �� �� �������� ");}
			}
			break;
			case IDM_USER_SETTINGS:{
				if(tk2Process==ProcessBusy){
				/* ����������� ��������� ������ ������� ���� �����������
				 * ���� ���� ����������� ��� ������� - ����� �� ������*/
				   if(!IsWindow(hWndTk2usCh)){
					   /* ����� �� ������� � ����� ����������� -
					   * �������� ��������� ���������� �������� ��������� */
							/* ��������� ������ ����� ������������� DeskTop <-> TK2 */
//							KillTimer(hwnd, p1000Timer);
							/* ������� ����� ������ */
//							f_clear_tkqueue(&tk2Queue);
					    /* ��������� ���� ����������� ����������� */
					    hWndTk2usCh=f_CreateTK112usWnd(hwnd);
//						snprintf(tk2LogStr,511,"��: ���. ������������ �����������");
				   	}//if(!IsWindow(usWndChild)){
				}
				else{
					snprintf (wmc,255," ������ ������������� ��������� �� �� �������� ");
				}
				}//case IDM_USER_SETTINGS:{
				break;
			case IDB_TK2DIST_EXIT:{
				fprintf(fp_tkLogFile,"\n\n\n\n");
				fclose(fp_tkLogFile);
				 if(!IsWindow(hWndTk2fsCh)){DestroyWindow(hWndTk2fsCh);}
				 if(!IsWindow(hWndTk2usCh)){DestroyWindow(hWndTk2usCh);}
				DestroyWindow(hwnd);
			}
			break;
			case IDM_QUIT:{
				fprintf(fp_tkLogFile,"\n\n\n\n");
				fclose(fp_tkLogFile);
				 if(!IsWindow(hWndTk2fsCh)){DestroyWindow(hWndTk2fsCh);}
				 if(!IsWindow(hWndTk2usCh)){DestroyWindow(hWndTk2usCh);}
				DestroyWindow(hwnd);
			}
			break;
			case IDM_COMPORT_SETTING:
				if(!IsWindow(comWndChild)){
					f_CreateSWnd(hwnd);
					snprintf(tk2LogStr,511,"��: ������������ ���-�����");
				}
				break;
			case IDB_TEST_DISTANCE_CTRL:{
				  /* ������� ������ ����� ������������� DeskTop <-> TK2 */
				  /* ������� ������� �����������*/
				tk2_Addr= f_GetModbussAddress(ID_TK2_EDIT_ADDRESS);

//				mb17.msg[0]=tk2_Addr;
//				mb17.msg[1]=0x11;
//				mb17.length=2;
				f_Set112TxReadReg(0x0208,&mbTxMsg);
				/* ������� ����� */
				f_clear_tkqueue(&tk2Queue);
				/* ������ ������� ����������� � ����� ����� */
				f_set_tkqueue(
						&tk2Queue,
						ENABLE,
						hwnd,
						IDB_TEST_DISTANCE_CTRL,
						&mbTxMsg,
						600
				);
				/* ����������� ������ ��� ������� ����������� ��2 ����������� */
				 RW_case=IDB_TEST_DISTANCE_CTRL;
				 snprintf(tk2LogStr,511,"��: ���������� ������������� ��������� ");
			}
			break;
			case IDB_START_DISTANCE_CTRL:{
				  /* ��������� ������ ����� ������������� DeskTop <-> TK2 */
				  if(tk2Process==ProcessBusy){
					  /* ������� ����� */
					  f_clear_tkqueue(&tk2Queue);
				  }
				  else if(tk2Process==ProcessIddle){
			      /* ����������� ������ ���������� ������������� ��������� */
					  tk2Process=ProcessBusy;
					  /* ��������� ������, �� ��������� ��������� � ���������� � ����� ����������� */
					  p1000Timer=SetTimer(
						hwnd,                // handle to main window
					    IDT_TIMER_TK2_1000MS,// timer identifier
						TIMER_PERIOD_TK2_1000_MS,                // msecond interval
					    (TIMERPROC) NULL     // no timer callback
						);

				  /* ��� ����� �������-���� ������ ����� �� ���������� ����� */
					/* ����� ������/�������/������� */
					modbus_master_tx_msg_t mbTxMsg;
					memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
					/* ����� ��kc.������ */
					f_Set112TxReadReg(RG_R_TK2_MAX_AMPERAGE,&mbTxMsg);
					  f_set_tkqueue(
						    &tk2Queue,
							DISABLE,
							hwnd,
							ID_READ_US_MAX_AMPERAGE,
							&mbTxMsg,
							600
							);
					snprintf(tk2LogStr,511,"��: ����� ������������� ��������� ");
				  }
				  else{}
				  /* ��������� ����������� �������� ��������� ������ ������ */
				  f_TxRxBusyDegreeCalcStart();


			}
			break;
			case IDB_STOP_DISTANCE_CTRL:{
				if(tk2Process==ProcessBusy){
					/* ��������� ������ ����� ������������� DeskTop <-> TK2 */
					KillTimer(hwnd, p1000Timer);
					f_clear_tkqueue(&tk2Queue);
					snprintf(tk2LogStr,511,"��: ���� ������������� ���������\n ");

				tk2Process=ProcessIddle;
				char r[256]={0};
				/* ��������� ����������� �������� ��������� ������ ������ */
				f_TxRxBusyDegreeCalcStop(r, sizeof(r));
				snprintf(tk2LogStr,511,"��: ���� ������������� ���������");
				strcat(tk2LogStr,r);
				f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));
				snprintf(tk2LogStr,511,"���������� ������ �� ��� ������������� ���������");
				f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));
				snprintf(tk2LogStr,511,"%d.%2d %% - ������� ��������� ������ ",
						(int)(gTxRxBusyAveDegreef*100.0f)/100,
						(int)(gTxRxBusyAveDegreef*100.0f)%100);
				f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));

				snprintf(tk2LogStr,511,"%d - ʳ������ ��������� ����������",
						(int)gTxRxSessionStartCntr);
				f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));

				snprintf(tk2LogStr,511,"%d.%2d %% - C��� ����������� ��� �������",
						(int)(gTxRxSessionAVESuccessDegreef*100.0f)/100,
						(int)(gTxRxSessionAVESuccessDegreef*100.0f)%100);
				f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));

				}

			}
			break;
			case IDB_RESET_MOTOR_PROTECTION_1:{
				if(tk2Process==ProcessBusy){
				 if(tk2btn_access==ENABLE){
					 tk2btn_access=DISABLE;
					/* ������� ����������� ��� ������ ������� */
					f_Set112TxWriteReg(RG_W_RESET_MOTOR_PROTECTION_1,RG_V_RESET_MOTOR_PROTECTION,&mbTxMsg);
					/* ������� ����������� � ����� �� �������   */
					f_set_tkqueue(
						&tk2Queue,
						ENABLE,
						hwnd,
						IDB_RESET_MOTOR_PROTECTION_1,
						&mbTxMsg,
						600
						);
					snprintf(tk2LogStr,511,"��: �������� ����  ");
				 }else{snprintf (wmc,255," �� ���������� ��� ����� �� ������! ");}
				}else{snprintf (wmc,255," ������ ������������� ��������� �� �� �������� ");}
		    }
		    break;
			case IDB_RESET_MOTOR_PROTECTION_2:{
				if(tk2Process==ProcessBusy){
				 if(tk2btn_access==ENABLE){
					 tk2btn_access=DISABLE;
					/* ������� ����������� ��� ������ ������� */
					f_Set112TxWriteReg(RG_W_RESET_MOTOR_PROTECTION_2,RG_V_RESET_MOTOR_PROTECTION,&mbTxMsg);
					/* ������� ����������� � ����� �� �������   */
					f_set_tkqueue(
						&tk2Queue,
						ENABLE,
						hwnd,
						IDB_RESET_MOTOR_PROTECTION_2,
						&mbTxMsg,
						600
						);
					snprintf(tk2LogStr,511,"��: �������� �������� �������� ����  ");
				 }else{snprintf (wmc,255," �� ���������� ��� ����� �� ������! ");}
				}else{snprintf (wmc,255," ������ ������������� ��������� �� �� �������� ");}
		    }
		    break;
		    /***************************************************************************/
			/* ������������ ���������� ������������ ������ -����������               */
		    /***************************************************************************/
			case IDB_SHOW_MOTOR_OPERATING:
				  /* ������ �� ���������� ����������, ��� ���� ���������, ������������*/
		    	  switch(q.d_MotorState){
		    	  case oFF_:SendMessage(hWndViewBtnMotorState, BM_SETCHECK, 0, 0L);break;
		    	  case oN_: SendMessage(hWndViewBtnMotorState, BM_SETCHECK, 1, 0L);break;
		    	  default:{}
		    	  }
			break;
			case IDB_SHOW_EMP_AUTOCTR1:
				 /* ������ �� ���������� ����������, ��� ���� ���������, ������������*/
		    	  switch(q.d_AutoCtrl){
		    	  case oFF_:SendMessage(hBtnViewAutoctrl, BM_SETCHECK, 0, 0L);break;
		    	  case oN_: SendMessage(hBtnViewAutoctrl, BM_SETCHECK, 1, 0L);break;
		    	  default:{}
		    	  }
		   break;
	  	   case IDB_SHOW_CX:
	  		    /* ������ �� ���������� ����������, ��� ���� ���������, ������������*/
	  	    	  switch(q.d_CX){
	  	    	  case oFF_:SendMessage(hBtnViewCX, BM_SETCHECK, 0, 0L);break;
	  	    	  case oN_ : SendMessage(hBtnViewCX, BM_SETCHECK, 1, 0L);break;
	  	    	  default:{}
	  	    	  }
	  	   break;
		   default:{}
		   }/* end switch param*/
			/* Error Message Box */
		   if(strlen(wmc) != 0){ MessageBox(hwnd, wmc, TEXT("����� ! "), MB_ICONERROR | MB_OK);}
		}/*WM_COMMAND:*/
		break;
		case WM_CLOSE:{
			 fprintf(fp_tkLogFile,"\n\n\n\n");
			 fclose(fp_tkLogFile);
			 if(!IsWindow(hWndTk2fsCh)){DestroyWindow(hWndTk2fsCh);}
			 if(!IsWindow(hWndTk2usCh)){DestroyWindow(hWndTk2usCh);}
			 DestroyWindow(hwnd);
		}

		break;
		case WM_DESTROY:
			PostQuitMessage(0);
		break;
		/*************************************************************************************************************************************
	    *      TIMERS 																						   TIMERS 					TIMERS
		**************************************************************************************************************************************/
		case WM_TIMER:{
			//The number of milliseconds that have elapsed since the system was started. This is the value returned by the GetTickCount function.
			  switch ((UINT)wParam){
			  case IDT_TIMER_TK2_1000MS:{
				  tk2TimerCntr++;
				  /* ���������� ������ ������ */
				  memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
				  if ((tk2TimerCntr % 5)==0){
					  /* ����� ����� ����� */
					  RW_case=ID_TK2_POOL;
					  tk2_Addr= f_GetModbussAddress(ID_TK2_EDIT_ADDRESS);
					  if(tk2_Addr<0){DestroyWindow(hwnd);}
					  mbTxMsg.msg[0]=tk2_Addr;
					  mbTxMsg.msg[1]=0x03;
					  mbTxMsg.msg[2]=0x01;
					  mbTxMsg.msg[3]=0x00;
					  mbTxMsg.msg[4]=0x00;
					  mbTxMsg.msg[5]=0x10;
					  mbTxMsg.length=6;
					  /* Գ����� ������ */
					  f_set_tkqueue(
							    &tk2Queue,
									DISABLE,
									hwnd,
									ID_TK2_POOL,
									&mbTxMsg,
									3000
									);
#ifdef TK2_THERMAL_MODEL_PROGRES_BAR_DEBUG
							q.d_aAmperage=Itk2_Max.floatf*1.1f;
						    q.d_bAmperage=Itk2_Max.floatf;
						    q.d_cAmperage=Itk2_Max.floatf*0.99f;

						    q.d_MotorState=oN_;
#endif
					  float amp =(q.d_aAmperage+q.d_bAmperage+q.d_cAmperage)/3.0f;
					  float HeatProcf =f_tk2Thermal_Motor_Model (
									amp,
									Itk2_Max.floatf,
									q.d_MotorState
									);

					  SendMessage(hWndProgressBar_ThermalModel, PBM_SETPOS,
							  	  	HeatProcf, //Signed integer that becomes the new position.
					  				0);  //Must be zero
//					  EnumChildWindows (
//							hWndTk2,						// ���������� ������������� ����
//							hWndProgressBar_ThermalModel,	// ��������� �� ������� ��������� ������
//							(int)(HeatProcf*100.0f)			// ��������, ������������ ����������
//					  );
					}
					else if ((tk2TimerCntr % 5)==1){
							/* ����� ��������� ������ ������ */
							f_Set112TxReadReg(RG_R_TK2_MODE,&mbTxMsg);
							f_set_tkqueue(
									&tk2Queue,
									DISABLE,
									hwnd,
									ID_TK2_POOL_MODE,
									&mbTxMsg,
									2000
									);
					}
					else if ((tk2TimerCntr % 5)==2){
							/* ����� ����������� */

							f_Set112TxReadReg(RG_R_TK2_SELFSTART,&mbTxMsg);
							f_set_tkqueue(
									&tk2Queue,
									DISABLE,
									hwnd,
									RG_R_TK2_SELFSTART,
									&mbTxMsg,
									2000
									);
					}
					else if ((tk2TimerCntr % 5)==3){
							/* ����� ������/�������/������� */
							f_Set112TxReadReg(RG_R_TK2_PUMPING_MODE,&mbTxMsg);
							f_set_tkqueue(
									&tk2Queue,
									DISABLE,
									hwnd,
									ID_TK2_POOL_PUMPING_MODE,
									&mbTxMsg,
									2000
									);
					}
					else if ((tk2TimerCntr % 5)==4){
						if((q.d_regime==remote_automatic_mode)||(q.d_regime==automatic_mode)){
							/* � ������������� ����� ����� �����/����� ������ �� �������� */
						f_Set112TxReadReg(0x021d,&mbTxMsg);
						f_set_tkqueue(
								&tk2Queue,
								DISABLE,
								hwnd,
								0x021d,
								&mbTxMsg,
								2000
								);
						}
						/* ����� ������� ������-������ */
						f_Set112TxReadReg(RG_R_TK2_MODBUS_ADDR,&mbTxMsg);
						f_set_tkqueue(
									&tk2Queue,
									DISABLE,
									hwnd,
									ID_TK2_POOL_MODBUSS_ADDRESS,
									&mbTxMsg,
									2000
									);
					}

					else{}
					uint16_t tk2Barrel=get_BarrrelPro();
//					EnumChildWindows (
//							hWndTk2,		// ���������� ������������� ����
//							hWndProgressBar_Barrel,	// ��������� �� ������� ��������� ������
//							tk2Barrel	// ��������, ������������ ����������
//						);
					SendMessage(hWndProgressBar_Barrel, PBM_SETPOS,
						tk2Barrel, //Signed integer that becomes the new position.
						0  		   //Must be zero
					);

			  }//IDT_TIMER_TK2_1000MS:{
			  break;
			  /* ���������� ������ ������ ���� ���������� ������������ ����� ������
			   * � ��������� �� ������� � �������� */
			  case IDT_TIMER_TK2_1333MS:{
				  /* ����������� ������ ��� ������� ����������� ��2 ����������� */
				  if( RW_case==IDB_TEST_DISTANCE_CTRL){
					  HDC hDCr; // ������ ���������� ���������� ������ �� ������
					  RECT rect; // ���-��, ������������ ������ ���������� �������
					  /*������� ������*/
					  hDCr= GetDC(hwnd);
					  GetClientRect(hwnd, &rect);  	// �������� ������ � ������ ������� ��� ���������
					  /* ������� ������� ����������� */
					  RECT rectht = {rect.left+839, rect.top+25,rect.left+910+140,rect.top+50};
//				  	  RECT rectht = {rect112.left+910, rect112.top+25,rect112.left+910+140,rect112.top+50};
					  FillRect(hDCr, &rectht, (HBRUSH)(COLOR_WINDOW+1));
					  ReleaseDC(hwnd, hDCr);
				 }
				  KillTimer(hwnd, p1333Timer);
			  }

			  break;
				  /* �� �� ������� ����������� �������, �� ��� ������� */
			  //case IDT_TIMER_TK2_1333MS:
				  /****/
			  case IDT_TIMER_TK2_2000MS:{

				  /* ���� � �������� �� ���� ������� - ���� ������..*/
				  /* ���� ������� ������ ��������� ���������� ������� �������� 10 ��� - ��������� � ����������� */
				  if(met_zero_MBsuccess == EVENT){
					  if(gTxRxSessionSuccessDegreef<10.0f){
						tk2ProcessState=0;
						/* ���� ����.��������� ��� ���� ��������, ������� ������,
						*  �� ����� ���������� ���������� �����������,
						*  ����� ��������� �������� ������ �� ��2   */
						if(tk2Process==ProcessBusy){
							KillTimer(hwnd, p1000Timer);
							f_clear_tkqueue(&tk2Queue);
							snprintf(tk2LogStr,511,"��2 �� �������\n "
								"����������� ��������� �������� ");

							tk2Process=ProcessIddle;
						}
						const int result = MessageBox(NULL, "��2 �� �������\n "
								"����������� ��������� ��������", "�����!",MB_ICONEXCLAMATION | MB_OK);
						switch (result){
						case IDOK:
							/* ³��������� ������ ����� */
							if(tk2Process==ProcessBusy){
								p1000Timer=SetTimer(
										hwnd,                // handle to main window
										IDT_TIMER_TK2_1000MS,  // timer identifier
										TIMER_PERIOD_TK2_1000_MS,                  // msecond interval
										(TIMERPROC) NULL     // no timer callback
								);
							}
						break;
						default:{}
						}

					 /* ���� �� �������� ����������� ��� ������� � ����-��������,
					  * ��� ���� ���������� ���� ����� � ����� ����   */
						memcpy(&tk2LogStr, "��2 �� �������", 256);
				   }// if (gTxRxSessionSuccessDegreef<10.0f){
				   met_zero_MBsuccess = NO_EVENT;
				  }
				  KillTimer(hwnd, p2000Timer);
			  }
			  break;
			  case IDT_TIMER_TK2_10MS:{
				  /* ���� � ��� ��������� ������ - ������� �������� �������� */
				  if((tk2Process==ProcessBusy)&&(tk2btn_access==DISABLE)){
					  tk2btnSleepCntr+=TIMER_MAIN_MS;
					  if(tk2btnSleepCntr>=TK2_BTN_BLOKING_MS){
						  tk2btn_access=ENABLE;
						  tk2btnSleepCntr=0;
					  }
				  }
			  }
			  break;
			  case IDT_TIMER_TK2_50MS:{
					f_tk2UpdateStat(
							hwnd,
							hWndProgressBar_Queue,
							p1000Timer,
							IDT_TIMER_TK2_1000MS
							);
			  }
			  break;
			  default:{}

		}  //switch ((UINT)wParam){
		break; //case WM_TIMER;
		}//case WM_TIMER:{
		case VK_PSW:
			break;
		case VK_UPDATE_WINDOW:
			/* �������� ����, ����������� �� ������� � ����, ��� ����������� ������ � ����� */
//			extern modbus_master_rx_msg_t mIn_rx_msg;
//			extern modbus_master_tx_msg_t mOut_tx_msg;
//			extern modbus_status_t RxMasterModbusStatus;
			f_tk2QPWD_RgAnswer(
					RxMasterModbusStatus,
					&mOut_tx_msg,
					&mIn_rx_msg,
					wParam
			);

			/* Windows-�����������, ���������� ������������, ����� ����
			 * � �������: f_desktop_tk_session(), ���� a3tk2TxRx.
			 * WPARAM wParam - RW_case, ������������ ���� ����������� ����������� */
			f_tk2UpdateWindow(hwnd);
		break;
		default:{
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}
	return 0;
}


/* �� ������� ����������� ����� ��� ��� �������� ����������� � VK_UPDATE_WINDOW
 * ��� ���������� ��� ������, ���������� � ��������� ����*/
static int f_tk2UpdateWindow(HWND hwnd){
//   /***********************************************************************************************************
//   * ������ � ������ ��²������� ������
//   ***********************************************************************************************************/
	f_tk2UpdateStat(
			hwnd,
			hWndProgressBar_Queue,
			p1000Timer,
			IDT_TIMER_TK2_1000MS
			);

	char pm[80]={0};
	/* ����� �������� ��� ��������� ������ � ����*/
	HDC hDCr; // ������ ���������� ���������� ������ �� ������
	RECT rect112; // ���-��, ������������ ������ ���������� �������
	/*������� ������*/
	hDCr= GetDC(hwnd);
	GetClientRect(hwnd, &rect112);  	// �������� ������ � ������ ������� ��� ���������
	/***********************************************************************************************************
	 * ���� ����� ��²�������� ��� ��������� ²���
	 ***********************************************************************************************************/
	if (tk2ProcessState==0){
		/* ���� �� ��� ����� ����� - ����� �� ������ */
		return 0;
	}

	/***********************************************************************************************************
	 * ���� ������� ��²��������
	 ***********************************************************************************************************/
    /* ��� ������� ������ ����������, �� �� ������������ �������,
     * ���� �� 10 ��� �������� �� �������� - �������� ����������� */
	if (gTxRxSessionSuccessDegreef<10.0f){
		  /* ��������� ������, ��� ������ ������� ����������� ����� ���� ������ */
		  if(met_zero_MBsuccess == NO_EVENT){
			  p2000Timer=SetTimer(
					  hwnd,                // handle to main window
					  IDT_TIMER_TK2_1333MS,// timer identifier
					  12000,        // msecond interval
					  (TIMERPROC) NULL     // no timer callback
			  );
			  met_zero_MBsuccess = EVENT;
		  }
	}
	if(tk2ProcessState==-1){
		/* �� �����������, ��� ����������� ��� �������������� ���-���� �������.*/
		/* ���� ������ ��� �����������, ��� ������ ��� ����� */
		/* ��� ������� ������ ����������� ������� ���������� ����� ������    */
		/* ���� �������� ����� ������ �������  - �������� ������� �����������
		 * �� ���������� ������������ ����� */
		if(strlen(user_msg112)!=0){
			tk2ProcessState=0;
//			{
				/* ���� ����.��������� ��� ���� ��������, ������� ������,
				 * ����� ��������� �������� ������ �� ��2   */
				if(tk2Process==ProcessBusy){KillTimer(hwnd, p1000Timer);}
				const int result = MessageBox(NULL, user_msg112, "�����!",MB_ICONEXCLAMATION | MB_OK);
				switch (result){
				case IDOK:
				/* ³��������� ������ ����� */
					if(tk2Process==ProcessBusy){
						p1000Timer=SetTimer(
							hwnd,                // handle to main window
							IDT_TIMER_TK2_1000MS,  // timer identifier
							TIMER_PERIOD_TK2_1000_MS,                  // msecond interval
							(TIMERPROC) NULL     // no timer callback
						);
					}
					break;
				default:{}
				}

				/* ���� �� �������� ����������� ��� ������� � ����-��������,
				 * ��� ���� ���������� ���� ����� � ����� ����   */
				memcpy(&tk2LogStr, &user_msg112, sizeof(tk2LogStr));
				/* ������� ����� ����������� ��� ������� �����������*/
				memset(&user_msg112,0,sizeof(user_msg112));
		}//if(strlen(user_msg112)!=0){

	}
	/***********************************************************************************************************
	 * ���� �� ���������� ������ � ��2
	 ***********************************************************************************************************/
	else if(tk2ProcessState==2){
		tk2ProcessState=0;
		/* ���� �� ���� ���������� - �������� ������� �����������*/
		RECT rectht = {rect112.left+839, rect112.top+25,rect112.left+839+140,rect112.top+50};
		DrawText(hDCr, user_msg112, 19, &rectht, DT_SINGLELINE|DT_LEFT|DT_TOP);
		memset(&user_msg112,0,sizeof(user_msg112));
		  /* ��������� ������, ��� ������ ������� ����������� ����� ���� ������ */
		  p1333Timer=SetTimer(
			hwnd,                // handle to main window
		    IDT_TIMER_TK2_1333MS,// timer identifier
			TIMER_AUX_MS,        // msecond interval
		    (TIMERPROC) NULL     // no timer callback
			);
	}
	/***********************************************************************************************************
	 * ���� �� ������ֲ��� �����˲���
	 ***********************************************************************************************************/
	else if(tk2ProcessState==1){
		tk2ProcessState=0;
		/* ���� �� ��� ���� �����  - �������� */
//		snprintf(tk2LogStr,80,"q.d_regime = %d", (int)q.d_regime);
//		f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));
		/* ���� �������� ���� ������ - ����� ������������  */
		if(q.d_regime != qOld.d_regime){
	     /* ���� ����� �̲����� �� � ������� �� ������������ - ������������� ������-���������� */
  	   	 if ((q.d_regime==remote_automatic_mode)||(q.d_regime==automatic_mode)){

  			 CreateWindowW(L"Button", L"", WS_CHILD /*| WS_VISIBLE*/| BS_GROUPBOX| WS_GROUP,
  				  x_left_panel, 410, 150, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
  			 hBtnViewAutoctrl= CreateWindow("button", "���.���" , WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON ,
  				 x_left_panel, y_LeftRadio+25, 150 , 30, hwnd, (HMENU) IDB_SHOW_EMP_AUTOCTR1, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

			 /* ���������� �������-��� ��� ����� */
			 hWndProgressBar_Barrel = CreateWindowEx(
					 				             0,
					 				             PROGRESS_CLASS,
					 				             (LPSTR)NULL,
					 				             WS_VISIBLE | WS_CHILD | PBS_VERTICAL,
					 				             x_left_panel,
												 y_PBBarrel_left,
					 				             40,
					 				             100,
					 				             hwnd,
					 				             (HMENU)IDPB_PROGRESS_BAR_BARREL,
					 				             (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
					 				             NULL
					 							 );
			 if (!hWndProgressBar_Barrel){
				 MessageBox(NULL, "Progress Bar Faileq.", "Error", MB_OK | MB_ICONERROR);
		     }
			 /* ����� 100% - ������ � ��������� */
			 SendMessage(hWndProgressBar_Barrel, PBM_SETRANGE, 0, MAKELPARAM(
				 0, 	//minimum range value
				 100    //maximum range value
			 ));

			 SendMessage(hWndProgressBar_Barrel, PBM_SETPOS,
				20, //Signed integer that becomes the new position.
				0   //Must be zero
			 );

  			 if(q.pressure_sensor==electrode){
//  			 CreateWindowW(L"Button", L"", WS_CHILD /*| WS_VISIBLE*/| BS_GROUPBOX| WS_GROUP,
//  			 	 x_left_panel, 410, 150, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
//  			 hBtnVU= CreateWindow("button", "����.�����" , WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON ,
//  			 	 x_left_panel, y_LeftRadio +75, 150 , 30, hwnd, (HMENU) IDB_SHOW_VU, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
  			 }
  			 if(   (q.pressure_sensor==electrode)
  				 ||(q.pressure_sensor==manometer_EKM)
  				 ||(q.pressure_sensor==pressure_relay)
  				 ||(q.pressure_sensor==float_sensor)
				 ){
  				 EnumChildWindows(hwnd /* parent hwnd*/, DestoryChildCallbackRA, 0);

  				 hABtnHighPanel=CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE |*/ BS_GROUPBOX,
  					x_left_panel, 60,  150, 330, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

  				 hABtnStart= CreateWindow("BUTTON", "���.���.",WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
  					x_left_panel, 90,  150, 30, hwnd, (HMENU)IDB_BtnOnAUTO,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);

  				 hABtnStop= CreateWindow("BUTTON", "����.���",WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
  					x_left_panel, 150, 150, 30, hwnd, (HMENU)IDB_BtnOffAUTO,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);

  				 hABtnBuster= CreateWindow("BUTTON", "������",WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
  					x_left_panel, 210, 150, 30, hwnd, (HMENU)IDB_BtnSTOP,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);

  				 /* ˳�� ����� ����� ��� ������ ���� ������������ ������  */
  				 hABtnLowPanel=CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE |*/ BS_GROUPBOX,
  					x_left_panel, 555,  150, 80, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);


  				 hABtnToR=CreateWindow("BUTTON", "����.����.",WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
					x_left_panel, 575,  150, 30, hwnd, (HMENU)IDB_Btn_MANUALE_DISTANCE_MODE,
  														   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
  				 /* log */
  				 snprintf(tk2LogStr,511,"����� ������ ������ �� ������������");
  			 }
  			 }/* K����� ��� ������������ ������*/
  	   	 	 /* ���� ����� ������� � ������������� �� ������ */
  	   	 	 else if((q.d_regime==manual_mode)||(q.d_regime==remote_manual_mode)){
      			 EnumChildWindows(hwnd , DestoryChildCallbackAR, 0);
//      			RECT rect_Mn = {x_left_panel,60,20+150,60+210};

      			hRBtnHighPanel=CreateWindowW(L"Button", L"",
		                  WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX,
		             x_left_panel, 60,  150, 210, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
      			hRBtnStart=CreateWindow("BUTTON", "����",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_left_panel, 90,  150, 30, hwnd, (HMENU)IDB_BtnSTART,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
      			hRBtnStop=CreateWindow("BUTTON", "����",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_left_panel, 150, 150, 30, hwnd, (HMENU)IDB_BtnSTOP,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
	           /* ˳�� ����� ����� ��� ������ ���� ������������ ������  */
      			hRBtnLowPanel=CreateWindowW(L"Button", L"",
	                   					       WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX,
	                 x_left_panel, 555,  150, 80, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

      			hRBtnToA=CreateWindow("BUTTON", "���.����.",
	                  				           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_left_panel, 575,  150, 40, hwnd, (HMENU)IDB_Btn_AUTOMATICAL_DIST_MODE,
	                  						   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);

		    }
		    else{}
  	   	    InvalidateRect(hwnd,NULL,TRUE);
  	     	UpdateWindow(hwnd);
  	     	f_tk2UpdateStat(
  	     			hwnd,
					hWndProgressBar_Queue,
					p1000Timer,
					IDT_TIMER_TK2_1000MS
  	  				);
  	        qOld.d_regime=q.d_regime;



   	    }
		/* �� ���� ������� �� ���� ������, � ������������� ������, � ���� � ������ ����
		 * ����� - ��������� �� ���� ���� ���� ������, �� �� ����: */
        if ((q.d_regime==remote_automatic_mode)||(q.d_regime==automatic_mode)){
          /* ������������ �����, �������� ��������� */
          	/* ������� ������ ��������� ������ ������������� ������  */
 		    switch(q.d_AutoCtrl){
 		    	  case oFF_:SendMessage(hBtnViewAutoctrl, BM_SETCHECK, 0, 0L);break;
 		    	  case oN_: SendMessage(hBtnViewAutoctrl, BM_SETCHECK, 1, 0L);break;
 		    	  default:{}
 		    }
          	snprintf (pm,50," %4d              				",q.d_HighLevelADC);
          	RECT recthl1 = {rect112.left+460, rect112.top+y_HighLevelADC,rect112.left+1000,rect112.top+y_HighLevelADC+25};
          	DrawText(hDCr, pm, 50, &recthl1, DT_SINGLELINE|DT_LEFT|DT_TOP);
          	/* в���� ������� ²� ������� ��������� в��� */
          	snprintf (pm,50," %4d              				",q.d_LowLevelADC);
          	RECT rectll1 = {rect112.left+460, rect112.top+y_LowLevelADC,rect112.left+1000,rect112.top+y_LowLevelADC+25};
          	DrawText(hDCr, pm, 50, &rectll1, DT_SINGLELINE|DT_LEFT|DT_TOP);

          	memset(&pm,0,sizeof(pm));
          	switch(q.tk2_PumpingMode){
   			                      //1234567890123456789012345678901234567890123
          	case 0:{snprintf(pm,40," ³������, ������� ����      (��)     ");}break;
          	case 1:{snprintf(pm,40," ������, ���                  (��)     ");}break;
          	case 2:{snprintf(pm,40," ������, ������� ����        (��)     ");}break;
          	case 3:{snprintf(pm,40," ������, ���� �����/����.����.(��)     ");}break;
          	case 4:{snprintf(pm,40," ³������, ����������� ������ (��)     ");}break;
          	case 5:{snprintf(pm,40," ������, ������ �����         (��)     ");}break;
          	default:{}
          	}
          	RECT rectvr = {rect112.left+460, rect112.top+yPumpingMode,rect112.left+1000,rect112.top+yPumpingMode+25};
     	    DrawText(hDCr, pm, 40, &rectvr, DT_SINGLELINE|DT_LEFT|DT_TOP);
     	    /* tk2_pumping_status (���.���� a5_tk2level.c/h)
     	     * ������� ���� ������� ������ ������� ����� � ���������*/
     	    tk2_pumping_t tk2_pumping_status = UnknownLvl;
     	  	if (q.tk2_PumpingMode == 2){
     	  		tk2_pumping_status =  f_get_pumping_status(
     	  				q.tk2_PumpingMode,
						q.d_LowLevelADC,
						q.d_HighLevelADC,
						&brl);
     	  		if(tk2_pumping_status==level_sensor_error){
     	  			KillTimer(hwnd, p1000Timer);
     	  			KillTimer(hwnd, p10Timer);
     	  			const int result = MessageBox(NULL, "���� �� ��� � ��������� ����", "�������", MB_OK | MB_ICONERROR);
     	  			switch (result){
     	  			case IDOK:{
     	 			  /* ������ ��� ������ � ������-����� ����� ���������� */
     	 			   p10Timer=SetTimer(
     	 						hwnd,                // handle to main window
     	 						IDT_TIMER_TK2_10MS,  // timer identifier
     	 						TIMER_MAIN_MS,       // msecond interval
     	 					    (TIMERPROC) NULL     // no timer callback
     	 						);
     	  				p1000Timer=SetTimer(
     	  					hwnd,                  // handle to main window
     	  					IDT_TIMER_TK2_1000MS,  // timer identifier
							TIMER_PERIOD_TK2_1000_MS,    // msecond interval
     	  						(TIMERPROC) NULL   // no timer callback
     	  						);

     	  			}
     	  				break;default:{}}

     	  		}//if(tk2_pumping_status==level_sensor_error){
     			memset(&pm,0,sizeof(pm));
     			switch(tk2_pumping_status){
    											  //123456789012345678901234567890
     			case HighLevel:  {snprintf (pm,13," ����.�����");}break;
     			case HighToLow:  {snprintf (pm,13," �����      ");}break;
     			case LowToHigh:  {snprintf (pm,13," ������     ");}break;
     			case LowLevel:   {snprintf (pm,13," ����.�����");}break;
     			case UnknownLvl: {snprintf (pm,13," �������   ");}break;
     			default:{}
     			}
     	        RECT rect1 = {rect112.left+20, rect112.top+420,rect112.left+190,rect112.top+420+25};
     	    	DrawText(hDCr, pm, 30, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
     	  	}//(q.tk2_PumpingMode == 2){

        }//((q.d_regime==remote_automatic_mode)||(q.d_regime==automatic_mode)){
        /* ���� ���� ����� */
        memset(&pm,0,sizeof(pm));
        /* ��������� ����� �������������� - ��������� �� ����� */
        switch(q.d_MotorState){
            case oFF_:SendMessage(hWndViewBtnMotorState, BM_SETCHECK, 0, 0L);break;
            case oN_: SendMessage(hWndViewBtnMotorState, BM_SETCHECK, 1, 0L);break;
            default:{}
        }
        memset(&pm,0,sizeof(pm));
        // ��������� ���� ������ ����
        switch(q.d_CX){
          	case 0x00:SendMessage(hBtnViewCX, BM_SETCHECK, 1, 0L);break;
          	case 0x01: SendMessage(hBtnViewCX, BM_SETCHECK, 0, 0L);break;
          	default:{}
        }
      //  char pm[256]={0};
		memset(&pm,0,sizeof(pm));
		switch(q.d_regime){                          //12345678901234567890123456789012345678901234567890
		case manual_mode:   		 {snprintf (pm,50," P�����,                    �          			");}break;
		case automatic_mode:		 {snprintf (pm,50," ������������,              �          			");}break;
		case remote_manual_mode:     {snprintf (pm,50," ������������ ������,       d          			");}break;
		case remote_automatic_mode:  {snprintf (pm,50," ������������ ������������, H          			");}break;
		case setting_special:   	 {snprintf (pm,50," ��������� ������������               			");}break;
		case ct_unknown: 	         {snprintf (pm,50," ��������                             			");}break;
		default:{}
		}
        RECT rect1 = {rect112.left+460, rect112.top+yMode,rect112.left+1500,rect112.top+yMode+25};
    	DrawText(hDCr, pm, 50, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
    	memset(&pm,0,sizeof(pm));
        switch(q.d_motorfault){                                       //12345678901234567890123456789012345678901234567890
    	case motor_ok:                        /* 0 	*/{snprintf (pm,50," ��,             ��� 0       		             ");}break;
    	case motor_Amperage_Unbalance:        /* 1 	*/{snprintf (pm,50," ��������,      ��� 1       					 ");}break;
    	case motor_OverLoad:                  /* 2 	*/{snprintf (pm,50," ��������������, ��� 2       					 ");}break;
    	case motor_start_jummed:   		      /* 22 */{snprintf (pm,50," ��������� ��� �����_22      					 ");}break;
    	case motor_operation_jummed:		  /* 23 */{snprintf (pm,50," ��������� � �����_23       					 ");}break;
    	case motor_UnderLoad:                 /* 3 	*/{snprintf (pm,50," ��������������  ��� 3       					 ");}break;
    	case motor_insulation_fault:          /* 4 	*/{snprintf (pm,50," ��������        ��� 4       					 ");}break;
    	case motor_differential_current:      /* 5 	*/{snprintf (pm,50," ���. ������     ��� 5       					 ");}break;
    	case motor_dry_move:				  /* 6  */{snprintf (pm,50," ����� ���       ��� 6       					 ");}break;
    	case motor_liquidSensor_fault:        /* 7  */{snprintf (pm,50," ����� ������� ��� 7       					 ");}break;
    	case motor_start_UnderVoltage:        /* 8  */{snprintf (pm,50," ������ �������,����_8       					 ");}break;
    	case motor_UnderVoltage:			  /* 81 */{snprintf (pm,50," ������ �������, ������_81   					 ");}break;
    	case motor_OverVoltage:				  /* 82 */{snprintf (pm,50," ������ �������_82           					 ");}break;
    	case motor_StartNumber_limit:		  /* 9  */{snprintf (pm,50," ������ ����     ��� 9              			 ");}break;
    	case motor_ThermalRelay_off:		/* 0x0A */{snprintf (pm,50," ���������_10                					 ");}break;
    	case motor_HumidityRelay_off:		/* 0x0B */{snprintf (pm,50," ���� �������� 11           					 ");}break;
    	case motor_distance_cntr: 			/* 0x0C */{snprintf (pm,50," ����� ����.��������� 12   					 ");}break;
    	case motor_other_faults:			/* 0x0D */{snprintf (pm,50," ����� ���� 13              					 ");}break;

    	case wrong_parameters_set:          /* 0x46 */{snprintf (pm,50," ������� �� ������_46        					 ");}break;
    	/* ����������� ����� ��������� ����������� ������� ����� */
    	case motor_power_amperage_suss:		/* 0x80 */{snprintf (pm,50," ������ ������/������._80    					 ");}break;
    	case no_pressure_sensor:            /* 0x86 */{snprintf (pm,50," �� �� ������� ����� 86     					 ");}break;
    	case pressure_sensor_setting_fault: /* 0x87 */{snprintf (pm,50," ������� ������� �����_87    					 ");}break;
    	case motor_unknown_fault:			/* 0xFF	*/{snprintf (pm,50," �� �������������           					 ");}break;
    	default:{}
    	}
    	RECT rectmf1 = {rect112.left+460, rect112.top+y_motorfault,rect112.left+1000,rect112.top+y_motorfault+25};
    	DrawText(hDCr, pm, 50, &rectmf1, DT_SINGLELINE|DT_LEFT|DT_TOP);
    	memset(&pm,0,sizeof(pm));

    	switch(q.d_FS){     	    //12345678901234567890123456789012345678901234567890
      	case DISABLE:{snprintf(pm,50," ����������                                      ");}break;
      	case ENABLE :{snprintf(pm,50," ���������                                       ");}break;
      	default:{}
      	}
      	RECT rectsz1 = {rect112.left+460, rect112.top+ySelfStart,rect112.left+1000,rect112.top+ySelfStart+25};
      	DrawText(hDCr, pm, 50, &rectsz1, DT_SINGLELINE|DT_LEFT|DT_TOP);



//			snprintf(tk2LogStr,80," q.d_aAmperage =%4d              ",(int)q.d_aAmperage);
//			f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));

      	memset(&pm,0,sizeof(pm));
        /* ����� ���� � */
        uint16_t amp= 0;
        uint16_t fr = 0;
        amp= (uint16_t)q.d_aAmperage;
        fr = (uint16_t)((q.d_aAmperage-(amperf_t)amp)*100.0f);
                      //1234567890123456789012345678901234567890
        snprintf (pm,50,"%3d.%2d A                              		",amp, fr);
        RECT rectfa1 = {rect112.left+460, rect112.top+y_aAmperage,rect112.left+1000,rect112.top+y_aAmperage+25};
        DrawText(hDCr, pm, 50, &rectfa1, DT_SINGLELINE|DT_LEFT|DT_TOP);
        //
        memset(&pm,0,sizeof(pm));
        /* ����� ���� � */
        amp= (uint16_t)q.d_bAmperage;
        fr = (uint16_t)((q.d_bAmperage-(amperf_t)amp)*100.0f);
        snprintf (pm,50,"%3d.%2d A                              		",amp, fr);
        RECT rectfb1 = {rect112.left+460, rect112.top+y_bAmperage,rect112.left+1000,rect112.top+y_bAmperage+25};
        DrawText(hDCr, pm, 50, &rectfb1, DT_SINGLELINE|DT_LEFT|DT_TOP);
        //
        memset(&pm,0,sizeof(pm));
        /* ����� ���� C */
        amp= (uint16_t)q.d_cAmperage;
        fr = (uint16_t)((q.d_cAmperage-(amperf_t)amp)*100.0f);
        snprintf (pm,50,"%3d.%2d A                              		",amp, fr);
        RECT rectfc1 = {rect112.left+460, rect112.top+y_cAmperage,rect112.left+1000,rect112.top+y_cAmperage+25};
        DrawText(hDCr, pm, 50, &rectfc1, DT_SINGLELINE|DT_LEFT|DT_TOP);
        memset(&pm,0,sizeof(pm));
        /* ������г� ������  */
        snprintf (pm,50,"%3d %%                              			", (uint16_t)q.d_AmperageUnbalance);
        RECT rectab1 = {rect112.left+460, rect112.top+y_AmperageUnbalance,rect112.left+1000,rect112.top+y_AmperageUnbalance+25};;
        DrawText(hDCr, pm, 50, &rectab1, DT_SINGLELINE|DT_LEFT|DT_TOP);
        memset(&pm,0,sizeof(pm));
        /* �ϲ� �����ֲ� */
        amp= (uint16_t)q.d_InsulationResistance;
        fr = (uint16_t)((q.d_InsulationResistance-(float)amp)*100.0f);
        snprintf (pm,50,"%6d                                 			",amp);
        RECT rectir1 = {rect112.left+460, rect112.top+y_InsulationResistance,rect112.left+1000,rect112.top+y_InsulationResistance+25};
        DrawText(hDCr, pm, 50, &rectir1, DT_SINGLELINE|DT_LEFT|DT_TOP);
        memset(&pm,0,sizeof(pm));

//		snprintf(tk2LogStr,80," q.d_DryModeADC =%d              ",q.d_DryModeADC);
//		f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));

        /* ��� ������ ����*/
        snprintf (pm,50," %d                                 ",q.d_DryModeADC);
        RECT rectcx1 = {rect112.left+460, rect112.top+y_DryModeADC,rect112.left+1000,rect112.top+y_DryModeADC+25};
        DrawText(hDCr, pm, 50, &rectcx1, DT_SINGLELINE|DT_LEFT|DT_TOP);
//		f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));
    	//����������� ������� ������ ������ ����
    	RECT rectblue = {rect112.left+680,rect112.top+70,rect112.left+819,rect112.top+700};
    	FillRect(hDCr, &rectblue, (HBRUSH)29);
        ReleaseDC(hwnd, hDCr);

	}//tk2ProcessState==1){/////////////////////////////////////////////////////////////////
	/***********************************************************************************************************
	 * ʲ���� ������� ��²������� ������ֲ����� �����˲���
	 ***********************************************************************************************************/
	else{}

//	SetFocus(hwnd);
//	snprintf(tk2LogStr,80," tk2ProcessCntr=%f              ",tk2ProcessCntr);
//	f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));
	return tk2ProcessCntr;
}

static void f_InitTK2_Drow(void){
	qOld.d_FS=UNKNOWN;
	qOld.tk2_PumpingMode=7;

}

static BOOL CALLBACK DestoryChildCallbackAR(
  HWND   hwnd,
  LPARAM lParam
)
{
  if (hwnd == hABtnHighPanel) { DestroyWindow(hwnd);}
  if (hwnd == hABtnStart) { DestroyWindow(hwnd);}
  if (hwnd == hABtnStop) { DestroyWindow(hwnd);}
  if (hwnd == hABtnBuster) { DestroyWindow(hwnd);}
  if (hwnd == hABtnLowPanel) { DestroyWindow(hwnd);}
  if (hwnd == hABtnToR) { DestroyWindow(hwnd);}
  if (hwnd == hWndProgressBar_Barrel) { DestroyWindow(hwnd);}
  //https://stackoverflow.com/questions/30786481/winapi-c-how-to-completeley-wipe-clear-everything-in-window
  return TRUE;
}
static BOOL CALLBACK DestoryChildCallbackRA(
  HWND   hwnd,
  LPARAM lParam
)
{
  if (hwnd == hRBtnHighPanel) { DestroyWindow(hwnd);}
  if (hwnd == hRBtnStart) { DestroyWindow(hwnd);}
  if (hwnd == hRBtnStop) { DestroyWindow(hwnd);}
  if (hwnd == hRBtnLowPanel) { DestroyWindow(hwnd);}
  if (hwnd == hRBtnToA) { DestroyWindow(hwnd);}
  //https://stackoverflow.com/questions/30786481/winapi-c-how-to-completeley-wipe-clear-everything-in-window
  return TRUE;
}





static void f_desktop_tk2_session(void){
	f_desktop_tk_session(
			&tk2Queue
			);
}
//      			FillRect(hDCr, &rect_Mn, (HBRUSH)(COLOR_WINDOW+1));
//
//
//
//      	   	    InvalidateRect(hwnd,NULL,TRUE);
//      	     	UpdateWindow(hwnd);
//      	        SetFocus(hwnd);
