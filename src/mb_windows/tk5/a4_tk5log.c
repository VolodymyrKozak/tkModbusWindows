/*
 * a4_tk2log.c
 *
 *  Created on: 6 ��. 2020 �.
 *      Author: KozakVF
 */
#include "a4_tk5log.h"
#include "windows.h"
#include <stdio.h>


/* �� ������� ������� ����� ���� ��� ����.
 * ����������� ����� ���*/
char tk5_project_index[10]="tk5";
char tk5_logFileName[256]={0};

FILE* f_tk5log_Test(void){
	FILE *fp0;
	fp0=f_tklog_Init("tk5", tk5_logFileName);
	FILE *fp1;
	fp1=f_tkcheck_logFilename(fp0, "tk5", tk5_logFileName);
	return fp1;
}

/* �� ������� ������� � ������� ����� ���� ��� ������ ����.
 * ����������� ��� �������� ����� ������������� ��������� ��2 */
FILE* f_tk5log_Init(void){
	FILE *fl=NULL;
	fl=f_tklog_Init("tk5",tk5_logFileName);
	return fl;
}

/* �� ������� �������� �� �� �������� ����� ���� �,
 * ���� ��������, ������� ������ ���� ���� � ������� ���������,
 * ������� �������� ��� ����������� ��'� ����������� ����� */
FILE* f_tk5check_logFilename(FILE *fp){
	FILE *fl=NULL;
	fl=f_tkcheck_logFilename(fp,"tk5",tk5_logFileName);
	return fl;
}

 //fclose(fp_tk2log);
