/*
 * demo_smi2.h
 *
 *  Created on: 10 вер. 2019 р.
 *      Author: KozakVF
 */

#ifndef MB_MONITOR_DEMO_SMI2_H_
#define MB_MONITOR_DEMO_SMI2_H_

#include "stdint.h"
#include "tw_oven_smi2.h"
#include "../wConfig.h"
void f_start_test_smi2(void);
void f_demo_smi(void);
void f_demo_smi2(void);

#define MODBUS_MASTER_for_OVEN_INDICATOR
#define MODBUS_MASTER_for_OVEN_INDICATOR_TESTING
/* тут будуть робочі функції проекту */



#ifdef MODBUS_MASTER_for_OVEN_INDICATOR

/* тут будуть робочі функції проекту */

#ifdef MODBUS_MASTER_for_OVEN_INDICATOR_TESTING




void f_oventest_Init(void);
/* Демонстрація управління PC під Windows роботою ОВЕН СМИ
 * Ті самі числа виводяться і на ОВЕН СМИ.
1. 5 хвилин виводимо випадкові числа з точкою у різних місцях,
послідовно - на один та на другий дисплей
(функція f_ovenSMI2_DisplayFloat(..)
2. 5 Хвилин виводимо числа однією циркулярною функцєю на всі
ОВЕН СМИ
(функція f_ovenSMI2_DisplayBroadCast(..)
3. Вмикаємо мигання на одному приладі
4. Вимикаємо мигання на одному приладі
*/
oven_session_status_t f_oventest(uint32_t ov_counter);
/*1. 5 хвилин виводимо випадкові числа з точкою у різних місцях,
послідовно - на один та на другий дисплей
(функція f_ovenSMI2_DisplayFloat(..)*/

oven_session_status_t f_oventest1(void);
/* 2. 5 Хвилин виводимо числа однією циркулярною функцєю на всі
ОВЕН СМИ */
void f_oventest2(void);
#endif// MODBUS_MASTER_for_OVEN_INDICATOR
#endif //MODBUS_MASTER_for_OVEN_INDICATOR_TESTING



#endif /* MB_MONITOR_DEMO_SMI2_H_ */
