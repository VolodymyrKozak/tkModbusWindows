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
 * і кожен раз при початку нової доби */
FILE * f_tklog_Init(char* project_index, char *filename);

/* Ця функція перевіряє чи не наступив новий день і,
 * якщо наступив, закриває денний файл логів і відкриває попередній */
FILE* f_tkcheck_logFilename(FILE *pfc, char* project_index, char *filename);

#endif /* SRC_MB_FUNCS_VK_LOG_H_ */
