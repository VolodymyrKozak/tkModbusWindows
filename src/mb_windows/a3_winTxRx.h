/*
 * a3_winTxRx.h
 *
 *  Created on: 7 груд. 2019 р.
 *      Author: KozakVF
 */

#ifndef SRC_MB_WINDOWS_A3_WINTXRX_H_
#define SRC_MB_WINDOWS_A3_WINTXRX_H_

#include "../mb_funcs/tw_mb.h"
#include <process.h>    /* _beginthread, _endthread */

#define IDPB_TXRX                  	    1001
#define IDT_TIMER_TXRX_5000MS				    1002
#define IDT_TIMER_TXRX_10MS				1003



LPCSTR f_RegisterPPB_TxRxWndClass(HINSTANCE hinst);
LRESULT CALLBACK PPB_TxRxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void f_BTH_TxRx(void);

#endif /* SRC_MB_WINDOWS_A3_WINTXRX_H_ */
