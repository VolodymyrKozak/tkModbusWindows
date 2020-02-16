/*
 ============================================================================
 Name        : 199802_Win0TST.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include "windows.h"

#include "mb_devs/tk2_slave3_1.h"
#include "mb_devs/demo_smi2.h"
#include "mb_funcs/tkRS232rx.h"



//#define  ANY_TEST
/* Функція входу в проект */
#define WINDOWS_T
//#define DEBUG_SMI2


//#include "mb_devs/tk2_mb_queue.h"

int main(void){
	puts("!!!Hello World!!!"); /* :) prints !!!Hello World!!! */
//	test_queue();


#ifdef DEBUG_TK2_SLAVE3_1
	//f_dIscrete_events();
	//tk2Test();
#endif
#ifdef DEBUG_SMI2

	f_set_rs232rx(&r);
	cport_nr=6;
	/* Якщо треба шукати адреси Модбас для сми */
//	f_start_test_smi2();
	/* Інакше */
//	f_demo_smi();
//	f_oven320_Tasks ();
//	_beginthread((void*)f_demo_smi, 0, NULL);

#endif //	DEBUG_SMI2


		HINSTANCE tkhInstance = NULL;
		LPSTR lpCmdLine = GetCommandLine();
		int nCmdShow=SW_SHOWMAXIMIZED;

		/*http://www.firststeps.ru/mfc/winapi/win/r.php?103
		nCmdShow oпределяет, как окно должно быть показано.
		Этот параметр может быть одним из следующих значений:
		SW_HIDE - Скрывает окно и активизирует другое окно.
		SW_MINIMIZE - 	Минимизирует определенное окно и активизирует окно верхнего уровня
						в списке системы.
		SW_RESTORE - 	Активизирует и отображает окно. Если окно минимизировано или развернуто,
						Windows восстанавливает его в первоначальном размере и позиции
						(то же самое, что и SW_SHOWNORMAL).
		SW_SHOW - 		Активизирует и отображает окно на экране в его текущем размере и позиции.
		SW_SHOWMAXIMIZED - Активизирует окно и отображает его как развернутое окно.
		SW_SHOWMINIMIZED - Активизирует окно и отображает его как пиктограмму.
		SW_SHOWMINNOACTIVE - Отображает окно как пиктограмму. Активное окно остается активным.
		SW_SHOWNA - 	Отображает окно в его текущем состоянии. Активное окно остается активным.
		SW_SHOWNOACTIVATE - Отображает окно в его самых последних размерах и позиции.
							Активное окно остается активным.
		SW_SHOWNORMAL - Активизирует и отображает окно. Если окно минимизировано или развернуто,
						Windows восстанавливает его в первоначальный размер и позицию
						(то же самое, что и SW_RESTORE).*/

		/* Пробігаємо всі COM-порти для того, щоб сформувати перелік задіяних для меню */
		/* всі порти відкриваються для роботи меню вибору потрібного порту.
		 Вони закриваються пізніше, при натисканню кнопки ОК меню установки, файл a2_setting.h*/
//#ifndef PASSWORDTEST

		f_get_rs232rxCompotrts();

		WinMain(
				tkhInstance,// указатель на текущий экземпляр
				NULL, 		// указатель на предыдущйи запушенный экземпляр
				lpCmdLine, 	// командная строка
				nCmdShow	// тип отображения окна программы
		);




#ifdef ANY_TEST

		LPCSTR hh = f_Register_PasswordWndClass(tkhInstance);
		WinMainTest (				tkhInstance,// указатель на текущий экземпляр
				NULL, 		// указатель на предыдущйи запушенный экземпляр
				lpCmdLine, 	// командная строка
				nCmdShow	// тип отображения окна программы
		);
#endif



	return EXIT_SUCCESS;
}
