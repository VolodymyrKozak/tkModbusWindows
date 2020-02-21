/*
 * a22_tk2fs.c
 *
 *  Created on: 16 лют. 2020 р.
 *      Author: KozakVF
 */

#include "a22_tk5fs.h"
#include "a3_tk5z.h"
#include <stdint.h>
#include <stdio.h>
#include "../../wConfig.h"
#include "../a3_tkTxRx.h"
#include "../e1_passwordEdit.h"
#include "../../mb_funcs/tkCRC.h"
#include "a4_tk5log.h"
#include "../../mb_funcs/vk_log.h"
#include "commdlg.h"





/* Глобальні змінні, передаються на обробку в вікно, яке повідомлення кидало в чергу */
extern modbus_master_rx_msg_t mIn_rx_msg;
/* Глобальні змінні, передаються на обробку в вікно, яке повідомлення кидало в чергу */
extern modbus_master_tx_msg_t mOut_tx_msg;
/* Глобальні змінні, передаються на обробку в вікно, яке повідомлення кидало в чергу */
extern modbus_status_t RxMasterModbusStatus;
extern queue_t tk5Queue;
extern const BYTE XORmaskTK_Icon[];


LPOPENFILENAMEA FileNameFsAs;
const char g_szClassNameTk5fs[] = "Tk5fs_WndClass";
HINSTANCE hinstTk5;
ProcessState_t tk5fsProcess=ProcessIddle;
tk5fs_t tk5fs={0};
static uint16_t y_amperageAclbr =  80; 	// 0х2000
static uint16_t y_clbr_iA       = 105;		// 0х2002
static uint16_t y_amperageBclbr = 130; 	// 0х2004
static uint16_t y_clbr_iB		 = 155;		// 0х2006
static uint16_t y_amperageCclbr = 180; 	// 0х2008
static uint16_t y_clbr_iC		 = 205;		// 0х200A
static uint16_t y_voltageCclbr	 = 230;	    // 0x200C
static uint16_t y_clbr_uC  	 = 255;		// 0х200E
static uint16_t y_DeviceMode	 = 280;     // 0x2010
static uint16_t y_Faznost		 = 305;		// 0x2011

static HWND hwnd_amperageAclbr; 	// 0х2000
static HWND hwnd_clbr_iA;			// 0х2002
static HWND hwnd_amperageBclbr; 	// 0х2004
static HWND hwnd_clbr_iB;			// 0х2006
static HWND hwnd_amperageCclbr; 	// 0х2008
static HWND hwnd_clbr_iC;			// 0х200A
static HWND hwnd_voltageCclbr;		// 0x200C
static HWND hwnd_clbr_uC;			// 0х200E
static HWND hwnd_tk5fsDeviceMode;      	// 0x2010
static HWND hwnd_tk5fsFaznost;		    // 0x2011


static  HFONT hfDefault;
static int f_facilitysettings_to_tk5Memory(HWND hwnd);
static int f_tk5fsUpdateData(HWND hwnd, WPARAM wParam, int responce_status);
static BOOL CALLBACK ChildCallbacktk5fs(HWND   hwnd,LPARAM lParam);
static size_t f_wchar_to_char(const wchar_t * src, char * dest, size_t dest_len);
static int f_tk5SetFilesWrite(void);
static int f_tk5SetFilesRead(tk5fs_t *ptk5fsr);
static int f_tk5SetFilesReadFile(tk5fs_t *ptk5fsr, char* sFileName);
static uint16_t dc(float a);
static uint16_t fr(float a);

LPCSTR f_RegisterTk5fsWndClass(HINSTANCE hinst){
	WNDCLASS w;
			memset(&w,0,sizeof(WNDCLASS));
			w.lpfnWndProc = WndProcTk5fs;
			w.hInstance = hinst;
			w.style = CS_HREDRAW | CS_VREDRAW;
			w.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
			w.lpszClassName = g_szClassNameTk5fs;
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

		hinstTk5	= hinst;
		if(!RegisterClassA(&w))
		{
	//		int Error = GetLastError();
			MessageBox(NULL, "Window Registration Failed!", "Error!",
				MB_ICONEXCLAMATION | MB_OK);
			return 0;
		}
	return w.lpszClassName;
}
HWND   f_CreateTk5fsWnd(HWND hh1){

	//	WNDCLASSEX wc;
		MSG Msg;
		HWND fsHwnd = CreateWindowEx(
			0,
			g_szClassNameTk5fs,
			"TK5. Налаштування виробника",
			(WS_OVERLAPPEDWINDOW&(~(WS_MAXIMIZEBOX|WS_MINIMIZEBOX)))|WS_BORDER|WS_VISIBLE
			,
			CW_USEDEFAULT, CW_USEDEFAULT, 1024, 800,
			NULL, NULL, hinstTk5, NULL
			);
//		volatile int yjh=GetLastError();
		HMENU hMenubar = CreateMenu();
		HMENU hMenu = CreateMenu();

//		AppendMenu(hMenu, MF_STRING, IDM_Tk5FACILITY_SETTING_from_FS, "&Налаштування виробника");
		AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

		AppendMenu(hMenubar, MF_POPUP, (UINT_PTR) hMenu, "&Файл");
		AppendMenu(hMenu, MF_STRING, IDB_tk5WRITE_FACILITYSETTING_TO_PC_DESK_FILE, "&Зберегти налаштування виробника");
		AppendMenu(hMenu, MF_STRING, IDB_tk5READ_FACILITYSETTING_FROM_PC_DESK_FILE, "&Зчитати налаштування виробника");


		AppendMenu(hMenu, MF_STRING, IDM_QUIT_Tk5FS, "&Вихід");
		SetMenu(fsHwnd, hMenubar);
		ShowWindow(fsHwnd, SW_NORMAL);
		UpdateWindow(fsHwnd);

		while(GetMessage(&Msg, NULL, 0, 0) > 0)
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
		tk5fsProcess=ProcessIddle;
		return fsHwnd;
}



