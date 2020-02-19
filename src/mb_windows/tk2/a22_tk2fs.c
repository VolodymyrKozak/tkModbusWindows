/*
 * a2_tk2fs.c
 *
 *  Created on: 4 січ. 2020 р.
 *      Author: KozakVF
 */



#include "a22_tk2fs.h"

#include "a3_tk2z.h"

#include <process.h>    /* _beginthread, _endthread */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <winbase.h>
#include <windef.h>
#include <wingdi.h>
#include <winuser.h>
#include <commctrl.h>
#include <float.h> //
#include "../../mb_funcs/tw_mb.h"
#include "../../mb_funcs/tx_manual.h"
#include "../a0_win_control.h"
//#include "../a2_Tx_msg.h"
//#include "../a3_monitoring.h"

#include "../e1_passwordEdit.h"
#include "../a3_tkTxRx.h"

extern HINSTANCE hinst_tk2;
extern char tk2LogStr[512];
const char g_szClassNameTk2fs[] = "tk2fs_WndClass";
HWND hWndTk2fsCh;
tk2_fs_t fs={0};

/* Структура, яка визначає чергу
 * Оголошення цієї структури означає, що створена
 * конкретна черга з конкретним іменем, поки що пуста */
extern   queue_t tk2Queue;
//Постійна часу нагрівання 	0х0209	0х1004	90
static HWND hWnd_HeatingTimeConstant						;
//Гранична кількість пусків ЕД протягом 10хв	0x0216	0x1014
static HWND hWnd_NumberStartLimit							;

//Затримка пуску ЕД по рівню	0х0214	0х1006
static HWND hWnd_OnSensorMotorDelay					    ;
//Затримка зупинки ЕД по рівню	0х0215	0х1007
static HWND hWnd_OffSensorMotorDelay						;
//Затримка відновлення роботи датчика «сухого ходу» після аварії «сухого ходу»	0х0217	0х1012
static HWND hWnd_AftrerDryMoveDelay						;

//Вибір контр опору ізоляції/Пл.Пуск arr[26]	0x0219	0x101A		0/1?
static HWND hWnd_Insul_SoftStart_Option					;
//Контроль ізоляції 	0x020D	0х1015						;
//Уставка для контроля Изоляции двигателя  при отсутствии плавного пуска
static HWND hWnd_Insul_WithoutSoftStart					;


//Уставка порога срабатывания датчика уровня  	0х0213	0х1016
static HWND hWnd_LevelSensorLimit						;
//Уставка для запуска двигателя с Плавным Пуском
static HWND hWnd_SoftStart_prm							;
//Типорозмір станції	0x0208	0х1008
static HWND hWnd_Typorozmir								;
//Фазність	0х020А	0x101E
static HWND hWnd_Phasnost								;
HWND hWndProgressBar_fsQueue  					;

static  UINT_PTR  pFS1000Timer;
static  UINT_PTR  p10fsTimer;

ProcessState_t tk2fsProcess=ProcessIddle;
static FunctionalState tk2fsBtn_access = ENABLE;
static uint32_t tk2fsBtnSleepCntr = 0;
#define  TK2FS_BTN_BLOKING_MS         400

static void f_readFStk2(HWND hwnd);
static void f_writeFStk2(HWND hwnd, tk2_fs_t *pw);
static uint16_t dc(float a);
static uint16_t fr(float a);


/* Регістрація класу вікна, викликається при запуску програми */
LPCSTR f_RegisterTk2fsWndClass(HINSTANCE hinst){
			WNDCLASS w;
			memset(&w,0,sizeof(WNDCLASS));
			w.lpfnWndProc = WndProcTk2fs;
			w.hInstance = hinst;
			w.style = CS_HREDRAW | CS_VREDRAW;
			w.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
			w.lpszClassName = g_szClassNameTk2fs;
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
		if(!RegisterClassA(&w)
		){
	//		int Error = GetLastError();
			MessageBox(NULL, "Window Registration Failed!", "Error!",
				MB_ICONEXCLAMATION | MB_OK);
			return 0;
		}
	return w.lpszClassName;
}
HWND f_CreateTk2fsWnd(HWND hh1){
	MSG Msg;
	HWND fsHwnd = CreateWindowEx(
		0,
		g_szClassNameTk2fs,
		"ТК2.Налаштування виробника",
		(WS_OVERLAPPEDWINDOW&(~(WS_MAXIMIZEBOX|WS_MINIMIZEBOX)))|WS_BORDER|WS_VISIBLE
		,
		CW_USEDEFAULT, CW_USEDEFAULT, 1024, 800,
		NULL, NULL, hinst_tk2, NULL);

	HMENU hMenubar = CreateMenu();
	HMENU hMenu = CreateMenu();

//	AppendMenu(hMenu, MF_STRING, IDM_USER_SETTINGS, "&Налаштування користувача");
//	AppendMenu(hMenu, MF_STRING, IDM_FACILITY_SETTING_1, "&Налаштування виробника");
//	AppendMenuW(hMenu, MF_STRING, IDM_FACILITY_SETTING_2, "&Калібрування");
//	AppendMenu(hMenu, MF_STRING, IDM_COMPORT_SETTING_US, "&Налаштування СОМ-порту");
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenu, MF_STRING, IDM_QUIT_FStk2, "&Вихід");

	AppendMenu(hMenubar, MF_POPUP, (UINT_PTR) hMenu, "&Вихід");

	SetMenu(fsHwnd, hMenubar);
	ShowWindow(fsHwnd, SW_NORMAL);
	UpdateWindow(fsHwnd);

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return hWndTk2fsCh;
}


