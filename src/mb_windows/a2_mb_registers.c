/*
 * a2_mb_registers.s
 *
 *  Created on: 6 груд. 2019 р.
 *      Author: KozakVF
 */
#include "a2_mb_registers.h"
#include "windows.h"
#include "a0_win_control.h"

#include <process.h>    /* _beginthread, _endthread */
extern HINSTANCE hinst;

const char ClassNameRg[] = "RgWndClass";

/* Змінні для передачі результатів паралельного процесу в материнське вікно */
HWND QPWRg_Wnd;
/* Варіанти роздруківки відповіді */
/* Статус завершення сеансу Модбас,
 * якщо MODBUS_SUCCESS - сеанс завершено, отримали відповідь,
 * якщо ні - отримали повідомлення про помилку у рядку BTH_User_Msg[256] */
extern modbus_status_t BTH_TxTx_status;
/* Повідомлення Модбас для передачі у процесі */
extern modbus_master_tx_msg_t BTH_TxMsg;
/* Граничний час очікування відповіді */
extern uint32_t BTH_Msg_TimeOut;
/* Відповідь Модбас процесу */
extern modbus_master_rx_msg_t BTH_RxMsg;
/* Повідомлення про помилку процесу */
extern char BTH_User_Msg[256];
/* Прапоp ходу процесу, відсутності процесу,
 * у парі для визначення подій переходу
 * від обробки до процесу і від процесу до обробки*/
ProcessState_t QPW_Rg_ProcessState=ProcessIddle;
ProcessState_t QPW_Rg_ProcessStateOld=ProcessIddle;
/* З цього вікна запускається декілька процесів, для ідентифікації відповіді */
static int RW_case=0;

/* Локальні змінні */
static modbus_master_tx_msg_t mbTxMsg={0};
static modbus_master_rx_msg_t mbRxMsg={0};
static char sNRegTW[6]={0};
static char sValueTW[6]={0};
static char sValueR[6]={0};

static char mb_Addr=0x0F;
static uint16_t RegNumberTW=0;
static uint16_t RegValueTW=0;
static uint16_t RegNumberTR=0;
static uint16_t RegValueR=0;

static char user_msg[256]={0};



static int RadioBN_R_DEC_State = BST_INDETERMINATE;
static int RadioBN_R_HEX_State = BST_INDETERMINATE;

static HWND hWnd_R_HEX;
static HWND hWnd_R_DEC;
static int x0=200;
static int y0=130;
static int dy=150;


