
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
#include "a20_tk5.h"
#include "a21_tk5us.h"
#include "a22_tk5fs.h"
#include "../../mb_funcs/tw_mb.h"
#include "../../mb_funcs/tx_manual.h"
#include "../a0_win_control.h"
#include "../a2_setting.h"
////#include "a5_tk5levels.h"
#include "a20_tk5.h"
#include "a21_tk5us.h"
//#include "a22_tk5fs.h"
#include "../a3_tkTxRx.h"
#include "../../auxs/hexdec.h"
#include "a3_tk5z.h"
//#include "../../mb_devs/tk5_dist.h"
#include "../e1_passwordEdit.h"
//#include "a22_tk5fs.h"
//#include "../../mb_devs/tk5/tkThermal.h"
#include "a4_tk5log.h"
#include "../../wConfig.h"
#include "../../mb_funcs/tw_mb.h"
/********************************************************************************************************************************
 *  �̲�Ͳ �� ����ֲ� �ղ����� (���������) ²��� ������ֲ����� �����˲��� PC_DESK <-> tk5
 * ******************************************************************************************************************************
 * */


/* ���������, ��� ������� �����
 * ���������� ���� ��������� ������, �� ��������
 * ��������� ����� � ���������� ������, ���� �� �����
 * ��������� ��������� - ���������� ����� ���������� �� ��2,
 * ������ � ������ ����������� ��������� ����� 'a3_tk5TxRx.c/h' */
   queue_t tk5Queue = {0};

   /* ��������� �������� �� ���������� �����, ���� ������ ������������ ����
    * ��'� ����� ����������� �����������,
    * ������ � ������ ����������� ��������� �����  a4_tk5log.c/h*/
   FILE *fp_tk5LogFile=NULL;
   /* �����, ���� ���������� ����� ����. ������������ ��������� � ������ 10��,
    * ���� �� ������, ����� ���������� � ����.
    * ���� ����� ������ - ����� ������ �������� � ����, ���. ������� f_tk5Logging(), ���� a3_tk5z.c/h */
   char tk5LogStr[512]={0};

/* ��� ��������� ����, ������ �� ���� �� �� ����� ������ ������,
 * ������������ ���� � �������, �� � �������� ��� ��� ������ ������
 *
 * ��� ��������� ������ '����.����.���������' ��������� ����� �� ���������� ���������� �������
 * �������� ���� ������������ �������� ������ ������ � tk5
 * ����� ������ ����������.
 * ��� ��������� ������ '����� ����.���������'
 * ����������� ������ ����� ������������� PC_DESK <-> tk5,
 * � ����� �� ���������� "���� ����.���"*/
ProcessState_t tk5Process=ProcessIddle;
 /* �� ������ ���������� ������� ������������� ���������.
  * ��� ���������  ������ '����� ����.���������' tk5Process=ProcessBusy
  * ��� ���������   '����� ����.���������' tk5Process=ProcessIddle
  * */

/* ���������� �� ������ ������������ ����� ����
 * ��������, ������ �� ���������� ������� ���������� �� ������*/
static FunctionalState tk5btn_access = ENABLE;
/* ˳������� ��������*/
static uint32_t tk5btnSleepCntr = 0;
/* ��������� �������� */
#define  tk5_BTN_BLOKING_MS         400

/* ��������� ���������, �� ������ ���������� ��������� ������
 * ������ ��2 - ��������� �� ������� �������������/��������������*/
wtk5_t q5={0};
extern tk5user_settings_t Tk5us;
extern tk5fs_t tk5fs;
/* �������� �������� ��������� ��� ������ � ���� ���� ���, �� ��������*/
//static wtk5_t q5Old={0};

/* ���������� ����� ��� ������ ������-���� ������� ����� �������������� */
//extern float2_t Itk5_Max;

HWND hWndtk5 = NULL;
HWND hWndtk5usCh;
HWND hWndTk5fsCh;
//extern HWND hWndTk5usCh;
HINSTANCE hinst_tk5;
static  HFONT hfDefault;
const char g_szClassNameTK5[] = "TK5_WndClass";
/* �������� ����, ����������� �� ������� � ����, ��� ����������� ������ � ����� */
extern modbus_master_rx_msg_t mIn_rx_msg;
/* �������� ����, ����������� �� ������� � ����, ��� ����������� ������ � ����� */
extern modbus_master_tx_msg_t mOut_tx_msg;
/* �������� ����, ����������� �� ������� � ����, ��� ����������� ������ � ����� */
extern modbus_status_t RxMasterModbusStatus;

/* �������� ����� ��� ���������� �������� ������� ���� ������*/
static event_t  met_zero_MBsuccess = NO_EVENT;
/* ˳������� ���� � ������ 100�� ��� �������� ����� ���������� � ����� ��������*/
static uint32_t tk5TimerCntr = 0;

//static uint16_t tk5ProcessState =0;
/* �����p ���� �������, ��������� �������,
 * � ��� ��� ���������� ���� ��������
 * �� ������� �� ������� � �� ������� �� �������*/
/* � ����� ���� ����������� ������� �������, ��� ������������� ������ */
int RW5_case=0;

//static char user_msg5[256]={0};

/* ���������� ��� ��������� ���� */
/* ������� �������� ���������� Y ����� ������������� */
static uint32_t y_LeftRadio = 240;
/* ��� ������ ����� ���� 1024 �� ������ 1009 */
static uint16_t x_left_panel  				= 20;
static uint16_t x_left_right_width_panel	= 150;
static uint16_t x_centr_panel   			= 190;
static uint16_t x_centr_width 				= 629;
static uint16_t x_right_panel 				= 839;
/* ����/����������� �� ������������ ������������ �������,
 * ����������� � ���� PPB ������������ ������� */

static uint16_t y_HeatPro							=0;
static uint16_t y_aAmperage							=0;
static uint16_t y_bAmperage							=0;
static uint16_t y_cAmperage							=0;
static uint16_t y_power								=00;
static uint16_t y_cosinus_factor					=00;
static uint16_t y_unbalance_factor					=00;
static uint16_t y_harm_factor						=00;
static uint16_t y_motohours							=00;
static uint16_t y_voltage							=00;
static uint16_t y_frequency							=00;

static uint16_t y_leakAmperage_A					=0;
static uint16_t y_GrundAmperageDistortion			=0;
static uint16_t y_LoadType							=0;

static uint16_t y_AmperageUnbalance					=0;
static uint16_t y_ActivePower_kW					=0;
static uint16_t y_ReactivePower_kW					=0;
static uint16_t y_UnbalanceLostPower_kW				=0;
static uint16_t y_HarmonicLostPower_kW				=0;

static uint16_t y_aTotalHarmonicDistortion			=0;
static uint16_t y_bTotalHarmonicDistortion			=0;
static uint16_t y_cTotalHarmonicDistortion			=0;
static uint16_t y_THDi_HarmonicAmperageDistortion	=0;
static uint16_t y_aNegativeAmperage_A				=0;
static uint16_t y_bNegativeAmperage_A				=0;
static uint16_t y_cNegativeAmperage_A				=0;
static uint16_t y_aNeutralAmperage_A				=0;
static uint16_t y_bNeutralAmperage_A				=0;
static uint16_t y_cNeutralAmperage_A				=0;
static uint16_t y_sumNeutralAmperage_A				=0;
static uint16_t y_PhaseRotation						=0;
static uint16_t y_leakFurie							=0;
static uint16_t y_leakFuriePhase                    =0;



static uint16_t yDebugMode							= 100;
static uint16_t yAmperageAmplitude					= 150;
static uint16_t yBAmperageAmplitude			    	= 200;
static uint16_t yCosFi                         		= 250;
static uint16_t yTk4screenMode						= 300;

uint16_t DebugMode						= 0;
uint16_t Tk4screenMode					= 0;
/* ��� ������� - ������, �������, ���-����, ���������� � �.�.*/
extern HWND  comWndChild;
static HWND  hEdit;
static HWND  hGRID_OverLoadProtection;
static HWND  hGRID_UnderLoadProtection;
static HWND  hGRID_VoltageFault;
static HWND  hGRID_AmperageUnbalanceProtection;
static HWND  hGRID_GroundProtection;
//
static HWND  hGRID_StartNumberLimit;
static HWND  hGRID_JummedProtection	;
//
static HWND  hGRID_Phase_A_OverLoadProtection;
static HWND  hGRID_Phase_B_OverLoadProtection;
static HWND  hGRID_Phase_C_OverLoadProtection;
//
static HWND  hGRID_FrequencyFault;
static HWND  hGRID_PhaseSeqFault;

//static HWND  hBtnViewAutoctrl;
static HWND  hWndViewBtnMotorState;
//static HWND  hBtnViewCX;
//static HWND  hWndProgressBar_Barrel;
static HWND  hWndProgressBar_ThermalModel;
static HWND  hWndProgressBar_Queue;
static HWND  hButtonSend;
static HWND  hEdit_tk5_ADDR;
//static HWND  hABtnHighPanel;
//static HWND  hABtnStart;
//static HWND  hABtnStop;
//static HWND  hABtnBuster;
//static HWND  hABtnLowPanel;
//static HWND  hABtnToR;

static HWND  hRBtnHighPanel;
static HWND  hRBtnStart;
static HWND  hRBtnStop;
//static HWND  hRBtnLowPanel;
//static HWND  hRBtnToA;

UINT_PTR  p1000Timer;
static UINT_PTR  p1333Timer;
static UINT_PTR  p10Timer;
static UINT_PTR  pTestTimer;
static HWND hDebugMode;
static HWND hAmperageAmplitude;
static HWND hBAmperageAmplitude;
static HWND hCosFi;
static HWND hTk4screenMode;




/* �� ������� ����������� ����� ��� ��� �������� ����������� � VK_UPDATE_WINDOW
 * ��� ���������� ��� ������, ���������� � ��������� ����
 * wParam-���������� � ��������� �����
 * ������������� ��������� ����������� RW5_case */
static int f_tk5UpdateWindow(HWND hwnd, int wParam, int responce_status);
/* �� �������-�������� ������� ������������ ������� */
static void f_desktop_tk5_session(void);
/* �����������, ��� �������� ��������� ������� ��������� ��� ����������� ����������� */
static void f_Inittk5_Drow(void);


//static BOOL CALLBACK DestoryTk5ChildCallbackRA(/* ����->���*/
//  HWND   hwnd,
//  LPARAM lParam
//);

