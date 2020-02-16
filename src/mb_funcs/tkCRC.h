/*
 * tkCRC.h
 *
 *  Created on: 15 זמגע. 2017 נ.
 *      Author: Volodymyr.Kozak
 */

#ifndef TKCRC_H_
#define TKCRC_H_
#include "tw_mb.h"
#include "stdint.h"

modbus_status_t f_add_CRC(modbus_master_tx_msg_t *tx);
modbus_status_t f_check_CRC(modbus_master_rx_msg_t *rx);
uint16_t CRC_16x (uint8_t *Buffer, uint8_t length);

#endif /* TKCRC_H_ */