LRESULT CALLBACK WndProcTk2fs(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){

HFONT hfDefault;
/* у-координати друку параметрів і комбобоксів*/
                    							//0x0401

//Наявність прав доступу
static uint16_t y_fs_access_status 						    =80;
//==============================================================

//Постійна часу нагрівання 	0х0209	0х1004	90
static uint16_t y_HeatingTimeConstant						=150;
//Гранична кількість пусків ЕД протягом 10хв	0x0216	0x1014
static uint16_t y_NumberStartLimit							=175;

//Затримка пуску ЕД по рівню	0х0214	0х1006
static uint16_t y_OnSensorMotorDelay					    =225;
//Затримка зупинки ЕД по рівню	0х0215	0х1007
static uint16_t y_OffSensorMotorDelay						=250;
//Затримка відновлення роботи датчика «сухого ходу» після аварії «сухого ходу»	0х0217	0х1012
static uint16_t y_AftrerDryMoveDelay						=275;

//Вибір контр опору ізоляції/Пл.Пуск arr[26]	0x0219	0x101A		0/1?
static uint16_t y_Insul_SoftStart_Option					=325;
//Контроль ізоляції 	                        0x020D	0х1015
//Уставка для контроля Изоляции двигателя  при отсутствии плавного пуска
static uint16_t y_Insul_WithoutSoftStart					=350;


//Уставка порога срабатывания датчика уровня  	0х0213	0х1016
static uint16_t y_LevelSensorLimit							=425;
//Уставка для запуска двигателя с Плавным Пуском
static uint16_t y_SoftStart_prm								=450;
//Типорозмір станції	0x0208	0х1008
static uint16_t y_Typorozmir								=475;
//Фазність	0х020А	0x101E
static uint16_t y_Phasnost									=500;
/*Кнопки*/
static uint16_t y_write										=440;
static uint16_t y_exit										=465;
//u.regime=manual_mode;
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

	          /****************/
	          /* Кнопка для вводу паролю, якщо необхідно */
	          CreateWindow("BUTTON", "Ввод пароля", WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
	                           839, y_fs_access_status, 130, 20, hwnd,
	          				 (HMENU)ID_BTN_WRITE_PSW_FStk2,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
	          /****************/

	         //ГРАНИЧНА КІЛЬКІСТЬ ПУСКІВ ЕД ПРОТЯГОМ 10ХВ									0X0216	0X1014   5
	         hWnd_NumberStartLimit = CreateWindow("COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
	          	        		  639, y_NumberStartLimit, 160, 800,
	          					  hwnd, (HMENU)ID_CMB_NUMBERSTARTLIMIT ,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),NULL);
	         //SendMessage(hWnd_NumberStartLimit, CB_ADDSTRING, 0, (LPARAM)"  0  					");
	         SendMessage(hWnd_NumberStartLimit, CB_ADDSTRING, 0, (LPARAM)"  1             		");
	         SendMessage(hWnd_NumberStartLimit, CB_ADDSTRING, 0, (LPARAM)"  2                  	");
	         SendMessage(hWnd_NumberStartLimit, CB_ADDSTRING, 0, (LPARAM)"  3					");
	         SendMessage(hWnd_NumberStartLimit, CB_ADDSTRING, 0, (LPARAM)"  4      				");
	         SendMessage(hWnd_NumberStartLimit, CB_ADDSTRING, 0, (LPARAM)"  5  					");

	         SendMessage(hWnd_NumberStartLimit, CB_SETCURSEL, fs.fsNumberStartLimit-1, 0);

	                    //ВИБІР КОНТР ОПОРУ ІЗОЛЯЦІЇ/ПЛ.ПУСК                             ARR[26]		0X0219	0X101A	 0/1?
	          		    hWnd_Insul_SoftStart_Option = CreateWindow("COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
	          		        	639, y_Insul_SoftStart_Option, 160, 800,
	          					hwnd, (HMENU)ID_CMB_INSUL_SOFTSTART_OPTION ,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),NULL);
	          		    SendMessage(hWnd_Insul_SoftStart_Option, CB_ADDSTRING, 0, (LPARAM)"  0             		");
	          		    SendMessage(hWnd_Insul_SoftStart_Option, CB_ADDSTRING, 0, (LPARAM)"  1                  	");
	          		    SendMessage(hWnd_Insul_SoftStart_Option, CB_SETCURSEL, fs.fsInsul_SoftStart_Option, 0);

	          		    //ТИПОРОЗМІР СТАНЦІЇ															0X0208	0Х1008    3
	          //          #define ID_CMB_TYPOROZMIR
	          		    hWnd_Typorozmir = CreateWindow("COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
	          		    		639, y_Typorozmir, 160, 800,
	          					hwnd, (HMENU)ID_CMB_TYPOROZMIR ,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),NULL);
	          		    SendMessage(hWnd_Typorozmir, CB_ADDSTRING, 0, (LPARAM)"  0             		");
	          		    SendMessage(hWnd_Typorozmir, CB_ADDSTRING, 0, (LPARAM)"  1             		");
	          		    SendMessage(hWnd_Typorozmir, CB_ADDSTRING, 0, (LPARAM)"  2                  ");
	          		    SendMessage(hWnd_Typorozmir, CB_ADDSTRING, 0, (LPARAM)"  3					");
	          		    SendMessage(hWnd_Typorozmir, CB_ADDSTRING, 0, (LPARAM)"  4      			");
	          		    SendMessage(hWnd_Typorozmir, CB_ADDSTRING, 0, (LPARAM)"  5  				");
	          		    SendMessage(hWnd_Typorozmir, CB_ADDSTRING, 0, (LPARAM)"  6					");
	          		    SendMessage(hWnd_Typorozmir, CB_ADDSTRING, 0, (LPARAM)"  7      			");
	          		    SendMessage(hWnd_Typorozmir, CB_ADDSTRING, 0, (LPARAM)"  8  				");
	          		    SendMessage(hWnd_Typorozmir, CB_SETCURSEL, fs.fsTyporozmir, 0);

	          		    //ФАЗНІСТЬ																	0Х020А	0X101E    3
	          			hWnd_Phasnost = CreateWindow("COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
	          			        	639, y_Phasnost, 160, 800,
	          						hwnd, (HMENU)ID_CMB_PHASNOST ,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),NULL);
	          			SendMessage(hWnd_Phasnost, CB_ADDSTRING, 0, (LPARAM)"  0 - не дозволено     ");
	          			SendMessage(hWnd_Phasnost, CB_ADDSTRING, 0, (LPARAM)"  1 фаза 1 датчик      ");
	          			SendMessage(hWnd_Phasnost, CB_ADDSTRING, 0, (LPARAM)"  2 - не дозволено     ");
	          			SendMessage(hWnd_Phasnost, CB_ADDSTRING, 0, (LPARAM)"  3 фази 2 датчики     ");
	          			SendMessage(hWnd_Phasnost, CB_ADDSTRING, 0, (LPARAM)"  3 фази 3 датчики     ");
	          			SendMessage(hWnd_Phasnost, CB_SETCURSEL, fs.fsPhasnost, 0);


	          			char str[128]={0};
	          			//ПОСТІЙНА ЧАСУ НАГРІВАННЯ 														0Х0209	0Х1004	90
	          			hWnd_HeatingTimeConstant = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", str,
	          						WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
	          						, 639, y_HeatingTimeConstant, 160, 25,
	          						hwnd, (HMENU)ID_EDIT_HEATINGTIMECONSTANT, GetModuleHandle(NULL), NULL);
	          			SendMessage(hWnd_HeatingTimeConstant, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));


	          			//ЗАТРИМКА ПУСКУ ЕД ПО РІВНЮ													0Х0214	0Х1006   4..
	          			hWnd_OnSensorMotorDelay = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", str,
	          						WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
	          						, 639, y_OnSensorMotorDelay, 160, 25,
	          						hwnd, (HMENU)ID_EDIT_ONSENSORMOTORDELAY, GetModuleHandle(NULL), NULL);
	          			SendMessage(hWnd_OnSensorMotorDelay, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

	          			//ЗАТРИМКА ЗУПИНКИ ЕД ПО РІВНЮ													0Х0215	0Х1007   12
	          			hWnd_OffSensorMotorDelay = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", str,
	          						WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
	          						, 639, y_OffSensorMotorDelay, 160, 25,
	          						hwnd, (HMENU)ID_EDIT_OFFSENSORMOTORDELAY, GetModuleHandle(NULL), NULL);
	          			SendMessage(hWnd_OffSensorMotorDelay, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

	          			//ЗАТРИМКА ВІДНОВЛЕННЯ РОБОТИ ДАТЧИКА «СУХОГО ХОДУ» ПІСЛЯ АВАРІЇ «СУХОГО ХОДУ»	0Х0217	0Х1012   13
	          			hWnd_AftrerDryMoveDelay = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", str,
	          						WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
	          						, 639, y_AftrerDryMoveDelay, 160, 25,
	          						hwnd, (HMENU)ID_EDIT_AFTRERDRYMOVEDELAY, GetModuleHandle(NULL), NULL);
	          			SendMessage(hWnd_AftrerDryMoveDelay, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

	          			//УСТАВКА ДЛЯ КОНТРОЛЯ ИЗОЛЯЦИИ ДВИГАТЕЛЯ
	          			//ПРИ ОТСУТСТВИИ ПЛАВНОГО ПУСКА  											    0X020D	0Х1015   381
	          //			#define ID_EDIT_INSUL_WITHOUTSOFTSTART
	          			hWnd_Insul_WithoutSoftStart = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", str,
	          						WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
	          						, 639, y_Insul_WithoutSoftStart, 160, 25,
	          						hwnd, (HMENU)ID_EDIT_INSUL_WITHOUTSOFTSTART, GetModuleHandle(NULL), NULL);
	          			SendMessage(hWnd_Insul_WithoutSoftStart, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

	          			//УСТАВКА ПОРОГА СРАБАТЫВАНИЯ ДАТЧИКА УРОВНЯ  					arr21		    0Х0213	0Х1016   1001
	          			hWnd_LevelSensorLimit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", str,
	          						WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
	          						, 639, y_LevelSensorLimit, 160, 25,
	          						hwnd, (HMENU)ID_EDIT_LEVELSENSORLIMIT, GetModuleHandle(NULL), NULL);
	          			SendMessage(hWnd_LevelSensorLimit, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

	          			//УСТАВКА ДЛЯ ЗАПУСКА ДВИГАТЕЛЯ С ПЛАВНЫМ ПУСКОМ                arr[28]         0x021A  0x101C    700 код АЦП
	          //			#define ID_EDIT_SOFTSTART_PRM
	          			hWnd_SoftStart_prm = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", str,
	          						WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
	          						, 639, y_SoftStart_prm, 160, 25,
	          						hwnd, (HMENU)ID_EDIT_SOFTSTART_PRM, GetModuleHandle(NULL), NULL);
	          			SendMessage(hWnd_SoftStart_prm, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));


