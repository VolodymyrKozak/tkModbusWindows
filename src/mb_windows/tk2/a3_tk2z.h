/*
 * a3_tk2z.h
 *
 *  Created on: 2 січ. 2020 р.
 *      Author: KozakVF
 */

#ifndef SRC_MB_WINDOWS_TK2_A3_TK2Z_H_
#define SRC_MB_WINDOWS_TK2_A3_TK2Z_H_
#include "../../mb_funcs/tw_mb.h"
#include "../../auxs/hexdec.h"
#include "../../mb_devs/tk4_dist.h"
#include "../../wConfig.h"
#include "stdint.h"
#include <windows.h>
#include "../../mb_devs/tk2_dist.h"
//#include "../../mb_devs/tk4_dist.h"
#include "../../auxs/electro.h"

/* РЕГІСТРИ ЧИТАТИ */

#define RG_R_TK2_MODE 					0x0200
#define RG_R_TK2_SELFSTART				0x020B
#define RG_R_TK2_PUMPING_MODE			0x0207
#define RG_R_TK2_MODBUS_ADDR			0x0218

#define RG_R_TK2_ACCESS_STATUS          0x0219 //

#define RG_R_TK2_MAX_AMPERAGE			0x0205
#define RG_R_TK2_MIN_AMPERAGE			0x0206
#define RG_R_TK2_TYPOROZMIR				0x0208

//#define RG_R_TK2_MODBUS_ADDR			0x0218
/* РЕГІСТРИ ПИСАТИ */

#define RG_W_TK2_MODE 					0x1017

#define RG_W_TK2_MOTOR_STARTSTOP		0x1202
#define RG_V_TK2_MOTOR_START			0x0001
#define RG_V_TK2_MOTOR_STOP			    0x0002

#define RG_W_TK2_AUTO_STARTSTOP			0x1206
#define RG_V_TK2_AUTO_START				0x0001
#define RG_V_TK2_AUTO_STOP              0x0002

#define RG_W_RESET_MOTOR_PROTECTION_1	0x1203
#define RG_W_RESET_MOTOR_PROTECTION_2   0x1204
#define RG_V_RESET_MOTOR_PROTECTION     0x0001
#define RG_W_TK2_MAX_AMPERAGE			0x1002
#define RG_W_TK2_MIN_AMPERAGE			0x1003

#define RG_W_TK2_PSW_CHECKING           0x1205//

#define RG_W_TK2_PUMPING_MODE			0x1005
#define RG_W_TK2_SELFSTART              0x1018
#define RG_W_TK2_NEW_MODBUS_ADDRESS     0x1001
#define RG_W_TK2_NEW_PASSWORD           0x101D

#define ID_TK2_POOL						710
#define ID_TK2_POOL_MODE				711
#define ID_TK2_POOL_SELFSTART			712
#define ID_TK2_POOL_PUMPING_MODE		713
#define ID_TK2_POOL_MODBUSS_ADDRESS		714

#define IDB_Btn_MANUALE_DISTANCE_MODE   620
#define IDB_Btn_AUTOMATICAL_DIST_MODE   621
#define IDB_TEST_DISTANCE_CTRL          623

/* Edit-box  у вікні a2_TK112.c*/
#define ID_TK2_EDIT_ADDRESS            400

/* Edits-box у вікні а2_tk112us.c*/
#define ID_EDIT_MODBUSS_ADDR             7776
#define ID_EDIT_NEW_PASSWORD             7775
/* Buttons */
#define ID_READ_US_MAX_AMPERAGE 	   9905
#define ID_READ_US_MIN_AMPERAGE 	   9906

#define ID_READ_US_PUMPING_MODE 	   9907
#define ID_READ_US_SELF_START 	       9908
#define ID_READ_US_MODBUS_ADDR 	       9909
#define ID_READ_US_PASSWORD            9911

#define ID_WR_US_MAX_AMPERAGE 	       9805
#define ID_WR_US_MIN_AMPERAGE 	       9806

#define ID_WR_US_PUMPING_MODE 	       9807
#define ID_WR_US_SELF_START 	       9808
#define ID_WR_US_MODBUS_ADDR 	       9809
#define ID_WR_US_PASSWORD              9811


#define IDT_TIMER_TK2_1000MS            615





extern uint8_t tk2_Addr;

extern HWND hWndTk2;
extern HWND hWndTk2fsCh;
extern HWND hWndTk2usCh;

extern char user_msg112[256];
extern int tk2ProcessState;
extern uint64_t tk2ProcessCntr;
extern uint16_t tk2_PasswpedFlag;
extern uint16_t typorozmir;
extern FunctionalState us_access_status;
extern FunctionalState us_access_statusOld;

uint32_t f_tk2QPWD_RgAnswer(
		modbus_status_t ms,
		modbus_master_tx_msg_t *mb_tx_msg,
		modbus_master_rx_msg_t *mb_rx_msg,
		int incase
		);
int f_Set112TxReadReg(uint16_t addr,modbus_master_tx_msg_t *tx);
int f_Set112TxWriteReg(uint16_t addr, uint16_t value,modbus_master_tx_msg_t *tx);

int16_t f_GetModbussAddress(int ID_EditBox);
/* Ця функція перевіряє відповідність адреси Модбас
 * поточним вимогам ТОВ"Техноконт"*/
BOOL f_Get_tk2ModbussAddressOk(uint8_t mb_addr);
int f_tk2UpdateStat(
		HWND hwnd,
		HWND hProcesBar,
		UINT_PTR pTimer,
		int timer_ID
		);
void f_tk2Logging(char *str, size_t n_str);
#endif /* SRC_MB_WINDOWS_TK2_A3_TK2Z_H_ */
