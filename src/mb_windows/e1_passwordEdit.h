/*
 * e1_passwordEdit.h
 *
 *  Created on: 14 груд. 2019 р.
 *      Author: KozakVF
 */

#ifndef SRC_MB_WINDOWS_E1_PASSWORDEDIT_H_
#define SRC_MB_WINDOWS_E1_PASSWORDEDIT_H_

/*
 * e1_password2.c
 *
 *  Created on: 12 груд. 2019 р.
 *      Author: KozakVF
 */
#include "windows.h"
#include "stdint.h"


#define VK_PSW                  0xC001    //от 0xC000 до 0xFFFF

#define IDE_PASSWORDEDIT     	2000
#define BUTTON_PSW_OK        	2001
#define BUTTON_PSW_CNSL			2002



/* Це вікно редагування паролю. Заповнює зірочками рядок вводу паролю і повертає
 * пароль - рядок символів мерше 254.
 * Порінювати пароль з еталоном і робити висновок - правильний він чи ні - проблема викликаючого вікно
 * Результат повертається у глобальну змінну - рядок outPassword[256];
 * */

extern char outPassword[256];
LPCSTR f_Register_PasswordWndClass(HINSTANCE hinst);
HWND CreatePswWindow(HWND hwnd);
LRESULT CALLBACK WndPasswordProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);





#endif /* SRC_MB_WINDOWS_E1_PASSWORDEDIT_H_ */
