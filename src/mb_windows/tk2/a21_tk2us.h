/* a2_TK112.h
 *
 *  Created on: 25 вер. 2019 р.
 *      Author: KozakVF
 */


#ifndef MB_WINDOWS_TK4US
#define MB_WINDOWS_TK4US
#include     <windows.h>
#include    "stdint.h"
#include "../../mb_devs/tk4_dist.h"
#include "a00_tk2Config.h"


#define IDC_POWER_EDIT    				9875
#define IDC_AMPERAGE_SETEDIT			9876
#define CURRENT_TRIPPING_LIM            9877
#define POWER_TRIPPING_LIMIT            9878
#define ID_EDIT_MODBUSS_ADDR            7776
#define ID_EDIT_NEW_PASSWORD            7775
#define 	EB_tk2US_PUMPREGIMEMODE     7777
#define 	EB_tk2US_CHECK_PASSWORD  	7778
#define 	EB_tk2US_SELFSTART      	7779

#define IDC_ADD_HIGH_LIMIT				9880
#define IDC_ADD_LOW_LIMIT				9881
#define IDC_OP_ADD_HIGH_LIMIT			9882
#define IDC_OP_ADD_LOW_LIMIT			9883

#define IDB_BTN_READ                    9884
#define IDB_BTN_WRITE_TO_MEMORY         9885
#define IDB_BTN_WRITE_TO_FLASH          9886
#define IDB_BTN_CHECK_PSW               9987
#define IDB_BTN_tk2US_EXIT              9988

#define IDPB_PROGRESS_BAR_READ				9897
#define IDPB_PROGRESS_BAR_WRITE_TO_MEMORY   9898
#define IDPB_PROGRESS_BAR_WRITE_TO_FLASH 	9899

#define IDM_FACILITY_SETTING_1              9901

#define IDM_COMPORT_SETTING_US              9902

#define IDM_QUIT_US                         9903


#define IDT_TIMER_USTK2_1000MS				9904
#define IDT_TIMER_USTK2_555MS               9907
#define IDT_usTIMER_TK2_10MS                9905
#define IDPB_usQUEUE_MSG                    9906
//#define TK2US_BTN_BLOKING_MS				9907

LPCSTR f_RegisterTK112USWndClass(HINSTANCE hinst);
HWND   f_CreateTK112usWnd(HWND hh1);
LRESULT CALLBACK WndProcTK112us(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL f_tk2usUpdateWindow(HWND hwnd);
/* „итанн€ вс≥х налаштувань користувача з тк2
 * ¬икликаЇтьс€ при натисканн≥ кнопки „»“ј“» ЌјЋјЎ“”¬јЌЌя
 * та при перший раз при виклику налаштувань з в≥кна дист.управл≥нн€*/
int f_read_user_settings(HWND hwnd);
int f_checkEdit112(int id_LABEL, HWND hwnd, float2_t *f2);
int f_strint_to_value112(uint8_t *string, uint8_t n_string, float2_t *float2, int LABEL );
#endif /* MB_WINDOWS_A2_TX_MSG_C_ */
