/*
 * a21tk5us.h
 *
 *  Created on: 16 лют. 2020 р.
 *      Author: KozakVF
 */

#ifndef SRC_MB_WINDOWS_TK5_A21_TK5US_H_
#define SRC_MB_WINDOWS_TK5_A21_TK5US_H_

#include "windows.h"
#include "../../auxs/hexdec.h"
#include <stdint.h>
#include "../../wConfig.h"

//typedef enum {
//	password_user_access,
//	free_user_access
//} menu_access_t;



/*
Час реакції при запуску електродвигуна з холодного стану при кратності перевищення номінативного струму 7.2 (ДСТУ IEC 60947-4-1
Клас реле:
10А 	2..10сек
10  	4..10сек
20		6..20сек
30 	постійна часу теплової моделі – 2..20сек/0.0195=102..1540сек.
За замовчуванням: 13 сек = 700сек постійна часу
*/











#define 	EB_tk5US_CHECK_PASSWORD  		7778

#define IDB_BTN_WRITE_tk5US_TO_FLASH        9886
#define IDB_BTN_CHECK_tk5US_PSW             9987
#define IDB_BTN_tk5US_EXIT              	9988
#define IDM_Tk5FACILITY_SETTING_from_US		8000
#define IDM_QUIT_Tk5US			8001


//#define IDM_QUIT_tk5US                      9903

#define IDB_BTN_tk5US_EXIT				8005
#define IDB_BTN_READ_TK5_US				9904
#define IDT_TIMER_TK5US_1000MS				8006
//#define IDT_TIMER_USTK5_555MS               9907
#define ID_WR_Tk5US_MODBUS_ADDR                9905
#define ID_WR_TK5US_PASSWORD				8003
#define IDM_TK5_FACILITY_SETTING_FROM_US				8004

#define IDT_TIMER_TK5US_10MS				8005

#define IDPB_tk5US_QUEUE_MSG                9906
//#define TK2US_BTN_BLOKING_MS				9907

#define IDE_MOTOR_OR_POWERSUPPLY            8010

LPCSTR f_RegisterTk5usWndClass(HINSTANCE hinst);
HWND   f_CreateTk5usWnd(HWND hh1);
LRESULT CALLBACK WndProcTk5us(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static BOOL CALLBACK ChildCallbacktk5us(
		HWND   hwnd,
		LPARAM lParam
);

/* Читання всіх налаштувань користувача з тк2
 * Викликається при натисканні кнопки ЧИТАТИ НАЛАШТУВАННЯ
 * та при перший раз при виклику налаштувань з вікна дист.управління*/
int f_read_tk5user_settings(HWND hwnd);
int f_checkEdittk5(int id_LABEL, HWND hwnd, float2_t *f2);
int f_strint_to_valuetk5(uint8_t *string, uint8_t n_string, float2_t *float2, int LABEL );



#endif /* SRC_MB_WINDOWS_TK5_A21_TK5US_H_ */
