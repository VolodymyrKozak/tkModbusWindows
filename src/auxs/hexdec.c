/*
 * hexdec.c
 *
 *  Created on: 10 груд. 2019 р.
 *      Author: KozakVF
 */

#include "stdint.h"
#include "hexdec.h"
#include <stdio.h>
#include <string.h>

/* Ця домоміжна функція виводить на екран MessageBox,
 * повідомлення про помилку при зчитуванні цілого чи float
 * з EditBox  */
int f_valueEditMessageBox(int err){
	char um[256]={0};
	switch(err){
	case 1:{snprintf(um,43," Зустріли недозволений символ             ");}break;
	case 2:{snprintf(um,43," Ціла частина числа більша за  65535      ");}break;
	case 3:{snprintf(um,43," Дробна частина числа більша за  65535    ");}break;
	case 4:{snprintf(um,43," Пустий рядок                             ");}break;
	default:{}
	}
	const int result = MessageBox(NULL,um,"Помилка",MB_OK);
	switch (result){case IDOK:{}break;default:{}}
	return 0;
}


/*Ця функція зчитує число з плаваючою точкою з заданого EditBar
 * id_LABEL - ідентифікатор EditBar,
 * hwnd -вікно, де цей EditBar
 * f2 - показчик на число з плаваючою точкою, яке повертається
 *  * повертається:
 * 0, якщо успіх,
 * 1- якщо зустріли недозволений символ
 * 2- якщо ціла частина більша за 0хFFFF
 * 3- якщо дробна частина більша за 0xFFFF
 * 4- пустий рядок
 * */
int f_checkEdit(int id_LABEL, HWND hwnd, float2_t *f2){
	int err= -100;
	int n_strValue1 = GetWindowTextLength(GetDlgItem(hwnd, id_LABEL));
//                        	int err= GetLastError();
	if(n_strValue1==0){
		err=4;
	}
	else {
 		char str[256]={0};
		GetDlgItemText(hwnd, id_LABEL, str, n_strValue1+1);

		err = f_strint_to_value(
					str,
					n_strValue1,
					f2
					);
		n_strValue1=0;


	}
	return err;
}
/* Ця функція перетворює рядок символів в число з плаваючою крапкою в форматі
 * float2_t або повертає код помилки
 * string - рядок символів
 * n_string - розмір рядка символів, число перших символів, які беремо до уваги
 * float2 - число з плаваючою точкою, що повертається в разі успіху
 * повертається:
 * 0, якщо успіх,
 * 1- якщо зустріли недозволений символ
 * 2- якщо ціла частина більша за 0хFFFF
 * 3- якщо дробна частина більша за 0xFFFF
 * */

int f_strint_to_value(char *string, uint8_t n_string, float2_t *float2){
	int err=0;
	uint8_t  dotFlag=0;
	uint32_t nDec=0;
	uint64_t nFrc=0;
	uint32_t  iFrc=1000;
	static float2_t r={0};
	for(uint8_t i=0;i<n_string;i++){
		uint8_t ch=string[i];
		if((ch<0x30)||(ch>0x39)){
			if(ch=='.'){
			dotFlag=1;
			}
			else if(ch==' '){;}
			else{
				err=1;/* error - недозволений символ */
			}
		}
		/* чергова цифра не крапка */
		else{

			/* це ціла частина?*/
			if(dotFlag==0){
				uint8_t d=ch-0x30;
				nDec=nDec*10+(uint32_t)d;
				if(nDec>0xFFFFF){
					err=2;/*Надто велика ціла частина*/
				}
			}
			/*це дробна частина */
			else{
				uint8_t f=ch-0x30;
				nFrc=nFrc+iFrc*f;
				if(iFrc>1){
					iFrc=iFrc/10;
				}
				else{
					iFrc=0;
				}

				if(nFrc>0xFFFFF){
					err=3;/*Надто велика дробна частина*/
				}
			}
		}
	}
	if(err==0){
		r.d=nDec;
		if(nFrc>10000){
			r.f=nFrc / 10000;
		}
		else{
			r.f=(uint16_t)nFrc;
		}
		r.floatf = (float)r.d+(float)r.f / 10000.0f;
		(*float2)=r;
	}
	return err;
}
/* Ця функція тестує рядок байтів ec16s на наявність числа до 0xFFFF
 * якщо довжина рядка <6 байт
 * і це послідовність hex- цифр в діапазоні від 0 до F
 * функція повертає число 0..0xFFFF,якщо Ок
 * -1 і  повідомлення for_user_msg,  якщо містить число > 0xFFFF
 * -2 і  повsдомлення for_user_msg якщо це чотири знаки, але не hex */
int f_check_hex(char *hex16s, char *for_user_msg){
	if((hex16s==NULL)||(for_user_msg==NULL)){
		return -3;
	}
	int output=0;
	int length=strlen(hex16s);
//	if(length != 4){
//		snprintf (for_user_msg,40,"HEX містить не чотири знаки");
//		output=-1;
//	}
//	else{

		char signValue=0;
		for(uint8_t i=0;i<length;i++){
			uint8_t byte = hex16s[i];

			if((byte>=0x30)&&(byte<=0x39)){signValue=byte-0x30;}
			else if (byte=='A'){signValue=10;}
			else if (byte=='B'){signValue=11;}
			else if (byte=='C'){signValue=12;}
			else if (byte=='D'){signValue=13;}
			else if (byte=='E'){signValue=14;}
			else if (byte=='F'){signValue=15;}
			else if (byte=='a'){signValue=10;}
			else if (byte=='b'){signValue=11;}
			else if (byte=='c'){signValue=12;}
			else if (byte=='d'){signValue=13;}
			else if (byte=='e'){signValue=14;}
			else if (byte=='f'){signValue=15;}
			else if (byte==' '){signValue=0;}
			else if ((byte=='x')||(byte=='X')){
		    	snprintf (for_user_msg,40,"Ведіть HEX без '0х' ");
		    	output=-2;
			}
			else{
		    	snprintf (for_user_msg,40,"Це не HEX");
		    	output=-2;
		    	break;
			}
			output=output*0x10+signValue;

//		}

	}
	return output;
}
int f_check_dec(char *dec16s, char *for_user_msg){
	if((dec16s==NULL)||(for_user_msg==NULL)){
		return -3;
	}
	int output=0;
	int length=strlen(dec16s);
	if(length>5){snprintf (for_user_msg,40,"Надвелике число");output= -2;}
	else{
		for(int i=0;i<length;i++){
			char byte=dec16s[i];
			if((byte>=0x30)&&(byte<=0x39)){
				output=output*10+(byte-0x30);
			}
			else if(byte!=' '){
				;
			}
			else{
				output=-1;
				snprintf (for_user_msg,40,"Це не DEC");break;
			}
		}
		if(output>0x10000){
			snprintf (for_user_msg,40,"Надвелике число");
			output=-1;
		}

	}
	return output;
}
