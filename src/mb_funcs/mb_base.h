/*
 * mb_base.h
 *
 *  Created on: 24 ���. 2019 �.
 *      Author: KozakVF
 */

#ifndef MB_MONITOR_MB_BASE_H_
#define MB_MONITOR_MB_BASE_H_
#include "stdint.h"
#include "windows.h"
#include <commctrl.h>

/***************************************************************************************************************
 * 1. ����������� ������, �� ������� � ����� ���������� �� ��������.
 * 2. �� ����� ����� ���������� ���������� ���������/������� passport
 * 3. � ������� ���������� ���� � �������� ������� ���������� �� ���� �� ��������.
 * ****************
 * ���� � ������� �� ���� � �� � �������
 * � ������������ ���� � 򳺿 � �������
 * � ��������� ����� ������� ������
 * � ��������� ����� - ���� ����������.
 *
 * *************************************************************************************************************/
typedef struct{
	uint8_t last_msg[256];
	uint8_t n_msg;
	SYSTEMTIME last_activity_time;
}msg_info_t;


typedef struct {
	uint8_t address;
	uint32_t intensity;
	msg_info_t MSG[4];
}mb_passport_t;
extern mb_passport_t mb_passport[256];
extern uint8_t iMBP;
extern uint32_t msg_count;
extern uint8_t nBase;
int16_t f_passportInit();
int16_t f_add_to_BASE(uint8_t addr, uint8_t line[]);

void check_coincidental_psp(int psp_index);
/*
1) ������� � ������� ����� ������ - �� ���������..
2) ���� ���� � ��� ����������..
   ��� ������� � ������ ��������� �� ����������..
   ���� ������ ������� � ������ ��������� �� ������, � ���� �������
   ���� ������ ���� ������� � ������� ��������� -

   �� ���.
   ���� �� ���� �����������..
   ���� ������ ������� � ������� ��������� �� ������ ������
   ���� ��� ����� �� ����� �� ����������..

*/

#endif /* MB_MONITOR_MB_BASE_H_ */
