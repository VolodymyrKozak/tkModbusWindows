/*
 * tk4_dist.c
 *
 *  Created on: 1 ����. 2019 �.
 *      Author: KozakVF
 */

#include "tk4_dist.h"
#include "string.h"
#include "windows.h"
#include "stdio.h"

FILE *fp4;

#define READ_MAIN_PARAM 			0
#define READ_USER_SETTINGS 			1
#define READ_PRESSURE_SENSOR_SET 	2
#define READ_FACILITY_SETTING1 		3
#define READ_FACILITY_SETTING2 		4


/* ��������� ��������������  */
volatile d_type_t d={0};
/* ������������ ����������� */
volatile user_settings_t us={0};
/* ������������ ������� �����, ���� �� ��������������� */
volatile adc_input_t ai={0};
/* ������������ ���������   */
volatile fSet1_t  fs1={0};
/* ����������� ���������   */
volatile fSet2f_t fs2={0};

uint16_t mb_req[5][2]={
	   /* O����� ��������� */
       { 0x0080,      0x20},
	   /* ������������ ����������� ���.*/
       { 0x0400,      0x10},
       /*������� ����������� �������*/
       { 0x0430,      0x0A},
	   /*  �������� ������������ 1 */
       { 0x0800,      0x10},
	   /*  �������� ������������ 2 */
       { 0x0820,      0x22}
};

static motorfault_t f_set_testing(user_settings_t *t);
static void t2_dBase(uint16_t variant, uint16_t *r);