//	          			/* Запускаємо таймер, щоб зчитати заводські налаштування */
//	          			pFS1000Timer=SetTimer(
//	          				hwnd,                // handle to main window
//	          				IDT_TIMER_FSTK2_1000MS,// timer identifier
//	          				4000,                // msecond interval
//	          			    (TIMERPROC) NULL     // no timer callback
//	          				);

	          			/* Безумовний прогрес-бар для черги повідомлень */
	          			hWndProgressBar_fsQueue = CreateWindowEx(
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
	          			if (!hWndProgressBar_fsQueue){
	          								 	MessageBox(NULL, "Progress Bar Failed", "Error", MB_OK | MB_ICONERROR);
	          			}

	          			SendMessage(hWndProgressBar_fsQueue, PBM_SETRANGE, 0, MAKELPARAM(
	          				0, 		//minimum range value
	          				10000  //maximum range value
	          				));
	          			SendMessage(hWndProgressBar_fsQueue, PBM_SETPOS,
	          				0, //Signed integer that becomes the new position.
	          				0  //Must be zero
	          				);
	          			/* таймер для роботи з процес-баром черги повідомлень */
	          			p10fsTimer=SetTimer(
	          				hwnd,                	// handle to main window
	          				IDT_fsTIMER_TK2_10MS,  	// timer identifier
	          				TIMER_MAIN_MS,          // msecond interval
	          				(TIMERPROC) NULL     	// no timer callback
	          				);
	          			pFS1000Timer=SetTimer(
	          				hwnd,                	// handle to main window
							IDT_TIMER_FSTK2_1000MS,  	// timer identifier
							TIMER_PERIOD_TK2_1000_MS,          // msecond interval
	          				(TIMERPROC) NULL     	// no timer callback
	          				);

	          			 /* Кнопки на правій панелі */
	          			 CreateWindow("BUTTON", "Записати", WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
	          					        		  839, y_write, 130, 20, hwnd,
	          									 (HMENU)ID_BTN_WRITE_FStk2,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
	          		     CreateWindow("BUTTON", "Вийти", WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
	          					        		  839, y_exit, 130, 20, hwnd,
	          									 (HMENU)ID_BTN_EXIT_FStk2,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
	    			 	 CreateWindow("BUTTON", "Читати з ТК2",
	    		                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
												 40, 90,  110, 40, hwnd,
												 (HMENU)ID_BTN_READ_FStk2,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
						 /* Кидаємо в чергу запити на зчитування заводських налаштувань
						  * регістр за регістром*/
						 memset(&fs,0,sizeof(tk2_fs_t));

						 f_readFStk2(hwnd);


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
	    	  DrawText(hDC, "Налаштування виробника", 23, &rectL, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  //Замальовуємо область справа центру синім
	    	  RECT rectblue = {rect.left+619,rect.top+70,rect.left+819,rect.top+700};
	    	  FillRect(hDC, &rectblue, (HBRUSH)29);
	    	  /* Синій фон для параметрів*/

	    	  /* Перший параметер */
	    	  	    	  // Наявність прав доступу
	    	  	    	  static uint16_t y_fs_access_status 						    =80;
	    	  	    	  //==============================================================
	    	  	    	  RECT rectcp = {rect.left+210, rect.top+y_fs_access_status,rect.left+1000,rect.top+25+y_fs_access_status};
	    	  	    	                 //123456789012345678901234567890
	    	  	    	  DrawText(hDC, "Наявність прав доступу ", 22, &rectcp, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	  	    	  RECT rectas = {rect.left+639, rect.top+y_fs_access_status,rect.left+819,rect.top+y_fs_access_status+25};
	    	  	    	  if(us_access_status==DISABLE){
	    	  	    		               //12345678901234567890123456789012345678901234567
	    	  	    		  DrawText(hDC, "Корегувати заборонено",22, &rectas, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  	    	  }
	    	  	    	  else if (us_access_status==ENABLE){
	    	  	    		  DrawText(hDC, "Корегувати дозволено ",22, &rectas, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  	    	  }

	    	  	    	  //Постійна часу нагрівання 	0х0209	0х1004	90
	    	  	    	  RECT rectHtc = {rect.left+210, rect.top+y_HeatingTimeConstant,rect.left+1000,rect.top+y_HeatingTimeConstant+25};
	    	  	    	               //123456789012345678901234567890
	    	  	    	  DrawText(hDC, "Постійна часу нагрівання, c ", 29, &rectHtc, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	  	    	  //Гранична кількість пусків ЕД протягом 10хв	0x0216	0x1014
	    	  	    	  RECT rectNsl = {rect.left+210, rect.top+y_NumberStartLimit,rect.left+1000,rect.top+y_NumberStartLimit+25};
	    	  	    	               //12345678901234567890123456789012345678901234567
	    	  	    	  DrawText(hDC, "Гранична кількість пусків ЕД протягом 10хв", 43, &rectNsl, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	  	    	  //Затримка пуску ЕД по рівню	0х0214	0х1006
	    	  	    	  RECT rectOsmd = {rect.left+210, rect.top+y_OnSensorMotorDelay,rect.left+1000,rect.top+y_OnSensorMotorDelay+25};
	    	  	    	               //12345678901234567890123456789012345678901234567
	    	  	    	  DrawText(hDC, "Затримка пуску ЕД по рівню ", 27, &rectOsmd, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	  	    	  //Затримка зупинки ЕД по рівню	0х0215	0х1007
	    	  	    	  RECT rectFsmd = {rect.left+210, rect.top+y_OffSensorMotorDelay,rect.left+1000,rect.top+y_OffSensorMotorDelay+25};
	    	  	    	               //12345678901234567890123456789012345678901234567
	    	  	    	  DrawText(hDC, "Затримка зупинки ЕД по рівню ", 30, &rectFsmd, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	  	    	  //Затримка відновлення роботи датчика «сухого ходу» після аварії «сухого ходу»	0х0217	0х1012
	    	  	    	  static uint16_t y_AftrerDryMoveDelay						=275;
	    	  	    	  RECT rectAdm = {rect.left+210, rect.top+y_AftrerDryMoveDelay,rect.left+1000,rect.top+y_AftrerDryMoveDelay+25};
	    	  	    	               //12345678901234567890123456789012345678901234567
	    	  	    	  DrawText(hDC, "Затримка відновлення роботи датчика «сухого ходу»",
	    	  	    			  49, &rectAdm, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  	    	  RECT rectAdm1 = {rect.left+210, rect.top+y_AftrerDryMoveDelay+20,rect.left+1000,rect.top+y_AftrerDryMoveDelay+25+20};
	    	  	    	  DrawText(hDC, "після аварії «сухого ходу ",
	    	  	    		    	  	    			  27, &rectAdm1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  	    	  //Вибір контр опору ізоляції/Пл.Пуск arr[26]	0x0219	0x101A		0/1?
	    	  	    	  static uint16_t y_Insul_SoftStart_Option					=325;
	    	  	    	  RECT rectIso = {rect.left+210, rect.top+y_Insul_SoftStart_Option,rect.left+1000,rect.top+y_Insul_SoftStart_Option+25};
	    	  	    	               //12345678901234567890123456789012345678901234567
	    	  	    	  DrawText(hDC, "Вибір контр. опору ізоляції/Пл.Пуск", 36, &rectIso, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	  	    	  //Уставка для контроля Изоляции двигателя  при отсутствии плавного пуска 0x020D	0х1015
	    	  	    	  RECT rectIws = {rect.left+210, rect.top+y_Insul_WithoutSoftStart,rect.left+1000,rect.top+y_Insul_WithoutSoftStart+25};
	    	  	    	               //12345678901234567890123456789012345678901234567890234
	    	  	    	  DrawText(hDC, "Уст.контролю ізоляції при відсутності плавного пуску", 53, &rectIws, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	  	    	  //Уставка порога срабатывания датчика уровня  	0х0213	0х1016
	    	  	    	  RECT rectLsl = {rect.left+210, rect.top+y_LevelSensorLimit,rect.left+1000,rect.top+y_LevelSensorLimit+25};
	    	  	    	               //12345678901234567890123456789012345678901234567
	    	  	    	  DrawText(hDC, "Уст.порога спрацювання датчика рівня", 37, &rectLsl, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	  	    	  //Уставка для запуска двигателя с Плавным Пуском
	    	  	    	  RECT rectSs = {rect.left+210, rect.top+y_SoftStart_prm,rect.left+1000,rect.top+y_SoftStart_prm+25};
	    	  	    	               //12345678901234567890123456789012345678901234567
	    	  	    	  DrawText(hDC, "Уст.для електродвигуна з плавним пуском", 40, &rectSs, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	  	    	  //Типорозмір станції	0x0208	0х1008
	    	  	    	  RECT rectTr = {rect.left+210, rect.top+y_Typorozmir,rect.left+1000,rect.top+y_Typorozmir+25};
	    	  	    	               //12345678901234567890123456789012345678901234567
	    	  	    	  DrawText(hDC, "Типорозмір", 11, &rectTr, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	  	    	  //Фазність	0х020А	0x101E
	    	  	    	  RECT rectTr7 = {rect.left+210, rect.top+y_Phasnost,rect.left+1000,rect.top+y_Phasnost+25};
	    	  	    	               //12345678901234567890123456789012345678901234567
	    	  	    	  DrawText(hDC, "Фазність", 9, &rectTr7, DT_SINGLELINE|DT_LEFT|DT_TOP);

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
//    	switch (HIWORD(wParam)) {
//			/* EDIT BOXES HANDLING якщо виявлено вибір користувача*/
//			case CBN_SELCHANGE:{
//				/* Запит - який саме номер по порядку з меню вибраний (незалежно від комбобоксу */
//	            int ItemIndex = SendMessage((HWND) lParam, (UINT) CB_GETCURSEL,
//	                (WPARAM) 0, (LPARAM) 0);
//
//				UINT idCtl=LOWORD(wParam);
//				/* Переносимо зроблений вибір у заголовок комбобокса*/
//				/* Ой.. Здається система це робить сама :)*/
//				// SendMessage((HWND)lParam, CB_SETCURSEL, idCtl, 0);
//				/* в залежності від параметру приймаємо вибрані значення параметрів*/
//				switch(idCtl){
//				    //ГРАНИЧНА КІЛЬКІСТЬ ПУСКІВ ЕД ПРОТЯГОМ 10ХВ
//					case ID_CMB_NUMBERSTARTLIMIT:
//						fsw.fsNumberStartLimit=ItemIndex+1;
//					break;
//					//ВИБІР КОНТР ОПОРУ ІЗОЛЯЦІЇ/ПЛ.ПУСК
//					case ID_CMB_INSUL_SOFTSTART_OPTION:
//						fsw.fsInsul_SoftStart_Option=ItemIndex;
//					break;
//					//ТИПОРОЗМІР СТАНЦІЇ
//					//ID_CMB_TYPOROZMIR:
//					case ID_CMB_TYPOROZMIR:
//						fsw.fsTyporozmir=ItemIndex;
//						break;
//					//ФАЗНІСТЬ
//					case ID_CMB_PHASNOST:
//						fsw.fsPhasnost=ItemIndex+1;
//					break;
//					default:{}
//				} // switch(idCtl)
//			}//case CBN_SELCHANGE:
//			break;
//			default:{};
//    		}//switch (HIWORD(wParam)) {
		switch(wParam) {
		    case ID_BTN_READ_FStk2:{
		    	/* Кидаємо в чергу запити на зчитування налаштувань
		    	 * регістр за регістром*/
		    	f_readFStk2(hwnd);
		    	/* Запускаємо таймер очікування відповіді*/
				pFS1000Timer=SetTimer(
					hwnd,                // handle to main window
					IDT_TIMER_FSTK2_1000MS,// timer identifier
					4000,                // msecond interval
				    (TIMERPROC) NULL     // no timer callback
					);
		    }
		    break;
		    case ID_BTN_WRITE_FStk2:{
		    	tk2_fs_t fsq={0};
		    	int ItemIndex=0;
			    //ГРАНИЧНА КІЛЬКІСТЬ ПУСКІВ ЕД ПРОТЯГОМ 10ХВ
		    	ItemIndex=SendDlgItemMessage(hwnd,ID_CMB_NUMBERSTARTLIMIT,CB_GETCURSEL,0,0);
				fsq.fsNumberStartLimit=ItemIndex+1;

				//ВИБІР КОНТР ОПОРУ ІЗОЛЯЦІЇ/ПЛ.ПУСК
				ItemIndex=SendDlgItemMessage(hwnd,ID_CMB_INSUL_SOFTSTART_OPTION,CB_GETCURSEL,0,0);
				fsq.fsInsul_SoftStart_Option=ItemIndex;

				//ТИПОРОЗМІР СТАНЦІЇ
				//ID_CMB_TYPOROZMIR:
				ItemIndex=SendDlgItemMessage(hwnd,ID_CMB_TYPOROZMIR,CB_GETCURSEL,0,0);
				fsq.fsTyporozmir=ItemIndex;

				//ФАЗНІСТЬ
				ItemIndex=SendDlgItemMessage(hwnd,ID_CMB_PHASNOST,CB_GETCURSEL,0,0);
				fsq.fsPhasnost=ItemIndex;
		    	/* Зчитуємо вміст Едіт-боксів,
		    	 * значення комбобоксів ВЖЕ!! зчитані в змінну при виході користувача
		    	 * з кожного з комбобоксів */
		    	int err = 0;
		    	while (err==0){
		    		float2_t fl={0};
		    	//По черзі зчитуємо ЕдітБокси. Якщо там помилка - повідомляємо користувача і         виходимо по помилці
		    	//ПОСТІЙНА ЧАСУ НАГРІВАННЯ 							0Х0209	0Х1004	90
		    		err = f_checkEdit(ID_EDIT_HEATINGTIMECONSTANT, hwnd, &fl);
		    		if(err != 0){f_valueEditMessageBox(err);break;};
		    		fsq.fsHeatingTimeConstant=fl.d;
		    	//ЗАТРИМКА ПУСКУ ЕД ПО РІВНЮ						0Х0214	0Х1006   4..
		    		err = f_checkEdit(ID_EDIT_ONSENSORMOTORDELAY, hwnd, &fl);
		    		if(err != 0){f_valueEditMessageBox(err);break;};
		    		fsq.fsOnSensorMotorDelay=fl.d;
		    	//ЗАТРИМКА ЗУПИНКИ ЕД ПО РІВНЮ						0Х0215	0Х1007   12
		    		err = f_checkEdit(ID_EDIT_OFFSENSORMOTORDELAY, hwnd, &fl);
		    		if(err != 0){f_valueEditMessageBox(err);break;};
		    		fsq.fsOffSensorMotorDelay=fl.d;
		    	//ЗАТРИМКА ВІДНОВЛЕННЯ РОБОТИ ДАТЧИКА «СУХОГО ХОДУ» ПІСЛЯ АВАРІЇ «СУХОГО ХОДУ»
		    	//													0Х0217	0Х1012   13
		    		err = f_checkEdit(ID_EDIT_AFTRERDRYMOVEDELAY, hwnd, &fl);
		    		if(err != 0){f_valueEditMessageBox(err);break;}
		    		fsq.fsAftrerDryMoveDelay=fl.d;
		    	//УСТАВКА ДЛЯ КОНТРОЛЯ ИЗОЛЯЦИИ ДВИГАТЕЛЯ
		    	//ПРИ ОТСУТСТВИИ ПЛАВНОГО ПУСКА  					0X020D	0Х1015   381
		    		err = f_checkEdit(ID_EDIT_INSUL_WITHOUTSOFTSTART,hwnd,&fl);
		    		if(err != 0){f_valueEditMessageBox(err);break;}
		    		fsq.fsInsul_WithoutSoftStart=fl.d;
		    	//УСТАВКА ПОРОГА СРАБАТЫВАНИЯ ДАТЧИКА УРОВНЯ  		0Х0213	0Х1016   1001
		    		err = f_checkEdit(ID_EDIT_LEVELSENSORLIMIT, hwnd, &fl);
		    		if(err != 0){f_valueEditMessageBox(err);break;}
		    		fsq.fsLevelSensorLimit=fl.d;
		    	//УСТАВКА ДЛЯ ЗАПУСКА ДВИГАТЕЛЯ С ПЛАВНЫМ ПУСКОМ    0x021A  0x101C    700 код АЦП
		    		err = f_checkEdit(ID_EDIT_SOFTSTART_PRM, hwnd, &fl);
		    		if(err != 0){f_valueEditMessageBox(err);break;}
		    		fsq.fsSoftStart_prm	=fl.d;

		    		f_writeFStk2(hwnd, &fsq);
		    		f_readFStk2(hwnd);

		    		snprintf(tk2LogStr,511,"ko: Змінити уставки виробника на:"),
		    		f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));
		    		snprintf(tk2LogStr,511,"%d, ГРАНИЧНА КІЛЬКІСТЬ ПУСКІВ ЕД ПРОТЯГОМ 10ХВ",
		    				fsq.fsNumberStartLimit);
		    		f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));
		    		snprintf(tk2LogStr,511,"%d, ВИБІР КОНТР ОПОРУ ІЗОЛЯЦІЇ/ПЛ.ПУСК",
		    				fsq.fsInsul_SoftStart_Option);
		    		f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));
		    		snprintf(tk2LogStr,511,"%d, ТИПОРОЗМІР СТАНЦІЇ",
		    				fsq.fsTyporozmir);
		    		f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));
		    		snprintf(tk2LogStr,511,"%d, ФАЗНІСТЬ",
		    				fsq.fsPhasnost);
		    		f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));
		    		snprintf(tk2LogStr,511,"%d, ПОСТІЙНА ЧАСУ НАГРІВАННЯ",// 							0Х0209	0Х1004	90
		    				fsq.fsHeatingTimeConstant);
		    		f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));
		    		snprintf(tk2LogStr,511,"%d, ЗАТРИМКА ПУСКУ ЕД ПО РІВНЮ",//						0Х0214	0Х1006   4..
		    		    	fsq.fsOnSensorMotorDelay);
		    		f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));
		    		snprintf(tk2LogStr,511,"%d, ЗАТРИМКА ЗУПИНКИ ЕД ПО РІВНЮ",//						0Х0215	0Х1007   12
		    		    	fsq.fsOffSensorMotorDelay);
		    		f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));
		    		snprintf(tk2LogStr,511,"%d, ЗАТРИМКА ВІДНОВЛЕННЯ РОБОТИ ДАТЧИКА «СУХОГО ХОДУ» ПІСЛЯ АВАРІЇ «СУХОГО ХОДУ»",//
		    		    	fsq.fsAftrerDryMoveDelay);
		    		f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));
		    		snprintf(tk2LogStr,511,"    УСТАВКА ДЛЯ КОНТРОЛЯ ИЗОЛЯЦИИ ДВИГАТЕЛЯ");//
		    		f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));
		    		snprintf(tk2LogStr,511,"%d, ПРИ ОТСУТСТВИИ ПЛАВНОГО ПУСКА",//  					0X020D	0Х1015   381
		    		    	fsq.fsInsul_WithoutSoftStart);
		    		f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));
		    		snprintf(tk2LogStr,511,"%d, УСТАВКА ПОРОГА СРАБАТЫВАНИЯ ДАТЧИКА УРОВНЯ",//  		0Х0213	0Х1016   1001
		    		    	fsq.fsLevelSensorLimit);
		    		f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));
		    		snprintf(tk2LogStr,511,"%d, УСТАВКА ДЛЯ ЗАПУСКА ДВИГАТЕЛЯ С ПЛАВНЫМ ПУСКОМ",//    0x021A  0x101C    700 код АЦП
		    		    	fsq.fsSoftStart_prm);
		    		f_tk2Logging(tk2LogStr, sizeof(tk2LogStr));
		    		err=0xffff;/* для виходу з while */
		       }// while (err==0){
			}//case ID_BTN_WRIRE_FStk2:{
			break;
			case ID_BTN_WRITE_PSW_FStk2:{
				/* Викликаємо вікно вводу пароля, яке
				 * поверне пароль в case VK_PSW    */
				CreatePswWindow(hwnd);
			}
			break;
			case ID_BTN_EXIT_FStk2:

				DestroyWindow(hwnd);
    		break;
			case IDM_QUIT_FStk2:

				DestroyWindow(hwnd);
    		break;
			default:{}
		}//switch(wParam) {
		}//WM_COMMAND
		break;
		/*************************************************************************************************************************************
		*      TIMERS 																						   TIMERS 					TIMERS
		**************************************************************************************************************************************/
        case WM_TIMER:{
			//The number of milliseconds that have elapsed since the system was started. This is the value returned by the GetTickCount function.
			switch ((UINT)wParam){
			case IDT_TIMER_FSTK2_1000MS:{
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
			break; //case WM_TIMER;
			case IDT_fsTIMER_TK2_10MS:{
				  /* Якщо у вікні натиснута кнопка - вмикаємо лічильник затримки */
				  if((tk2fsProcess==ProcessBusy)&&(tk2fsBtn_access==DISABLE)){
					  tk2fsBtnSleepCntr+=10;
					  if(tk2fsBtnSleepCntr>=TK2FS_BTN_BLOKING_MS){
						  tk2fsBtn_access=ENABLE;
						  tk2fsBtnSleepCntr=0;
					  }
				  }
			}
			break;
			default:{}
        }//switch ((UINT)wParam){
        }//WM_TIMER:{
        break;

		case WM_CLOSE:{

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
				300
				);
//				memset(&float2psw,0,sizeof(float2_t));

		}
		break;
		case VK_UPDATE_WINDOW:{
			char as[45]={0};
			/* Windows-повідомлення, сформоване користувачам, тобто мною
			 * у функції: f_desktop_tk_session(), файл a3tk2TxRx.
			 * WPARAM wParam - RW_case, дентифікатор типу конкретного повідомлення */
			switch(wParam){
			/* Перевірка статусу прав доступу  */
			case RG_R_TK2_ACCESS_STATUS:{
				  /* рядок динамічно для підготовки тексту у вікно*/
				  HDC hDC; // создаём дескриптор ориентации текста на экране
				  RECT rect; // стр-ра, определяющая размер клиентской области
				  /*малюємо заново*/
				  hDC= GetDC(hwnd);
				  GetClientRect(hwnd, &rect);  	// получаем ширину и высоту области для рисования
	  	    	  // Наявність прав доступу
	  	    	  static uint16_t y_fs_access_status 						    =80;
	  	    	  //==============================================================
	  	    	  RECT rectcp = {rect.left+210, rect.top+y_fs_access_status,rect.left+1000,rect.top+25+y_fs_access_status};
	  	    	                 //123456789012345678901234567890
	  	    	  DrawText(hDC, "Наявність прав доступу ", 22, &rectcp, DT_SINGLELINE|DT_LEFT|DT_TOP);

	  	    	  RECT rectas = {rect.left+639, rect.top+y_fs_access_status,rect.left+819,rect.top+y_fs_access_status+25};
	  	    	  if(us_access_status==DISABLE){
	  	    		               //12345678901234567890123456789012345678901234567
	  	    		  DrawText(hDC, "Корегувати заборонено",22, &rectas, DT_SINGLELINE|DT_LEFT|DT_TOP);
	  	    	  }
	  	    	  else if (us_access_status==ENABLE){
	  	    		  DrawText(hDC, "Корегувати дозволено ",22, &rectas, DT_SINGLELINE|DT_LEFT|DT_TOP);
	  	    	  }
	  	    	  else{}
	  	          ReleaseDC(hwnd, hDC);
			}
			break;
			//ПОСТІЙНА ЧАСУ НАГРІВАННЯ 														0Х0209	0Х1004	90
			case 		0x0209:{
				  snprintf(as,23,"%d",fs.fsHeatingTimeConstant);
				  SetWindowText(hWnd_HeatingTimeConstant,(LPCTSTR)as);
			}
			break;
			//ГРАНИЧНА КІЛЬКІСТЬ ПУСКІВ ЕД ПРОТЯГОМ 10ХВ									0X0216	0X1014   5
			case 		0x0216:
				  SendMessage(hWnd_NumberStartLimit, CB_SETCURSEL, fs.fsNumberStartLimit-1, 0);
			break;
			//	//ЗАТРИМКА ПУСКУ ЕД ПО РІВНЮ													0Х0214	0Х1006   4..
			//	#define ID_EDIT_ONSENSORMOTORDELAY					    225
			case 		0x0214:{
				  memset(&as,0,sizeof(as));
				  snprintf(as,23,"%d",fs.fsOnSensorMotorDelay);
				  SetWindowText(hWnd_OnSensorMotorDelay,(LPCTSTR)as);
			}
			break;
			//	//ЗАТРИМКА ЗУПИНКИ ЕД ПО РІВНЮ													0Х0215	0Х1007   12
			//	#define ID_EDIT_OFFSENSORMOTORDELAY						250
			case 		0x0215:{
				  memset(&as,0,sizeof(as));
				  snprintf(as,23,"%d",fs.fsOffSensorMotorDelay);
				  SetWindowText(hWnd_OffSensorMotorDelay,(LPCTSTR)as);
			}
			break;
			//	//ЗАТРИМКА ВІДНОВЛЕННЯ РОБОТИ ДАТЧИКА «СУХОГО ХОДУ» ПІСЛЯ АВАРІЇ «СУХОГО ХОДУ»	0Х0217	0Х1012   13
			//	#define ID_EDIT_AFTRERDRYMOVEDELAY_						275
			case 		0x0217:{
				  memset(&as,0,sizeof(as));
				  snprintf(as,23,"%d",fs.fsAftrerDryMoveDelay);
				  SetWindowText(hWnd_AftrerDryMoveDelay,(LPCTSTR)as);
			}
			break;
			//	//ВИБІР КОНТР ОПОРУ ІЗОЛЯЦІЇ/ПЛ.ПУСК                             ARR[26]		0X0219	0X101A	 0/1?
			//	#define ID_CMB_INSUL_SOFTSTART_OPTION					325
			case 		0x020B:
				  SendMessage(hWnd_Insul_SoftStart_Option, CB_SETCURSEL, fs.fsInsul_SoftStart_Option, 0);
			break;
			//	//УСТАВКА ДЛЯ КОНТРОЛЯ ИЗОЛЯЦИИ ДВИГАТЕЛЯ
			//	//ПРИ ОТСУТСТВИИ ПЛАВНОГО ПУСКА  											    0X020D	0Х1015   381
			//	#define ID_EDIT_INSUL_WITHOUTSOFTSTART					350
			case 		0x020D:{
				  memset(&as,0,sizeof(as));
				  snprintf(as,23,"%d",fs.fsInsul_WithoutSoftStart);
				  SetWindowText(hWnd_Insul_WithoutSoftStart,(LPCTSTR)as);
			}
			break;
			//	//УСТАВКА ПОРОГА СРАБАТЫВАНИЯ ДАТЧИКА УРОВНЯ  					arr21		    0Х0213	0Х1016   1001
			//	#define ID_EDIT_LEVELSENSORLIMIT						425
			case 		0x0213:{
				  memset(&as,0,sizeof(as));
				  snprintf(as,23,"%d",fs.fsLevelSensorLimit);
				  SetWindowText(hWnd_LevelSensorLimit,(LPCTSTR)as);
			}
			break;
			//	//УСТАВКА ДЛЯ ЗАПУСКА ДВИГАТЕЛЯ С ПЛАВНЫМ ПУСКОМ                arr[28]         0x021A  0x101C    700 код АЦП
			//	#define ID_EDIT_SOFTSTART_PRM							450
			case		0x021A:{
				  memset(&as,0,sizeof(as));
				  snprintf(as,23,"%d",fs.fsSoftStart_prm);
				  SetWindowText(hWnd_SoftStart_prm,(LPCTSTR)as);
			}
			break;
			//	//ТИПОРОЗМІР СТАНЦІЇ															0X0208	0Х1008    3
			//	#define ID_CMB_TYPOROZMIR_								475
			case		0x0208:
				  SendMessage(hWnd_Typorozmir, CB_SETCURSEL, fs.fsTyporozmir, 0);
			break;
			//	//ФАЗНІСТЬ																		0Х020А	0X101E    3
			//	#define ID_CMB_PHASNOST								    500
			case 		0x020A:
				  SendMessage(hWnd_Phasnost, CB_SETCURSEL, fs.fsPhasnost, 0);
			break;
			default:{}
			}
			/***********************************************************************************************************
			* РОБОТА З ЧЕРГОЮ ПОВІДОМЛЕНЬ МОДБАС
			***********************************************************************************************************/
			f_tk2UpdateStat(
						hwnd,
						hWndProgressBar_fsQueue,
						p10fsTimer,
						IDT_fsTIMER_TK2_10MS
						);
		}
		break;
		default:{
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}
	return 0;

}