LRESULT CALLBACK WndProcTk5fs(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
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


			char str[128]={0};
			hwnd_amperageAclbr = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", str,
						WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
						, 460, y_amperageAclbr, 300, 25,
						hwnd, (HMENU)IDE_AMPERAGEACLBR, GetModuleHandle(NULL), NULL);
			SendMessage(hwnd_amperageAclbr, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));


			hwnd_clbr_iA = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", str,
						WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
						, 460, y_clbr_iA, 300, 25,
						hwnd, (HMENU)IDE_AMPERAGEACLBR, GetModuleHandle(NULL), NULL);
			SendMessage(hwnd_clbr_iA, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

			hwnd_amperageBclbr = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", str,
						WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
						, 460, y_amperageBclbr, 300, 25,
						hwnd, (HMENU)IDE_AMPERAGEBCLBR, GetModuleHandle(NULL), NULL);
			SendMessage(hwnd_amperageBclbr, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

			hwnd_clbr_iB = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", str,
						WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
						, 460, y_clbr_iB, 300, 25,
						hwnd, (HMENU)IDE_AMPERAGEBCLBR, GetModuleHandle(NULL), NULL);
			SendMessage(hwnd_clbr_iB, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

			hwnd_amperageCclbr = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", str,
						WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
						, 460, y_amperageCclbr, 300, 25,
						hwnd, (HMENU)IDE_AMPERAGECCLBR, GetModuleHandle(NULL), NULL);
			SendMessage(hwnd_amperageCclbr, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

			hwnd_clbr_iC = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", str,
						WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
						, 460, y_clbr_iC, 300, 25,
						hwnd, (HMENU)IDE_AMPERAGEACLBR, GetModuleHandle(NULL), NULL);
			SendMessage(hwnd_clbr_iC, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

			hwnd_voltageCclbr = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", str,
						WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
						, 460, y_voltageCclbr, 300, 25,
						hwnd, (HMENU)IDE_AMPERAGEACLBR, GetModuleHandle(NULL), NULL);
			SendMessage(hwnd_voltageCclbr, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

			hwnd_clbr_uC = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", str,
						WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
						, 460, y_clbr_uC, 300, 25,
						hwnd, (HMENU)IDE_VOLTAGECCLBR, GetModuleHandle(NULL), NULL);
			SendMessage(hwnd_clbr_uC, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

			hwnd_tk5fsDeviceMode = CreateWindow("COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
			    		  460, y_DeviceMode, 300, 800,
			  		      hwnd, (HMENU)IDCB_DEVICEMODE,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),NULL);
			SendMessage(hwnd_tk5fsDeviceMode, CB_ADDSTRING, 0, (LPARAM)" 1    ");
			SendMessage(hwnd_tk5fsDeviceMode, CB_ADDSTRING, 0, (LPARAM)" 2 	 ");
			SendMessage(hwnd_tk5fsDeviceMode, CB_ADDSTRING, 0, (LPARAM)" 3    ");
			SendMessage(hwnd_tk5fsDeviceMode, CB_ADDSTRING, 0, (LPARAM)" 4 	 ");
			SendMessage(hwnd_tk5fsDeviceMode, CB_ADDSTRING, 0, (LPARAM)" 5    ");
			tk5fs.DeviceMode=1;
			SendMessage(hwnd_tk5fsDeviceMode, CB_SETCURSEL, tk5fs.DeviceMode-1, 0);

			hwnd_tk5fsFaznost = CreateWindow("COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
			    		  460, y_Faznost, 300, 800,
			  		      hwnd, (HMENU)IDCD_FAZNOST,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),NULL);
			SendMessage(hwnd_tk5fsFaznost, CB_ADDSTRING, 0, (LPARAM)" Однофазний   ");
			SendMessage(hwnd_tk5fsFaznost, CB_ADDSTRING, 0, (LPARAM)" Трифазний    ");
			tk5fs.Faznost=3;
			if      (tk5fs.Faznost==1){
				SendMessage(hwnd_tk5fsFaznost, CB_SETCURSEL, 0, 0);
			}
			else if (tk5fs.Faznost==3){
				SendMessage(hwnd_tk5fsFaznost, CB_SETCURSEL, 1, 0);
			}
			else{}


  			 /* Кнопки на правій панелі */
  			 CreateWindow("BUTTON", "В ПАМ'ЯТЬ ТК5", WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
  					        		  839, 440, 130, 20, hwnd,
  									 (HMENU)IDB_tk5WRITE_FACILITYSETTING_TO_MEMORY,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
  		     CreateWindow("BUTTON", "НА ФЛЕШ ТК5", WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
  					        		  839, 465, 130, 20, hwnd,
  									 (HMENU)IDB_tk5WRITE_FACILITYSETTING_TO_FLASH,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);

  		     CreateWindow("BUTTON", "ВИЙТИ", WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
  					        		  839, 490, 130, 20, hwnd,
  									 (HMENU)IDB_BTN_tk5FS_EXIT,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);

  		     CreateWindow("BUTTON", "ЧИТАТИ з ТК5",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
										 40, 90,  110, 40, hwnd,
										 (HMENU)IDB_tk5READ_FACILITYSETTING_FROM_MEMORY,
										 (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);


//					hEditCheckPassword = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", strValue,
//										WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
//									, 460, y_us_passport, 300, 25,
//									   hwnd, (HMENU)EB_tk2US_CHECK_PASSWORD, GetModuleHandle(NULL), NULL);
//									SendMessage(hEditPassword, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
//									SetWindowText(hEditMBAddr,"");
//
//			  hEditMBAddr = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", strValue,
//						WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
//					, 460, y_mb_addr, 300, 25,
//					   hwnd, (HMENU)ID_EDIT_MODBUSS_ADDR, GetModuleHandle(NULL), NULL);
//
//
//					SendMessage(hEditMBAddr, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
//					char addr_s[20]={0};
//					snprintf (addr_s,20,"%X",tk2_Addr);
//					SetWindowText(hEditMBAddr,(LPCTSTR)addr_s);
//
//			  hEditPassword = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", strValue,
//								WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
//							, 460, y_us_passport, 300, 25,
//							   hwnd, (HMENU)ID_EDIT_NEW_PASSWORD, GetModuleHandle(NULL), NULL);
//							SendMessage(hEditPassword, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
//
//			  /* Запускаємо таймер, щоб зчитати типорозмір */
//			  pUS1000Timer=SetTimer(
//					  hwnd,                // handle to main window
//					  IDT_TIMER_USTK2_1000MS,// timer identifier
//					  1000,                // msecond interval
//					  (TIMERPROC) NULL     // no timer callback
//					);
//
//			/* Безумовний прогрес-бар для черги повідомлень */
//			  hWndProgressBar_usQueue = CreateWindowEx(
//					0,
//					PROGRESS_CLASS,
//					(LPSTR)NULL,
//					WS_VISIBLE | WS_CHILD /*| PBS_VERTICAL*/,
//					839, 680, 150, 20,
//					//910, 620, 150, 640
//					hwnd,
//					(HMENU)IDPB_usQUEUE_MSG ,
//					(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
//					NULL
//					);
//			  if (!hWndProgressBar_usQueue){
//					MessageBox(NULL, "Progress Bar Faileq.", "Error", MB_OK | MB_ICONERROR);
//			  }
//			  /* Шкала 120% - задаємо в процентах */
//			  SendMessage(hWndProgressBar_usQueue, PBM_SETRANGE, 0, MAKELPARAM(
//					 0, //minimum range value
//					 10000  //maximum range value
//					 )
//			  );
//			  SendMessage(hWndProgressBar_usQueue, PBM_SETPOS,
//					  0, //Signed integer that becomes the new position.
//					  0  //Must be zero
//			  );
//			  /* таймер для роботи з процес-баром черги повідомлень */
//			  p10usTimer=SetTimer(
//					  hwnd,                // handle to main window
//					  IDT_usTIMER_TK2_10MS,  // timer identifier
//					  TIMER_MAIN_MS,                  // msecond interval
//					  (TIMERPROC) NULL     // no timer callback
//					);
//			 /* Таймер для постійного періодичного опитування
//			 * статусу доступу до налаштувань*/
//			 pUS1000Timer=SetTimer(
//					  hwnd,                // handle to main window
//					  IDT_TIMER_USTK2_1000MS,// timer identifier
//					  1000,                // msecond interval
//					  (TIMERPROC) NULL     // no timer callback
//					);
//
//					/* Збираємо повідомлення для запису регістра */
//					modbus_master_tx_msg_t mbTxMsg={0};
//					f_Set112TxReadReg(RG_R_TK2_TYPOROZMIR,&mbTxMsg);
//					/* Ставимо повідомлення в чергу на обробку   */
//					f_set_tkqueue(
//							&tk2Queue,
//							DISABLE,
//							hwnd,
//							RG_R_TK2_TYPOROZMIR,
//							&mbTxMsg,
//							600
//					);
//					/* Скидаємо всі параметри для обновлення параметрів в 0*/
//					f_tk2usUpdateInit();
//			        /* Формуємо запит в ТК2 на зчитування налаштувань користувача */
//				    f_read_user_settings(hwnd);
//				    tk2usProcess=ProcessIddle;

  		     	 f_read_facilitysetting_from_tk5memory(hwnd);
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
//		    	  RECT rectcp = {rect.left+210, rect.top+y_check_password,rect.left+1000,rect.top+25+y_check_password};
//		    	  DrawText(hDC, "Введіть пароль ", 16, &rectcp, DT_SINGLELINE|DT_LEFT|DT_TOP);
//
//		    	  RECT rect0 = {rect.left+210, rect.top+y_max_amperage,rect.left+1000,rect.top+y_max_amperage+25};
//		    	  DrawText(hDC, "Максимальний струм, А ", 21, &rect0, DT_SINGLELINE|DT_LEFT|DT_TOP);
//
//		    	  RECT rect1 = {rect.left+210, rect.top+y_min_amperage,rect.left+1000,rect.top+25+y_min_amperage};
//		    	  DrawText(hDC, "Мінімальний струм, A  ", 21, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
//
//		    	  RECT rect2 = {rect.left+210, rect.top+y_pumping_mode,rect.left+1000,rect.top+25+y_pumping_mode};
//		    	  DrawText(hDC, "Вид роботи ", 12, &rect2, DT_SINGLELINE|DT_LEFT|DT_TOP);
//uint16_t y_amperageAclbr =  80; 	// 0х2000

//			   	  RECT rectcp1 = {rect.left+210, rect.top+y_check_password,rect.left+1000,rect.top+25+y_check_password};
//				  DrawText(hDC, "Введіть пароль ", 16, &rectcp1, DT_SINGLELINE|DT_LEFT|DT_TOP);

////		      uint16_t y_clbr_iA       = 105;		// 0х2002
//		    	  uint16_t y_amperageBclbr = 130; 	// 0х2004
//		    	  uint16_t y_clbr_iB		 = 155;		// 0х2006
//		    	  uint16_t y_amperageCclbr = 180; 	// 0х2008
//		    	  uint16_t y_clbr_iC		 = 205;		// 0х200A
//		    	  uint16_t y_voltageCclbr	 = 230;	    // 0x200C
//		    	  uint16_t y_clbr_uC  	 = 255;		// 0х200E
//		    	  uint16_t y_DeviceMode	 = 280;     // 0x2010
//		    	  uint16_t y_Faznost		 = 305;		// 0x2011

		    	  RECT rect5 = {rect.left+210, rect.top+y_amperageAclbr,rect.left+1000,rect.top+25+y_amperageAclbr};
		    	  DrawText(hDC, "Калібрування. Струм iА, A ", 27, &rect5, DT_SINGLELINE|DT_LEFT|DT_TOP);

		    	  RECT rect6 = {rect.left+210, rect.top+y_clbr_iA,rect.left+1000,rect.top+25+y_clbr_iA};
		    	  DrawText(hDC, "Коефіцієнт iA             ", 27, &rect6, DT_SINGLELINE|DT_LEFT|DT_TOP);

		    	  RECT rect7 = {rect.left+210, rect.top+y_amperageBclbr,rect.left+1000,rect.top+25+y_amperageBclbr};
		    	  DrawText(hDC, "Калібрування. Струм iB, A ", 27, &rect7, DT_SINGLELINE|DT_LEFT|DT_TOP);

		    	  RECT rect8 = {rect.left+210, rect.top+y_clbr_iB,rect.left+1000,rect.top+25+y_clbr_iB};
		    	  DrawText(hDC, "Коефіцієнт iB             ", 27, &rect8, DT_SINGLELINE|DT_LEFT|DT_TOP);

		    	  RECT rect9 = {rect.left+210, rect.top+y_amperageCclbr,rect.left+1000,rect.top+25+y_amperageCclbr};
		    	  DrawText(hDC, "Калібрування. Струм iC, A ", 27, &rect9, DT_SINGLELINE|DT_LEFT|DT_TOP);

		    	  RECT rectq = {rect.left+210, rect.top+y_clbr_iC,rect.left+1000,rect.top+25+y_clbr_iC};
		    	  DrawText(hDC, "Коефіцієнт iC             ", 27, &rectq, DT_SINGLELINE|DT_LEFT|DT_TOP);

		    	  RECT rectw = {rect.left+210, rect.top+y_voltageCclbr,rect.left+1000,rect.top+25+y_voltageCclbr};
		    	  DrawText(hDC, "Калібрування.Напруга uC,B ", 27, &rectw, DT_SINGLELINE|DT_LEFT|DT_TOP);

		    	  RECT recte = {rect.left+210, rect.top+y_clbr_uC,rect.left+1000,rect.top+25+y_clbr_uC};
		    	  DrawText(hDC, "Коефіцієнт uC             ", 27, &recte, DT_SINGLELINE|DT_LEFT|DT_TOP);

		    	  RECT rectr = {rect.left+210, rect.top+y_DeviceMode,rect.left+1000,rect.top+25+y_DeviceMode};
		    	  DrawText(hDC, "Типорозмір датчика струму ", 27, &rectr, DT_SINGLELINE|DT_LEFT|DT_TOP);

		    	  RECT rectt = {rect.left+210, rect.top+y_Faznost,rect.left+1000,rect.top+25+y_Faznost};
		    	  DrawText(hDC, "Струм трифазний/однофазний", 27, &rectt, DT_SINGLELINE|DT_LEFT|DT_TOP);


//		    	  RECT rect7 = {rect.left+210, rect.top+y_us_passport,rect.left+1000,rect.top+25+y_us_passport};
//		    	               //123456789012345678901234567890
//		    	  DrawText(hDC, "Замінити пароль на:  ",22, &rect7, DT_SINGLELINE|DT_LEFT|DT_TOP);
//
//		    	  RECT rectas = {rect.left+460, rect.top+y_check_password,rect.left+819,rect.top+y_check_password+25};
//	  	    	  if(us_access_status==DISABLE){
//	  	    		               //12345678901234567890123456789012345678901234567
//	  	    		  DrawText(hDC, "Корегувати заборонено",22, &rectas, DT_SINGLELINE|DT_LEFT|DT_TOP);
//	  	    	  }
//	  	    	  else if (us_access_status==ENABLE){
//	  	    		  DrawText(hDC, "Корегувати дозволено ",22, &rectas, DT_SINGLELINE|DT_LEFT|DT_TOP);
//	  	    	  }
//		    	  /* Розмальовування ПроцесБарів справа знизу - Нагрівання електродвигуна */
//		    	  //910, 680, 150, 20,
//	  	    	  int x_right_panel = 839;
//		    	  RECT recttr = {rect.left+x_right_panel, rect.top+535, rect.left+x_right_panel+150, rect.top+660+20};
//		    	  DrawText(hDC, "МОДБАС:", 7, &recttr, DT_SINGLELINE|DT_LEFT|DT_TOP);
//
//		    	  RECT rectt9 = {rect.left+x_right_panel, rect.top+560, rect.left+x_right_panel+150, rect.top+660+20};
//		    	  DrawText(hDC, "Повідомлень:", 12, &rectt9, DT_SINGLELINE|DT_LEFT|DT_TOP);
//
//		    	  RECT recttg = {rect.left+x_right_panel, rect.top+610, rect.left+x_right_panel+150, rect.top+660+20};
//		    	  DrawText(hDC, "Успіх:", 7, &recttg, DT_SINGLELINE|DT_LEFT|DT_TOP);
//
//
//		    	  RECT recttj = {rect.left+x_right_panel, rect.top+635, rect.left+x_right_panel+150, rect.top+660+20};
//		    	  DrawText(hDC, "Зайнятість:", 12, &recttj, DT_SINGLELINE|DT_LEFT|DT_TOP);
//
//		    	  RECT rectty = {rect.left+x_right_panel, rect.top+660, rect.left+x_right_panel+150, rect.top+660+20};
//		    	  DrawText(hDC, "Черга:", 7, &rectty, DT_SINGLELINE|DT_LEFT|DT_TOP);
//		    	  EndPaint(hwnd, &ps);
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
							case  IDCB_DEVICEMODE:
								tk5fs.DeviceMode=ItemIndex+1;
							break;
							case  IDCD_FAZNOST:
								if      (ItemIndex==0){tk5fs.Faznost=1;}
								else if (ItemIndex==1){tk5fs.Faznost=3;}
								else{}

							break;
							default:{}
						} // switch(idCtl)
					}//case CBN_SELCHANGE:
					break;
					default:{}
				}


			/*WM_COMMAND:*/
			switch(wParam) {
				case IDM_QUIT_Tk5FS:
				case IDB_BTN_tk5FS_EXIT:{
//					/* Перед завершенням роботи з налаштуванням скидаємо пароль */
//					/* Збираємо повідомлення для запису регістра */
//					modbus_master_tx_msg_t mbTxMsg={0};
//					f_Set112TxWriteReg(RG_W_TK2_PSW_CHECKING,0,&mbTxMsg);
//					/* Ставимо повідомлення про стинання ознаки ввода пароля
//					 * в чергу на обробку   */
//					f_set_tkqueue(
//						&tk2Queue,
//						ENABLE,
//						hwnd,
//						RG_W_TK2_PSW_CHECKING,
//						&mbTxMsg,
//						1000
//					);
//					 snprintf(tk2LogStr,511,"ко: Завершено налаштування користувача");
//					 if(IsWindow(hWndTk2fsCh)){DestroyWindow(hWndTk2fsCh);}
					 DestroyWindow(hwnd);
				}
				break;

			    /* Зчитуємо налаштування користувача всі */
				case IDB_BTN_READ_TK5_FS:{
//					if(tk2usProcess==ProcessBusy){
//					  if(tk2usBtn_access==ENABLE){
//						 tk2usBtn_access=DISABLE;
//						 f_read_user_settings(hwnd);
//						 snprintf(tk2LogStr,511,"КО: Зчитування налаштувань користувача");
//					  }else{snprintf (wmc,255," Не натискайте так часто на кнопки! ");}
//					}else{snprintf (wmc,255," Налаштування користувача ще не завантажилися.\nПочекайте ");}
				}
				break;
				/* Записуємо максимальний струм */


				case ID_WR_Tk5FS_MODBUS_ADDR:{
//					if(tk2usProcess==ProcessBusy){
//					  if(tk2usBtn_access==ENABLE){
//						 tk2usBtn_access=DISABLE;
//						 /* Зчитуємо адресу Модбас */
//						 float2_t fl={0};
//						 int errWM_C=f_checkEdit(ID_EDIT_MODBUSS_ADDR, hwnd, &fl);
//						 if(errWM_C==0){
//							 uint16_t newMB_Addr=fl.d;
//							 if((newMB_Addr==2)||(newMB_Addr==3)){
//								 /* Збираємо повідомлення для запису регістра */
//								 modbus_master_tx_msg_t mbTxMsg={0};
//								 f_Set112TxWriteReg(RG_W_TK2_NEW_MODBUS_ADDRESS,newMB_Addr,&mbTxMsg);
//								 /* Ставимо повідомлення в чергу на обробку   */
//								 f_set_tkqueue(
//								    &tk2Queue,
//									ENABLE,
//									hwnd,
//									RG_W_TK2_NEW_MODBUS_ADDRESS,
//									&mbTxMsg,
//									1000
//									);
//								 snprintf(tk2LogStr,511,"ко: Змінено АДРЕСу МОДБАС на %d",newMB_Addr);
//							 }
//							 else{
//								 const int result = MessageBox(NULL,"Підтримуються лише адреси 0х02 і 0х03","Помилка",MB_OK);
//								 switch (result){case IDOK:{}break;default:{}}
//							 }
//						 }
//						 /* Якщо там не число */
//						 else{
//							 f_valueEditMessageBox(errWM_C);
//						 }
//						 f_read_user_settings(hwnd);
//					  }else{snprintf (wmc,255," Не натискайте так часто на кнопки! ");}
//				   }else{snprintf (wmc,255," Налаштування користувача ще не завантажилися.\nПочекайте ");}
				}
				break;

				case ID_WR_TK5FS_PASSWORD:{
//					if(tk2usProcess==ProcessBusy){
//					  if(tk2usBtn_access==ENABLE){
//						 tk2usBtn_access=DISABLE;
//						 //RG_W_TK2_NEW_PASSWORD     ID_EDIT_NEW_PASSWORD
//						 /* Зчитуємо з ЕдітБокса нове значення пароля */
//						 float2_t fl={0};
//						 int errWM_C=f_checkEdit(ID_EDIT_NEW_PASSWORD, hwnd, &fl);
//						 if(errWM_C==0){
//							 uint16_t newPSW=fl.d;
//							 /* Збираємо повідомлення для запису регістра */
//							 modbus_master_tx_msg_t mbTxMsg={0};
//							 f_Set112TxWriteReg(RG_W_TK2_NEW_PASSWORD,newPSW,&mbTxMsg);
//							 /* Ставимо повідомлення в чергу на обробку   */
//						     f_set_tkqueue(
//								    &tk2Queue,
//									ENABLE,
//									hwnd,
//									RG_W_TK2_NEW_PASSWORD,
//									&mbTxMsg,
//									1000
//									);
//							 f_read_user_settings(hwnd);
//							 snprintf(tk2LogStr,511,"ко: Змінено ПАРОЛЬ на %d", (0x1234 | newPSW));
//
//						 }
//						 /* Якщо там не число */
//						 else{
//							 f_valueEditMessageBox(errWM_C);
//						 }
//					  }else{snprintf (wmc,255," Не натискайте так часто на кнопки! ");}
//					}else{snprintf (wmc,255," Налаштування користувача ще не завантажилися.\nПочекайте ");}
				}
				break;
				case IDB_tk5WRITE_FACILITYSETTING_TO_MEMORY:{
					f_facilitysettings_to_tk5Memory(hwnd);
					f_read_facilitysetting_from_tk5memory(hwnd);
				}
				break;

				case IDB_tk5WRITE_FACILITYSETTING_TO_FLASH:{
					f_facilitysettings_to_tk5Memory(hwnd);
					modbus_master_tx_msg_t mbTxMsg={0};
					f_Set5TxWriteReg(0x0011,0x01,&mbTxMsg);
					/* Ставимо повідомлення в чергу на обробку   */
					f_set_tkqueue(
							&tk5Queue,
							DISABLE,
							hwnd,
							IDB_tk5WRITE_FACILITYSETTING_TO_FLASH,
							&mbTxMsg,
							1000
						);
					f_read_facilitysetting_from_tk5memory(hwnd);
				}
				break;
				case  IDB_tk5READ_FACILITYSETTING_FROM_MEMORY:{
					f_read_facilitysetting_from_tk5memory(hwnd);
				}
				break;
				case IDB_tk5WRITE_FACILITYSETTING_TO_PC_DESK_FILE:{
					f_tk5SetFilesWrite();

				}
				break;
				case IDB_tk5READ_FACILITYSETTING_FROM_PC_DESK_FILE:{
					tk5fs_t tk5fsr={0};
					f_tk5SetFilesRead(&tk5fsr);
					tk5fs=tk5fsr;
					f_tk5fsUpdateData(hwnd, IDB_tk5READ_FACILITYSETTING_FROM_PC_DESK_FILE, 1);
				}
				break;
				case IDB_tk5WRITE_FACILITYSETTING_FILE_AS:{

					//BOOL b= GetSaveFileNameA(FileNameFsAs);

				}
				break;
				case IDB_tk5READ_FACILITYSETTING_FILE_AS:{

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
				case IDT_TIMER_TK5FS_10MS:{
//					  /* Якщо у вікні натиснута кнопка - вмикаємо лічильник затримки */
//					  if((tk2usProcess==ProcessBusy)&&(tk2usBtn_access==DISABLE)){
//						  tk2usBtnSleepCntr+= TIMER_MAIN_MS;
//						  if(tk2usBtnSleepCntr>=TK2US_BTN_BLOKING_MS){
//							  tk2usBtn_access=ENABLE;
//							  tk2usBtnSleepCntr=0;
//						  }
//					  }
				}
				break;
				 /* Таймер для постійного періодичного опитування
				 * статусу доступу до налаштувань*/
				case  IDT_TIMER_TK5FS_1000MS:{
//					/* Перевірка статусу прав доступу  */
//					modbus_master_tx_msg_t mbTxMsg={0};
//					f_Set112TxReadReg(RG_R_TK2_ACCESS_STATUS,&mbTxMsg);
//					f_set_tkqueue(
//							&tk2Queue,
//							DISABLE,
//							hwnd,
//							RG_R_TK2_ACCESS_STATUS,
//							&mbTxMsg,
//							1000
//							);
				}
				break;
				default:{}
				}
			}
			break;
			case WM_CLOSE:{
//				/* Перед завершенням роботи з налаштуванням скидаємо пароль */
//				/* Збираємо повідомлення для запису регістра */
//				modbus_master_tx_msg_t mbTxMsg={0};
//				f_Set112TxWriteReg(RG_W_TK2_PSW_CHECKING,0,&mbTxMsg);
//				/* Ставимо повідомлення про стинання ознаки ввода пароля
//				 * в чергу на обробку   */
//				f_set_tkqueue(
//					&tk2Queue,
//					ENABLE,
//					hwnd,
//					RG_W_TK2_PSW_CHECKING,
//					&mbTxMsg,
//					1000
//				);
//				 snprintf(tk2LogStr,511,"ко: Завершено налаштування користувача");
//				 if(IsWindow(hWndTk2fsCh)){DestroyWindow(hWndTk2fsCh);}
				 DestroyWindow(hwnd);
			}
			break;
			case WM_DESTROY:
				PostQuitMessage(0);
			break;
			case VK_PSW:{
//	//			float2_t float2psw={0};
//	//			int err = f_strint_to_value(outPassword, strlen(outPassword), &float2psw);
//	//			if (err !=0){f_valueEditMessageBox(err);}
//	//			else{
//	//				memset(outPassword,0,sizeof(outPassword));
//				uint16_t psw=(uint16_t)wParam;
//				/* Збираємо повідомлення для запису регістра */
//				modbus_master_tx_msg_t mbTxMsg={0};
//				f_Set112TxWriteReg(RG_W_TK2_PSW_CHECKING,psw,&mbTxMsg);
//					/* Ставимо повідомлення в чергу на обробку   */
//				f_set_tkqueue(
//					&tk2Queue,
//					DISABLE,
//					hwnd,
//					RG_W_TK2_PSW_CHECKING,
//					&mbTxMsg,
//					1000
//					);
//	//				memset(&float2psw,0,sizeof(float2_t));
//				f_read_user_settings(hwnd);
			}
			break;
			case VK_UPDATE_WINDOW:{
				int tk5fs_responce_status=f_tk5QPWD_RgAnswer(
						RxMasterModbusStatus,
						&mOut_tx_msg,
						&mIn_rx_msg,
						wParam
				);
				f_tk5fsUpdateData(hwnd,wParam,tk5fs_responce_status);
//				/***********************************************************************************************************
//				* РОБОТА З ЧЕРГОЮ ПОВІДОМЛЕНЬ МОДБАС
//				***********************************************************************************************************/
//				f_tk5usUpdateWindow(hwnd);
//				f_tk2UpdateStat(
//							hwnd,
//							hWndProgressBar_usQueue,
//							p10usTimer,
//							IDT_usTIMER_TK2_10MS
//							);
			}
			break;
			default:{
			return DefWindowProc(hwnd, msg, wParam, lParam);
			}
		}
		return 0;
}