static void t2_dBase(uint16_t variant, uint16_t *r){

 switch(variant){
 case READ_MAIN_PARAM:{

	 //	     	// ������� ������ ��� ��������� �����, ���:
	 //	     	// 0..3 ����� ������, ��������  0 - ������������
	 //	     	//					 			1 - ������
	 //	     	//                   			2 - ����������� ������������
	 //	     	//								3 - ������������ ������
	 //	     	//                              4 - ��������� ������������
	 d.d_regime      = (control_type_t)	   ((r[0] & 0xF000)>>12);
	 //	     	// 4     �������� ����          0 - ������ ��������
	 //	     	//   							1 - ������ ������
	 d.d_MotorState  = (on_off_t)          ((r[0] & 0x0800)>>11);
	 //	     	// 5     ��� ������             0 - ������ �����
	 //	     	//								1 - ������� �����
	 d.d_oper        = (operation_mode_t)  ((r[0] & 0x0400)>>10);
	 //	     	// 6     ���������� � ���.���.  0 - �� �����������
	 //	     	//								1 - �����������
	 d.d_FS          = (FunctionalState)   ((r[0] & 0x0200)>>9);
	 //	     	// 7     �������� � ���. ����� 0 - ���.����� �� ��������
	 //	     	//								1 - ���.����� ��������
	 d.d_AutoCtrl    = (on_off_t)  ((r[0] & 0x0200)>>8);
	 //	     	//       C��� ����������� ������� �����
	 //	     	// 8                            0 - ������� ����� �������
	 //	     	//                              1 - ������� ����� �������
	 d.d_ADD         = (on_off_t)  ((r[0] & 0x0200)>>7);
	 //	 	    //       ���� ������� ������ ����
	 //	 	    // 9                            0 - ������� ����� �������
	 //	 	    //                              1 - ������� ����� �������
	 d.d_CX          = (uint8_t) 	       ((r[0] & 0x0040)>>6);
	 //	 	    //       ���� ������� �������� ����
	 //	         //10                            0 - ������� ����� �������
	 //	         //                              1 - ������� ����� �������
	 d.d_NU          = (uint8_t)           ((r[0] & 0x0020)>>5);
	 //	 	    //       ���� ������� ��������� ����
	 //	         //11                            0 - ������� ����� �������
	 //	         //                              1 - ������� ����� �������
	 d.d_VU          = (uint8_t)           ((r[0] & 0x0010)>>4);
	 //	 		//       C��� ������� ����������� ��
	 //	         //12                            0 - ������� ����� �������
	 //	         //                              1 - ������� ����� �������
	 d.d_TR          = (uint8_t)           ((r[0] & 0x0010)>>3);
	 //	 		//       C��� ������� �������� ������
	 //	     	//13                            0 - ������� ����� �������
	 //	     	//                              1 - ������� ����� �������
	 d.d_HR          = (uint8_t)           ((r[0] & 0x0010)>>2);


	 /* ��� ������� �������������� */
	 d.d_motorfault          = (motorfault_t) 		     r[1] % 0x100;
	 d.d_ThermalModelBehavior        =(float)r[1]/100.0f;

	 d.d_ActivePower                 =(float)r[2]+(float)r[3]/10000.0f;
	 d.d_aAmperage                   =(float)r[4]+(float)r[5]/10000.0f;
	 d.d_bAmperage                   =(float)r[6]+(float)r[7]/10000.0f;
	 d.d_cAmperage                   =(float)r[8]+(float)r[9]/10000.0f;
	 d.d_Voltage					  =        r[0xA];
	 d.d_Frequency	               =(float)r[0xB]/100.0f;
	 d.d_PowerFactor                      =  r[0xC];
	 d.d_AmperageUnbalance				  =  r[0xD];
	 d.d_MotorOperationalLife             =  r[0xE]*0x10000+r[0xF];

	 //������������� �����, ��
	 d.d_leakAmperage_mA             =(float)r[0x10]+(float)r[0x11]/10000.0f;
	 //��� ��������, ���
	 d.d_InsulationResistance        =(float)r[0x12]+(float)r[0x13]/10000.0f;
	 	 /* �������� ������������ ��� ������� ��� ������������� ������ */
	 d.pressure_sensor              =        r[0x14];


	 // г���� ������� �� ������� ������ ����, ����� ��� (0..4095)

	 d.d_AutoCtrl                  =(on_off_t)r[0x15];

	 d.d_DryModeADC                =          r[0x16];
	 // г���� ������� �� ������������ ������� �������� ����
	 // ��� �� ������������������ ���������
	 d.d_LowLevelADC                 =       r[0x17];
	 // г���� ������� �� ������������ ������� ��������� ����
	 d.d_HighLevelADC                =       r[0x18];
	   //	  	 // г���� ������� �� ���������, ����� ��� (0..4095)
	 d.d_TermoRelayADC      =                r[0x19];
	   //	       // г���� ������� �� ������� ��������
	 d.d_HumidityADC        =                r[0x1A];
	   //	       // г���� ������� �� ����������� ������� �����
	 d.d_AnalogPS_ADC       =                r[0x1B];
		//�������� ����� � �������, �������� ������������,
	 d.d_PresureValue                =(float)r[0x1C]+(float)r[0x1D]/10000.0f;


   }
  break;
  case READ_USER_SETTINGS: {

	  volatile user_settings_t us={0};
	    us.reserve400 =                         r[0];			//0x0400
		us.regime=	(control_type_t)            r[1]; 			//0x0401
		us.operational_mode=(operation_mode_t)  r[2];			//0x0402
		us.drive_self_starting=(FunctionalState)r[3]; 			//0x0403
		us.pressure_sensor=(pressure_sensor_t) 	r[4];			//0x0404
		us.reserve405=		          (uint32_t)r[5];		    //0x0405
		us.humidity_relay=	   (FunctionalState)r[6];			//0x0406
		us.reserve407=				    r[7];			//0x0407
		us.rated_motor_power_d=                 r[0x08];
		us.rated_motor_power_fr=                 r[0x09];
		us.rated_motor_power   =	         (float)r[0x08]+(float)r[0x09]/10000.0f;//0x0408..9
		us.rated_amperage_set_d=                 r[0x08];
		us.rated_amperage_set_f=                 r[0x09];
		us.rated_amperage_set=	      	 (float)r[0x0A]+(float)r[0x0B]/10000.0f;//0x040a..b
		us.current_tripping_limit=	  (uint16_t)r[0x0C];		//0x040AC
		us.power_tripping_limit=	  (uint16_t)r[0x0D];	    //0x040D


  }
  	  break;
  case READ_PRESSURE_SENSOR_SET:{

//	  	scale_percentage,
//	  	bar,
//	  	atm,
//	  	kilopascal,
//	  	megapascal,
//	  	pascal,
//	  	metr_of_water,
//	  	sm_of_water,//d_add_unit
//	  	unit_of_ADC
	   ai.add_unit=			(add_unit_t)r[0x00];
	   ai.reserve431=					r[0x01];
	   ai.add_high_limit=              (float)r[0x02]+(float)r[0x03]/10000.0f;
	   ai.add_low_limit=               (float)r[0x04]+(float)r[0x05]/10000.0f;
	   ai.op_add_high_limit=           (float)r[0x06]+(float)r[0x07]/10000.0f;
	   ai.op_add_low_limit=            (float)r[0x08]+(float)r[0x09]/10000.0f;
  }
	  break;
  case READ_FACILITY_SETTING1:{
	  volatile fSet1_t fs1={0};
		fs1.Reserve800 				    =r[0x00];//0x0800
		fs1.OnMotorDelay        		=r[0x01];//0�0801 �������� ���.�� ��, c
		fs1.OnSensorMotorDelay  		=r[0x02];//0�0802 �������� ��������� �� ��, c
		fs1.HeatingTimeConstant		    =r[0x03];//0�0803 ������� ���������,c
		fs1.AftrerDryMoveDelay 		    =r[0x04];//0�0804 ����� �� ���. �� ���/��,���
		fs1.NumberStartLimit			=r[0x05];//0�0805 ����������� ������� ����� �� 10��
		fs1.LowInsulationRes	 =(float)r[0x06]+(float)r[0x07]/10000.0f; 	//0�0806..7 ����� ���� ����� �������� ��� ����.���


		fs1.LevelSensorSwichPnt		    =r[0x08];//0�0808 ���� ����������� R��� ������ ���
		fs1.Reserve808				    =r[0x09];//0x0809  ������
		fs1.DifferentialTransformer     =r[0x0A];//0�080A �������� ����������������� ENABLE/DISABLE
		fs1.GrundLeak_mA           	    =r[0x0B];//0�080B ����� ������ �� �����, ��
		fs1.DeviceMode                  =r[0x0C];//0x080C ��������� ���������� 1...5
		fs1.AssemblyMode		        =r[0x0D];//0x080D ��������� ������� 0..8

		fs1.ProductionMode 			    =r[0x0E];//0x080E 0-���������, 1 �������, 2 ������� 2-� �������,
											// 3 ������� 3-� �������,  4 - 2 ������ + ������
		fs1.ProtectionPlus			    =r[0x0F];//0x080f ���������� ������ ENABLE/DISABLE

  }
  break;
  case READ_FACILITY_SETTING2:{
  //READ_FACILITY_SETTING2{
	  volatile fSet2f_t fs2={0};
		fs2.amperageAclbr		=(float)r[0x00]+(float)r[0x01]/10000.0f;// 0�0820
		fs2.clbr_iA      		=(float)r[0x02]+(float)r[0x03]/10000.0f;// 0�0822

		fs2.amperageBclbr		=(float)r[0x04]+(float)r[0x05]/10000.0f;// 0�0824
		fs2.clbr_iB				=(float)r[0x06]+(float)r[0x07]/10000.0f;// 0�0826

		fs2.amperageCclbr		=(float)r[0x08]+(float)r[0x09]/10000.0f;// 0�0828
		fs2.clbr_iC				=(float)r[0x0A]+(float)r[0x0B]/10000.0f;// 0�082A

		fs2.amperageTrclbr		=(float)r[0x0C]+(float)r[0x0D]/10000.0f;// 0�082C
		fs2.clbr_iTr			=(float)r[0x0E]+(float)r[0x0F]/10000.0f;// 0x082E

		fs2.voltageCclbr		=(float)r[0x10]+(float)r[0x11]/10000.0f;// 0�0830
		fs2.clbr_uC				=(float)r[0x12]+(float)r[0x13]/10000.0f;// 0�0832

		fs2.insulationRes		=(float)r[0x14]+(float)r[0x15]/10000.0f;// 0�0834
		fs2.clbrResIns			=(float)r[0x16]+(float)r[0x17]/10000.0f;// 0�0836

		fs2.clbrADD				=(float)r[0x18]+(float)r[0x19]/10000.0f;// 0x0838..9

		fs2.phasing				=(float)r[0x1A]+(float)r[0x1B]/10000.0f;// 0x083A

		fs2.reserve3B			=(float)r[0x1C]+(float)r[0x1D]/10000.0f;// 0x083B
		fs2.reserve3C			=(float)r[0x1E]+(float)r[0x1F]/10000.0f;// 0x083C

		fs2.reserve3E           =(float)r[0x20]+(float)r[0x21]/10000.0f;// 0x083E
		fs2.reserve3F           =(float)r[0x22]+(float)r[0x23]/10000.0f;// 0x083F
  	  }
      break;
   default:{}
   }/* end wsitch*/
 }/* end Ox03 */

















