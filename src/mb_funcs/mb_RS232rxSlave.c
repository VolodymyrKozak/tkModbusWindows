/*
 * mb_RS232rxSlave.c
 *
 *  Created on: 23 жовт. 2019 р.
 *      Author: KozakVF
 */
#include "tkRS232rx.h"
/* Тут функції отримання модбас повідомлення на PC, на відміну від моніторингу
 * відразу ж як визначено повідомлення від Майстера повинний бути перехід
 * на підготовку відповіді */
	  extern mb_buf[256];
	  extern unsigned char buf[4096];
	  extern uint16_t n;
	  extern int bsize;
	  extern int cport_nr;
      extern int bdrate;       /* 9600 baud */
	  extern char mode[];
//	  extern FILE *fp;

	  uint16_t dbI;
	  uint16_t dbY;

/* Приймаємо черговий кусок на вході Модбас і шукаємо там повідомлення */
crc_search_t f_AskMasterMB(modbus_master_rx_msg_t *prx_msg){
	/* Відкриваємо компорт на читання */
	int16_t ocom = RS232_OpenComportRead(cport_nr, bdrate, mode);
	crc_search_t CRCSS = crcs_Unknown;
	//f_rs232rx_mbSlave(modbus_master_rx_msg_t *rx_msg);
	/* Крутимося поки там щось не знайшли схоже на повідомлення */
	while (CRCSS == crcs_Unknown){
		CRCSS =  f_rs232rx_mbSlave(prx_msg);
	}
    /* Закриваємо компорт і повертаємо повідомлення на обробку */
	RS232_CloseComport(cport_nr);
	return CRCSS;

}

crc_search_t f_rs232rx_mbSlave(modbus_master_rx_msg_t *rx_msg){
	/* ВІДКРИТТЯ СОМ-порту */
	 crc_search_t crcss = crcs_Unknown;
//	  Sleep(200);
	  int8_t output=0;
	  /* Отримуємо щось в буфер  */
	  n = RS232_PollComport(cport_nr, buf+bsize, 4095-bsize);

//		  n= 30;
//		  n = n_residue+RS232_PollComport(cport_nr, buf+n_residue, 4095-n_residue);
	  int m=0;
	  if (n!=0){
		 m=n+bsize;
	  }
	  buf[m] = 0;   /* always put a "null" at the end of a string! 			*/
	  if((m) > 0){
		  bsize=0;
		  /* Шукаємо в буфері повідомлення Модбас */
		  if ((m)>2){
			  int16_t bm=0; /* підозрюваний початок повідомлення Модбас */
			  int16_t eM=2; /* підозрюваний кінець повідомлення Модбас  */
			  /* Зовнішній цикл - кінець повідомлення модбас - це кінець повідомлення */

			  while ((eM<m)){
				  /* перевіряємо підозру на те, в куску буфера є повідоблення Модбас
				   * в функції перевіряються всі варіанти:
				   * кінець куска - фіксований, еМ
				   * початок - різний, від кінця до bm                                */

				  crcss=f_crc_searchSlave (
						  buf,
						  bm,
						  eM,
						  rx_msg
						  );
				  /* якщо десь є програмна помилка */
				  if (crcss==crcs_Error){
					  rx_msg->length=0xFF; break;
				  }
				  /* Якщо даний кусок буфера не не містить повідомлення Модбас -
				   * інкрементуємо кінець куска буфера*/
				  else if (crcss==crcs_not_found){
					  eM++;
				  }
				  /* Якщо даний кусок буфера містить повідомлення Модбас або
				   * повідомлення модбас з куском битого тексту попереду  -
				  	 переходимо до наступного куска буфера       				   */
				  else if ((crcss==crcs_full_found)||
						  (crcss==crcs_crc_and_wrongbytes_found)){

					  bm=eM+1;
					  eM=eM+3;
				  }
				  else{}

			  }//while
			  if (crcss==crcs_not_found){
				  /* Якщо дійшли до кінця тексту  в буфері, і не знайшли повідомлення,
				     переписуємо цей кусок на початок буфера, адже
				     можливо що це початок наступного повідомлення, кінцева частина якого
				     знайдеться в наступнуму тексті буфера, отриманого з СОМ-порту   * */
				  dbY++;
				  bsize=eM-bm;
				  if (bsize>256){
					  memcpy (mb_buf, buf+eM-256, 256);
//					  f_mbPrint(buf+bm, bsize-256);
//					  f_mbPrint(mb_buf, 256);
				  }
				  else{
					   memcpy (mb_buf, buf+bm, bsize);
//					   f_mbPrint(mb_buf, bsize);
				  }
				  memset (buf, 0, sizeof(buf));
				  memcpy (buf, mb_buf, bsize);
				  memset (mb_buf, 0, sizeof(mb_buf));
			  }
			  else {
				  memset (buf, 0, sizeof(buf));
			  }

		  } // if (n>2){
	} ///if(n > 0){

	return crcss;
}
//typedef enum {
//	crcs_Error,
//	crcs_Unknown,
//	crcs_not_found,
//	crcs_full_found,
//	crcs_crc_and_wrongbytes_found,
//}crc_serch_t;
//crc_search_t crc_search_state = crcs_Unknown;
crc_search_t f_crc_searchSlave (
		uint8_t *buf,
		int16_t bi,
		int16_t ei,
		modbus_master_rx_msg_t *msg
		){
	uint16_t msg_size=0;
	if (ei-bi<2){return crcs_Error;}
	crc_search_t cs = crcs_not_found;
	for (int16_t i=ei-2;i>=bi;i--){
	  int8_t iYes = false;
	  iYes =  f_crcOK (
				buf,
				i,
				ei
				);
	       if (iYes== -1    ){cs=crcs_Error;break;	}
	  else if (iYes==  false){/*нічого*/}
	  else if (iYes==  true ){
		  /* Якщо знайшли в буфері повідомлення МОДБАС - роздруковуємо його*/
		  if (i==bi){
			  cs=crcs_full_found;
			  /* ідентифіковане Модбас-повідомлення */
			  msg_size = ei-bi+1;
//			  f_mbPrint(buf+bi, msg_size);
			  memcpy(msg->msg,buf+bi,msg_size);
			  msg->length=msg_size;
			  if (msg_size>256){
				  cs = crcs_not_found;
			  }
		  }
		  else if (i>bi){
			  cs=crcs_crc_and_wrongbytes_found;
			  /* частина байт - не ідентифіковані як Модбас повідомлення*/
//			  uint16_t umsg_size = i-bi+1;
//			  f_mbPrint(buf+bi,  umsg_size);
			  /* ідентифіковане Модбас-повідомлення */
			  uint16_t imsg_size = ei-i+1;
//			  f_mbPrint(buf+bi, imsg_size);
			  if (imsg_size>256){
			 	  cs = crcs_not_found;
			  }
			  else {
//				  f_mbPrint(buf+i, ei-i+1);
//				  f_write_to_passport (buf+i,ei-i+1);
				  msg_size=ei-i+1;
				  memcpy(msg->msg,buf+bi,msg_size);
				  msg->length=msg_size;
			  }
		  }
		  else{}
	  }
	}
	return cs;
}

