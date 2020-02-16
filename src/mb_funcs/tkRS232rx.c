/*
 * tkRS232rx.c
 *
 *  Created on: 21 квіт. 2019 р.
 *      Author: Volodymyr.Kozak
 */

#include "tkRS232rx.h"

#include "windows.h"


	  extern char *comports[];
	  FILE *fp;


	  static SYSTEMTIME  SystemTime ={0};
	  static LPSYSTEMTIME lpSystemTime=&SystemTime;


//	  static uint16_t bB =0; /* Початок буфера для зчитування з СОМ-порту */
//	  static uint16_t bM =0; /* Початок виявленого Модбас-повідомлення    */
//	  static uint16_t eM =0; /* Кінець  виявленого Модбас-повідомлення    */




/************************************************************************************/
	uint16_t dbI=0;
	uint16_t dbY=0;
/************************************************************************************/


      int cport_nr=4;        /* /dev/ttyS0 (COM1 on windows) */
      int bdrate=9600;       /* 9600 baud */
	  char mode[4]={'8','N','1',0};
	  int COMPORTs_State[16]={0xFF};
	  unsigned char mb_buf[256]={0};
	  unsigned char buf[4096]={0};
	  uint16_t n=0;
	  int bsize = 0;

	  char sdate[12]={0};
	  char stime[12]={0};
	  comports_open_t po={0};
//	  rx232prm_t r={-1,1,1,0,0};
	  rx232prm_t r={-1,1,1,0,0};
//	  bool ComportsFixed=false;


	  void f_get_rs232rxCompotrts(void){
//		ComportsFixed=false;
	  	f_rs232rx_start();
	  	for (uint8_t i=0;i<16;i++){
	  		if (COMPORTs_State[i]==0){
	  			po.comprt[po.n]=i;
	  			po.n++;
	  		}
	  	}
	  }

int f_rs232rxMain(void){
	return 0;
}
int f_set_rs232rx(rx232prm_t *hr ){
	int icport = hr->xcport;
	cport_nr=po.comprt[icport];
	switch(hr->xbdrate){
//	    case     110 : strcpy(mode_str, "baud=110");
//	                   break;
//	    case     300 : strcpy(mode_str, "baud=300");
//	                   break;
//	    case     600 : strcpy(mode_str, "baud=600");
//	                   break;
//	    case    1200 : strcpy(mode_str, "baud=1200");
//	                   break;
//	    case    2400 : strcpy(mode_str, "baud=2400")	                   break;
	    case    0:bdrate=4800;
	            break;
	    case    1:bdrate=9600;
	                   break;
	    case    2:bdrate=19200;
	                   break;
	    case    3:bdrate=38400;
	                   break;
	    case    4:bdrate=57600;
	                   break;
	    case    5:bdrate=115200;
	                   break;
	    default  :bdrate=-1;
	}
	switch (hr->xnbits){
		case 0: mode[0]='9';
		  break;
		case 1: mode[0]='8';
          break;
		case 2: mode[0]='7';
          break;
		case 3: mode[0]='6';
          break;
		case 4: mode[0]='5';
		  break;
		default:mode[0]='-';
	}
	switch(hr->xparity){
		case    0:mode[1]='N';
        break;
		case    1:mode[1]='E';
               break;
		case    2:mode[1]='O';
               break;
		default  :mode[1]='-';
	}
	switch(hr->xstopbit){
    	case 0: mode[2]='1';
              break;
    	case 1: mode[2]='2';
              break;
    	default : mode[2]='-';
	}
	for (uint8_t i=0;i<16;i++){
		if (COMPORTs_State[i]==0){
			RS232_CloseComport(i);
			COMPORTs_State[i]=1;
		}
	}

	return 0;
}