static int f_tk5fsUpdateData(HWND hwnd, WPARAM wParam, int responce_status){

    if(responce_status == -1){

    }
    else if(responce_status == 1){
    	switch(wParam){
    	case IDB_tk5READ_FACILITYSETTING_FROM_MEMORY:
    	case IDB_tk5READ_FACILITYSETTING_FROM_PC_DESK_FILE:{
    		EnumChildWindows(hwnd, ChildCallbacktk5fs, 0);

    		if      (tk5fs.Faznost==1){
    		    SendMessage(hwnd_tk5fsDeviceMode, CB_SETCURSEL, 0, 0);
    		}
    		else if (tk5fs.Faznost==3){
    		    SendMessage(hwnd_tk5fsDeviceMode, CB_SETCURSEL, 1, 0);
    		}
    		else{}
    		InvalidateRect(hwnd, NULL, TRUE);
    		UpdateWindow(hwnd);
    	}
    	break;
		case IDB_tk5WRITE_FACILITYSETTING_TO_MEMORY:{
		}
		break;
		case IDB_tk5WRITE_FACILITYSETTING_TO_FLASH:{
		}
		break;
		default:{}
		}
	}
    return 0;

}

static BOOL CALLBACK ChildCallbacktk5fs(
		HWND   hwnd,
		LPARAM lParam
){
  char as[45]={0};
	if(hwnd==hwnd_amperageAclbr){	// 0х2000
		snprintf(as,sizeof(as),"%d.%04d",tk5fs.amperageAclbr.d,tk5fs.amperageAclbr.f);
		SetWindowTextA(hwnd_amperageAclbr,(LPCTSTR)as);
	}
	if(hwnd==hwnd_clbr_iA){
		snprintf(as,sizeof(as),"%d.%04d",tk5fs.clbr_iA.d,tk5fs.clbr_iA.f);
		SetWindowTextA(hwnd_clbr_iA,(LPCTSTR)as);
	}
    if(hwnd==hwnd_amperageBclbr){	// 0х2004
    	snprintf(as,sizeof(as),"%d.%04d",tk5fs.amperageBclbr.d,tk5fs.amperageBclbr.f);
    	SetWindowTextA(hwnd_amperageBclbr,(LPCTSTR)as);
    }

    if(hwnd==hwnd_clbr_iB){			// 0х2006
    	snprintf(as,sizeof(as),"%d.%04d",tk5fs.clbr_iB.d,tk5fs.clbr_iB.f);
    	SetWindowTextA(hwnd_clbr_iB,(LPCTSTR)as);
    }
    if(hwnd==hwnd_amperageCclbr){ 	// 0х2008
    	snprintf(as,sizeof(as),"%d.%04d",tk5fs.amperageCclbr.d,tk5fs.amperageCclbr.f);
    	SetWindowTextA(hwnd_amperageCclbr,(LPCTSTR)as);
    }
    if(hwnd==hwnd_clbr_iC){			// 0х200A
    	snprintf(as,sizeof(as),"%d.%04d",tk5fs.clbr_iC.d,tk5fs.clbr_iC.f);
    	SetWindowTextA(hwnd_clbr_iC,(LPCTSTR)as);
    }
    if(hwnd==hwnd_voltageCclbr){		// 0x200C
    	snprintf(as,sizeof(as),"%d.%04d",tk5fs.amperageCclbr.d,tk5fs.amperageCclbr.f);
    	SetWindowTextA(hwnd_voltageCclbr,(LPCTSTR)as);
    }
    if(hwnd==hwnd_clbr_uC){			// 0х200E
    	snprintf(as,sizeof(as),"% d.%04d",tk5fs.clbr_uC.d,tk5fs.clbr_uC.f);
    	SetWindowTextA(hwnd_clbr_uC,(LPCTSTR)as);
    }
    if(hwnd==hwnd_tk5fsDeviceMode){      	// 0x2010
    	SendMessageA(hwnd, CB_SETCURSEL, tk5fs.DeviceMode-1, 0);
    }
    if(hwnd==hwnd_tk5fsFaznost){		    // 0x2011
    	if      (tk5fs.Faznost==1){
    		SendMessageA(hwnd, CB_SETCURSEL, 0, 0);
    	}
    	else if (tk5fs.Faznost==3){
    		SendMessageA(hwnd, CB_SETCURSEL, 1, 0);
    	}
    	else{}
	 }
	 return true;
  }
