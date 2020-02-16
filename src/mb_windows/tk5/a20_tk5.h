/* a2_TK112.h
 *
 *  Created on: 25 вер. 2019 р.
 *      Author: KozakVF
 */

#ifndef MB_WINDOWS_A2_112_C_
#define MB_WINDOWS_A2_112_C_
#include     <windows.h>
#include    "stdint.h"

#include "../tk2/a00_tk2Config.h"
#include "../../wConfig.h"


#define IDC_tk5MAIN_EDIT					101
#define IDM_tk5USER_SETTINGS				4010
#define IDM_tk5FACILITY_SETTING			    4011
//#define IDM_FACILITY_SETTING_2			1012
//#define IDM_CALIBRATIONS				1013
#define IDM_tk5QUIT						1014
#define IDM_tk5COMPORT_SETTING             1015

#define IDB_tk5BtnSTART                    600
#define IDB_tk5BtnSTOP                     601
#define IDB_tk5BtnOnAUTO                   602
#define IDB_tk5BtnOffAUTO                  603
#define IDB_tk5StartWhenAuto                604
#define IDB_tk5SHOW_EMP_AUTOCTR1            606
#define IDB_tk5SHOW_MOTOR_OPERATING         607

#define IDBI_GRID_OVERLOADPROTECTION					1
#define IDBI_GRID_UNDERLOADPROTECTION					2
#define IDBI_GRID_VOLTAGEFAULT							4
#define IDBI_GRID_AMPERAGEUNBALANCEPROTECTION			8
#define IDBI_GRID_GROUNDPROTECTION						0X10
//
#define IDBI_GRID_STARTNUMBERLIMIT						0X20
#define IDBI_GRID_JUMMEDPROTECTION						0X40
//
#define IDBI_GRID_PHASE_A_OVERLOADPROTECTION			0X80
#define IDBI_GRID_PHASE_B_OVERLOADPROTECTION			0X100
#define IDBI_GRID_PHASE_C_OVERLOADPROTECTION			0X200
//
#define IDBI_GRID_FREQUENCY_FAULT                       0X400
#define IDBI_GRID_PHASE_SEQUENCE_FAULT                  0X800
#define IDBI_GRID_UNKNOWNFAULT


//GRID_OverLoadProtection						= 1,
//GRID_UnderLoadProtection					= 2,
//GRID_VoltageFault							= 4,
//GRID_AmperageUnbalanceProtection			= 8,
//GRID_GroundProtection						= 0x10,
//
//GRID_StartNumberLimit						= 0x20,
//GRID_JummedProtection						= 0x40,
//
//GRID_Phase_A_OverLoadProtection				= 0x80,
//GRID_Phase_B_OverLoadProtection				= 0x100,
//GRID_Phase_C_OverLoadProtection				= 0x200,
//
//GRID_FREQUENCY_FAULT                        = 0x400,
//GRID_PHASE_SEQUENCE_FAULT                   = 0x800,
//GRID_UnknownFault

#define IDB_tk5SHOW_VU						608
#define IDB_tk5SHOW_NU						609
#define IDB_tk5SHOW_CX						610
#define IDPB_tk5PROGRESS_BAR_ThermalModelBehavior				611
#define IDPB_tk5PROGRESS_BAR_BARREL		613
//#define ID_TIMER						612
//#define IDB_SHOW_TR						808
//#define IDB_SHOW_HR						809
#define IDPB_tk5PROGRESS_BAR_PESSURE		812







#define IDT_TIMER_tk5_10MS              626
#define IDT_TIMER_tk5_1000MS            423
#define IDT_TIMER_tk5_1333MS            622
#define IDT_TIMER_tk5_TEST 				424


//#define IDPB_tk5QUEUE_MSG                   427
//#define IDB_tk5RESET_MOTOR_PROTECTION_1     428
//#define IDB_tk5RESET_MOTOR_PROTECTION_2     429
//#define IDB_tk5DIST_EXIT				    430


//#define IDB_tk5SHOW_MOTOR_OPERATING			431
//#define IDB_tk5SHOW_CX						432
//#define IDPB_tk5PROGRESS_BAR_ThermalModelBehavior 433

//#define IDB_tk5TEST_DISTANCE_CTRL			435
//#define IDB_tk5START_DISTANCE_CTRL			436
//#define IDB_tk5STOP_DISTANCE_CTRL			436
//#define IDB_tk5RESET_MOTOR_PROTECTION_1		437
//#define IDB_tk5RESET_MOTOR_PROTECTION_2 	438
//#define IDB_tk5DIST_EXIT					439
//#define IDPB_tk5QUEUE_MSG 					440

#define IDCB_YDEBUGMODE						180
#define IDE_YHAMPERAGEAMPLITUDE			    1105
#define IDE_YBAMPERAGEAMPLITUDE			    1130
#define IDE_YCOSFI                          1155
#define IDCB_YTK4SCREENMODE					1205

#define IDB_YDEBUGMODE						280
#define IDB_YHAMPERAGEAMPLITUDE			    2105
#define IDB_YBAMPERAGEAMPLITUDE			    2130
#define IDB_YCOSFI                          2155
#define IDB_YTK4SCREENMODE					2205




typedef struct {
/* Зглажена частота в герцах 	  */
hzf_t 		Frequency;
/* Зглажена напруга в вольтах*/
voltf_t 	Voltage;
/* Зглажені струми в амперах */
amperf_t 	aAmperage;
amperf_t 	bAmperage;
amperf_t 	cAmperage;

/* Зглажені основні коєфіцієнти ефективності мережі  */
procentf_t PowerFactor;
procentf_t UnbalanceDeratingFactor;
procentf_t HarmonicsDeratingFactor;

//procentf_t LeakDeratingFactor=100.0;
//procentf_t GrundDeratingFactor=100.0;
//procentf_t NeutralDeratingFactor=100.0;

/* Зглажені інтегральні показники ефективності мережі */
procentf_t HmUnbDerating;
procentf_t fullDerating;
/* Показники енергоспоживання                               */
vatt_t	ActivePower 		;
vatt_t	ReactivePower 		;
vatt_t	UnbalanceLostPower 	;
vatt_t	HarmonicLostPower 	;
vatt_t	NeutralHarmonicLostPower 	;
vatt_t	NegativeHarmonicLostPower 	;
/**********************************************************************************************/

/*Зглажені параметри ефективності мережі 					*/
/*Коефіцієнт нелінійних спотворень по струму у кожній фазі  */
procentf_t aTotalHarmonicDistortion ;
procentf_t bTotalHarmonicDistortion ;
procentf_t cTotalHarmonicDistortion ;
} wtk5_t;


LPCSTR f_RegisterTK5WndClass(HINSTANCE hinst);
HWND f_CreateTK5Wnd(HWND hwnd);
LRESULT CALLBACK WndProcTK5(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

//int16_t f_Gettk5ModbussAddress(int ID_EditBox);


#endif /* MB_WINDOWS_A2_TX_MSG_C_ */
