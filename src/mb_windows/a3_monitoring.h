/*
 * 1main_paint.h
 *
 *  Created on: 26 лип. 2019 р.
 *      Author: KozakVF
 */

#ifndef WIN32APP_API_1MAIN_PAINT_H_
#define WIN32APP_API_1MAIN_PAINT_H_
#include     <windows.h>
#include    "stdint.h"
#include "../wConfig.h"
#define Y_HEADER   100
#define Y_PASSPORT 150
#define Y_STRING    25
#define X_TIME     120
#define X_BYTE      25

typedef enum {
	mb_mon_presetting,
	mb_mon_setted,
	mb_mon_starting,
	mb_mon_started,
	mb_mon_poused,
	mb_mon_stopped,

	mb_manual_dialog,

	mb_tk412

}mb_mon_t;
extern mb_mon_t mb_mon;



/* Після запуску користувачем процедури моніторинку мережі Модбас (кнопка Старт основного меню)
 * як паралельний Windows-процес викликаєтсья ця функція f_main_paint(), у якій:
 *
 * 1. Відкриваєтьсь вибраний раніше СОМ-порт f_OpenComport_forMB(), файл 'tkRS232rx.c/h'
 * 2. Після заповнення буфера СОМ-порта f_rs232rx_buf_poll()
 * 2.1. Кожне повідоблення Модбас записується у базу даних повідомлень,
 *      де для кожної адреси Модбас зберігається чотири останніх повідомлень
 *      f_crc_search() файл 'tkRS232rx.c/h' ->  f_write_to_passport(), файл mb_base.c/h
 * 2.2. На основне вікно виводяться отримані модбас повідомлення
 * 		групами по чотири повідомлення від кожного окремого Slave
 * 		у порядку зростання адреси Slave
 * 		f_paint_modbuss_msg()
 * 2.3.	У кожній групі у кожній парі двох послідовних повідомлень шукаються байти,
 * 		що не співпадають (за умов співпадаючої команди Модбас)
 *      f_paint_modbuss_msg() -> check_coincidental_psp(), файл mb_base.c/h
 *                                         |
 *                                         V і виділяються в вікні іншим кольором
 *                               f_printColorByte2(),цей файл


 * */


void f_main_paint(void);

/* Паралельний Windows-роцес припиняється при виборі пункту мменю Стоп, або при закритті вікна.
* При цьому закривається СОМ-порт після вибору пункту меню Стоп*/
void f_main_paint_clouse(void);
void f_paint_modbuss_msg(int istr_file);

void f_printColorByte(
		int iPassport,
		int iMSG,
		int iByte,
		char Byte,
		HBRUSH hbr
		);


void f_printColorByte2(
		int iPassport, /* індекс паспорта, 0..127              */
		int iMSG,      /* індекс повідомлення в паспорті, 0..3 */
		int iByte,     /* індекс байта, який треба підсвітити  */
		char Byte,     /* значення байта, який треба підсвітити*/
		HBRUSH hbr     /* колір підсвітки                      */
		);

int f_tm(void);
#endif /* WIN32APP_API_1MAIN_PAINT_H_ */
