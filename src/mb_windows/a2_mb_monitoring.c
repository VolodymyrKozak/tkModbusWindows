/*
 * a2_mb_monitoring.c
 *
 *  Created on: 10 ����. 2019 �.
 *      Author: KozakVF
 */


#include "a2_mb_monitoring.h"

#include <commctrl.h>
#include <process.h>
#include <stddef.h>
#include <winbase.h>
#include <windef.h>
#include <winnt.h>
#include <winuser.h>

#include "a3_monitoring.h"
//#include "a2_Tx_msg.h"
//#include "tk4/a2_TK412.h"
#include "a2_setting.h"

/* HINSTANCE - ��� ���������� ������: dll, exe � �.�.
��������, ����� �� ������ ��������� ������� ��� ������� ���������� ����,
�� ���������� ���������� ������ ��� �������� �������, ����� �������
�����, � ����� ������ ��� ������� ������
� ��� �� ���������� �������                                           */
HINSTANCE hinst;

/* �������� �������������� ��������� ���� � ����� ������� ��������� ����,
 * ���������������� � ���� 'a3_monitoring.c'                         */
HWND hwndMain=NULL;
HWND hStatus=NULL;
/* ������� ��������� ��������� ��������*/
int iVscrollPos =0;
int screen_heigh =0;

mb_mon_t mb_mon = mb_mon_presetting;

static int iMDIHeight=0;
char g_MnClassName[] = "MnWndClass";

