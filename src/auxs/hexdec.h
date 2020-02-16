/*
 * hexdec.h
 *
 *  Created on: 10 ����. 2019 �.
 *      Author: KozakVF
 */

#ifndef SRC_AUXS_HEXDEC_H_
#define SRC_AUXS_HEXDEC_H_
#include "windows.h"

typedef struct{
	int d;			/* ���� ������� float  */
	int f;			/* ������ ������� float*/
	float    floatf;/* ����� float         */
}float2_t;



/* �� �������� ������� ���������� ��� ���������� ����� DEC ����
 * �� ����� ��䳿 */
int f_checkEdit(int id_LABEL, HWND hwnd, float2_t *f2);

/* �� ������� ���������� ����� ������� � ����� � ��������� ������� � ������
 * float2_t ��� ������� ��� �������
 * string - ����� �������
 * n_string - ����� ����� �������, ����� ������ �������, �� ������ �� �����
 * float2 - ����� � ��������� ������, �� ����������� � ��� �����
 * �����������:
 * 0, ���� ����,
 * 1- ���� ������� ������������ ������
 * 2- ���� ���� ������� ����� �� 0�FFFF
 * 3- ��� ������ ������� ����� �� 0xFFFF
 * */
int f_strint_to_value(char *string, uint8_t n_string, float2_t *float2);

/* �� �������� ������� �������� �� ����� MessageBox,
 * ����������� ��� ������� ��� ��������� ������ �� float
 * � EditBox  */
int f_valueEditMessageBox(int err);


/* �� ������� ����� ����� ����� ec16s �� �������� ����� �� 0xFFFF
 * ���� ������� ����� <6 ����
 * � �� ����������� hex- ���� � ������� �� 0 �� F
 * ������� ������� ����� 0..0xFFFF,���� ��
 * -1 �  ����������� for_user_msg,  ���� ������ ����� > 0xFFFF
 * -2 �  ���s�������� for_user_msg ���� �� ������ �����, ��� �� hex */
int f_check_hex(char *hex16s, char *for_user_msg);
int f_check_dec(char *dec16s, char *for_user_msg);
#endif /* SRC_AUXS_HEXDEC_H_ */
