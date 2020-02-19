/*
 * tkCRC.h
 *
 *  Created on: 15 ����. 2017 �.
 *      Author: Volodymyr.Kozak
 */

#ifndef TKCRC_H_
#define TKCRC_H_
#include "tw_mb.h"
#include "stdint.h"

modbus_status_t f_add_CRC(modbus_master_tx_msg_t *tx);
modbus_status_t f_check_CRC(modbus_master_rx_msg_t *rx);
uint16_t CRC_16x (uint8_t *Buffer, uint8_t length);

/* �� ������� ���������� ����� � ��������� �������
 * �� ����������� �������� ���� � ��������� ����������� ������
 * ������� 0�03
 * ��������� � �������� ������ ����� + 3
 * (0-����� ������,1-��� �������, 2 - ������� ����)
 * ������� �������:
      0, ���� ��� ��
     -1, ���� �� �� ��������� ��� ����� ����� �� ������� 4    */
int8_t f_03_float_to_Tx(
		float af,                  // ������ �����
		modbus_master_tx_msg_t *tx, // �������� �� ������� ����������� ������
		uint16_t i);               // ����� �����, � ����� �������� ����� float � ������ �����

/* �� ������� ���������� ����� � ��������� �������
 * �� ����� � ���� ����� - ����� ������� float
 * �� ������ ������� float ����� ������ �������� �����
 * ��������� =
 * 		������ ����� float af
 * 		�������� �� ����� ���� ����� int16_t
 * ������� �������
 *      0, ���� ��� ��
 *      -1,���� ����� ����� ������ ��� ���� ��������� �� �����*/
int8_t f_float_to_2x16(float af, uint16_t *ptwo);


#endif /* TKCRC_H_ */
