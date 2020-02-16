/*
 * hexdec.h
 *
 *  Created on: 10 груд. 2019 р.
 *      Author: KozakVF
 */

#ifndef SRC_AUXS_HEXDEC_H_
#define SRC_AUXS_HEXDEC_H_
#include "windows.h"

typedef struct{
	int d;			/* ціла частина float  */
	int f;			/* дробна частина float*/
	float    floatf;/* ласне float         */
}float2_t;



/* Ця допоміжна функція призначеня для зчитування рядка DEC цифр
 * по певній події */
int f_checkEdit(int id_LABEL, HWND hwnd, float2_t *f2);

/* Ця функція перетворює рядок символів в число з плаваючою крапкою в форматі
 * float2_t або повертає код помилки
 * string - рядок символів
 * n_string - розмір рядка символів, число перших символів, які беремо до уваги
 * float2 - число з плаваючою точкою, що повертається в разі успіху
 * повертається:
 * 0, якщо успіх,
 * 1- якщо зустріди недозволений символ
 * 2- якщо ціла частина більша за 0хFFFF
 * 3- кщо дробна частина більша за 0xFFFF
 * */
int f_strint_to_value(char *string, uint8_t n_string, float2_t *float2);

/* Ця домоміжна функція виводить на екран MessageBox,
 * повідомлення про помилку при зчитуванні цілого чи float
 * з EditBox  */
int f_valueEditMessageBox(int err);


/* Ця функція тестує рядок байтів ec16s на наявність числа до 0xFFFF
 * якщо довжина рядка <6 байт
 * і це послідовність hex- цифр в діапазоні від 0 до F
 * функція повертає число 0..0xFFFF,якщо Ок
 * -1 і  повідомлення for_user_msg,  якщо містить число > 0xFFFF
 * -2 і  повsдомлення for_user_msg якщо це чотири знаки, але не hex */
int f_check_hex(char *hex16s, char *for_user_msg);
int f_check_dec(char *dec16s, char *for_user_msg);
#endif /* SRC_AUXS_HEXDEC_H_ */
