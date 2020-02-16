/*
 * a3_winTxRx.c

 *
 *  Created on: 7 груд. 2019 р.
 *      Author: KozakVF
 */
#include "string.h"
#include <windows.h>
#include <commctrl.h>
#include <stdbool.h>
#include <winuser.h>
#include <windef.h>
#include <wingdi.h>
#include <process.h>    /* _beginthread, _endthread */
#include "a0_win_control.h"
#include "a3_winTxRx.h"

const char g_szClassNameTxRx[] = "g_szClassNameTxRx";
extern HINSTANCE hinst;



static int PB_position=0;
extern int cport_nr;

/* Змінні для передачі результатів паралельного процесу в материнське вікно */
modbus_status_t BTH_TxTx_status = MODBUS_UNKNOWN_ERROR;
modbus_master_tx_msg_t BTH_TxMsg={0};
uint32_t BTH_Msg_TimeOut = 0;
modbus_master_rx_msg_t BTH_RxMsg={0};
char BTH_User_Msg[256]={0};

extern HWND QPWTx_Wnd;
extern HWND QPWRg_Wnd;
extern HWND TK2QPWRg_Wnd;

static HWND hwnd_called_from = NULL;
ProcessState_t BTH_Process=ProcessIddle;
ProcessState_t BTH_ProcessOld=ProcessIddle;
LRESULT CALLBACK PPB_TxRxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	static HWND hWndPB_RxTx;
	static HWND hwndTimer10;
	static HWND hwndTimer5000;


	switch(msg){
		/**************************************************************************************************************
		 *      CREATE 																						     CREATE
		 **************************************************************************************************************/
		case WM_CREATE:{


			/* Безумовний прогрес-бар  */
			hWndPB_RxTx = CreateWindowEx(
				0,
				PROGRESS_CLASS,
				(LPSTR)NULL,
				WS_VISIBLE | WS_CHILD /*| PBS_VERTICAL*/,
				0,
				0,
				150,
				10,
				hwnd,
				(HMENU)IDPB_TXRX ,
				(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
				NULL
				);

			if (!hWndPB_RxTx){
				MessageBox(NULL, "Progress Bar Failed.", "Error", MB_OK | MB_ICONERROR);
			}
			/* Тривалість сеансу у 10-м.сек інтервалах*/
			uint32_t timer = /*msg_timeout+3*mbTxMsg.length*/ 5000;
			/* Шкала 120% - задаємо в процентах */
			SendMessage(hWndPB_RxTx, PBM_SETRANGE, 0, MAKELPARAM(
				0, //minimum range value
				timer  //maximum range value
				));

			SendMessage(hWndPB_RxTx, PBM_SETPOS,
				0, //Signed integer that becomes the new position.
				0  //Must be zero
			);

//			SetTimer(
//				hwnd,               // handle to main window
//				IDT_TIMER_TXRX_5000MS,     // timer identifier
//			    5000,               // 5000ms
//			    (TIMERPROC) NULL    // no timer callback
//				);
			SetTimer(
				hwnd,               // handle to main window
			    IDT_TIMER_TXRX_10MS,// timer identifier
			    10,                 // 10-second interval
			    (TIMERPROC) NULL    // no timer callback
				);

		}
		break;
		/**************************************************************************************************************
	    *      PAINT 																						      PAINT
		**************************************************************************************************************/
		case WM_PAINT: {// если нужно нарисовать, то:
	    	  RECT rect; // стр-ра, определяющая размер клиентской области
	    	  LPPAINTSTRUCT ps={0}; // структура, сод-щая информацию о клиентской области (размеры, цвет и тп)
	    	  HDC hDC = BeginPaint(hwnd, ps); 	// инициализируем контекст устройства
	    	  GetClientRect(hwnd, &rect);  	// получаем ширину и высоту области для рисования

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

		}
		break;

		case WM_TIMER:{
			//The number of milliseconds that have elapsed since the system was started. This is the value returned by the GetTickCount function.
			  switch ((UINT)wParam){
			  case IDT_TIMER_TXRX_10MS:{
					/* Інкремент лічильника ПрогресБара*/
					//SendMessage(hWndPB_RxTx, PBM_STEPIT, 0, 0);
					PB_position+=10;
					SendMessage(hWndPB_RxTx, PBM_SETPOS,
					    PB_position, //Signed integer that becomes the new position.
						0);  //Must be zero
					/* Перемалювати віконце */
					HDC hdcr= GetDC(hwnd);
					RECT rect={0};
					GetClientRect(hwnd, &rect);
					RECT rect_Mn={rect.left, rect.top+15, rect.left+150, rect.top+40};
					DrawText(hdcr, "0", 2, &rect_Mn, DT_SINGLELINE|DT_LEFT|DT_TOP);
					rect_Mn.left=rect.left+35;
					DrawText(hdcr, "5000", 5, &rect_Mn, DT_SINGLELINE|DT_LEFT|DT_TOP);
					char str[6]={0};
					snprintf(str, 6, "%d",PB_position);
					rect_Mn.left=rect.left+70;
					DrawText(hdcr, str, 6, &rect_Mn, DT_SINGLELINE|DT_LEFT|DT_TOP);
					//FillRect(HDC_Mn, &rect_Mn, (HBRUSH)(COLOR_WINDOW+1));
					ReleaseDC(hwnd, hdcr);

					/* Перемалювати віконце */
					InvalidateRect(hWndPB_RxTx, NULL, TRUE);
					UpdateWindow(hWndPB_RxTx);
					//https://docs.microsoft.com/en-us/windows/win32/controls/create-progress-bar-controls
					if(PB_position>=5000){
						PB_position=0;
						KillTimer(hwnd,IDT_TIMER_TXRX_10MS);
						/* Закриваємо вікно, повертаємося до вікна, з якого викликали функцію */
						if(hwnd_called_from==QPWTx_Wnd){
							f_QPWD_TxAnswer();
						}
						else if(hwnd_called_from==QPWRg_Wnd){
							f_QPWD_RgAnswer();
						}
						else{}
						BTH_Process=ProcessIddle;
						BTH_ProcessOld=ProcessIddle;
						DestroyWindow(hwnd);
					}
					/* Тут працюємо з вікном*/
							/* Закриваємо вікно, повертаємося до вікна, з якого викликали функцію
							 * zкщо функція висне - закриваємо по таймеру */
					if((BTH_Process==ProcessIddle)&&(BTH_ProcessOld==ProcessBusy)){
							KillTimer(hwnd,IDT_TIMER_TXRX_10MS);
							PB_position=0;
							/* Закриваємо вікно, повертаємося до вікна, з якого викликали функцію */
							if(hwnd_called_from==QPWTx_Wnd){
								f_QPWD_TxAnswer();
							}
							else if(hwnd_called_from==QPWRg_Wnd){
								f_QPWD_RgAnswer();
							}
							else{}
							BTH_ProcessOld=ProcessIddle;
							DestroyWindow(hwnd);
					}
			  }
			  break;

//			  case IDT_TIMER_TXRX_5000MS:{
//					KillTimer(hwnd,IDT_TIMER_TXRX_5000MS);
//					KillTimer(hwnd,IDT_TIMER_TXRX_10MS);
//					/* Закриваємо вікно, повертаємося до вікна, з якого викликали функцію */
//					DestroyWindow(hwnd);
//			  }
//			  break;
			  default:{}
			  }

			break;
		}//WM_COMMAND

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
LPCSTR f_RegisterPPB_TxRxWndClass(HINSTANCE hinst){

	WNDCLASS w;
		memset(&w,0,sizeof(WNDCLASS));
		w.lpfnWndProc = PPB_TxRxWndProc;
		w.hInstance = hinst;
		w.style = CS_HREDRAW | CS_VREDRAW,
		w.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
		w.lpszClassName = "g_szClassNameTxRx";
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
		WNDCLASS wx={0};
//		LPWNDCLASSEXA lpwcx;
		BOOL gci = GetClassInfoA(hinst,"g_szClassNameTxRx",&wx);
		if(gci==false){
			if(!RegisterClassA(&w)){
				MessageBox(NULL, "Window Registration Failed!", "Error!",
						MB_ICONEXCLAMATION | MB_OK);
			}
		}
		gci =GetClassInfoA(hinst,"g_szClassNameTxRx",&wx);
	    int Error = GetLastError();
	    return w.lpszClassName;

}
int WINAPI f_PPB_TxRxWnd(HWND hh1){
	hwnd_called_from=hh1;
//	WNDCLASSEX wc;
	HWND childTxRx;
	MSG Msg;
childTxRx=CreateWindowEx(
		0,
		g_szClassNameTxRx,
		"Сесія Модбас",

//        WS_CHILD
//		| (WS_OVERLAPPEDWINDOW&(~(WS_MAXIMIZEBOX|WS_MINIMIZEBOX)))
//		|WS_BORDER
//		|WS_VISIBLE
//
//		,

		(WS_OVERLAPPEDWINDOW&(~(WS_MAXIMIZEBOX|WS_MINIMIZEBOX)))|WS_BORDER|WS_VISIBLE
		,
//		CW_USEDEFAULT, CW_USEDEFAULT, 600, 372,
		0,0,150,50,
		NULL, NULL, hinst, NULL);

	ShowWindow(childTxRx, SW_NORMAL);
	UpdateWindow(childTxRx);

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}



void f_BTH_TxRx(void){
	BTH_Process=ProcessBusy;
	BTH_ProcessOld=ProcessBusy;
//	modbus_status_t BTH_TxTx_status = MODBUS_UNKNOWN_ERROR;
//	modbus_master_tx_msg_t BTH_TxMsg={0};
//	modbus_master_rx_msg_t BTH_RxMsg={0};
//	char BTH_User_Msg[256]={0};
	if((BTH_TxMsg.length != 0)&&(BTH_Msg_TimeOut!=0)){
		/* Додаємо RCC*/
		BTH_TxTx_status = f_add_CRC(&BTH_TxMsg);
		/* Запускаємо сесію	*/
		BTH_TxTx_status=tw_txrxModbus_Session (
				cport_nr,
				&BTH_TxMsg,
				&BTH_RxMsg,
				BTH_Msg_TimeOut
		);
	}
	memset(BTH_User_Msg,0,sizeof(BTH_User_Msg));
    if(BTH_TxTx_status == MODBUS_SUCCESS){
	    /* Якщо сесія RTC завершилася успішно, перевіряємо CRC */
	    BTH_TxTx_status = f_check_CRC(&BTH_RxMsg);
	    if (BTH_TxTx_status != MODBUS_SUCCESS){
	    	memset(&BTH_RxMsg,0,sizeof(modbus_master_rx_msg_t));

	    	/* якщо вхідна адреса не рівна вихідній адресі - це помилка*/
	    	if(BTH_RxMsg.msg[0] != BTH_TxMsg.msg[0]){
	    		BTH_TxTx_status=MODBUS_ERR_SLAVE_ADDR;	/*  If the transmitted slave address doesn't correspond to the received slave address	*/
	    	}
	    }
    }
    /* Якщо помилка */
	switch(BTH_TxTx_status){

		case MODBUS_ERR_RX:{
			snprintf (BTH_User_Msg,80,"Message wasn't receive");
			break;
		}
		case MODBUS_ERR_TX:{
			snprintf (BTH_User_Msg,80,"Message wasn't sent");
			break;
		}
		case MODBUS_ERR_TIMEOUT:{
			snprintf (BTH_User_Msg,80, "Відповіді не дочекалися протягом заданого граничного часу ");//"Response was not received from the slave within the timeout");
			break;
		}
		case MODBUS_ERR_SLAVE_ADDR:{
			snprintf (BTH_User_Msg,80,"Transmitted slave address doesn't correspond to the received slave address");
			break;
		}
		case MODBUS_ERR_CRC:{
			snprintf (BTH_User_Msg,80,"Received message CRC error");
			break;
		}
		case MODBUS_ERR_FC:{
			snprintf (BTH_User_Msg,180,"Transmitted function code doesn't correspond to the received function code");
			break;
		}
		case MODBUS_COMPORT_ERR:{
			snprintf (BTH_User_Msg,40,"Помилка СОМ-порту");
			break;
		}

		default:{

		}
	}
	/* Припинення процесу */
	BTH_Process=ProcessIddle;
	_endthread();
	//http://gamesmaker.ru/programming/c/vvedenie-v-winapi-chast-pervaya-sozdanie-okna/

}


