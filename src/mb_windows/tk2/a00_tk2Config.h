/*
 * a00_tk2Config.h
 *
 *  Created on: 9 ��. 2020 �.
 *      Author: KozakVF
 */

#ifndef SRC_MB_WINDOWS_TK2_A00_TK2CONFIG_H_
#define SRC_MB_WINDOWS_TK2_A00_TK2CONFIG_H_

/************************************************************************************************************
 *  ����� ����˲� �� � ��� ������ֲ����� �����˲��� PC_DESK <-> TK2
 * ** ********************************************************************************************************
 *  ����                    �����������
 *  ----------------------------------------------------------------------------------------------------------
 *  ����� mb_windows/tk2:
 *  a20_tk2.c/h				������� ���� - ��������� �� �� ���������� ���������
 *  a21_tk2us.c/h			����� ����   - ������������ �����������
 *  a22_tk2fs.c/h           ���� ����   - ������������ ���������
 *  a3_tk2TxRx.c/h  		����� ���������� � �� �������: �������� �� ������, ��������� ������� ������
 *  a3_tk2z.c/h             ���� �������, �� � ������� ��� ����� ����
 *  a4_tk2log./h			������� ����. ����� ���� � ����, ����� - ����� ����.
 *  a5_tk2levels.c/h        ������ � ������ �� ������-����� ���������� ����������
 *
 *  ����� mb_windows
 *  a2_setting.c/h          ³��� ������������ ���-�����
 *  e1_passwordEdit.c/h     ³��� ����� ������, ����� � �������, ����� ����������� �����
 *  tkIconBitMask.c         ������ �������� ���������
 *
 *  ������� ���������
 *  ����� b_funcs
 *  vk_log.c/h              ����� � ������� ������ � ������, ����� ��'� ����� � �����, �� exe����, ����� ����� ����
 *  vk_wqueue.c/h           ������ � ������, ��-��������, �� ������� � �����(�� -����� :))
 *  tw_mb.c/h               ���� ������ ��� ���-�����, ��������������� �������� (����� RS232),
 *                          ����� ��� ������� - �� ������ �������, ���� ������ �, � ������ �� ����� �� �������
 *                          40�� ������ 1000�� ��������
 *  tkCRC.c/h               CRC
 *
 *  ������� ��������
 *  hexdec.c/h              ���������� �����-����� � ���-����� � ������������ � �����
 */

/*******************************************************************************************************************
 * �����������ʲ ��²�������� WINDOWS
 * ****************************************************************************************************************
 * ���������� ��� ���� �������������� ���������� Windows:
 *
 * VK_PSW                  (0xC001), ���� "e1_passwordEdit.c/h"
 * � ���� �� ����������� ���������� ���� ������ ������, ������� ����� ����������� ��������
							0
LRESULT SendMessage(
  HWND   hWnd,              - ����, � ����� ��� ����� �� ���� ������ � ���� ����������� �����������
  UINT   Msg,				- VK_PSW, ������������� �����������, �������� ����
  WPARAM wParam,            - �������� ������, �� ����-�� ����� 0�0000..0�FFFF � ����������� ������
  LPARAM lParam				- �� ���������������
);

��������, �� ������ ����������� � ����, ��� ������� �
�ase VK_PSW:
break;
 * ***************************************************************************************************************
  * VK_UPDATE_WINDOW                  (0xC002), ���� "a3_tk2TxRx.c/h"
 *  ���� ������, ���� � ����� ��������� �����������, ������� ����������� ��� ���������� ���,
 *  ��� ��������, �� ��� �������� �������� ��������� �� ������������ ��� ������
							0
LRESULT SendMessage(
  HWND   hWnd,              - ����, � ����� ��� ����� �� ���� ������ � ���� ����������� �����������
  UINT   Msg,				- VK_UPDATE_WINDOW, ������������� �����������, �������� ����
  WPARAM wParam,            - ������������� ���� ������ �����������, ���� ���������� � ����� ����� � ������������
  LPARAM lParam				- �� ���������������
);

��������, �� ������ ����������� � ����, ��� ������� �
�ase VK_UPDATE_WINDOW:
break;

 */


/* ����� ���������� ��� � ���� */
/* ��в��� �������� �����в� */
#define TIMER_MAIN_MS                   50  /* IDT_TIMER_TK2_10MS  */
#define TIMER_AUX_MS                    3000/* IDT_TIMER_TK2_1333MS */


//#define TIMER_PERIOD_TK2_10_MS     10
/* ����� �������� �������� ���������� ������ ���������
 * 5 ������ = TIMER_PERIOD_TK2_1000_MS � 5         */
#define TIMER_PERIOD_TK2_1000_MS   1000
//#define TIMER_PERIOD_TK2_1333_MS   1333



#endif /* SRC_MB_WINDOWS_TK2_A00_TK2CONFIG_H_ */
