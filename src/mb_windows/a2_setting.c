/*
 * a2_childS.c
 *
 *  Created on: 5 серп. 2019 р.
 *      Author: Volodymyr.Kozak
 */

#include <stdbool.h>

#include "../mb_funcs/tkRS232rx.h"
#include "a0_win_control.h"
#include "a2_setting.h"
#include "a3_monitoring.h"


const char g_szChildClassNameS[] = "myMDIChildWindowClassS";
window_state_t windowS_state = child_windows_state_unKnown;


#define COMPORT  	0x0001
#define BOUDRATE 	0x0002
#define NBITS    	0x0003
#define PARITY   	0x0004
#define STOP_BIT 	0x0005
#define BUTTON_CNSL 0x0010
#define BUTTON_OK   0x0011

extern HINSTANCE hinst;
extern mb_mon_t mb_mon;

HWND comWndChild;
char g_szClassNameS[]="Налаштування";

/* Реєстрація вікна налаштувань */
LPCSTR f_RegisterSWndClass(HINSTANCE hinst){
	WNDCLASS w;
		memset(&w,0,sizeof(WNDCLASS));
		w.lpfnWndProc = ChildWndProcS;
		w.hInstance = hinst;
		w.style = CS_HREDRAW | CS_VREDRAW;
		w.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
		w.lpszClassName = g_szClassNameS;
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
	//	RegisterClassA(&w);

		if(!RegisterClassA(&w)){
			MessageBox(NULL, "Window Registration Failed!", "Error!",
					MB_ICONEXCLAMATION | MB_OK);
		}
		return w.lpszClassName;
}

/* Створення вікна налаштувань */
int WINAPI f_CreateSWnd(HWND hh){
	MSG Msg;
	comWndChild=CreateWindowEx(
		0,
		g_szClassNameS,
		TEXT("Налаштування параметрів мережі Модбас"),
		CS_HREDRAW | CS_VREDRAW,

		20,
		20,
		648,
		400,
		hh,
		NULL,
		hinst,
		NULL
		);

	ShowWindow(comWndChild,SW_NORMAL);
	UpdateWindow(comWndChild);
	return Msg.wParam;;
}

/* Функція обробки повідомлень  вікна налаштувань                              */
LRESULT CALLBACK ChildWndProcS(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
//	MessageBox(hwnd, "ComboBox Started.", "Ok", MB_OK | MB_ICONERROR);
	switch(msg){
		case WM_CREATE:{
//			MessageBox(hwnd, "ComboBox Started.", "Ok", MB_OK | MB_ICONERROR);
//			char *uuu = comports;
			int x=300; int y = 80;
//			hrx232=f_get_rs232rx();
			HWND ComporthWnd;
			ComporthWnd = CreateWindow("COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
            x, y, 200, 800,
			hwnd, (HMENU)COMPORT,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),NULL);
            if (!ComporthWnd){
 //               MessageBox(NULL, "ComboBox Failed.", "Error", MB_OK | MB_ICONERROR);
            }
            char com[6]={"COM__"};
            com[5]=0;
            for (uint8_t i=0;i<po.n;i++){
            	if ((po.comprt[i]+1)<10){
            		com[3]=0x30+(uint8_t)(po.comprt[i]+1);
            		com[4]='\0';
            	}
            	else{
            		com[3]=0x30+(po.comprt[i]+1)/10;
            		com[4]=0x30+(po.comprt[i]+1)%10;
            	}
            	SendMessage(ComporthWnd, CB_ADDSTRING, 0, (LPARAM)com);/*0*/
            }

            if (r.xcport != -1){
            	SendMessage(ComporthWnd, CB_SETCURSEL, r.xcport/*2*/, 0);
            }
			HWND hWndComboBoxBoudRate;
            hWndComboBoxBoudRate = CreateWindow("COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
            x, y+30, 200, 800,
			hwnd, (HMENU)BOUDRATE,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),NULL);
            if (!hWndComboBoxBoudRate){
//                MessageBox(NULL, "ComboBox Failed.", "Error", MB_OK | MB_ICONERROR);
            }
            SendMessage(hWndComboBoxBoudRate, CB_ADDSTRING, 0, (LPARAM)"  4800 bits/s");
            SendMessage(hWndComboBoxBoudRate, CB_ADDSTRING, 0, (LPARAM)"  9600 bits/s");
            SendMessage(hWndComboBoxBoudRate, CB_ADDSTRING, 0, (LPARAM)" 19200 bits/s");
            SendMessage(hWndComboBoxBoudRate, CB_ADDSTRING, 0, (LPARAM)" 38400 bits/s");
            SendMessage(hWndComboBoxBoudRate, CB_ADDSTRING, 0, (LPARAM)" 57600 bits/s");
            SendMessage(hWndComboBoxBoudRate, CB_ADDSTRING, 0, (LPARAM)"115200 bits/s");
            SendMessage(hWndComboBoxBoudRate, CB_SETCURSEL,r.xbdrate , 0);
//            SendMessage(hStatus, SB_SETTEXT, 1, (LPARAM)(br+15*r.xbdrate));
			HWND hWndComboBoxBits;
            hWndComboBoxBits = CreateWindow("COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
            x, y+60, 200, 800,
			hwnd, (HMENU)NBITS,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),NULL);
            if (!hWndComboBoxBits){
//                MessageBox(NULL, "ComboBox Failed.", "Error", MB_OK | MB_ICONERROR);
            }
            SendMessage(hWndComboBoxBits, CB_ADDSTRING, 0, (LPARAM)" 9 bits including parity");
            SendMessage(hWndComboBoxBits, CB_ADDSTRING, 0, (LPARAM)" 8 bits including parity");
            SendMessage(hWndComboBoxBits, CB_ADDSTRING, 0, (LPARAM)" 7 bits including parity");
            SendMessage(hWndComboBoxBits, CB_SETCURSEL, r.xnbits, 0);

			HWND hWndComboBoxParity;
            hWndComboBoxParity = CreateWindow("COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
            x, y+90, 200, 800,
			hwnd, (HMENU)PARITY,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),NULL);
            if (!hWndComboBoxParity){
//                MessageBox(NULL, "ComboBox Failed.", "Error", MB_OK | MB_ICONERROR);
            }
            SendMessage(hWndComboBoxParity, CB_ADDSTRING, 0, (LPARAM)" None");
            SendMessage(hWndComboBoxParity, CB_ADDSTRING, 0, (LPARAM)" Even");
            SendMessage(hWndComboBoxParity, CB_ADDSTRING, 0, (LPARAM)" Odd") ;
            SendMessage(hWndComboBoxParity, CB_SETCURSEL, r.xparity, 0);
			HWND hWndComboBoxStopBit;
            hWndComboBoxStopBit = CreateWindow("COMBOBOX", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
            x, y+120, 200, 800,
			hwnd, (HMENU)STOP_BIT,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),NULL);
            if (!hWndComboBoxStopBit){
 //               MessageBox(NULL, "ComboBox Failed.", "Error", MB_OK | MB_ICONERROR);
            }

            SendMessage(hWndComboBoxStopBit, CB_ADDSTRING, 0, (LPARAM)"1 bit");
