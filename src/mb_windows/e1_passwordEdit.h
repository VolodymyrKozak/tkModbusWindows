/*
 * e1_passwordEdit.h
 *
 *  Created on: 14 ����. 2019 �.
 *      Author: KozakVF
 */

#ifndef SRC_MB_WINDOWS_E1_PASSWORDEDIT_H_
#define SRC_MB_WINDOWS_E1_PASSWORDEDIT_H_

/*
 * e1_password2.c
 *
 *  Created on: 12 ����. 2019 �.
 *      Author: KozakVF
 */
#include "windows.h"
#include "stdint.h"


#define VK_PSW                  0xC001    //�� 0xC000 �� 0xFFFF

#define IDE_PASSWORDEDIT     	2000
#define BUTTON_PSW_OK        	2001
#define BUTTON_PSW_CNSL			2002



/* �� ���� ����������� ������. �������� �������� ����� ����� ������ � �������
 * ������ - ����� ������� ����� 254.
 * ��������� ������ � �������� � ������ �������� - ���������� �� �� � - �������� ������������ ����
 * ��������� ����������� � ��������� ����� - ����� outPassword[256];
 * */

extern char outPassword[256];
LPCSTR f_Register_PasswordWndClass(HINSTANCE hinst);
HWND CreatePswWindow(HWND hwnd);
LRESULT CALLBACK WndPasswordProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);





#endif /* SRC_MB_WINDOWS_E1_PASSWORDEDIT_H_ */