/* Функція формування послідовності Модбас-запитів на зчитування регістрів заводських налаштувань
 * один за одним.
 * Викликається при вході в меню заводських налаштувань з WM_CREATE
 * та при натисканні кнопки ЧИТАТИ НАЛАШТУВАННЯ */
static void f_readFStk2(HWND hwnd){
	modbus_master_tx_msg_t mbTxMsg;
	memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));

	/* Перевірка статусу прав доступу  */
	f_Set112TxReadReg(RG_R_TK2_ACCESS_STATUS,&mbTxMsg);
	f_set_tkqueue(
			&tk2Queue,
			DISABLE,
			hwnd,
			RG_R_TK2_ACCESS_STATUS,
			&mbTxMsg,
			2000
			);
	//ПОСТІЙНА ЧАСУ НАГРІВАННЯ 														0Х0209	0Х1004	90
	memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
	f_Set112TxReadReg(0x0209,&mbTxMsg);
	f_set_tkqueue(
			&tk2Queue,
			DISABLE,
			hwnd,
			0x0209,
			&mbTxMsg,
			2000
			);
	//ГРАНИЧНА КІЛЬКІСТЬ ПУСКІВ ЕД ПРОТЯГОМ 10ХВ									0X0216	0X1014   5
	memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
	f_Set112TxReadReg(0x0216,&mbTxMsg);
	f_set_tkqueue(
			&tk2Queue,
			DISABLE,
			hwnd,
			0x0216,
			&mbTxMsg,
			2000
			);