LPCSTR f_RegisterMonitoringWndClass(HINSTANCE hinst){
	WNDCLASSEX wc={0};
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.style		 = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc	 = f_WndProcMonitoring;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hinst;
	wc.hIcon		 =
//	LoadIcon(NULL, IDI_APPLICATION);
	CreateIcon(
			hinst,       // application instance
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
	wc.lpszClassName = g_MnClassName;
//	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION);


	if(!RegisterClassEx(&wc)){
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	return wc.lpszClassName;
}
/* ���������  ���� ���������� */
int WINAPI f_WinMonitoring(HWND hwnd){
	MSG Msg;
	/* ����������� :) */
	int width      = GetSystemMetrics(SM_CXSCREEN);
	screen_heigh   = GetSystemMetrics(SM_CYSCREEN);
	HWND hwndm=NULL;
    hwndm = CreateWindowEx (
    		0,
			g_MnClassName,
    		TEXT ("��������� ����� ������ �� '���������'"),
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN |WS_VSCROLL | WS_HSCROLL,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, NULL, hinst, NULL
			);
    int Err = GetLastError();
	if(hwndm == NULL){
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	hwndMain = hwndm;

	HMENU hMenu;
	hMenu = CreateMenu();
	AppendMenu(hMenu, MF_STRING /* | */, ID_MBSETTING, 	"&������������");
	AppendMenu(hMenu, MF_STRING /* | */, ID_MBSTART, 	"&�����");
	AppendMenu(hMenu, MF_STRING /* | */, ID_MBPOUSE, 	"&�����");
	AppendMenu(hMenu, MF_STRING /* | */, ID_MBRESTART, 	"&����������");
	AppendMenu(hMenu, MF_STRING /* | */, ID_MBSTOP, 	"&��o�");
//	AppendMenu(hMenu, MF_STRING /* | */, ID_MS_SEND_MSG,"&³��������_�����������");
//	AppendMenu(hMenu, MF_STRING /* | */, ID_MS_SEND_RG," &������� ��");
//	AppendMenu(hMenu, MF_STRING /* | */, ID_MS_TK412,   "&TK_412_����.���.");
//	AppendMenu(hMenu, MF_STRING /* | */, ID_MS_TK412Light,"&TK_412_����.���.���.");
//	AppendMenu(hMenu, MF_STRING /* | */, ID_MS_TK412uSet,"&TK_412_������������ �����������.");
//	AppendMenu(hMenu, MF_STRING /* | */, ID_MS_TK412f1Set,"&TK_412_������������ ���������1");
//	AppendMenu(hMenu, MF_STRING /* | */, ID_MS_TK412f2Set,"&TK_412_������������ ���������2");
//	AppendMenu(hMenu, MF_STRING /* | */, ID_IP320PLAYER,"&IP320_Player");
//	AppendMenu(hMenu, MF_STRING /* | */, ID_CTR_ONE, "&CTR_ONE");
	SetMenu(hwndMain, hMenu);

	ShowWindow(hwndm, SW_SHOWMAXIMIZED);
	UpdateWindow(hwndm);

	while(GetMessage(&Msg, NULL, 0, 0) > 0){
					TranslateMessage(&Msg);
					DispatchMessage(&Msg);
	}
	return Msg.wParam;
}

/* ������� ������� ���������� ��������� ����                           */
LRESULT CALLBACK f_WndProcMonitoring(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){

	/* ��������� ������������� ���� ����������� */

	static PAINTSTRUCT ps={0}; // ���������, ���-��� ���������� � ���������� ������� (�������, ���� � ��)
	switch(msg){
	    /**************************************************************************************************************
	     *      CREATE 																							CREATE
	     **************************************************************************************************************/
		case WM_CREATE:{
			/* ������ �� ���� Toolbar, Statusbar i ������ ���������*/
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
			hStatus =
			 CreateWindowEx(0, STATUSCLASSNAME, NULL,
				WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
				CW_USEDEFAULT, CW_USEDEFAULT,
	            CW_USEDEFAULT, CW_USEDEFAULT,
				hwnd, (HMENU)IDC_MAIN_STATUS, GetModuleHandle(NULL), NULL);
//			2. ���������� ������ �� ������. ��� ���������� ������ ��������� �� ������, ����� ������� ������ � ��������� ������������:
//			int pParts[3];

//			cx=LOWORD(lParam);
//
//			pParts[0]=cx-200;
//			pParts[1]=cx-100;
//			pParts[2]=cx;
//
//			����� �� �������� ��������� SB_SETPARTS � ��������� ���������� ������������ � ������� �� ����.
//			SendMessage(hStatus, SB_SETPARTS, 3, (LPARAM)pParts);
			//WORD cx=LOWORD(lParam);
			RECT rectStatus={0};
			GetClientRect(hwnd, &rectStatus);
			int width = rectStatus.right- rectStatus.left;
			int ar[2] = {700, width-700};
			SendMessage(hStatus, SB_SETPARTS, 2, (LPARAM)ar);
			SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"Hi there :)");

			int numlines=f_tm();
			SetScrollRange(hwnd,SB_VERT,0,numlines-1,TRUE);
			SetScrollPos (hwnd, SB_VERT, iVscrollPos , TRUE);
		}
		break;
		/* ������� ����������� ������ ���������
		 * �� ������� � ������  - Charles Petzold Programming Windows */
		case WM_VSCROLL:{
			if (LOWORD (wParam) < 5){
				screen_heigh++;
			}
			switch (LOWORD (wParam)){
				case SB_LINEUP:
					iVscrollPos -= 1 ;
				break ;
				case SB_LINEDOWN:
					iVscrollPos += 1 ;
				break ;
				case SB_PAGEUP:
					iVscrollPos -= Y_PASSPORT/Y_STRING;
				break ;
				case SB_PAGEDOWN:
					iVscrollPos += Y_PASSPORT/Y_STRING;
				break ;
				case SB_THUMBPOSITION:
					iVscrollPos = HIWORD (wParam) ;
				break ;
				default :
				break ;
			}
			int numlines=f_tm();
			iVscrollPos = max (0, min (iVscrollPos, numlines-1)) ;
			if (iVscrollPos != GetScrollPos (hwnd, SB_VERT)){
				SetScrollPos (hwnd, SB_VERT, iVscrollPos, TRUE) ;
				InvalidateRect (hwnd, NULL, TRUE) ;
			}
		}
			return 0 ;

		/**************************************************************************************************************
	    *      C���AND 																						    COMMAND
		**************************************************************************************************************/
		case WM_COMMAND:
			/* ������� ������ ����  */
			switch(LOWORD(wParam)){
				case ID_MBSTART:{
					if(mb_mon == mb_mon_presetting){
						MessageBox(hwnd, "��������� ��������� ����� ������/RS232/RS485. ����� ���� '������������'", "�����", MB_OK | MB_ICONINFORMATION);
					}
					else if ((mb_mon == mb_mon_setted)||(mb_mon =mb_mon_stopped)){
						mb_mon = mb_mon_starting;
						_beginthread((void*)f_main_paint, 0, NULL);
						//SendMessage(hwnd, WM_CREATE, 0, 0);
						mb_mon = mb_mon_started;
					}
					else if(mb_mon == mb_mon_started){
						MessageBox(hwnd, "��������� ������ ����� ��� �����������.\n�� ������ ���� �������� ('�����') ��� ��������� ('����')",
								         "�����", MB_OK | MB_ICONINFORMATION);
					}

					else{}
				}


				break;
				case ID_MBPOUSE:
					if(mb_mon == mb_mon_presetting){
						MessageBox(hwnd, "��������� ��������� ����� ������/RS232/RS485. \n����� ���� '������������'", "�����", MB_OK | MB_ICONINFORMATION);
					}
					else if(mb_mon == mb_mon_started){
						mb_mon =mb_mon_poused;
					}
					else{
						MessageBox(hwnd, "����������� ��������� ����� ���� ���� ���� ������. \n����� ���� '�����' ��� '����������'", "�����", MB_OK | MB_ICONINFORMATION);
					}
				break;
				case ID_MBRESTART:
					if(mb_mon == mb_mon_presetting){
						MessageBox(hwnd, "��������� ��������� ����� ������/RS232/RS485. \n����� ���� '������������'", "�����", MB_OK | MB_ICONINFORMATION);
					}
					else if(mb_mon == mb_mon_started){
						MessageBox(hwnd, "��������� ������ ����� ��� �����������.\n�� ������ ���� �������� ('�����') ��� ��������� ('����')",
								         "�����", MB_OK | MB_ICONINFORMATION);
					}

					else if (mb_mon ==mb_mon_poused){
						mb_mon =mb_mon_started;
					}
					else{
						MessageBox(hwnd, "³������� ��������� ����� ���� ���� ���� ����������. \n����� ���� '�����'", "�����", MB_OK | MB_ICONINFORMATION);
					}
				break;
				case ID_MBSTOP:
					if(mb_mon == mb_mon_presetting){
						MessageBox(hwnd, "��������� ��������� ����� ������/RS232/RS485. \n����� ���� '������������'", "�����", MB_OK | MB_ICONINFORMATION);
					}
					else if((mb_mon == mb_mon_started)||
							(mb_mon ==mb_mon_poused)){
						mb_mon =mb_mon_stopped;
					}
					else{
						MessageBox(hwnd, "���� �� ��������� ��������� �������� ����. \n����� ���� '�����'", "�����", MB_OK | MB_ICONINFORMATION);
					}

				break;
				case ID_MBSETTING:{
					if(
					   (mb_mon == mb_mon_presetting)||
					   (mb_mon == mb_mon_setted)
					   ){
						f_CreateSWnd(hwnd);
					}
					else if(mb_mon == mb_mon_started){
						MessageBox(hwnd, "��������� ������ ����� ��� �����������.\n�� ������ ���� �������� ('�����') ��� ��������� ('����')",
								         "�����", MB_OK | MB_ICONINFORMATION);
					}
					else{
					}
				}
				break;

//				case ID_MS_SEND_MSG:{
//					if(
//							(mb_mon == mb_mon_setted)    ||
//							(mb_mon == mb_mon_stopped)
//					   ){
////						HWND hwndTx = NULL;
////						hwndTx =
//						        f_Create_TxWnd(hwnd);
//								mb_mon = mb_manual_dialog;
//					}
//					else if(mb_mon == mb_mon_started){
//						MessageBox(hwnd, "����������� ��������� ������ �����.\n��� �������� ����������� ��������� ���������('����')",
//								         "�����", MB_OK | MB_ICONINFORMATION);
//					}
//					else if(mb_mon == mb_mon_presetting){
//						MessageBox(hwnd, "��������� ��������� ����� ������/RS232/RS485. \n����� ���� '������������'",
//								         "�����", MB_OK | MB_ICONINFORMATION);
//					}
//					else{}
//				}
//				break;
//				case ID_MS_SEND_RG:{
//					f_CreateRgWnd(hwnd);
//				}
//				break;
//				case ID_MS_TK412:{
//					if(
//							(mb_mon == mb_mon_setted)    ||
//							(mb_mon == mb_mon_stopped)
//					   ){
////						HWND hwndTx = NULL;
////						hwndTx =
//						    f_412dist_Init();
//							f_CreateTK412Wnd(hwnd);
//							mb_mon = mb_tk412;
//					}
//					else if(mb_mon == mb_manual_dialog){
//						MessageBox(hwnd, "����������� ������ ����� ������\n��� ������������� ������������ �� ��������� ��412 ������ � � ������� ������",
//								         "�����", MB_OK | MB_ICONINFORMATION);
//					}
//					else if(mb_mon == mb_mon_started){
//						MessageBox(hwnd, "����������� ��������� ������ �����.\n��� �������� ����������� ��������� ���������('����')",
//								         "�����", MB_OK | MB_ICONINFORMATION);
//					}
//					else if(mb_mon == mb_mon_presetting){
//						MessageBox(hwnd, "��������� ��������� ����� ������/RS232/RS485. \n����� ���� '������������'",
//								         "�����", MB_OK | MB_ICONINFORMATION);
//					}
//					else{}
//				}
//				break;
//
//				case ID_MS_TK412Light:
//					f_412dist_Init();
//					f_CreateTK412LightWnd(hwnd);
//					break;
//				case ID_MS_TK412uSet:
//					f_CreateTK412usWnd(hwnd);
//					break;
//				case ID_MS_TK412f1Set:
//					f_CreateTK412fs1Wnd(hwnd);
//					break;
//				case ID_MS_TK412f2Set:
//					 f_CreateTK412fs2Wnd(hwnd);
//					break;
//
//
//				case ID_IP320PLAYER:{
//					if(
//							(mb_mon == mb_mon_setted)    ||
//							(mb_mon == mb_mon_stopped)
//						){
//				//						HWND hwndTx = NULL;
//				//						hwndTx =
//						f_Create320Wnd(hwnd);
//						mb_mon = mb_manual_dialog;
//					}
//					else if(mb_mon == mb_mon_started){
//										MessageBox(hwnd, "����������� ��������� ������ �����.\n��� ������ IP320_Player ��������� ���������('����')",
//												         "�����", MB_OK | MB_ICONINFORMATION);
//					}
//					else if(mb_mon == mb_mon_presetting){
//						MessageBox(hwnd, "��������� ��������� ����� ������/RS232/RS485. \n����� ���� '������������'",
//												         "�����", MB_OK | MB_ICONINFORMATION);
//					}
//					else{}
//				}
//				break;
				default:{}
			}
		break;
		/**************************************************************************************************************
	    *      PAINT 																						      PAINT
		**************************************************************************************************************/
		case WM_PAINT: {// ���� ����� ����������, ��:
			HDC hDC = NULL;
			hDC = BeginPaint(hwnd, &ps);
			/* ����������� ����� ���� ���� ���������*/
//        	hDC= GetDC(hWnd);
			RECT rect;
        	GetClientRect(hwnd, &rect);  	// �������� ������ � ������ ������� ��� ���������
        	FillRect(hDC, &rect, (HBRUSH)(COLOR_WINDOW+1));
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