static int f_facilitysettings_to_tk5Memory(HWND hwnd){
	/* Зчитуємо вміст Едіт-боксів,
	 * значення комбобоксів ВЖЕ!! зчитані в змінну при виході користувача
	 * з кожного з комбобоксів */
	int err = 0;
	while (err==0){
		float2_t fl={0};
	//По черзі зчитуємо ЕдітБокси. Якщо там помилка - повідомляємо користувача і         виходимо по помилці 0Х0209	0Х1004	90
//static HWND hwnd_tk5fsDeviceMode;      	// 0x2010
//static HWND hwnd_tk5fsFaznost;		    // 0x2011

		err=f_checkEditWCHAR(hwnd_amperageAclbr, hwnd, &fl);
//		err = f_checkEdit(IDE_AMPERAGEACLBR, hwnd, &fl);
		if(err != 0){f_valueEditMessageBox(err);break;};
		tk5fs.amperageAclbr=fl;

		err = f_checkEditWCHAR(hwnd_clbr_iA, hwnd, &fl);
		if(err != 0){f_valueEditMessageBox(err);
		break;
		};
		tk5fs.clbr_iA=fl;

		err=f_checkEditWCHAR(hwnd_amperageBclbr, hwnd, &fl);
//		if(err != 0){f_valueEditMessageBox(err);break;};
		tk5fs.amperageBclbr=fl;
		err = f_checkEditWCHAR(hwnd_clbr_iB, hwnd, &fl);
//		if(err != 0){f_valueEditMessageBox(err);break;};
		tk5fs.clbr_iB=fl;

		err=f_checkEditWCHAR(hwnd_amperageAclbr, hwnd, &fl);
//		if(err != 0){f_valueEditMessageBox(err);break;};
		tk5fs.amperageCclbr=fl;
		err = f_checkEditWCHAR(hwnd_clbr_iC, hwnd, &fl);
//		if(err != 0){f_valueEditMessageBox(err);break;};
		tk5fs.clbr_iC=fl;

		err=f_checkEditWCHAR(hwnd_voltageCclbr, hwnd, &fl);
//		if(err != 0){f_valueEditMessageBox(err);break;};
		tk5fs.voltageCclbr=fl;
		err = f_checkEditWCHAR(hwnd_clbr_uC, hwnd, &fl);
//		if(err != 0){f_valueEditMessageBox(err);break;};
		tk5fs.clbr_uC=fl;

/* Запит - який саме номер по порядку з меню вибраний (незалежно від комбобоксу */
		int ItemIndex = 0;
		ItemIndex =SendMessage(hwnd_tk5fsDeviceMode, (UINT) CB_GETCURSEL,  (WPARAM) 0, (LPARAM) 0);
		tk5fs.DeviceMode=ItemIndex+1;
		ItemIndex =SendMessage(hwnd_tk5fsFaznost, (UINT) CB_GETCURSEL,  (WPARAM) 0, (LPARAM) 0);
		if      (ItemIndex==0){tk5fs.Faznost=1;}
		else if (ItemIndex==1){tk5fs.Faznost=3;}
		else{}


	/* Збираємо повідомлення для запису регістр */
	modbus_master_tx_msg_t mbTxMsg={0};
	uint16_t StartingAddress     = 0x2020;
	uint16_t QuantityOfRegisters = 0x20;
	uint16_t ByteCntr            = 0x40;
	uint16_t value_i=0;
	mbTxMsg.msg[0]=0x01;/* modbuss address */
	mbTxMsg.msg[1]=0x10;/* modbuss cmnd */
	mbTxMsg.msg[2]=StartingAddress/0x100;
	mbTxMsg.msg[3]=StartingAddress%0x100;
	mbTxMsg.msg[4]=QuantityOfRegisters/0x100;
	mbTxMsg.msg[5]=QuantityOfRegisters%0x100;
	mbTxMsg.msg[6]=ByteCntr;/*  */

//  float amperageAclbr; 	// 0х2020
    f_03_float2_to_Tx(&tk5fs.amperageAclbr,   &mbTxMsg, 0x00);
//  float clbr_iA;			// 0х2022
    f_03_float2_to_Tx(&tk5fs.clbr_iA, 		&mbTxMsg, 0x04);
//  float amperageBclbr; 	// 0х2024
    f_03_float2_to_Tx(&tk5fs.amperageBclbr, 	&mbTxMsg, 0x08);
//  float clbr_iB;			// 0х2026
    f_03_float2_to_Tx(&tk5fs.clbr_iB, 		&mbTxMsg, 0x0C);//float amperageCclbr; 		// 0х2008
    //  float amperageCclbr; 	// 0х2004
	f_03_float2_to_Tx(&tk5fs.amperageCclbr, 	&mbTxMsg, 0x10);
	//float clbr_iC;			// 0х200A
	f_03_float2_to_Tx(&tk5fs.clbr_iC,       	&mbTxMsg, 0x14);

	//fSet.voltageCclbr			//0x200C
	f_03_float2_to_Tx(&tk5fs.voltageCclbr,  	&mbTxMsg, 0x18);
	//float clbr_uC;			// 0х200E
	f_03_float2_to_Tx(&tk5fs.clbr_uC,       	&mbTxMsg, 0x1C);

	//uint16_t DeviceMode;     	//0x1010 Типорозмір контролера 1...5
	value_i=(uint16_t)tk5fs.DeviceMode;
	mbTxMsg.msg[7+0x20]=value_i/0x100;
	mbTxMsg.msg[7+0x21]=value_i%0x100;

	//uint16_t Faznost;		 //0x1011
	value_i=(uint16_t)tk5fs.Faznost;
	mbTxMsg.msg[7+0x22]=value_i/0x100;
	mbTxMsg.msg[7+0x23]=value_i%0x100;

	mbTxMsg.length=7+ByteCntr;

	/* Ставимо повідомлення в чергу на обробку   */
	f_set_tkqueue(
			&tk5Queue,
			DISABLE,
			hwnd,
			0x10,
			&mbTxMsg,
			1000
	);
	err=0xffff;
  }
  return err;


}




