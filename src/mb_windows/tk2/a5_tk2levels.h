/*
 * a5_tk2levels.h
 *
 *  Created on: 7 ��. 2020 �.
 *      Author: KozakVF
 */

#ifndef SRC_MB_WINDOWS_TK2_A5_TK2LEVELS_H_
#define SRC_MB_WINDOWS_TK2_A5_TK2LEVELS_H_
#include "stdint.h"


/* ��� ��� ����� ������� ���� ������� ������ ������.
 * ����. �� ������ ����� �� ���� ��� �������:
 * ������ ������� ����  - ������ �����,
 * ������ ������� ���� - ������� �����,
 * ������ ������ ������, ������� �����:
 * �) ����� ������ � ���� �����
 * �) ����� �� ������, ����� ����� ����� �� �� ����������
 */
typedef enum{
	//������ ������� ���� - ������� �����,
	HighLevel = 0x22,
	//������ ������ ������, ������� �����
	//�) ����� ������ � ���� �����
	LowToHigh = 0x02,
	//������ ������ ������, ������� �����:
	//����� �� ������, ����� ����� ����� �� �� ����������
	HighToLow = 0x20,
	//������ ������� ����  - ������ �����,
	LowLevel  = 0x11,
	/* ������ �����, ������� ������ - ������� */
	level_sensor_error = 0xFF,
	/*����� ��������*/
	UnknownLvl= 0x00
}tk2_pumping_t;

/* ���������� ��������� ��� �������������� ���� ���� � ��������� */
typedef struct {
	uint64_t Time;  /* ��������� ����������/����������� ����������, �������� */
	uint64_t Cntr;  /* ˳������� ����� */
	uint64_t Sum;   /* ���� ����������� */
	uint64_t Ave;   /* ��������� ����������/����������� ����������, �������
	 	 	 	 	   �� ���� ��� �������� ����� */
}lvl_t;

/* ������ � ��������� � ������-����� ���������� ���������� */
typedef struct  {
	lvl_t lh;  /* ���������� ��� ������ � ���������� ���������� */
	lvl_t hl;  /* ���������� ��� ������ � ������������ ���������� */

	tk2_pumping_t Old;   /* ���� ������� ����������*/
	tk2_pumping_t Event; /* ���� ������� - ����*/
}brl_t;

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
		uint16_t lh_adc,
		uint16_t hl_adc,
		brl_t *pbrl
		);

uint16_t get_BarrrelPro(void);
/*******************************************************/

#endif /* SRC_MB_WINDOWS_TK2_A5_TK2LEVELS_H_ */
