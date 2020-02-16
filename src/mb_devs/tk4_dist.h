/*
 * tk4_dist.h
 *
 *  Created on: 1 груд. 2019 р.
 *      Author: KozakVF
 */

#ifndef SRC_MB_DEVS_TK4_DIST_H_
#define SRC_MB_DEVS_TK4_DIST_H_

#include "../wConfig.h"

#include "stdint.h"
#include "../auxs/hexdec.h"
#include "../auxs/electro.h"
typedef enum{
	fast_measurement,
	slow_measurement
}measurement_t;

typedef struct{
	uint16_t imin;
	uint16_t imax;
} i412limit_t;



/* Ці дефайни, по суті, закріпляють індекси в масиві
 * зглажених входів АЦП, а опосередковано - і номер АЦП
 * Оскільки поки що призначення входів АЦП точно не відомо*/

#define btn_REVERCE    	0	 //4 кнопка РЕВЕРС,				початковий стан - 0
#define evt_ISOL      	1    //6-ізоляція,									  0
#define btn_START    	2    //7-кнопка ПУСК,                                 0
#define btn_STOP		3    //8-кнопка STOP,                                 0
#define evt_HIGHLEVEL	4  	 //10-датчик високого рівень,                     0
#define evt_LOWLEVEL	5  	 //11-датчик низького рівня,                      0
#define evt_DRYMOVE 	6  	 //12-датчик сухого ходу ,                        0
#define valADD	        7 	 //13-аналоговий датчик,                          все одно
#define evtHIMIDITY	    8  	 //14-датчик вологості в насосц/двигуні,          0
#define evtTEMPRELAY	9  	 //15-реле температури в насосі/на двигуні*/      0

typedef enum {
	ct_unknown				=0,
	manual_mode 			=1,
	automatic_mode    		=2,
	remote_manual_mode 		=3,
	remote_automatic_mode	=4,
	setting_special			=5

} control_type_t;

typedef enum {
	electrode,
	manometer_EKM,
	pressure_relay,
	float_sensor,
	analogue,
	sensor_unknown
} pressure_sensor_t;

typedef enum {
	liquid_supply,
	liquid_pumping_out,
	op_unknown
} operation_mode_t;



typedef enum {
	password_user_access,
	free_user_access
} menu_access_t;

typedef enum {
	scale_percentage,
	bar,
	atm,
	kilopascal,
	megapascal,
	pascal_,
	metr_of_water,
	sm_of_water,
	unit_of_ADC,
	unit_unknown
} add_unit_t;


typedef struct {
	add_unit_t add_unit;							//0x0430
	uint16_t   reserve431;

	float add_high_limit;
	uint16_t add_high_limit_d;
	uint16_t add_high_limit_fr;

	float add_low_limit;
	uint16_t add_low_limit_d;
	uint16_t add_low_limit_fr;

	float op_add_high_limit;
	uint16_t op_add_high_limit_d;
	uint16_t op_add_high_limit_fr;

	float op_add_low_limit;
	uint16_t op_add_low_limit_d;
	uint16_t op_add_low_limit_fr;

} adc_input_t;
/* Налаштування датчика тиску, якщо він використовується */
extern volatile adc_input_t ai;
typedef enum{
	motor_ok                        = 0,
	motor_Amperage_Unbalance        = 1,
	motor_OverLoad                  = 2,
	motor_start_jummed   		    = 22,
	motor_operation_jummed		    = 23,
	motor_UnderLoad                 = 3,
	motor_insulation_fault          = 4,
	motor_differential_current      = 5,
	motor_dry_move					= 6,//
	motor_liquidSensor_fault        = 7,//
	motor_start_UnderVoltage        = 8,
	motor_UnderVoltage				= 81,
	motor_OverVoltage				= 82,
	motor_StartNumber_limit			= 9,
	motor_ThermalRelay_off			= 0x0A,
	motor_HumidityRelay_off			= 0x0B,
	motor_distance_cntr 			= 0x0C,
	motor_other_faults				= 0x0D,

	wrong_parameters_set            = 0x46,
	/* Неправильно задані параметри аналогового датчика тиску */
	motor_power_amperage_suss		 =0x80,
	no_pressure_sensor              = 0x86,
	pressure_sensor_setting_fault   = 0x87,

	motor_unknown_fault				= 0xFF
}motorfault_t;