//	//ЗАТРИМКА ПУСКУ ЕД ПО РІВНЮ													0Х0214	0Х1006   4..
//	#define ID_EDIT_ONSENSORMOTORDELAY					    225
	memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
	f_Set112TxReadReg(0x0214,&mbTxMsg);
	f_set_tkqueue(
			&tk2Queue,
			DISABLE,
			hwnd,
			0x0214,
			&mbTxMsg,
			2000
			);
//	//ЗАТРИМКА ЗУПИНКИ ЕД ПО РІВНЮ													0Х0215	0Х1007   12
//	#define ID_EDIT_OFFSENSORMOTORDELAY						250
	memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
	f_Set112TxReadReg(0x0215,&mbTxMsg);
	f_set_tkqueue(
			&tk2Queue,
			DISABLE,
			hwnd,
			0x0215,
			&mbTxMsg,
			2000
			);
//	//ЗАТРИМКА ВІДНОВЛЕННЯ РОБОТИ ДАТЧИКА «СУХОГО ХОДУ» ПІСЛЯ АВАРІЇ «СУХОГО ХОДУ»	0Х0217	0Х1012   13
//	#define ID_EDIT_AFTRERDRYMOVEDELAY_						275
	memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
	f_Set112TxReadReg(0x0217,&mbTxMsg);
	f_set_tkqueue(
			&tk2Queue,
			DISABLE,
			hwnd,
			0x0217,
			&mbTxMsg,
			2000
			);