static uint16_t dc(float a){
	return (uint16_t)a;
}
static uint16_t fr(float a){
	float b= a*10000.0f;
	return (uint32_t)b % 10000;
}

static size_t f_wchar_to_char(const wchar_t * src, char * dest, size_t dest_len){
  size_t i;
  wchar_t code;

  i = 0;

  while (src[i] != '\0' && i < (dest_len - 1)){
    code = src[i];
    if (code < 128){
      dest[i] = (char)code;
    }
    else{
      dest[i] = '?';
      if (code >= 0xD800 && code <= 0xD8FF)
        // lead surrogate, skip the next code unit, which is the trail
        i++;
    }
    i++;
  }

  dest[i] = '\0';

  return i - 1;

}

/* Ця функція створює і відкриває новий файл для запису алаштувань виробника .
 * Викликається з меню заводських налаштувань  tk5 */

FILE *file_to_safe_set;
char  setFileName[248]={0};
char  pInd[]="tk5";
static int f_tk5SetFilesWrite(void){
	  memset(&setFileName,0,sizeof(setFileName));
	  int nbr=0;
	  while(nbr==0){
		  int file_name_length=0;
		  file_name_length=f_tkSetFiles_Init(pInd, setFileName, tk5fs.DeviceMode);
		  if(file_name_length==0){nbr= -2;break;}
		  if(file_name_length>248){nbr= -3;break;}
		  file_to_safe_set= fopen(setFileName, "w");
		  if(file_to_safe_set==NULL){nbr= -1;break;}
		  nbr+=fprintf(file_to_safe_set, "\nКлбр.струм_iА  %05d %05d", tk5fs.amperageAclbr.d, tk5fs.amperageAclbr.f);
		  nbr+=fprintf(file_to_safe_set, "\nКлбр_iА        %05d %05d", tk5fs.clbr_iA.d,       tk5fs.clbr_iA.f);
		  nbr+=fprintf(file_to_safe_set, "\nКлбр.струм_iB  %05d %05d", tk5fs.amperageBclbr.d, tk5fs.amperageBclbr.f);
		  nbr+=fprintf(file_to_safe_set, "\nКлбр_iB        %05d %05d", tk5fs.clbr_iB.d,       tk5fs.clbr_iB.f);
		  nbr+=fprintf(file_to_safe_set, "\nКлбр.струм_iC  %05d %05d", tk5fs.amperageCclbr.d, tk5fs.amperageCclbr.f);
		  nbr+=fprintf(file_to_safe_set, "\nКлбр_iC        %05d %05d", tk5fs.clbr_iC.d,       tk5fs.clbr_iC.f);
		  nbr+=fprintf(file_to_safe_set, "\nКлбр.напр__uC  %05d %05d", tk5fs.voltageCclbr.d, tk5fs.voltageCclbr.f);
		  nbr+=fprintf(file_to_safe_set, "\nКлбр_uC        %05d %05d", tk5fs.clbr_uC.d,       tk5fs.clbr_uC.f);
		  nbr+=fprintf(file_to_safe_set, "\nТипорозмір     %05d",     tk5fs.DeviceMode);
		  nbr+=fprintf(file_to_safe_set, "\nФазність       %05d",     tk5fs.Faznost);
		  fclose(file_to_safe_set);
	  }
	  return nbr;
}