typedef struct{
	uint16_t iMax;
	uint16_t a;
	uint16_t b;
	uint16_t c;
} ki_t;

ki_t ET_AmpSens[6]={
	{0,                0,  0,  0 },
//  ��������� � ������������ �������� ���� x=1..5
//  �����/10   ����.   �   �   �
    {240	/* x=1 */, 26, 26, 26},
    {750	/* x=2 */, 26, 26, 26},
    {4000	/* x=3 */, 26, 26, 26},
    {5500	/* x=4 */, 26, 26, 26},
    {330	/* x=5 */, 26 ,26, 26}
};



/* ������ ������ � ��������� �� ���������� ������� 412 */
i412limit_t i412limit[9]={
//����.  ����.
//����	 ����
//���/10 ���/10
{20,   	 170}, /* ��������� ���� 412 0*/
{60,	 240}, /*                     1*/
{100,	 620}, /*                     2*/
{200,	 900}, /*                     3*/
{300,	1100}, /*                     4*/
{500,	2500}, /*                     5*/
{500,	3800}, /*                     6*/
{1000,	5700}, /*                     7*/
{9999,	9999}   /*                    8*/
};





//HBRUSH CreateMyBrush( 	/* r g b*/
//  COLORREF crColor  = 0x00FF0000 // ��� ����� �����
//  return
//);

