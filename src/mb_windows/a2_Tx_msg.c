
#include "a2_Tx_msg.h"
#include "a0_win_control.h"
#include <windows.h>
#include <process.h>    /* _beginthread, _endthread */

#include "../mb_funcs/tw_mb.h"
#include "../mb_funcs/tx_manual.h"
//#include "a3_monitoring.h"
#include "a3_winTxRx.h"
#define BUTTON_MBSEND 9678
//extern HWND childTxRx;
static HINSTANCE hinstTx=NULL;


/* ���� ��� �������� ���������� ������������ ������� � ����������� ���o*/

extern modbus_status_t BTH_TxTx_status;
extern modbus_master_tx_msg_t BTH_TxMsg;
extern uint32_t BTH_Msg_TimeOut;
extern modbus_master_rx_msg_t BTH_RxMsg;
extern char BTH_User_Msg[256];
static char user_msg[256]={0};
/* ���� ��� ���������� ������ � ����������� �������� */
HWND QPWTx_Wnd=NULL;
static ProcessState_t QPW_ProcessState=ProcessIddle;
static ProcessState_t QPW_ProcessStateOld=ProcessIddle;

static modbus_master_tx_msg_t mbTxMsg;
static modbus_master_rx_msg_t mbRxMsg;
const char g_szClassNameC1[] = "TxWndClass";


//static int winapiTxRx=0;

