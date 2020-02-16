/*
 * tkThermal.c
 *
 *  Created on: 10 вер. 2017 р.
 *      Author: Volodymyr.Kozak
 */
#include <string.h>
#include <stdint.h> /*int32_t style anable*/
#include "tkThermal.h"
#include "../../wConfig.h"

/* Постійна часу теплової моделі = 700 сек =13.7 сек затримки відключення ЕД при І=7.2 Іном */




static float adjustedHeat_previous_heating=0;
float i2TimeConstant_s = 700.0f;
float tk2tm_FREQUENCY  = 1.0f/5.0f;


float f_tk2Thermal_Motor_Model (
		float iAmperage_f,
		float FullLoadAmperes_f,
		on_off_t MotorState
		) {


	float adjustedHeat_next=0;
	if ((iAmperage_f<0)|| (i2TimeConstant_s<=0)||(FullLoadAmperes_f<=0)) {return -2.0f;}
	else {
		if (MotorState==oN_){
		float adjustedAmperage =  iAmperage_f / FullLoadAmperes_f;
			adjustedHeat_next =adjustedHeat_previous_heating+
				( (adjustedAmperage* adjustedAmperage) - adjustedHeat_previous_heating  ) /
									 (i2TimeConstant_s * tk2tm_FREQUENCY) ;
			adjustedHeat_previous_heating=adjustedHeat_next;
		}
		else{
			if(adjustedHeat_previous_heating>0.00000001f){
				adjustedHeat_next =adjustedHeat_previous_heating -
					adjustedHeat_previous_heating   /   (3.0f*i2TimeConstant_s * tk2tm_FREQUENCY) ;
				adjustedHeat_previous_heating=adjustedHeat_next;
			}
		}
	}
	return adjustedHeat_next;
}



