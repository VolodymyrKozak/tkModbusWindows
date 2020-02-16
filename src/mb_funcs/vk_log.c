/*
 * vk_log.c
 *
 *  Created on: 7 січ. 2020 р.
 *      Author: KozakVF
 */
#include <windows.h>

#include "vk_log.h"



/* Дата і час старту програми */
static WORD currentDay ={0};



/* Ця функція створює і відкриває новий файл для запису логів.
 * Викликається при загрузці софту дистанційного управління ТК2
 * і кожен раз при початку нової доби */
FILE * f_tklog_Init(char* project_index, char *filename){
    /* Рядок, куди зчитується поточна директорія*/
	char cd[256]={0};
	/* Зчитуємо в рядок поточну директорію */
	GetCurrentDirectory(254,cd);


    SYSTEMTIME  SystemTime ={0};
    LPSYSTEMTIME lpSystemTime=&SystemTime;

    GetSystemTime(lpSystemTime);
    char st[256]={0};
    snprintf(st,23,"\\%slog%04d_%02d_%02d.log",project_index, lpSystemTime->wYear,lpSystemTime->wMonth,lpSystemTime->wDay);
    /* Отримуємо повне ім'я файла для запису логів  */
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

/* Ця функція перевіряє чи не наступив новий день і,
 * якщо наступив, закриває денний файл логів і відкриває попередній */
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