typedef struct{
  control_type_t 	d_regime;
  on_off_t       	d_MotorState;
  operation_mode_t  d_oper;
  FunctionalState   d_FS;
//   uint8_t          d_Levl;
  on_off_t          d_AutoCtrl;
  on_off_t          d_ADD;
  on_off_t          d_CX;
  on_off_t          d_NU;
  on_off_t          d_VU;
  on_off_t          d_TR;
  on_off_t          d_HR;
  motorfault_t      d_motorfault;

  procentf_t	    d_ThermalModelBehavior;
  vatt_t	   	   	d_ActivePower;
  amperf_t          d_aAmperage;
  amperf_t          d_bAmperage;
  amperf_t          d_cAmperage;
  uint16_t          d_Voltage;
  float             d_Frequency;
  uint16_t          d_PowerFactor;
  uint16_t		    d_AmperageUnbalance;
  uint32_t          d_MotorOperationalLife;
  //Диференційний струм, мА
  float	   	   	    d_leakAmperage_mA;
  //Опір ізоляції, мОм
  float	   	   	    d_InsulationResistance;
  // Рівень сигналу від датчика сухого ходу, відліки АЦП (0..4095)
  uint16_t          d_DryModeADC;
  /* Вибраний користувачем тип датчика для автоматичного режиму */
  pressure_sensor_t pressure_sensor;

  // Рівень сигналу від електродного датчика нижнього рівня
  // або від електроконтактного манометра
  uint16_t         d_LowLevelADC;
  // Рівень сигналу від електродного датчика верхнього рівня
  uint16_t         d_HighLevelADC;
  //	  	 // Рівень сигналу від термореле, відліки АЦП (0..4095)
  uint16_t         d_TermoRelayADC;
  //	       // Рівень сигналу від датчика вологості
  uint16_t         d_HumidityADC;
  //	       // Рівень сигналу від аналогового датчика тиску
  uint16_t         d_AnalogPS_ADC;
  //	  	//Величина тиску в одницях, вибраних користувачем,
  //	      if(f_float_to_2x16(PresureValue, rif)!=0){Ex=0x02;}
  float            d_PresureValue;

  uint16_t         tk2_PumpingMode;

} d_type_t;
/* Параметри функціонування  */
extern volatile  d_type_t d;



typedef struct {
	uint32_t reserve400;						    //0x0400
	control_type_t regime;							//0x0401
	operation_mode_t operational_mode;				//0x0402
	FunctionalState drive_self_starting; 			//0x0403
	pressure_sensor_t pressure_sensor;				//0x0404
	uint32_t reserve405;							//0x0405
	FunctionalState humidity_relay;					//0x0406
	uint32_t reserve407;							//0x0407

	float rated_motor_power;//0x0408..9
	uint16_t rated_motor_power_d;
	uint16_t rated_motor_power_fr;

	float rated_amperage_set;					//0x040A..B
	uint16_t rated_amperage_set_d;
	uint16_t rated_amperage_set_f;
	uint16_t current_tripping_limit;				//0x040C
	uint16_t power_tripping_limit;					//0x040D
	FunctionalState termorelay;						//0x040Е
	uint16_t reserve40F;							//0x040F


//	0x0432 Верхній поріг вимірювання аналогового датчика, ціла частина
//	0x0433 Верхній поріг вимірювання аналогового датчика, дробна частина
//	0х0434 Нижній поріг вимірювання аналогового датчика, ціла частина
//	0x0435 Нижній поріг вимірювання аналогового датчика, дробна частина
//	0х0436 Поріг відключення аналогового датчика на верхньому рівні, ціла частина
//	0х0437 Поріг відключення аналогового датчика на верхньому рівні, дробна частина
//	0x0438 Поріг відключення аналогового датчика на нижньому рівні, ціла частина
//	0x0439 Поріг відключення аналогового датчика на нижньому рівні, дробна частина
	adc_input_t add_input;


	date_time_t date_time_label;

	uint32_t usCRC;
} user_settings_t;
/* Налаштування користувача */
extern volatile user_settings_t us;

