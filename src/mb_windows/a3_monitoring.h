/*
 * 1main_paint.h
 *
 *  Created on: 26 ���. 2019 �.
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



/* ϳ��� ������� ������������ ��������� ���������� ����� ������ (������ ����� ��������� ����)
 * �� ����������� Windows-������ ����������� �� ������� f_main_paint(), � ���:
 *
 * 1. ³���������� �������� ����� ���-���� f_OpenComport_forMB(), ���� 'tkRS232rx.c/h'
 * 2. ϳ��� ���������� ������ ���-����� f_rs232rx_buf_poll()
 * 2.1. ����� ����������� ������ ���������� � ���� ����� ����������,
 *      �� ��� ����� ������ ������ ���������� ������ ������� ����������
 *      f_crc_search() ���� 'tkRS232rx.c/h' ->  f_write_to_passport(), ���� mb_base.c/h
 * 2.2. �� ������� ���� ���������� ������� ������ �����������
 * 		������� �� ������ ����������� �� ������� �������� Slave
 * 		� ������� ��������� ������ Slave
 * 		f_paint_modbuss_msg()
 * 2.3.	� ����� ���� � ����� ��� ���� ���������� ���������� ��������� �����,
 * 		�� �� ���������� (�� ���� ���������� ������� ������)
 *      f_paint_modbuss_msg() -> check_coincidental_psp(), ���� mb_base.c/h
 *                                         |
 *                                         V � ���������� � ��� ����� ��������
 *                               f_printColorByte2(),��� ����


 * */


void f_main_paint(void);

/* ����������� Windows-����� ������������ ��� ����� ������ ����� ����, ��� ��� ������� ����.
* ��� ����� ����������� ���-���� ���� ������ ������ ���� ����*/
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
		int iPassport, /* ������ ��������, 0..127              */
		int iMSG,      /* ������ ����������� � �������, 0..3 */
		int iByte,     /* ������ �����, ���� ����� ��������  */
		char Byte,     /* �������� �����, ���� ����� ��������*/
		HBRUSH hbr     /* ���� �������                      */
		);

int f_tm(void);
#endif /* WIN32APP_API_1MAIN_PAINT_H_ */