rx232prm_t*  f_get_rs232rx(void ){

	f_get_rs232rxCompotrts();
//	for (uint8_t i=0;i<hpo->n;i++){
//
//	}
//	r.xcport=cport_nr;
	switch(bdrate){
//	    case     110 : strcpy(mode_str, "baud=110");
//	                   break;
//	    case     300 : strcpy(mode_str, "baud=300");
//	                   break;
//	    case     600 : strcpy(mode_str, "baud=600");
//	                   break;
//	    case    1200 : strcpy(mode_str, "baud=1200");
//	                   break;
//	    case    2400 : strcpy(mode_str, "baud=2400")	                   break;
	    case    4800 : r.xbdrate=0;
	                   break;
	    case    9600 : r.xbdrate=1;
	                   break;
	    case   19200 : r.xbdrate=2;
	                   break;
	    default      : r.xbdrate=-1;
	}
	switch (mode[0]){
		case '9': r.xnbits=0;
			  break;
    	case '8': r.xnbits=1;
              break;
    	case '7': r.xnbits=2;
              break;
    	case '6': r.xnbits=3;
              break;
    	case '5': r.xnbits=4;
    		  break;
    	default : r.xbdrate=-1;
	}
	switch(mode[1]){
    	case 'N':
    	case 'n': r.xparity=0;
              break;
    	case 'E':
    	case 'e': r.xparity=1;
              break;
    	case 'O':
    	case 'o': r.xparity=2;
              break;
    default : r.xparity=-1;
	}
	switch(mode[2]){
    	case '1': r.xstopbit=0;
              break;
    	case '2': r.xstopbit=1;
              break;
    	default : r.xstopbit=-1;
	}
	return &r;
}
int f_rs232rx(void){




//			  0x01,
//			  0x8F,
//			  0x02,
//			  0xC5, /* crc */
///* 4 */		  0xF1,	/* crc */
//
//			  	  0x00, /* musor */
//			  	  0x13,
//			  	  0x00,
//			  	  0x0A,
//			  	  0x02,
///* 10 */		  0xCD,
//
//	  0x01,
//	  0x0F,
//	  0x00,
//	  0x13,
//	  0x00,
//	  0x0A,
//	  0x02,
//	  0xCD,
//	  0x01,
//	  0x72, /* crc */
///*21*/0xCB, /* crc */
//
//	  0x01,
//	  0x0F,
//	  0x00,
//	  0x13,
//	  0x00,
//	  0x0A,
//	  0x24, /* crc */
///*29*/0x09,  /* crc */
//
///*30*/	  0x01, /* початок наступного повідомлення, яке не влізло в буфер */
///*31*/	  0x0F,
///*32*/	  0x00,
///*33*/	  0x13
///*n=34*/
//	  };
//      uint16_t n=34;
	  f_rs232rx_start();
	  /* На початку буфера може вже бути залишок від попереднього буфера,
	   * можливо початок повідомлення Модбас, яке не помістився в попередній буфер */

//	  while(1){
	  for (uint32_t iWhile=0;iWhile<2;iWhile++){
		  f_rs232rx_buf_poll();
	  }//while
	  f_rs232rx_stop();

  return(0);
}


int f_rs232rx_start(void){
	  fp = fopen("D:\\C_OUTPUT.txt", "w+");
	  uint16_t n=0;

	  GetSystemTime(lpSystemTime);
	  printf("\n%4d.%2d.%2d",
			  lpSystemTime->wYear,
	  	  	  lpSystemTime->wMonth,
	  	  	  lpSystemTime->wDay
	  	  	  );
	  fprintf(fp,"\n%4d.%2d.%2d",
			  lpSystemTime->wYear,
	  	  	  lpSystemTime->wMonth,
	  	  	  lpSystemTime->wDay
	  	  	  );
	  sprintf (sdate,"%4d.%2d.%2d",
			  lpSystemTime->wYear,
	  	  	  lpSystemTime->wMonth,
	  	  	  lpSystemTime->wDay
	  	  	  );
	  for (uint8_t icport_nr=0;icport_nr<16; icport_nr++){
		  if(RS232_OpenComport(icport_nr, bdrate, mode)){
			  printf("Can not open comport %s \n", comports[icport_nr]);
//			  printf("Can not open comport %2d \n", cport_nr);
//			  return(0);
			  COMPORTs_State[icport_nr]=1;
		  }
		  else {
			  COMPORTs_State[icport_nr]=0;
		  }
	  }
	  //cport_nr=3;
	  return 0;
}

int f_OpenComport_forMB(void){
	 RS232_OpenComport(cport_nr, bdrate, mode);
	 return 0;
}


