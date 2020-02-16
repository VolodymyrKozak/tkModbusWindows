/*
 * a2_mb_registers.h
 *
 *  Created on: 6 груд. 2019 р.
 *      Author: KozakVF
 */

#ifndef SRC_MB_WINDOWS_A2_MB_REGISTERS_H_
#define SRC_MB_WINDOWS_A2_MB_REGISTERS_H_
#include "windows.h"
#include "stdint.h"
#include "../wConfig.h"
#include "a3_winTxRx.h"
#define ID_EDIT_ADDRESS			18
#define ID_EDIT_W_NUMBER        20
#define ID_EDIT_W_VALUE			21
#define ID_BTN_W_DEC			22
#define ID_BTN_W_HEX			19
#define ID_BTN_W_SEND			23



#define ID_EDIT_R_NUMBER		24
#define ID_EDIT_R_VALUE		    25
#define ID_BTN_R_DEC			26
#define ID_BTN_R_HEX			27
#define ID_BTN_R_SEND           29

LPCSTR f_RegisterRgWndClass(HINSTANCE hinst);
int WINAPI f_CreateRgWnd(HWND hh);
LRESULT CALLBACK WndProcRg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif /* SRC_MB_WINDOWS_A2_MB_REGISTERS_H_ */