//	//ВИБІР КОНТР ОПОРУ ІЗОЛЯЦІЇ/ПЛ.ПУСК                             ARR[26]		0X0219	0X101A	 0/1?
//	#define ID_CMB_INSUL_SOFTSTART_OPTION					325
	memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
	f_Set112TxReadReg(0x0219,&mbTxMsg);
	f_set_tkqueue(
			&tk2Queue,
			DISABLE,
			hwnd,
			0x0219,
			&mbTxMsg,
			2000
			);
	//	//УСТАВКА ДЛЯ КОНТРОЛЯ ИЗОЛЯЦИИ ДВИГАТЕЛЯ
//	//ПРИ ОТСУТСТВИИ ПЛАВНОГО ПУСКА  											    0X020D	0Х1015   381
//	#define ID_EDIT_INSUL_WITHOUTSOFTSTART					350
	memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
	f_Set112TxReadReg(0x020D,&mbTxMsg);
	f_set_tkqueue(
			&tk2Queue,
			DISABLE,
			hwnd,
			0x020D,
			&mbTxMsg,
			2000
			);
//	//УСТАВКА ПОРОГА СРАБАТЫВАНИЯ ДАТЧИКА УРОВНЯ  					arr21		    0Х0213	0Х1016   1001
//	#define ID_EDIT_LEVELSENSORLIMIT						425
	memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
	f_Set112TxReadReg(0x0213,&mbTxMsg);
	f_set_tkqueue(
			&tk2Queue,
			DISABLE,
			hwnd,
			0x0213,
			&mbTxMsg,
			2000
			);
