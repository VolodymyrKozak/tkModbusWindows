/*
 * tkCRC.h
 *
 *  Created on: 15 жовт. 2017 р.
 *      Author: Volodymyr.Kozak
 */

#ifndef TKCRC_H_
#define TKCRC_H_
#include "tw_mb.h"
#include "stdint.h"

modbus_status_t f_add_CRC(modbus_master_tx_msg_t *tx);
modbus_status_t f_check_CRC(modbus_master_rx_msg_t *rx);
uint16_t CRC_16x (uint8_t *Buffer, uint8_t length);

/* Ця функція перетворює число з плаваючою крапкою
 * на послідовність чотирьох байт в вихідному повідомлення Модбас
 * функції 0х03
 * починаючи з заданого номера байта + 3
 * (0-адрес модбас,1-код функції, 2 - кількість байт)
 * Функція повертає:
      0, якщо все Ок
     -1, якщо не має показчика або номер байта не кратний 4    */
int8_t f_03_float_to_Tx(
		float af,                  // вхідне число
		modbus_master_tx_msg_t *tx, // показчик на вихідне повідомлення Модбас
		uint16_t i);               // номер байта, з якого записати число float в чотири байти

/* Ця функція перетворює число з плаваючою крапкою
 * на масив з двох чисел - цілою частини float
 * та дробної частини float перші чотири десятичні цифри
 * Аргументи =
 * 		вхідне число float af
 * 		показчик на масив двох числе int16_t
 * Функція повертає
 *      0, якщо все Ок
 *      -1,якщо число надто велике або немає показчика на масив*/
int8_t f_float_to_2x16(float af, uint16_t *ptwo);


#endif /* TKCRC_H_ */
