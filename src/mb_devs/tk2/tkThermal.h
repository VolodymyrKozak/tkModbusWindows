/*
 * tkThermal.h
 *
 *  Created on: 10 вер. 2017 р.
 *      Author: Volodymyr.Kozak
 */

#ifndef TKTHERMAL_H_
#define TKTHERMAL_H_
#include "../../wConfig.h"
#define tk4_FREQUENCY 10


#endif /* TKTHERMAL_H_ */





extern struct trm_t TRM;


float f_tk2Thermal_Motor_Model (
		float iAmperage_f,
		float FullLoadAmperes_f,
		on_off_t MotorState
		);

