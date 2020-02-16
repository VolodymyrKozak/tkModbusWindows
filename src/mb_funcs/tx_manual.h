/*
 * tx_manual.h
 *
 *  Created on: 27 вер. 2019 р.
 *      Author: Volodymyr.Kozak
 */

#ifndef SRC_MB_FUNCS_TX_MANUAL_H_
#define SRC_MB_FUNCS_TX_MANUAL_H_

#include "../mb_funcs/tw_mb.h"
#include "../wConfig.h"
typedef enum{
	lt_unknown_state,
	lt_OK,
	lt_ZeroPtr,
	lt_Zerosize,
	lt_notHEX,
	lt_byteWrong

}lt_status_t;

lt_status_t MB_TxRxManual(uint8_t *hmsg, uint16_t size,	modbus_master_tx_msg_t *hmbTxMsg);
int16_t f_mbSPrint(uint8_t *buf, int16_t n, uint8_t *outbuf);
#endif /* SRC_MB_FUNCS_TX_MANUAL_H_ */
