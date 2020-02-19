/*
 * a3_tk5z.h
 *
 *  Created on: 2 січ. 2020 р.
 *      Author: KozakVF
 */

#ifndef SRC_MB_WINDOWS_tk5_A3_tk5Z_H_
#define SRC_MB_WINDOWS_tk5_A3_tk5Z_H_
#include "../../mb_funcs/tw_mb.h"
#include "../../auxs/hexdec.h"
#include "../../mb_devs/tk4_dist.h"
#include "../../wConfig.h"
#include "stdint.h"
#include <windows.h>
//#include "../../mb_devs/tk5_dist.h"
//#include "../../mb_devs/tk4_dist.h"
#include "../../auxs/electro.h"


typedef struct {
	float amperageAclbr; 	// 0х2000
	float clbr_iA;			// 0х2002

	float amperageBclbr;	// 0х2004
	float clbr_iB;			// 0х2006

	float amperageCclbr;    // 0х2008
	float clbr_iC;	        // 0х200A

	float voltageCclbr;		// 0х200C
	float clbr_uC;			// 0х200E

	uint16_t DeviceMode;     //0x1010 Типорозмір контролера 1...5
	uint16_t Faznost;		 //0x1011

	uint32_t xx12;       	 //0x1012

	uint32_t xx14;			 //0x1014
	uint32_t xx16;			 //0x1016

	uint32_t xx18;			//0x1018
	uint32_t xx1A;			//0x101A

	uint32_t xx1C;			//0x101C
	uint32_t xCRCA;			//0x101E
}tk5fs_t;








typedef enum {
 LowVoltageInductionMotorsProtection,
 LowVoltagePowerSupply
}oper_mode_t;

typedef enum{
	//10А	2..10 сек 	102..513 сек   300сек
	IEC_60947_Class10A   = 0,
	//10	4..10 сек	204..513 сек,  400сек
	IEC_60947_Class10    = 1,
	//20	6..20 сек	308..1026 сек/* Еквівалент Тк2 687сес */
	IEC_60947_Class20	 = 2,
	//30	9..30 сек	462..1540 сек  1000сек
	IEC_60947_Class30	 = 3
}motor_protection_relay_class_t;
typedef struct {


	oper_mode_t oper_mode;                          //0х1000

	FunctionalState motor_OverLoadProtection;       //0х1001

	float rated_amperage_set;						//0x1002
	motor_protection_relay_class_t OverLoadInstProtectionClass;
	uint16_t current_limit;							//0x040C

	FunctionalState motor_UnderLoadProtection;
	float rated_motor_power;						//0x0408..9
	uint16_t power_limit;

	FunctionalState motor_OverVoltageProtection;
	uint16_t overvoltage_limit;

	FunctionalState motor_AmperageUnbalanceProtection;
	uint16_t unbalance_limit;

	FunctionalState motor_GroundProtection;
	uint16_t ground_leak_limit;

	FunctionalState motor_StartNumberLimit;
	uint16_t start_numbe_limit;

	FunctionalState motor_JummedProtection;
	uint16_t reserve0;

	FunctionalState Phase_A_OverLoadProtection;
	float Phase_A_current_tripping_limit;
	FunctionalState Phase_B_OverLoadProtection;
	float Phase_B_current_tripping_limit;
	FunctionalState Phase_C_OverLoadProtection;
	float Phase_C_current_tripping_limit;


	uint32_t usCRC;
} tk5user_settings_t;



/* РЕГІСТРИ ЧИТАТИ */
#define TIMER_MAIN_MS                   50  /* IDT_TIMER_tk5_10MS  */
#define RG_R_tk5_MODE 					0x0200
#define RG_R_tk5_SELFSTART				0x020C
#define RG_R_tk5_PUMPING_MODE			0x0207
#define RG_R_tk5_MODBUS_ADDR			0x0218

#define RG_R_tk5_ACCESS_STATUS          0x021B //

#define RG_R_tk5_MAX_AMPERAGE			0x0205
#define RG_R_tk5_MIN_AMPERAGE			0x0206
#define RG_R_tk5_TYPOROZMIR				0x0208

//#define RG_R_tk5_MODBUS_ADDR			0x0218
/* РЕГІСТРИ ПИСАТИ */

#define RG_W_tk5_MODE 					0x1017

#define RG_W_tk5_MOTOR_STARTSTOP		0x1202
#define RG_V_tk5_MOTOR_START			0x0001
#define RG_V_tk5_MOTOR_STOP			    0x0002

#define RG_W_tk5_AUTO_STARTSTOP			0x1206
#define RG_V_tk5_AUTO_START				0x0001
#define RG_V_tk5_AUTO_STOP              0x0002

#define RG_W_RESET_MOTOR_PROTECTION_1	0x1203
#define RG_W_RESET_MOTOR_PROTECTION_2   0x1204
#define RG_V_RESET_MOTOR_PROTECTION     0x0001
#define RG_W_tk5_MAX_AMPERAGE			0x1002
#define RG_W_tk5_MIN_AMPERAGE			0x1003

