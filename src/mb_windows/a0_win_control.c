#include "a0_win_control.h"

#include <commctrl.h>
#include <process.h>
#include <stddef.h>
#include <winbase.h>
#include <windef.h>
#include <winnt.h>
#include <winuser.h>

#include "a2_setting.h"
#include "a2_mb_monitoring.h"
#include "a2_Tx_msg.h"
#include "a2_mb_registers.h"
#include "a2_xIP320.h"

#include "a3_monitoring.h"
#include "tk5/a20_tk5.h"

#include "tk2/a20_tk2.h"
#include "../mb_devs/tk4_dist.h"
#include "e1_passwordEdit.h"



/* HINSTANCE - это дескриптор модуля: dll, exe и т.п.
Например, когда вы хотите загрузить ресурсы или создать диалоговое окно,
то указываете дескриптор модуля как аргумент функции, чтобы система
знала, в каком модуле эти ресурсы искать
У нас це дескриптор проекту                                           */
HINSTANCE wcPrevInstance = NULL;
HINSTANCE wchInstance = NULL;
/* лобальні параметри управління скролами*/
static int iVscrollPos =0;
static int screen_heigh =0;
static int iMDIHeight=0;
static const char g_WC_ClassName0[] = "WC_WndClass";

#ifndef		TK2_DISTANCE_WND
/* Точка входу в Windows-інтерфейс, створення основного вікна */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	wcPrevInstance=hPrevInstance;
	wchInstance =hInstance;
//    if (po.n==0){
//    	MessageBox(NULL, "Не має жодного відкритого СОМ-порту, ймовірно Модбас до комп'тера не підключено ",
//			             "Помилка", MB_OK | MB_ICONERROR
//						 );
////    	return 0;
//    }
	WNDCLASSEX wc={0};
	MSG Msg;