void f_412dist_Init(void){

	 //	     	// ������� ������ ��� ��������� �����, ���:
	 //	     	// 0..3 ����� ������, ��������  0 - ������������
	 //	     	//					 			1 - ������
	 //	     	//                   			2 - ����������� ������������
	 //	     	//								3 - ������������ ������
	 //	     	//                              4 - ��������� ������������
	 d.d_regime      = ct_unknown;     	//								1 - ������� �����
	 //	     	   5     ��� ������             0 - ������ �����
	 //	     								    1 - ������� �����
	 d.d_oper        = op_unknown;
	 /* ��� ������� �������������� */
	 d.d_motorfault  = 0xFF;
	 /* �������� ������������ ��� ������� ��� ������������� ������ */
	 d.pressure_sensor=     sensor_unknown;

}


int f_ReadUS_fromTK4(user_settings_t *husfm){
	return 0;
}
int f_WriteUS_toTK4_Memory(user_settings_t *hustm){
	return 0;
}
int f_WriteUS_toTK4_Flash(user_settings_t *hustf){
	return 0;
}

int f_ReadFS1_fromTK4(fSet1_t *husfm){}
int f_WriteFS1_toTK4_Memory(fSet1_t *hustm){}
int f_WriteFS1_toTK4_Flash(fSet1_t *hustf){}

int f_ReadFS2_fromTK4(fSet2f_t *husfm){}

