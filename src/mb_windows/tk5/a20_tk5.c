
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
 *  ЗМІННІ ТА ФУНКЦІЇ ВХІДНОГО (ОСНОВНОГО) ВІКНА ДИСТАНЦІЙНОГО УПРАВЛІННЯ PC_DESK <-> tk5
 * ******************************************************************************************************************************
 * */


/* Структура, яка визначає чергу
 * Визначення цієї структури означає, що створена
 * конкретна черга з конкретним іменем, поки що пуста
 * Визначено структуру - дескриптор черги повідомлень до Тк2,
 * Робота з чергою здійснюється функціями файлу 'a3_tk5TxRx.c/h' */
   queue_t tk5Queue = {0};

   /* Визначаємо показчик на дескриптор файла, куди будуть записуватися логи
    * Ім'я файла визначається автоматично,
    * робота з логами здійснюється функціями файлу  a4_tk5log.c/h*/
   FILE *fp_tk5LogFile=NULL;
   /* Рядок, куди скидаються текст логів. Перевіряєтсья періодично в таймері 10мс,
    * якщо не пустий, рядок виводиться у файл.
    * Якщо рядків багато - треба відразу виводити в файл, див. функцію f_tk5Logging(), файл a3_tk5z.c/h */
   char tk5LogStr[512]={0};

/* При відкриванні вікна, скільки ми поки що не знаємо РЕЖИМу РОБОТИ,
 * відкриваються лише ті контролі, які є спільними для всіх режимів роботи
 *
 * При натисканні кнопки 'Тест.дист.управління' формується запит на зчитування типорозміру станції
 * фактично лише перевіряється наявність каналу Модбас з tk5
 * Решта кнопок заблоковані.
 * При натисканні кнопки 'Старт дист.управління'
 * запускається процес обміну повідомленнями PC_DESK <-> tk5,
 * і триває до натискання "Стоп дист.упр"*/
ProcessState_t tk5Process=ProcessIddle;
 /* Це прапор запущеного процесу дистанційного управління.
  * При натисканні  кнопки 'Старт дист.управління' tk5Process=ProcessBusy
  * При натисканні   'Старт дист.управління' tk5Process=ProcessIddle
  * */

/* Натискання на кнопки обробляється тільки після
 * затримки, захист від бездумного частого натискання на кномки*/
static FunctionalState tk5btn_access = ENABLE;
/* Лічильник затримки*/
static uint32_t tk5btnSleepCntr = 0;
/* Тривалість затримки */
#define  tk5_BTN_BLOKING_MS         400

/* Визначаємо структуру, де будуть зберігатися параметри роботи
 * роботи тк2 - управління та захисту електронасоса/електродвигуна*/
wtk5_t q5={0};
extern tk5user_settings_t Tk5us;
extern tk5fs_t tk5fs;
/* Попередні значення параметрів для виводу в вікно лише тих, які змінилися*/
//static wtk5_t q5Old={0};

/* Номінальний струм для роботи процес-бару теплової моделі електродвигуна */
//extern float2_t Itk5_Max;

HWND hWndtk5 = NULL;
HWND hWndtk5usCh;
HWND hWndTk5fsCh;
//extern HWND hWndTk5usCh;
HINSTANCE hinst_tk5;
static  HFONT hfDefault;
const char g_szClassNameTK5[] = "TK5_WndClass";
/* Глобальні змінні, передаються на обробку в вікно, яке повідомлення кидало в чергу */
extern modbus_master_rx_msg_t mIn_rx_msg;
/* Глобальні змінні, передаються на обробку в вікно, яке повідомлення кидало в чергу */
extern modbus_master_tx_msg_t mOut_tx_msg;
/* Глобальні змінні, передаються на обробку в вікно, яке повідомлення кидало в чергу */
extern modbus_status_t RxMasterModbusStatus;

/* Локальна змінна для обчислення проценту успішних сесій Модбас*/
static event_t  met_zero_MBsuccess = NO_EVENT;
/* Лічильник водів в таймер 100мс для передачі різних повідомлень в рісних випадках*/
static uint32_t tk5TimerCntr = 0;

//static uint16_t tk5ProcessState =0;
/* Прапоp ходу процесу, відсутності процесу,
 * у парі для визначення подій переходу
 * від обробки до процесу і від процесу до обробки*/
/* З цього вікна запускається декілька процесів, для ідентифікації відповіді */
int RW5_case=0;

//static char user_msg5[256]={0};

/* Координати для малювання вікна */
/* Поточне значення координати Y лівого радіоіндикатора */
static uint32_t y_LeftRadio = 240;
/* При заданій ширині вікна 1024 ми бачимо 1009 */
static uint16_t x_left_panel  				= 20;
static uint16_t x_left_right_width_panel	= 150;
static uint16_t x_centr_panel   			= 190;
static uint16_t x_centr_width 				= 629;
static uint16_t x_right_panel 				= 839;
/* Друк/розмальовка за результатами паралельного процесу,
 * викликається з вікна PPB паралельного процесу */

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
/* Всікі контролі - кнопки, таймери, Едіт-бари, Комбобокси і т.п.*/
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




/* Ця функція викликається кожен раз при отриманні повідомлення в VK_UPDATE_WINDOW
 * про завершення сесії Модбас, ініційованої з основного вікна
 * wParam-повернутий з обробника черги
 * ідентифікатор вихідного повідомлення RW5_case */
static int f_tk5UpdateWindow(HWND hwnd, int wParam, int responce_status);
/* Це функція-обгортка запуску паралельного процесу */
static void f_desktop_tk5_session(void);
/* Ініціалізація, щоб уникнути небажаних величин параметрів при почанковому завантаженні */
static void f_Inittk5_Drow(void);


//static BOOL CALLBACK DestoryTk5ChildCallbackRA(/* РУЧН->АВТ*/
//  HWND   hwnd,
//  LPARAM lParam
//);

static void Init_YMode(uint16_t Phasnost, uint16_t LoadMode);
uint8_t tk5_Addr=1;
//static int period=1000;
#ifndef		tk5_DISTANCE_WND
/* Це відкриття вікна ДИСТ.УПРАВЛІННЯ як дочірного в коаплексі програм,
може щось прийдеться поправити тут */
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
		"ТК5.Дистанційне управління",

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
	/* Реєстрація класів вікон, що відкриваються багатократно
	 * різними батьківськими вікнами */
	hWndtk5=child5;
	f_RegisterTk5usWndClass(hinst_tk5);
	f_RegisterTk5fsWndClass(hinst_tk5);


	HMENU hMenubar = CreateMenu();
	HMENU hMenu = CreateMenu();

	AppendMenu(hMenu, MF_STRING, IDM_tk5COMPORT_SETTING, "&Налаштування СОМ-порту");
	AppendMenu(hMenu, MF_STRING, IDM_tk5USER_SETTINGS,   "&Налаштування користувача");
	AppendMenu(hMenu, MF_STRING, IDM_tk5FACILITY_SETTING,"&Налаштування виробника");
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenu, MF_STRING, IDM_tk5QUIT, "&Вихід");

	AppendMenu(hMenubar, MF_POPUP, (UINT_PTR) hMenu, "&Налаштування");

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



