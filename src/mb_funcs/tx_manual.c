/*
 * tx_manual.c
 *
 *  Created on: 27 вер. 2019 р.
 *      Author: Volodymyr.Kozak
 */
#include "../mb_funcs/tx_manual.h"

#include <stddef.h>
#include "string.h"

#include "../mb_funcs/tkCRC.h"
#include "../mb_funcs/tkRS232rx.h"
/* */

static uint8_t f_ASCII_to_HEX(uint8_t ch);

/* Повідомлення без CRC у текстовій формі ASCII трансформується у повідомлення HEX без CRC */
lt_status_t MB_TxRxManual(uint8_t *hmsg, uint16_t size,	modbus_master_tx_msg_t *hmbTxMsg){
	modbus_master_tx_msg_t mbTxMsg={0};

	if(hmsg==NULL){return lt_ZeroPtr;}
	if (size==0) {return lt_Zerosize;}
	uint8_t length = 0;
	uint8_t ni=size/3;

    for(uint16_t i=0;i<ni;i++){
    	char ch16=hmsg[i*3];
    	char ch1 =hmsg[i*3+1];
    	char ch_ =hmsg[i*3+2];
    	uint8_t c16 = f_ASCII_to_HEX(ch16);
    	uint8_t c1  = f_ASCII_to_HEX(ch1);
    	uint8_t byte =0;
    	if ((c16==0xFF)||(c1==0xFF)) {return 	lt_notHEX;}
    	else{
    		byte = c16*0x10+c1;
    	}
    	if((ch_==' ')||(ch_==',')||(ch_=='\0')||(ch_=='\n')||(ch_=='\r')){
    		mbTxMsg.msg[length]=byte;
    		length++;
    	}
    	else {return lt_byteWrong;}
    	if((ch_=='\0')||(ch_=='\n')||(ch_=='\r')){
    		break;
    	}
    }
	mbTxMsg.length=length;
	memcpy(hmbTxMsg,&mbTxMsg,sizeof(modbus_master_tx_msg_t));
	return lt_OK;
}

static uint8_t f_ASCII_to_HEX(uint8_t ch){
	uint8_t byte=0xFF;
	switch(ch){
		case 0x30: byte = 0; break;
		case 0x31: byte = 1; break;
		case 0x32: byte = 2; break;
		case 0x33: byte = 3; break;
		case 0x34: byte = 4; break;
		case 0x35: byte = 5; break;
		case 0x36: byte = 6; break;
		case 0x37: byte = 7; break;
		case 0x38: byte = 8; break;
		case 0x39: byte = 9; break;
		case 0x61:case 0x41: byte = 0x0A; break;
		case 0x62:case 0x42: byte = 0x0B; break;
		case 0x63:case 0x43: byte = 0x0C; break;
		case 0x64:case 0x44: byte = 0x0D; break;
		case 0x65:case 0x45: byte = 0x0E; break;
		case 0x66:case 0x46: byte = 0x0F; break;
		default:            byte =0xFF;
	}
	return byte;
}
int16_t f_mbSPrint(uint8_t *buf, int16_t n, uint8_t *outbuf){
	uint16_t ns=0;
	if((buf==NULL) || (outbuf==NULL)){return -1;}
	  for (uint16_t i=0;i<n;i++){
		  outbuf[3*i]=f_xprint(buf[i]/0x10);
		  outbuf[3*i+1] =f_xprint(buf[i]%0x10);
		  outbuf[3*i+2] =' ';
	  }
	  return ns;
}
