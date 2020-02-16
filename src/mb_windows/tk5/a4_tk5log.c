/*
 * a4_tk2log.c
 *
 *  Created on: 6 січ. 2020 р.
 *      Author: KozakVF
 */
#include "a4_tk5log.h"
#include "windows.h"
#include <stdio.h>


/* Ця функція створює новий файл для логів.
 * Викликається кожен раз*/
char tk5_project_index[10]="tk5";
char tk5_logFileName[256]={0};

FILE* f_tk5log_Test(void){
	FILE *fp0;
	fp0=f_tklog_Init("tk5", tk5_logFileName);
	FILE *fp1;
	fp1=f_tkcheck_logFilename(fp0, "tk5", tk5_logFileName);
	return fp1;
}

/* Ця функція створює і відкриває новий файл для запису логів.
 * Викликається при загрузці софту дистанційного управління ТК2 */
FILE* f_tk5log_Init(void){
	FILE *fl=NULL;
	fl=f_tklog_Init("tk5",tk5_logFileName);
	return fl;
}

/* Ця функція перевіряє чи не наступив новий день і,
 * якщо наступив, закриває денний файл логів і відкриває попередній,
 * залиючи незмінним для користувача ім'я дескриптора файла */
FILE* f_tk5check_logFilename(FILE *fp){
	FILE *fl=NULL;
	fl=f_tkcheck_logFilename(fp,"tk5",tk5_logFileName);
	return fl;
}

 //fclose(fp_tk2log);
