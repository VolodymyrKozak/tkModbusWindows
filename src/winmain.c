/*
 ============================================================================
 Name        : 199802_Win0TST.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include "windows.h"

#include "mb_devs/tk2_slave3_1.h"
#include "mb_devs/demo_smi2.h"
#include "mb_funcs/tkRS232rx.h"



//#define  ANY_TEST
/* ������� ����� � ������ */
#define WINDOWS_T
//#define DEBUG_SMI2


//#include "mb_devs/tk2_mb_queue.h"

int main(void){
	puts("!!!Hello World!!!"); /* :) prints !!!Hello World!!! */
//	test_queue();


#ifdef DEBUG_TK2_SLAVE3_1
	//f_dIscrete_events();
	//tk2Test();
#endif
#ifdef DEBUG_SMI2

	f_set_rs232rx(&r);
	cport_nr=6;
	/* ���� ����� ������ ������ ������ ��� ��� */
//	f_start_test_smi2();
	/* ������ */
//	f_demo_smi();
//	f_oven320_Tasks ();
//	_beginthread((void*)f_demo_smi, 0, NULL);

#endif //	DEBUG_SMI2


		HINSTANCE tkhInstance = NULL;
		LPSTR lpCmdLine = GetCommandLine();
		int nCmdShow=SW_SHOWMAXIMIZED;

		/*http://www.firststeps.ru/mfc/winapi/win/r.php?103
		nCmdShow o���������, ��� ���� ������ ���� ��������.
		���� �������� ����� ���� ����� �� ��������� ��������:
		SW_HIDE - �������� ���� � ������������ ������ ����.
		SW_MINIMIZE - 	������������ ������������ ���� � ������������ ���� �������� ������
						� ������ �������.
		SW_RESTORE - 	������������ � ���������� ����. ���� ���� �������������� ��� ����������,
						Windows ��������������� ��� � �������������� ������� � �������
						(�� �� �����, ��� � SW_SHOWNORMAL).
		SW_SHOW - 		������������ � ���������� ���� �� ������ � ��� ������� ������� � �������.
		SW_SHOWMAXIMIZED - ������������ ���� � ���������� ��� ��� ����������� ����.
		SW_SHOWMINIMIZED - ������������ ���� � ���������� ��� ��� �����������.
		SW_SHOWMINNOACTIVE - ���������� ���� ��� �����������. �������� ���� �������� ��������.
		SW_SHOWNA - 	���������� ���� � ��� ������� ���������. �������� ���� �������� ��������.
		SW_SHOWNOACTIVATE - ���������� ���� � ��� ����� ��������� �������� � �������.
							�������� ���� �������� ��������.
		SW_SHOWNORMAL - ������������ � ���������� ����. ���� ���� �������������� ��� ����������,
						Windows ��������������� ��� � �������������� ������ � �������
						(�� �� �����, ��� � SW_RESTORE).*/

		/* �������� �� COM-����� ��� ����, ��� ���������� ������ ������� ��� ���� */
		/* �� ����� ������������ ��� ������ ���� ������ ��������� �����.
		 ���� ������������ �����, ��� ���������� ������ �� ���� ���������, ���� a2_setting.h*/
//#ifndef PASSWORDTEST

		f_get_rs232rxCompotrts();

		WinMain(
				tkhInstance,// ��������� �� ������� ���������
				NULL, 		// ��������� �� ���������� ���������� ���������
				lpCmdLine, 	// ��������� ������
				nCmdShow	// ��� ����������� ���� ���������
		);




#ifdef ANY_TEST

		LPCSTR hh = f_Register_PasswordWndClass(tkhInstance);
		WinMainTest (				tkhInstance,// ��������� �� ������� ���������
				NULL, 		// ��������� �� ���������� ���������� ���������
				lpCmdLine, 	// ��������� ������
				nCmdShow	// ��� ����������� ���� ���������
		);
#endif



	return EXIT_SUCCESS;
}