LPCSTR f_RegisterRgWndClass(HINSTANCE hinst){
	WNDCLASS w;
		memset(&w,0,sizeof(WNDCLASS));
		w.lpfnWndProc = WndProcRg;
		w.hInstance = hinst;
		w.style = CS_HREDRAW | CS_VREDRAW;
		w.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
		w.lpszClassName = ClassNameRg;
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
int WINAPI f_CreateRgWnd(HWND hh){

	HWND childRg;
	MSG Msg;
	childRg = CreateWindowEx(
		0,
		ClassNameRg,
		"Передача та прийом повідомлень Модбас: зчитування та запис одного регістра ",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1100, 700,
		NULL, NULL, hinst, NULL
		);

	if(childRg == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	QPWRg_Wnd=childRg;

//	f_PPB_TxRxWndClass(childRg);

	ShowWindow(childRg, SW_NORMAL);
	UpdateWindow(childRg);

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}


LRESULT CALLBACK WndProcRg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{   static HWND hEditADDR;
	static HWND hEditRW;
	static HWND hEditVW;

	static HWND hWnd_W_DEC;

    static HWND hWnd_W_HEX;

	static HWND hWndBTN_WSend;

	static HWND hEditRR;

	static HWND hEditRDEC;
    static HWND hWndBTN_RSend;



	/* Перемальовуємо вікно, коли завершився паралельний процес і
	 * самознищилось вікно PPB ProcessBar                                 */
	if((QPW_Rg_ProcessState==ProcessIddle)&&(QPW_Rg_ProcessStateOld==ProcessIddle)){
		  UpdateWindow(hwnd);
		  QPW_Rg_ProcessStateOld=ProcessBusy;
	}

	switch(msg)
	{
		case WM_CREATE:
		{
		  HFONT hfDefault;
		  /* Доска писати регістр */
          CreateWindowW(L"Button", L"",
                WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
               x0, y0-50,  700, 438, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

          /* EditBox для Модбас-адреси */
          char addr_s[3]={0};
		  hEditADDR = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", &addr_s,
				WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
				, x0+100, y0, 50, 25,
				hwnd, (HMENU)ID_EDIT_ADDRESS , GetModuleHandle(NULL), NULL);
		  SendMessage(hEditADDR, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

		  snprintf (addr_s,20,"%X",mb_Addr);
		  SetWindowText(hEditADDR,(LPCTSTR)addr_s);



		  // ПИСАТИ
		  hEditRW = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", &sNRegTW,
				WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
				, x0+100, y0+100, 100, 25,
				hwnd, (HMENU)ID_EDIT_W_NUMBER , GetModuleHandle(NULL), NULL);
		  SendMessage(hEditRW, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

		  hEditVW = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", &sValueTW,
				WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
				, x0+300, y0+100, 100, 25,
				hwnd, (HMENU)ID_EDIT_W_VALUE , GetModuleHandle(NULL), NULL);
		  SendMessage(hEditVW, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

       /* Радіокнопка - НЕХ-DEC*/
	   	  CreateWindowW(L"Button", L"", WS_CHILD /*| WS_VISIBLE*/| BS_GROUPBOX| WS_GROUP,
	   			 x0+400, y0+90 /*410*/, 50, 50, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

	   	  hWnd_W_HEX= CreateWindow("button", "HEX" , WS_CHILD |
	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
	   			 x0+400, y0+90 /*410*/, 50, 25,hwnd,(HMENU) ID_BTN_W_HEX, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);


	   	  hWnd_W_DEC= CreateWindow("button", "DEC" , WS_CHILD |
	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
	   			 x0+400, y0+110 /*410*/,50, 25,hwnd,(HMENU) ID_BTN_W_DEC, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

	   	  SendMessage(hWnd_W_HEX, BM_SETCHECK, 1, 0L);
	   	   RadioBN_R_HEX_State=1;
	   	   RadioBN_R_DEC_State=0;
	      hWndBTN_WSend = CreateWindow("BUTTON", "Відправити",
                 WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
                 x0+500, y0+100, 100, 25, hwnd, (HMENU)ID_BTN_W_SEND,
										   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);



	      // ЧИТАТИ
		  hEditRR = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", &sNRegTW,
				WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
				, x0+100, y0+100+dy, 100, 25,
				hwnd, (HMENU)ID_EDIT_R_NUMBER , GetModuleHandle(NULL), NULL);
		  SendMessage(hEditRR, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
		  /* Доска запису регістра  */
		  CreateWindowW(L"Button", L"",
                WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				 x0+300, y0+100+dy-5,  100, 37, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
//		  hEditRDEC = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", &sNRegTW,
//				WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
//				, x0+300, y0+100+dy, 100, 25,
//				hwnd, (HMENU)ID_EDIT_R_VALUE , GetModuleHandle(NULL), NULL);
//		  SendMessage(hEditRDEC, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

       /* Радіокнопка - НЕХ-DEC*/
	   	  CreateWindowW(L"Button", L"", WS_CHILD /*| WS_VISIBLE*/| BS_GROUPBOX| WS_GROUP,
	   			 x0+400, y0+90+dy /*410*/, 50, 50, hwnd, (HMENU) 0,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);

	   	  hWnd_R_HEX= CreateWindow("button", "HEX" , WS_CHILD |
	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
	   			 x0+400, y0+90+dy /*410*/, 50, 25,hwnd,(HMENU) ID_BTN_R_HEX, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);
 		  SendMessage(hWnd_R_HEX, BM_SETCHECK, 1, 0L);

	   	  hWnd_R_DEC= CreateWindow("button", "DEC" , WS_CHILD |
	   		         WS_VISIBLE | BS_AUTORADIOBUTTON ,
	   			 x0+400, y0+110+dy /*410*/,50, 25,hwnd,(HMENU) ID_BTN_R_DEC, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) , NULL);


	      hWndBTN_RSend = CreateWindow("BUTTON", "Відправити",
                 WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
                 x0+500, y0+100+dy, 100, 25, hwnd, (HMENU)ID_BTN_R_SEND,
										   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);


/*
            hButtonSend = CreateWindow("BUTTON", "Відправити",
                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
                                           200, 200, 100, 20, hwnd, (HMENU)BUTTON_MBSEND,
										   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);*/
		}
		break;

		case WM_PAINT: {// если нужно нарисовать, то:

	    	  HDC hDC; // создаём дескриптор ориентации текста на экране

//	    	  typedef struct tagPAINTSTRUCT {
//	    	    HDC  hdc;
//	    	    BOOL fErase;
//	    	    RECT rcPaint;
//	    	    BOOL fRestore;
//	    	    BOOL fIncUpdate;
//	    	    BYTE rgbReserved[32];
//	    	  } PAINTSTRUCT, *PPAINTSTRUCT;
//	    	  hdc  		Дескриптор DC устройства отображения, который используется для того, чтобы красить.
//			  fErase 	Определяет, должен ли фон быть стерт. Это значение - не нуль, если приложение должно стереть фон.
//	    	  Приложение ответственно за стирание фона, если класс окна создается без кисти для фона.
//			  Дополнительную информацию смотри в статье с описанием члена hbrBackground структуры WNDCLASS.
//			  rcPaint   Определяет структуру RECT, которая устанавливает левый верхний и нижний правый углы прямоугольника, в который запрашивается окрашивание, в единицах устройства относительно левого верхнего угла рабочей области.
	    	  RECT rect; // стр-ра, определяющая размер клиентской области
	    	  PAINTSTRUCT ps; // структура, сод-щая информацию о клиентской области (размеры, цвет и тп)

//	    	  ps.fErase = 0;
//	    	  ps.rcPaint=rect;
	    	  //(HBRUSH)(COLOR_3DFACE+1);
//	    	  COLORREF colorText = RGB(255, 255, 0); // задаём цвет текста
//	    	  CHAR db_str[20]={"COM-port"};



	    	  hDC = BeginPaint(hwnd, &ps); 	// инициализируем контекст устройства
	    	  GetClientRect(hwnd, &rect);  	// получаем ширину и высоту области для рисования
//	    	  FillRect(hDC, &rect, (HBRUSH)(COLOR_WINDOW+1));

	    	  RECT rectAddr ={x0+175, y0, x0+175+400, y0+30};
	    	  DrawText(hDC, "Адреса Модбас, HEX 0x00..0xF7", 30, &rectAddr, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	  RECT rect1 = {x0+100,y0+40,x0+400,y0+40+25};
	    	  DrawText(hDC, "ПИСАТИ РЕГІСТР", 15, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
  			  RECT rect2 = {x0+100,y0+65,x0+400,y0+65+25};
	    	  DrawText(hDC, "Регістр №, HEX                           Значення", 50, &rect2, DT_SINGLELINE|DT_LEFT|DT_TOP);

	    	  if (strlen(user_msg) != 0){
	    		  RECT rectum = {x0+100,y0+65+30,x0+400,y0+65+25};
	    		  DrawText(hDC, user_msg, 50, &rectum, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  }



	    	  RECT rect3 = {x0+100,y0+40+dy,x0+400,y0+40+25+dy};
	    	  DrawText(hDC, "ЧИТАТИ РЕГІСТР", 15, &rect3, DT_SINGLELINE|DT_LEFT|DT_TOP);
  			  RECT rect4 = {x0+100,y0+65+dy,x0+400,y0+65+25+dy};
	    	  DrawText(hDC, "Регістр,№, HEX                           Значення", 50, &rect4, DT_SINGLELINE|DT_LEFT|DT_TOP);



/*	    	  DrawText(hDC, "   байт 1 -> код команди Modbus        ", 40, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  rect1.top=rect1.top+20;
	    	  DrawText(hDC, "   CRC додається автоматично           ", 40, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  rect1.top=rect1.top+50;
	    	  DrawText(hDC, "0000",                                  40, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);*/
//	    	  int DrawText(
//	    	    HDC hDC,          // дескриптор контекста устройства
//	    	    LPCTSTR lpString, // текст для вывода
//	    	    int nCount,       // длина текста
//	    	    LPRECT lpRect,    // размеры поля форматирования
//	    	    UINT uFormat      // параметры вывода текста
//	    	  );
	    	EndPaint(hwnd, &ps);
		}

		break;
		case WM_COMMAND:{

				memset(&user_msg,0,sizeof(user_msg));
				char  u16s[20]={0};
				char  n_u16s=0;
				uint16_t u16;

			/* Зчитування адреси Модбас */
			if((wParam==ID_BTN_W_SEND) || (wParam==ID_BTN_R_SEND)){
				if(QPW_Rg_ProcessState==ProcessIddle){
					memset(&u16s,0,sizeof(u16s));
					/* Зчитуємо регістр адреси, це завжди HEX*/
					n_u16s = GetWindowTextLength(GetDlgItem(hwnd, ID_EDIT_ADDRESS));
					if(n_u16s==0){snprintf (user_msg,17,"Відсутня адреса");break;}
					if(n_u16s>2) {snprintf (user_msg,17,"Значення недопустиме");break;}
						GetDlgItemText(hwnd, ID_EDIT_ADDRESS, u16s, n_u16s+1);

						int u64i= f_check_hex(u16s, user_msg);
						if(u64i>=0){u16=(char)u64i;}else{break;}
						mb_Addr=u16;
				}
			}
			switch(wParam) {
			case ID_BTN_W_SEND:{


			  if(QPW_Rg_ProcessState==ProcessIddle){

					/* Стирання попередніх повідомлень */
					HDC hDCr; // создаём дескриптор ориентации текста на экране
					RECT rect; // стр-ра, определяющая размер клиентской области
					/*малюємо заново*/
					hDCr= GetDC(QPWRg_Wnd);
					GetClientRect(QPWRg_Wnd, &rect);  	// получаем ширину и высоту области для рисования


					/* Стирання повідомлення про помилку у */
					RECT rect_rre={x0+100, y0+100+50,  x0+600, y0+100+50+25};
					FillRect(hDCr, &rect_rre, (HBRUSH)(COLOR_WINDOW+1));
					ReleaseDC(QPWRg_Wnd, hDCr);

				memset(&u16s,0,sizeof(u16s));
				/* Зчитуємо регістр адреси, це завжди HEX*/
				n_u16s = GetWindowTextLength(GetDlgItem(hwnd, ID_EDIT_W_NUMBER));
				if(n_u16s==0){snprintf (user_msg,40,"Відсутнє повідомлення для передачі");break;}
				GetDlgItemText(hwnd, ID_EDIT_W_NUMBER, u16s, n_u16s+1);

				int u64i= f_check_hex(u16s, user_msg);
				if(u64i>=0){u16=(uint16_t)u64i;}else{break;}
				RegNumberTW=u16;
				/* Зчитуємо значення для запису в регістр, це або HEX або DEC*/
				memset(&u16s,0,sizeof(u16s));
				n_u16s = GetWindowTextLength(GetDlgItem(hwnd, ID_EDIT_W_VALUE));
				if(n_u16s==0){snprintf (user_msg,40,"Відсутнє повідомлення для передачі");break;}
				GetDlgItemText(hwnd, ID_EDIT_W_VALUE, u16s, n_u16s+1);

				int btn_pushed=SendMessage(hWnd_W_HEX, BM_GETSTATE, 0, 0L);
				int RegValue_i =0;
				if(btn_pushed==1){
					/* Якщо радіокнопка - в стані HEX */
					RegValue_i= f_check_hex(u16s, user_msg);
					if(RegValue_i>=0){RegValueTW=(uint16_t)RegValue_i;}
				}
				else{
				    /* Якщо радіокнопка - в стані DEC */
					RegValue_i = f_check_dec(u16s, user_msg);
					if(RegValue_i>=0){RegValueTW=(uint16_t)RegValue_i;}
				}
				memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
				mbTxMsg.msg[0]=mb_Addr;
				mbTxMsg.msg[1]=0x06;
				mbTxMsg.msg[2]=RegNumberTW / 0x100;
				mbTxMsg.msg[3]=RegNumberTW % 0x100;
				mbTxMsg.msg[4]=RegValueTW / 0x100;
				mbTxMsg.msg[5]=RegValueTW % 0x100;
				mbTxMsg.length=6;
				//f_add_CRC(&mbTxMsg);
				/* Запуск паралельного процесу..*/
				/* Фіксуємо варіант */
				RW_case=ID_BTN_W_SEND;
				/* Стираємо попереднє повідомлення про результат запису регістру */
//				modbus_status_t BTH_TxTx_status = MODBUS_UNKNOWN_ERROR;
//				modbus_master_tx_msg_t BTH_TxMsg={0};
//				uint32_t BTH_Msg_TimeOut = 0;
//				modbus_master_rx_msg_t BTH_RxMsg={0};
//				char BTH_User_Msg[256]={0};
//				extern modbus_status_t BTH_Rg_status = MODBUS_UNKNOWN_ERROR;
//				extern modbus_master_rx_msg_t BTH_RxMsg={0};
//				extern char BTH_User_Msg[256]={0};

				BTH_Msg_TimeOut=2000;
				BTH_TxMsg = mbTxMsg;
				QPW_Rg_ProcessState=ProcessBusy;
				QPW_Rg_ProcessStateOld=ProcessIddle;
				_beginthread((void*)f_BTH_TxRx, 0, NULL);
				f_PPB_TxRxWnd(hwnd);
			    /* Передача, прийом відповіді і друк результату - див. функція  f_QPWD_RgAnswer*/
			  }
			}

			break;
			case ID_BTN_R_SEND:{
			  if(QPW_Rg_ProcessState==ProcessIddle){

					/* Стирання попередніх повідомлень */
					HDC hDCr;  // создаём дескриптор ориентации текста на экране
					RECT rect; // стр-ра, определяющая размер клиентской области
					/*малюємо заново*/
					hDCr= GetDC(QPWRg_Wnd);
					GetClientRect(QPWRg_Wnd, &rect);  	// получаем ширину и высоту области для рисования
					/* Стираємо повідомлення для користувача */
					RECT rect_rw={x0+100, y0+100+50,  x0+600, y0+100+50+25};
					FillRect(hDCr, &rect_rw, (HBRUSH)(COLOR_WINDOW+1));

					/* Стирання повідомлення про помилку */
					RECT rect_rre={x0+100, y0+100+dy+50,  x0+600, y0+100+dy+50+25};
					FillRect(hDCr, &rect_rre, (HBRUSH)(COLOR_WINDOW+1));
					ReleaseDC(QPWRg_Wnd, hDCr);

				memset(&u16s,0,sizeof(u16s));
				/* Зчитуємо регістр адреси, це завжди HEX*/
				n_u16s = GetWindowTextLength(GetDlgItem(hwnd, ID_EDIT_R_NUMBER));
				if(n_u16s==0){snprintf (user_msg,40,"Відсутнє повідомлення для передачі");break;}
				GetDlgItemText(hwnd, ID_EDIT_R_NUMBER, u16s, n_u16s+1);

				int u64i= f_check_hex(u16s, user_msg);
				if(u64i>=0){u16=(uint16_t)u64i;}else{
					/* Стирання попередніх повідомлень */
					HDC hDCr;  // создаём дескриптор ориентации текста на экране
					RECT rect; // стр-ра, определяющая размер клиентской области
					/*малюємо заново*/
					hDCr= GetDC(QPWRg_Wnd);
					RECT rect_rre={x0+100, y0+100+dy+50,  x0+600, y0+100+dy+50+25};
					DrawText(hDCr, user_msg, 25, &rect_rre, DT_SINGLELINE|DT_LEFT|DT_TOP);
					ReleaseDC(QPWRg_Wnd, hDCr);
					break;}
				RegNumberTR=u16;


				memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
				mbTxMsg.msg[0]=mb_Addr;
				mbTxMsg.msg[1]=0x03;
				mbTxMsg.msg[2]=RegNumberTR / 0x100;
				mbTxMsg.msg[3]=RegNumberTR % 0x100;
				mbTxMsg.msg[4]=0x00;
				mbTxMsg.msg[5]=0x01;
				mbTxMsg.length=6;
				/* Запуск паралельного процесу..*/
				/* Фіксуємо варіант */
				RW_case=ID_BTN_R_SEND;
				/* Стираємо попереднє повідомлення про результат запису регістру */
//				modbus_status_t BTH_TxTx_status = MODBUS_UNKNOWN_ERROR;
//				modbus_master_tx_msg_t BTH_TxMsg={0};
//				uint32_t BTH_Msg_TimeOut = 0;
//				modbus_master_rx_msg_t BTH_RxMsg={0};
//				char BTH_User_Msg[256]={0};
//				extern modbus_status_t BTH_Rg_status = MODBUS_UNKNOWN_ERROR;
//				extern modbus_master_rx_msg_t BTH_RxMsg={0};
//				extern char BTH_User_Msg[256]={0};

				BTH_Msg_TimeOut=300;
				BTH_TxMsg = mbTxMsg;
				QPW_Rg_ProcessState=ProcessBusy;
				QPW_Rg_ProcessStateOld=ProcessIddle;
				_beginthread((void*)f_BTH_TxRx, 0, NULL);
				f_PPB_TxRxWnd(hwnd);
			    /* Передача, прийом відповіді і друк результату - див. функція  f_QPWD_RgAnswer*/
			  }
			}
			break;
			case ID_BTN_W_DEC:{

					memset(&u16s,0,sizeof(u16s));
					n_u16s = GetWindowTextLength(GetDlgItem(hwnd, ID_EDIT_W_VALUE));
					/* HEX to DEC */
					if(n_u16s==0){snprintf (user_msg,40,"Відсутнє повідомлення для передачі");break;}

					GetDlgItemText(hwnd, ID_EDIT_W_VALUE, u16s, n_u16s+1);

					int RegValue_i = f_check_hex(u16s, user_msg);
					if(RegValue_i >= 0){RegValueTW=(uint16_t)RegValue_i;}else{break;}
					char RegValue_s[20]={0};
					snprintf (RegValue_s,20,"%d",RegValueTW);
					SetWindowText(hEditVW,(LPCTSTR)RegValue_s);

			}
			break;
			case ID_BTN_W_HEX:{



					memset(&u16s,0,sizeof(u16s));
					n_u16s = GetWindowTextLength(GetDlgItem(hwnd, ID_EDIT_W_VALUE));
					/* DEC to HEX */
					if(n_u16s==0){snprintf (user_msg,40,"Відсутнє повідомлення для передачі");break;}
					GetDlgItemText(hwnd, ID_EDIT_W_VALUE, u16s, n_u16s+1);

					int RegValue_i = f_check_dec(u16s, user_msg);
					if(RegValueTW>=0){RegValueTW=(uint16_t)RegValue_i;}else{break;}
					char RegValue_s[20]={0};
					snprintf (RegValue_s,20,"%4x",RegValueTW);
					BOOL uii = SetWindowText(hEditVW,(LPCTSTR)RegValue_s);

			}
			break;
			case ID_BTN_R_DEC:{
//
//					int RegValue_i = f_check_hex(sValueR, user_msg);
//					if(RegValue_i >= 0){RegValueR=(uint16_t)RegValue_i;}else{break;}
					char RegValue_s[20]={0};
					snprintf (RegValue_s,20,"%5d",RegValueR);
					HDC hDCr= GetDC(QPWRg_Wnd);
					RECT rect;
					GetClientRect(QPWRg_Wnd, &rect);  	// получаем ширину и высоту области для рисования
					RECT rect_res= {x0+300+10, y0+100+dy+5,  x0+400, y0+100+dy+25+5};
					//FillRect(hDCr, &rect_res, (HBRUSH)(COLOR_WINDOW+1));
					DrawText(hDCr, RegValue_s, 6, &rect_res, DT_SINGLELINE|DT_LEFT|DT_TOP);
					ReleaseDC(QPWRg_Wnd, hDCr);

			}
			break;
			case ID_BTN_R_HEX:{

					char RegValue_s[20]={0};
					snprintf (RegValue_s,20,"%4X ",RegValueR);
					HDC hDCr= GetDC(QPWRg_Wnd);
					RECT rect;
					GetClientRect(QPWRg_Wnd, &rect);  	// получаем ширину и высоту области для рисования
					RECT rect_res= {x0+300+10, y0+100+dy+5,  x0+400, y0+100+dy+25+5};;
					//FillRect(hDCr, &rect_res, (HBRUSH)(COLOR_WINDOW+1));
					DrawText(hDCr, RegValue_s, 6, &rect_res, DT_SINGLELINE|DT_LEFT|DT_TOP);
					ReleaseDC(QPWRg_Wnd, hDCr);


			}
			break;

			default:{}
		}
		}
		break;
		case WM_CLOSE:
//			 mb_mon = mb_mon_setted;
			 DestroyWindow(hwnd);
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
		break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);

//			int len = GetWindowTextLength(GetDlgItem(hwnd, IDC_TEXT));
//			if(len > 0)
//			{
//			int i;
//			char* buf;
//			buf = (char*)GlobalAlloc(GPTR, len + 1);
//			GetDlgItemText(hwnd, IDC_TEXT, buf, len + 1);
//			//... do stuff with text ...
//			GlobalFree((HANDLE)buf);
//			}

	}
	return 0;
}




/* Друк/розмальовка за результатами паралельного процесу,
 * викликається з вікна PPB паралельного процесу */
void f_QPWD_RgAnswer(void){

	HDC hDCr; // создаём дескриптор ориентации текста на экране
	RECT rect; // стр-ра, определяющая размер клиентской области
	/*малюємо заново*/
	hDCr= GetDC(QPWRg_Wnd);
	GetClientRect(QPWRg_Wnd, &rect);  	// получаем ширину и высоту области для рисования


	memset(user_msg, 0,sizeof(user_msg));
	if (BTH_TxTx_status!=MODBUS_SUCCESS){
		/* Zкщо була виявлена помилка сеансу RTC  */
		memcpy(&user_msg,BTH_User_Msg,sizeof(user_msg));
	}
	/* Якщо помилки виявлено не було */
	while(BTH_TxTx_status==MODBUS_SUCCESS){
		mbRxMsg=BTH_RxMsg;
		/* Якщо не співпадає код команди */
        if(!((mbRxMsg.msg[1] == mbTxMsg.msg[1])||(mbRxMsg.msg[1] == (mbTxMsg.msg[1]+0x80)))){
        		  snprintf (user_msg,80," Недозволений код команди ");
        		  break;
        }
        /* Якщо це повідомлення, про помилку, виявлену сервером Модбас */
        if(!((mbRxMsg.msg[1] == mbTxMsg.msg[1])||(mbRxMsg.msg[1] == (mbTxMsg.msg[1]+0x80)))){
         		  snprintf (user_msg,80," Недозволений код команди ");
         		  break;
        }
        else if(mbRxMsg.msg[1] == (mbTxMsg.msg[1]+0x80)){
        	if(mbRxMsg.msg[1]==0x02){
        		snprintf (user_msg,80," Регістр з таким номером не підтримується  ");
        	}
        	else{
        		snprintf (user_msg,80," Так бути не повинно   ");
        	}
        }
        /* Відповідь правильна */
        else if((mbRxMsg.msg[1] == mbTxMsg.msg[1])){

        	if(RW_case==ID_BTN_W_SEND){
        		/* */
        		uint16_t NRegFW = mbRxMsg.msg[2]*0x100 + mbRxMsg.msg[3];
        		uint16_t ValueFW = mbRxMsg.msg[4]*0x100 + mbRxMsg.msg[5];
        		if((NRegFW==RegNumberTW)&&(ValueFW==RegValueTW)){

        			snprintf (user_msg,80," Значення регістра записано БЕЗ ПОМИЛОК ");
       		    }
        		else{
        			snprintf (user_msg,80," Помилка запису значення в регістр");
        		}
        	}
        	else if(RW_case==ID_BTN_R_SEND){
        		/* */
        		uint16_t NRegTW = mbTxMsg.msg[4]*0x100 + mbTxMsg.msg[5];
        		uint16_t NBytesR = mbRxMsg.msg[2];
        		if((NBytesR==2)&&(NRegTW==1)){

        			RegValueR=mbRxMsg.msg[3]*0x100 + mbRxMsg.msg[4];

    				/* Зчитуємо значення для запису в регістр, це або HEX або DEC*/
    				int btn_pushed=SendMessage(hWnd_R_HEX, BM_GETSTATE, 0, 0L);
    				if(btn_pushed==1){
    					/* Якщо радіокнопка - в стані HEX */
    					snprintf (sValueR,6,"%4X",RegValueR);
    				}
    				else{
    				    /* Якщо радіокнопка - в стані DEC */
    					snprintf (sValueR,6,"%5d",RegValueR);
    				}
        		}
        		else{
        			snprintf (user_msg,80," Відповідь не відповідає формату Модбас");
        		}
        	}
        	else{}
        }
        else{}
        /* Вихід з блоку */
        BTH_TxTx_status=MODBUS_UNKNOWN_ERROR;
	}

	if  (RW_case==ID_BTN_W_SEND){
		/* Друк повідомлення для користувача */
		RECT rect_rw={x0+100, y0+100+50,  x0+600, y0+100+50+25};
		DrawText(hDCr, (char*)user_msg, 140, &rect_rw, DT_SINGLELINE|DT_LEFT|DT_TOP);
	}

	else if (RW_case==ID_BTN_R_SEND){
		if(strlen((char*)user_msg) ==0){
    		//register x0+300, y0+100+dy-5,  100, 30,
    		//y0+100+dy,
			/* Друк значення зчитаного регістра */
			RECT rect_rrv={x0+300+10, y0+100+dy+5,  x0+390, y0+100+dy+25+5};
			FillRect(hDCr, &rect_rrv, (HBRUSH)(COLOR_WINDOW+1));
			DrawText(hDCr, (char*)sValueR, 5, &rect_rrv, DT_SINGLELINE|DT_LEFT|DT_TOP);
			/* Стирання повідомлення про помилку у */
			RECT rect_rre={x0+100, y0+100+dy+50,  x0+600, y0+100+dy+50+25};
			FillRect(hDCr, &rect_rre, (HBRUSH)(COLOR_WINDOW+1));
		}
		else{
			/* Друк тексту повідомлення про помилку */
			RECT rect_rr={x0+100, y0+100+dy+50,  x0+600, y0+100+dy+50+25};
			DrawText(hDCr, (char*)user_msg, 140, &rect_rr, DT_SINGLELINE|DT_LEFT|DT_TOP);

		};
		memset(&mbTxMsg,0,sizeof(modbus_master_tx_msg_t));
		memset(&mbRxMsg,0,sizeof(modbus_master_rx_msg_t));

	}
	else{}

	ReleaseDC(QPWRg_Wnd, hDCr);
	QPW_Rg_ProcessState=ProcessIddle;

}