static void Init_YMode(uint16_t Phasnost, uint16_t LoadMode);
uint8_t tk5_Addr=1;
//static int period=1000;
#ifndef		tk5_DISTANCE_WND
/* �� �������� ���� ����.�����˲��� �� ��������� � �������� �������,
���� ���� ���������� ��������� ��� */
LPCSTR f_RegisterTK5WndClass(HINSTANCE hinst){
	hinst_tk5=hinst;
	WNDCLASS w;
		memset(&w,0,sizeof(WNDCLASS));
		w.lpfnWndProc = WndProcTK5;
		w.hInstance = hinst;
		w.style = CS_HREDRAW | CS_VREDRAW;
		w.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
		w.lpszClassName = g_szClassNameTK5;
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
HWND f_CreateTK5Wnd(HWND hwnd){
	fp_tk5LogFile=f_tk5log_Init();
	HWND child5;
	MSG Msg;
	child5 = CreateWindowEx(
		0,
		g_szClassNameTK5,
		"��5.����������� ���������",

//        WS_CHILD
//		| (WS_OVERLAPPEDWINDOW&(~(WS_MAXIMIZEBOX|WS_MINIMIZEBOX)))
//		|WS_BORDER
//		|WS_VISIBLE
//
//		,

		(WS_OVERLAPPEDWINDOW&(~(WS_MAXIMIZEBOX|WS_MINIMIZEBOX)))|WS_BORDER|WS_VISIBLE
		,
		CW_USEDEFAULT, CW_USEDEFAULT, 1024+512+128, 1000,
		NULL, NULL, hinst_tk5, NULL
		);


	if(child5 == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	/* ��������� ����� ����, �� ������������ ������������
	 * ������ ������������ ������ */
	hWndtk5=child5;
	f_RegisterTk5usWndClass(hinst_tk5);
	f_RegisterTk5fsWndClass(hinst_tk5);


	HMENU hMenubar = CreateMenu();
	HMENU hMenu = CreateMenu();

	AppendMenu(hMenu, MF_STRING, IDM_tk5COMPORT_SETTING, "&������������ ���-�����");
	AppendMenu(hMenu, MF_STRING, IDM_tk5USER_SETTINGS,   "&������������ �����������");
	AppendMenu(hMenu, MF_STRING, IDM_tk5FACILITY_SETTING,"&������������ ���������");
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenu, MF_STRING, IDM_tk5QUIT, "&�����");

	AppendMenu(hMenubar, MF_POPUP, (UINT_PTR) hMenu, "&������������");

	SetMenu(child5, hMenubar);


	ShowWindow(child5, SW_NORMAL);
	UpdateWindow(child5);

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return child5;
}
#endif
#ifdef		tk5_DISTANCE_WND



/* ����� ����� � Windows-���������, ��������� ��������� ���� */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow){
	/* ³�������� ���������� ���� ��� ���� */
	fp_tkLogFile=f_tk5log_Init();
	/* ����� ��������� ���� */
	hinst_tk5 =hInstance;
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

	hWndtk5 = CreateWindowEx (
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

	if(hWndtk5 == NULL){
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	/* ��������� ����� ����, �� ������������ ������������
	 * ������ ������������ ������ */
	f_Register_PasswordWndClass(hInstance);
	f_RegisterSWndClass(hInstance);

	f_RegisterTK112USWndClass(hInstance);
	f_Registertk5fsWndClass(hInstance);

	HMENU hMenubar = CreateMenu();
	HMENU hMenu = CreateMenu();

	AppendMenu(hMenu, MF_STRING, IDM_COMPORT_SETTING, "&������������ ���-�����");
	AppendMenu(hMenu, MF_STRING, IDM_USER_SETTINGS, "&������������ �����������");
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenu, MF_STRING, IDM_QUIT, "&�����");

	AppendMenu(hMenubar, MF_POPUP, (UINT_PTR) hMenu, "&������������");

	SetMenu(hWndtk5, hMenubar);
	SetMenu(hWndtk5, hMenu);
	ShowWindow(hWndtk5, SW_NORMAL);
//	ShowWindow(hwndm, nCmdShow);
	UpdateWindow(hWndtk5);

	while(GetMessage(&Msg, NULL, 0, 0) > 0){
					TranslateMessage(&Msg);
					DispatchMessage(&Msg);
	}
	return Msg.wParam;
}
#endif		//tk5_DISTANCE_WND
#define PHASNOST 3 /* 	3 ��� ����
						1 ���� ���� */
#define LOADMODE 0 /*0 - ����� ������������,
 	 	 	 	 	 1 - ������������
					 2 - �� ������������*/

LRESULT CALLBACK WndProcTK5(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	modbus_master_tx_msg_t mbTxMsg={0};
//	Init_YMode( PHASNOST, LOADMODE);
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
	   			 x_left_panel, y_LeftRadio, x_left_right_width_panel , 20, hwnd,(HMENU) IDB_tk5SHOW_MOTOR_OPERATING, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
//#define IDBI_GRID_OVERLOADPROTECTION					1

	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
	   	hGRID_OverLoadProtection= CreateWindow("button", "��������������" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +20, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_OVERLOADPROTECTION, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

//#define IDBI_GRID_UNDERLOADPROTECTION					2

	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
	   	hGRID_UnderLoadProtection= CreateWindow("button", "��������������" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +40, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_UNDERLOADPROTECTION, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

//#define IDBI_GRID_VOLTAGEFAULT							4

	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
	   	  hGRID_VoltageFault= CreateWindow("button", "�������" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +60, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_VOLTAGEFAULT, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

//#define IDBI_GRID_AMPERAGEUNBALANCEPROTECTION			8
	   	  /* ˳��� ��������� ������ ���� */
	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
		  hGRID_AmperageUnbalanceProtection= CreateWindow("button", "��������" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +80, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_AMPERAGEUNBALANCEPROTECTION, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

//#define IDBI_GRID_GROUNDPROTECTION						0X10

	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
		  hGRID_GroundProtection= CreateWindow("button", "���� �� �����" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +100, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_GROUNDPROTECTION, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

////
//#define IDBI_GRID_STARTNUMBERLIMIT						0X20

	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
		  hGRID_StartNumberLimit= CreateWindow("button", "����� ������" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +120, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_STARTNUMBERLIMIT, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

//#define IDBI_GRID_JUMMEDPROTECTION						0X40

	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
		  hGRID_JummedProtection= CreateWindow("button", "��������" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +140, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_JUMMEDPROTECTION, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

////
//#define IDBI_GRID_PHASE_A_OVERLOADPROTECTION			0X80
	   	  /* ˳��� ��������� ������ ���� */
	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
		  hGRID_Phase_A_OverLoadProtection= CreateWindow("button", "�������������� �" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +160, x_left_right_width_panel , 20, hwnd,(HMENU) IDB_tk5SHOW_CX, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

//#define IDBI_GRID_PHASE_B_OVERLOADPROTECTION			0X100
	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
		  hGRID_Phase_B_OverLoadProtection= CreateWindow("button", "�������������� B" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +180, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_PHASE_B_OVERLOADPROTECTION, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

//#define IDBI_GRID_PHASE_C_OVERLOADPROTECTION			0X200

	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
		  hGRID_Phase_C_OverLoadProtection= CreateWindow("button", "�������������� C" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +200, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_PHASE_C_OVERLOADPROTECTION, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

////
//#define IDBI_GRID_FREQUENCY_FAULT                       0X400

	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
	   	  hGRID_FrequencyFault= CreateWindow("button", "�������" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +220, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_FREQUENCY_FAULT, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

//#define IDBI_GRID_PHASE_SEQUENCE_FAULT                  0X800

	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
		  hGRID_PhaseSeqFault= CreateWindow("button", "����������� ���" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +240, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_PHASE_SEQUENCE_FAULT, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

		  //hGRID_OverLoadProtection
		  //hGRID_UnderLoadProtection
		  //hGRID_VoltageFault
		  //hGRID_AmperageUnbalanceProtection
		  //hGRID_GroundProtection










          /* ����� ����� ����� ��� ������ ���������  */
	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE |*/ BS_GROUPBOX| WS_GROUP,
	   			x_right_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 290, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);


			hRBtnHighPanel=CreateWindowW(L"Button", L"",
	                  WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX,
	             x_left_panel, 60,  150, 210, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
			hRBtnStart=CreateWindow("BUTTON", "����",
                                         WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
										   x_left_panel, 90,  150, 30, hwnd, (HMENU)IDB_tk5BtnSTART,
										   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
			hRBtnStop=CreateWindow("BUTTON", "����",
                                         WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
										   x_left_panel, 150, 150, 30, hwnd, (HMENU)IDB_tk5BtnSTOP,
										   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
//			/* ���������� �������-��� ��� ��������� �������������� */
//			 hWndProgressBar_ThermalModel = CreateWindowEx(
//			 				             0,
//			 				             PROGRESS_CLASS,
//			 				             (LPSTR)NULL,
//			 				             WS_VISIBLE | WS_CHILD /*| PBS_VERTICAL*/,
//			 				             x_left_panel,
//			 							 660,
//			 				             150,
//			 				             20,
//			 				             hwnd,
//			 				             (HMENU)IDPB_tk5PROGRESS_BAR_ThermalModelBehavior,
//			 				             (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
//			 				             NULL
//			 							 );
//			 if (!hWndProgressBar_ThermalModel){
//			 		MessageBox(NULL, "Progress Bar Faileg5.", "Error", MB_OK | MB_ICONERROR);
//			 }
//			 /* ����� 120% - ������ � ��������� */
//			 SendMessage(hWndProgressBar_ThermalModel, PBM_SETRANGE, 0, MAKELPARAM(
//			 		0,   //minimum range value
//			 		150  //maximum range value
//			 		));
//
//			 SendMessage(hWndProgressBar_ThermalModel, PBM_SETPOS,
//			 		0, //Signed integer that becomes the new position.
//			 		0  //Must be zero
//			 		);


	          /* ����� ����� ��� ������ ���������  */
		   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE| */BS_GROUPBOX| WS_GROUP,
		   			x_right_panel, 60, 150, 640, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);


	          /* EditBox ��� ������-������ */
	          char addr_s[3]={0};
	          hEdit_tk5_ADDR = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", addr_s,
					WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
					, x_right_panel, 80, 40, 25,
					hwnd, (HMENU)ID_tk5_EDIT_ADDRESS , GetModuleHandle(NULL), NULL);
			  SendMessage(hEdit_tk5_ADDR, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

			  snprintf (addr_s,20,"%X",tk5_Addr);
			  SetWindowText(hEdit_tk5_ADDR,(LPCTSTR)addr_s);

	          CreateWindow("BUTTON", "���� ����.���.",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_right_panel, 105, 150, 20, hwnd, (HMENU)IDB_tk5TEST_DISTANCE_CTRL,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
	          CreateWindow("BUTTON", "����� ����.���.",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_right_panel, 130, 150, 20, hwnd, (HMENU)IDB_tk5START_DISTANCE_CTRL,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
	          CreateWindow("BUTTON", "����  ����.���.",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_right_panel, 155, 150, 20, hwnd, (HMENU)IDB_tk5STOP_DISTANCE_CTRL,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);

	          CreateWindow("BUTTON", "����.�������",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_right_panel+30, 205, 150-30, 20, hwnd, (HMENU)IDB_tk5RESET_MOTOR_PROTECTION_1,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
	          CreateWindow("BUTTON", "����.������� 2",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_right_panel, 230, 150, 20, hwnd, (HMENU)IDB_tk5RESET_MOTOR_PROTECTION_2,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);


	          CreateWindow("BUTTON", "�����",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_right_panel, 300, 150, 20, hwnd, (HMENU)IDB_tk5DIST_EXIT,
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
				 	(HMENU)IDPB_tk5QUEUE_MSG ,
				 	(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
				 	NULL
				 	);
			  if (!hWndProgressBar_Queue){
				 	MessageBox(NULL, "Progress Bar Failed.", "Error", MB_OK | MB_ICONERROR);
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
						IDT_TIMER_tk5_10MS,  // timer identifier
						TIMER_MAIN_MS,       // msecond interval
					    (TIMERPROC) NULL     // no timer callback
						);
			  f_Inittk5_Drow();

/***********************************************************************************************************
 * ²�����������
 * *********************************************************************************************************/
			  hDebugMode = CreateWindow("COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
			         		  1009, yDebugMode, 200, 800,
			 				  hwnd, (HMENU)IDCB_YDEBUGMODE,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),NULL);
			  SendMessage(hDebugMode, CB_ADDSTRING, 0, (LPARAM)" ������� ������������			  ");
			  SendMessage(hDebugMode, CB_ADDSTRING, 0, (LPARAM)" ���������, ��������             ");
			  SendMessage(hDebugMode, CB_ADDSTRING, 0, (LPARAM)" ���������, �����������           ");
			  SendMessage(hDebugMode, CB_SETCURSEL, DebugMode, 0);

	          CreateWindow("BUTTON", "Ok",
	                         WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
						     1210, yDebugMode, 30, 20, hwnd, (HMENU)IDB_YDEBUGMODE,
							 (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
	    	  char strValue[256]={0};



	    	  hAmperageAmplitude = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", strValue,
						WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
						, 1009, yAmperageAmplitude, 200, 25,
						hwnd, (HMENU)IDE_YHAMPERAGEAMPLITUDE, GetModuleHandle(NULL), NULL);
			  SendMessage(hAmperageAmplitude, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
			  SetWindowText(hAmperageAmplitude,"100");

			  CreateWindow("BUTTON", "Ok",
			             WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
						1210, yAmperageAmplitude, 30, 20, hwnd, (HMENU)IDB_YHAMPERAGEAMPLITUDE,
						(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);

			  hBAmperageAmplitude = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", strValue,
						WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
						, 1009, yBAmperageAmplitude, 200, 25,
						hwnd, (HMENU)IDE_YBAMPERAGEAMPLITUDE, GetModuleHandle(NULL), NULL);
			  SendMessage(hBAmperageAmplitude, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
			  SetWindowText(hBAmperageAmplitude,"100");
			  CreateWindow("BUTTON", "Ok",
						WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
						1210, yBAmperageAmplitude, 30, 20, hwnd, (HMENU)IDB_YBAMPERAGEAMPLITUDE,
						(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);

			  hCosFi = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", strValue,
						WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
						, 1009, yCosFi, 200, 25,
						hwnd, (HMENU)IDE_YCOSFI, GetModuleHandle(NULL), NULL);
			  SendMessage(hCosFi, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
			  SetWindowText(hCosFi,"100");
			  CreateWindow("BUTTON", "Ok",
						WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
						1210, yCosFi, 30, 20, hwnd, (HMENU)IDB_YCOSFI,
						(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);

			  hTk4screenMode = CreateWindow("COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
			         		  1009, yTk4screenMode, 200, 800, hwnd, (HMENU)IDCB_YTK4SCREENMODE,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),NULL);

			  SendMessage(hTk4screenMode, CB_ADDSTRING, 0, (LPARAM)" ���� �� ���        			  ");
			  SendMessage(hTk4screenMode, CB_ADDSTRING, 0, (LPARAM)" �������(�) � �������(��)         ");
			  SendMessage(hTk4screenMode, CB_ADDSTRING, 0, (LPARAM)" ����� �.�. vs �.�., ���          ");
			  SendMessage(hTk4screenMode, CB_ADDSTRING, 0, (LPARAM)" ����� �.�. vs �.�., ������       ");
			  SendMessage(hTk4screenMode, CB_ADDSTRING, 0, (LPARAM)" ������� Գ                       ");
			  SendMessage(hTk4screenMode, CB_ADDSTRING, 0, (LPARAM)" ������(�) � ��������            ");
			  SendMessage(hTk4screenMode, CB_SETCURSEL, Tk4screenMode, 0);
			  CreateWindow("BUTTON", "Ok",
						WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
						1210, yTk4screenMode, 30, 20, hwnd, (HMENU)IDB_YTK4SCREENMODE,
						(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);

			  _beginthread((void*)f_desktop_tk5_session, 0, NULL);
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

	    	RECT rectyL =    {rect.left+210,rect.top+25, rect.left+510,rect.top+50};
	    	DrawText(hDC, "���������", 10, &rectyL, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	//����������� ������� ������ ������ ����
	    	RECT rectblue = {rect.left+550,rect.top+70,rect.left+819,rect.top+700};
	    	FillRect(hDC, &rectblue, (HBRUSH)29);


	    	/*������� ��������� ����������� ����� 					*/
	    	/*���������� �������� ���������� �� ������ � ����� ���  */
//	    	extern procentf_t aTotalHarmonicDistortion;
//	    	extern procentf_t bTotalHarmonicDistortion;
//	    	extern procentf_t cTotalHarmonicDistortion;

	    	/************************************************************************
	    	���������� �������� ���������� �� ������ ������������    				*/
	    	//extern procentf_t THDi_HarmonicAmperageDistortion;
	    	/*THD� ����� �� ������ ����������� ����� �������� ������.
	    	THDi ����� 10% ��������� ����������,
	    	����� ������������� ���������� �������, ����������� �� 6%.

	    	THDi �� 10% � 50% ����� �� ������ ��������� �����������.
	    	������ ����������� ��������� �������������, ��������� �����������,
	    	�� ������, �� ����� �� ��������� ������ ���� ����������������.

	    	THDi, �� �������� 50%, ����� �� ����� �������� �����������.
	    	������� ����� ������ ���������� � ����. ���������� ����������� ����� ������
	    	 � ������ � �� ��������, ������� �������������
	    	 ������� ���������� ���������������� ������.

	    	���������� ������ ������ ����������:
	    	(��� ��������� ��������� ���������).
	    	���� ����������� ������� �������� ����������� � ������� 10%,
	    	����� ������ ���������� ������ �����������.
	    	������� �� ���� ��������, ���������� ������ ������:
	    	32,5% ��� ���������� ��������������
	    	18% ��� ��������� ��������������
	    	5% ��� ��������������.
	    	***************************************************************************
	    	3, 5, 7, 11 and 13 - �������� ��������� ��� ��������������




	    		//vatt_t	ActivePower 		= 	0;
	    	 	//������� ������� ���������

	    		//vatt_t	ReactivePower 		=   0;

	    		//vatt_t	UnbalanceLostPower 	=	0;

	    		//vatt_t	HarmonicLostPower 	= 	0;

	    		//vatt_t	NeutralHarmonicLostPower 	= 	0;


	    		//vatt_t	NegativeHarmonicLostPower 	= 	0;*/



	    if(y_HeatPro!=0){
	    	RECT rect0 = {rect.left+210, rect.top+y_HeatPro,rect.left+1000,rect.top+y_HeatPro+25};
	    	             //1234567890123456789012345678901234
	    	DrawText(hDC, "������ ������ ��������������, %", 34, &rect0, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_aAmperage!=0){
			RECT rectfa = {rect.left+210, rect.top+y_aAmperage,rect.left+1000,rect.top+y_aAmperage+25};
			DrawText(hDC, "����� ���� �, �                ", 32, &rectfa, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_bAmperage!=0){
	    	RECT rectfb = {rect.left+210, rect.top+y_bAmperage,rect.left+1000,rect.top+y_bAmperage+25};
    		DrawText(hDC, "����� ���� �, �                ", 32, &rectfb, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_cAmperage!=0){
    		RECT rectfc = {rect.left+210, rect.top+y_cAmperage,rect.left+1000,rect.top+y_cAmperage+25};
    		DrawText(hDC, "����� ���� �, �                ", 32, &rectfc, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_power!=0){
	    	RECT rectpw = {rect.left+210, rect.top+y_power,rect.left+1000,rect.top+y_power+25};
	    	DrawText(hDC, "���������, ���                ", 32, &rectpw, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_cosinus_factor!=0){
    		RECT rectcf = {rect.left+210, rect.top+y_cosinus_factor,rect.left+1000,rect.top+y_cosinus_factor+25};
    		DrawText(hDC, "���������� ���������(������� �i)", 34, &rectcf, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_unbalance_factor!=0){
    		RECT rectub = {rect.left+210, rect.top+y_unbalance_factor,rect.left+1000,rect.top+y_unbalance_factor+25};
    		DrawText(hDC, "���������� ���������(��������)", 34, &rectub, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_harm_factor!=0){

    		RECT rectih = {rect.left+210, rect.top+y_harm_factor,rect.left+1000,rect.top+y_harm_factor+25};
			DrawText(hDC, "���������� ���������(��������) ", 34, &rectih, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_motohours!=0){
    		RECT rectmh = {rect.left+210, rect.top+y_motohours,rect.left+1000,rect.top+y_motohours+25};
			DrawText(hDC, "����������                       ", 34, &rectmh, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_voltage!=0){
    		RECT rectu = {rect.left+210, rect.top+y_voltage,rect.left+1000,rect.top+y_voltage+25};
			DrawText(hDC, "�������                          ", 34, &rectu, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_frequency!=0){
    		RECT rectff = {rect.left+210, rect.top+y_frequency,rect.left+1000,rect.top+y_frequency+25};
			DrawText(hDC, "�������                          ", 34, &rectff, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    }



	    if(y_leakAmperage_A!=0){
			//			leakAmperage_A
						RECT rectla = {rect.left+210, rect.top+y_leakAmperage_A,rect.left+1000,rect.top+y_leakAmperage_A+25};
						DrawText(hDC, "�����, ���� �� �����, �", 25, &rectla, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_GrundAmperageDistortion!=0){

			//			GrundAmperageDistortion
						RECT rectad = {rect.left+210, rect.top+y_GrundAmperageDistortion,rect.left+1000,rect.top+y_GrundAmperageDistortion+25};
						DrawText(hDC, "���� ������, %", 16, &rectad, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_LoadType!=0){

						/* >LoadType - ��� ������������ - ����������/�������/������   */
						RECT recttl = {rect.left+210, rect.top+y_LoadType,rect.left+1000,rect.top+y_LoadType+25};
						DrawText(hDC, "��� ������������", 17, &recttl, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_PhaseRotation!=0){

						/*(uint16_t)(g->PhaseRotation)*/
						RECT rectak = {rect.left+210, rect.top+y_PhaseRotation,rect.left+1000,rect.top+y_PhaseRotation+25};
						DrawText(hDC, "����������� ���", 18, &rectak, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_cosinus_factor!=0){
						RECT rectcf1 = {rect.left+210, rect.top+y_cosinus_factor,rect.left+1000,rect.top+y_cosinus_factor+25};
						DrawText(hDC, "���������� ���������(������� ��), %", 37, &rectcf1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_unbalance_factor!=0){
						RECT rectub1 = {rect.left+210, rect.top+y_unbalance_factor,rect.left+839+140,rect.top+y_unbalance_factor+25};
						DrawText(hDC, "���������� ���������(��������), %", 36, &rectub1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_harm_factor!=0){
						RECT rectfh = {rect.left+210, rect.top+y_harm_factor,rect.left+839+140,rect.top+y_harm_factor+25};
						DrawText(hDC, "���������� ���������(��������), %", 36, &rectfh, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_ActivePower_kW!=0){
						RECT rectap = {rect.left+210, rect.top+y_ActivePower_kW,rect.left+839+140,rect.top+y_ActivePower_kW+25};
						DrawText(hDC, "��������� �������, ���", 24, &rectap, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_ReactivePower_kW!=0){
						/* ��������� ���������*/
						RECT rectrap = {rect.left+210, rect.top+y_ReactivePower_kW,rect.left+839+140,rect.top+y_ReactivePower_kW+25};
						DrawText(hDC, "��������� ���������, ���", 26, &rectrap, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_UnbalanceLostPower_kW!=0){
						/* ��������� �������� ��������������� �������� ������� ������ (���� ��� ��������������)*/
						RECT rectrap2 = {rect.left+210, rect.top+y_UnbalanceLostPower_kW,rect.left+839+140,rect.top+y_UnbalanceLostPower_kW+25};
						DrawText(hDC, "���������, ������ ����� ��������, ���", 40, &rectrap2, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_HarmonicLostPower_kW!=0){
						/* ���������, �������� ��������������� �������� ���������������� ������ */
						RECT rectrap3 = {rect.left+210, rect.top+y_HarmonicLostPower_kW,rect.left+839+140,rect.top+y_HarmonicLostPower_kW+25};

						DrawText(hDC, "���������, ������ ����� ��������, ���", 40, &rectrap3, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_power!=0){
						/*kvatt_t	ActivePower_kW*/
						RECT rectap1 = {rect.left+210, rect.top+y_power,rect.left+839+140,rect.top+y_power+25};
						DrawText(hDC, "��������� �������, ���", 24, &rectap1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_aTotalHarmonicDistortion!=0){

						/*������� ��������� ����������� ����� 					                    */
						/*���������� �������� ���������� �� ������ � ����� ���                      */
						//aTotalHarmonicDistortion
						RECT rectah = {rect.left+210, rect.top+y_aTotalHarmonicDistortion,rect.left+839+140,rect.top+y_aTotalHarmonicDistortion+25};
						DrawText(hDC, "���������� �������� ����������, ���� �, %", 44, &rectah, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_bTotalHarmonicDistortion!=0){
						//bTotalHarmonicDistortion
						RECT rectbh = {rect.left+210, rect.top+y_bTotalHarmonicDistortion,rect.left+839+140,rect.top+y_bTotalHarmonicDistortion+25};
						DrawText(hDC, "���������� �������� ����������, ���� �, %", 44, &rectbh, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_cTotalHarmonicDistortion!=0){
						//cTotalHarmonicDistortion
						RECT rectch = {rect.left+210, rect.top+y_cTotalHarmonicDistortion,rect.left+839+140,rect.top+y_cTotalHarmonicDistortion+25};
						DrawText(hDC, "���������� �������� ����������, ���� �, %", 44, &rectch, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_THDi_HarmonicAmperageDistortion!=0){
						//���������� �������� ���������� �� ������ ������������  (����)
			//			/*procentf_t THDi_HarmonicAmperageDistortion*/
						RECT rectth = {rect.left+210, rect.top+y_THDi_HarmonicAmperageDistortion,rect.left+839+140,rect.top+y_THDi_HarmonicAmperageDistortion+25};
						DrawText(hDC, "���������� �������� ����������, %", 36, &rectth, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_aNegativeAmperage_A!=0){
						RECT rectan = {rect.left+210, rect.top+y_aNegativeAmperage_A,rect.left+839+140,rect.top+y_aNegativeAmperage_A+25};
						DrawText(hDC, "������ ���������� �������, ���� �, �", 38, &rectan, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_bNegativeAmperage_A!=0){
			//			bNegativeAmperage_A*/
						RECT rectbn = {rect.left+210, rect.top+y_bNegativeAmperage_A,rect.left+839+140,rect.top+y_bNegativeAmperage_A+25};
						DrawText(hDC, "������ ���������� �������, ���� �, �", 38, &rectbn, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_cNegativeAmperage_A!=0){
			//			amperf_t cNegativeAmperage_A*/
						RECT rectcn = {rect.left+210, rect.top+y_cNegativeAmperage_A,rect.left+839+140,rect.top+y_cNegativeAmperage_A+25};
						DrawText(hDC, "������ ���������� �������, ���� �, �", 38, &rectcn, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_aNeutralAmperage_A!=0){
						RECT rectnaa = {rect.left+210, rect.top+y_aNeutralAmperage_A,rect.left+839+140,rect.top+ y_aNeutralAmperage_A+25};
						DrawText(hDC, "������ ����������� �������, ���� �, �", 38, &rectnaa, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_bNeutralAmperage_A!=0){
			//			bNeutralAmperage_A
						RECT rectnab = {rect.left+210, rect.top+ y_bNeutralAmperage_A,rect.left+839+140,rect.top+ y_bNeutralAmperage_A+25};
						DrawText(hDC, "������ ����������� �������, ���� �, �", 38, &rectnab, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_cNeutralAmperage_A!=0){
			//			cNeutralAmperage_A
						RECT rectnac = {rect.left+210, rect.top+ y_cNeutralAmperage_A,rect.left+839+140,rect.top+y_cNeutralAmperage_A+25};
						DrawText(hDC, "������ ����������� �������, ���� �, �", 38, &rectnac, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_sumNeutralAmperage_A!=0){
			//			sumNeutralAmperage_A
						RECT rectnat = {rect.left+210, rect.top+ y_sumNeutralAmperage_A,rect.left+839+140,rect.top+ y_sumNeutralAmperage_A+25};
						DrawText(hDC, "������ ����������� ������� �����, �", 37, &rectnat, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_AmperageUnbalance!=0){						            // 1234567890123456789012345678901234567890123456789
						RECT rectnkf = {rect.left+210, rect.top+ y_AmperageUnbalance,rect.left+839+140,rect.top+ y_AmperageUnbalance+25};
						DrawText(hDC, "������� ���, %", 15, &rectnkf, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_leakFurie!=0){
						RECT rectnlf = {rect.left+210, rect.top+ y_leakFurie,rect.left+839+140,rect.top+ y_leakFurie+25};
						            // 1234567890123456789012345678901234567890123456789
						DrawText(hDC, "���� ������ �� �����(�), �", 28, &rectnlf, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_leakFuriePhase!=0){
						RECT rectnlfp = {rect.left+210, rect.top+ y_leakFuriePhase,rect.left+839+140,rect.top+ y_leakFuriePhase+25};
						DrawText(hDC, "���� ������(�)", 15, &rectnlfp, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    /*************************************************************************************************
	     *
	     ************************************************************************************************/
		   /* ������ ������� ��������� ��� ������� �������-���� */
		   CreateWindowW(L"Button", L"", WS_CHILD | WS_VISIBLE| BS_GROUPBOX| WS_GROUP,
				   rect.left+x_left_panel,
				   rect.top+650,
				   150,
				   30,
				hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

	    	RECT rectblueHeating1 = {rect.left+20+101,rect.top+640,rect.left+170,rect.top+660};
	    	FillRect(hDC, &rectblueHeating1, (HBRUSH)26);

	    	RECT rectblueHeating2 = {rect.left+20+101,rect.top+640,rect.left+20+103,rect.top+700};
	    	FillRect(hDC, &rectblueHeating2, (HBRUSH)24);
	    	//RECT rect_testbrush ={rect.left+20,rect.top+660,rect.left+20+116,rect.top+680};

	    	RECT eHeating3 = {rect.left+20,rect.top+640,rect.left+190,rect.top+700};
	    	DrawText(hDC, "�����", 6, &eHeating3, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	RECT rectblueHeating3 = {rect.left+20+103,rect.top+680,rect.left+190,rect.top+700};
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

//1390
	    	  RECT rect1 = {rect.left+1010, rect.top+yDebugMode-20,rect.right,rect.top+25+yDebugMode};
	    	  DrawText(hDC, "���������?", 11, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  RECT rect2 = {rect.left+1010, rect.top+yAmperageAmplitude-20,rect.right,rect.top+25+yAmperageAmplitude};
	    	  DrawText(hDC, "�����", 6, &rect2, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  RECT rect3 = {rect.left+1010, rect.top+yBAmperageAmplitude-20,rect.right,rect.top+25+yBAmperageAmplitude};
	    	  DrawText(hDC, "����� B", 8, &rect3, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  RECT rect4 = {rect.left+1010, rect.top+yCosFi-20,rect.right,rect.top+25+yCosFi};
	    	  DrawText(hDC, "�������", 8, &rect4, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  RECT rect5 = {rect.left+1010, rect.top+yTk4screenMode-20,rect.right,rect.top+25+yTk4screenMode};
	    	  DrawText(hDC, "����� ��412", 12, &rect5, DT_SINGLELINE|DT_LEFT|DT_TOP);











	    	EndPaint(hwnd, &ps);

		}//end WM_PAINT
		break;
		/**************************************************************************************************************
	    *      SIZE 																						      SIZE
		**************************************************************************************************************/
		case WM_SIZE:{
			RECT rcClient;

			GetClientRect(hwnd, &rcClient);
			hEdit = GetDlgItem(hwnd, IDC_tk5MAIN_EDIT);
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
//			 ��������� �����. ���� If(idCtrl==ID_combo&&code==CBN_SELCHANGE)
			UINT NotificationCode = HIWORD(wParam);
			switch (NotificationCode) {
				/* ���� �������� ���� �����������*/
				case CBN_SELCHANGE:{
					/* ����� - ���� ���� ����� �� ������� � ���� �������� (��������� �� ���������� */
					int ItemIndex = SendMessage((HWND) lParam, (UINT) CB_GETCURSEL,  (WPARAM) 0, (LPARAM) 0);
					UINT idCtl=LOWORD(wParam);
					/* ���������� ��������� ���� � ��������� ����������*/
					/* ��.. ������� ������� �� ������ ���� :)*/
					// SendMessage((HWND)lParam, CB_SETCURSEL, idCtl, 0);
					/* � ��������� �� ��������� �������� ������ �������� ���������*/
					switch(idCtl){
						case  IDCB_YDEBUGMODE:
							DebugMode =ItemIndex;
						break;
//						case  IDCB_YTK4SCREENMODE:
//							Tk4screenMode=ItemIndex;
//						break;
						default:{}
					} // switch(idCtl)
				}//case CBN_SELCHANGE:
				break;
				default:{}
			}


			switch(wParam) {

			case IDB_tk5BtnSTART:{
				 if(tk5Process==ProcessBusy){
				   if(tk5btn_access==ENABLE){
					 tk5btn_access=DISABLE;
//					 /* ������� ����������� ��� ������ ������� */
					 f_Set5TxWriteReg(0x0000,0xFF,&mbTxMsg);
//					 /* ������� ����������� � ����� �� �������   */
					  f_set_tkqueue(
							&tk5Queue,
							DISABLE,
							hwnd,
							0x0300,
							&mbTxMsg,
							600
							);
					 snprintf(tk5LogStr,511,"��: ���� ��");
				   }else{snprintf (wmc,255," �� ���������� ��� ����� �� ������! ");}
				 }else{snprintf (wmc,255," ������ ������������� ��������� �� �� �������� ");}
			}
			break;
			case IDB_tk5BtnSTOP:{
				 if(tk5Process==ProcessBusy){
				  if(tk5btn_access==ENABLE){
					 tk5btn_access=DISABLE;
					 /* ������� ����������� ��� ������ ������� */
					 f_Set5TxWriteReg(0x0000,0x00,&mbTxMsg);
					 /* ������� ����������� � ����� �� �������   */
					 f_set_tkqueue(
							&tk5Queue,
							DISABLE,
							hwnd,
							0x0000,
							&mbTxMsg,
							600
							);

					 snprintf(tk5LogStr,511,"��: ���� ��");
				  }else{snprintf (wmc,255," �� ���������� ��� ����� �� ������! ");}
				 }else{snprintf (wmc,255," ������ ������������� ��������� �� �� �������� ");}

			}
			break;


			case IDM_tk5USER_SETTINGS:{
//				if(tk5Process==ProcessBusy){
				/* ����������� ��������� ������ ������� ���� �����������
				 * ���� ���� ����������� ��� ������� - ����� �� ������*/
				   if(!IsWindow(hWndtk5usCh)){
//					   /* ����� �� ������� � ����� ����������� -
//					   * �������� ��������� ���������� �������� ��������� */
//							/* ��������� ������ ����� ������������� DeskTop <-> tk5 */
							//KillTimer(hwnd, p1000Timer);
//							/* ������� ����� ������ */
							f_clear_tkqueue(&tk5Queue);
//					    /* ��������� ���� ����������� ����������� */
							hWndtk5usCh=f_CreateTk5usWnd(hwnd);
////						snprintf(tk5LogStr,511,"��: ���. ������������ �����������");
				   }//if(!IsWindow(usWndChild)){
//				}
//				else{
//					snprintf (wmc,255," ������ ������������� ��������� �� �� �������� ");
//				}

				}//case IDM_USER_SETTINGS:{
			break;
			case	IDM_tk5FACILITY_SETTING:{
				if(!IsWindow(hWndTk5fsCh)){
					//KillTimer(hwnd, p1000Timer);
//							/* ������� ����� ������ */
					f_clear_tkqueue(&tk5Queue);
//					    /* ��������� ���� ����������� ����������� */

					hWndTk5fsCh=f_CreateTk5fsWnd(hwnd);
				}
			}
			break;
			case IDB_tk5DIST_EXIT:{
				fprintf(fp_tk5LogFile,"\n\n\n\n");
				fclose(fp_tk5LogFile);
//				 if(!IsWindow(hWndtk5fsCh)){DestroyWindow(hWndtk5fsCh);}
//				 if(!IsWindow(hWndtk5usCh)){DestroyWindow(hWndtk5usCh);}
				DestroyWindow(hwnd);
			}
			break;
			case IDM_tk5QUIT:{
				fprintf(fp_tk5LogFile,"\n\n\n\n");
				fclose(fp_tk5LogFile);
//				 if(!IsWindow(hWndtk5fsCh)){DestroyWindow(hWndtk5fsCh);}
//				 if(!IsWindow(hWndtk5usCh)){DestroyWindow(hWndtk5usCh);}
				DestroyWindow(hwnd);
			}
			break;
			case IDM_tk5COMPORT_SETTING:
				if(!IsWindow(comWndChild)){
					f_CreateSWnd(hwnd);
					snprintf(tk5LogStr,511,"��: ������������ ���-�����");
				}
				break;
			case IDB_tk5TEST_DISTANCE_CTRL:{
				  /* ������� ������ ����� ������������� DeskTop <-> tk5 */
				  /* ������� ������� �����������*/
				tk5_Addr= f_GetTk5ModbussAddress(ID_tk5_EDIT_ADDRESS);
				modbus_master_tx_msg_t mb17={0};
				mb17.msg[0]=tk5_Addr;
				mb17.msg[1]=0x11;
				mb17.length=2;
//				p1000Timer=SetTimer(
//					hwnd,                // handle to main window
//					IDT_TIMER_tk5_1000MS,// timer identifier
//					5000,                // msecond interval
//									    (TIMERPROC) NULL     // no timer callback
//										);
//				/* ������� ����� */
				f_clear_tkqueue(&tk5Queue);
				/* ������ ������� ����������� � ����� ����� */
				f_set_tkqueue(
						&tk5Queue,
						DISABLE,
						hwnd,
						IDB_tk5TEST_DISTANCE_CTRL,
						&mb17,
						600
				);
				f_read_usersetting_from_tk5memory(hwnd);
				f_read_facilitysetting_from_tk5memory(hwnd);
//				/* ����������� ������ ��� ������� ����������� ��2 ����������� */
//				 RW5_case=IDB_tk5TEST_DISTANCE_CTRL;
//				 snprintf(tk5LogStr,511,"��: ���������� ������������� ��������� ");
//			  	  char str[256]={0};
//			  	  snprintf (str,sizeof(str)," a20_tk5.c: ���������� �������� �����. ������ � �����");
//			  	  f_tk5Logging(str, sizeof(str));
			}
			break;
			case IDB_tk5START_DISTANCE_CTRL:{
				  /* ��������� ������ ����� ������������� DeskTop <-> tk5 */
				  if(tk5Process==ProcessBusy){
					  /* ������� ����� */
					  f_clear_tkqueue(&tk5Queue);
				  }
				  else if(tk5Process==ProcessIddle){
			      /* ����������� ������ ���������� ������������� ��������� */
					  tk5Process=ProcessBusy;
//					  /* ��������� ������, �� ��������� ��������� � ���������� � ����� ����������� */
					  p1000Timer=SetTimer(
						hwnd,                // handle to main window
					    IDT_TIMER_tk5_1000MS,// timer identifier
						1000,                // msecond interval
					    (TIMERPROC) NULL     // no timer callback
						);
//
//				  /* ��� ����� �������-���� ������ ����� �� ���������� ����� */
//					/* ����� ������/�������/������� */
//					modbus_master_tx_msg_t mbTxMsg;
//					memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
//					/* ����� ��kc.������ */
////					f_Set112TxReadReg(RG_R_tk5_MAX_AMPERAGE,&mbTxMsg);
////					  f_set_tkqueue(
////							    &tk5Queue,
////							DISABLE,
////							hwnd,
////							ID_READ_US_MAX_AMPERAGE,
////							&mbTxMsg,
////							600
////							);
					snprintf(tk5LogStr,511,"��: ����� ������������� ��������� ");
				  }
				  else{}
//				  /* ��������� ����������� �������� ��������� ������ ������ */
				  f_TxRxBusyDegreeCalcStart();
//
			}
			break;
			case IDB_tk5STOP_DISTANCE_CTRL:{
				if(tk5Process==ProcessBusy){
//					/* ��������� ������ ����� ������������� DeskTop <-> tk5 */
					KillTimer(hwnd, p1000Timer);
					f_clear_tkqueue(&tk5Queue);
					snprintf(tk5LogStr,511,"��: ���� ������������� ���������\n ");
//
					tk5Process=ProcessIddle;
					char r[256]={0};
					/* ��������� ����������� �������� ��������� ������ ������ */
					f_TxRxBusyDegreeCalcStop(r, sizeof(r));
					snprintf(tk5LogStr,511,"��: ���� ������������� ���������");
					strcat(tk5LogStr,r);
					f_tk5Logging(tk5LogStr, sizeof(tk5LogStr));
					snprintf(tk5LogStr,511,"���������� ������ �� ��� ������������� ���������");
					f_tk5Logging(tk5LogStr, sizeof(tk5LogStr));
					snprintf(tk5LogStr,511,"%d.%2d %% - ������� ��������� ������ ",
						(int)(gTxRxBusyAveDegreef*100.0f)/100,
						(int)(gTxRxBusyAveDegreef*100.0f)%100);
					f_tk5Logging(tk5LogStr, sizeof(tk5LogStr));

					snprintf(tk5LogStr,511,"%d - ʳ������ ��������� ����������",
						(int)gTxRxSessionStartCntr);
					f_tk5Logging(tk5LogStr, sizeof(tk5LogStr));

					snprintf(tk5LogStr,511,"%d.%2d %% - C��� ����������� ��� �������",
						(int)(gTxRxSessionAVESuccessDegreef*100.0f)/100,
						(int)(gTxRxSessionAVESuccessDegreef*100.0f)%100);
					f_tk5Logging(tk5LogStr, sizeof(tk5LogStr));

				}

			}
			break;
			case IDB_tk5RESET_MOTOR_PROTECTION_1:{
				if(tk5Process==ProcessBusy){
				 if(tk5btn_access==ENABLE){
					 tk5btn_access=DISABLE;
//					/* ������� ����������� ��� ������ ������� */
//					f_Set112TxWriteReg(RG_W_RESET_MOTOR_PROTECTION_1,RG_V_RESET_MOTOR_PROTECTION,&mbTxMsg);
//					/* ������� ����������� � ����� �� �������   */
//					f_set_tkqueue(
//						&tk5Queue,
//						ENABLE,
//						hwnd,
//						IDB_RESET_MOTOR_PROTECTION_1,
//						&mbTxMsg,
//						600
//						);
					snprintf(tk5LogStr,511,"��: �������� ����  ");
				 }else{snprintf (wmc,255," �� ���������� ��� ����� �� ������! ");}
				}else{snprintf (wmc,255," ������ ������������� ��������� �� �� �������� ");}
		    }
		    break;
			case IDB_tk5RESET_MOTOR_PROTECTION_2:{
				if(tk5Process==ProcessBusy){
				 if(tk5btn_access==ENABLE){
					 tk5btn_access=DISABLE;
//					/* ������� ����������� ��� ������ ������� */
//					f_Set112TxWriteReg(RG_W_RESET_MOTOR_PROTECTION_2,RG_V_RESET_MOTOR_PROTECTION,&mbTxMsg);
//					/* ������� ����������� � ����� �� �������   */
//					f_set_tkqueue(
//						&tk5Queue,
//						ENABLE,
//						hwnd,
//						IDB_tk5RESET_MOTOR_PROTECTION_2,
//						&mbTxMsg,
//						600
//						);
					snprintf(tk5LogStr,511,"��: �������� �������� �������� ����  ");
				 }else{snprintf (wmc,255," �� ���������� ��� ����� �� ������! ");}
				}else{snprintf (wmc,255," ������ ������������� ��������� �� �� �������� ");}
		    }
		    break;
			case IDB_YDEBUGMODE:{
				/* ������� ����������� ��� ������ ������� */
				modbus_master_tx_msg_t mbTxMsg={0};
				f_Set5TxWriteReg(0xE001,DebugMode,&mbTxMsg);
				/* ������� ����������� � ����� �� �������   */
				f_set_tkqueue(
						&tk5Queue,
						DISABLE,
						hwnd,
						0xE001,
						&mbTxMsg,
						1000
						);
				/* ������� ������� �����������*/
				tk5_Addr= f_GetTk5ModbussAddress(ID_tk5_EDIT_ADDRESS);
				modbus_master_tx_msg_t mb17={0};
				mb17.msg[0]=tk5_Addr;
				mb17.msg[1]=0x03;
				mb17.msg[2]=0xE0;
				mb17.msg[3]=0x00;
				mb17.msg[4]=0x00;
				mb17.msg[5]=0x05;
				mb17.length=6;
				f_set_tkqueue(
						&tk5Queue,
						DISABLE,
						hwnd,
						0xEEEE,
						&mb17,
						1000
						);

			}
			break;
			case IDB_YHAMPERAGEAMPLITUDE:{
				if (DebugMode != 0){
					/* ������� �������� ������, �� , �� ������ HEX*/
					float2_t wItk5={0};
					int errWM_C=f_checkEdit(IDE_YHAMPERAGEAMPLITUDE, hwnd, &wItk5);
					if(errWM_C==0){
						uint16_t value = wItk5.d;

						/* ������� ����������� ��� ������ ������� */
						modbus_master_tx_msg_t mbTxMsg={0};
						f_Set5TxWriteReg(0xE002,value,&mbTxMsg);
						/* ������� ����������� � ����� �� �������   */

						f_set_tkqueue(
							&tk5Queue,
							DISABLE,
							hwnd,
							0xE002,
							&mbTxMsg,
							1000
							);
					}
				}
				else{
					SetWindowText(hAmperageAmplitude,"100");
				}
			}
			break;
			case IDB_YBAMPERAGEAMPLITUDE:{
				if (DebugMode != 0){
					/* ������� �������� ������, �� , �� ������ HEX*/
					float2_t wItk5={0};
					int errWM_C=f_checkEdit(IDE_YBAMPERAGEAMPLITUDE, hwnd, &wItk5);
					if(errWM_C==0){
						uint16_t value = wItk5.d;

						/* ������� ����������� ��� ������ ������� */
						modbus_master_tx_msg_t mbTxMsg={0};
						f_Set5TxWriteReg(0xE003,value,&mbTxMsg);
						/* ������� ����������� � ����� �� �������   */

						f_set_tkqueue(
								&tk5Queue,
								DISABLE,
								hwnd,
								0xE003,
								&mbTxMsg,
								1000
							);
					}
				}
				else{
					SetWindowText(hBAmperageAmplitude,"100");
				}
			}
			break;
			case IDB_YCOSFI:{
				if (DebugMode != 0){
					/* ������� �������� ������, �� , �� ������ HEX*/
					float2_t wItk5={0};
					int errWM_C=f_checkEdit(IDE_YCOSFI, hwnd, &wItk5);
					if(errWM_C==0){
						uint16_t value = wItk5.d;
						if(value>100){value=100;}
						/* ������� ����������� ��� ������ ������� */
						modbus_master_tx_msg_t mbTxMsg={0};
						f_Set5TxWriteReg(0xE004,value,&mbTxMsg);
						/* ������� ����������� � ����� �� �������   */

						f_set_tkqueue(
								&tk5Queue,
								DISABLE,
								hwnd,
								0xE004,
								&mbTxMsg,
								1000
						);
					}
				}
				else{
					SetWindowText(hCosFi,"100");
				}
			}
			break;
			case IDB_YTK4SCREENMODE:{
				/* ������� ����������� ��� ������ ������� */
				modbus_master_tx_msg_t mbTxMsg={0};
				f_Set5TxWriteReg(0xE000,Tk4screenMode,&mbTxMsg);
				/* ������� ����������� � ����� �� �������   */
				f_set_tkqueue(
						&tk5Queue,
						DISABLE,
						hwnd,
						0xE000,
						&mbTxMsg,
						1000
						);


			}
			break;
		    /***************************************************************************/
			/* ������������ ���������� ������������ ������ -����������               */
		    /***************************************************************************/
			case IDB_tk5SHOW_MOTOR_OPERATING:
				  /* ������ �� ���������� ����������, ��� ���� ���������, ������������*/
//		    	  switch(g5.d_MotorState){
//		    	  case oFF_:SendMessage(hWndViewBtnMotorState, BM_SETCHECK, 0, 0L);break;
//		    	  case oN_: SendMessage(hWndViewBtnMotorState, BM_SETCHECK, 1, 0L);break;
//		    	  default:{}
//		    	  }
			break;
			case IDB_tk5SHOW_EMP_AUTOCTR1:
				 /* ������ �� ���������� ����������, ��� ���� ���������, ������������*/
//		    	  switch(g5.d_AutoCtrl){
//		    	  case oFF_:SendMessage(hBtnViewAutoctrl, BM_SETCHECK, 0, 0L);break;
//		    	  case oN_: SendMessage(hBtnViewAutoctrl, BM_SETCHECK, 1, 0L);break;
//		    	  default:{}
//		    	  }
		   break;
	  	   case IDB_tk5SHOW_CX:
	  		    /* ������ �� ���������� ����������, ��� ���� ���������, ������������*/
//	  	    	  switch(g5.d_CX){
//	  	    	  case oFF_:SendMessage(hBtnViewCX, BM_SETCHECK, 0, 0L);break;
//	  	    	  case oN_ : SendMessage(hBtnViewCX, BM_SETCHECK, 1, 0L);break;
//	  	    	  default:{}
//	  	    	  }
	  	   break;
		   default:{}
		   }/* end switch param*/
			/* Error Message Box */
		   if(strlen(wmc) != 0){ MessageBox(hwnd, wmc, TEXT("����� ! "), MB_ICONERROR | MB_OK);}
		}/*WM_COMMAND:*/
		break;
		case WM_CLOSE:{
			 fprintf(fp_tk5LogFile,"\n\n\n\n");
			 fclose(fp_tk5LogFile);
//			 if(!IsWindow(hWndtk5fsCh)){DestroyWindow(hWndtk5fsCh);}
//			 if(!IsWindow(hWndtk5usCh)){DestroyWindow(hWndtk5usCh);}
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
			  case IDT_TIMER_tk5_1000MS:{
				  tk5TimerCntr++;
				  modbus_master_tx_msg_t mbTxMsg={0};
				  /* ������� ������� �����������*/
				  tk5_Addr= f_GetTk5ModbussAddress(ID_tk5_EDIT_ADDRESS);
				  mbTxMsg.msg[0]=tk5_Addr;
				  mbTxMsg.msg[1]=0x03;
				  mbTxMsg.msg[2]=0x01;
				  mbTxMsg.msg[3]=0x00;
				  mbTxMsg.msg[4]=0x00;
				  mbTxMsg.msg[5]=0x40;
				  mbTxMsg.length=6;
				  /* ������� ����������� � ����� �� �������   */
				  RW5_case=0x0100;
				  f_set_tkqueue(
						  &tk5Queue,
						  DISABLE,
						  hwnd,
						  RW5_case,
						  &mbTxMsg,
						  3000
				  	  );
//				  tk5Process=ProcessIddle;
//				  KillTimer(hwnd, p1000Timer);
//
//				  uint16_t iQ=f_get_tkrs232_length(&tk5Queue);
//				  if (iQ>2)period++;
//				  else{period--;}
//				  if(period<10){period=10;}
//				  p1000Timer=SetTimer(
//							hwnd,                // handle to main window
//							IDT_TIMER_tk5_1000MS,  // timer identifier
//							period,                  // msecond interval
//							(TIMERPROC) NULL     // no timer callback
//				  );
//					snprintf(tk5LogStr,511,"Period %d",period);
//					f_tk5Logging(tk5LogStr, sizeof(tk5LogStr));
			  }//IDT_TIMER_tk5_1000MS:{
			  break;
			  /* ���������� ������ ������ ���� ���������� ������������ ����� ������
			   * � ��������� �� ������� � �������� */
			  case IDT_TIMER_tk5_1333MS:{
				  /* ���� � �������� �� ���� ������� - ���� ������..*/
				  /* ���� ������� ������ ��������� ���������� ������� �������� 10 ��� - ��������� � ����������� */
				  if(met_zero_MBsuccess == EVENT){
					  if(gTxRxSessionSuccessDegreef<10.0f){
						tk5ProcessState=0;
						/* ���� ����.��������� ��� ���� ��������, ������� ������,
						*  �� ����� ����������� ���������� �����������,
						*  ����� ��������� �������� ������ �� ��5   */
						if(tk5Process==ProcessBusy){
							KillTimer(hwnd, p1000Timer);
							KillTimer(hwnd, p1333Timer);
							f_clear_tkqueue(&tk5Queue);
							snprintf(tk5LogStr,511,"��5 �� �������\n "
								"����������� ��������� �������� ");

							tk5Process=ProcessIddle;
						}
						const int result = MessageBox(NULL, "��5 �� �������\n "
								"����������� ��������� ��������", "�����!",MB_ICONEXCLAMATION | MB_OK);
						switch (result){
						case IDOK:
							/* ³��������� ������ ����� */
							if(tk5Process==ProcessBusy){
								p1000Timer=SetTimer(
										hwnd,                // handle to main window
										IDT_TIMER_tk5_1000MS,  // timer identifier
										1000,                  // msecond interval
										(TIMERPROC) NULL     // no timer callback
								);
								p1333Timer=SetTimer(
										hwnd,                // handle to main window
										IDT_TIMER_tk5_1333MS,  // timer identifier
										1333,                  // msecond interval
										(TIMERPROC) NULL     // no timer callback
										);

							}
						break;
						default:{}
						}

					 /* ���� �� �������� ����������� ��� ������� � ����-��������,
					  * ��� ���� ���������� ���� ����� � ����� ����   */
						memcpy(&tk5LogStr, "��5 �� �������", 256);
				   }// if (gTxRxSessionSuccessDegreef<10.0f){
				   met_zero_MBsuccess = NO_EVENT;
				  }
			  	KillTimer(hwnd, p1333Timer);
			  }
			  break;
			  case IDT_TIMER_tk5_TEST:{
				  /* ����������� ������ ��� ������� ����������� ��5 ����������� */
//				  if( RW5_case==IDB_tk5TEST_DISTANCE_CTRL){
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
					  KillTimer(hwnd, pTestTimer);
				  	  char str[256]={0};
				  	  snprintf (str,sizeof(str)," a20_tk5.c: ������ ������� ����������� ");
				  	  f_tk5Logging(str, sizeof(str));
//				 }
			  }
			  break;
			  case IDT_TIMER_tk5_10MS:{
				  /* ���� � ��� ��������� ������ - ������� �������� �������� */
				  if((tk5Process==ProcessBusy)&&(tk5btn_access==DISABLE)){
					  tk5btnSleepCntr+=TIMER_MAIN_MS;
					  if(tk5btnSleepCntr>=tk5_BTN_BLOKING_MS){
						  tk5btn_access=ENABLE;
						  tk5btnSleepCntr=0;
					  }
				  }
				  f_tk5UpdateQueueProcessBar(
				  		hwnd,
						hWndProgressBar_Queue,
						p1000Timer,
						IDT_TIMER_tk5_1000MS,
				  		p10Timer,
						IDT_TIMER_tk5_10MS
				  		);

			  }
			  break;
			  default:{}

		}  //switch ((UINT)wParam){
		break; //case WM_TIMER;
		}//case WM_TIMER:{
		case VK_PSW:
			break;
		case VK_UPDATE_WINDOW:{
			int responce_status=f_tk5QPWD_RgAnswer(
					RxMasterModbusStatus,
					&mOut_tx_msg,
					&mIn_rx_msg,
					wParam
			);
			/* * WPARAM wParam - RW_case, ������������ ���� ����������� ����������� */
			f_tk5UpdateWindow(hwnd,wParam,responce_status);
		}
		break;
		default:{
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}
	return 0;
}


extern uint16_t grid_m16[0x40];
static int test_answer_cntr=0;
uint16_t AmperageAmplitude	=0;
uint16_t BAmperageAmplitude	=0;
uint16_t CosFi	=0;
static uint16_t PhasnostOld=0xFFFF;
static uint16_t LoadModeOld=0xFFFF;
static int f_tk5UpdateValuesNames(HWND hwnd);

/* �� ������� ����������� ����� ��� ��� �������� ����������� � VK_UPDATE_WINDOW
 * ��� ���������� ��� ������, ���������� � ��������� ����*/
static int f_tk5UpdateWindow(HWND hwnd, int wParam, int responce_status){
	uint16_t x_value = 560;
//   /***********************************************************************************************************
//   * ������ � ������ ��²������� ������
//   ***********************************************************************************************************/
	f_tk5UpdateStat(
			hwnd,
			hWndProgressBar_Queue,
			p1000Timer,
			IDT_TIMER_tk5_1000MS
			);
	/***********************************************************************************************************
	 * ���� ������� ��²��������
	 ***********************************************************************************************************/
    /* ��� ������� ������ ����������, �� �� ������������ �������,
     * ���� �� 10 ��� �������� �� �������� - �������� ����������� */
	if(tk5Process==ProcessBusy){
		if (gTxRxSessionSuccessDegreef<10.0f){
		  /* ��������� ������, ��� ������ ������� ����������� ����� ���� ������ */
		  if(met_zero_MBsuccess == NO_EVENT){
			  p1333Timer=SetTimer(
					  hwnd,                // handle to main window
					  IDT_TIMER_tk5_1333MS,// timer identifier
					  12000,        	   // msecond interval
					  (TIMERPROC) NULL     // no timer callback
			  );
			  met_zero_MBsuccess = EVENT;
		  }
		}
	}
    if((wParam==IDB_tk5READ_USERSETTING_FROM_MEMORY)||
   		 (wParam==IDB_tk5READ_FACILITYSETTING_FROM_MEMORY)) {
   	 	 /* ���� ��������� ������� ��� ������/�� ������*/
   		if(((uint16_t)tk5fs.Faznost!=PhasnostOld)||((uint16_t)Tk5us.oper_mode!=LoadModeOld)){
   			Init_YMode(
					(uint16_t)tk5fs.Faznost,
					(uint16_t)Tk5us.oper_mode);
   			/* ������� ����� - ����� � �������� ���������
   			 * �������� ��� ����� ��������� */
   			f_tk5UpdateValuesNames(hwnd);
   			  /* ���������� ����� ��� ��������� */

		    	PhasnostOld=(uint16_t)tk5fs.Faznost;
		    	LoadModeOld=(uint16_t)Tk5us.oper_mode;
   		}
    }

//	char pm[80]={0};
	/* ����� �������� ��� ��������� ������ � ����*/
	HDC hDCr; // ������ ���������� ���������� ������ �� ������
	RECT rect; // ���-��, ������������ ������ ���������� �������
	/*������� ������*/
	hDCr= GetDC(hwnd);
	GetClientRect(hwnd, &rect);  	// �������� ������ � ������ ������� ��� ���������
	/***********************************************************************************************************
	 * ���� ����� ��²�������� ��� ��������� ²���
	 ***********************************************************************************************************/
//	if (tk5ProcessState==0){
//		/* ���� �� ��� ����� ����� - ����� �� ������ */
//		return 0;
//	}



//	if(tk5ProcessState==-1){
		if(wParam==IDB_tk5TEST_DISTANCE_CTRL){
			char str[256]={0};
			snprintf (str,sizeof(str)," a20_tk5.c: � ��������� ����� �������� ������� ��� ������� ��������� ������");
			f_tk5Logging(str, sizeof(str));
		}
		/* �� �����������, ��� ����������� ��� �������������� ���-���� �������.*/
		/* ���� ������ ��� �����������, ��� ������ ��� ����� */
		/* ��� ������� ������ ����������� ������� ���������� ����� ������    */
		/* ���� �������� ����� ������ �������  - �������� ������� �����������
		 * �� ���������� ������������ ����� */
		if(strlen(user_msg5)!=0){
			tk5ProcessState=0;
//			{
				/* ���� ����.��������� ��� ���� ��������, ������� ������,
				 * ����� ��������� �������� ������ �� ��2   */
//				if(tk5Process==ProcessBusy){KillTimer(hwnd, p1000Timer);}
//				const int result = MessageBox(NULL, user_msg5, "�����!",MB_ICONEXCLAMATION | MB_OK);
//				switch (result){
//				case IDOK:
//				/* ³��������� ������ ����� */
//					if(tk5Process==ProcessBusy){
//						p1000Timer=SetTimer(
//							hwnd,                // handle to main window
//							IDT_TIMER_tk5_1000MS,  // timer identifier
//							1000,                  // msecond interval
//							(TIMERPROC) NULL     // no timer callback
//						);
//					}
//					break;
//				default:{}
//				}

				/* ���� �� �������� ����������� ��� ������� � ����-��������,
				 * ��� ���� ���������� ���� ����� � ����� ����   */
				memcpy(&tk5LogStr, &user_msg5, sizeof(tk5LogStr));
				/* ������� ����� ����������� ��� ������� �����������*/
//				memset(&user_msg5,0,sizeof(user_msg5));
		}//if(strlen(user_msg112)!=0){

//	}
	/***********************************************************************************************************
	 * ���� �� ���������� ������ � ��2
	 ***********************************************************************************************************/
//	else if(tk5ProcessState==2){
		if(wParam==IDB_tk5TEST_DISTANCE_CTRL){
		tk5ProcessState=0;
		 if(strlen(user_msg5) !=0){
			/* ���� �� ���� ���������� - �������� ������� �����������*/
			RECT rectht = {rect.left+839, rect.top+25,rect.left+839+140,rect.top+50};
			DrawText(hDCr, user_msg5, 19, &rectht, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&user_msg5,0,sizeof(user_msg5));
			/* ��������� ������, ��� ������ ������� ����������� ����� ���� ������ */
			pTestTimer=SetTimer(
					hwnd,                // handle to main window
					IDT_TIMER_tk5_TEST,// timer identifier
					TIMER_AUX_MS,        // msecond interval
					(TIMERPROC) NULL     // no timer callback
			);
			test_answer_cntr++;
			char str[256]={0};
				snprintf (str,sizeof(str),"a20_tk5.c: ������� �� ������� %d �� ��5. �������� 'tk5', �������� ������", test_answer_cntr);
			f_tk5Logging(str, sizeof(str));
		}
	  }
//	}
	/***********************************************************************************************************
	 * ���� �� ������ֲ��� �����˲���
	 ***********************************************************************************************************/
//	else if(tk5ProcessState==1){
		tk5ProcessState=0;
		switch(wParam){
		case 0x100:{







			char str[128]={0};
			//m[0x3A]=dc(g->MotorHeatProcentage);	m[0x3B]=fr(g->MotorHeatProcentage);

	   if(y_frequency != 0){
//			grid_m16[0]	/*0*/ 						grid_m16[1]/*1*/
			RECT rectfc = {rect.left+x_value, rect.top+y_frequency,rect.left+839+140,rect.top+y_frequency+25};
			snprintf (str,sizeof(str),"%4d.%04d",
			grid_m16[2],	/*dc(g->Frequency_Hz)*/		grid_m16[3]/*fr(g->Frequency_Hz)*/
			);
			DrawText(hDCr, str, 19, &rectfc, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_voltage != 0){
			/* �������� ������� � �������*/
			RECT rectvl = {rect.left+x_value, rect.top+y_voltage,rect.left+839+140,rect.top+y_voltage+25};
			snprintf (str,sizeof(str),"%4d.%01d",
			grid_m16[4],	/*dc(g->Voltage_V)*/		grid_m16[5]/1000  /*fr(g->Voltage_V)*/
			);
			DrawText(hDCr, str, 19, &rectvl, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_aAmperage != 0){
			/* ������� ������ � ������� */
			RECT rectia = {rect.left+x_value, rect.top+y_aAmperage,rect.left+839+140,rect.top+y_aAmperage+25};
			snprintf (str,sizeof(str),"%4d.%02d",
			grid_m16[6],	/*dc(g->aAmperage_A)*/		grid_m16[7]/100/*fr(g->aAmperage_A)*/
			);
			DrawText(hDCr, str, 19, &rectia, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_bAmperage != 0){
			RECT rectib = {rect.left+x_value, rect.top+y_bAmperage,rect.left+839+140,rect.top+y_bAmperage+25};
			snprintf (str,sizeof(str),"%4d.%02d",
			grid_m16[8],	/*dc(g->bAmperage_A)*/		grid_m16[9]/100/*fr(g->bAmperage_A)*/
			);
			DrawText(hDCr, str, 19, &rectib, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_cAmperage != 0){
			RECT rectic = {rect.left+x_value, rect.top+y_cAmperage,rect.left+839+140,rect.top+y_cAmperage+25};
			snprintf (str,sizeof(str),"%4d.%02d",
			grid_m16[0xA],	/*dc(g->cAmperage_A)*/		grid_m16[0xB]/100 /*fr(g->cAmperage_A)*/
			);
			DrawText(hDCr, str, 19, &rectic, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));

	   }
//		m[0x3E]=g->mPOPUGAYS;
//		m[0x3F]=g->kPOPUGAYS;



	   if(y_leakAmperage_A != 0){

//			leakAmperage_A
			RECT rectla = {rect.left+x_value, rect.top+y_leakAmperage_A,rect.left+839+140,rect.top+y_leakAmperage_A+25};
			snprintf (str,sizeof(str),"%4d.%03d",
			grid_m16[0xC],	/*dc(g->leakAmperage_A)*/	grid_m16[0xD]/10/*fr(g->leakAmperage_A)*/
			);
			DrawText(hDCr, str, 19, &rectla, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_GrundAmperageDistortion != 0){
//			GrundAmperageDistortion
			RECT rectad = {rect.left+x_value, rect.top+y_GrundAmperageDistortion,rect.left+839+140,rect.top+y_GrundAmperageDistortion+25};
			snprintf (str,sizeof(str),"%4d.%02d",
			/* ������� ������, �� ����� �� �����
		 	 * ����� ��������� �� �����, ���������� �� ����� ����� ������,
		 	 * �� ���� �� leakAmperage_A, ��� � ��������� �� ����������� ������*/
			grid_m16[0xE],/*dc(g->GrundAmperageDistortion)*/	grid_m16[0xF]/100/*fr(g->GrundAmperageDistortion)*/
			);
			DrawText(hDCr, str, 19, &rectad, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_LoadType != 0){
			/* >LoadType - ��� ������������ - ����������/�������/������*/
			RECT recttl = {rect.left+x_value, rect.top+y_LoadType,rect.left+839+140,rect.top+y_LoadType+25};
			if(grid_m16[0x10]==2){
				snprintf (str,sizeof(str),"����������");
			}
			else if(grid_m16[0x10]==3){
				snprintf (str,sizeof(str),"������");
			}
			else if(grid_m16[0x10]==1){
				snprintf (str,sizeof(str),"�������");
			}
			else{}
			DrawText(hDCr, str, 19, &recttl, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_PhaseRotation != 0){
			/*(uint16_t)(g->PhaseRotation)*/
			RECT rectak = {rect.left+x_value, rect.top+y_PhaseRotation,rect.left+839+140,rect.top+y_PhaseRotation+25};
			if(grid_m16[0x11]==1){
				snprintf (str,sizeof(str),"������");
			}
			else if(grid_m16[0x11]==2){
				snprintf (str,sizeof(str),"���������");
			}
			else if(grid_m16[0x11]==3){
				snprintf (str,sizeof(str),"����� ���");
			}
			else if(grid_m16[0x11]==4){
				snprintf (str,sizeof(str),"����� ���� �");
			}
			else if(grid_m16[0x11]==5){
				snprintf (str,sizeof(str),"����� ���� �");
			}
			else{}
			DrawText(hDCr, str, 19, &rectak, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_cosinus_factor != 0){


			/* ������� ��*/
			RECT rectcf = {rect.left+x_value, rect.top+y_cosinus_factor,rect.left+839+140,rect.top+y_cosinus_factor+25};


			snprintf (str,sizeof(str),"%3d.%04d",
			grid_m16[0x12]/*dc(g->PowerFactor_pro)*/,grid_m16[0x16]
			);
			DrawText(hDCr, str, 19, &rectcf, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_unbalance_factor != 0){
			/* ҳ���� ��� ��������������, ���� ������������ �� ������ - �� �� �����
			NEMA (National Electric Manufacturers Associations of the
			USA) Std. (1993) - ���������� ����������� �������:
			�������� ��������� �� ���������� ��������
			https://wp.kntu.ac.ir/tavakoli/pdf/Journal/j.ijepes.2010.12.003.pdf
			������ ��������� - ����� ������� ������� �������              */
			RECT rectub = {rect.left+x_value, rect.top+y_unbalance_factor,rect.left+839+140,rect.top+y_unbalance_factor+25};
			snprintf (str,sizeof(str),"%3d",
			grid_m16[0x13]/*dc(g->UnbalanceDeratingFactor_pro)*/
			);
			DrawText(hDCr, str, 19, &rectub, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_harm_factor != 0){
		    /* ���������� �����������, ���� 99%, �� ���� ������� ����������� �������� ����� ��������
		     * ����� �� ���� ���� ��� ��������������  */
			RECT rectfh = {rect.left+x_value, rect.top+y_harm_factor,rect.left+839+140,rect.top+y_harm_factor+25};
			snprintf (str,sizeof(str),"%3d.%04d",
			grid_m16[0x14],grid_m16[0x15]/*dc(g->HarmonicsDeratingFactor_pro)*/
			);
			DrawText(hDCr, str, 19, &rectfh, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_AmperageUnbalance != 0){
			/* ������� ���������� ��������� ����������� �����, ����� ���� ��� �������  */



//			grid_m16[0x16]/*dc(g->fullDerating_pro)*/
			//y_AmperageUnbalance
//			/* ����i����� �������� ���,  �� ���� �� ���� ��� ��������������	*/
			RECT rectaub = {rect.left+x_value, rect.top+y_AmperageUnbalance,rect.left+839+140,rect.top+y_AmperageUnbalance+25};
			snprintf (str,sizeof(str),"%3d", grid_m16[0x17]);
			DrawText(hDCr, str, 19, &rectaub, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_ActivePower_kW != 0){
			/********************************************************************************************
			 * ��������� ��������Ҳ
			 * ******************************************************************************************/
			/* ������� ���������*/
			RECT rectap = {rect.left+x_value, rect.top+y_ActivePower_kW,rect.left+839+140,rect.top+y_ActivePower_kW+25};
			snprintf (str,sizeof(str),"%4d.%02d",
			grid_m16[0x18],/*dc(g->ActivePower_kW)*/	grid_m16[0x19]/100/*fr(g->ActivePower_kW)*/
			);
			DrawText(hDCr, str, 19, &rectap, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_ReactivePower_kW != 0){
			/* ��������� ���������*/
			RECT rectrap = {rect.left+x_value, rect.top+y_ReactivePower_kW,rect.left+839+140,rect.top+y_ReactivePower_kW+25};
			snprintf (str,sizeof(str),"%4d.%02d",
			grid_m16[0x1A],	grid_m16[0x1B]/100);
			DrawText(hDCr, str, 19, &rectrap, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_UnbalanceLostPower_kW != 0){

			/* ��������� �������� ��������������� �������� ������� ������ (���� ��� ��������������)*/
			RECT rectrap2 = {rect.left+x_value, rect.top+y_UnbalanceLostPower_kW,rect.left+839+140,rect.top+y_UnbalanceLostPower_kW+25};
			snprintf (str,sizeof(str),"%4d.%02d",
			grid_m16[0x1C], grid_m16[0x1D]/100);
			DrawText(hDCr, str, 19, &rectrap2, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_HarmonicLostPower_kW != 0){
			/* ���������, �������� ��������������� �������� ���������������� ������ */
			RECT rectrap3 = {rect.left+x_value, rect.top+y_HarmonicLostPower_kW,rect.left+839+140,rect.top+y_HarmonicLostPower_kW+25};
			snprintf (str,sizeof(str),"%4d.%02d",
			grid_m16[0x1E], grid_m16[0x1F]/100);
			DrawText(hDCr, str, 19, &rectrap3, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_power != 0){

			/**********************************************************************************************/




			/*kvatt_t	ActivePower_kW*/
			RECT rectap1 = {rect.left+x_value, rect.top+y_power,rect.left+839+140,rect.top+y_power+25};
			snprintf (str,sizeof(str),"%4d.%02d",
			grid_m16[0x18],/*dc(g->ActivePower_kW)*/	grid_m16[0x19]/100/*fr(g->ActivePower_kW)*/
			);
			DrawText(hDCr, str, 19, &rectap1, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str)						);
//
//			/**********************************************************************************************/
//			/*************************************************************************/
	   }
	   if(y_aTotalHarmonicDistortion != 0){

			/*������� ��������� ����������� ����� 					                    */
			/*���������� �������� ���������� �� ������ � ����� ���                      */
			//aTotalHarmonicDistortion
			RECT rectah = {rect.left+x_value, rect.top+y_aTotalHarmonicDistortion,rect.left+839+140,rect.top+y_aTotalHarmonicDistortion+25};
			snprintf (str,sizeof(str),"%2d.%02d",
			grid_m16[0x20],grid_m16[0x21]/100);
			DrawText(hDCr, str, 19, &rectah, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_bTotalHarmonicDistortion != 0){
			//bTotalHarmonicDistortion
			RECT rectbh = {rect.left+x_value, rect.top+y_bTotalHarmonicDistortion,rect.left+839+140,rect.top+y_bTotalHarmonicDistortion+25};
			snprintf (str,sizeof(str),"%2d.%02d",
			grid_m16[0x22],grid_m16[0x23]/100);
			DrawText(hDCr, str, 19, &rectbh, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_cTotalHarmonicDistortion != 0){
			//cTotalHarmonicDistortion
			RECT rectch = {rect.left+x_value, rect.top+y_cTotalHarmonicDistortion,rect.left+839+140,rect.top+y_cTotalHarmonicDistortion+25};
			snprintf (str,sizeof(str),"%2d.%02d",
			grid_m16[0x24],grid_m16[0x25]/100);
			DrawText(hDCr, str, 19, &rectch, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_THDi_HarmonicAmperageDistortion != 0){
			//���������� �������� ���������� �� ������ ������������  (����)
//			/*procentf_t THDi_HarmonicAmperageDistortion*/
			RECT rectth = {rect.left+x_value, rect.top+y_THDi_HarmonicAmperageDistortion,rect.left+839+140,rect.top+y_THDi_HarmonicAmperageDistortion+25};
			snprintf (str,sizeof(str),"%2d.%02d",
			grid_m16[0x26],grid_m16[0x27]/100);
			DrawText(hDCr, str, 19, &rectth, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_aNegativeAmperage_A != 0){
//			/*THD� ����� �� ������ ����������� ����� �������� ������.
//			THDi ����� 10% ��������� ����������,
//			����� ������������� ���������� �������, ����������� �� 6%.
//
//			THDi �� 10% � 50% ����� �� ������ ��������� �����������.
//			������ ����������� ��������� �������������, ��������� �����������,
//			�� ������, �� ����� �� ��������� ������ ���� ����������������.
//
//			THDi, �� �������� 50%, ����� �� ����� �������� �����������.
//			������� ����� ������ ���������� � ����. ���������� ����������� ����� ������
//		 	 � ������ � �� ��������, ������� �������������
//		 	 ������� ���������� ���������������� ������.
//
//			���������� ������ ������ ����������:
//			(��� ��������� ��������� ���������).
//			���� ����������� ������� �������� ����������� � ������� 10%,
//			����� ������ ���������� ������ �����������.
//			������� �� ���� ��������, ���������� ������ ������:
//			32,5% ��� ���������� ��������������
//			18% ��� ��������� ��������������
//			5% ��� ��������������.
//		 	 ***************************************************************************
//			3, 5, 7, 11 and 13 - �������� ��������� ��� ��������������
//		 	 */
//		    /* 1,4,7,10 3k+1 - 䳺 � ���� � �������� ��������� ��� ������, ��� ������ ��������� �������� �������*/
//
//			/* 2, 5, 8, 11   - ���� ���� ����� �������� ���������, �� ���������� �� ������� ��������� �������� �������*/
//
//			/* ������, �� ��������� ������ � ���������� �������*/
//			amperf_t aNegativeAmperage_A*/
//			aNegativeAmperage_A
			RECT rectan = {rect.left+x_value, rect.top+y_aNegativeAmperage_A,rect.left+839+140,rect.top+y_aNegativeAmperage_A+25};
			snprintf (str,sizeof(str),"%2d.%03d",
			grid_m16[0x28],grid_m16[0x29]/10);
			DrawText(hDCr, str, 19, &rectan, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_bNegativeAmperage_A != 0){
//			bNegativeAmperage_A*/
			RECT rectbn = {rect.left+x_value, rect.top+y_bNegativeAmperage_A,rect.left+839+140,rect.top+y_bNegativeAmperage_A+25};
			snprintf (str,sizeof(str),"%2d.%03d",
			grid_m16[0x2a],grid_m16[0x2b]/10);
			DrawText(hDCr, str, 19, &rectbn, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_cNegativeAmperage_A != 0){
//			amperf_t cNegativeAmperage_A*/
			RECT rectcn = {rect.left+x_value, rect.top+y_cNegativeAmperage_A,rect.left+839+140,rect.top+y_cNegativeAmperage_A+25};
			snprintf (str,sizeof(str),"%2d.%03d",
			grid_m16[0x2c],grid_m16[0x2d]/10);
			DrawText(hDCr, str, 19, &rectcn, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_aNeutralAmperage_A != 0){
//			grid_m16[0x2C]/*dc(g->cNegativeAmperage_A)*/	grid_m16[0x27]/*fr(g->cNegativeAmperage_A)*/
//			/* 3, 6, 9 .. 3k+3 - ����� �� �������, �� ���������.
//			 * ����� ��������� �� ����� �� ��������� �� ������.
//			 * �� ����� �� ��������� �� ��������� ����������� ��������
//		      ������ �������� ������� ������ �� �������������,
//		      � ����������� ����������� � ���������� ���.*/
//			/* ������, �� ����� �������� �������� �� ����� ��� � ��������,
//			 * ��������� ��������
//			aNeutralAmperage_A
			RECT rectnaa = {rect.left+x_value, rect.top+y_aNeutralAmperage_A,rect.left+839+140,rect.top+ y_aNeutralAmperage_A+25};
			snprintf (str,sizeof(str),"%2d.%03d",
			grid_m16[0x2e],grid_m16[0x2f]/10);
			DrawText(hDCr, str, 19, &rectnaa, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_bNeutralAmperage_A != 0){
//			bNeutralAmperage_A
			RECT rectnab = {rect.left+x_value, rect.top+ y_bNeutralAmperage_A,rect.left+839+140,rect.top+ y_bNeutralAmperage_A+25};
			snprintf (str,sizeof(str),"%2d.%03d",
			grid_m16[0x30],grid_m16[0x31]/10);
			DrawText(hDCr, str, 19, &rectnab, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_cNeutralAmperage_A != 0){
//			cNeutralAmperage_A
			RECT rectnac = {rect.left+x_value, rect.top+ y_cNeutralAmperage_A,rect.left+839+140,rect.top+y_cNeutralAmperage_A+25};
			snprintf (str,sizeof(str),"%2d.%03d",
			grid_m16[0x32],grid_m16[0x33]/10);
			DrawText(hDCr, str, 19, &rectnac, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_sumNeutralAmperage_A != 0){
//			sumNeutralAmperage_A
			RECT rectnat = {rect.left+x_value, rect.top+ y_sumNeutralAmperage_A,rect.left+839+140,rect.top+ y_sumNeutralAmperage_A+25};
			snprintf (str,sizeof(str),"%2d.%03d",
			grid_m16[0x34],grid_m16[0x35]/10);
			DrawText(hDCr, str, 19, &rectnat, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_leakFurie != 0){
			//leakFurie
			RECT rectnlf = {rect.left+x_value, rect.top+ y_leakFurie,rect.left+839+140,rect.top+ y_leakFurie+25};
			snprintf (str,sizeof(str),"%4d.%03d",
			grid_m16[0x36],grid_m16[0x37]/10);
			DrawText(hDCr, str, 19, &rectnlf, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_leakFuriePhase != 0){
			//y_leakFuriePhase
			RECT rectlfp = {rect.left+x_value, rect.top+ y_leakFuriePhase,rect.left+839+140,rect.top+ y_leakFuriePhase+25};
			snprintf (str,sizeof(str),"%d",grid_m16[0x38]);
			DrawText(hDCr, str, 10, &rectlfp, DT_SINGLELINE|DT_LEFT|DT_TOP);
//			if (grid_m16[0x38] == 1){
//				DrawText(hDCr, "���� �", 6, &rectlfp, DT_SINGLELINE|DT_LEFT|DT_TOP);
//			}
//			else if (grid_m16[0x38] == 2){
//				DrawText(hDCr, "���� B", 6, &rectlfp, DT_SINGLELINE|DT_LEFT|DT_TOP);
//			}
//			else if (grid_m16[0x38] == 3){
//				DrawText(hDCr, "���� C", 6, &rectlfp, DT_SINGLELINE|DT_LEFT|DT_TOP);
//			}
//			else{
//				DrawText(hDCr, "                     ", 15, &rectlfp, DT_SINGLELINE|DT_LEFT|DT_TOP);
//			}
			memset(&str,0,sizeof(str));
	   }
	   if(y_HeatPro != 0){
//			grid_m16[0x36]/*0x36*/	grid_m16[0x37]/*0x37*/
			RECT recthp = {rect.left+x_value, rect.top+y_HeatPro,rect.left+839+140,rect.top+y_HeatPro+25};
			snprintf (str,sizeof(str),"%3d.%02d",grid_m16[0x3A],grid_m16[0x3B]/100);
			DrawText(hDCr, str, 19, &recthp, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }


			int lParamPB=grid_m16[0x3A];
			if(lParamPB>150){lParamPB=150;}

		    RECT rectWhiteHeating4 = {rect.left+20,rect.top+660,rect.left+20+150,rect.top+678};
		    FillRect(hDCr, &rectWhiteHeating4, (HBRUSH)(COLOR_WINDOW+1)/*(HBRUSH)29*/);
		    RECT rectblueHeating4 = {rect.left+20,rect.top+660,rect.left+20+lParamPB,rect.top+678};
		    FillRect(hDCr, &rectblueHeating4, (HBRUSH)14);



			 /* ����� 120% - ������ � ��������� ��������� ��������������*/
			 SendMessage(hWndProgressBar_ThermalModel, PBM_SETRANGE, 0, MAKELPARAM(
			 		0,   //minimum range value
			 		160  //maximum range value
			 		));
			 SendMessage(hWndProgressBar_ThermalModel, PBM_SETPOS,
					lParamPB, //Signed integer that becomes the new position.
			 		0  //Must be zero
			 		);
			;
//			EnumChildWindows(hwnd , HeatProcessBarSet, lParamPB);

		}
		break;
		case 0xEEEE:{
//    		Tk4screenMode	=mb_rx_msg->msg[3]*0x100+mb_rx_msg->msg[4];
//    		DebugMode		=mb_rx_msg->msg[5]*0x100+mb_rx_msg->msg[6];
//    		AmperageAmplitude	=mb_rx_msg->msg[3]*0x100+mb_rx_msg->msg[4];
//    		BAmperageAmplitude		=mb_rx_msg->msg[5]*0x100+mb_rx_msg->msg[6];
//    		CosFi	=mb_rx_msg->msg[3]*0x100+mb_rx_msg->msg[4];

    		/***********************************************************************************************************
    		 * ²�����������
    		 * *********************************************************************************************************/
    		SendMessage(hDebugMode, CB_SETCURSEL, DebugMode, 0);
    		char str[10]={0};
    		snprintf (str,sizeof(str),"%3d",AmperageAmplitude);
    		SetWindowText(hAmperageAmplitude,str);
    		snprintf (str,sizeof(str),"%3d",BAmperageAmplitude);
    		SetWindowText(hBAmperageAmplitude,str);
    		snprintf (str,sizeof(str),"%3d",CosFi);
    		SetWindowText(hCosFi,str);
    		SendMessage(hTk4screenMode, CB_SETCURSEL, Tk4screenMode, 0);
    	}
		break;
		default:{}
		}
    	RECT rectblue = {rect.left+680,rect.top+70,rect.left+819,rect.top+700};
    	FillRect(hDCr, &rectblue, (HBRUSH)29);

    	if(y_cAmperage!=0){
    		/* ������ ����������� vS �����*/
    		char str[50]={0};
    		RECT rectict = {rect.left+x_value+140, rect.top+y_cAmperage,rect.left+839+140,rect.top+y_cAmperage+25};
    		snprintf (str,sizeof(str),"%4d       %4d",
    				grid_m16[0x3E],	/*dc(g->cAmperage_A)*/		grid_m16[0x3F] /*fr(g->cAmperage_A)*/
    		);
    		DrawText(hDCr, str, 19, &rectict, DT_SINGLELINE|DT_LEFT|DT_TOP);
    		memset(&str,0,sizeof(str));
    	}




        ReleaseDC(hwnd, hDCr);

//	}//tk5ProcessState==1){/////////////////////////////////////////////////////////////////
	/***********************************************************************************************************
	 * ʲ���� ������� ��²������� ������ֲ����� �����˲���
	 ***********************************************************************************************************/
//	else{}

//	SetFocus(hwnd);
//	snprintf(tk5LogStr,80," tk5ProcessCntr=%f              ",tk5ProcessCntr);
//	f_tk5Logging(tk5LogStr, sizeof(tk5LogStr));
	return 0;//tk5ProcessCntr;
}

static void f_Inittk5_Drow(void){
//	qOld.d_FS=UNKNOWN;
//	qOld.tk5_PumpingMode=7;

}



//static BOOL CALLBACK HeatProcessBarSet(
//  HWND   hwnd,
//  LPARAM lParam
//){
//  //https://stackoverflow.com/questions/30786481/winapi-c-how-to-completeley-wipe-clear-everything-in-window
//  return TRUE;
//}
//static BOOL CALLBACK DestoryTk5ChildCallbackRA(
//  HWND   hwnd,
//  LPARAM lParam
//)
//{
//  if (hwnd == hRBtnHighPanel) { DestroyWindow(hwnd);}
//  if (hwnd == hRBtnStart) { DestroyWindow(hwnd);}
//  if (hwnd == hRBtnStop) { DestroyWindow(hwnd);}
//  if (hwnd == hRBtnLowPanel) { DestroyWindow(hwnd);}
//  if (hwnd == hRBtnToA) { DestroyWindow(hwnd);}
//  //https://stackoverflow.com/questions/30786481/winapi-c-how-to-completeley-wipe-clear-everything-in-window
//  return TRUE;
//}

/* ���� �������� ��� ������� ������������ �������*/
static void f_desktop_tk5_session(void){
	f_desktop_tk_session(
			&tk5Queue
			);
}
//      			FillRect(hDCr, &rect_Mn, (HBRUSH)(COLOR_WINDOW+1));
//
//
//
//      	   	    InvalidateRect(hwnd,NULL,TRUE);
//      	     	UpdateWindow(hwnd);
//      	        SetFocus(hwnd);
int f_tk5UpdateQueueProcessBar(
		HWND hwnd,
		HWND hProcesBar,
		UINT_PTR pTimer1000,
		int timer1000_ID,
		UINT_PTR pTimer10,
		int timer10_ID
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
		uint16_t iQ= f_get_tkrs232_length(&tk5Queue);
	//	uint16_t iQ=f_get_tk5queue_length();
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
		/* ����� */
		snprintf(pm,80,"      %2d", iQ);
		RECT rect_E = {rect.left+x_right_panel+100, rect.top+660, rect.left+x_right_panel+170+100, rect.top+660+20};
		DrawText(hDCr, pm, 9, &rect_E, DT_SINGLELINE|DT_LEFT|DT_TOP);
		memset(pm,0,sizeof(pm));


		if(iQ>40){
			KillTimer(hwnd, pTimer1000);
			KillTimer(hwnd, pTimer10);
			const int result = MessageBox(NULL,
					"����� ���������� ��� ������ �����������\n��������� �����?",
					"Modbus Error",
			 MB_YESNO);
			switch (result){
			case IDYES:{
				f_clear_tkqueue(&tk5Queue);
			   /* ������������� �������� ������ */
	//		   p10Timer=SetTimer(
	//				hwnd,                // handle to main window
	//				IDT_TIMER_TK5_10MS,  // timer identifier
	//				TIMER_MAIN_MS,                  // msecond interval
	//				(TIMERPROC) NULL     // no timer callback
	//				);

			  pTimer1000=SetTimer(
					hwnd,                // handle to main window
					timer1000_ID,// timer identifier
					1000,                // msecond interval
					(TIMERPROC) NULL     // no timer callback
					);
			  pTimer10=SetTimer(
					hwnd,                // handle to main window
					timer10_ID,			 // timer identifier
					TIMER_MAIN_MS,       // msecond interval
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
/* �� ������� ����������� � 10� ������� � �������� �� �� ��2 ������� �����������, ��� ������ ���������
 *  */
int f_tk5UpdateStat(
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
	/* Գ����� ������� ����� ������ �� ����� */
//	uint16_t iQ= f_get_tkrs232_length(&tk5Queue);

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
		ReleaseDC(hwnd, hDCr);
	return 0;
}


/* ����/����������� �� ������������ ������������ �������,
 * ����������� � ���� PPB ������������ ������� */
static void Init_YMode(uint16_t Phasnost, uint16_t LoadMode){
	/* ��������� ������ */
	if((Phasnost==3)&&(LoadMode==0)){
		y_HeatPro							 =80;
		y_aAmperage							=100;
		y_bAmperage							=120;
		y_cAmperage							=140;
		y_power								=160;
		y_cosinus_factor					=180;
		y_unbalance_factor					=200;
		y_harm_factor						=220;
		y_motohours							=240;
		y_voltage							=260;
		y_frequency							=280;

		y_leakAmperage_A					=300;
		y_GrundAmperageDistortion			=320;
		y_LoadType							=340;

  	  y_AmperageUnbalance					=360;
  	  y_ActivePower_kW						=380;
  	  y_ReactivePower_kW					=400;
  	  y_UnbalanceLostPower_kW				=420;
  	  y_HarmonicLostPower_kW				=440;

  	  y_aTotalHarmonicDistortion			=460;
  	  y_bTotalHarmonicDistortion			=480;
  	  y_cTotalHarmonicDistortion			=500;
  	  y_THDi_HarmonicAmperageDistortion		=520;
  	  y_aNegativeAmperage_A					=540;
  	  y_bNegativeAmperage_A					=560;
  	  y_cNegativeAmperage_A					=580;
  	  y_aNeutralAmperage_A					=600;
  	  y_bNeutralAmperage_A					=620;
  	  y_cNeutralAmperage_A					=640;
  	  y_sumNeutralAmperage_A				=660;
  	  y_PhaseRotation						=680;
  	  y_leakFurie							=700;
  	  y_leakFuriePhase                    	=720;
	}
	/* ���������� ������ */
	else if ((Phasnost==1)&&(LoadMode==0)){
		y_HeatPro							 =80;
		y_aAmperage							=0;
		y_bAmperage							=0;
		y_cAmperage							=105;
		y_power								=130;
		y_cosinus_factor					=155;
		y_unbalance_factor					=0;
		y_harm_factor						=180;
		y_motohours							=205;
		y_voltage							=230;
		y_frequency							=255;

		y_leakAmperage_A					=0;
		y_GrundAmperageDistortion			=0;
		y_LoadType							=0;

	  y_AmperageUnbalance					=0;
	  y_ActivePower_kW						=280;
	  y_ReactivePower_kW					=305;
	  y_UnbalanceLostPower_kW				=0;
	  y_HarmonicLostPower_kW				=330;

	  y_aTotalHarmonicDistortion			=0;
	  y_bTotalHarmonicDistortion			=0;
	  y_cTotalHarmonicDistortion			=355;
	  y_THDi_HarmonicAmperageDistortion		=0;
	  y_aNegativeAmperage_A					=0;
	  y_bNegativeAmperage_A					=0;
	  y_cNegativeAmperage_A					=380;
	  y_aNeutralAmperage_A					=0;
	  y_bNeutralAmperage_A					=0;
	  y_cNeutralAmperage_A					=405;
	  y_sumNeutralAmperage_A				=0;
	  y_PhaseRotation						=0;
	  y_leakFurie							=0;
	  y_leakFuriePhase                    	=0;
	}
	/* �������� ���������������� */
	else if((Phasnost==3)&&(LoadMode==1)){
		y_HeatPro							 =0;
		y_aAmperage							=100;
		y_bAmperage							=120;
		y_cAmperage							=140;
		y_power								=160;
		y_cosinus_factor					=180;
		y_unbalance_factor					=0;
		y_harm_factor						=220;
		y_motohours							=240;
		y_voltage							=260;
		y_frequency							=280;

		y_leakAmperage_A					=0;
		y_GrundAmperageDistortion			=0;
		y_LoadType							=340;

  	  y_AmperageUnbalance					=360;
  	  y_ActivePower_kW						=380;
  	  y_ReactivePower_kW					=400;
  	  y_UnbalanceLostPower_kW				=0;
  	  y_HarmonicLostPower_kW				=0;

  	  y_aTotalHarmonicDistortion			=460;
  	  y_bTotalHarmonicDistortion			=480;
  	  y_cTotalHarmonicDistortion			=500;
  	  y_THDi_HarmonicAmperageDistortion		=520;
  	  y_aNegativeAmperage_A					=540;
  	  y_bNegativeAmperage_A					=560;
  	  y_cNegativeAmperage_A					=580;
  	  y_aNeutralAmperage_A					=600;
  	  y_bNeutralAmperage_A					=620;
  	  y_cNeutralAmperage_A					=640;
  	  y_sumNeutralAmperage_A				=660;
  	  y_PhaseRotation						=680;
  	  y_leakFurie							=0;
  	  y_leakFuriePhase                    	=0;
	}
	/* ��������� ���������������� */
	else if ((Phasnost==1)&&(LoadMode==1)){
		y_HeatPro							=0;
		y_aAmperage							=0;
		y_bAmperage							=0;
		y_cAmperage							=105;
		y_power								=130;
		y_cosinus_factor					=155;
		y_unbalance_factor					=0;
		y_harm_factor						=180;
		y_motohours							=0;
		y_voltage							=230;
		y_frequency							=255;

		y_leakAmperage_A					=0;
		y_GrundAmperageDistortion			=0;
		y_LoadType							=0;

	  y_AmperageUnbalance					=0;
	  y_ActivePower_kW						=280;
	  y_ReactivePower_kW					=305;
	  y_UnbalanceLostPower_kW				=0;
	  y_HarmonicLostPower_kW				=0;

	  y_aTotalHarmonicDistortion			=0;
	  y_bTotalHarmonicDistortion			=0;
	  y_cTotalHarmonicDistortion			=355;
	  y_THDi_HarmonicAmperageDistortion		=0;
	  y_aNegativeAmperage_A					=0;
	  y_bNegativeAmperage_A					=0;
	  y_cNegativeAmperage_A					=380;
	  y_aNeutralAmperage_A					=0;
	  y_bNeutralAmperage_A					=0;
	  y_cNeutralAmperage_A					=405;
	  y_sumNeutralAmperage_A				=0;
	  y_PhaseRotation						=0;
	  y_leakFurie							=0;
	  y_leakFuriePhase                    	=0;
	}


}


static int f_tk5UpdateValuesNames(HWND hwnd){
	HDC hDC; // ������ ���������� ���������� ������ �� ������
	RECT rect; // ���-��, ������������ ������ ���������� �������
	/*������� ������*/
	hDC= GetDC(hwnd);
	GetClientRect(hwnd, &rect);  	// �������� ������ � ������ ������� ��� ���������
	  	RECT rectwhite = {rect.left+190+1,rect.top+70,rect.left+550,rect.top+700-1};
	  	FillRect(hDC, &rectwhite, (HBRUSH)(COLOR_WINDOW+1));
		//����������� ������� ������ ������ ����
		RECT rectblue = {rect.left+550,rect.top+70,rect.left+819,rect.top+700-1};
		FillRect(hDC, &rectblue, (HBRUSH)29);
	/* ���������� ���������� ����� */

		/* �������� ��� ����� ��������� */
	if(y_HeatPro!=0){
		RECT rect0 = {rect.left+210, rect.top+y_HeatPro,rect.left+1000,rect.top+y_HeatPro+25};
		             //1234567890123456789012345678901234
		DrawText(hDC, "������ ������ ��������������, %", 34, &rect0, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_aAmperage!=0){
		RECT rectfa = {rect.left+210, rect.top+y_aAmperage,rect.left+1000,rect.top+y_aAmperage+25};
		DrawText(hDC, "����� ���� �, �                ", 32, &rectfa, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_bAmperage!=0){
		RECT rectfb = {rect.left+210, rect.top+y_bAmperage,rect.left+1000,rect.top+y_bAmperage+25};
		DrawText(hDC, "����� ���� �, �                ", 32, &rectfb, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_cAmperage!=0){
		RECT rectfc = {rect.left+210, rect.top+y_cAmperage,rect.left+1000,rect.top+y_cAmperage+25};
		DrawText(hDC, "����� ���� �, �                ", 32, &rectfc, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_power!=0){
		RECT rectpw = {rect.left+210, rect.top+y_power,rect.left+1000,rect.top+y_power+25};
		DrawText(hDC, "���������, ���                ", 32, &rectpw, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_cosinus_factor!=0){
		RECT rectcf = {rect.left+210, rect.top+y_cosinus_factor,rect.left+1000,rect.top+y_cosinus_factor+25};
		DrawText(hDC, "���������� ���������(������� �i)", 34, &rectcf, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_unbalance_factor!=0){
		RECT rectub = {rect.left+210, rect.top+y_unbalance_factor,rect.left+1000,rect.top+y_unbalance_factor+25};
		DrawText(hDC, "���������� ���������(��������)", 34, &rectub, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_harm_factor!=0){

		RECT rectih = {rect.left+210, rect.top+y_harm_factor,rect.left+1000,rect.top+y_harm_factor+25};
		DrawText(hDC, "���������� ���������(��������) ", 34, &rectih, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_motohours!=0){
		RECT rectmh = {rect.left+210, rect.top+y_motohours,rect.left+1000,rect.top+y_motohours+25};
		DrawText(hDC, "����������                       ", 34, &rectmh, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_voltage!=0){
		RECT rectu = {rect.left+210, rect.top+y_voltage,rect.left+1000,rect.top+y_voltage+25};
		DrawText(hDC, "�������                          ", 34, &rectu, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_frequency!=0){
		RECT rectff = {rect.left+210, rect.top+y_frequency,rect.left+1000,rect.top+y_frequency+25};
		DrawText(hDC, "�������                          ", 34, &rectff, DT_SINGLELINE|DT_LEFT|DT_TOP);

	}

	if(y_leakAmperage_A!=0){
		//			leakAmperage_A
					RECT rectla = {rect.left+210, rect.top+y_leakAmperage_A,rect.left+1000,rect.top+y_leakAmperage_A+25};
					DrawText(hDC, "�����, ���� �� �����, �", 25, &rectla, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_GrundAmperageDistortion!=0){

		//			GrundAmperageDistortion
					RECT rectad = {rect.left+210, rect.top+y_GrundAmperageDistortion,rect.left+1000,rect.top+y_GrundAmperageDistortion+25};
					DrawText(hDC, "���� ������, %", 16, &rectad, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_LoadType!=0){

					/* >LoadType - ��� ������������ - ����������/�������/������   */
					RECT recttl = {rect.left+210, rect.top+y_LoadType,rect.left+1000,rect.top+y_LoadType+25};
					DrawText(hDC, "��� ������������", 17, &recttl, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_PhaseRotation!=0){

					/*(uint16_t)(g->PhaseRotation)*/
					RECT rectak = {rect.left+210, rect.top+y_PhaseRotation,rect.left+1000,rect.top+y_PhaseRotation+25};
					DrawText(hDC, "����������� ���", 18, &rectak, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_cosinus_factor!=0){
					RECT rectcf1 = {rect.left+210, rect.top+y_cosinus_factor,rect.left+1000,rect.top+y_cosinus_factor+25};
					DrawText(hDC, "���������� ���������(������� ��), %", 37, &rectcf1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_unbalance_factor!=0){
					RECT rectub1 = {rect.left+210, rect.top+y_unbalance_factor,rect.left+839+140,rect.top+y_unbalance_factor+25};
					DrawText(hDC, "���������� ���������(��������), %", 36, &rectub1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_harm_factor!=0){
					RECT rectfh = {rect.left+210, rect.top+y_harm_factor,rect.left+839+140,rect.top+y_harm_factor+25};
					DrawText(hDC, "���������� ���������(��������), %", 36, &rectfh, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_ActivePower_kW!=0){
					RECT rectap = {rect.left+210, rect.top+y_ActivePower_kW,rect.left+839+140,rect.top+y_ActivePower_kW+25};
					DrawText(hDC, "��������� �������, ���", 24, &rectap, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_ReactivePower_kW!=0){
					/* ��������� ���������*/
					RECT rectrap = {rect.left+210, rect.top+y_ReactivePower_kW,rect.left+839+140,rect.top+y_ReactivePower_kW+25};
					DrawText(hDC, "��������� ���������, ���", 26, &rectrap, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_UnbalanceLostPower_kW!=0){
					/* ��������� �������� ��������������� �������� ������� ������ (���� ��� ��������������)*/
					RECT rectrap2 = {rect.left+210, rect.top+y_UnbalanceLostPower_kW,rect.left+839+140,rect.top+y_UnbalanceLostPower_kW+25};
					DrawText(hDC, "���������, ������ ����� ��������, ���", 40, &rectrap2, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_HarmonicLostPower_kW!=0){
					/* ���������, �������� ��������������� �������� ���������������� ������ */
					RECT rectrap3 = {rect.left+210, rect.top+y_HarmonicLostPower_kW,rect.left+839+140,rect.top+y_HarmonicLostPower_kW+25};

					DrawText(hDC, "���������, ������ ����� ��������, ���", 40, &rectrap3, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_power!=0){
					/*kvatt_t	ActivePower_kW*/
					RECT rectap1 = {rect.left+210, rect.top+y_power,rect.left+839+140,rect.top+y_power+25};
					DrawText(hDC, "��������� �������, ���", 24, &rectap1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_aTotalHarmonicDistortion!=0){

					/*������� ��������� ����������� ����� 					                    */
					/*���������� �������� ���������� �� ������ � ����� ���                      */
					//aTotalHarmonicDistortion
					RECT rectah = {rect.left+210, rect.top+y_aTotalHarmonicDistortion,rect.left+839+140,rect.top+y_aTotalHarmonicDistortion+25};
					DrawText(hDC, "���������� �������� ����������, ���� �, %", 44, &rectah, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_bTotalHarmonicDistortion!=0){
					//bTotalHarmonicDistortion
					RECT rectbh = {rect.left+210, rect.top+y_bTotalHarmonicDistortion,rect.left+839+140,rect.top+y_bTotalHarmonicDistortion+25};
					DrawText(hDC, "���������� �������� ����������, ���� �, %", 44, &rectbh, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_cTotalHarmonicDistortion!=0){
					//cTotalHarmonicDistortion
					RECT rectch = {rect.left+210, rect.top+y_cTotalHarmonicDistortion,rect.left+839+140,rect.top+y_cTotalHarmonicDistortion+25};
					DrawText(hDC, "���������� �������� ����������, ���� �, %", 44, &rectch, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_THDi_HarmonicAmperageDistortion!=0){
					//���������� �������� ���������� �� ������ ������������  (����)
		//			/*procentf_t THDi_HarmonicAmperageDistortion*/
					RECT rectth = {rect.left+210, rect.top+y_THDi_HarmonicAmperageDistortion,rect.left+839+140,rect.top+y_THDi_HarmonicAmperageDistortion+25};
					DrawText(hDC, "���������� �������� ����������, %", 36, &rectth, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_aNegativeAmperage_A!=0){
					RECT rectan = {rect.left+210, rect.top+y_aNegativeAmperage_A,rect.left+839+140,rect.top+y_aNegativeAmperage_A+25};
					DrawText(hDC, "������ ���������� �������, ���� �, �", 38, &rectan, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_bNegativeAmperage_A!=0){
		//			bNegativeAmperage_A*/
					RECT rectbn = {rect.left+210, rect.top+y_bNegativeAmperage_A,rect.left+839+140,rect.top+y_bNegativeAmperage_A+25};
					DrawText(hDC, "������ ���������� �������, ���� �, �", 38, &rectbn, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_cNegativeAmperage_A!=0){
		//			amperf_t cNegativeAmperage_A*/
					RECT rectcn = {rect.left+210, rect.top+y_cNegativeAmperage_A,rect.left+839+140,rect.top+y_cNegativeAmperage_A+25};
					DrawText(hDC, "������ ���������� �������, ���� �, �", 38, &rectcn, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_aNeutralAmperage_A!=0){
					RECT rectnaa = {rect.left+210, rect.top+y_aNeutralAmperage_A,rect.left+839+140,rect.top+ y_aNeutralAmperage_A+25};
					DrawText(hDC, "������ ����������� �������, ���� �, �", 38, &rectnaa, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_bNeutralAmperage_A!=0){
		//			bNeutralAmperage_A
					RECT rectnab = {rect.left+210, rect.top+ y_bNeutralAmperage_A,rect.left+839+140,rect.top+ y_bNeutralAmperage_A+25};
					DrawText(hDC, "������ ����������� �������, ���� �, �", 38, &rectnab, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_cNeutralAmperage_A!=0){
		//			cNeutralAmperage_A
					RECT rectnac = {rect.left+210, rect.top+ y_cNeutralAmperage_A,rect.left+839+140,rect.top+y_cNeutralAmperage_A+25};
					DrawText(hDC, "������ ����������� �������, ���� �, �", 38, &rectnac, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_sumNeutralAmperage_A!=0){
		//			sumNeutralAmperage_A
					RECT rectnat = {rect.left+210, rect.top+ y_sumNeutralAmperage_A,rect.left+839+140,rect.top+ y_sumNeutralAmperage_A+25};
					DrawText(hDC, "������ ����������� ������� �����, �", 37, &rectnat, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_AmperageUnbalance!=0){						            // 1234567890123456789012345678901234567890123456789
					RECT rectnkf = {rect.left+210, rect.top+ y_AmperageUnbalance,rect.left+839+140,rect.top+ y_AmperageUnbalance+25};
					DrawText(hDC, "������� ���, %", 15, &rectnkf, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_leakFurie!=0){
					RECT rectnlf = {rect.left+210, rect.top+ y_leakFurie,rect.left+839+140,rect.top+ y_leakFurie+25};
					            // 1234567890123456789012345678901234567890123456789
					DrawText(hDC, "���� ������ �� �����(�), �", 28, &rectnlf, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_leakFuriePhase!=0){
					RECT rectnlfp = {rect.left+210, rect.top+ y_leakFuriePhase,rect.left+839+140,rect.top+ y_leakFuriePhase+25};
					DrawText(hDC, "���� ������(�)", 15, &rectnlfp, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	ReleaseDC(hwnd, hDC);
	return 0;
}
