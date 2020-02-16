/*
 * a3_monitoring_scroll.c
 *
 *  Created on: 4 вер. 2019 р.
 *      Author: KozakVF
 */


#include "../mb_funcs/tkRS232rx.h"
#include "a3_monitoring.h"

extern int screen_heigh;
int n_str=0;
int firstbase=0;
int lastbase =0;
int f_tm(void){
	n_str =screen_heigh/Y_STRING;
	int r_str =0;
	if (nBase<5){
		r_str=n_str;
	}
	else {
		r_str=nBase*Y_PASSPORT/Y_STRING+Y_HEADER/Y_STRING;
	}
	return r_str;
}

int f_tm_1(int istr){
	if (nBase<0){
		firstbase=0;
		lastbase=nBase;
	}
	else{
		firstbase=istr;
	}
	return 0;
}

