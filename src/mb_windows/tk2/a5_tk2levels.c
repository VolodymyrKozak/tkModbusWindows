/*
 * a5_tk2levels.c
 *
 *  Created on: 7 січ. 2020 р.
 *      Author: KozakVF
 */
#include "a5_tk2levels.h"


//extern uint32_t tk2Barrel;
static uint16_t barrel_pro=0;
brl_t brl={0};

/* приватні функції для обчислення значення процес-баром подачі рідини */
static int f_h_barrel(brl_t *pb);
static int f_lh_barrel(brl_t *pb);
static int f_hl_barrel(brl_t *pb);
static int f_l_barrel(brl_t *pb);


uint16_t get_BarrrelPro(void){
	return barrel_pro;
}

/* Ця функція повертає статус функціонування насоса на основі
 * величини відліків АЦП на вході контролера від датчиків
 * НИЖНЬОГО РІВНЯ і ВЕРХНЬОГО РІВНЯ.
 * Параметри:
   pumping_mode: Вид роботи тк2:

          	case 0:Відкачка, датчики рівня              (ОЭ)
          	case 1:Подача, ЕКМ                          (ПЕ)
          	case 2:Подача, датчики рівня                (ПЭ)
          	case 3:Подача, реле тиску/поплавковий датчик(ПП)
          	case 4:Відкачка, поплавковий датчик         (ОП)
          	case 5:Подача, датчик тиску                 (ПО)
   ll_adc - відліки АЦП від датчика нижнього рівня
   hl_adc - відліки АЦП від датчика верхнього рівня        */
tk2_pumping_t  f_get_pumping_status(
		uint16_t pumping_mode,
		uint16_t ll_adc,
		uint16_t hl_adc,
		brl_t *pbrl
		){
	tk2_pumping_t tp = UnknownLvl;
  	switch(pumping_mode){
  	//Відкачка, датчики рівня              (ОЭ)	                          //1234567890123456789012345678901234567890123
  	case 0:{}break;
  	//Подача, ЕКМ                          (ПЕ)
  	case 1:{}break;
  	//Подача, датчики рівня                (ПЭ)
  	case 2:{
  		/* датчик мокрий, якщо ацп < 1000      */
  		if ((ll_adc<=1000)&&(hl_adc<=1000)){
	  			/* Бочка повна */
  			tp=HighLevel;
	  		if( tp!= pbrl->Old){
	  			/* Подія - tk2 мусив вимкнути мотор */
	  			pbrl->Event=HighLevel;
	  			barrel_pro=f_h_barrel(pbrl);
	  		}
	  		pbrl->Old=tp;

	  	}
  		//нижній датчик мокрий, верхній сухий
	  	else if ((ll_adc<=1000)&&(hl_adc>1000)){
	  		//b)насос не працює, рівень рідини спадає по мірі споживання
	  		if(pbrl->Event==HighLevel){
	  			tp=HighToLow;
	  			barrel_pro=f_hl_barrel(pbrl);
	  		}
	  	    //а) насос працює і подає рідину
	  		else if(pbrl->Event==LowLevel){
	  			tp=LowToHigh;
	  			barrel_pro=f_lh_barrel(pbrl);
	  			}
	  		else{}
	  		pbrl->Old=tp;

	  	}
  		//обидва датчики сухі  - нижній рівень,
	  	else if ((ll_adc>1000)&&(hl_adc>1000)){
	  		tp=LowLevel;
	  		if( tp!= pbrl->Old){
	  			pbrl->Event=LowLevel;
	  			barrel_pro=f_l_barrel(pbrl);
	  		}
	  		pbrl->Old=tp;
	  	}
  		//помилка датчиків - верхній мокрий. коли нижній сухий
	  	else if ((ll_adc>1000)&&(hl_adc<1000)){
	  		tp=level_sensor_error;
	  	}
	  	else{}
  	}
  	break;

  	//Подача, реле тиску/поплавковий датчик(ПП)
  	case 3:{}break;
  	//Відкачка, поплавковий датчик         (ОП)
  	case 4:{}break;
  	//Подача, датчик тиску                 (ПО)
  	case 5:{}break;
  	default:{}
  	}
  	return tp;
}

static int f_h_barrel(brl_t *pb){

	pb->lh.Sum+=pb->lh.Time;  //LowToHighSum+=HighToLowTime;
	pb->lh.Cntr++;//LowToHighCntr++;
	pb->lh.Ave=pb->lh.Sum / pb->lh.Cntr; //LowToHighAve=LowToHighSum / LowToHighCntr;

	pb->lh.Time=0;
	return 100;
}
static int f_lh_barrel(brl_t *pb){
	pb->lh.Time++;
	if(pb->lh.Ave>10){
		return pb->lh.Time * 100 / pb->lh.Ave;
	}
	else{
		return 50;
	}
}
static int f_hl_barrel(brl_t *pb){
	pb->hl.Time++;
	if(pb->hl.Ave>10){
		return pb->hl.Time * 100 / pb->hl.Ave;
	}
	else{
		return 50;
	}
}
static int f_l_barrel(brl_t *pb){
	pb->hl.Sum+=pb->hl.Time;  //LowToHighSum+=HighToLowTime;
	pb->hl.Cntr++;//LowToHighCntr++;
	pb->hl.Ave=pb->hl.Sum / pb->hl.Cntr; //LowToHighAve=LowToHighSum / LowToHighCntr;
	pb->hl.Time=0;
	return 0;
}





