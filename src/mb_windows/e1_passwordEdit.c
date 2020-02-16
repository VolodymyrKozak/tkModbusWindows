/*
 * e1_password2.c
 *
 *  Created on: 12 груд. 2019 р.
 *      Author: KozakVF
 */

#include "stdint.h"
#include "e1_passwordEdit.h"
#include "../auxs/hexdec.h"

static HWND WndCalledPsw;
char outPassword[256]={0};
extern HINSTANCE wchInstance;

const char g_szClassNamePsw[] = "g_szClassNamePsw";

LPCSTR f_Register_PasswordWndClass(HINSTANCE hinst){
	WNDCLASSEX  wndclass;									// WNDCLASSEX - This variable will hold all the information about the window.

	wndclass.cbSize        = sizeof (wndclass);				// Here we set the size of the wndclass.
	wndclass.style         = CS_HREDRAW | CS_VREDRAW;		// The style we want is Verticle-Redraw and Horizontal-Redraw
	wndclass.lpfnWndProc   = WndPasswordProc;						// Here is where we assign our CALLBACK function.
	wndclass.cbClsExtra    = 0;								// We don't want to allocate any extra bytes for a class (useless for us)
	wndclass.cbWndExtra    = 0;								// Another useless thing for us.
	wndclass.hInstance     = wchInstance;						// We assign our hInstance to our window.
	wndclass.hIcon         = LoadIcon (NULL, IDI_WINLOGO);	// We call a function called LoadIcon that returns information about what icon we want.
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);	// We call a function called LoadCursor that returns information about what cursor we want.
															// Here we set the background color.
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wndclass.lpszMenuName  = NULL;							// We set the menu ID to NULL because we don't have a menu

	wndclass.lpszClassName = g_szClassNamePsw;					// Here we set a name for our class, we use the name passed in to the function.
	wndclass.hIconSm       = LoadIcon (NULL, IDI_WINLOGO);	// We want the icon to be the windows logo.

	RegisterClassEx (&wndclass);							// We need to register our windows class with the windows OS.
	return wndclass.lpszClassName;
}
HWND CreatePswWindow(HWND hwnd){
MSG Msg;
HWND childPsw=CreateWindowEx(
		0,
		g_szClassNamePsw,
		"Перевірка прав доступу",

//        WS_CHILD
//		| (WS_OVERLAPPEDWINDOW&(~(WS_MAXIMIZEBOX|WS_MINIMIZEBOX)))
//		|WS_BORDER
//		|WS_VISIBLE
//
//		,

		(WS_OVERLAPPEDWINDOW&(~(WS_MAXIMIZEBOX|WS_MINIMIZEBOX)))|WS_BORDER|WS_VISIBLE
		,
		CW_USEDEFAULT, CW_USEDEFAULT, 600, 272,
		NULL, NULL, wchInstance, NULL);
    WndCalledPsw = hwnd;
	ShowWindow(childPsw, SW_NORMAL);
	UpdateWindow(childPsw);

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return childPsw;
}
LRESULT CALLBACK WndPasswordProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	static HWND  hEditPsw;
	//static char sPassword[256]={0};
	//static int n_strValueOld1=0;
	switch(msg){
		case WM_CREATE:{
			HFONT hfDefault;
//			hEditPsw = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", NULL/*&qPassport*/,
//						WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
//						,100, 80, 100, 25,
//						hwnd, (HMENU)IDE_PASSWORDEDIT, GetModuleHandle(NULL), NULL);
//			SendMessage(hEditPsw, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

//			SetWindowText(hEditPsw, "****");
			hEditPsw = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", NULL/*&qPassport*/,
						WS_CHILD  | ES_PASSWORD | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
						,100, 80, 100, 25,
						hwnd, (HMENU)IDE_PASSWORDEDIT, GetModuleHandle(NULL), NULL);
			SendMessage(hEditPsw, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));



	        HWND  hWndButtonCnsl = CreateWindow("BUTTON", "Cкасувати",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
	                                           100, 160, 100, 25, hwnd, (HMENU)BUTTON_PSW_CNSL,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
	        if (!hWndButtonCnsl){
	        	MessageBox(NULL, "ButtonCnsl Failed.", "Error", MB_OK | MB_ICONERROR);
	        }
	        HWND  hWndButtonOk   = CreateWindow("BUTTON", "Підтвердити",
	                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
	                                           400, 160, 100, 25, hwnd, (HMENU)BUTTON_PSW_OK,
											   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
            if (!hWndButtonOk){
            	MessageBox(NULL, "ButtonooOk Failed.", "Error", MB_OK | MB_ICONERROR);
            }

        }
	break;
	case WM_PAINT: {// если нужно нарисовать, то:
		    	  HDC hDC; // создаём дескриптор ориентации текста на экране
		    	  RECT rect; // стр-ра, определяющая размер клиентской области
		    	  PAINTSTRUCT ps; // структура, сод-щая информацию о клиентской области (размеры, цвет и тп)
		    	  hDC = BeginPaint(hwnd, &ps); 	// инициализируем контекст устройства
		    	  GetClientRect(hwnd, &rect);  	// получаем ширину и высоту области для рисования
		    	  RECT  rectp ={100, 50, 213, 75};
		    	  DrawText(hDC, "Введіть пароль:", 16, &rectp, DT_SINGLELINE|DT_LEFT|DT_TOP);
		    	  EndPaint(hwnd, &ps);
	}
	break;
	case WM_COMMAND:{

		if(LOWORD(wParam)==BUTTON_PSW_CNSL){
			DestroyWindow(hwnd);
		}
		else if	(LOWORD(wParam)==BUTTON_PSW_OK){
			float2_t float2psw = {0};
			int psw = 0;
			int err = f_checkEdit(IDE_PASSWORDEDIT, hwnd, &float2psw);
		    if( err!=0){f_valueEditMessageBox(err);}
		    else{psw = float2psw.d;}
			SendMessage(
		    				WndCalledPsw,
							VK_PSW,
							psw,
							0
		    				);
			DestroyWindow(hwnd);
		}
		else{}

	}
	break;
	case WM_CLOSE:{
				 DestroyWindow(hwnd);
	}
	break;
	case WM_DESTROY:
				PostQuitMessage(0);
	break;
	default:{
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	}
	return 0;

}


