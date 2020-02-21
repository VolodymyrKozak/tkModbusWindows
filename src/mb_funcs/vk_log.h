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
 * � ����� ��� ��� ������� ���� ����
 * ������� ��������� �� ���� � ��� �����*/
FILE * f_tklog_Init(char* project_index, char *filename);

/* �� ������� �������� �� �� �������� ����� ���� �,
 * ���� ��������, ������� ������ ���� ���� � ������� ��������� */
FILE* f_tkcheck_logFilename(FILE *pfc, char* project_index, char *filename);

/* �� ������� ������� � ������� ����� ���� ��� ���������� �����������.
 * ����������� ��� �������� ����� ������������� ��������� ��2
 * � ����� ��� ��� ������� ���� ���� */
int f_tkSetFiles_Init(char* project_index, char *filename, int device_mode);

/* �� ������� ������� � ������� ����� ���� ��� ���������� �����������.
 * ����������� ��� �������� ����� ������������� ��������� ��2
 * � ����� ��� ��� ������� ���� ���� */
int f_tkSetFilesReadSrch(int device_mode, char *filename);

#endif /* SRC_MB_FUNCS_VK_LOG_H_ */
