/*
 * a2_Tx_msg.c
 *
 *  Created on: 25 вер. 2019 р.
 *      Author: KozakVF
 */

#ifndef MB_WINDOWS_A2_TX_MSG_C_
#define MB_WINDOWS_A2_TX_MSG_C_
#include     <windows.h>
#include    "stdint.h"
#include "../wConfig.h"
//#define IDC_STATIC 						-1
#define IDC_MAIN_EDIT2 					  1101
#define ID_TRACKBAR                       2100
//#define IDC_TEXT                        1000
//#define IDC_NUMBER                      1001
//#define IDC_LIST                        1002
//#define IDC_ADD                         1003
//#define IDC_CLEAR                       1004
//#define IDC_REMOVE                      1005
//#define IDC_SHOWCOUNT                   1006
/*
Coils/Discrete Output(0x)
Discrete Input (1x)
Input Registers(3x)
Holding Registers(4x)
*/

int WINAPI f_Create320Wnd(HWND hh);
LRESULT CALLBACK WndProcC320(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int WINAPI WinCtrOne(HWND hh);
#endif /* MB_WINDOWS_A2_TX_MSG_C_ */
