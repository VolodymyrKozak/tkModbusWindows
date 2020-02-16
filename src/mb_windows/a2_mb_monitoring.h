/*
 * a2_mb_monitoring.h
 *
 *  Created on: 10 груд. 2019 р.
 *      Author: KozakVF
 */

#ifndef SRC_MB_WINDOWS_A2_MB_MONITORING_H_
#define SRC_MB_WINDOWS_A2_MB_MONITORING_H_

#include "../mb_funcs/tkRS232rx.h"
#include "../wConfig.h"
/* Ідентифікатори пунктів меню і кнопок    */
#define ID_MBSETTING                    49992
#define ID_MBSTART                    	49993
#define ID_MBSTOP                    	49994
#define ID_MBPOUSE						49995
#define ID_MBRESTART				    49996
//#define ID_MS_SEND_MSG                  49997
//#define ID_MS_SEND_RG					48900
//#define ID_IP320PLAYER                  49998

//#define ID_MS_TK412                     49999
//#define ID_MS_TK412Light                49991
//#define ID_MS_TK412uSet                 49990
//#define ID_MS_TK412f1Set                49989
//#define ID_MS_TK412f2Set                49988




/* Різні прибамбаси прокруток            */
#define IDC_CHILD_EDIT					100
#define IDC_MAIN_MDI					101
#define IDC_MAIN_TOOL					102
#define IDC_MAIN_STATUS					103

/* Бітові маски іконки Техноконту */
extern const BYTE ANDmaskTK_Icon[];
extern const BYTE XORmaskTK_Icon[];


LPCSTR f_RegisterMonitoringWndClass(HINSTANCE hinst);
/* Точка входу в Windows-інтерфейс, створення основного вікна */
int WINAPI f_WinMonitoring(HWND hwnd);

/* Функція обробки повідомлень основного вікна                           */
LRESULT CALLBACK f_WndProcMonitoring(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);




#endif /* SRC_MB_WINDOWS_A2_MB_MONITORING_H_ */
