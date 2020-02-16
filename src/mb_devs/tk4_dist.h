/*
 * tk4_dist.h
 *
 *  Created on: 1 ����. 2019 �.
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



/* ֳ �������, �� ���, ���������� ������� � �����
 * ��������� ����� ���, � �������������� - � ����� ���
 * ������� ���� �� ����������� ����� ��� ����� �� �����*/

#define btn_REVERCE    	0	 //4 ������ ������,				���������� ���� - 0
#define evt_ISOL      	1    //6-��������,									  0
#define btn_START    	2    //7-������ ����,                                 0
#define btn_STOP		3    //8-������ STOP,                                 0
#define evt_HIGHLEVEL	4  	 //10-������ �������� �����,                     0
#define evt_LOWLEVEL	5  	 //11-������ �������� ����,                      0
#define evt_DRYMOVE 	6  	 //12-������ ������ ���� ,                        0
#define valADD	        7 	 //13-���������� ������,                          ��� ����
#define evtHIMIDITY	    8  	 //14-������ �������� � ������/������,          0
#define evtTEMPRELAY	9  	 //15-���� ����������� � �����/�� ������*/      0

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
/* ������������ ������� �����, ���� �� ��������������� */
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
	/* ����������� ����� ��������� ����������� ������� ����� */
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
  //������������� �����, ��
  float	   	   	    d_leakAmperage_mA;
  //��� ��������, ���
  float	   	   	    d_InsulationResistance;
  // г���� ������� �� ������� ������ ����, ����� ��� (0..4095)
  uint16_t          d_DryModeADC;
  /* �������� ������������ ��� ������� ��� ������������� ������ */
  pressure_sensor_t pressure_sensor;

  // г���� ������� �� ������������ ������� �������� ����
  // ��� �� ������������������ ���������
  uint16_t         d_LowLevelADC;
  // г���� ������� �� ������������ ������� ��������� ����
  uint16_t         d_HighLevelADC;
  //	  	 // г���� ������� �� ���������, ����� ��� (0..4095)
  uint16_t         d_TermoRelayADC;
  //	       // г���� ������� �� ������� ��������
  uint16_t         d_HumidityADC;
  //	       // г���� ������� �� ����������� ������� �����
  uint16_t         d_AnalogPS_ADC;
  //	  	//�������� ����� � �������, �������� ������������,
  //	      if(f_float_to_2x16(PresureValue, rif)!=0){Ex=0x02;}
  float            d_PresureValue;

  uint16_t         tk2_PumpingMode;

} d_type_t;
/* ��������� ��������������  */
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
	FunctionalState termorelay;						//0x040�
	uint16_t reserve40F;							//0x040F


//	0x0432 ������ ���� ���������� ����������� �������, ���� �������
//	0x0433 ������ ���� ���������� ����������� �������, ������ �������
//	0�0434 ����� ���� ���������� ����������� �������, ���� �������
//	0x0435 ����� ���� ���������� ����������� �������, ������ �������
//	0�0436 ���� ���������� ����������� ������� �� ��������� ���, ���� �������
//	0�0437 ���� ���������� ����������� ������� �� ��������� ���, ������ �������
//	0x0438 ���� ���������� ����������� ������� �� �������� ���, ���� �������
//	0x0439 ���� ���������� ����������� ������� �� �������� ���, ������ �������
	adc_input_t add_input;


	date_time_t date_time_label;

	uint32_t usCRC;
} user_settings_t;
/* ������������ ����������� */
extern volatile user_settings_t us;

typedef struct  {

	uint16_t Reserve800; 				//0x0800
	uint16_t OnMotorDelay;        		//0�0801 �������� ���.�� ��, c
	uint16_t OnSensorMotorDelay;  		//0�0802 �������� ��������� �� ��, c
	uint16_t HeatingTimeConstant;		//0�0803 ������� ���������,c
	uint16_t AftrerDryMoveDelay; 		//0�0804 ����� �� ���. �� ���/��,���
	uint16_t NumberStartLimit;			//0�0805 ����������� ������� ����� �� 10��
	float    LowInsulationRes;			//����� ���� ����� �������� ��� ����.���
	uint16_t LowInsulationRes_d;		//0�0806

	uint16_t LowInsulationRes_fr;		//0�0807
	uint16_t LevelSensorSwichPnt;		//0�0808 ���� ����������� R��� ������ ���
	uint16_t Reserve808;				//0x0809  ������
	FunctionalState DifferentialTransformer;   //0�080A �������� ����������������� ENABLE/DISABLE
	uint16_t GrundLeak_mA;            	//0�080B ����� ������ �� �����, ��
	uint16_t DeviceMode;                //0x080C ��������� ���������� 1...5
	uint16_t AssemblyMode;		        //0x080D ��������� ������� 0..8

	uint16_t ProductionMode; 			//0x080E 0-���������, 1 �������, 2 ������� 2-� �������,
										// 3 ������� 3-� �������,  4 - 2 ������ + ������
	uint16_t ProtectionPlus;			//0x080f ���������� ������ ENABLE/DISABLE

}fSet1_t;
/* ������������ ���������   */
extern volatile fSet1_t  fs1;

typedef struct {
	float amperageAclbr; 	// 0�0820
	uint16_t amperageAclbr_d;
	uint16_t amperageAclbr_fr;

	float clbr_iA;			// 0�0822
	uint16_t clbr_iA_d;
	uint16_t clbr_iA_fr;

	float amperageBclbr;	// 0�0824
	uint16_t amperageBclbr_d;
	uint16_t amperageBclbr_fr;
	float clbr_iB;			// 0�0826
	uint16_t clbr_iB_d;
	uint16_t clbr_iB_fr;

	float amperageCclbr;    // 0�0828
	uint16_t amperageCclbr_d;
	uint16_t amperageCclbr_fr;
	float clbr_iC;	        // 0�082A
	uint16_t clbr_iC_d;
	uint16_t clbr_iC_fr;

	float amperageTrclbr;	// 0�082C
	uint16_t amperageTrclbr_d;
	uint16_t amperageTrclbr_fr;
	float clbr_iTr;			// 0x082E
	uint16_t clbr_iTr_d;
	uint16_t clbr_iTr_fr;

	float voltageCclbr;		// 0�0830
	uint16_t voltageCclbr_d;
	uint16_t voltageCclbr_fr;
	float clbr_uC;			// 0�0832
	uint16_t clbr_uC_d;
	uint16_t clbr_uC_fr;

	float insulationRes;	// 0�0834
	uint16_t insulationRes_d;
	uint16_t insulationRes_fr;

	float clbrResIns;		// 0�0836
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
/* ����������� ���������   */
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