typedef struct  {

	uint16_t Reserve800; 				//0x0800
	uint16_t OnMotorDelay;        		//0х0801 Затримка вкл.ЕД ПН, c
	uint16_t OnSensorMotorDelay;  		//0х0802 Затримка включення ЕД ДУ, c
	uint16_t HeatingTimeConstant;		//0х0803 Постійна нагрівання,c
	uint16_t AftrerDryMoveDelay; 		//0х0804 Дозвіл на вкл. ЕД Твн/Сх,мин
	uint16_t NumberStartLimit;			//0х0805 Максимальна кількість пусків за 10хв
	float    LowInsulationRes;			//Нижній поріг опору ізоляції для відкл.мОм
	uint16_t LowInsulationRes_d;		//0х0806

	uint16_t LowInsulationRes_fr;		//0х0807
	uint16_t LevelSensorSwichPnt;		//0х0808 Поріг перемикання Rдуб попугаї ацп
	uint16_t Reserve808;				//0x0809  Резерв
	FunctionalState DifferentialTransformer;   //0х080A Наявність дифтрансформатора ENABLE/DISABLE
	uint16_t GrundLeak_mA;            	//0х080B Струм витоку на землю, мА
	uint16_t DeviceMode;                //0x080C Типорозмір контролера 1...5
	uint16_t AssemblyMode;		        //0x080D Типорозмір станції 0..8

	uint16_t ProductionMode; 			//0x080E 0-стройство, 1 станция, 2 станция 2-х насосов,
										// 3 станция 3-х насосов,  4 - 2 насоса + джокер
	uint16_t ProtectionPlus;			//0x080f Додатковий захист ENABLE/DISABLE

}fSet1_t;
/* Налаштування виробника   */
extern volatile fSet1_t  fs1;

typedef struct {
	float amperageAclbr; 	// 0х0820
	uint16_t amperageAclbr_d;
	uint16_t amperageAclbr_fr;

	float clbr_iA;			// 0х0822
	uint16_t clbr_iA_d;
	uint16_t clbr_iA_fr;

	float amperageBclbr;	// 0х0824
	uint16_t amperageBclbr_d;
	uint16_t amperageBclbr_fr;
	float clbr_iB;			// 0х0826
	uint16_t clbr_iB_d;
	uint16_t clbr_iB_fr;

	float amperageCclbr;    // 0х0828
	uint16_t amperageCclbr_d;
	uint16_t amperageCclbr_fr;
	float clbr_iC;	        // 0х082A
	uint16_t clbr_iC_d;
	uint16_t clbr_iC_fr;

	float amperageTrclbr;	// 0х082C
	uint16_t amperageTrclbr_d;
	uint16_t amperageTrclbr_fr;
	float clbr_iTr;			// 0x082E
	uint16_t clbr_iTr_d;
	uint16_t clbr_iTr_fr;

	float voltageCclbr;		// 0х0830
	uint16_t voltageCclbr_d;
	uint16_t voltageCclbr_fr;
	float clbr_uC;			// 0х0832
	uint16_t clbr_uC_d;
	uint16_t clbr_uC_fr;

	float insulationRes;	// 0х0834
	uint16_t insulationRes_d;
	uint16_t insulationRes_fr;

	float clbrResIns;		// 0х0836
	uint16_t clbrResIns_d;
	uint16_t clbrResIns_fr;

	float clbrADD;			// 0x0838..0x0839
	uint16_t clbrADD_d;
	uint16_t clbrADD_fr;


	uint16_t phasing;		// 0x083A

	uint16_t  reserve3B;	// 0x083B

	uint16_t  reserve3C;	// 0x083C

	uint16_t reserve3D;     // 0x083D

	uint16_t reserve3E;		// 0x083E

	uint16_t reserve3F;     // 0x083F

}fSet2f_t;
/* Калібрування виробника   */
extern volatile fSet2f_t fs2;
//typedef struct {
//	fSet1_t fSet1;
//	fSet2f_t fset2f;
//}fSet_t;

int f_ReadUS_fromTK4(user_settings_t *husfm);
int f_WriteUS_toTK4_Memory(user_settings_t *hustm);
int f_WriteUS_toTK4_Flash(user_settings_t *hustf);

int f_ReadFS1_fromTK4(fSet1_t *husfm);
int f_WriteFS1_toTK4_Memory(fSet1_t *hustm);
int f_WriteFS1_toTK4_Flash(fSet1_t *hustf);

int f_ReadFS2_fromTK4(fSet2f_t *husfm);
int f_WriteFS2_toTK4_Memory(fSet2f_t *hustm);
int f_WriteFS2_toTK4_Flash(fSet2f_t *hustf);


#endif /* SRC_MB_DEVS_TK4_DIST_H_ */
