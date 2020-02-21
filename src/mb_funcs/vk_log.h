/*
 * vk_log.h
 *
 *  Created on: 7 січ. 2020 р.
 *      Author: KozakVF
 */

#ifndef SRC_MB_FUNCS_VK_LOG_H_
#define SRC_MB_FUNCS_VK_LOG_H_
#include <stdio.h>


/* Ця функція створює і відкриває новий файл для запису логів.
 * Викликається при загрузці софту дистанційного управління ТК2
 * і кожен раз при початку нової доби
 * повертає посилання на файл і імя файла*/
FILE * f_tklog_Init(char* project_index, char *filename);

/* Ця функція перевіряє чи не наступив новий день і,
 * якщо наступив, закриває денний файл логів і відкриває попередній */
FILE* f_tkcheck_logFilename(FILE *pfc, char* project_index, char *filename);

/* Ця функція створює і відкриває новий файл для збереження налаштувань.
 * Викликається при загрузці софту дистанційного управління ТК2
 * і кожен раз при початку нової доби */
int f_tkSetFiles_Init(char* project_index, char *filename, int device_mode);

/* Ця функція створює і відкриває новий файл для збереження налаштувань.
 * Викликається при загрузці софту дистанційного управління ТК2
 * і кожен раз при початку нової доби */
int f_tkSetFilesReadSrch(int device_mode, char *filename);

#endif /* SRC_MB_FUNCS_VK_LOG_H_ */