static int f_tk5SetFilesRead(tk5fs_t *ptk5fsr){
	memset(&setFileName, 0, sizeof(setFileName));
	int rs=f_tkSetFilesReadSrch(tk5fs.DeviceMode, &setFileName);
	if(rs!=0){
		f_tk5SetFilesReadFile(ptk5fsr, setFileName);
	}
	else rs=-1;
	return rs;
}
static int f_tk5SetFilesReadFile(tk5fs_t *ptk5fsr, char* sFileName){

	  //r"	Opens for reading. If the file does not exist or cannot be found, the fopen call fails.
	  int return_value=0;
	  while(return_value==0){
		  file_to_safe_set= fopen(sFileName, "r");
		  if(file_to_safe_set==NULL){return_value=-1;break;}
		  char str[64]={0};
		  float2_t fl={0};
		  return_value+=fscanf(file_to_safe_set, "%16s  %05d %05d", str, &(fl.d), &(fl.f));
		  fl.floatf=(float)fl.d+(float)fl.f/10000.0f;
	      ptk5fsr->amperageAclbr=fl;
	      return_value+=fscanf(file_to_safe_set, "%16s  %05d %05d", str, &(fl.d), &(fl.f));
	      fl.floatf=(float)fl.d+(float)fl.f/10000.0f;
	      ptk5fsr->clbr_iA=fl;

	      return_value+=fscanf(file_to_safe_set, "%16s  %05d %05d", str, &(fl.d), &(fl.f));
	      fl.floatf=(float)fl.d+(float)fl.f/10000.0f;
	      ptk5fsr->amperageBclbr=fl;
	      return_value+=fscanf(file_to_safe_set, "%16s  %05d %05d", str, &(fl.d), &(fl.f));
	      fl.floatf=(float)fl.d+(float)fl.f/10000.0f;
	      ptk5fsr->clbr_iB=fl;
	      fl.floatf=(float)fl.d+(float)fl.f/10000.0f;
	      return_value+=fscanf(file_to_safe_set, "%16s  %05d %05d", str, &(fl.d), &(fl.f));
	      fl.floatf=(float)fl.d+(float)fl.f/10000.0f;
	      ptk5fsr->amperageCclbr=fl;
	      return_value+=fscanf(file_to_safe_set, "%16s  %05d %05d", str, &(fl.d), &(fl.f));
	      fl.floatf=(float)fl.d+(float)fl.f/10000.0f;
	      ptk5fsr->clbr_iC=fl;
	      return_value+=fscanf(file_to_safe_set, "%16s  %05d %05d", str, &(fl.d), &(fl.f));
	      fl.floatf=(float)fl.d+(float)fl.f/10000.0f;
	      ptk5fsr->voltageCclbr=fl;
	      return_value+=fscanf(file_to_safe_set, "%16s  %05d %05d", str, &(fl.d), &(fl.f));
	      fl.floatf=(float)fl.d+(float)fl.f/10000.0f;
	      ptk5fsr->clbr_uC=fl;
	      return_value+=fscanf(file_to_safe_set, "%16s  %05d",      str, (int*)(&ptk5fsr->DeviceMode));
	      return_value+=fscanf(file_to_safe_set, "%16s  %05d",      str, (int*)(&ptk5fsr->Faznost));
	      if(return_value==0){return_value=-100;}
	      //https://learnc.info/c/text_files.html
	      fclose(file_to_safe_set);

	  }
	  return return_value;

}
//HRESULT BasicFileOpen()
//{
//    // CoCreate the File Open Dialog object.
//    IFileDialog *pfd = NULL;
//    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
//                      NULL,
//                      CLSCTX_INPROC_SERVER,
//                      IID_PPV_ARGS(&pfd));
//    if (SUCCEEDED(hr))
//    {
//        // Create an event handling object, and hook it up to the dialog.
//        IFileDialogEvents *pfde = NULL;
//        hr = CDialogEventHandler_CreateInstance(IID_PPV_ARGS(&pfde));
//        if (SUCCEEDED(hr))
//        {
//            // Hook up the event handler.
//            DWORD dwCookie;
//            hr = pfd->Advise(pfde, &dwCookie);
//            if (SUCCEEDED(hr))
//            {
//                // Set the options on the dialog.
//                DWORD dwFlags;
//
//                // Before setting, always get the options first in order
//                // not to override existing options.
//                hr = pfd->GetOptions(&dwFlags);
//                if (SUCCEEDED(hr))
//                {
//                    // In this case, get shell items only for file system items.
//                    hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
//                    if (SUCCEEDED(hr))
//                    {
//                        // Set the file types to display only.
//                        // Notice that this is a 1-based array.
//                        hr = pfd->SetFileTypes(ARRAYSIZE(c_rgSaveTypes), c_rgSaveTypes);
//                        if (SUCCEEDED(hr))
//                        {
//                            // Set the selected file type index to Word Docs for this example.
//                            hr = pfd->SetFileTypeIndex(INDEX_WORDDOC);
//                            if (SUCCEEDED(hr))
//                            {
//                                // Set the default extension to be ".doc" file.
//                                hr = pfd->SetDefaultExtension(L"doc;docx");
//                                if (SUCCEEDED(hr))
//                                {
//                                    // Show the dialog
//                                    hr = pfd->Show(NULL);
//                                    if (SUCCEEDED(hr))
//                                    {
//                                        // Obtain the result once the user clicks
//                                        // the 'Open' button.
//                                        // The result is an IShellItem object.
//                                        IShellItem *psiResult;
//                                        hr = pfd->GetResult(&psiResult);
//                                        if (SUCCEEDED(hr))
//                                        {
//                                            // We are just going to print out the
//                                            // name of the file for sample sake.
//                                            PWSTR pszFilePath = NULL;
//                                            hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH,
//                                                               &pszFilePath);
//                                            if (SUCCEEDED(hr))
//                                            {
//                                                TaskDialog(NULL,
//                                                           NULL,
//                                                           L"CommonFileDialogApp",
//                                                           pszFilePath,
//                                                           NULL,
//                                                           TDCBF_OK_BUTTON,
//                                                           TD_INFORMATION_ICON,
//                                                           NULL);
//                                                CoTaskMemFree(pszFilePath);
//                                            }
//                                            psiResult->Release();
//                                        }
//                                    }
//                                }
//                            }
//                        }
//                    }
//                }
//                // Unhook the event handler.
//                pfd->Unadvise(dwCookie);
//            }
//            pfde->Release();
//        }
//        pfd->Release();
//    }
//    return hr;
//}