#define RG_W_tk5_PSW_CHECKING           0x1205//

#define RG_W_tk5_PUMPING_MODE			0x1005
#define RG_W_tk5_SELFSTART              0x1018
#define RG_W_tk5_NEW_MODBUS_ADDRESS     0x1001
#define RG_W_tk5_NEW_PASSWORD           0x101D

#define ID_tk5_POOL						710
#define ID_tk5_POOL_MODE				711
#define ID_tk5_POOL_SELFSTART			712
#define ID_tk5_POOL_PUMPING_MODE		713
#define ID_tk5_POOL_MODBUSS_ADDRESS		714

#define IDB_Btn_MANUALE_DISTANCE_MODE   620
#define IDB_Btn_AUTOMATICAL_DIST_MODE   621
#define IDB_TEST_DISTANCE_CTRL          623

/* Edit-box  у вікні a2_TK112.c*/
#define ID_tk5_EDIT_ADDRESS            400

/* Edits-box у вікні а2_tk112us.c*/
//#define ID_EDIT_MODBUSS_ADDR             7776
//#define ID_EDIT_NEW_PASSWORD             7775
///* Buttons */
//#define ID_READ_US_MAX_AMPERAGE5 	   9905
//#define ID_READ_US_MIN_AMPERAGE5 	   9906
//
//#define ID_READ_US_PUMPING_MODE 	   9907
//#define ID_READ_US_SELF_START 	       9908
#define ID_READ_5US_MODBUS_ADDR 	       9909
//#define ID_READ_US_PASSWORD            9911
//
#define ID_WR_US_MAX_AMPERAGE5 	       9805
#define ID_WR_US_MIN_AMPERAGE5 	       9806
//

//#define ID_WR_US_PUMPING_MODE 	       9807
//#define ID_WR_US_SELF_START 	       9808
#define ID_WR_5US_MODBUS_ADDR 	       9809
#define ID_WR_5US_PASSWORD              9811

//#define ID_tk5_EDIT_ADDRESS 				434
#define IDT_TIMER_TK5_1000MS            615

#define IDB_tk5TEST_DISTANCE_CTRL			435
#define IDB_tk5START_DISTANCE_CTRL			436
#define IDB_tk5STOP_DISTANCE_CTRL			536
#define IDB_tk5RESET_MOTOR_PROTECTION_1		437
#define IDB_tk5RESET_MOTOR_PROTECTION_2 	438
#define IDB_tk5DIST_EXIT					439
#define IDPB_tk5QUEUE_MSG 					440

#define IDB_tk5WRITE_USERSETTING_TO_MEMORY     8011
#define IDB_tk5WRITE_USERSETTING_TO_FLASH      8012
#define IDB_tk5READ_USERSETTING_FROM_MEMORY    8013
#define IDB_tk5READ_FACILITYSETTING_FROM_MEMORY 		8023
extern uint8_t tk5_Addr;

extern HWND hWndTk5;


extern char user_msg5[256];
extern int tk5ProcessState;
extern uint64_t tk5ProcessCntr;
extern uint16_t tk5_PasswpedFlag;
extern uint16_t typorozmir;
extern FunctionalState us_access_status;
extern FunctionalState us_access_statusOld;