int f_rs232rx_buf_poll_deInit(void){
	  memset(&mb_buf,0,sizeof(mb_buf));
	  memset(&buf,0,sizeof(buf));
	  bsize = 0;
	  memset(&sdate,0,sizeof(sdate));
	  memset(&stime,0,sizeof(stime));
	  f_passportInit();

	  RS232_CloseComport(cport_nr);
//	  fclose (fp);
	  return 0;
}

int f_rs232rx_buf_poll(void){
	  Sleep(6000);
//	  cport_nr=3;
	  /* Отримуємо і друкуємо все підряд */
	  n = RS232_PollComport(cport_nr, buf+bsize, 4095-bsize);

//		  n= 30;
//		  n = n_residue+RS232_PollComport(cport_nr, buf+n_residue, 4095-n_residue);
	  int m=0;
	  if (n!=0){
		 m=n+bsize;
	  }
	  buf[m] = 0;   /* always put a "null" at the end of a string! 			*/
	  if((m) > 0){
		  /* Друкуємо дату і час */
		  GetSystemTime(lpSystemTime);
		  printf ("\n%2d:%2d:%2d.%3d ",
		  lpSystemTime->wHour,
		  lpSystemTime->wMinute,
		  lpSystemTime->wSecond,
		  lpSystemTime->wMilliseconds
		  );
		  fprintf(fp,"\n%2d:%2d:%2d.%3d  ",
		  lpSystemTime->wHour,
		  lpSystemTime->wMinute,
		  lpSystemTime->wSecond,
		  lpSystemTime->wMilliseconds
		  );
		  sprintf(stime,"\n%2d:%2d:%2d.%3d  ",
		  lpSystemTime->wHour,
		  lpSystemTime->wMinute,
		  lpSystemTime->wSecond,
		  lpSystemTime->wMilliseconds
		  );
		  /* Друкуємо весь буфер */
		  f_mbPrint(buf, m);
		  printf ("COM-port buffer, n=%d, bsize=%d",n, bsize );
		  fprintf (fp,"COM-port buffer, n=%d, bsize=%d",n, bsize);
		  bsize=0;
		  /* Шукаємо в буфері повідомлення Модбас */
		  if ((m)>2){
			  int16_t bm=0; /* підозрюваний початок повідомлення Модбас */
			  int16_t eM=2; /* підозрюваний кінець повідомлення Модбас  */
			  /* Зовнішній цикл - кінець повідомлення модбас - це кінець повідомлення */
			  crc_search_t crcss = crcs_Unknown;
			  while ((eM<m)){
				  /* перевіряємо підозру на те, в куску буфера є повідоблення Модбас
				   * в функції перевіряються всі варіанти:
				   * кінець куска - фіксований, еМ
				   * початок - різний, від кінця до bm                                */

				  crcss=f_crc_search (
						  &buf,
						  bm,
						  eM);
				  /* якщо десь є програмна помилка */
				  if (crcss==crcs_Error){
					  printf ("\nERROR");
					  fprintf(fp,"\nERROR");
					  break;
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
					  f_mbPrint(buf+bm, bsize-256);
					  f_mbPrint(mb_buf, 256);
				  }
				  else{
					   memcpy (mb_buf, buf+bm, bsize);
					   f_mbPrint(mb_buf, bsize);
				  }

				  printf ("Remainder, bsize=%d", bsize);
				  fprintf (fp,"Remainder, bsize=%d", bsize);
				  memset (buf, 0, sizeof(buf));
				  memcpy (buf, mb_buf, bsize);
				  memset (mb_buf, 0, sizeof(mb_buf));
			  }
			  else {
				  memset (buf, 0, sizeof(buf));
			  }

		  } // if (n>2){
	} ///if(n > 0){
#ifdef _WIN32
//   Sleep(1000);
#else
usleep(100000);  /* sleep for 100 milliSeconds */
#endif

	return 0;
}
int f_rs232rx_stop(void){
	  fclose(fp);
	  RS232_CloseComport(cport_nr);
	  return 0;
}

char f_xprint(uint8_t inc){
	char outc = 0;
	if (inc>15)						{outc = '?';}
	else if ((inc>=0)&&(inc<10))	{outc = 0x30+inc;}
	else if ((inc==10))				{outc = 'A';}
	else if ((inc==11))				{outc = 'B';}
	else if ((inc==12))				{outc = 'C';}
	else if ((inc==13))				{outc = 'D';}
	else if ((inc==14))				{outc = 'E';}
	else if ((inc==15))				{outc = 'F';}
	else{}
	return outc;
}

