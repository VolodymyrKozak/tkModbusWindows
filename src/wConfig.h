/*
 * wConfig.h
 *
 *  Created on: 8 груд. 2019 р.
 *      Author: KozakVF
 */

#ifndef SRC_WCONFIG_H_
#define SRC_WCONFIG_H_
#include "stdint.h"
#include "winnt.h"

//#define TK2_DISTANCE_WND
//#define TK2_THERMAL_MODEL_PROGRES_BAR_DEBUG
typedef enum{
  DISABLE = 0,
  ENABLE  = 1,
  UNKNOWN = 2
} FunctionalState;
#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == DISABLE) || ((STATE) == ENABLE))

typedef enum{
	TK2_DISTANCE,
	TK4_DISTANCE,
	TK5_DISTANCE
}vk_software_t;
typedef enum{
  NO_EVENT = 0,
  EVENT = !NO_EVENT
} event_t;
/* wk_motor_commands.h */
typedef enum{
	oFF_,
	oN_,
	unknownOnOff
}on_off_t;
typedef enum {
	ProcessIddle =0,
	ProcessBusy  =!ProcessIddle
}ProcessState_t;
typedef struct{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
} date_time_t;
typedef enum{
	UnknownTypeLoad,
	ActiveLoad,
	InductiveLoad,
	CapacitiveLoad
} grid_load_t;

typedef enum{
	PhasesUnknownTypeRotation,
	PhasesDirectRotation,
	PhasesReverceRotation,
	PhasesSequenceFault
}phases_rotation_type_t;

extern const BYTE ANDmaskTK_Icon[];

#define procentd_t int



#define adcf_t float
#define adcu16_t uint16_t
#define amperf_t float
#define hzf_t  float
#define voltf_t  float
#define vatt_t float
#define kvatt_t  float
#define gradf_t  float
#define procentf_t float



#define USER_SESSAGE_SIZE 256





#endif /* SRC_WCONFIG_H_ */



//#define COLOR_SCROLLBAR 				    0
//#define COLOR_BACKGROUND				    1
//#define COLOR_DESKTOP					    1
//#define COLOR_ACTIVECAPTION				    2
//#define COLOR_INACTIVECAPTION				3
//#define COLOR_MENU					        4
//#define COLOR_WINDOW					    5
//#define COLOR_WINDOWFRAME				    6
//#define COLOR_MENUTEXT					    7
//#define COLOR_WINDOWTEXT				    8
//#define COLOR_CAPTIONTEXT				    9
//#define COLOR_ACTIVEBORDER				   10
//
//#define COLOR_INACTIVEBORDER			   11
//#define COLOR_APPWORKSPACE				   12
//#define COLOR_HIGHLIGHT 				   13
//#define COLOR_HIGHLIGHTTEXT				   14
//#define COLOR_BTNFACE					   15
//#define COLOR_3DSHADOW					   16
//#define COLOR_BTNSHADOW 				   16
//#define COLOR_3DFACE					   15
//#define COLOR_GRAYTEXT					   17
//#define COLOR_BTNTEXT					   18
//#define COLOR_INACTIVECAPTIONTEXT		   19
//#define COLOR_3DHILIGHT 				   20
//#define COLOR_3DHIGHLIGHT				   20
//#define COLOR_BTNHILIGHT				   20
//#define COLOR_BTNHIGHLIGHT				   20
//
//#define COLOR_3DDKSHADOW				   21
//#define COLOR_3DLIGHT					   22
//#define COLOR_INFOTEXT					   23
//#define COLOR_INFOBK					   24
//
//#define COLOR_HOTLIGHT					   26
//#define COLOR_GRADIENTACTIVECAPTION		   27
//#define COLOR_GRADIENTINACTIVECAPTION	   28




//leakAmperage_A
//GrundAmperageDistortion
//LoadType
//AmperageUnbalance
//ActivePower_kW
//ReactivePower_kW
//UnbalanceLostPower_kW
//HarmonicLostPower_kW
//aTotalHarmonicDistortion;
//bTotalHarmonicDistortion;
//cTotalHarmonicDistortion;
//THDi_HarmonicAmperageDistortion;
//aNegativeAmperage_A
//bNegativeAmperage_A
//cNegativeAmperage_A
//aNeutralAmperage_A
//bNeutralAmperage_A
//cNeutralAmperage_A
//sumNeutralAmperage_A