typedef struct{
	/* Зглажена частота в герцах
	 * https://ua.energy/wp-content/uploads/2019/07/SOU-Zabezpechennya-kontrolyu-i-dotrymannya-pokaznykiv-yakosti-e_e-pry-peredachi.pdf
	нормально допустиме
	значення                    від 49,8 до 50,2 Гц,
	гранично допустиме значення від 49,2 до 50,8 Гц        */

	hzf_t 		Frequency_Hz;
	/* Зглажена напруга в вольтах*/
	voltf_t 	Voltage_V;
	/* Зглажені струми в амперах */
	amperf_t 	aAmperage_A;
	amperf_t 	bAmperage_A;
	amperf_t 	cAmperage_A;
	amperf_t	leakAmperage_A;
	/* Процент струму, що витікає на землю
 	 * через замикання на землю, обчислений по сумах відліків струму,
 	 * те саме що leakAmperage_A, але в процентах від номінального струму*/
	procentf_t GrundAmperageDistortion;

	/* Зглажені основні коєфіцієнти ефективності мережі  */
	/* Тип навантаження - індуктивние чи ємнісне */
	grid_load_t LoadType;
	phases_rotation_type_t PhaseRotation;
	/* Косинус фі*/
	procentf_t PowerFactor_pro;
	/* Тільки для електродвигуна, якщо навантаження не двигун - не має сенсу
	NEMA (National Electric Manufacturers Associations of the
	USA) Std. (1993) - визначення коефіцієнта асиметрії:
	найбільше відхилення від середнього значення
	https://wp.kntu.ac.ir/tavakoli/pdf/Journal/j.ijepes.2010.12.003.pdf
	Падіння потужності - через формулу асиметрії напруги              */
	procentf_t UnbalanceDeratingFactor_pro;

    /* Коефіцієнт ефективності, якщо 99%, то один процент ефективності втратили через гармоніки
     * також має сенс лише для електродвигуна  */
	procentf_t HarmonicsDeratingFactor_pro;

	/* Зглажені інтегральні показники ефективності мережі, також лише для двигуна  */
	procentf_t HmUnbDerating_pro;
	procentf_t fullDerating_pro;

	/* Коефiцієнт перекосу фаз,  має сенс не лише для едектродвигуна									               */
	procentf_t AmperageUnbalance;

	/* Показники енергоспоживання                                                  */
	kvatt_t	ActivePower_kW;
	kvatt_t	ReactivePower_kW;
	kvatt_t	UnbalanceLostPower_kW;//(лише для електродвигуна)
	kvatt_t	HarmonicLostPower_kW;

	/**********************************************************************************************/

	/*Зглажені параметри ефективності мережі 					                    */
	/*Коефіцієнт нелінійних спотворень по струму у кожній фазі                      */
	procentf_t aTotalHarmonicDistortion;
	procentf_t bTotalHarmonicDistortion;
	procentf_t cTotalHarmonicDistortion;

	/************************************************************************
	Коефіцієнт нелінійних спотворень по струму узагальнений    				*/
	procentf_t THDi_HarmonicAmperageDistortion;
	/*THDі вказує на ступінь спотворення форми синусоїди струму.
	THDi менше 10% вважається нормальним,
	ризик несправностей обладнання відсутній, перегрівання до 6%.

	THDi між 10% і 50% вказує на значне гармонійне спотворення.
	Падіння коефіцієнта потужності елекродвигунів, підвищення температури,
	що означає, що кабелі та установок повинні бути понадгабаритними.

	THDi, що перевищує 50%, вказує на високі гармонічні спотворення.
	Високий ризик виходу обладнання з ладу. Необхідний поглиблений аналіз причин
 	 і заходи з їх усунення, зокрема рекомендується
 	 система придушення несинусоїдальнусті струму.

	Скорочення терміну служби обладнання:
	(Дані Канадської асоціації електриків).
	Коли спотворення напруги живлення знаходиться в діапазоні 10%,
	термін служби обладнання значно скорочується.
	Залежно від типу пристрою, скорочення терміну служби:
	32,5% для однофазних електродвигунів
	18% для трифазних електродвигунів
	5% для трансформаторів.
 	 ***************************************************************************
	3, 5, 7, 11 and 13 - найбільше небезпечні для електродвигунів
 	 */
    /* 1,4,7,10 3k+1 - діє в тому ж напрямку обертання сіпає двигун, але сприяє зростанню крутного моменту*/

	/* 2, 5, 8, 11   - буде діяти проти напрямку обертання, що призводить до значних пульсацій крутного моменту*/

	/* Струми, які обертають двигун в протилежну сторону*/
	amperf_t aNegativeAmperage_A;
	amperf_t bNegativeAmperage_A;
	amperf_t cNegativeAmperage_A;

	/* 3, 6, 9 .. 3k+3 - нічого не крутить, гріє провідники.
	 * Струм циркулюює між фазою та нейтраллю чи землею.
	 * На відміну від позитивної та негативної послідовності гармонійні
      струми третього порядку взаємно не компенсуються,
      а додадаються арифметично в нейтральній шині.*/
	/* Струми, що через гармоніки витікають на землю або в нейтраль,
	 * нагрівають нейтраль */
	amperf_t aNeutralAmperage_A;
	amperf_t bNeutralAmperage_A;
	amperf_t cNeutralAmperage_A;
	amperf_t sumNeutralAmperage_A;
	/**/
}grid_t;
extern grid_t grid;

int32_t f_tk5QPWD_RgAnswer(
		modbus_status_t ms,
		modbus_master_tx_msg_t *mb_tx_msg,
		modbus_master_rx_msg_t *mb_rx_msg,
		int incase);

int f_Set5TxReadReg(uint16_t addr,modbus_master_tx_msg_t *tx);
int f_Set5TxWriteReg(uint16_t addr, uint16_t value,modbus_master_tx_msg_t *tx);

int f_tk5UpdateQueueProcessBar(
		HWND hwnd,
		HWND hProcesBar,
		UINT_PTR pTimer1000,
		int timer1000_ID,
		UINT_PTR pTimer10,
		int timer10_ID
		);
//int16_t f_GetModbussAddress(int ID_EditBox);
///* Ця функція перевіряє відповідність адреси Модбас
// * поточним вимогам ТОВ"Техноконт"*/
//BOOL f_Get_tk5ModbussAddressOk(uint8_t mb_addr);
int f_tk5UpdateStat(
		HWND hwnd,
		HWND hProcesBar,
		UINT_PTR pTimer,
		int timer_ID
		);
void f_tk5Logging(char *str, size_t n_str);
int16_t f_GetTk5ModbussAddress(int ID_EditBox);
int f_read_facilitysetting_from_tk5memory(HWND hwnd);
int f_read_usersetting_from_tk5memory(HWND hwnd);
#endif /* SRC_MB_WINDOWS_tk5_A3_tk5Z_H_ */