LPCSTR f_RegisterTxWndClass(HINSTANCE hinst){
	hinstTx=hinst;
	WNDCLASS w;
		memset(&w,0,sizeof(WNDCLASS));
		w.lpfnWndProc = WndProcTx;
		w.hInstance = hinst;
		w.style = CS_HREDRAW | CS_VREDRAW;
		w.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
		w.lpszClassName = g_szClassNameC1;
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

	if(!RegisterClassA(&w))
	{
//		int Error = GetLastError();
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
		return w.lpszClassName;
}
int WINAPI f_CreateTxWnd(HWND hh){

//	WNDCLASSEX wc;
	HWND childTx;

	MSG Msg;


	childTx = CreateWindowEx(
		0,
		g_szClassNameC1,
		"��������-������ ���������� ������",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1200, 320,
		NULL, NULL, hinstTx, NULL);
	QPWTx_Wnd=childTx;
	if(childTx == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
//	f_PPB_TxRxWndClass(childTx);
	ShowWindow(childTx, SW_NORMAL);
	UpdateWindow(childTx);

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}


LRESULT CALLBACK WndProcTx(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	/* ������������� ����, ���� ���������� ����������� ������ �
	 * ������������� ���� PPB ProcessBar                                 */
	if((QPW_ProcessState==ProcessIddle)&&(QPW_ProcessStateOld==ProcessIddle)){
		  UpdateWindow(hwnd);
		  QPW_ProcessStateOld=ProcessBusy;
	}

	static HWND hEdit;
	static HWND  hButtonSend;
	static uint8_t  mbMSG0[768]="0F 11";
	static uint16_t n_mbMSG0=0;

	/* ������������� ����, ���� ���������� ����������� ������ �
	 * ������������� ���� PPB ProcessBar                                 */
	if((QPW_ProcessState==ProcessIddle)&&(QPW_ProcessStateOld==ProcessIddle)){
		  UpdateWindow(hwnd);
		  QPW_ProcessStateOld=ProcessBusy;
	}

	switch(msg)
	{
		case WM_CREATE:
		{   //SetDlgItemText(hwnd, IDC_TEXT, "This is a string");


//			HDC hDCTx = NULL;
//			PAINTSTRUCT psTx; // ���������, ���-��� ���������� � ���������� ������� (�������, ���� � ��)
//			hDCTx = BeginPaint(hwnd, &psTx);
//			RECT rectTx={0};
//			GetClientRect(hwnd, &rectTx);  	// �������� ������ � ������ ������� ��� ���������
//			DrawText(hDCTx, TEXT("Modbus ����������� ��� ��������"), 12, &rectTx, DT_SINGLELINE|DT_LEFT|DT_TOP);
//			EndPaint(hwnd, &psTx);
			HFONT hfDefault;

			hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", (char*)mbMSG0,
				WS_CHILD | ES_MULTILINE | WS_VISIBLE | WS_BORDER//| WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL
				,
				0,0,0,0,
//				100, 100, 1000, 100,

				hwnd, (HMENU)IDC_MAIN_EDIT, GetModuleHandle(NULL), NULL);
			if(hEdit == NULL)
				MessageBox(hwnd, "Could not create edit box.", "Error", MB_OK | MB_ICONERROR);

//			hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			SendMessage(hEdit, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
            hButtonSend = CreateWindow("BUTTON", "³��������",
                                           WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_PUSHBUTTON,
                                           0, 0, 0, 0, hwnd, (HMENU)BUTTON_MBSEND,
										   (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
		}
		break;

		case WM_PAINT: {// ���� ����� ����������, ��:

	    	  HDC hDC; // ������ ���������� ���������� ������ �� ������

//	    	  typedef struct tagPAINTSTRUCT {
//	    	    HDC  hdc;
//	    	    BOOL fErase;
//	    	    RECT rcPaint;
//	    	    BOOL fRestore;
//	    	    BOOL fIncUpdate;
//	    	    BYTE rgbReserved[32];
//	    	  } PAINTSTRUCT, *PPAINTSTRUCT;
//	    	  hdc  		���������� DC ���������� �����������, ������� ������������ ��� ����, ����� �������.
//			  fErase 	����������, ������ �� ��� ���� �����. ��� �������� - �� ����, ���� ���������� ������ ������� ���.
//	    	  ���������� ������������ �� �������� ����, ���� ����� ���� ��������� ��� ����� ��� ����.
//			  �������������� ���������� ������ � ������ � ��������� ����� hbrBackground ��������� WNDCLASS.
//			  rcPaint   ���������� ��������� RECT, ������� ������������� ����� ������� � ������ ������ ���� ��������������, � ������� ������������� �����������, � �������� ���������� ������������ ������ �������� ���� ������� �������.
	    	  RECT rect; // ���-��, ������������ ������ ���������� �������
	    	  PAINTSTRUCT ps; // ���������, ���-��� ���������� � ���������� ������� (�������, ���� � ��)

//	    	  ps.fErase = 0;
//	    	  ps.rcPaint=rect;
	    	  //(HBRUSH)(COLOR_3DFACE+1);
//	    	  COLORREF colorText = RGB(255, 255, 0); // ����� ���� ������
//	    	  CHAR db_str[20]={"COM-port"};


	    	  hDC = BeginPaint(hwnd, &ps); 	// �������������� �������� ����������
	    	  GetClientRect(hwnd, &rect);  	// �������� ������ � ������ ������� ��� ���������
//	    	  FillRect(hDC, &rect, (HBRUSH)(COLOR_WINDOW+1));
	    	  RECT rect1 =rect;

	    	  rect1.top   =rect.top+20;
//	    	  rect1.bottom   =rect1.top+75;
	    	  DrawText(hDC, "³���������� ����������� ��� ��������:", 40, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  rect1.top=rect1.top+20;
	    	  DrawText(hDC, "   ���� 0 -> ������ Modbus Slave       ", 40, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  rect1.top=rect1.top+20;
	    	  DrawText(hDC, "   ���� 1 -> ��� ������� Modbus        ", 40, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  rect1.top=rect1.top+20;
	    	  DrawText(hDC, "   CRC �������� �����������           ", 40, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
	    	  rect1.top=rect1.top+50;
	    	  DrawText(hDC, user_msg,                                  40, &rect1, DT_SINGLELINE|DT_LEFT|DT_TOP);
//	    	  int DrawText(
//	    	    HDC hDC,          // ���������� ��������� ����������
//	    	    LPCTSTR lpString, // ����� ��� ������
//	    	    int nCount,       // ����� ������
//	    	    LPRECT lpRect,    // ������� ���� ��������������
//	    	    UINT uFormat      // ��������� ������ ������
//	    	  );
	    	EndPaint(hwnd, &ps);
		}

		break;
		case WM_SIZE:
		{
			RECT rcClient;

			GetClientRect(hwnd, &rcClient);
			hEdit = GetDlgItem(hwnd, IDC_MAIN_EDIT);
			SetWindowPos(hEdit, NULL,       0,       100,           rcClient.right-110, 25, SWP_NOZORDER);
			SetWindowPos(hButtonSend, NULL, rcClient.right-110, 100, 100,               25, SWP_NOZORDER);


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
			if(QPW_ProcessState==ProcessBusy){
				break;
			}
			switch(wParam) {
				case BUTTON_MBSEND:{
					memset(user_msg,0,sizeof(user_msg));
					memset(mbMSG0, 0, sizeof(mbMSG0));
					n_mbMSG0 = GetWindowTextLength(GetDlgItem(hwnd, IDC_MAIN_EDIT));
					if(n_mbMSG0==0){
						snprintf (user_msg,40,"³����� ����������� ��� ��������");
						break;
					}
					GetDlgItemText(hwnd, IDC_MAIN_EDIT, (char*)mbMSG0, n_mbMSG0+1);
					/* ����������� ��� CRC � �������� ���� ASCII �������������� � ����������� HEX ��� CRC */
					lt_status_t ltt = MB_TxRxManual(
							mbMSG0,
							n_mbMSG0+1,
							&mbTxMsg
							);


					switch(ltt){
						case lt_notHEX:   {
							snprintf (user_msg,40,"�� �� ������� �� ���");
							break;
						}
						case lt_byteWrong:{
							snprintf (user_msg,40,"�������� ����� �� �������");
							break;
						}
						case lt_OK:       {
							/* ����������� ��� CRC */
							BTH_Msg_TimeOut=300;
							BTH_TxMsg = mbTxMsg;
							QPW_ProcessState=ProcessBusy;
							QPW_ProcessStateOld=ProcessIddle;
							_beginthread((void*)f_BTH_TxRx, 0, NULL);
							f_PPB_TxRxWnd(hwnd);
					break;
						}
						default:{}
					}

				}
				break;
				//https://docs.microsoft.com/en-us/previous-versions/windows/desktop/inputmsg/wm-parentnotify

			    default:{}
			}
		}
		break;
//		case WM_PARENTNOTIFY:{
//			//https://docs.microsoft.com/en-us/previous-versions/windows/desktop/inputmsg/wm-parentnotify
//			int uu = GetDlgCtrlID(childTxRx);
//			if(uu!=0){
//				int uu = GetDlgCtrlID(childTxRx);
//			}
//			if(LOWORD(wParam)==WM_DESTROY){
//				int uu = GetDlgCtrlID(childTxRx);
//				if(LOWORD(lParam)==uu){
//
////				The child window is being destroyed.
////				HIWORD(wParam) is the identifier of the child window.
////				lParam is a handle to the child window.
//					f_a2TxAnswer();
//
//				}
//			}
//		}
//		break;
		case WM_CLOSE:
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







/* ����/����������� �� ������������ ������������ �������,
 * ����������� � ���� PPB ������������ ������� */
void f_QPWD_TxAnswer(void){

//	modbus_status_t BTH_TxTx_status = MODBUS_UNKNOWN_ERROR;
//	modbus_master_tx_msg_t BTH_TxMsg={0};
//	modbus_master_rx_msg_t BTH_RxMsg={0};
//	char BTH_User_Msg[256]={0};
	memset(user_msg, 0,sizeof(user_msg));
	/* ���� ������� �������� �� ���� */
	if(BTH_TxTx_status==MODBUS_SUCCESS){
		mbRxMsg=BTH_RxMsg;
		f_mbSPrint(mbRxMsg.msg, mbRxMsg.length, (uint8_t*)user_msg);
	}
	/* Z��� ���� �������� ������� ������ RTC  */
	else{
		memcpy(&user_msg,BTH_User_Msg,sizeof(user_msg));
	}

	HDC hDCr; // ������ ���������� ���������� ������ �� ������
	RECT rect; // ���-��, ������������ ������ ���������� �������
	/*������� ������*/
	hDCr= GetDC(QPWTx_Wnd);
	GetClientRect(QPWTx_Wnd, &rect);  	// �������� ������ � ������ ������� ��� ���������
	RECT rect_res=rect;

	rect_res.top+=130;
	FillRect(hDCr, &rect_res, (HBRUSH)(COLOR_WINDOW+1));
	DrawText(hDCr, user_msg, 140, &rect_res, DT_SINGLELINE|DT_LEFT|DT_TOP);

	ReleaseDC(QPWTx_Wnd, hDCr);
	QPW_ProcessState=ProcessIddle;

}

/*
 * =====================================================================================================
 * ���������� ������� �������	 	³���           	����                         ������� ����������
 * ����.����� TxRxModbus			ProcessBar			� ������������ ���		 ����
 * 														���������� ������
 * 														���������� �������
 * 														�������
 * =====================================================================================================
 * 		BTH							PPB					QPWD							 QPW
 * =====================================================================================================
 * 																					 QPW_ProcessBusy_Flag
 * 	fBTH      <----------------------------------------------------------------------_beginthread (fBTH)
 *  BTH_ProcessBusy_Flag
 *                                  PPB_Wnd          <-------------------------------------Create PPB_Wnd
 *
 *                                  PPB_Busy_Flag
 *
 *      ...                            ...
 *
 *  BTH_ProcessIddle_Flag---------->PPB_Iddle_Flag
	_endthread()                        ���
 *                     PPB_TimeOut->PPB_Iddle_Flag    --->fQPW_Draw
 *                                                        QPW_ProcessIddly_Flag
 *                     				PPB_DESTROY		  ------------------------------->UpdateWindow(QPW);

=======================================================================================================
 *
 *
 *
 * */
