
#include "a3_tk2z.h"
#include "../../mb_devs/tk4_dist.h"
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
//#include "../a2_Tx_msg.h"
//#include "../a3_monitoring.h"
#include "../../auxs/hexdec.h"
#include "../e1_passwordEdit.h"
#include "a21_tk2us.h"
#include "../a3_tkTxRx.h"
#include "a22_tk2fs.h"
extern HINSTANCE hinst_tk2;
/* Структура, яка визначає чергу
 * Оголошення цієї структури означає, що створена
 * конкретна черга з конкретним іменем, поки що пуста */
extern   queue_t tk2Queue;
extern i412limit_t i412limit[9];

const char g_szClassNameTK112us[] = "TK112US_WndClass";

extern d_type_t q;
extern char tk2LogStr[512];

// 0x0205	Уставка защиты от токовой перегрузки, А	10.0 –  600.0	Iн	arr[2]
float2_t Itk2_Max={0};
//0x0206	Уставка минимально допустимого тока (недогрузки), А 	10.0–600.0 Iсх	arr[3]
float2_t Itk2_Min={0};


uint32_t tk2USTimerCntr=0;

//static user_settings_t	u = {0};
//static volatile uint32_t n_strValue=0;
//static volatile uint32_t n_strValueOld=0;
//static volatile uint32_t n_strValue1=0;

static uint16_t pumping_mode_tmp = 999;
static FunctionalState selfstart_tmp=DISABLE;
/* Намагання корегувати параметри, які ще не заватнажились з ТК2
 * в РС призводить до до некорктної роботи.
 * Цей прапор виставляється в ProcessBusy
 * після того, як усі параметри завантажилися в РС*/
ProcessState_t tk2usProcess=ProcessIddle;
/* Часте безладне натискання на клавіші забиває чергу повідомлень.
 * Доступ до натискання клавіші на 400мсек заблоковано
 * після попереднього натискання*/
static FunctionalState tk2usBtn_access = ENABLE;
static uint32_t tk2usBtnSleepCntr = 0;
#define  TK2US_BTN_BLOKING_MS         400



/* Статичні змінні для відображення динамічної зміни рядків вікна */
//static HWND PresSWnd =  NULL;
//static HWND PresUnitWnd = NULL;
//static HWND hEditadd_high_limit;
//static HWND hEdit_add_low_limit = NULL;
//static HWND hEdit_OPadd_high_limit = NULL;
//static HWND hEdit_OPadd_low_limit = NULL;

static HWND OpModeWnd;

static HWND SelfWnd;
static HWND hEditAmperageMax;
static HWND hEditAmperageMin;
static HWND hEditMBAddr;
static HWND hEditPassword;
static HWND hWndProgressBar_usQueue;
static HWND hEditCheckPassword;
static  UINT_PTR  pUS1000Timer;
static  UINT_PTR  p10usTimer;

HWND hWndTk2usCh=NULL;
static void f_tk2usUpdateInit(void);
static BOOL CALLBACK ChildCallbacktk2us(
		HWND   hwnd,
		LPARAM lParam
);
/* Регістрація класу вікна, викликається при запуску програми */
LPCSTR f_RegisterTK112USWndClass(HINSTANCE hinst){
	WNDCLASS w;
			memset(&w,0,sizeof(WNDCLASS));
			w.lpfnWndProc = WndProcTK112us;
			w.hInstance = hinst;
			w.style = CS_HREDRAW | CS_VREDRAW;
			w.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
			w.lpszClassName = g_szClassNameTK112us;
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
			;
		if(!RegisterClassA(&w))
		{
	//		int Error = GetLastError();
			MessageBox(NULL, "Window Registration Failed!", "Error!",
				MB_ICONEXCLAMATION | MB_OK);
			return 0;
		}
	return w.lpszClassName;
}
/* Створення вікна, викликається при вході в меню Користуваці налаштування основного вікна */
HWND  f_CreateTK112usWnd(HWND hh1){

//	WNDCLASSEX wc;
	MSG Msg;
	HWND usHwnd = CreateWindowEx(
		0,
		g_szClassNameTK112us,
		"TK2. Налаштування користувача",
		(WS_OVERLAPPEDWINDOW&(~(WS_MAXIMIZEBOX|WS_MINIMIZEBOX)))|WS_BORDER|WS_VISIBLE
		,
		CW_USEDEFAULT, CW_USEDEFAULT, 1024, 800,
		NULL, NULL, hinst_tk2, NULL);

	HMENU hMenubar = CreateMenu();
	HMENU hMenu = CreateMenu();

//	AppendMenu(hMenu, MF_STRING, IDM_USER_SETTINGS, "&Налаштування користувача");
	AppendMenu(hMenu, MF_STRING, IDM_FACILITY_SETTING_1, "&Налаштування виробника");
//	AppendMenuW(hMenu, MF_STRING, IDM_FACILITY_SETTING_2, "&Калібрування");
//	AppendMenu(hMenu, MF_STRING, IDM_COMPORT_SETTING_US, "&Налаштування СОМ-порту");
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenu, MF_STRING, IDM_QUIT_US, "&Вихід");

	AppendMenu(hMenubar, MF_POPUP, (UINT_PTR) hMenu, "&Налаштування додатково");

	SetMenu(usHwnd, hMenubar);
	ShowWindow(usHwnd, SW_NORMAL);
	UpdateWindow(usHwnd);

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	tk2usProcess=ProcessIddle;
	return usHwnd;
}