//            SendMessage(hWndComboBoxStopBit, CB_ADDSTRING, 0, (LPARAM)"1.5 bits");
            SendMessage(hWndComboBoxStopBit, CB_ADDSTRING, 0, (LPARAM)"2 bits") ;
            SendMessage(hWndComboBoxStopBit, CB_SETCURSEL, r.xstopbit, 0);

            HWND  hWndButtonCnsl = CreateWindow("BUTTON", "Cкасувати",
                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
                                           130, y+200, 97, 30, hwnd, (HMENU)BUTTON_CNSL,
										   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
            if (!hWndButtonCnsl){
//                MessageBox(NULL, "ButtonCnsl Failed.", "Error", MB_OK | MB_ICONERROR);
            }
            HWND  hWndButtonOk   = CreateWindow("BUTTON", "Підтвердити",
                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
                                           400, y+200, 97, 30, hwnd, (HMENU)BUTTON_OK,
										   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
            if (!hWndButtonOk){
//                MessageBox(NULL, "ButtonOk Failed.", "Error", MB_OK | MB_ICONERROR);

            }
//            MessageBox(NULL, "ComboBox Finished.", "Ok", MB_OK | MB_ICONERROR);
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
	    	  RECT rect1 =rect;
	    	  rect1.top=rect.top+83;
	    	  rect1.left   =rect.left+150;

	    	  DrawText(hDC, "COM-port", 8, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  rect1.top+=30;
	    	  DrawText(hDC, "Boud Rate", 9, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  rect1.top+=30;
	    	  DrawText(hDC, "Bits", 4, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  rect1.top+=30;
	    	  DrawText(hDC, "Parity", 6, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  rect1.top+=30;
	    	  DrawText(hDC, "Stop Bit", 9, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
//	    	  int DrawText(
//	    	    HDC hDC,          // дескриптор контекста устройства
//	    	    LPCTSTR lpString, // текст для вывода
//	    	    int nCount,       // длина текста
//	    	    LPRECT lpRect,    // размеры поля форматирования
//	    	    UINT uFormat      // параметры вывода текста
//	    	  );
	//    	f_main_paint(hWnd);

	    }
	    return 0;
		case WM_MDIACTIVATE:
//		{
//			HMENU hMenu, hFileMenu;
//			UINT EnableFlag;
//			hMenu = GetMenu(g_hMainWindow);
//			if(hwnd == (HWND)lParam)
//			{	   //being activated, enable the menus
//				EnableFlag = MF_ENABLED;
//			}
//			else
//			{						   //being de-activated, gray the menus
//				EnableFlag = MF_GRAYED;
//			}
//
//			EnableMenuItem(hMenu, 1, MF_BYPOSITION | EnableFlag);
//			EnableMenuItem(hMenu, 2, MF_BYPOSITION | EnableFlag);
//
//			hFileMenu = GetSubMenu(hMenu, 0);
//			EnableMenuItem(hFileMenu, ID_FILE_SAVEAS, MF_BYCOMMAND | EnableFlag);
//
//			EnableMenuItem(hFileMenu, ID_FILE_CLOSE, MF_BYCOMMAND | EnableFlag);
//			EnableMenuItem(hFileMenu, ID_FILE_CLOSEALL, MF_BYCOMMAND | EnableFlag);
//
//			DrawMenuBar(g_hMainWindow);
//		}
		break;
		case WM_COMMAND:{

			// https://studfiles.net/preview/1189313/page:65/
			// https://docs.microsoft.com/en-us/windows/win32/controls/create-a-simple-combo-box?fbclid=IwAR02a5CDLjVlZel_Tz3dWkTWDkWqwK1ECXkU-8b73ktYsxprm7tPPoLrdhk
			// https://lektsii.org/5-31025.html
			// https://studfiles.net/preview/1410070/page:35/
			// дескриптор дочер. окна If(idCtrl==ID_combo&&code==CBN_SELCHANGE)
			UINT NotificationCode = HIWORD(wParam);
			switch (NotificationCode) {
				/* якщо виявлено вибір користувача*/
     			case CBN_SELCHANGE:{
     				/* Запит - який саме номер по порядку з меню вибраний (незалежно від комбобоксу */
		            int ItemIndex = SendMessage((HWND) lParam, (UINT) CB_GETCURSEL,
		                (WPARAM) 0, (LPARAM) 0);

					UINT idCtl=LOWORD(wParam);
					/* Переносимо зроблений вибір у заголовок комбобокса*/
					/* Ой.. Здається система це робить сама :)*/
					// SendMessage((HWND)lParam, CB_SETCURSEL, idCtl, 0);
					/* в залежності від параметру приймаємо вибрані значення параметрів*/
					switch(idCtl){
						case COMPORT:
							r.xcport=ItemIndex;
							break;
						case BOUDRATE:
							r.xbdrate=ItemIndex;
							break;
						break;
						case NBITS:
							r.xnbits=ItemIndex;
							break;
						case PARITY:
							r.xparity=ItemIndex;
							break;
						case STOP_BIT:
							r.xstopbit=ItemIndex;
							break;
						default:{}

					} // switch(idCtl)
				}//case CBN_SELCHANGE:
     			break;
			} //switch (NotificationCode)
			switch(wParam) {
				case BUTTON_CNSL:
					DestroyWindow(hwnd);
					break;
				case BUTTON_OK:

					if (r.xcport != -1){
						/* Закриваємо всі СОМ-порти, відкриті для роботи меню,
						 * включаючи вибраний.
						Вибраний компорт запамятовується і потім відкривається на
						запис чи читання в iнших пунктах меню */
						f_set_rs232rx(&r);

//						SendMessage(hStatus, SB_SETTEXT, 1, (LPARAM)(comports[r.xcport]));
//						int msbok = MessageBox(0, TEXT("Налаштування RS-232 завершено успішно"),
//														TEXT("Повідомлення"), MB_ICONERROR | MB_OK);
//						MessageBox(hwnd, TEXT("Налаштування RS-232 завершено успішно"),
//														TEXT("Повідомлення"), MB_ICONERROR | MB_OK);
						mb_mon = mb_mon_setted;
						SendMessage(hwnd, WM_CLOSE, 0, 0);
					}
					else {
						MessageBox(hwnd, TEXT("Не визначено СОМ-порт"),
								TEXT("Увага!"), MB_ICONERROR | MB_OK);
					}

					break;
				default:{}
			}
		}
		break;
		case WM_SIZE:
		//	https://studfiles.net/preview/1410070/page:13/
		{
			HWND hEdit;
			RECT rcClient;

			// Calculate remaining height and size edit
			GetClientRect(hwnd, &rcClient);
			hEdit = GetDlgItem(hwnd, IDC_CHILD_EDIT);
			SetWindowPos(hEdit, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);
		}
		break;
		case SW_MINIMIZE:
		break;
		case SW_MAXIMIZE:
		break;
		case SW_RESTORE:
		break;
		case WM_CLOSE:
			if (windowS_state==child_window_open){windowS_state=child_window_closed;}
			DestroyWindow(hwnd);
		break;

		default:
			return DefMDIChildProc(hwnd, msg, wParam, lParam);

	}
	return 0;
//    HWND hWnd,
//    UINT Msg,
//    WPARAM wParam,
//    LPARAM lParam
}

