/*
 * a2_Tx_msg.c
 *
 *  Created on: 25 вер. 2019 р.
 *      Author: KozakVF
 */

#ifndef MB_WINDOWS_A2_TX_MSG_C_
#define MB_WINDOWS_A2_TX_MSG_C_
#include  <windows.h>
#include "stdint.h"
#include "../wConfig.h"

#define IDC_STATIC 						-1
#define IDC_MAIN_EDIT					101
#define IDC_TEXT                        1000
#define IDC_NUMBER                      1001
#define IDC_LIST                        1002
#define IDC_ADD                         1003
#define IDC_CLEAR                       1004
#define IDC_REMOVE                      1005
#define IDC_SHOWCOUNT                   1006

LPCSTR f_RegisterTxWndClass(HINSTANCE hinst);
int WINAPI f_CreateTxWnd(HWND hh);
LRESULT CALLBACK WndProcTx(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int WINAPI f_PPB_TxRxWnd(HWND hh1);
void f_QPWD_TxAnswer(void);
#endif /* MB_WINDOWS_A2_TX_MSG_C_ */