//	//УСТАВКА ДЛЯ ЗАПУСКА ДВИГАТЕЛЯ С ПЛАВНЫМ ПУСКОМ                arr[28]         0x021A  0x101C    700 код АЦП
//	#define ID_EDIT_SOFTSTART_PRM							450
	memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
	f_Set112TxReadReg(0x021A,&mbTxMsg);
	f_set_tkqueue(
			&tk2Queue,
			DISABLE,
			hwnd,
			0x021A,
			&mbTxMsg,
			2000
			);
//	//ТИПОРОЗМІР СТАНЦІЇ															0X0208	0Х1008    3
//	#define ID_CMB_TYPOROZMIR_								475
	memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
	f_Set112TxReadReg(0x0208,&mbTxMsg);
	f_set_tkqueue(
			&tk2Queue,
			DISABLE,
			hwnd,
			0x0208,
			&mbTxMsg,
			2000
			);
//	//ФАЗНІСТЬ																		0Х020А	0X101E    3
//	#define ID_CMB_PHASNOST								    500
	memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
	f_Set112TxReadReg(0x020A,&mbTxMsg);
	f_set_tkqueue(
			&tk2Queue,
			DISABLE,
			hwnd,
			0x020A,
			&mbTxMsg,
			2000
			);
}

