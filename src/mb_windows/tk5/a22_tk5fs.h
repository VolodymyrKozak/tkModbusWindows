/*
 * a21_tk5fs.h
 *
 *  Created on: 16 лют. 2020 р.
 *      Author: KozakVF
 */

#ifndef SRC_MB_WINDOWS_TK5_A22_TK5FS_H_
#define SRC_MB_WINDOWS_TK5_A22_TK5FS_H_

#include "windows.h"
#include "../../auxs/hexdec.h"
#include <stdint.h>
#define 	EB_tk5FS_CHECK_PASSWORD  		7778

#define IDB_BTN_WRITE_tk5FS_TO_FLASH        9886
#define IDB_BTN_CHECK_tk5FS_PSW             9987
#define IDB_BTN_tk5FS_EXIT              	9988

#define IDM_QUIT_Tk5FS			8001


//#define IDM_QUIT_tk5US                      9903

#define IDB_BTN_tk5FS_EXIT				8005
#define IDB_BTN_READ_TK5_FS				9904
#define IDT_TIMER_TK5FS_1000MS				8006
//#define IDT_TIMER_USTK5_555MS               9907
#define ID_WR_Tk5FS_MODBUS_ADDR                9905
#define ID_WR_TK5FS_PASSWORD				8003
#define IDM_TK5_FACILITY_SETTING_FROM_FS				8004

#define IDT_TIMER_TK5FS_10MS				8005


#define IDPB_tk5FS_QUEUE_MSG                9906
//#define TK2US_BTN_BLOKING_MS				9907

#define IDB_tk5WRITE_FACILITYSETTING_TO_MEMORY 			8020
#define IDB_tk5WRITE_FACILITYSETTING_TO_FLASH 			8021
#define IDB_BTN_tk5FS_EXIT 								8022
#define IDB_tk5READ_FACILITYSETTING_FROM_MEMORY 		8023




#define   IDE_AMPERAGEACLBR 		0x2000
#define   IDE_CLBR_IA 				0x2802
#define   IDE_AMPERAGEBCLBR 		0x2004
#define   IDE_CLBR_IB 				0x2006
#define   IDE_AMPERAGECCLBR 		0x2008
#define   IDE_CLBR_IC 				0x200A
#define   IDE_VOLTAGECCLBR 			0x200C
#define   IDE_CLBR_UC  				0x200E
#define   IDCB_DEVICEMODE 			0x2010
#define   IDCD_FAZNOST 				0x2011








LPCSTR f_RegisterTk5fsWndClass(HINSTANCE hinst);
HWND   f_CreateTk5fsWnd(HWND hh1);
LRESULT CALLBACK WndProcTk5fs(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL f_tk5fsUpdateWindow(HWND hwnd);
/* „итанн€ вс≥х налаштувань користувача з тк2
 * ¬икликаЇтьс€ при натисканн≥ кнопки „»“ј“» ЌјЋјЎ“”¬јЌЌя
 * та при перший раз при виклику налаштувань з в≥кна дист.управл≥нн€*/
//int f_read_tk5user_settings(HWND hwnd);
//int f_checkEdittk5(int id_LABEL, HWND hwnd, float2_t *f2);
//int f_strint_to_valuetk5(uint8_t *string, uint8_t n_string, float2_t *float2, int LABEL );


#endif /* SRC_MB_WINDOWS_TK5_A22_TK5FS_H_ */