int f_WriteFS2_toTK4_Memory(fSet2f_t *hustm){
	fSet2f_t fs2tm={0};
	memcpy(&fs2tm,hustm,sizeof(fSet2f_t));

    uint16_t fs2_16[0x20]={0};
//	float amperageAclbr; 	// 0�0820
	fs2_16[0x00]=fs2tm.amperageAclbr_d;
	fs2_16[0x01]=fs2tm.amperageAclbr_fr;

//	float clbr_iA;			// 0�0822
	fs2_16[0x02]=fs2tm.clbr_iA_d;
	fs2_16[0x03]=fs2tm.clbr_iA_fr;

//	float amperageBclbr;	// 0�0824
	fs2_16[0x04]=fs2tm.amperageBclbr_d;
	fs2_16[0x05]=fs2tm.amperageBclbr_fr;
//	float clbr_iB;			// 0�0826
	fs2_16[0x06]=fs2tm.clbr_iB_d;
	fs2_16[0x07]=fs2tm.clbr_iB_fr;

//	float amperageCclbr;    // 0�0828
	fs2_16[0x08]=fs2tm.amperageCclbr_d;
	fs2_16[0x09]=fs2tm.amperageCclbr_fr;
//	float clbr_iC;	        // 0�082A
	fs2_16[0x0A]=fs2tm.clbr_iC_d;
	fs2_16[0x0B]=fs2tm.clbr_iC_fr;

//	float amperageTrclbr;	// 0�082C
	fs2_16[0x0C]=fs2tm.amperageTrclbr_d;
	fs2_16[0x0D]=fs2tm.amperageTrclbr_fr;
//	float clbr_iTr;			// 0x082E
	fs2_16[0x0E]=fs2tm.clbr_iTr_d;
	fs2_16[0x0F]=fs2tm.clbr_iTr_fr;

//	float voltageCclbr;		// 0�0830
	fs2_16[0x10]=fs2tm.voltageCclbr_d;
	fs2_16[0x11]=fs2tm.voltageCclbr_fr;
//	float clbr_uC;			// 0�0832
	fs2_16[0x12]=fs2tm.clbr_uC_d;
	fs2_16[0x13]=fs2tm.clbr_uC_fr;

//	float insulationRes;	// 0�0834
	fs2_16[0x14]=fs2tm.insulationRes_d;
	fs2_16[0x15]=fs2tm.insulationRes_fr;

//	float clbrResIns;		// 0�0836
	fs2_16[0x16]=fs2tm.clbrResIns_d;
	fs2_16[0x17]=fs2tm.clbrResIns_fr;

//	float clbrADD;			// 0x0838..0x0839
	fs2_16[0x18]=fs2tm.clbrADD_d;
	fs2_16[0x19]=fs2tm.clbrADD_fr;


	fs2_16[0x1A]=fs2tm.phasing;		// 0x083A

	fs2_16[0x1B]=0x083B;

	fs2_16[0x1C]= 0x083C;

	fs2_16[0x1D]=0x083D;

	fs2_16[0x1E]=0x083E;

	fs2_16[0x1F]=0x083F;

	 char buffer[256]={0};
	 GetCurrentDirectory(sizeof(buffer),buffer);

	 /* ���������� ����� */
	    const char *first  = "first";
	    const char *second = "second";

	    char third[512];
	    snprintf(third, sizeof third, "%s%s", buffer, ":\\fs2.txt");

     //FILE *fp2;
	    fp4 = fopen("D:\\tk412_fs2.txt", "w+");
//	 fp4 = fopen(/*"D:\\C_OUTPUT.txt"*/ third, "w+");
	 for(uint16_t i=0;i<0x20;i++){
		 fprintf(fp4,"%4x  ",fs2_16[i]);

	 }
	 fclose (fp4);
	 fp4 = fopen(/*"D:\\C_OUTPUT.txt"*/ third, "r");
	 uint16_t fs2_16fm[0x20]={0};

	 for(uint16_t i=0;i<0x20;i++){
		 fscanf(fp4,"%4x  ",fs2_16fm[i]);

	 }
	 fclose (fp4);

}
int f_WriteFS2_toTK4_Flash(fSet2f_t *hustf){}
