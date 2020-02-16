/*
 * a2_setting.h
 *
 *  Created on: 13 ρεπο. 2019 π.
 *      Author: KozakVF
 */

#ifndef MB_WINDOWS_A2_SETTING_H_
#define MB_WINDOWS_A2_SETTING_H_

#include <windows.h>
#include <commctrl.h>
#include "../wConfig.h"

LPCSTR f_RegisterSWndClass(HINSTANCE hinst);
int WINAPI f_CreateSWnd(HWND hh);
LRESULT CALLBACK ChildWndProcS(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif /* MB_WINDOWS_A2_SETTING_H_ */
