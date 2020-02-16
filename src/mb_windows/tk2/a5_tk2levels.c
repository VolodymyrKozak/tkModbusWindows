/*
 * a5_tk2levels.c
 *
 *  Created on: 7 ��. 2020 �.
 *      Author: KozakVF
 */
#include "a5_tk2levels.h"


//extern uint32_t tk2Barrel;
static uint16_t barrel_pro=0;
brl_t brl={0};

/* ������� ������� ��� ���������� �������� ������-����� ������ ����� */
static int f_h_barrel(brl_t *pb);
static int f_lh_barrel(brl_t *pb);
static int f_hl_barrel(brl_t *pb);
static int f_l_barrel(brl_t *pb);


uint16_t get_BarrrelPro(void){
	return barrel_pro;
}

/* �� ������� ������� ������ �������������� ������ �� �����
 * �������� ����� ��� �� ���� ���������� �� �������
 * �������� в��� � ��������� в���.
 * ���������:
   pumping_mode: ��� ������ ��2:

          	case 0:³������, ������� ����              (��)
          	case 1:������, ���                          (��)
          	case 2:������, ������� ����                (��)
          	case 3:������, ���� �����/����������� ������(��)
          	case 4:³������, ����������� ������         (��)
          	case 5:������, ������ �����                 (��)
   ll_adc - ����� ��� �� ������� �������� ����
   hl_adc - ����� ��� �� ������� ��������� ����        */
tk2_pumping_t  f_get_pumping_status(
		uint16_t pumping_mode,
		uint16_t ll_adc,
		uint16_t hl_adc,
		brl_t *pbrl
		){
	tk2_pumping_t tp = UnknownLvl;
  	switch(pumping_mode){
  	//³������, ������� ����              (��)	                          //1234567890123456789012345678901234567890123
  	case 0:{}break;
  	//������, ���                          (��)
  	case 1:{}break;
  	//������, ������� ����                (��)
  	case 2:{
  		/* ������ ������, ���� ��� < 1000      */
  		if ((ll_adc<=1000)&&(hl_adc<=1000)){
	  			/* ����� ����� */
  			tp=HighLevel;
	  		if( tp!= pbrl->Old){
	  			/* ���� - tk2 ����� �������� ����� */
	  			pbrl->Event=HighLevel;
	  			barrel_pro=f_h_barrel(pbrl);
	  		}
	  		pbrl->Old=tp;

	  	}
  		//����� ������ ������, ������ �����
	  	else if ((ll_adc<=1000)&&(hl_adc>1000)){
	  		//b)����� �� ������, ����� ����� ����� �� �� ����������
	  		if(pbrl->Event==HighLevel){
	  			tp=HighToLow;
	  			barrel_pro=f_hl_barrel(pbrl);
	  		}
	  	    //�) ����� ������ � ���� �����
	  		else if(pbrl->Event==LowLevel){
	  			tp=LowToHigh;
	  			barrel_pro=f_lh_barrel(pbrl);
	  			}
	  		else{}
	  		pbrl->Old=tp;

	  	}
  		//������ ������� ����  - ����� �����,
	  	else if ((ll_adc>1000)&&(hl_adc>1000)){
	  		tp=LowLevel;
	  		if( tp!= pbrl->Old){
	  			pbrl->Event=LowLevel;
	  			barrel_pro=f_l_barrel(pbrl);
	  		}
	  		pbrl->Old=tp;
	  	}
  		//������� ������� - ������ ������. ���� ����� �����
	  	else if ((ll_adc>1000)&&(hl_adc<1000)){
	  		tp=level_sensor_error;
	  	}
	  	else{}
  	}
  	break;

  	//������, ���� �����/����������� ������(��)
  	case 3:{}break;
  	//³������, ����������� ������         (��)
  	case 4:{}break;
  	//������, ������ �����                 (��)
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





