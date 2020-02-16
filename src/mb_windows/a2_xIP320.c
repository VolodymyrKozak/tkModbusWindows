#include <windows.h>
#include "a2_xIP320.h"
#include "a0_win_control.h"

#include <process.h>    /* _beginthread, _endthread */

#include "../mb_funcs/tx_manual.h"
#include "a3_monitoring.h"
#include "a3_winTxRx.h"
#define BUTTON_MBSEND 9678

extern HINSTANCE hinst;
const char g_szClassNameC320[] = "IP320_WndClass";

extern int cport_nr;
static modbus_master_tx_msg_t mbTxMsg={0};
static modbus_master_rx_msg_t mbRxMsg={0};
static char user_msg[256]={0};
//static void f_txrx(void);
static HWND hwndTx=NULL;
static ProcessState_t TxProcessState = ProcessIddle;





static HWND WINAPI CreateTrackbar(
    HWND hwndDlg,  // handle of dialog box (parent window)
    UINT iMin,     // minimum value in trackbar range
    UINT iMax,     // maximum value in trackbar range
    UINT iSelMin,  // minimum value in trackbar selection
    UINT iSelMax   // maximum value in trackbar selection
	);

LRESULT CALLBACK WndProcC320(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	hwndTx=hwnd;
	static HWND hEdit;
	static HWND  hButtonSend;
	static uint8_t  mbMSG0[768]="0F 11";
	static uint16_t n_mbMSG0=0;
	static HWND hwndTrack1; // handle of trackbar window
    static UINT iSelMin = 0;   // minimum value of trackbar selection
    static UINT iSelMax =100;   // maximum value of trackbar selection
    static UINT dwPos=0;
	switch(msg)
	{
		case WM_CREATE:
		{
//			HWND hwndTrack1 = CreateTrackbar(
//			    hwnd,      // handle of dialog box (parent window)
//				iSelMin,   // minimum value in trackbar range
//				iSelMax,   // maximum value in trackbar range
//				iSelMin,   // minimum value in trackbar selection
//				iSelMax);  // maximum value in trackbar selection



			HFONT hfDefault;

			hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", (char*)mbMSG0,
				WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
				,
				0,0,0,0,
//				100, 100, 1000, 100,

				hwnd, (HMENU)IDC_MAIN_EDIT2, GetModuleHandle(NULL), NULL);
			if(hEdit == NULL)
				MessageBox(hwnd, "Could not create edit box.", "Error", MB_OK | MB_ICONERROR);

//			hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			SendMessage(hEdit, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
            hButtonSend = CreateWindow("BUTTON", "Відправити",
                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
                                           0, 0, 0, 0, hwnd, (HMENU)BUTTON_MBSEND,
										   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
		}
		break;
		case TB_ENDTRACK:

		     dwPos = SendMessage(hwndTrack1, TBM_GETPOS, 0, 0);

		     if (dwPos > iSelMax)
		         SendMessage(hwndTrack1, TBM_SETPOS,
		                    (WPARAM) TRUE,       // redraw flag
		                    (LPARAM) iSelMax);

		     else if (dwPos < iSelMin)
		                SendMessage(hwndTrack1, TBM_SETPOS,
		                    (WPARAM) TRUE,       // redraw flag
		                    (LPARAM) iSelMin);

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
	    	  RECT rect1 =rect;

	    	  rect1.top   =rect.top+800+20;
//	    	  rect1.bottom   =rect1.top+75;
	    	  DrawText(hDC, "Відредагуйте повідомлення для передачі:", 40, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  rect1.top=rect1.top+20;
	    	  DrawText(hDC, "   байт 0 -> адреса Modbus Slave       ", 40, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  rect1.top=rect1.top+20;
	    	  DrawText(hDC, "   байт 1 -> код команди Modbus        ", 40, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  rect1.top=rect1.top+20;
	    	  DrawText(hDC, "   CRC додається автоматично           ", 40, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  rect1.top=rect1.top+50;
	    	  DrawText(hDC, user_msg,                                  40, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
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
		case WM_SIZE:
		{
			RECT rcClient;

			GetClientRect(hwnd, &rcClient);
			RECT R1 = rcClient;
			R1.top+=800+100;
			hEdit = GetDlgItem(hwnd, IDC_MAIN_EDIT2);
//			SetWindowPos(hEdit, NULL,       R1.top,       R1.top+100,           R1.right-110, 25, SWP_NOZORDER);
//			SetWindowPos(hButtonSend, NULL, R1.right-110, 100, 100,               25, SWP_NOZORDER);

			SetWindowPos(hEdit, NULL,       0,                  R1.top, rcClient.right-110, 25, SWP_NOZORDER);
			SetWindowPos(hButtonSend, NULL, rcClient.right-110, R1.top, 100,                25, SWP_NOZORDER);

		//	https://studfiles.net/preview/1410070/page:13/

            // Make the edit control the size of the window's client area.

//            MoveWindow(hEdit,
//                       0, 0+100,              // starting x- and y-coordinates
//                       LOWORD(lParam),        // width of client area
//					   25,
////                       HIWORD(lParam-100),    // height of client area
//                       TRUE);                 // repaint window

			// Calculate remaining height and size edit
//			GetClientRect(hwnd, &rcClient);
//			hEdit = GetDlgItem(hwnd, IDC_CHILD_EDIT);
//			SetWindowPos(hEdit, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);
		}

		break;
		case WM_COMMAND:{
			switch(wParam) {
				case BUTTON_MBSEND:
				if(TxProcessState==ProcessBusy){
					break;
				}
				else{
					memset(user_msg,0,sizeof(user_msg));
					memset(mbMSG0, 0, sizeof(mbMSG0));
//					n_mbMSG0 = GetWindowTextLength(GetDlgItem(hwnd, IDC_MAIN_EDIT));
					if(n_mbMSG0==0){
						snprintf (user_msg,40,"Відсутнє повідомлення для передачі");
						break;
					}
//					GetDlgItemText(hwnd, IDC_MAIN_EDIT, (char*)mbMSG0, n_mbMSG0+1);
					lt_status_t ltt = MB_TxRxManual(
							mbMSG0,
							n_mbMSG0+1,
							&mbTxMsg
							);

					switch(ltt){
						case lt_notHEX:   {
							snprintf (user_msg,40,"Не всі символи це НЕХ");
							break;
						}
						case lt_byteWrong:{
							snprintf (user_msg,40,"Перевірте знаки між байтами");
							break;
						}
						case lt_OK:       {
//							_beginthread((void*)f_txrx, 0, NULL);
							break;
						}
						default:{}
					}
					HDC hDCr; // создаём дескриптор ориентации текста на экране
					RECT rect; // стр-ра, определяющая размер клиентской области
		    		/*малюємо заново*/
		        	hDCr= GetDC(hwnd);
		        	GetClientRect(hwnd, &rect);  	// получаем ширину и высоту области для рисования
		        	RECT rect_res=rect;

		    		rect_res.top+=130;
		        	FillRect(hDCr, &rect_res, (HBRUSH)(COLOR_WINDOW+1));
		        	DrawText(hDCr, user_msg, 40, &rect_res, DT_SINGLELINE|DT_LEFT|DT_TOP);

		        	ReleaseDC(hwnd, hDCr);
				}
				break;
				default:{}
			}
		}
		break;
		case WM_CLOSE:
			 mb_mon = mb_mon_setted;
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


int WINAPI f_Create320Wnd(HWND hh)
{

//	WNDCLASSEX wc;
	HWND childTx;
	MSG Msg;
//	memset(&wc,0,sizeof(WNDCLASS));
//
//	wc.cbSize		 = sizeof(WNDCLASSEX);
//	wc.style		 = 0;
//	wc.lpfnWndProc	 = WndProcC1;
//	wc.cbClsExtra	 = 0;
//	wc.cbWndExtra	 = 0;
//	wc.hInstance	 = hinst;
//	wc.hIcon	 =
//		CreateIcon(
//			hinst,       	// application instance
//			32,             // icon width
//			32,             // icon height
//			1,              // number of XOR planes
//			1,              // number of bits per pixel
//			ANDmaskTK_Icon, // AND bitmask
//			XORmaskTK_Icon  // XOR bitmask
//			);
//	wc.hCursor=LoadCursor(NULL, IDC_ARROW);
//	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION);
//	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
//	wc.lpszMenuName  = NULL;
//	wc.lpszClassName = g_szClassNameC1;
	WNDCLASS w;
		memset(&w,0,sizeof(WNDCLASS));
		w.lpfnWndProc = WndProcC320;
		w.hInstance = hinst;
		w.style = CS_HREDRAW | CS_VREDRAW;
		w.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
		w.lpszClassName = g_szClassNameC320;
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

	childTx = CreateWindowEx(
		0,
		g_szClassNameC320,
		"IP320_Player",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1200, 3200,
		NULL, NULL, hinst, NULL);

	if(childTx == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(childTx, SW_NORMAL);
	UpdateWindow(childTx);

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}