//typedef enum {
//	crcs_Error,
//	crcs_Unknown,
//	crcs_not_found,
//	crcs_full_found,
//	crcs_crc_and_wrongbytes_found,
//}crc_serch_t;
//crc_search_t crc_search_state = crcs_Unknown;
crc_search_t f_crc_search (uint8_t *buf, int16_t bi, int16_t ei){
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
			  uint16_t msg_size = ei-bi+1;
			  f_mbPrint(buf+bi, msg_size);
			  f_write_to_passport (buf+bi,msg_size);
			  if (msg_size>256){
				  printf (" WrongMsg, nMB=%d", msg_size);
				  fprintf (fp, " WrongMsg, nMB=%d", msg_size);
				  cs = crcs_not_found;
			  }
			  printf (" ModbusMsg, nMB=%d", msg_size);
			  fprintf(fp,"ModbusMsg, nMB=%d", msg_size);

			  /* Якщо в базі вже є слейв з такою адресою, зафіксувати параметри повідомлення,
			   * якщо слейва з такою адресою не має - внести в базу і зафіксувати параметри */

		  }
		  else if (i>bi){
			  cs=crcs_crc_and_wrongbytes_found;
			  /* частина байт - не ідентифіковані як Модбас повідомлення*/
			  uint16_t umsg_size = i-bi+1;
			  f_mbPrint(buf+bi,  umsg_size);
			  printf ("WrongBytes, %d", umsg_size);
			  fprintf(fp,"WrongBytes,, %d", umsg_size);
			  /* ідентифіковане Модбас-повідомлення */
			  uint16_t imsg_size = ei-i+1;
			  f_mbPrint(buf+bi, imsg_size);
			  if (imsg_size>256){
				  printf (" WrongMsg, nMB=%d", imsg_size);
			 	  fprintf (fp, " WrongMsg, nMB=%d", imsg_size);
			 	  cs = crcs_not_found;
			  }
			  else {
				  f_mbPrint(buf+i, ei-i+1);
				  printf ("ModbusMsg, %d", ei-i+1);
				  fprintf(fp,"ModbusMsg, %d", ei-i+1);
				  f_write_to_passport (buf+i,ei-i+1);
			  }
		  }
		  else{}
	  }
	}
	return cs;
}


/* перевірка CRC для послідовності байт в буфері СОМ-порта
   buf       - показчик на буфер
   firstbyte - індекс першого байта з групи байт
   lastbyte  - індекс останнього байта, останній і передостанній байт - власне CRC
*/
int8_t f_crcOK (
		uint8_t  *buf,
		uint16_t firstbyte,
		uint16_t lastbyte
		){
	int8_t crcOK=false;
	if ((lastbyte-firstbyte)<2){
		return -1;
	}
	if (lastbyte>4095){
		return -1;
	}
	uint16_t LastByte = (uint16_t)buf[lastbyte];
	LastByte=LastByte*0x100;
	uint16_t PreLastByte=(uint16_t)buf[lastbyte-1];
	uint16_t readCRC=LastByte+PreLastByte;

	readCRC=buf[lastbyte]*0x100+buf[lastbyte-1];

	uint16_t calcCRC=CRC_16x (buf+firstbyte, lastbyte-firstbyte-1);
	if (readCRC==calcCRC){
		crcOK=true;
	}
	return crcOK;
}
/* Друкування послідовності байт в шістнадцятковій формі
 * buf - показчик на перший байт буфера
 * b   - зміщення, тобто кількість байт як пропускаємо
 * т   - кількість байт, які надрукуються   */
void f_mbPrint(uint8_t *buf, int16_t n){
	  printf ("\n");
	  fprintf(fp,"\n");
	  for (uint16_t i=0;i<n;i++){
		  char x10=f_xprint(buf[i]/0x10);
		  char x1 =f_xprint(buf[i]%0x10);
		  char comma =',';
	 //	  snprintf ((char*)outs+3*(i-k), 4, "%c%c%c",x10,x1,comma);
		  printf ("%c%c%c",x10,x1,comma);
		  fprintf(fp,"%c%c%c",x10,x1,comma);
	  }
}
