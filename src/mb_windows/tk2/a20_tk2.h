/* a2_TK112.h
 *
 *  Created on: 25 вер. 2019 р.
 *      Author: KozakVF
 */

#ifndef MB_WINDOWS_A21_112_C_
#define MB_WINDOWS_A21_112_C_
#include     <windows.h>
#include    "stdint.h"

#include "a00_tk2Config.h"


#define IDC_MAIN_EDIT					101
#define IDM_USER_SETTINGS				1010
//#define IDM_FACILITY_SETTING_1			1011
//#define IDM_FACILITY_SETTING_2			1012
//#define IDM_CALIBRATIONS				1013
#define IDM_QUIT						1014
#define IDM_COMPORT_SETTING             1015

#define IDB_BtnSTART                    600
#define IDB_BtnSTOP                     601
#define IDB_BtnOnAUTO                   602
#define IDB_BtnOffAUTO                  603
#define IDB_StartWhenAuto                604
#define IDB_SHOW_EMP_AUTOCTR1            606
#define IDB_SHOW_MOTOR_OPERATING         607

#define IDB_SHOW_VU						608
#define IDB_SHOW_NU						609
#define IDB_SHOW_CX						610
#define IDPB_PROGRESS_BAR_ThermalModelBehavior				611
#define IDPB_PROGRESS_BAR_BARREL		613
//#define ID_TIMER						612
//#define IDB_SHOW_TR						808
//#define IDB_SHOW_HR						809
#define IDPB_PROGRESS_BAR_PESSURE		812







#define IDT_TIMER_TK2_10MS              626

#define IDT_TIMER_TK2_1333MS            622
#define IDT_TIMER_TK2_2000MS            632
#define IDT_TIMER_TK2_50MS              633

#define IDB_START_DISTANCE_CTRL          624
#define IDB_STOP_DISTANCE_CTRL           625
#define IDPB_QUEUE_MSG                   627
#define IDB_RESET_MOTOR_PROTECTION_1     628
#define IDB_RESET_MOTOR_PROTECTION_2     629
#define IDB_TK2DIST_EXIT				 630






LPCSTR f_RegisterTK112WndClass(HINSTANCE hinst);
int WINAPI f_CreateTK112Wnd(HWND hh1, HINSTANCE hInstance);
LRESULT CALLBACK WndProcTK112(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);




#endif /* MB_WINDOWS_A2_TX_MSG_C_ */
