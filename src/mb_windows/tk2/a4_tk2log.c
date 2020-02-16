/*
 * a4_tk2log.c
 *
 *  Created on: 6 ��. 2020 �.
 *      Author: KozakVF
 */
#include "a4_tk2log.h"
#include "windows.h"
#include <stdio.h>



/* �� ������� ������� ����� ���� ��� ����.
 * ����������� ����� ���*/
char tk2_project_index[10]="tk2";
char tk2_logFileName[256]={0};

FILE* f_tklog_Test(void){
	FILE *fp0;
	fp0=f_tklog_Init("tk2", tk2_logFileName);
	FILE *fp1;
	fp1=f_tkcheck_logFilename(fp0, "tk2", tk2_logFileName);
	return fp1;
}

/* �� ������� ������� � ������� ����� ���� ��� ������ ����.
 * ����������� ��� �������� ����� ������������� ��������� ��2 */
FILE* f_tk2log_Init(void){
	FILE *fl=NULL;
	fl=f_tklog_Init("tk2",tk2_logFileName);
	return fl;
}

/* �� ������� �������� �� �� �������� ����� ���� �,
 * ���� ��������, ������� ������ ���� ���� � ������� ���������,
 * ������� �������� ��� ����������� ��'� ����������� ����� */
FILE* f_tk2check_logFilename(FILE *fp){
	FILE *fl=NULL;
	fl=f_tkcheck_logFilename(fp,"tk2",tk2_logFileName);
	return fl;
}

 //fclose(fp_tk2log);
