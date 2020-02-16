/*
 * tkRS232rx.h
 *
 *  Created on: 1 серп. 2019 р.
 *      Author: KozakVF
 */

#ifndef MB_MONITOR_TKRS232RX_H_
#define MB_MONITOR_TKRS232RX_H_
#include <stdbool.h>
#include "mb_base.h"
#include "tkCRC.h"

#include "../mb_funcs/RS_232/rs232.h"
#include "../mb_funcs/tw_mb.h"
typedef struct {
	int xcport;
	int xbdrate;
	char xnbits;
	char xparity;
	char xstopbit;
}rx232prm_t;
extern rx232prm_t r;
typedef struct {
	int comprt[16];/* номери відкритих сомрортів 0=COM1*/
	int n;         /* кількість відкритих компортів    */
} comports_open_t;
extern comports_open_t po;
extern bool ComportsFixed;
typedef struct {
	unsigned char x10;
	unsigned char x;
	unsigned char comma;
} bytehex_t;
extern char sdate[12];
extern char stime[12];
typedef enum {
	crcs_Unknown,
	crcs_Error,
	crcs_not_found,
	crcs_full_found,
	crcs_crc_and_wrongbytes_found
}crc_search_t;

int f_rs232rxMain(void);
int f_rs232rx(void);
int f_rs232rx_start(void);
int f_rs232rx_buf_poll_deInit(void);
int f_OpenComport_forMB(void);
int f_rs232rx_buf_poll(void);
int f_rs232rx_stop(void);

int f_set_rs232rx(rx232prm_t *hr);
rx232prm_t*  f_get_rs232rx(void );
void f_get_rs232rxCompotrts(void);
char f_xprint(uint8_t inc);

void f_mbPrint(uint8_t *buf, int16_t n);
crc_search_t f_crc_search (uint8_t *buf, int16_t bi, int16_t ei);
/* перевірка CRC для послідовності байт в буфері СОМ-порта
   buf       - показчик на буфер
   firstbyte - індекс першого байта з групи байт
   lastbyte  - індекс останнього байта, останній і передостанній байт - власне CRC
*/
int8_t f_crcOK (
		uint8_t  *buf,
		uint16_t firstbyte,
		uint16_t lastbyte
		);

crc_search_t f_AskMasterMB(modbus_master_rx_msg_t *prx_msg);
crc_search_t f_rs232rx_mbSlave(modbus_master_rx_msg_t *rx_msg);
crc_search_t f_crc_searchSlave (
		uint8_t *buf,
		int16_t bi,
		int16_t ei,
		modbus_master_rx_msg_t *msg
		);


#endif /* MB_MONITOR_TKRS232RX_H_ */
