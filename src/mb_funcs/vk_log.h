/*
 * vk_log.h
 *
 *  Created on: 7 ��. 2020 �.
 *      Author: KozakVF
 */

#ifndef SRC_MB_FUNCS_VK_LOG_H_
#define SRC_MB_FUNCS_VK_LOG_H_
#include <stdio.h>
/* �� ������� ������� � ������� ����� ���� ��� ������ ����.
 * ����������� ��� �������� ����� ������������� ��������� ��2
 * � ����� ��� ��� ������� ���� ���� */
FILE * f_tklog_Init(char* project_index, char *filename);

/* �� ������� �������� �� �� �������� ����� ���� �,
 * ���� ��������, ������� ������ ���� ���� � ������� ��������� */
FILE* f_tkcheck_logFilename(FILE *pfc, char* project_index, char *filename);

#endif /* SRC_MB_FUNCS_VK_LOG_H_ */