LRESULT CALLBACK WndProcTK112us(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){

/* у-координати друку параметрів і комбобоксів*/
uint16_t y_max_amperage                =100;							//0x0401
uint16_t y_min_amperage			  	   =125;

uint16_t y_check_password              =175;
uint16_t y_pumping_mode                =200;
uint16_t y_drive_self_starting 		   =225;
uint16_t y_mb_addr 					   =250;
uint16_t y_us_passport                 =275;
uint16_t y_us_exit                     =325;

HFONT hfDefault;
	switch(msg){
		/**************************************************************************************************************
		 *      CREATE 																						     CREATE
		 **************************************************************************************************************/
		case WM_CREATE:{
			  /* Доска для верхнього надпису */
			  CreateWindowW(L"Button", L"",
	                WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
					190, 10,  629, 40, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
			  /* Центральна доска для параметрів */
	          CreateWindowW(L"Button", L"",
	                WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
					190, 60,  629, 640, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
	          //http://zetcode.com/gui/winapi/controlsIII/

          /* Кнопки читати/писати уставки через Модбас */
		 	 CreateWindowW(L"Button", L"",
		                  WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
		             20, 60,  150, 330, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);




	          /* Кнопки читати/писати по одній уставці у через Модбас */
//			 	 CreateWindowW(L"Button", L"",
//			                  WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
//			             20, 60,  150, 330, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
		          CreateWindow("BUTTON", "Записати", WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
		                 839, y_max_amperage, 130, 20, hwnd,
						 (HMENU)ID_WR_US_MAX_AMPERAGE,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
		          CreateWindow("BUTTON", "Записати", WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
		                 839, y_min_amperage, 130, 20, hwnd,
						 (HMENU)ID_WR_US_MIN_AMPERAGE,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);

		          CreateWindow("BUTTON", "Ввести", WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
		        		  839, y_check_password, 130, 20, hwnd,
						 (HMENU)IDB_BTN_CHECK_PSW,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
		          CreateWindow("BUTTON", "Записати", WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
		        		  839, y_pumping_mode, 130, 20, hwnd,
						 (HMENU)ID_WR_US_PUMPING_MODE,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
		          CreateWindow("BUTTON", "Записати", WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
		                 839, y_drive_self_starting, 130, 20, hwnd,
						 (HMENU)ID_WR_US_SELF_START,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
		          CreateWindow("BUTTON", "Записати", WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
		        		  839, y_mb_addr, 130, 20, hwnd,
						 (HMENU)ID_WR_US_MODBUS_ADDR,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
		          CreateWindow("BUTTON", "Записати", WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
		        		  839, y_us_passport, 130, 20, hwnd,
						 (HMENU)ID_WR_US_PASSWORD,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
		          CreateWindow("BUTTON", "Вийти", WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
		        		  839, y_us_exit, 130, 20, hwnd,
						 (HMENU)IDB_BTN_tk2US_EXIT,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);


			 	 CreateWindow("BUTTON", "Читати з ТК2",
		                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
		                 40, 90,  110, 40, hwnd, (HMENU)IDB_BTN_READ,
												   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
          /* Вид роботи */
          if ((q.d_regime==remote_automatic_mode)||(q.d_regime==automatic_mode)){
        	  HWND OpModeWnd = CreateWindow("COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
        		  460, y_pumping_mode, 300, 800,
				  hwnd, (HMENU)EB_tk2US_PUMPREGIMEMODE,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),NULL);

        	  SendMessage(OpModeWnd, CB_ADDSTRING, 0, (LPARAM)" Відкачка, датчики рівня      (ОЭ)");
        	  SendMessage(OpModeWnd, CB_ADDSTRING, 0, (LPARAM)" Подача, ЕКМ                  (ПЕ)");
        	  SendMessage(OpModeWnd, CB_ADDSTRING, 0, (LPARAM)" Подача, датчики рівня        (ПЭ)");
        	  SendMessage(OpModeWnd, CB_ADDSTRING, 0, (LPARAM)" Подача, реле тиску/попл.датч.(ПП)");
        	  SendMessage(OpModeWnd, CB_ADDSTRING, 0, (LPARAM)" Відкачка, поплавковий датчик (ОП)");
        	  SendMessage(OpModeWnd, CB_ADDSTRING, 0, (LPARAM)" Подача, датчик тиску         (ПО)");
        	  SendMessage(OpModeWnd, CB_SETCURSEL, (uint16_t)q.tk2_PumpingMode, 0);

          }
    	  /* Самозапуск */
    	  HWND SelfWnd;
    	  SelfWnd = CreateWindow("COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
    		  460, y_drive_self_starting, 300, 800,
			  hwnd, (HMENU)EB_tk2US_SELFSTART,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),NULL);
    	  SendMessage(SelfWnd, CB_ADDSTRING, 0,       (LPARAM)" Не дозволено             ");
    	  SendMessage(SelfWnd, CB_ADDSTRING, 0,       (LPARAM)" Дозволено                ");
    	  SendMessage(SelfWnd, CB_SETCURSEL, q.d_FS, 0);
//		  hEditPower = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", &strValue,
//				WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
//				, 460, y_rated_motor_power, 300, 25,
//				hwnd, (HMENU)IDC_POWER_EDIT, GetModuleHandle(NULL), NULL);
//		  SendMessage(hEditPower, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
    	  char strValue[256]={0};
		  hEditAmperageMax = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", strValue,
					WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
					, 460, y_max_amperage, 300, 25,
					hwnd, (HMENU)IDC_AMPERAGE_SETEDIT, GetModuleHandle(NULL), NULL);
				SendMessage(hEditAmperageMax, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

		  hEditAmperageMin = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", strValue,
					WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
					, 460, y_min_amperage, 300, 25,
					hwnd, (HMENU)CURRENT_TRIPPING_LIM, GetModuleHandle(NULL), NULL);
				SendMessage(hEditAmperageMin, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
//				char as[40]={0};
//				snprintf(as,23,"%3d.%02d",2999/10,2999%10);
//				SetWindowText(hEditAmperageMax,(LPCTSTR)as);
				hEditCheckPassword = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", strValue,
									WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
								, 460, y_us_passport, 300, 25,
								   hwnd, (HMENU)EB_tk2US_CHECK_PASSWORD, GetModuleHandle(NULL), NULL);
								SendMessage(hEditPassword, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
								SetWindowText(hEditMBAddr,"");

		  hEditMBAddr = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", strValue,
					WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
				, 460, y_mb_addr, 300, 25,
				   hwnd, (HMENU)ID_EDIT_MODBUSS_ADDR, GetModuleHandle(NULL), NULL);


				SendMessage(hEditMBAddr, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
				char addr_s[20]={0};
				snprintf (addr_s,20,"%X",tk2_Addr);
				SetWindowText(hEditMBAddr,(LPCTSTR)addr_s);

		  hEditPassword = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", strValue,
							WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
						, 460, y_us_passport, 300, 25,
						   hwnd, (HMENU)ID_EDIT_NEW_PASSWORD, GetModuleHandle(NULL), NULL);
						SendMessage(hEditPassword, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

		  /* Запускаємо таймер, щоб зчитати типорозмір */
		  pUS1000Timer=SetTimer(
				  hwnd,                // handle to main window
				  IDT_TIMER_USTK2_1000MS,// timer identifier
				  1000,                // msecond interval
				  (TIMERPROC) NULL     // no timer callback
				);

		/* Безумовний прогрес-бар для черги повідомлень */
		  hWndProgressBar_usQueue = CreateWindowEx(
				0,
				PROGRESS_CLASS,
				(LPSTR)NULL,
				WS_VISIBLE | WS_CHILD /*| PBS_VERTICAL*/,
				839, 680, 150, 20,
				//910, 620, 150, 640
				hwnd,
				(HMENU)IDPB_usQUEUE_MSG ,
				(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
				NULL
				);
		  if (!hWndProgressBar_usQueue){
				MessageBox(NULL, "Progress Bar Faileq.", "Error", MB_OK | MB_ICONERROR);
		  }
		  /* Шкала 120% - задаємо в процентах */
		  SendMessage(hWndProgressBar_usQueue, PBM_SETRANGE, 0, MAKELPARAM(
				 0, //minimum range value
				 10000  //maximum range value
				 )
		  );
		  SendMessage(hWndProgressBar_usQueue, PBM_SETPOS,
				  0, //Signed integer that becomes the new position.
				  0  //Must be zero
		  );
		  /* таймер для роботи з процес-баром черги повідомлень */
		  p10usTimer=SetTimer(
				  hwnd,                // handle to main window
				  IDT_usTIMER_TK2_10MS,  // timer identifier
				  TIMER_MAIN_MS,                  // msecond interval
				  (TIMERPROC) NULL     // no timer callback
				);
		 /* Таймер для постійного періодичного опитування
		 * статусу доступу до налаштувань*/
		 pUS1000Timer=SetTimer(
				  hwnd,                // handle to main window
				  IDT_TIMER_USTK2_1000MS,// timer identifier
				  1000,                // msecond interval
				  (TIMERPROC) NULL     // no timer callback
				);

				/* Збираємо повідомлення для запису регістра */
				modbus_master_tx_msg_t mbTxMsg={0};
				f_Set112TxReadReg(RG_R_TK2_TYPOROZMIR,&mbTxMsg);
				/* Ставимо повідомлення в чергу на обробку   */
				f_set_tkqueue(
						&tk2Queue,
						DISABLE,
						hwnd,
						RG_R_TK2_TYPOROZMIR,
						&mbTxMsg,
						600
				);
				/* Скидаємо всі параметри для обновлення параметрів в 0*/
				f_tk2usUpdateInit();
		        /* Формуємо запит в ТК2 на зчитування налаштувань користувача */
			    f_read_user_settings(hwnd);
			    tk2usProcess=ProcessIddle;

		}
		break;
		/**************************************************************************************************************
	    *      PAINT 																						      PAINT
		**************************************************************************************************************/
		case WM_PAINT: {// если нужно нарисовать, то:

	    	  HDC hDC; // создаём дескриптор ориентации текста на экране
    	      RECT rect; // стр-ра, определяющая размер клиентской области
	    	  PAINTSTRUCT ps; // структура, сод-щая информацию о клиентской области (размеры, цвет и тп)

	    	  hDC = BeginPaint(hwnd, &ps); 	// инициализируем контекст устройства
	    	  GetClientRect(hwnd, &rect);  	// получаем ширину и высоту области для рисования
	    	  RECT rectL =    {rect.left+210,rect.top+25, rect.left+1000,rect.top+50};
	    	               //123456789012345678901234567890
	    	  DrawText(hDC, "Налаштування користувача", 25, &rectL, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  //Замальовуємо область справа центру синім
	    	  //Замальовуємо область справа центру синім
	    	  RECT rectblue = {rect.left+450,rect.top+70,rect.left+819,rect.top+700};
	    	  FillRect(hDC, &rectblue, (HBRUSH)29);
	    	  /* Синій фон для параметрів*/

              /* Перший параметер */
	    	  RECT rectcp = {rect.left+210, rect.top+y_check_password,rect.left+1000,rect.top+25+y_check_password};
	    	  DrawText(hDC, "Введіть пароль ", 16, &rectcp, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	  RECT rect0 = {rect.left+210, rect.top+y_max_amperage,rect.left+1000,rect.top+y_max_amperage+25};
	    	  DrawText(hDC, "Максимальний струм, А ", 21, &rect0, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	  RECT rect1 = {rect.left+210, rect.top+y_min_amperage,rect.left+1000,rect.top+25+y_min_amperage};
	    	  DrawText(hDC, "Мінімальний струм, A  ", 21, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	  RECT rect2 = {rect.left+210, rect.top+y_pumping_mode,rect.left+1000,rect.top+25+y_pumping_mode};
	    	  DrawText(hDC, "Вид роботи ", 12, &rect2, DT_SINGLELINE|DT_LEFT|DT_TOP);

		   	  RECT rectcp1 = {rect.left+210, rect.top+y_check_password,rect.left+1000,rect.top+25+y_check_password};
			  DrawText(hDC, "Введіть пароль ", 16, &rectcp1, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	  RECT rect5 = {rect.left+210, rect.top+y_drive_self_starting,rect.left+1000,rect.top+25+y_drive_self_starting};
	    	  DrawText(hDC, "Самозапуск ", 12, &rect5, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	  RECT rect6 = {rect.left+210, rect.top+y_mb_addr,rect.left+1000,rect.top+25+y_mb_addr};
	    	  DrawText(hDC, "Адреса Модбас ", 15, &rect6, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	  RECT rect7 = {rect.left+210, rect.top+y_us_passport,rect.left+1000,rect.top+25+y_us_passport};
	    	               //123456789012345678901234567890
	    	  DrawText(hDC, "Замінити пароль на:  ",22, &rect7, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	  RECT rectas = {rect.left+460, rect.top+y_check_password,rect.left+819,rect.top+y_check_password+25};
  	    	  if(us_access_status==DISABLE){
  	    		               //12345678901234567890123456789012345678901234567
  	    		  DrawText(hDC, "Корегувати заборонено",22, &rectas, DT_SINGLELINE|DT_LEFT|DT_TOP);
  	    	  }
  	    	  else if (us_access_status==ENABLE){
  	    		  DrawText(hDC, "Корегувати дозволено ",22, &rectas, DT_SINGLELINE|DT_LEFT|DT_TOP);
  	    	  }
	    	  /* Розмальовування ПроцесБарів справа знизу - Нагрівання електродвигуна */
	    	  //910, 680, 150, 20,
  	    	  int x_right_panel = 839;
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
	    	  EndPaint(hwnd, &ps);
		}//end WM_PAINT
		break;
		/**************************************************************************************************************
	    *      SIZE 																						      SIZE
		**************************************************************************************************************/
		case WM_SIZE:{
		}
		break;
		/**************************************************************************************************************
	    *      COMMAND 																						    COMMAND
		**************************************************************************************************************/
		case WM_COMMAND:{
			char wmc[256]={0};
//			   дескриптор дочер. окна If(idCtrl==ID_combo&&code==CBN_SELCHANGE)
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
						case  EB_tk2US_PUMPREGIMEMODE:
							pumping_mode_tmp =ItemIndex;
						break;
						case  EB_tk2US_SELFSTART:
							selfstart_tmp=ItemIndex;
						break;
						default:{}
					} // switch(idCtl)
				}//case CBN_SELCHANGE:
				break;
				default:{}
			}


		/*WM_COMMAND:*/
		switch(wParam) {
			case IDM_QUIT_US:
			case IDB_BTN_tk2US_EXIT:{
				/* Перед завершенням роботи з налаштуванням скидаємо пароль */
				/* Збираємо повідомлення для запису регістра */
				modbus_master_tx_msg_t mbTxMsg={0};
				f_Set112TxWriteReg(RG_W_TK2_PSW_CHECKING,0,&mbTxMsg);
				/* Ставимо повідомлення про стинання ознаки ввода пароля
				 * в чергу на обробку   */
				f_set_tkqueue(
					&tk2Queue,
					ENABLE,
					hwnd,
					RG_W_TK2_PSW_CHECKING,
					&mbTxMsg,
					1000
				);
				 snprintf(tk2LogStr,511,"ко: Завершено налаштування користувача");
				 if(IsWindow(hWndTk2fsCh)){DestroyWindow(hWndTk2fsCh);}
				 DestroyWindow(hwnd);
			}
			break;

		    /* Зчитуємо налаштування користувача всі */
			case IDB_BTN_READ:{
				if(tk2usProcess==ProcessBusy){
				  if(tk2usBtn_access==ENABLE){
					 tk2usBtn_access=DISABLE;
					 f_read_user_settings(hwnd);
					 snprintf(tk2LogStr,511,"КО: Зчитування налаштувань користувача");
				  }else{snprintf (wmc,255," Не натискайте так часто на кнопки! ");}
				}else{snprintf (wmc,255," Налаштування користувача ще не завантажилися.\nПочекайте ");}
			}
			break;
			/* Записуємо максимальний струм */
			case ID_WR_US_MAX_AMPERAGE:{
				if(tk2usProcess==ProcessBusy){
				  if(tk2usBtn_access==ENABLE){
					 tk2usBtn_access=DISABLE;
					 /* Зчитуємо значення струму, це , це завжди HEX*/
					 float2_t wItk2_Max={0};
					 int errWM_C=f_checkEdit(IDC_AMPERAGE_SETEDIT, hwnd, &wItk2_Max);
					 if(errWM_C==0){
						 uint16_t value = wItk2_Max.d*10 + wItk2_Max.f / 1000;

						 if ((typorozmir<9)&&(value >= i412limit[typorozmir].imax)){
							const int result = MessageBox(NULL,"Cтрум завеликий","Помилка",MB_OK);
								switch (result){case IDOK:{}break;default:{}}
								return 0;
						 }
						 else if ((typorozmir<9)&&(value <=i412limit[typorozmir].imin)){
							const int result = MessageBox(NULL,"Cтрум замалий","Помилка",MB_OK);
								switch (result){case IDOK:{}break;default:{}}
								return 0;
						 }
						 else{
							 /* Збираємо повідомлення для запису регістра */
							 modbus_master_tx_msg_t mbTxMsg={0};
							 f_Set112TxWriteReg(RG_W_TK2_MAX_AMPERAGE,value,&mbTxMsg);
							 /* Ставимо повідомлення в чергу на обробку   */

							 f_set_tkqueue(
								&tk2Queue,
								ENABLE,
								hwnd,
								ID_WR_US_MAX_AMPERAGE,
								&mbTxMsg,
								1000
								);
						 }
						 f_read_user_settings(hwnd);
						 snprintf(tk2LogStr,511,"ко: Введено уставку Імакс = %d.%d  A", wItk2_Max.d,wItk2_Max.f);
					 }
					 /* Якщо там не число */
					 else{
					 f_valueEditMessageBox(errWM_C);
					 }
				  }else{snprintf (wmc,255," Не натискайте так часто на кнопки! ");}
				}else{snprintf (wmc,255," Налаштування користувача ще не завантажилися.\nПочекайте ");}
			}
			break;
			case ID_WR_US_MIN_AMPERAGE:{
				if(tk2usProcess==ProcessBusy){
				  if(tk2usBtn_access==ENABLE){
					 tk2usBtn_access=DISABLE;
					 float2_t wItk2_Min={0};
					 /* Зчитуємо значення струму */
					 int errWM_C=f_checkEdit(CURRENT_TRIPPING_LIM, hwnd, &wItk2_Min);
					 if(errWM_C==0){
						 uint16_t value = wItk2_Min.d*10 + wItk2_Min.f / 1000;
						 if (wItk2_Min.floatf > wItk2_Min.floatf){
							 const int result = MessageBox(NULL,"Мінімальний струм повинен бути меншим за максимальний","Помилка",MB_OK);
							 switch (result){case IDOK:{}break;default:{}}
							 return 0;
						 }
					else{
						/* Збираємо повідомлення для запису регістра */
						modbus_master_tx_msg_t mbTxMsg={0};
						f_Set112TxWriteReg(RG_W_TK2_MIN_AMPERAGE,value,&mbTxMsg);
						/* Ставимо повідомлення в чергу на обробку   */

						f_set_tkqueue(
								&tk2Queue,
								ENABLE,
								hwnd,
								ID_WR_US_MIN_AMPERAGE,
								&mbTxMsg,
								1000
								);
					 	 }
						 f_read_user_settings(hwnd);
						 snprintf(tk2LogStr,511,"ко: Введено уставку Імin = %d.%d  A", wItk2_Min.d,wItk2_Min.f);
					 }
				     /* Якщо там не число */
					 else{
						 f_valueEditMessageBox(errWM_C);
					 }
				  }else{snprintf (wmc,255," Не натискайте так часто на кнопки! ");}
				}else{snprintf (wmc,255," Налаштування користувача ще не завантажилися.\nПочекайте ");}
			}
			break;

			case IDB_BTN_CHECK_PSW:{
				if(tk2usProcess==ProcessBusy){
				  if(tk2usBtn_access==ENABLE){
					 tk2usBtn_access=DISABLE;
					 CreatePswWindow(hwnd);
					 snprintf(tk2LogStr,511,"КО: Введення паролю");

				  }else{snprintf (wmc,255," Не натискайте так часто на кнопки! ");}
				}else{snprintf (wmc,255," Налаштування користувача ще не завантажилися.\nПочекайте ");}
				}
			break;
			case ID_WR_US_PUMPING_MODE:{
				if(tk2usProcess==ProcessBusy){
				  if(tk2usBtn_access==ENABLE){
					 tk2usBtn_access=DISABLE;

					 /* Збираємо повідомлення для запису регістра */
					 modbus_master_tx_msg_t mbTxMsg={0};
					 f_Set112TxWriteReg(RG_W_TK2_PUMPING_MODE,pumping_mode_tmp,&mbTxMsg);
					 /* Ставимо повідомлення в чергу на обробку   */

					 f_set_tkqueue(
							&tk2Queue,
							ENABLE,
							hwnd,
							RG_W_TK2_PUMPING_MODE,
							&mbTxMsg,
							1000
							);

					 f_read_user_settings(hwnd);

					 snprintf(tk2LogStr,511,"КО: Заміна виду роботи на.. %d", (int)pumping_mode_tmp);
				  }else{snprintf (wmc,255," Не натискайте так часто на кнопки! ");}
				}else{snprintf (wmc,255," Налаштування користувача ще не завантажилися.\nПочекайте");}
			}
			break;
			case ID_WR_US_SELF_START:{
				if(tk2usProcess==ProcessBusy){
				  if(tk2usBtn_access==ENABLE){
					 tk2usBtn_access=DISABLE;
					 /* Збираємо повідомлення для запису регістра */
					 modbus_master_tx_msg_t mbTxMsg={0};
					 f_Set112TxWriteReg(RG_W_TK2_SELFSTART,(uint16_t)selfstart_tmp,&mbTxMsg);
					 /* Ставимо повідомлення в чергу на обробку   */

					 f_set_tkqueue(
							&tk2Queue,
							ENABLE,
							hwnd,
							ID_WR_US_MIN_AMPERAGE,
							&mbTxMsg,
							1000
							);
					 f_read_user_settings(hwnd);
					 snprintf(tk2LogStr,511,"ко: Змінено уставку САМОЗАПУСК на %d",(uint16_t)selfstart_tmp);
				  }else{snprintf (wmc,255," Не натискайте так часто на кнопки! ");}
			   }else{snprintf (wmc,255," Налаштування користувача ще не завантажилися.\nПочекайте ");}
			}
			break;

			case ID_WR_US_MODBUS_ADDR:{
				if(tk2usProcess==ProcessBusy){
				  if(tk2usBtn_access==ENABLE){
					 tk2usBtn_access=DISABLE;
					 /* Зчитуємо адресу Модбас */
					 float2_t fl={0};
					 int errWM_C=f_checkEdit(ID_EDIT_MODBUSS_ADDR, hwnd, &fl);
					 if(errWM_C==0){
						 uint16_t newMB_Addr=fl.d;
						 if((newMB_Addr==2)||(newMB_Addr==3)){
							 /* Збираємо повідомлення для запису регістра */
							 modbus_master_tx_msg_t mbTxMsg={0};
							 f_Set112TxWriteReg(RG_W_TK2_NEW_MODBUS_ADDRESS,newMB_Addr,&mbTxMsg);
							 /* Ставимо повідомлення в чергу на обробку   */
							 f_set_tkqueue(
							    &tk2Queue,
								ENABLE,
								hwnd,
								RG_W_TK2_NEW_MODBUS_ADDRESS,
								&mbTxMsg,
								1000
								);
							 snprintf(tk2LogStr,511,"ко: Змінено АДРЕСу МОДБАС на %d",newMB_Addr);
						 }
						 else{
							 const int result = MessageBox(NULL,"Підтримуються лише адреси 0х02 і 0х03","Помилка",MB_OK);
							 switch (result){case IDOK:{}break;default:{}}
						 }
					 }
					 /* Якщо там не число */
					 else{
						 f_valueEditMessageBox(errWM_C);
					 }
					 f_read_user_settings(hwnd);
				  }else{snprintf (wmc,255," Не натискайте так часто на кнопки! ");}
			   }else{snprintf (wmc,255," Налаштування користувача ще не завантажилися.\nПочекайте ");}
			}
			break;

			case ID_WR_US_PASSWORD:{
				if(tk2usProcess==ProcessBusy){
				  if(tk2usBtn_access==ENABLE){
					 tk2usBtn_access=DISABLE;
					 //RG_W_TK2_NEW_PASSWORD     ID_EDIT_NEW_PASSWORD
					 /* Зчитуємо з ЕдітБокса нове значення пароля */
					 float2_t fl={0};
					 int errWM_C=f_checkEdit(ID_EDIT_NEW_PASSWORD, hwnd, &fl);
					 if(errWM_C==0){
						 uint16_t newPSW=fl.d;
						 /* Збираємо повідомлення для запису регістра */
						 modbus_master_tx_msg_t mbTxMsg={0};
						 f_Set112TxWriteReg(RG_W_TK2_NEW_PASSWORD,newPSW,&mbTxMsg);
						 /* Ставимо повідомлення в чергу на обробку   */
					     f_set_tkqueue(
							    &tk2Queue,
								ENABLE,
								hwnd,
								RG_W_TK2_NEW_PASSWORD,
								&mbTxMsg,
								1000
								);
						 f_read_user_settings(hwnd);
						 snprintf(tk2LogStr,511,"ко: Змінено ПАРОЛЬ на %d", (0x1234 | newPSW));

					 }
					 /* Якщо там не число */
					 else{
						 f_valueEditMessageBox(errWM_C);
					 }
				  }else{snprintf (wmc,255," Не натискайте так часто на кнопки! ");}
				}else{snprintf (wmc,255," Налаштування користувача ще не завантажилися.\nПочекайте ");}
			}
			break;
			case IDM_FACILITY_SETTING_1:{
				if(tk2usProcess==ProcessBusy){
				  if(tk2usBtn_access==ENABLE){
					 tk2usBtn_access=DISABLE;
					 if(!IsWindow(hWndTk2fsCh)){
//						snprintf(tk2LogStr,511,"ко: Завершено налаштування користувача");
//						 KillTimer(hwnd, pUS1000Timer);
						 /* Відкриваємо вікно заводських налаштувань     */
						 hWndTk2fsCh=f_CreateTk2fsWnd(hwnd);
					 }
					 snprintf(tk2LogStr,511,"КО: Вхід у налаштування виробника");
				  }else{snprintf (wmc,255," Не натискайте так часто на кнопки! ");}
				}else{snprintf (wmc,255," Налаштування користувача ще не завантажилися.\nПочекайте ");}
			}
			break;
			default:{}
			}
		}//WM_COMMAND
		break;
		/*************************************************************************************************************************************
		*      TIMERS 																						   TIMERS 					TIMERS
		**************************************************************************************************************************************/

		case WM_TIMER:{
			switch ((UINT)wParam){
			case IDT_usTIMER_TK2_10MS:{
				  /* Якщо у вікні натиснута кнопка - вмикаємо лічильник затримки */
				  if((tk2usProcess==ProcessBusy)&&(tk2usBtn_access==DISABLE)){
					  tk2usBtnSleepCntr+= TIMER_MAIN_MS;
					  if(tk2usBtnSleepCntr>=TK2US_BTN_BLOKING_MS){
						  tk2usBtn_access=ENABLE;
						  tk2usBtnSleepCntr=0;
					  }
				  }
			}
			break;
			 /* Таймер для постійного періодичного опитування
			 * статусу доступу до налаштувань*/
			case  IDT_TIMER_USTK2_1000MS:{
				/* Перевірка статусу прав доступу  */
				modbus_master_tx_msg_t mbTxMsg={0};
				f_Set112TxReadReg(RG_R_TK2_ACCESS_STATUS,&mbTxMsg);
				f_set_tkqueue(
						&tk2Queue,
						DISABLE,
						hwnd,
						RG_R_TK2_ACCESS_STATUS,
						&mbTxMsg,
						1000
						);
			}
			break;
			default:{}
			}
		}
		break;
		case WM_CLOSE:{
			/* Перед завершенням роботи з налаштуванням скидаємо пароль */
			/* Збираємо повідомлення для запису регістра */
			modbus_master_tx_msg_t mbTxMsg={0};
			f_Set112TxWriteReg(RG_W_TK2_PSW_CHECKING,0,&mbTxMsg);
			/* Ставимо повідомлення про стинання ознаки ввода пароля
			 * в чергу на обробку   */
			f_set_tkqueue(
				&tk2Queue,
				ENABLE,
				hwnd,
				RG_W_TK2_PSW_CHECKING,
				&mbTxMsg,
				1000
			);
			 snprintf(tk2LogStr,511,"ко: Завершено налаштування користувача");
			 if(IsWindow(hWndTk2fsCh)){DestroyWindow(hWndTk2fsCh);}
			 DestroyWindow(hwnd);
		}
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
		break;
		case VK_PSW:{
//			float2_t float2psw={0};
//			int err = f_strint_to_value(outPassword, strlen(outPassword), &float2psw);
//			if (err !=0){f_valueEditMessageBox(err);}
//			else{
//				memset(outPassword,0,sizeof(outPassword));
			uint16_t psw=(uint16_t)wParam;
			/* Збираємо повідомлення для запису регістра */
			modbus_master_tx_msg_t mbTxMsg={0};
			f_Set112TxWriteReg(RG_W_TK2_PSW_CHECKING,psw,&mbTxMsg);
				/* Ставимо повідомлення в чергу на обробку   */
			f_set_tkqueue(
				&tk2Queue,
				DISABLE,
				hwnd,
				RG_W_TK2_PSW_CHECKING,
				&mbTxMsg,
				1000
				);
//				memset(&float2psw,0,sizeof(float2_t));
			f_read_user_settings(hwnd);
		}
		break;
		case VK_UPDATE_WINDOW:{
			if(wParam==ID_READ_US_MODBUS_ADDR){
				tk2usProcess=ProcessBusy;
			}
			/* Windows-повідомлення, сформоване користувачам, тобто мною
			 * у функції: f_desktop_tk_session(), файл a3tk2TxRx.
			 * WPARAM wParam - RW_case, дентифікатор типу конкретного повідомлення */
			/***********************************************************************************************************
			* РОБОТА З ЧЕРГОЮ ПОВІДОМЛЕНЬ МОДБАС
			***********************************************************************************************************/
			f_tk2usUpdateWindow(hwnd);
			f_tk2UpdateStat(
						hwnd,
						hWndProgressBar_usQueue,
						p10usTimer,
						IDT_usTIMER_TK2_10MS
						);
		}
		break;
		default:{
		return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}
	return 0;

}


BOOL f_tk2usUpdateWindow(HWND hwnd){

	EnumChildWindows(hwnd, ChildCallbacktk2us, 0);
	InvalidateRect(hwnd, NULL, TRUE);
	UpdateWindow(hwnd);
	return true;
}
static uint16_t iMaxd=0xFFFF;
static uint16_t iMaxf=0xFFFF;
static uint16_t iMind=0xFFFF;
static uint16_t iMinf=0xFFFF;
static uint16_t iSamozapusk=0xFFFF;
static uint16_t iPumpingMode=0xFFFF;
static uint16_t iModbusAddr=0xFFFF;

static void f_tk2usUpdateInit(void){
	  iMaxd=0xFFFF;
	  iMaxd=0xFFFF;
	  iMaxf=0xFFFF;
	  iMind=0xFFFF;
	  iMinf=0xFFFF;
	  iSamozapusk=0xFFFF;
	  iPumpingMode=0xFFFF;
	  iModbusAddr =0xFFFF;
}
static BOOL CALLBACK ChildCallbacktk2us(
		HWND   hwnd,
		LPARAM lParam
)
{

  char as[45]={0};
  if (hwnd == hEditAmperageMax) {
	  if(!((Itk2_Max.d == iMaxd)&&(Itk2_Max.f == iMaxf))){
		  snprintf(as,23,"%3d.%1d",Itk2_Max.d,Itk2_Max.f/1000);
		  SetWindowText(hEditAmperageMax,(LPCTSTR)as);
		  iMaxd=Itk2_Max.d;
		  iMaxf=Itk2_Max.f;

	  }
	  return true;
  }

  if (hwnd == hEditAmperageMin) {
	  if(!((Itk2_Min.d == iMind)&&(Itk2_Min.f == iMinf))){
		snprintf(as,23,"%3d.%1d",Itk2_Min.d,Itk2_Min.f/1000);
		SetWindowText(hEditAmperageMin,(LPCTSTR)as);
		  iMind=Itk2_Min.d;
		  iMinf=Itk2_Min.f;
	  }
  }
  if (hwnd == SelfWnd) {
	  if((uint16_t)q.d_FS != iSamozapusk){
		switch(q.d_FS){  	    		                    //1234567890123456789012345678901234567890
    	case DISABLE:{snprintf(as,23," Заборонено           ");}break;
    	case ENABLE :{snprintf(as,23," Дозволено            ");}break;
    	default:{}
		}
	  SendMessage(SelfWnd, CB_SETCURSEL, (uint16_t)q.d_FS, 0);
	  iSamozapusk = q.d_FS;
	  }
  }
  if (hwnd == OpModeWnd) {     	switch(q.tk2_PumpingMode){
  	  //1234567890123456789012345678901234567890123
//  	  case 0:{snprintf(as,43," Відкачка, датчики рівня              (ОЭ)");}break;
//  	  case 1:{snprintf(as,43," Подача, ЕКМ                          (ПЕ)");}break;
//  	  case 2:{snprintf(as,43," Подача, датчики рівня                (ПЭ)");}break;
//  	  case 3:{snprintf(as,43," Подача, реле тиску/поплавковий датчик(ПП)");}break;
//  	  case 4:{snprintf(as,43," Відкачка, поплавковий датчик         (ОП)");}break;
//  	  case 5:{snprintf(as,43," Подача, датчик тиску                 (ПО)");}break;
//  	  default:{}
      if( iPumpingMode != (uint16_t)q.tk2_PumpingMode){
    	  SendMessage(OpModeWnd, CB_SETCURSEL, (uint16_t)q.tk2_PumpingMode, 0);
    	  iPumpingMode = (uint16_t)q.tk2_PumpingMode;
      }
}  //switch ((UINT)wParam){
  }
  if (hwnd == hEditMBAddr) {
	  if(iModbusAddr != tk2_Addr){
		  snprintf (as,20,"%X",tk2_Addr);
		  SetWindowText(hEditMBAddr,(LPCTSTR)as);
		  iModbusAddr = tk2_Addr;
	  }

  }
  //https://stackoverflow.com/questions/30786481/winapi-c-how-to-completeley-wipe-clear-everything-in-window
  return TRUE;
}

/* Читання всіх налаштувань користувача з тк2
 * Викликається при натисканні кнопки ЧИТАТИ НАЛАШТУВАННЯ
 * та при перший раз при виклику налаштувань з вікна дист.управління*/
int f_read_user_settings(HWND hwnd){

	modbus_master_tx_msg_t mbTxMsg;
	memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
	/* Запит мас.струму */
	f_Set112TxReadReg(RG_R_TK2_MAX_AMPERAGE,&mbTxMsg);
	f_set_tkqueue(
			&tk2Queue,
			DISABLE,
			hwnd,
			ID_READ_US_MAX_AMPERAGE,
			&mbTxMsg,
			1000
			);
	/* Запит мin.струму */
	f_Set112TxReadReg(RG_R_TK2_MIN_AMPERAGE,&mbTxMsg);
	f_set_tkqueue(
			&tk2Queue,
			DISABLE,
			hwnd,
			ID_READ_US_MIN_AMPERAGE,
			&mbTxMsg,
			1000
			);
		/* Запит мін.струму */
		f_Set112TxReadReg(RG_R_TK2_SELFSTART,&mbTxMsg);
		f_set_tkqueue(
				&tk2Queue,
				DISABLE,
				hwnd,
				ID_READ_US_SELF_START,
				&mbTxMsg,
				1000
				);
		/* Запит подача/відкачка/датчики */
		f_Set112TxReadReg(RG_R_TK2_PUMPING_MODE,&mbTxMsg);
		f_set_tkqueue(
				&tk2Queue,
				DISABLE,
				hwnd,
				ID_READ_US_PUMPING_MODE,
				&mbTxMsg,
				1000
				);
		/* Запит адреси Модбас */
		f_Set112TxReadReg(RG_R_TK2_MODBUS_ADDR,&mbTxMsg);
		f_set_tkqueue(
				&tk2Queue,
				DISABLE,
				hwnd,
				ID_READ_US_MODBUS_ADDR,
				&mbTxMsg,
				1000
				);
		return 0;
}



//HDC hDC; // создаём дескриптор ориентации текста на экране
//RECT rect; // стр-ра, определяющая размер клиентской области
//PAINTSTRUCT ps; // структура, сод-щая информацию о клиентской области (размеры, цвет и тп)
//RECT rectblue = {rect.left+500,rect.top+70,rect.left+890,rect.top+700};
//FillRect(hDC, &rectblue, 29);
//
//RECT rect0 = {rect.left+210, rect.top+yMode,rect.left+1000,rect.top+yMode+25};
//DrawText(hDC, "Режим роботи ", 14, &rect0, DT_SINGLELINE|DT_LEFT|DT_TOP);
//ReleaseDC(hwnd112, hDCr);