//	InitCommonControls();

	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.style		 = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc	 = WndProc0;
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
//	wc.hbrBackground =
//			(HBRUSH)3;
////			(COLOR_3DFACE+1);
//	wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MAINMENU);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = g_WC_ClassName0;
//	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION);


	if(!RegisterClassEx(&wc)){
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	/* Пригодиться :) */
//	int width      = GetSystemMetrics(SM_CXSCREEN);
	screen_heigh   = GetSystemMetrics(SM_CYSCREEN);
	HWND hwndm=NULL;
    hwndm = CreateWindowEx (
    		0,
			g_WC_ClassName0,
    		TEXT ("Модбас-сервіс від 'Техноконт'"),
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN |WS_VSCROLL | WS_HSCROLL,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, NULL, hInstance, NULL
			);

	if(hwndm == NULL){
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	/* Реєстрація класів вікон, що відкриваються багатократно
	 * різними батьківськими вікнами */
	f_RegisterPPB_TxRxWndClass(hInstance);


	f_RegisterMonitoringWndClass(hInstance);
	f_RegisterTxWndClass(hInstance);
	f_RegisterRgWndClass(hInstance);
//	f_RegisterTK412WndClass(hInstance);

	f_Register_PasswordWndClass(hInstance);
	f_RegisterTK112WndClass(hInstance);
	f_RegisterSWndClass(hInstance);

	f_RegisterTK5WndClass(hInstance);


	HMENU hMenu;
	hMenu = CreateMenu();
	AppendMenu(hMenu, MF_STRING /* | */, ID_MBSETTING, 	"&Налаштування");
//	AppendMenu(hMenu, MF_STRING /* | */, ID_MBSTART, 	"&Старт");
//	AppendMenu(hMenu, MF_STRING /* | */, ID_MBPOUSE, 	"&Пауза");
//	AppendMenu(hMenu, MF_STRING /* | */, ID_MBRESTART, 	"&Продовжити");
//	AppendMenu(hMenu, MF_STRING /* | */, ID_MBSTOP, 	"&Стoп");
	AppendMenu(hMenu, MF_STRING /* | */, ID_MONITORINGW,"&Моніторинг");
	AppendMenu(hMenu, MF_STRING /* | */, ID_MS_SEND_MSG,"&Відправити_повідомлення");
	AppendMenu(hMenu, MF_STRING /* | */, ID_MS_SEND_RG," &Читати/писати регістр");
	AppendMenu(hMenu, MF_STRING /* | */, ID_MS_TK112,   "&TK2_Дист.");
//	AppendMenu(hMenu, MF_STRING /* | */, ID_MS_TK412,   "&TK4_Дист.");
	AppendMenu(hMenu, MF_STRING /* | */, ID_MS_TK5,   "&TK5_Дист.");
//	AppendMenu(hMenu, MF_STRING /* | */, ID_MS_TK412Light,"&TK_412_Дист.упр.спр.");
//	AppendMenu(hMenu, MF_STRING /* | */, ID_MS_TK412uSet,"&TK_412_Налаштування користувача.");
//	AppendMenu(hMenu, MF_STRING /* | */, ID_MS_TK412f1Set,"&TK_412_Налаштування виробника1");
//	AppendMenu(hMenu, MF_STRING /* | */, ID_MS_TK412f2Set,"&TK_412_Налаштування виробника2");
//	AppendMenu(hMenu, MF_STRING /* | */, ID_IP320PLAYER,"&IP320");
//	AppendMenu(hMenu, MF_STRING /* | */, ID_CTR_ONE, "&CTR_ONE");
	SetMenu(hwndm, hMenu);

	ShowWindow(hwndm, nCmdShow);
	UpdateWindow(hwndm);

	while(GetMessage(&Msg, NULL, 0, 0) > 0){
					TranslateMessage(&Msg);
					DispatchMessage(&Msg);
	}
	return Msg.wParam;
}
#endif	//TK2_DISTANCE_WND

/* Функція обробки повідомлень основного вікна                           */
LRESULT CALLBACK WndProc0(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){

	/* Унікальний ідентифікатор вікна налаштувань */

	static PAINTSTRUCT ps={0}; // структура, сод-щая информацию о клиентской области (размеры, цвет и тп)
	switch(msg){
	    /**************************************************************************************************************
	     *      CREATE 																							CREATE
	     **************************************************************************************************************/
		case WM_CREATE:{
			/* Додаємо до вікна Toolbar, Statusbar i стрічки прокрутки*/
			HWND hTool;
			hTool = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
				hwnd, (HMENU)IDC_MAIN_TOOL, GetModuleHandle(NULL), NULL);
			if(hTool == NULL)
				MessageBox(hwnd, "Could not create tool bar.", "Error", MB_OK | MB_ICONERROR);

			// Send the TB_BUTTONSTRUCTSIZE message, which is required for
			// backward compatibility.
			SendMessage(hTool, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
			

			// Create Status bar
			//http://radiofront.narod.ru/htm/prog/htm/winda/api/status1.html
			HWND hStatus =
			 CreateWindowEx(0, STATUSCLASSNAME, NULL,
				WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
				CW_USEDEFAULT, CW_USEDEFAULT,
	            CW_USEDEFAULT, CW_USEDEFAULT,
				hwnd, (HMENU)IDC_MAIN_STATUS, GetModuleHandle(NULL), NULL);
			RECT rectStatus={0};
			GetClientRect(hwnd, &rectStatus);
			int width = rectStatus.right- rectStatus.left;
			int ar[2] = {700, width-700};
			SendMessage(hStatus, SB_SETPARTS, 2, (LPARAM)ar);
			SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"Hi there :)");
		}
		break;

		/**************************************************************************************************************
	    *      CОММAND 																						    COMMAND
		**************************************************************************************************************/
		case WM_COMMAND:
			/* Обробка команд меню  */
			switch(LOWORD(wParam)){
			case ID_MBSETTING:
					f_CreateSWnd(hwnd);
			break;
				case ID_MONITORINGW:{
					f_WinMonitoring(hwnd);
				}
				break;
				case ID_MS_SEND_MSG:
					f_CreateTxWnd(hwnd);
				break;
				case ID_MS_SEND_RG:
					f_CreateRgWnd(hwnd);
				break;
#ifndef		TK2_DISTANCE_WND
				case ID_MS_TK112:{
//					f_112dist_Init();
				    f_CreateTK112Wnd(hwnd, wchInstance);
				}
#endif
				break;
//				case ID_MS_TK412:{
//					f_412dist_Init();
//				    f_CreateTK412Wnd(hwnd);
//				}
//				break;
				case ID_MS_TK5:
					f_CreateTK5Wnd(hwnd);
				break;
				break;
//				case ID_IP320PLAYER:
//					f_Create320Wnd(hwnd);
//				break;

				default:{}
			}
		break;
		/**************************************************************************************************************
	    *      PAINT 																						      PAINT
		**************************************************************************************************************/
		case WM_PAINT: {// если нужно нарисовать, то:
			HDC hDC = NULL;
			hDC = BeginPaint(hwnd, &ps);
			/* перерисовка вмісту вікна після прокрутки*/
//        	hDC= GetDC(hWnd);
			RECT rect;
        	GetClientRect(hwnd, &rect);  	// получаем ширину и высоту области для рисования
        	FillRect(hDC, &rect, (HBRUSH)(COLOR_WINDOW+1));

//
//        	for (int i = 0; i<0x100;i++){
////        		for (int j = 0; i<0x100;i++){
//        			RECT rectblueHeating4 = {rect.left+20,rect.top+20+i*4,rect.left+20+200,rect.top+20+i*4+4};
//        			FillRect(hDC, &rectblueHeating4, (HBRUSH)(i));
//        			Sleep(50);
////        		}
//        	}
//			RECT rectblueHeating4 = {rect.left+20,rect.top+300,rect.left+20+200,rect.top+300+100};
//			FillRect(hDC, &rectblueHeating4, (HBRUSH)(14));
//
//
//


    		f_paint_modbuss_msg(iVscrollPos);
    		ReleaseDC(hwnd, hDC);

    		EndPaint(hwnd, &ps);
		}
		break;

		case WM_SIZE:{

			HWND hTool;
			RECT rcTool;
			int iToolHeight;

			HWND hStatus;
			RECT rcStatus;
			int iStatusHeight;

			HWND hMDI;

			RECT rcClient;

			// Size toolbar and get height
			hTool = GetDlgItem(hwnd, IDC_MAIN_TOOL);
			SendMessage(hTool, TB_AUTOSIZE, 0, 0);
			GetWindowRect(hTool, &rcTool);
			iToolHeight = rcTool.bottom - rcTool.top;

			// Size status bar and get height
			hStatus = GetDlgItem(hwnd, IDC_MAIN_STATUS);
			SendMessage(hStatus, WM_SIZE, 0, 0);
			GetWindowRect(hStatus, &rcStatus);
			iStatusHeight = rcStatus.bottom - rcStatus.top;

			// Calculate remaining height and size edit
			GetClientRect(hwnd, &rcClient);
			iMDIHeight = rcClient.bottom - iToolHeight - iStatusHeight;
			hMDI = GetDlgItem(hwnd, IDC_MAIN_MDI);
			SetWindowPos(hMDI, NULL, 0, iToolHeight, rcClient.right, iMDIHeight, SWP_NOZORDER);
		}
		break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
		break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);

	}
	return 0;
}


//HANDLE hIcon = LoadImage(0, _T("imagepath/image.ico"), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
//if (hIcon) {
//    //Change both icons to the same icon handle.
//    SendMessage(hwnd, WM_SETICON, ICON_SMALL, hIcon);
//    SendMessage(hwnd, WM_SETICON, ICON_BIG, hIcon);
//
//    //This will ensure that the application icon gets changed too.
//    SendMessage(GetWindow(hwnd, GW_OWNER), WM_SETICON, ICON_SMALL, hIcon);
//    SendMessage(GetWindow(hwnd, GW_OWNER), WM_SETICON, ICON_BIG, hIcon);
//}


