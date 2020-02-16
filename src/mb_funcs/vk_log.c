/*
 * vk_log.c
 *
 *  Created on: 7 ��. 2020 �.
 *      Author: KozakVF
 */
#include <windows.h>

#include "vk_log.h"



/* ���� � ��� ������ �������� */
static WORD currentDay ={0};



/* �� ������� ������� � ������� ����� ���� ��� ������ ����.
 * ����������� ��� �������� ����� ������������� ��������� ��2
 * � ����� ��� ��� ������� ���� ���� */
FILE * f_tklog_Init(char* project_index, char *filename){
    /* �����, ���� ��������� ������� ���������*/
	char cd[256]={0};
	/* ������� � ����� ������� ��������� */
	GetCurrentDirectory(254,cd);


    SYSTEMTIME  SystemTime ={0};
    LPSYSTEMTIME lpSystemTime=&SystemTime;

    GetSystemTime(lpSystemTime);
    char st[256]={0};
    snprintf(st,23,"\\%slog%04d_%02d_%02d.log",project_index, lpSystemTime->wYear,lpSystemTime->wMonth,lpSystemTime->wDay);
    /* �������� ����� ��'� ����� ��� ������ ����  */
    strcat(filename,cd);
    strcat(filename,st);

    FILE *pf=NULL;
    if (( pf = fopen(filename, "a"))==NULL) {
    	printf("Cannot open file.\n");
    	exit(1);
    }
//    fp_tk2log = fopen(filename, "w+");

    fprintf(pf,"\n%s",filename);
    fprintf(pf,"\n=========================================================================================================");
    fprintf(pf,"\n\n");

    currentDay=lpSystemTime->wDay;

return pf;
}

/* �� ������� �������� �� �� �������� ����� ���� �,
 * ���� ��������, ������� ������ ���� ���� � ������� ��������� */
FILE* f_tkcheck_logFilename(FILE *pfc, char* project_index, char *filename){
    SYSTEMTIME  SystemTime ={0};
    LPSYSTEMTIME lpSystemTime=&SystemTime;
	GetSystemTime(lpSystemTime);
	FILE *pf = NULL;
	if(lpSystemTime->wDay == currentDay){
		pf=pfc;
	}
	else if(lpSystemTime->wDay != currentDay){
		fclose(pfc);
		pf=f_tklog_Init(project_index, filename);
	}
	return pf;
}