/* Точка входу в Windows-інтерфейс, створення основного вікна */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow){
	/* Відкриваємо початковий файл для логів */
	fp_tkLogFile=f_tk5log_Init();
	/* Тепер створюємо вікно */
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
	/* Пригодиться :) */
//	int width      = GetSystemMetrics(SM_CXSCREEN);
//	screen_heigh   = GetSystemMetrics(SM_CYSCREEN);

	hWndtk5 = CreateWindowEx (
    		0,
			g_szClassNameTK112,
    		TEXT ("ТК2.Дистанційне управління"),
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
	/* Реєстрація класів вікон, що відкриваються багатократно
	 * різними батьківськими вікнами */
	f_Register_PasswordWndClass(hInstance);
	f_RegisterSWndClass(hInstance);

	f_RegisterTK112USWndClass(hInstance);
	f_Registertk5fsWndClass(hInstance);

	HMENU hMenubar = CreateMenu();
	HMENU hMenu = CreateMenu();

	AppendMenu(hMenu, MF_STRING, IDM_COMPORT_SETTING, "&Налаштування СОМ-порту");
	AppendMenu(hMenu, MF_STRING, IDM_USER_SETTINGS, "&Налаштування користувача");
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenu, MF_STRING, IDM_QUIT, "&Вихід");

	AppendMenu(hMenubar, MF_POPUP, (UINT_PTR) hMenu, "&Налаштування");

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
#define PHASNOST 3 /* 	3 три фази
						1 одна фаза */
#define LOADMODE 0 /*0 - всяке навантаження,
 	 	 	 	 	 1 - електромотор
					 2 - не електромотор*/

LRESULT CALLBACK WndProcTK5(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	modbus_master_tx_msg_t mbTxMsg={0};
//	Init_YMode( PHASNOST, LOADMODE);
	switch(msg){

		/**************************************************************************************************************
		 *      CREATE 																						     CREATE
		 **************************************************************************************************************/
		case WM_CREATE:{
		  /* Доска для верхнього надпису */
		  CreateWindowW(L"Button", L"",
                WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				x_centr_panel, 10,  x_centr_width, 40, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
		  /* Центральна доска для параметрів */
          CreateWindowW(L"Button", L"",
                WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				x_centr_panel, 60,  x_centr_width, 640, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
          //http://zetcode.com/gui/winapi/controlsIII/


          /* Ліва доска для кнопок управління  */
	   	  CreateWindowW(L"Button", L"", WS_CHILD /*| WS_VISIBLE */| BS_GROUPBOX| WS_GROUP,
	   			 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 290, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
	   	  hWndViewBtnMotorState= CreateWindow("button", "Мережа ОК" , WS_CHILD |
	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
	   			 x_left_panel, y_LeftRadio, x_left_right_width_panel , 20, hwnd,(HMENU) IDB_tk5SHOW_MOTOR_OPERATING, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
//#define IDBI_GRID_OVERLOADPROTECTION					1

	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
	   	hGRID_OverLoadProtection= CreateWindow("button", "Перевантаження" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +20, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_OVERLOADPROTECTION, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

//#define IDBI_GRID_UNDERLOADPROTECTION					2

	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
	   	hGRID_UnderLoadProtection= CreateWindow("button", "Недовантаження" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +40, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_UNDERLOADPROTECTION, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

//#define IDBI_GRID_VOLTAGEFAULT							4

	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
	   	  hGRID_VoltageFault= CreateWindow("button", "Напруга" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +60, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_VOLTAGEFAULT, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

//#define IDBI_GRID_AMPERAGEUNBALANCEPROTECTION			8
	   	  /* Лівий індикатор сухого ходу */
	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
		  hGRID_AmperageUnbalanceProtection= CreateWindow("button", "Асиметрія" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +80, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_AMPERAGEUNBALANCEPROTECTION, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

//#define IDBI_GRID_GROUNDPROTECTION						0X10

	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
		  hGRID_GroundProtection= CreateWindow("button", "Витік на землю" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +100, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_GROUNDPROTECTION, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

////
//#define IDBI_GRID_STARTNUMBERLIMIT						0X20

	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
		  hGRID_StartNumberLimit= CreateWindow("button", "Число стартів" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +120, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_STARTNUMBERLIMIT, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

//#define IDBI_GRID_JUMMEDPROTECTION						0X40

	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
		  hGRID_JummedProtection= CreateWindow("button", "Дрижання" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +140, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_JUMMEDPROTECTION, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

////
//#define IDBI_GRID_PHASE_A_OVERLOADPROTECTION			0X80
	   	  /* Лівий індикатор сухого ходу */
	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
		  hGRID_Phase_A_OverLoadProtection= CreateWindow("button", "Перевантаження А" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +160, x_left_right_width_panel , 20, hwnd,(HMENU) IDB_tk5SHOW_CX, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

//#define IDBI_GRID_PHASE_B_OVERLOADPROTECTION			0X100
	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
		  hGRID_Phase_B_OverLoadProtection= CreateWindow("button", "Перевантаження B" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +180, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_PHASE_B_OVERLOADPROTECTION, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

//#define IDBI_GRID_PHASE_C_OVERLOADPROTECTION			0X200

	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
		  hGRID_Phase_C_OverLoadProtection= CreateWindow("button", "Перевантаження C" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +200, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_PHASE_C_OVERLOADPROTECTION, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

////
//#define IDBI_GRID_FREQUENCY_FAULT                       0X400

	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
	   	  hGRID_FrequencyFault= CreateWindow("button", "Частота" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +220, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_FREQUENCY_FAULT, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

//#define IDBI_GRID_PHASE_SEQUENCE_FAULT                  0X800

	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX| WS_GROUP,
			 	 x_left_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 300, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
		  hGRID_PhaseSeqFault= CreateWindow("button", "Послідовність фаз" , WS_CHILD |
			 	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
			 	 x_left_panel,y_LeftRadio +240, x_left_right_width_panel , 20, hwnd,(HMENU) IDBI_GRID_PHASE_SEQUENCE_FAULT, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

		  //hGRID_OverLoadProtection
		  //hGRID_UnderLoadProtection
		  //hGRID_VoltageFault
		  //hGRID_AmperageUnbalanceProtection
		  //hGRID_GroundProtection










          /* Права дошка доска для кнопок управління  */
	   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE |*/ BS_GROUPBOX| WS_GROUP,
	   			x_right_panel, y_LeftRadio /*410*/, x_left_right_width_panel, 290, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);


			hRBtnHighPanel=CreateWindowW(L"Button", L"",
	                  WS_CHILD | /*WS_VISIBLE | */BS_GROUPBOX,
	             x_left_panel, 60,  150, 210, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
			hRBtnStart=CreateWindow("BUTTON", "ПУСК",
                                         WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
										   x_left_panel, 90,  150, 30, hwnd, (HMENU)IDB_tk5BtnSTART,
										   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
			hRBtnStop=CreateWindow("BUTTON", "СТОП",
                                         WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
										   x_left_panel, 150, 150, 30, hwnd, (HMENU)IDB_tk5BtnSTOP,
										   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
//			/* Безумовний прогрес-бар для нагрівання електродвигуна */
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
//			 /* Шкала 120% - задаємо в процентах */
//			 SendMessage(hWndProgressBar_ThermalModel, PBM_SETRANGE, 0, MAKELPARAM(
//			 		0,   //minimum range value
//			 		150  //maximum range value
//			 		));
//
//			 SendMessage(hWndProgressBar_ThermalModel, PBM_SETPOS,
//			 		0, //Signed integer that becomes the new position.
//			 		0  //Must be zero
//			 		);


	          /* Права доска для кнопок управління  */
		   	  CreateWindowW(L"Button", L"", WS_CHILD | /*WS_VISIBLE| */BS_GROUPBOX| WS_GROUP,
		   			x_right_panel, 60, 150, 640, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);


	          /* EditBox для Модбас-адреси */
	          char addr_s[3]={0};
	          hEdit_tk5_ADDR = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", addr_s,
					WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
					, x_right_panel, 80, 40, 25,
					hwnd, (HMENU)ID_tk5_EDIT_ADDRESS , GetModuleHandle(NULL), NULL);
			  SendMessage(hEdit_tk5_ADDR, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

			  snprintf (addr_s,20,"%X",tk5_Addr);
			  SetWindowText(hEdit_tk5_ADDR,(LPCTSTR)addr_s);

	          CreateWindow("BUTTON", "Тест дист.упр.",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_right_panel, 105, 150, 20, hwnd, (HMENU)IDB_tk5TEST_DISTANCE_CTRL,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
	          CreateWindow("BUTTON", "Старт дист.упр.",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_right_panel, 130, 150, 20, hwnd, (HMENU)IDB_tk5START_DISTANCE_CTRL,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
	          CreateWindow("BUTTON", "Стоп  дист.упр.",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_right_panel, 155, 150, 20, hwnd, (HMENU)IDB_tk5STOP_DISTANCE_CTRL,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);

	          CreateWindow("BUTTON", "Скид.захисту",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_right_panel+30, 205, 150-30, 20, hwnd, (HMENU)IDB_tk5RESET_MOTOR_PROTECTION_1,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
	          CreateWindow("BUTTON", "Скид.захисту 2",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_right_panel, 230, 150, 20, hwnd, (HMENU)IDB_tk5RESET_MOTOR_PROTECTION_2,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);


	          CreateWindow("BUTTON", "Вихід",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
											   x_right_panel, 300, 150, 20, hwnd, (HMENU)IDB_tk5DIST_EXIT,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);


			  /* Безумовний прогрес-бар для черги повідомлень */
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
			  /* Шкала 0x10000 */
			  SendMessage(hWndProgressBar_Queue, PBM_SETRANGE, 0, MAKELPARAM(
							  0,       //minimum range value
							  10000  //maximum range value
							  )
			  );
			  SendMessage(hWndProgressBar_Queue, PBM_SETPOS,
				 			  0, //Signed integer that becomes the new position.
				 			  0  //Must be zero
			  );
			  /* таймер для роботи з процес-баром черги повідомлень */
			  p10Timer=SetTimer(
						hwnd,                // handle to main window
						IDT_TIMER_tk5_10MS,  // timer identifier
						TIMER_MAIN_MS,       // msecond interval
					    (TIMERPROC) NULL     // no timer callback
						);
			  f_Inittk5_Drow();

/***********************************************************************************************************
 * ВІДЛАГОДЖЕННЯ
 * *********************************************************************************************************/
			  hDebugMode = CreateWindow("COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
			         		  1009, yDebugMode, 200, 800,
			 				  hwnd, (HMENU)IDCB_YDEBUGMODE,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),NULL);
			  SendMessage(hDebugMode, CB_ADDSTRING, 0, (LPARAM)" Реальне навантаження			  ");
			  SendMessage(hDebugMode, CB_ADDSTRING, 0, (LPARAM)" Симулятор, синусоїда             ");
			  SendMessage(hDebugMode, CB_ADDSTRING, 0, (LPARAM)" Симулятор, прямокутник           ");
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

			  SendMessage(hTk4screenMode, CB_ADDSTRING, 0, (LPARAM)" Дата та час        			  ");
			  SendMessage(hTk4screenMode, CB_ADDSTRING, 0, (LPARAM)" Напруга(В) і частота(Гц)         ");
			  SendMessage(hTk4screenMode, CB_ADDSTRING, 0, (LPARAM)" Струм В.К. vs Є.М., ацп          ");
			  SendMessage(hTk4screenMode, CB_ADDSTRING, 0, (LPARAM)" Струм В.К. vs Є.М., ампери       ");
			  SendMessage(hTk4screenMode, CB_ADDSTRING, 0, (LPARAM)" Косинус Фі                       ");
			  SendMessage(hTk4screenMode, CB_ADDSTRING, 0, (LPARAM)" Струми(А) і асиметрія            ");
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
		case WM_PAINT: {// если нужно нарисовать, то:

			HDC hDC; // создаём дескриптор ориентации текста на экране
    	    RECT rect; // стр-ра, определяющая размер клиентской области
	    	PAINTSTRUCT ps; // структура, сод-щая информацию о клиентской области (размеры, цвет и тп)

	    	hDC = BeginPaint(hwnd, &ps); 	// инициализируем контекст устройства
	    	GetClientRect(hwnd, &rect);  	// получаем ширину и высоту области для рисования

	    	RECT rectyL =    {rect.left+210,rect.top+25, rect.left+510,rect.top+50};
	    	DrawText(hDC, "Параметри", 10, &rectyL, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	//Замальовуємо область справа центру синім
	    	RECT rectblue = {rect.left+550,rect.top+70,rect.left+819,rect.top+700};
	    	FillRect(hDC, &rectblue, (HBRUSH)29);


	    	/*Зглажені параметри ефективності мережі 					*/
	    	/*Коефіцієнт нелінійних спотворень по струму у кожній фазі  */
//	    	extern procentf_t aTotalHarmonicDistortion;
//	    	extern procentf_t bTotalHarmonicDistortion;
//	    	extern procentf_t cTotalHarmonicDistortion;

	    	/************************************************************************
	    	Коефіцієнт нелінійних спотворень по струму узагальнений    				*/
	    	//extern procentf_t THDi_HarmonicAmperageDistortion;
	    	/*THDі вказує на ступінь спотворення форми синусоїди струму.
	    	THDi менше 10% вважається нормальним,
	    	ризик несправностей обладнання відсутній, перегрівання до 6%.

	    	THDi між 10% і 50% вказує на значне гармонійне спотворення.
	    	Падіння коефіцієнта потужності елекродвигунів, підвищення температури,
	    	що означає, що кабелі та установок повинні бути понадгабаритними.

	    	THDi, що перевищує 50%, вказує на високі гармонічні спотворення.
	    	Високий ризик виходу обладнання з ладу. Необхідний поглиблений аналіз причин
	    	 і заходи з їх усунення, зокрема рекомендується
	    	 система придушення несинусоїдальнусті струму.

	    	Скорочення терміну служби обладнання:
	    	(Дані Канадської асоціації електриків).
	    	Коли спотворення напруги живлення знаходиться в діапазоні 10%,
	    	термін служби обладнання значно скорочується.
	    	Залежно від типу пристрою, скорочення терміну служби:
	    	32,5% для однофазних електродвигунів
	    	18% для трифазних електродвигунів
	    	5% для трансформаторів.
	    	***************************************************************************
	    	3, 5, 7, 11 and 13 - найбільше небезпечні для електродвигунів




	    		//vatt_t	ActivePower 		= 	0;
	    	 	//Поточна активна потужність

	    		//vatt_t	ReactivePower 		=   0;

	    		//vatt_t	UnbalanceLostPower 	=	0;

	    		//vatt_t	HarmonicLostPower 	= 	0;

	    		//vatt_t	NeutralHarmonicLostPower 	= 	0;


	    		//vatt_t	NegativeHarmonicLostPower 	= 	0;*/



	    if(y_HeatPro!=0){
	    	RECT rect0 = {rect.left+210, rect.top+y_HeatPro,rect.left+1000,rect.top+y_HeatPro+25};
	    	             //1234567890123456789012345678901234
	    	DrawText(hDC, "Ступінь нагріву електродвигуна, %", 34, &rect0, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_aAmperage!=0){
			RECT rectfa = {rect.left+210, rect.top+y_aAmperage,rect.left+1000,rect.top+y_aAmperage+25};
			DrawText(hDC, "Струм фази А, А                ", 32, &rectfa, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_bAmperage!=0){
	    	RECT rectfb = {rect.left+210, rect.top+y_bAmperage,rect.left+1000,rect.top+y_bAmperage+25};
    		DrawText(hDC, "Струм фази В, А                ", 32, &rectfb, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_cAmperage!=0){
    		RECT rectfc = {rect.left+210, rect.top+y_cAmperage,rect.left+1000,rect.top+y_cAmperage+25};
    		DrawText(hDC, "Струм фази С, А                ", 32, &rectfc, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_power!=0){
	    	RECT rectpw = {rect.left+210, rect.top+y_power,rect.left+1000,rect.top+y_power+25};
	    	DrawText(hDC, "Потужність, кВт                ", 32, &rectpw, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_cosinus_factor!=0){
    		RECT rectcf = {rect.left+210, rect.top+y_cosinus_factor,rect.left+1000,rect.top+y_cosinus_factor+25};
    		DrawText(hDC, "Коефіцієнт потужності(косинус фi)", 34, &rectcf, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_unbalance_factor!=0){
    		RECT rectub = {rect.left+210, rect.top+y_unbalance_factor,rect.left+1000,rect.top+y_unbalance_factor+25};
    		DrawText(hDC, "Коефіцієнт потужності(асиметрія)", 34, &rectub, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_harm_factor!=0){

    		RECT rectih = {rect.left+210, rect.top+y_harm_factor,rect.left+1000,rect.top+y_harm_factor+25};
			DrawText(hDC, "Коефіцієнт потужності(гармоніки) ", 34, &rectih, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_motohours!=0){
    		RECT rectmh = {rect.left+210, rect.top+y_motohours,rect.left+1000,rect.top+y_motohours+25};
			DrawText(hDC, "Мотогодини                       ", 34, &rectmh, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_voltage!=0){
    		RECT rectu = {rect.left+210, rect.top+y_voltage,rect.left+1000,rect.top+y_voltage+25};
			DrawText(hDC, "Напруга                          ", 34, &rectu, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_frequency!=0){
    		RECT rectff = {rect.left+210, rect.top+y_frequency,rect.left+1000,rect.top+y_frequency+25};
			DrawText(hDC, "Частота                          ", 34, &rectff, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    }



	    if(y_leakAmperage_A!=0){
			//			leakAmperage_A
						RECT rectla = {rect.left+210, rect.top+y_leakAmperage_A,rect.left+1000,rect.top+y_leakAmperage_A+25};
						DrawText(hDC, "Струм, витік на землю, А", 25, &rectla, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_GrundAmperageDistortion!=0){

			//			GrundAmperageDistortion
						RECT rectad = {rect.left+210, rect.top+y_GrundAmperageDistortion,rect.left+1000,rect.top+y_GrundAmperageDistortion+25};
						DrawText(hDC, "Витік струму, %", 16, &rectad, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_LoadType!=0){

						/* >LoadType - Тип навантаження - індуктивне/активне/ємнісне   */
						RECT recttl = {rect.left+210, rect.top+y_LoadType,rect.left+1000,rect.top+y_LoadType+25};
						DrawText(hDC, "Тип навантаження", 17, &recttl, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_PhaseRotation!=0){

						/*(uint16_t)(g->PhaseRotation)*/
						RECT rectak = {rect.left+210, rect.top+y_PhaseRotation,rect.left+1000,rect.top+y_PhaseRotation+25};
						DrawText(hDC, "Послідовність фаз", 18, &rectak, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_cosinus_factor!=0){
						RECT rectcf1 = {rect.left+210, rect.top+y_cosinus_factor,rect.left+1000,rect.top+y_cosinus_factor+25};
						DrawText(hDC, "Коефіцієнт потужності(косинус фі), %", 37, &rectcf1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_unbalance_factor!=0){
						RECT rectub1 = {rect.left+210, rect.top+y_unbalance_factor,rect.left+839+140,rect.top+y_unbalance_factor+25};
						DrawText(hDC, "Коефіцієнт потужності(асиметрія), %", 36, &rectub1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_harm_factor!=0){
						RECT rectfh = {rect.left+210, rect.top+y_harm_factor,rect.left+839+140,rect.top+y_harm_factor+25};
						DrawText(hDC, "Коефіцієнт потужності(гармоніки), %", 36, &rectfh, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_ActivePower_kW!=0){
						RECT rectap = {rect.left+210, rect.top+y_ActivePower_kW,rect.left+839+140,rect.top+y_ActivePower_kW+25};
						DrawText(hDC, "Потужність активна, кВт", 24, &rectap, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_ReactivePower_kW!=0){
						/* Реактивна потужність*/
						RECT rectrap = {rect.left+210, rect.top+y_ReactivePower_kW,rect.left+839+140,rect.top+y_ReactivePower_kW+25};
						DrawText(hDC, "Потужність реактивна, кВт", 26, &rectrap, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_UnbalanceLostPower_kW!=0){
						/* Потужність втрачена електродвигуном внаслідок асиметрії струму (лише для електродвигуна)*/
						RECT rectrap2 = {rect.left+210, rect.top+y_UnbalanceLostPower_kW,rect.left+839+140,rect.top+y_UnbalanceLostPower_kW+25};
						DrawText(hDC, "Потужність, втрати через асиметрію, кВт", 40, &rectrap2, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_HarmonicLostPower_kW!=0){
						/* Потужність, втрачена електродвигуном внаслідок несинусоїдальності струму */
						RECT rectrap3 = {rect.left+210, rect.top+y_HarmonicLostPower_kW,rect.left+839+140,rect.top+y_HarmonicLostPower_kW+25};

						DrawText(hDC, "Потужність, втрати через гармоніки, кВт", 40, &rectrap3, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_power!=0){
						/*kvatt_t	ActivePower_kW*/
						RECT rectap1 = {rect.left+210, rect.top+y_power,rect.left+839+140,rect.top+y_power+25};
						DrawText(hDC, "Потужність активна, кВт", 24, &rectap1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_aTotalHarmonicDistortion!=0){

						/*Зглажені параметри ефективності мережі 					                    */
						/*Коефіцієнт нелінійних спотворень по струму у кожній фазі                      */
						//aTotalHarmonicDistortion
						RECT rectah = {rect.left+210, rect.top+y_aTotalHarmonicDistortion,rect.left+839+140,rect.top+y_aTotalHarmonicDistortion+25};
						DrawText(hDC, "Коефіцієнт нелінійних спотворень, фаза А, %", 44, &rectah, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_bTotalHarmonicDistortion!=0){
						//bTotalHarmonicDistortion
						RECT rectbh = {rect.left+210, rect.top+y_bTotalHarmonicDistortion,rect.left+839+140,rect.top+y_bTotalHarmonicDistortion+25};
						DrawText(hDC, "Коефіцієнт нелінійних спотворень, фаза В, %", 44, &rectbh, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_cTotalHarmonicDistortion!=0){
						//cTotalHarmonicDistortion
						RECT rectch = {rect.left+210, rect.top+y_cTotalHarmonicDistortion,rect.left+839+140,rect.top+y_cTotalHarmonicDistortion+25};
						DrawText(hDC, "Коефіцієнт нелінійних спотворень, фаза С, %", 44, &rectch, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_THDi_HarmonicAmperageDistortion!=0){
						//Коефіцієнт нелінійних спотворень по струму узагальнений  (макс)
			//			/*procentf_t THDi_HarmonicAmperageDistortion*/
						RECT rectth = {rect.left+210, rect.top+y_THDi_HarmonicAmperageDistortion,rect.left+839+140,rect.top+y_THDi_HarmonicAmperageDistortion+25};
						DrawText(hDC, "Коефіцієнт нелінійних спотворень, %", 36, &rectth, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_aNegativeAmperage_A!=0){
						RECT rectan = {rect.left+210, rect.top+y_aNegativeAmperage_A,rect.left+839+140,rect.top+y_aNegativeAmperage_A+25};
						DrawText(hDC, "Струми негативних гармонік, фаза А, А", 38, &rectan, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_bNegativeAmperage_A!=0){
			//			bNegativeAmperage_A*/
						RECT rectbn = {rect.left+210, rect.top+y_bNegativeAmperage_A,rect.left+839+140,rect.top+y_bNegativeAmperage_A+25};
						DrawText(hDC, "Струми негативних гармонік, фаза В, А", 38, &rectbn, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_cNegativeAmperage_A!=0){
			//			amperf_t cNegativeAmperage_A*/
						RECT rectcn = {rect.left+210, rect.top+y_cNegativeAmperage_A,rect.left+839+140,rect.top+y_cNegativeAmperage_A+25};
						DrawText(hDC, "Струми негативних гармонік, фаза С, А", 38, &rectcn, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_aNeutralAmperage_A!=0){
						RECT rectnaa = {rect.left+210, rect.top+y_aNeutralAmperage_A,rect.left+839+140,rect.top+ y_aNeutralAmperage_A+25};
						DrawText(hDC, "Струми нейтральних гармонік, фаза А, А", 38, &rectnaa, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_bNeutralAmperage_A!=0){
			//			bNeutralAmperage_A
						RECT rectnab = {rect.left+210, rect.top+ y_bNeutralAmperage_A,rect.left+839+140,rect.top+ y_bNeutralAmperage_A+25};
						DrawText(hDC, "Струми нейтральних гармонік, фаза В, А", 38, &rectnab, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_cNeutralAmperage_A!=0){
			//			cNeutralAmperage_A
						RECT rectnac = {rect.left+210, rect.top+ y_cNeutralAmperage_A,rect.left+839+140,rect.top+y_cNeutralAmperage_A+25};
						DrawText(hDC, "Струми нейтральних гармонік, фаза С, А", 38, &rectnac, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_sumNeutralAmperage_A!=0){
			//			sumNeutralAmperage_A
						RECT rectnat = {rect.left+210, rect.top+ y_sumNeutralAmperage_A,rect.left+839+140,rect.top+ y_sumNeutralAmperage_A+25};
						DrawText(hDC, "Струми нейтральних гармонік разом, А", 37, &rectnat, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_AmperageUnbalance!=0){						            // 1234567890123456789012345678901234567890123456789
						RECT rectnkf = {rect.left+210, rect.top+ y_AmperageUnbalance,rect.left+839+140,rect.top+ y_AmperageUnbalance+25};
						DrawText(hDC, "Перекос фаз, %", 15, &rectnkf, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_leakFurie!=0){
						RECT rectnlf = {rect.left+210, rect.top+ y_leakFurie,rect.left+839+140,rect.top+ y_leakFurie+25};
						            // 1234567890123456789012345678901234567890123456789
						DrawText(hDC, "Витік струму на землю(Ф), А", 28, &rectnlf, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    if(y_leakFuriePhase!=0){
						RECT rectnlfp = {rect.left+210, rect.top+ y_leakFuriePhase,rect.left+839+140,rect.top+ y_leakFuriePhase+25};
						DrawText(hDC, "Фаза витоку(Ф)", 15, &rectnlfp, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    }
	    /*************************************************************************************************
	     *
	     ************************************************************************************************/
		   /* Вручну малюємо коробочку для ручного прогрес-бара */
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
	    	DrawText(hDC, "Нагрів", 6, &eHeating3, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	RECT rectblueHeating3 = {rect.left+20+103,rect.top+680,rect.left+190,rect.top+700};
	    	DrawText(hDC, ">100%", 6, &rectblueHeating3, DT_SINGLELINE|DT_LEFT|DT_TOP);





			/* Доска для напису адреси Модбас  */

	    	RECT rect_EditAddress = {rect.left+x_right_panel+50, rect.top+75, rect.left+x_right_panel+50+60, rect.top+75+25};
	    	DrawText(hDC, "Адреса", 7, &rect_EditAddress, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	RECT rect_EditAddress1 = {rect.left+x_right_panel+50, rect.top+90, rect.left+x_right_panel+50+60, rect.top+90+25};
	    	DrawText(hDC, "Модбас", 8, &rect_EditAddress1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	/* Розмальовування ПроцесБарів справа знизу - Нагрівання електродвигуна */
	    	//910, 680, 150, 20,
	    	RECT recttr = {rect.left+x_right_panel, rect.top+535, rect.left+x_right_panel+150, rect.top+660+20};
	    	DrawText(hDC, "МОДБАС:", 7, &recttr, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	RECT rectt9 = {rect.left+x_right_panel, rect.top+560, rect.left+x_right_panel+150, rect.top+660+20};
	    	DrawText(hDC, "Повідомлень:", 12, &rectt9, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	RECT recttg = {rect.left+x_right_panel, rect.top+610, rect.left+x_right_panel+150, rect.top+660+20};
	    	DrawText(hDC, "Успіх:", 7, &recttg, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	RECT recttj = {rect.left+x_right_panel, rect.top+635, rect.left+x_right_panel+150, rect.top+660+20};
	    	DrawText(hDC, "Зайнятість:", 12, &recttj, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	RECT rectty = {rect.left+x_right_panel, rect.top+660, rect.left+x_right_panel+150, rect.top+660+20};
	    	DrawText(hDC, "Черга:", 7, &rectty, DT_SINGLELINE|DT_LEFT|DT_TOP);

//1390
	    	  RECT rect1 = {rect.left+1010, rect.top+yDebugMode-20,rect.right,rect.top+25+yDebugMode};
	    	  DrawText(hDC, "Симулятор?", 11, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  RECT rect2 = {rect.left+1010, rect.top+yAmperageAmplitude-20,rect.right,rect.top+25+yAmperageAmplitude};
	    	  DrawText(hDC, "Струм", 6, &rect2, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  RECT rect3 = {rect.left+1010, rect.top+yBAmperageAmplitude-20,rect.right,rect.top+25+yBAmperageAmplitude};
	    	  DrawText(hDC, "Струм B", 8, &rect3, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  RECT rect4 = {rect.left+1010, rect.top+yCosFi-20,rect.right,rect.top+25+yCosFi};
	    	  DrawText(hDC, "Косинус", 8, &rect4, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  RECT rect5 = {rect.left+1010, rect.top+yTk4screenMode-20,rect.right,rect.top+25+yTk4screenMode};
	    	  DrawText(hDC, "Екран ТК412", 12, &rect5, DT_SINGLELINE|DT_LEFT|DT_TOP);











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
		/* ІНДИКАЦІЯ */
		case WM_COMMAND:{
			char wmc[256]={0};
//			 дскриптор дочер. окна If(idCtrl==ID_combo&&code==CBN_SELCHANGE)
			UINT NotificationCode = HIWORD(wParam);
			switch (NotificationCode) {
				/* якщо виявлено вибір користувача*/
				case CBN_SELCHANGE:{
					/* Запит - який саме номер по порядку з меню вибраний (незалежно від комбобоксу */
					int ItemIndex = SendMessage((HWND) lParam, (UINT) CB_GETCURSEL,  (WPARAM) 0, (LPARAM) 0);
					UINT idCtl=LOWORD(wParam);
					/* Переносимо зроблений вибір у заголовок комбобокса*/
					/* Ой.. Здається система це робить сама :)*/
					// SendMessage((HWND)lParam, CB_SETCURSEL, idCtl, 0);
					/* в залежності від параметру приймаємо вибрані значення параметрів*/
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
//					 /* Збираємо повідомлення для запису регістра */
					 f_Set5TxWriteReg(0x0000,0xFF,&mbTxMsg);
//					 /* Ставимо повідомлення в чергу на обробку   */
					  f_set_tkqueue(
							&tk5Queue,
							DISABLE,
							hwnd,
							0x0300,
							&mbTxMsg,
							600
							);
					 snprintf(tk5LogStr,511,"КО: ПУСК ЕД");
				   }else{snprintf (wmc,255," Не натискайте так часто на кнопки! ");}
				 }else{snprintf (wmc,255," Процес дистанційного управління ще не запущено ");}
			}
			break;
			case IDB_tk5BtnSTOP:{
				 if(tk5Process==ProcessBusy){
				  if(tk5btn_access==ENABLE){
					 tk5btn_access=DISABLE;
					 /* Збираємо повідомлення для запису регістра */
					 f_Set5TxWriteReg(0x0000,0x00,&mbTxMsg);
					 /* Ставимо повідомлення в чергу на обробку   */
					 f_set_tkqueue(
							&tk5Queue,
							DISABLE,
							hwnd,
							0x0000,
							&mbTxMsg,
							600
							);

					 snprintf(tk5LogStr,511,"КО: СТОП ЕД");
				  }else{snprintf (wmc,255," Не натискайте так часто на кнопки! ");}
				 }else{snprintf (wmc,255," Процес дистанційного управління ще не запущено ");}

			}
			break;


			case IDM_tk5USER_SETTINGS:{
//				if(tk5Process==ProcessBusy){
				/* Забороняємо відкривати відразу декілька вікон користувача
				 * Якщо вікно користувача вже відкрите - нічого не робимо*/
				   if(!IsWindow(hWndtk5usCh)){
//					   /* Перше ніж перейти в режим налаштувань -
//					   * відключим періодичне опитування основних параметрів */
//							/* Зупиняємо процес обміну повідомленнями DeskTop <-> tk5 */
							//KillTimer(hwnd, p1000Timer);
//							/* Чистимо чергу Модбас */
							f_clear_tkqueue(&tk5Queue);
//					    /* Створюємо вікно налаштувань користувача */
							hWndtk5usCh=f_CreateTk5usWnd(hwnd);
////						snprintf(tk5LogStr,511,"ко: вкл. Налаштування користувача");
				   }//if(!IsWindow(usWndChild)){
//				}
//				else{
//					snprintf (wmc,255," Процес дистанційного управління ще не запущено ");
//				}

				}//case IDM_USER_SETTINGS:{
			break;
			case	IDM_tk5FACILITY_SETTING:{
				if(!IsWindow(hWndTk5fsCh)){
					//KillTimer(hwnd, p1000Timer);
//							/* Чистимо чергу Модбас */
					f_clear_tkqueue(&tk5Queue);
//					    /* Створюємо вікно налаштувань користувача */

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
					snprintf(tk5LogStr,511,"КО: Налаштування ком-порта");
				}
				break;
			case IDB_tk5TEST_DISTANCE_CTRL:{
				  /* Тестуємо процес обміну повідомленнями DeskTop <-> tk5 */
				  /* Формуємо тестове повідомлення*/
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
//				/* Стираємо чергу */
				f_clear_tkqueue(&tk5Queue);
				/* Кидаємо тестове повідомлення в пусту чергу */
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
//				/* виставляємо прапор для обробки поверненого ТК2 повідомлення */
//				 RW5_case=IDB_tk5TEST_DISTANCE_CTRL;
//				 snprintf(tk5LogStr,511,"КО: Тестування дистанційного управління ");
//			  	  char str[256]={0};
//			  	  snprintf (str,sizeof(str)," a20_tk5.c: Сформовано тестовий запит. Кинуто в чергу");
//			  	  f_tk5Logging(str, sizeof(str));
			}
			break;
			case IDB_tk5START_DISTANCE_CTRL:{
				  /* Запускаємо процес обміну повідомленнями DeskTop <-> tk5 */
				  if(tk5Process==ProcessBusy){
					  /* Стираємо чергу */
					  f_clear_tkqueue(&tk5Queue);
				  }
				  else if(tk5Process==ProcessIddle){
			      /* Виставляємо прапор включеного дистанційного управління */
					  tk5Process=ProcessBusy;
//					  /* Запускаємо таймер, де щосекунди формуюють і ставляться у чергу повідомлення */
					  p1000Timer=SetTimer(
						hwnd,                // handle to main window
					    IDT_TIMER_tk5_1000MS,// timer identifier
						1000,                // msecond interval
					    (TIMERPROC) NULL     // no timer callback
						);
//
//				  /* Для лівого прогрес-бару робимо запит на Номінальний струм */
//					/* Запит подача/відкачка/датчики */
//					modbus_master_tx_msg_t mbTxMsg;
//					memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
//					/* Запит маkc.струму */
////					f_Set112TxReadReg(RG_R_tk5_MAX_AMPERAGE,&mbTxMsg);
////					  f_set_tkqueue(
////							    &tk5Queue,
////							DISABLE,
////							hwnd,
////							ID_READ_US_MAX_AMPERAGE,
////							&mbTxMsg,
////							600
////							);
					snprintf(tk5LogStr,511,"КО: СТАРТ дистанційного управління ");
				  }
				  else{}
//				  /* Запускаємо калькуляцію процента зайнятості каналу Модбас */
				  f_TxRxBusyDegreeCalcStart();
//
			}
			break;
			case IDB_tk5STOP_DISTANCE_CTRL:{
				if(tk5Process==ProcessBusy){
//					/* Зупиняємо процес обміну повідомленнями DeskTop <-> tk5 */
					KillTimer(hwnd, p1000Timer);
					f_clear_tkqueue(&tk5Queue);
					snprintf(tk5LogStr,511,"КО: СТОП дистанційного управління\n ");
//
					tk5Process=ProcessIddle;
					char r[256]={0};
					/* Зупиняємо калькуляцію процента зайнятості каналу Модбас */
					f_TxRxBusyDegreeCalcStop(r, sizeof(r));
					snprintf(tk5LogStr,511,"КО: СТОП дистанційного управління");
					strcat(tk5LogStr,r);
					f_tk5Logging(tk5LogStr, sizeof(tk5LogStr));
					snprintf(tk5LogStr,511,"СТАТИСТИКА Модбас за час дистанційного управління");
					f_tk5Logging(tk5LogStr, sizeof(tk5LogStr));
					snprintf(tk5LogStr,511,"%d.%2d %% - Процент зайнятості каналу ",
						(int)(gTxRxBusyAveDegreef*100.0f)/100,
						(int)(gTxRxBusyAveDegreef*100.0f)%100);
					f_tk5Logging(tk5LogStr, sizeof(tk5LogStr));

					snprintf(tk5LogStr,511,"%d - Кількість надісланих повідомлень",
						(int)gTxRxSessionStartCntr);
					f_tk5Logging(tk5LogStr, sizeof(tk5LogStr));

					snprintf(tk5LogStr,511,"%d.%2d %% - Cесія завершилися без помилок",
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
//					/* Збираємо повідомлення для запису регістра */
//					f_Set112TxWriteReg(RG_W_RESET_MOTOR_PROTECTION_1,RG_V_RESET_MOTOR_PROTECTION,&mbTxMsg);
//					/* Ставимо повідомлення в чергу на обробку   */
//					f_set_tkqueue(
//						&tk5Queue,
//						ENABLE,
//						hwnd,
//						IDB_RESET_MOTOR_PROTECTION_1,
//						&mbTxMsg,
//						600
//						);
					snprintf(tk5LogStr,511,"КО: Скидання аварії  ");
				 }else{snprintf (wmc,255," Не натискайте так часто на кнопки! ");}
				}else{snprintf (wmc,255," Процес дистанційного управління ще не запущено ");}
		    }
		    break;
			case IDB_tk5RESET_MOTOR_PROTECTION_2:{
				if(tk5Process==ProcessBusy){
				 if(tk5btn_access==ENABLE){
					 tk5btn_access=DISABLE;
//					/* Збираємо повідомлення для запису регістра */
//					f_Set112TxWriteReg(RG_W_RESET_MOTOR_PROTECTION_2,RG_V_RESET_MOTOR_PROTECTION,&mbTxMsg);
//					/* Ставимо повідомлення в чергу на обробку   */
//					f_set_tkqueue(
//						&tk5Queue,
//						ENABLE,
//						hwnd,
//						IDB_tk5RESET_MOTOR_PROTECTION_2,
//						&mbTxMsg,
//						600
//						);
					snprintf(tk5LogStr,511,"КО: Скидання обмежень скидання аварії  ");
				 }else{snprintf (wmc,255," Не натискайте так часто на кнопки! ");}
				}else{snprintf (wmc,255," Процес дистанційного управління ще не запущено ");}
		    }
		    break;
			case IDB_YDEBUGMODE:{
				/* Збираємо повідомлення для запису регістра */
				modbus_master_tx_msg_t mbTxMsg={0};
				f_Set5TxWriteReg(0xE001,DebugMode,&mbTxMsg);
				/* Ставимо повідомлення в чергу на обробку   */
				f_set_tkqueue(
						&tk5Queue,
						DISABLE,
						hwnd,
						0xE001,
						&mbTxMsg,
						1000
						);
				/* Формуємо тестове повідомлення*/
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
					/* Зчитуємо значення струму, це , це завжди HEX*/
					float2_t wItk5={0};
					int errWM_C=f_checkEdit(IDE_YHAMPERAGEAMPLITUDE, hwnd, &wItk5);
					if(errWM_C==0){
						uint16_t value = wItk5.d;

						/* Збираємо повідомлення для запису регістра */
						modbus_master_tx_msg_t mbTxMsg={0};
						f_Set5TxWriteReg(0xE002,value,&mbTxMsg);
						/* Ставимо повідомлення в чергу на обробку   */

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
					/* Зчитуємо значення струму, це , це завжди HEX*/
					float2_t wItk5={0};
					int errWM_C=f_checkEdit(IDE_YBAMPERAGEAMPLITUDE, hwnd, &wItk5);
					if(errWM_C==0){
						uint16_t value = wItk5.d;

						/* Збираємо повідомлення для запису регістра */
						modbus_master_tx_msg_t mbTxMsg={0};
						f_Set5TxWriteReg(0xE003,value,&mbTxMsg);
						/* Ставимо повідомлення в чергу на обробку   */

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
					/* Зчитуємо значення струму, це , це завжди HEX*/
					float2_t wItk5={0};
					int errWM_C=f_checkEdit(IDE_YCOSFI, hwnd, &wItk5);
					if(errWM_C==0){
						uint16_t value = wItk5.d;
						if(value>100){value=100;}
						/* Збираємо повідомлення для запису регістра */
						modbus_master_tx_msg_t mbTxMsg={0};
						f_Set5TxWriteReg(0xE004,value,&mbTxMsg);
						/* Ставимо повідомлення в чергу на обробку   */

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
				/* Збираємо повідомлення для запису регістра */
				modbus_master_tx_msg_t mbTxMsg={0};
				f_Set5TxWriteReg(0xE000,Tk4screenMode,&mbTxMsg);
				/* Ставимо повідомлення в чергу на обробку   */
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
			/* Нейтралізація натискання користувачем кнопок -індикаторів               */
		    /***************************************************************************/
			case IDB_tk5SHOW_MOTOR_OPERATING:
				  /* Захист від натискання радіокнопку, яка лише індикатор, користувачем*/
//		    	  switch(g5.d_MotorState){
//		    	  case oFF_:SendMessage(hWndViewBtnMotorState, BM_SETCHECK, 0, 0L);break;
//		    	  case oN_: SendMessage(hWndViewBtnMotorState, BM_SETCHECK, 1, 0L);break;
//		    	  default:{}
//		    	  }
			break;
			case IDB_tk5SHOW_EMP_AUTOCTR1:
				 /* Захист від натискання радіокнопку, яка лише індикатор, користувачем*/
//		    	  switch(g5.d_AutoCtrl){
//		    	  case oFF_:SendMessage(hBtnViewAutoctrl, BM_SETCHECK, 0, 0L);break;
//		    	  case oN_: SendMessage(hBtnViewAutoctrl, BM_SETCHECK, 1, 0L);break;
//		    	  default:{}
//		    	  }
		   break;
	  	   case IDB_tk5SHOW_CX:
	  		    /* Захист від натискання радіокнопку, яка лише індикатор, користувачем*/
//	  	    	  switch(g5.d_CX){
//	  	    	  case oFF_:SendMessage(hBtnViewCX, BM_SETCHECK, 0, 0L);break;
//	  	    	  case oN_ : SendMessage(hBtnViewCX, BM_SETCHECK, 1, 0L);break;
//	  	    	  default:{}
//	  	    	  }
	  	   break;
		   default:{}
		   }/* end switch param*/
			/* Error Message Box */
		   if(strlen(wmc) != 0){ MessageBox(hwnd, wmc, TEXT("Увага ! "), MB_ICONERROR | MB_OK);}
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
				  /* Формуємо тестове повідомлення*/
				  tk5_Addr= f_GetTk5ModbussAddress(ID_tk5_EDIT_ADDRESS);
				  mbTxMsg.msg[0]=tk5_Addr;
				  mbTxMsg.msg[1]=0x03;
				  mbTxMsg.msg[2]=0x01;
				  mbTxMsg.msg[3]=0x00;
				  mbTxMsg.msg[4]=0x00;
				  mbTxMsg.msg[5]=0x40;
				  mbTxMsg.length=6;
				  /* Ставимо повідомлення в чергу на обробку   */
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
			  /* Організація сеансу Модбас після натискання користувачем певної кнопки
			   * з затримкою на секунду з третиною */
			  case IDT_TIMER_tk5_1333MS:{
				  /* Сюди ж чіпляємо ще одну функцію - успіх Модбас..*/
				  /* Якщо процент успішно переданих повідомлень низький протягом 10 сек - зупиняємо і повідомляємо */
				  if(met_zero_MBsuccess == EVENT){
					  if(gTxRxSessionSuccessDegreef<10.0f){
						tk5ProcessState=0;
						/* Якщо дист.управління вже було запущено, вбиваємо таймер,
						*  на період виправлення оператором несправності,
						*  тобто ліквідовуємо періодичні запити до тк5   */
						if(tk5Process==ProcessBusy){
							KillTimer(hwnd, p1000Timer);
							KillTimer(hwnd, p1333Timer);
							f_clear_tkqueue(&tk5Queue);
							snprintf(tk5LogStr,511,"ТК5 не відповідає\n "
								"Дистанційне управління зупинено ");

							tk5Process=ProcessIddle;
						}
						const int result = MessageBox(NULL, "ТК5 не відповідає\n "
								"Дистанційне управління зупинено", "Увага!",MB_ICONEXCLAMATION | MB_OK);
						switch (result){
						case IDOK:
							/* Відновлюємо таймер назад */
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

					 /* Поки що виводимо повідомлення про помилку в файл-протокол,
					  * для чого перепишемо вміст рядка в рядок логів   */
						memcpy(&tk5LogStr, "ТК5 не відповідає", 256);
				   }// if (gTxRxSessionSuccessDegreef<10.0f){
				   met_zero_MBsuccess = NO_EVENT;
				  }
			  	KillTimer(hwnd, p1333Timer);
			  }
			  break;
			  case IDT_TIMER_tk5_TEST:{
				  /* виставляємо прапор для обробки поверненого ТК5 повідомлення */
//				  if( RW5_case==IDB_tk5TEST_DISTANCE_CTRL){
					  HDC hDCr; // создаём дескриптор ориентации текста на экране
					  RECT rect; // стр-ра, определяющая размер клиентской области
					  /*малюємо заново*/
					  hDCr= GetDC(hwnd);
					  GetClientRect(hwnd, &rect);  	// получаем ширину и высоту области для рисования
					  /* Стираємо тестове повідомлення */
					  RECT rectht = {rect.left+839, rect.top+25,rect.left+910+140,rect.top+50};
//				  	  RECT rectht = {rect112.left+910, rect112.top+25,rect112.left+910+140,rect112.top+50};
					  FillRect(hDCr, &rectht, (HBRUSH)(COLOR_WINDOW+1));
					  ReleaseDC(hwnd, hDCr);
					  KillTimer(hwnd, pTestTimer);
				  	  char str[256]={0};
				  	  snprintf (str,sizeof(str)," a20_tk5.c: Стерто тестове повідомлення ");
				  	  f_tk5Logging(str, sizeof(str));
//				 }
			  }
			  break;
			  case IDT_TIMER_tk5_10MS:{
				  /* Якщо у вікні натиснута кнопка - вмикаємо лічильник затримки */
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
			/* * WPARAM wParam - RW_case, дентифікатор типу конкретного повідомлення */
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

/* Ця функція викликається кожен раз при отриманні повідомлення в VK_UPDATE_WINDOW
 * про завершення сесії Модбас, ініційованої з основного вікна*/
static int f_tk5UpdateWindow(HWND hwnd, int wParam, int responce_status){
	uint16_t x_value = 560;
//   /***********************************************************************************************************
//   * РОБОТА З ЧЕРГОЮ ПОВІДОМЛЕНЬ МОДБАС
//   ***********************************************************************************************************/
	f_tk5UpdateStat(
			hwnd,
			hWndProgressBar_Queue,
			p1000Timer,
			IDT_TIMER_tk5_1000MS
			);
	/***********************************************************************************************************
	 * ЯКЩО ПОМИЛКА ПОВІДОМЛЕННЯ
	 ***********************************************************************************************************/
    /* При помилці МОДБАС перевіряємо, чи це систематична помилка,
     * якщо за 10 сек ситуація не змінилася - виводимо повідомлення */
	if(tk5Process==ProcessBusy){
		if (gTxRxSessionSuccessDegreef<10.0f){
		  /* Запускаємо таймер, щоб стерти тестове повідомлення через пару секунд */
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
   	 	 /* Якщо помінялася фазність або двигун/не двигун*/
   		if(((uint16_t)tk5fs.Faznost!=PhasnostOld)||((uint16_t)Tk5us.oper_mode!=LoadModeOld)){
   			Init_YMode(
					(uint16_t)tk5fs.Faznost,
					(uint16_t)Tk5us.oper_mode);
   			/* Стираємо текст - назви і значення параметрів
   			 * Другкуємо нові назви параметрів */
   			f_tk5UpdateValuesNames(hwnd);
   			  /* Центральна доска для параметрів */

		    	PhasnostOld=(uint16_t)tk5fs.Faznost;
		    	LoadModeOld=(uint16_t)Tk5us.oper_mode;
   		}
    }

//	char pm[80]={0};
	/* рядок динамічно для підготовки тексту у вікно*/
	HDC hDCr; // создаём дескриптор ориентации текста на экране
	RECT rect; // стр-ра, определяющая размер клиентской области
	/*малюємо заново*/
	hDCr= GetDC(hwnd);
	GetClientRect(hwnd, &rect);  	// получаем ширину и высоту области для рисования
	/***********************************************************************************************************
	 * ЯКЩО НЕМАЄ ПОВІДОМЛЕННЯ ДЛЯ ОНОВЛЕННЯ ВІКНА
	 ***********************************************************************************************************/
//	if (tk5ProcessState==0){
//		/* Якщо це той самий сеанс - нічого не робимо */
//		return 0;
//	}



//	if(tk5ProcessState==-1){
		if(wParam==IDB_tk5TEST_DISTANCE_CTRL){
			char str[256]={0};
			snprintf (str,sizeof(str)," a20_tk5.c: З обробника черги отримано відповідь про помилку тестового сеансу");
			f_tk5Logging(str, sizeof(str));
		}
		/* Усі повідомлення, крім повідомлення про неналаштований СОМ-порт блокуємо.*/
		/* Вони корисні при відлагодженні, але шкідливі при роботі */
		/* При помилці Модбас користуємося низьким показником успіху Модбас    */
		/* Якщо показник успіху Модбас низький  - виводимо помилку користувачу
		 * та припиняємо дистанційний режим */
		if(strlen(user_msg5)!=0){
			tk5ProcessState=0;
//			{
				/* Якщо дист.управління вже було запущено, вбиваємо таймер,
				 * тобто ліквідовуємо періодичні запити до тк2   */
//				if(tk5Process==ProcessBusy){KillTimer(hwnd, p1000Timer);}
//				const int result = MessageBox(NULL, user_msg5, "Увага!",MB_ICONEXCLAMATION | MB_OK);
//				switch (result){
//				case IDOK:
//				/* Відновлюємо таймер назад */
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

				/* Поки що виводимо повідомлення про помилку в файл-протокол,
				 * для чого перепишемо вміст рядка в рядок логів   */
				memcpy(&tk5LogStr, &user_msg5, sizeof(tk5LogStr));
				/* Стираємо рядок повідомлення про помилку користувачу*/
//				memset(&user_msg5,0,sizeof(user_msg5));
		}//if(strlen(user_msg112)!=0){

//	}
	/***********************************************************************************************************
	 * ЯКЩО ЦЕ ТЕСТУВАННЯ ЗВЯЗКУ З ТК2
	 ***********************************************************************************************************/
//	else if(tk5ProcessState==2){
		if(wParam==IDB_tk5TEST_DISTANCE_CTRL){
		tk5ProcessState=0;
		 if(strlen(user_msg5) !=0){
			/* Якще це було тестування - виводимо тестове повідомлення*/
			RECT rectht = {rect.left+839, rect.top+25,rect.left+839+140,rect.top+50};
			DrawText(hDCr, user_msg5, 19, &rectht, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&user_msg5,0,sizeof(user_msg5));
			/* Запускаємо таймер, щоб стерти тестове повідомлення через пару секунд */
			pTestTimer=SetTimer(
					hwnd,                // handle to main window
					IDT_TIMER_tk5_TEST,// timer identifier
					TIMER_AUX_MS,        // msecond interval
					(TIMERPROC) NULL     // no timer callback
			);
			test_answer_cntr++;
			char str[256]={0};
				snprintf (str,sizeof(str),"a20_tk5.c: Реакція на відповідь %d від ТК5. Виведено 'tk5', запущено таймер", test_answer_cntr);
			f_tk5Logging(str, sizeof(str));
		}
	  }
//	}
	/***********************************************************************************************************
	 * ЯКЩО ЦЕ ДИСТАНЦІЙНЕ УПРАВЛІННЯ
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
			/* Зглажена напруга в вольтах*/
			RECT rectvl = {rect.left+x_value, rect.top+y_voltage,rect.left+839+140,rect.top+y_voltage+25};
			snprintf (str,sizeof(str),"%4d.%01d",
			grid_m16[4],	/*dc(g->Voltage_V)*/		grid_m16[5]/1000  /*fr(g->Voltage_V)*/
			);
			DrawText(hDCr, str, 19, &rectvl, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_aAmperage != 0){
			/* Зглажені струми в амперах */
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
			/* Процент струму, що витікає на землю
		 	 * через замикання на землю, обчислений по сумах відліків струму,
		 	 * те саме що leakAmperage_A, але в процентах від номінального струму*/
			grid_m16[0xE],/*dc(g->GrundAmperageDistortion)*/	grid_m16[0xF]/100/*fr(g->GrundAmperageDistortion)*/
			);
			DrawText(hDCr, str, 19, &rectad, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_LoadType != 0){
			/* >LoadType - Тип навантаження - індуктивне/активне/ємнісне*/
			RECT recttl = {rect.left+x_value, rect.top+y_LoadType,rect.left+839+140,rect.top+y_LoadType+25};
			if(grid_m16[0x10]==2){
				snprintf (str,sizeof(str),"Індуктивне");
			}
			else if(grid_m16[0x10]==3){
				snprintf (str,sizeof(str),"Ємнісне");
			}
			else if(grid_m16[0x10]==1){
				snprintf (str,sizeof(str),"Активне");
			}
			else{}
			DrawText(hDCr, str, 19, &recttl, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_PhaseRotation != 0){
			/*(uint16_t)(g->PhaseRotation)*/
			RECT rectak = {rect.left+x_value, rect.top+y_PhaseRotation,rect.left+839+140,rect.top+y_PhaseRotation+25};
			if(grid_m16[0x11]==1){
				snprintf (str,sizeof(str),"Прямий");
			}
			else if(grid_m16[0x11]==2){
				snprintf (str,sizeof(str),"Зворотний");
			}
			else if(grid_m16[0x11]==3){
				snprintf (str,sizeof(str),"Аварія фаз");
			}
			else if(grid_m16[0x11]==4){
				snprintf (str,sizeof(str),"Аварія фаза А");
			}
			else if(grid_m16[0x11]==5){
				snprintf (str,sizeof(str),"Аварія фаза В");
			}
			else{}
			DrawText(hDCr, str, 19, &rectak, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_cosinus_factor != 0){


			/* Косинус фі*/
			RECT rectcf = {rect.left+x_value, rect.top+y_cosinus_factor,rect.left+839+140,rect.top+y_cosinus_factor+25};


			snprintf (str,sizeof(str),"%3d.%04d",
			grid_m16[0x12]/*dc(g->PowerFactor_pro)*/,grid_m16[0x16]
			);
			DrawText(hDCr, str, 19, &rectcf, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_unbalance_factor != 0){
			/* Тільки для електродвигуна, якщо навантаження не двигун - не має сенсу
			NEMA (National Electric Manufacturers Associations of the
			USA) Std. (1993) - визначення коефіцієнта асиметрії:
			найбільше відхилення від середнього значення
			https://wp.kntu.ac.ir/tavakoli/pdf/Journal/j.ijepes.2010.12.003.pdf
			Падіння потужності - через формулу асиметрії напруги              */
			RECT rectub = {rect.left+x_value, rect.top+y_unbalance_factor,rect.left+839+140,rect.top+y_unbalance_factor+25};
			snprintf (str,sizeof(str),"%3d",
			grid_m16[0x13]/*dc(g->UnbalanceDeratingFactor_pro)*/
			);
			DrawText(hDCr, str, 19, &rectub, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_harm_factor != 0){
		    /* Коефіцієнт ефективності, якщо 99%, то один процент ефективності втратили через гармоніки
		     * також має сенс лише для електродвигуна  */
			RECT rectfh = {rect.left+x_value, rect.top+y_harm_factor,rect.left+839+140,rect.top+y_harm_factor+25};
			snprintf (str,sizeof(str),"%3d.%04d",
			grid_m16[0x14],grid_m16[0x15]/*dc(g->HarmonicsDeratingFactor_pro)*/
			);
			DrawText(hDCr, str, 19, &rectfh, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_AmperageUnbalance != 0){
			/* Зглажені інтегральні показники ефективності мережі, також лише для двигуна  */



//			grid_m16[0x16]/*dc(g->fullDerating_pro)*/
			//y_AmperageUnbalance
//			/* Коефiцієнт перекосу фаз,  має сенс не лише для едектродвигуна	*/
			RECT rectaub = {rect.left+x_value, rect.top+y_AmperageUnbalance,rect.left+839+140,rect.top+y_AmperageUnbalance+25};
			snprintf (str,sizeof(str),"%3d", grid_m16[0x17]);
			DrawText(hDCr, str, 19, &rectaub, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_ActivePower_kW != 0){
			/********************************************************************************************
			 * ПОКАЗНИКИ ПОТУжНОСТІ
			 * ******************************************************************************************/
			/* Активна потужність*/
			RECT rectap = {rect.left+x_value, rect.top+y_ActivePower_kW,rect.left+839+140,rect.top+y_ActivePower_kW+25};
			snprintf (str,sizeof(str),"%4d.%02d",
			grid_m16[0x18],/*dc(g->ActivePower_kW)*/	grid_m16[0x19]/100/*fr(g->ActivePower_kW)*/
			);
			DrawText(hDCr, str, 19, &rectap, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_ReactivePower_kW != 0){
			/* Реактивна потужність*/
			RECT rectrap = {rect.left+x_value, rect.top+y_ReactivePower_kW,rect.left+839+140,rect.top+y_ReactivePower_kW+25};
			snprintf (str,sizeof(str),"%4d.%02d",
			grid_m16[0x1A],	grid_m16[0x1B]/100);
			DrawText(hDCr, str, 19, &rectrap, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_UnbalanceLostPower_kW != 0){

			/* Потужність втрачена електродвигуном внаслідок асиметрії струму (лише для електродвигуна)*/
			RECT rectrap2 = {rect.left+x_value, rect.top+y_UnbalanceLostPower_kW,rect.left+839+140,rect.top+y_UnbalanceLostPower_kW+25};
			snprintf (str,sizeof(str),"%4d.%02d",
			grid_m16[0x1C], grid_m16[0x1D]/100);
			DrawText(hDCr, str, 19, &rectrap2, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_HarmonicLostPower_kW != 0){
			/* Потужність, втрачена електродвигуном внаслідок несинусоїдальності струму */
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

			/*Зглажені параметри ефективності мережі 					                    */
			/*Коефіцієнт нелінійних спотворень по струму у кожній фазі                      */
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
			//Коефіцієнт нелінійних спотворень по струму узагальнений  (макс)
//			/*procentf_t THDi_HarmonicAmperageDistortion*/
			RECT rectth = {rect.left+x_value, rect.top+y_THDi_HarmonicAmperageDistortion,rect.left+839+140,rect.top+y_THDi_HarmonicAmperageDistortion+25};
			snprintf (str,sizeof(str),"%2d.%02d",
			grid_m16[0x26],grid_m16[0x27]/100);
			DrawText(hDCr, str, 19, &rectth, DT_SINGLELINE|DT_LEFT|DT_TOP);
			memset(&str,0,sizeof(str));
	   }
	   if(y_aNegativeAmperage_A != 0){
//			/*THDі вказує на ступінь спотворення форми синусоїди струму.
//			THDi менше 10% вважається нормальним,
//			ризик несправностей обладнання відсутній, перегрівання до 6%.
//
//			THDi між 10% і 50% вказує на значне гармонійне спотворення.
//			Падіння коефіцієнта потужності елекродвигунів, підвищення температури,
//			що означає, що кабелі та установок повинні бути понадгабаритними.
//
//			THDi, що перевищує 50%, вказує на високі гармонічні спотворення.
//			Високий ризик виходу обладнання з ладу. Необхідний поглиблений аналіз причин
//		 	 і заходи з їх усунення, зокрема рекомендується
//		 	 система придушення несинусоїдальнусті струму.
//
//			Скорочення терміну служби обладнання:
//			(Дані Канадської асоціації електриків).
//			Коли спотворення напруги живлення знаходиться в діапазоні 10%,
//			термін служби обладнання значно скорочується.
//			Залежно від типу пристрою, скорочення терміну служби:
//			32,5% для однофазних електродвигунів
//			18% для трифазних електродвигунів
//			5% для трансформаторів.
//		 	 ***************************************************************************
//			3, 5, 7, 11 and 13 - найбільше небезпечні для електродвигунів
//		 	 */
//		    /* 1,4,7,10 3k+1 - діє в тому ж напрямку обертання сіпає двигун, але сприяє зростанню крутного моменту*/
//
//			/* 2, 5, 8, 11   - буде діяти проти напрямку обертання, що призводить до значних пульсацій крутного моменту*/
//
//			/* Струми, які обертають двигун в протилежну сторону*/
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
//			/* 3, 6, 9 .. 3k+3 - нічого не крутить, гріє провідники.
//			 * Струм циркулюює між фазою та нейтраллю чи землею.
//			 * На відміну від позитивної та негативної послідовності гармонійні
//		      струми третього порядку взаємно не компенсуються,
//		      а додадаються арифметично в нейтральній шині.*/
//			/* Струми, що через гармоніки витікають на землю або в нейтраль,
//			 * нагрівають нейтраль
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
//				DrawText(hDCr, "Фаза А", 6, &rectlfp, DT_SINGLELINE|DT_LEFT|DT_TOP);
//			}
//			else if (grid_m16[0x38] == 2){
//				DrawText(hDCr, "Фаза B", 6, &rectlfp, DT_SINGLELINE|DT_LEFT|DT_TOP);
//			}
//			else if (grid_m16[0x38] == 3){
//				DrawText(hDCr, "Фаза C", 6, &rectlfp, DT_SINGLELINE|DT_LEFT|DT_TOP);
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



			 /* Шкала 120% - задаємо в процентах нагрівання електродвигуна*/
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
    		 * ВІДЛАГОДЖЕННЯ
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
    		/* Попугаї тимофійович vS Козак*/
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
	 * КІНЕЦЬ ОБРОБКИ ПОВІДОМЛЕНЬ ДИСТАНЦІЙНОГО УПРАВЛІННЯ
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

/* Лише обгортка для запуску паралельного процесу*/
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
	/* рядок динамічно для підготовки тексту у вікно*/
	HDC hDCr; // создаём дескриптор ориентации текста на экране
	RECT rect; // стр-ра, определяющая размер клиентской области
	/*малюємо заново*/
	hDCr= GetDC(hwnd);
	GetClientRect(hwnd, &rect);  	// получаем ширину и высоту области для рисования
	   /***********************************************************************************************************
	   * РОБОТА З ЧЕРГОЮ ПОВІДОМЛЕНЬ МОДБАС
	   ***********************************************************************************************************/
	    //* Запитуємо довжину черги, включаючи зайнятість RS232*/
		uint16_t iQ= f_get_tkrs232_length(&tk5Queue);
	//	uint16_t iQ=f_get_tk5queue_length();
		int32_t pbQ=10000-10000/(iQ+1);
	//	if(iQ!=iQold){iQold=iQ;}
		float alpha=0.95;
		float MbQ=0;
		if(iQ>0){MbQ=(float)pbQ;}
		else{MbQ=MbQ*alpha;}
		/* Відмальовуємо процес бар зайнятості черги повідомлень Модбас*/

	//	snprintf(tk2LogStr,80,"MbQ = %f", MbQ);
	//	f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));

		SendMessage(hProcesBar, PBM_SETPOS,
				    (int)MbQ, //Signed integer that becomes the new position.
					0);   //Must be zero
		/* Черга */
		snprintf(pm,80,"      %2d", iQ);
		RECT rect_E = {rect.left+x_right_panel+100, rect.top+660, rect.left+x_right_panel+170+100, rect.top+660+20};
		DrawText(hDCr, pm, 9, &rect_E, DT_SINGLELINE|DT_LEFT|DT_TOP);
		memset(pm,0,sizeof(pm));


		if(iQ>40){
			KillTimer(hwnd, pTimer1000);
			KillTimer(hwnd, pTimer10);
			const int result = MessageBox(NULL,
					"Черга повідомлень для Модбас переповнена\nПочистити чергу?",
					"Modbus Error",
			 MB_YESNO);
			switch (result){
			case IDYES:{
				f_clear_tkqueue(&tk5Queue);
			   /* Перезапускаємо періодичні запити */
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
/* Ця функція викликається з 10с таймера і перевіряє чи від ТК2 надійшло повідомлення, яке змінило параметри
 *  */
int f_tk5UpdateStat(
		HWND hwnd,
		HWND hProcesBar,
		UINT_PTR pTimer,
		int timer_ID
		){
	uint16_t x_right_panel 				= 839;
	char pm[80]={0};
	/* рядок динамічно для підготовки тексту у вікно*/
	HDC hDCr; // создаём дескриптор ориентации текста на экране
	RECT rect; // стр-ра, определяющая размер клиентской области
	/*малюємо заново*/
	hDCr= GetDC(hwnd);
	GetClientRect(hwnd, &rect);  	// получаем ширину и высоту области для рисования
	/* Фіксуємо довжину черги Модбас як число */
//	uint16_t iQ= f_get_tkrs232_length(&tk5Queue);

		/* Повідомлень */

		memset(&pm,0,sizeof(pm));
		/* Повідомлень */
		snprintf(pm,80,"%9d", (uint32_t)gTxRxSessionStartCntr);
		RECT rect_av = {rect.left+x_right_panel+100, rect.top+585, rect.left+x_right_panel+170+100, rect.top+660+20};
		DrawText(hDCr, pm, 9, &rect_av, DT_SINGLELINE|DT_LEFT|DT_TOP);
		memset(&pm,0,sizeof(pm));
		/* Успіх */
		uint32_t d2=(uint32_t)((gTxRxSessionSuccessDegreef+0.001)*100.0f)/100;
		uint32_t f2=(uint32_t)((gTxRxSessionSuccessDegreef+0.001)*100.0f)%100;
		snprintf(pm,80,"  %4d.%02d", d2,f2);
		RECT rect_a = {rect.left+x_right_panel+100, rect.top+610, rect.left+x_right_panel+170+100, rect.top+660+20};
		DrawText(hDCr, pm, 9, &rect_a, DT_SINGLELINE|DT_LEFT|DT_TOP);
		memset(&pm,0,sizeof(pm));
		/* Зайнятість */
		uint32_t d1=(uint32_t)(gTxRxBusyDegreef*100.0f)/100;
		uint32_t f1=(uint32_t)(gTxRxBusyDegreef*100.0f)%100;
		snprintf(pm,80,"    %02d.%02d", d1,f1);
		RECT rect_b = {rect.left+x_right_panel+100, rect.top+635, rect.left+x_right_panel+170+100, rect.top+660+20};
		DrawText(hDCr, pm, 9, &rect_b, DT_SINGLELINE|DT_LEFT|DT_TOP);
		memset(&pm,0,sizeof(pm));
		ReleaseDC(hwnd, hDCr);
	return 0;
}


/* Друк/розмальовка за результатами паралельного процесу,
 * викликається з вікна PPB паралельного процесу */
static void Init_YMode(uint16_t Phasnost, uint16_t LoadMode){
	/* Трифазний двигун */
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
	/* Однофазний двигун */
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
	/* Трифазне енергопостачання */
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
	/* Однофазне енергопостачання */
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
	HDC hDC; // создаём дескриптор ориентации текста на экране
	RECT rect; // стр-ра, определяющая размер клиентской области
	/*малюємо заново*/
	hDC= GetDC(hwnd);
	GetClientRect(hwnd, &rect);  	// получаем ширину и высоту области для рисования
	  	RECT rectwhite = {rect.left+190+1,rect.top+70,rect.left+550,rect.top+700-1};
	  	FillRect(hDC, &rectwhite, (HBRUSH)(COLOR_WINDOW+1));
		//Замальовуємо область справа центру синім
		RECT rectblue = {rect.left+550,rect.top+70,rect.left+819,rect.top+700-1};
		FillRect(hDC, &rectblue, (HBRUSH)29);
	/* Обновляємо координату рядків */

		/* Виводимо нові назви параметрів */
	if(y_HeatPro!=0){
		RECT rect0 = {rect.left+210, rect.top+y_HeatPro,rect.left+1000,rect.top+y_HeatPro+25};
		             //1234567890123456789012345678901234
		DrawText(hDC, "Ступінь нагріву електродвигуна, %", 34, &rect0, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_aAmperage!=0){
		RECT rectfa = {rect.left+210, rect.top+y_aAmperage,rect.left+1000,rect.top+y_aAmperage+25};
		DrawText(hDC, "Струм фази А, А                ", 32, &rectfa, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_bAmperage!=0){
		RECT rectfb = {rect.left+210, rect.top+y_bAmperage,rect.left+1000,rect.top+y_bAmperage+25};
		DrawText(hDC, "Струм фази В, А                ", 32, &rectfb, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_cAmperage!=0){
		RECT rectfc = {rect.left+210, rect.top+y_cAmperage,rect.left+1000,rect.top+y_cAmperage+25};
		DrawText(hDC, "Струм фази С, А                ", 32, &rectfc, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_power!=0){
		RECT rectpw = {rect.left+210, rect.top+y_power,rect.left+1000,rect.top+y_power+25};
		DrawText(hDC, "Потужність, кВт                ", 32, &rectpw, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_cosinus_factor!=0){
		RECT rectcf = {rect.left+210, rect.top+y_cosinus_factor,rect.left+1000,rect.top+y_cosinus_factor+25};
		DrawText(hDC, "Коефіцієнт потужності(косинус фi)", 34, &rectcf, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_unbalance_factor!=0){
		RECT rectub = {rect.left+210, rect.top+y_unbalance_factor,rect.left+1000,rect.top+y_unbalance_factor+25};
		DrawText(hDC, "Коефіцієнт потужності(асиметрія)", 34, &rectub, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_harm_factor!=0){

		RECT rectih = {rect.left+210, rect.top+y_harm_factor,rect.left+1000,rect.top+y_harm_factor+25};
		DrawText(hDC, "Коефіцієнт потужності(гармоніки) ", 34, &rectih, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_motohours!=0){
		RECT rectmh = {rect.left+210, rect.top+y_motohours,rect.left+1000,rect.top+y_motohours+25};
		DrawText(hDC, "Мотогодини                       ", 34, &rectmh, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_voltage!=0){
		RECT rectu = {rect.left+210, rect.top+y_voltage,rect.left+1000,rect.top+y_voltage+25};
		DrawText(hDC, "Напруга                          ", 34, &rectu, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_frequency!=0){
		RECT rectff = {rect.left+210, rect.top+y_frequency,rect.left+1000,rect.top+y_frequency+25};
		DrawText(hDC, "Частота                          ", 34, &rectff, DT_SINGLELINE|DT_LEFT|DT_TOP);

	}

	if(y_leakAmperage_A!=0){
		//			leakAmperage_A
					RECT rectla = {rect.left+210, rect.top+y_leakAmperage_A,rect.left+1000,rect.top+y_leakAmperage_A+25};
					DrawText(hDC, "Струм, витік на землю, А", 25, &rectla, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_GrundAmperageDistortion!=0){

		//			GrundAmperageDistortion
					RECT rectad = {rect.left+210, rect.top+y_GrundAmperageDistortion,rect.left+1000,rect.top+y_GrundAmperageDistortion+25};
					DrawText(hDC, "Витік струму, %", 16, &rectad, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_LoadType!=0){

					/* >LoadType - Тип навантаження - індуктивне/активне/ємнісне   */
					RECT recttl = {rect.left+210, rect.top+y_LoadType,rect.left+1000,rect.top+y_LoadType+25};
					DrawText(hDC, "Тип навантаження", 17, &recttl, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_PhaseRotation!=0){

					/*(uint16_t)(g->PhaseRotation)*/
					RECT rectak = {rect.left+210, rect.top+y_PhaseRotation,rect.left+1000,rect.top+y_PhaseRotation+25};
					DrawText(hDC, "Послідовність фаз", 18, &rectak, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_cosinus_factor!=0){
					RECT rectcf1 = {rect.left+210, rect.top+y_cosinus_factor,rect.left+1000,rect.top+y_cosinus_factor+25};
					DrawText(hDC, "Коефіцієнт потужності(косинус фі), %", 37, &rectcf1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_unbalance_factor!=0){
					RECT rectub1 = {rect.left+210, rect.top+y_unbalance_factor,rect.left+839+140,rect.top+y_unbalance_factor+25};
					DrawText(hDC, "Коефіцієнт потужності(асиметрія), %", 36, &rectub1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_harm_factor!=0){
					RECT rectfh = {rect.left+210, rect.top+y_harm_factor,rect.left+839+140,rect.top+y_harm_factor+25};
					DrawText(hDC, "Коефіцієнт потужності(гармоніки), %", 36, &rectfh, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_ActivePower_kW!=0){
					RECT rectap = {rect.left+210, rect.top+y_ActivePower_kW,rect.left+839+140,rect.top+y_ActivePower_kW+25};
					DrawText(hDC, "Потужність активна, кВт", 24, &rectap, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_ReactivePower_kW!=0){
					/* Реактивна потужність*/
					RECT rectrap = {rect.left+210, rect.top+y_ReactivePower_kW,rect.left+839+140,rect.top+y_ReactivePower_kW+25};
					DrawText(hDC, "Потужність реактивна, кВт", 26, &rectrap, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_UnbalanceLostPower_kW!=0){
					/* Потужність втрачена електродвигуном внаслідок асиметрії струму (лише для електродвигуна)*/
					RECT rectrap2 = {rect.left+210, rect.top+y_UnbalanceLostPower_kW,rect.left+839+140,rect.top+y_UnbalanceLostPower_kW+25};
					DrawText(hDC, "Потужність, втрати через асиметрію, кВт", 40, &rectrap2, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_HarmonicLostPower_kW!=0){
					/* Потужність, втрачена електродвигуном внаслідок несинусоїдальності струму */
					RECT rectrap3 = {rect.left+210, rect.top+y_HarmonicLostPower_kW,rect.left+839+140,rect.top+y_HarmonicLostPower_kW+25};

					DrawText(hDC, "Потужність, втрати через гармоніки, кВт", 40, &rectrap3, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_power!=0){
					/*kvatt_t	ActivePower_kW*/
					RECT rectap1 = {rect.left+210, rect.top+y_power,rect.left+839+140,rect.top+y_power+25};
					DrawText(hDC, "Потужність активна, кВт", 24, &rectap1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_aTotalHarmonicDistortion!=0){

					/*Зглажені параметри ефективності мережі 					                    */
					/*Коефіцієнт нелінійних спотворень по струму у кожній фазі                      */
					//aTotalHarmonicDistortion
					RECT rectah = {rect.left+210, rect.top+y_aTotalHarmonicDistortion,rect.left+839+140,rect.top+y_aTotalHarmonicDistortion+25};
					DrawText(hDC, "Коефіцієнт нелінійних спотворень, фаза А, %", 44, &rectah, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_bTotalHarmonicDistortion!=0){
					//bTotalHarmonicDistortion
					RECT rectbh = {rect.left+210, rect.top+y_bTotalHarmonicDistortion,rect.left+839+140,rect.top+y_bTotalHarmonicDistortion+25};
					DrawText(hDC, "Коефіцієнт нелінійних спотворень, фаза В, %", 44, &rectbh, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_cTotalHarmonicDistortion!=0){
					//cTotalHarmonicDistortion
					RECT rectch = {rect.left+210, rect.top+y_cTotalHarmonicDistortion,rect.left+839+140,rect.top+y_cTotalHarmonicDistortion+25};
					DrawText(hDC, "Коефіцієнт нелінійних спотворень, фаза С, %", 44, &rectch, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_THDi_HarmonicAmperageDistortion!=0){
					//Коефіцієнт нелінійних спотворень по струму узагальнений  (макс)
		//			/*procentf_t THDi_HarmonicAmperageDistortion*/
					RECT rectth = {rect.left+210, rect.top+y_THDi_HarmonicAmperageDistortion,rect.left+839+140,rect.top+y_THDi_HarmonicAmperageDistortion+25};
					DrawText(hDC, "Коефіцієнт нелінійних спотворень, %", 36, &rectth, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_aNegativeAmperage_A!=0){
					RECT rectan = {rect.left+210, rect.top+y_aNegativeAmperage_A,rect.left+839+140,rect.top+y_aNegativeAmperage_A+25};
					DrawText(hDC, "Струми негативних гармонік, фаза А, А", 38, &rectan, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_bNegativeAmperage_A!=0){
		//			bNegativeAmperage_A*/
					RECT rectbn = {rect.left+210, rect.top+y_bNegativeAmperage_A,rect.left+839+140,rect.top+y_bNegativeAmperage_A+25};
					DrawText(hDC, "Струми негативних гармонік, фаза В, А", 38, &rectbn, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_cNegativeAmperage_A!=0){
		//			amperf_t cNegativeAmperage_A*/
					RECT rectcn = {rect.left+210, rect.top+y_cNegativeAmperage_A,rect.left+839+140,rect.top+y_cNegativeAmperage_A+25};
					DrawText(hDC, "Струми негативних гармонік, фаза С, А", 38, &rectcn, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_aNeutralAmperage_A!=0){
					RECT rectnaa = {rect.left+210, rect.top+y_aNeutralAmperage_A,rect.left+839+140,rect.top+ y_aNeutralAmperage_A+25};
					DrawText(hDC, "Струми нейтральних гармонік, фаза А, А", 38, &rectnaa, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_bNeutralAmperage_A!=0){
		//			bNeutralAmperage_A
					RECT rectnab = {rect.left+210, rect.top+ y_bNeutralAmperage_A,rect.left+839+140,rect.top+ y_bNeutralAmperage_A+25};
					DrawText(hDC, "Струми нейтральних гармонік, фаза В, А", 38, &rectnab, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_cNeutralAmperage_A!=0){
		//			cNeutralAmperage_A
					RECT rectnac = {rect.left+210, rect.top+ y_cNeutralAmperage_A,rect.left+839+140,rect.top+y_cNeutralAmperage_A+25};
					DrawText(hDC, "Струми нейтральних гармонік, фаза С, А", 38, &rectnac, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_sumNeutralAmperage_A!=0){
		//			sumNeutralAmperage_A
					RECT rectnat = {rect.left+210, rect.top+ y_sumNeutralAmperage_A,rect.left+839+140,rect.top+ y_sumNeutralAmperage_A+25};
					DrawText(hDC, "Струми нейтральних гармонік разом, А", 37, &rectnat, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_AmperageUnbalance!=0){						            // 1234567890123456789012345678901234567890123456789
					RECT rectnkf = {rect.left+210, rect.top+ y_AmperageUnbalance,rect.left+839+140,rect.top+ y_AmperageUnbalance+25};
					DrawText(hDC, "Перекос фаз, %", 15, &rectnkf, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_leakFurie!=0){
					RECT rectnlf = {rect.left+210, rect.top+ y_leakFurie,rect.left+839+140,rect.top+ y_leakFurie+25};
					            // 1234567890123456789012345678901234567890123456789
					DrawText(hDC, "Витік струму на землю(Ф), А", 28, &rectnlf, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	if(y_leakFuriePhase!=0){
					RECT rectnlfp = {rect.left+210, rect.top+ y_leakFuriePhase,rect.left+839+140,rect.top+ y_leakFuriePhase+25};
					DrawText(hDC, "Фаза витоку(Ф)", 15, &rectnlfp, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}
	ReleaseDC(hwnd, hDC);
	return 0;
}