/* Функція формування послідовності Модбас-запитів на запис регістрів TK2 заводських налаштувань
 * один за одним.
 * Викликається з вікна користувацьких налаштувань при натисканні кнопки ЗАПИСАТИ НАЛАШТУВАННЯ.
 * hwnd - дескриптор вікна з якого викликається (цей параметер передається в чергу повідомлень,
 *        і після отримання відповіді - сюди ж і повертається повідомлення про завершення сесії Модбас
 * pw   - показчик на структуру параметрів зчитаних з Едітт-боксів та Комбо-боксів вікна       */
static void f_writeFStk2(HWND hwnd, tk2_fs_t *pw){
	modbus_master_tx_msg_t mbTxMsg;
//	//ПОСТІЙНА ЧАСУ НАГРІВАННЯ 														0Х0209	0Х1004	90
//	#define ID_EDIT_HEATINGTIMECONSTANT						150
		f_Set112TxWriteReg(0x1004, pw->fsHeatingTimeConstant, &mbTxMsg);
		f_set_tkqueue(
				&tk2Queue,
				DISABLE,
				hwnd,
				0x1004,
				&mbTxMsg,
				2000
				);
//	//ГРАНИЧНА КІЛЬКІСТЬ ПУСКІВ ЕД ПРОТЯГОМ 10ХВ									0X0216	0X1014   5
//	#define ID_CMB_NUMBERSTARTLIMIT						    175
		memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
		f_Set112TxWriteReg(0x1014, pw->fsNumberStartLimit, &mbTxMsg);
		f_set_tkqueue(
				&tk2Queue,
				DISABLE,
				hwnd,
				0x1014,
				&mbTxMsg,
				2000
				);
//	//ЗАТРИМКА ПУСКУ ЕД ПО РІВНЮ													0Х0214	0Х1006   4..
//	#define ID_EDIT_ONSENSORMOTORDELAY					    225
		memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
		f_Set112TxWriteReg(0x1006, pw->fsOnSensorMotorDelay, &mbTxMsg);
		f_set_tkqueue(
				&tk2Queue,
				DISABLE,
				hwnd,
				0x1006,
				&mbTxMsg,
				2000
				);
//	//ЗАТРИМКА ЗУПИНКИ ЕД ПО РІВНЮ													0Х0215	0Х1007   12
//	#define ID_EDIT_OFFSENSORMOTORDELAY						250
		memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
		f_Set112TxWriteReg(0x1007, pw->fsOffSensorMotorDelay, &mbTxMsg);
		f_set_tkqueue(
				&tk2Queue,
				DISABLE,
				hwnd,
				0x1007,
				&mbTxMsg,
				2000
				);
//	//ЗАТРИМКА ВІДНОВЛЕННЯ РОБОТИ ДАТЧИКА «СУХОГО ХОДУ» ПІСЛЯ АВАРІЇ «СУХОГО ХОДУ»	0Х0217	0Х1012   13
//	#define ID_EDIT_AFTRERDRYMOVEDELAY						275
		memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
		f_Set112TxWriteReg(0x1012, pw->fsAftrerDryMoveDelay, &mbTxMsg);
		f_set_tkqueue(
				&tk2Queue,
				DISABLE,
				hwnd,
				0x1012,
				&mbTxMsg,
				2000
				);
//	//ВИБІР КОНТР ОПОРУ ІЗОЛЯЦІЇ/ПЛ.ПУСК                             ARR[26]		0X0219	0X101A	 0/1?
//	#define ID_CMB_INSUL_SOFTSTART_OPTION					325
		memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
		f_Set112TxWriteReg(0x101A, pw->fsInsul_SoftStart_Option, &mbTxMsg);
		f_set_tkqueue(
				&tk2Queue,
				DISABLE,
				hwnd,
				0x101A,
				&mbTxMsg,
				2000
				);
//	//УСТАВКА ДЛЯ КОНТРОЛЯ ИЗОЛЯЦИИ ДВИГАТЕЛЯ
//	//ПРИ ОТСУТСТВИИ ПЛАВНОГО ПУСКА  											    0X020D	0Х1015   381
//	#define ID_EDIT_INSUL_WITHOUTSOFTSTART					350
		memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
		f_Set112TxWriteReg(0x1015, pw->fsInsul_WithoutSoftStart, &mbTxMsg);
		f_set_tkqueue(
				&tk2Queue,
				DISABLE,
				hwnd,
				0x1015,
				&mbTxMsg,
				2000
				);
//	//УСТАВКА ПОРОГА СРАБАТЫВАНИЯ ДАТЧИКА УРОВНЯ  					arr21		    0Х0213	0Х1016   1001
//	#define ID_EDIT_LEVELSENSORLIMIT						425
		memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
		f_Set112TxWriteReg(0x1016, pw->fsLevelSensorLimit, &mbTxMsg);
		f_set_tkqueue(
				&tk2Queue,
				DISABLE,
				hwnd,
				0x1016,
				&mbTxMsg,
				2000
				);
//	//УСТАВКА ДЛЯ ЗАПУСКА ДВИГАТЕЛЯ С ПЛАВНЫМ ПУСКОМ                arr[28]         0x021A  0x101C    700 код АЦП
//	#define ID_EDIT_SOFTSTART_PRM							450
		memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
		f_Set112TxWriteReg(0x101C, pw->fsSoftStart_prm, &mbTxMsg);
		f_set_tkqueue(
				&tk2Queue,
				DISABLE,
				hwnd,
				0x101C,
				&mbTxMsg,
				2000
				);
//	//ТИПОРОЗМІР СТАНЦІЇ															0X0208	0Х1008    3
//	#define ID_CMB_TYPOROZMIR								475
		memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
		f_Set112TxWriteReg(0x1008, pw->fsTyporozmir, &mbTxMsg);
		f_set_tkqueue(
				&tk2Queue,
				DISABLE,
				hwnd,
				0x1008,
				&mbTxMsg,
				2000
				);
//	//ФАЗНІСТЬ																		0Х020А	0X101E    3
//	#define ID_CMB_PHASNOST								    500
		memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
		f_Set112TxWriteReg(0x101E, pw->fsPhasnost, &mbTxMsg);
		f_set_tkqueue(
				&tk2Queue,
				DISABLE,
				hwnd,
				0x101E,
				&mbTxMsg,
				2000
				);

		snprintf(tk2LogStr,511,"Запис уставок виробника  ");
}







