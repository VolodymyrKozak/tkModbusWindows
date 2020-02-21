/*
 * vk_log.c
 *
 *  Created on: 7 січ. 2020 р.
 *      Author: KozakVF
 */
#include <windows.h>

#include "vk_log.h"



/* Дата і час старту програми */
static WORD currentDay ={0};



/* Ця функція створює і відкриває новий файл для запису логів.
 * Викликається при загрузці софту дистанційного управління ТК2
 * і кожен раз при початку нової доби
 * повертає посилання на файл і імя файла*/
FILE * f_tklog_Init(char* project_index, char *filename){
    /* Рядок, куди зчитується поточна директорія*/
	//For the ANSI version of this function, there is a default string size limit for paths of 248 characters
	char cd[248]={0};
	FILE *pf=NULL;
	while (pf==NULL){
		/* Зчитуємо в рядок поточну директорію */
		//If the function succeeds, the return value specifies the number of characters
		//that are written to the buffer, not including the terminating null character.
		//If the function fails, the return value is zero. To get extended error information, call GetLastError.
		int dir_name_length=GetCurrentDirectory(254,cd);
		if(dir_name_length>(248-40)){
			/* Якщо шлях до директорії більше ніж 208 символів - система не підтримує логування */
			break;
		}

		strcat(filename,cd);
		strcat(filename,"\\LOGS_TK5");
		CreateDirectory(filename, NULL);
		int THE_DIRECTORY_ALREADY_EXIST=GetLastError();
		if(THE_DIRECTORY_ALREADY_EXIST==0)		 {  /* Все ОК директорію успішно створено  				 */}
		else if(THE_DIRECTORY_ALREADY_EXIST==183){ 	/* Така директорія вже існує, отже все Ок 			 */}
		else{break;                                 /* Помилка створення директорії. Логування неможливе */}
		SYSTEMTIME  SystemTime ={0};
		LPSYSTEMTIME lpSystemTime=&SystemTime;
		GetSystemTime(lpSystemTime);
		char st[248]={0};

		snprintf(st,23,"\\%slog%04d_%02d_%02d.log",project_index, lpSystemTime->wYear,lpSystemTime->wMonth,lpSystemTime->wDay);
		/* Отримуємо повне ім'я файла для запису логів  */
		//strcat(filename,cd);
		strcat(filename,st);
		pf = fopen(filename, "a");
		if (pf==NULL) {break; /* Помилка створення файлу */}
		//    fp_tk2log = fopen(filename, "w+");

		fprintf(pf,"\n%s",filename);
		fprintf(pf,"\n=========================================================================================================");
		fprintf(pf,"\n\n");

		currentDay=lpSystemTime->wDay;
	}
	return pf;
}

/* Ця функція перевіряє чи не наступив новий день і,
 * якщо наступив, закриває денний файл логів і відкриває попередній */
FILE* f_tkcheck_logFilename(FILE *pfc, char* project_index, char *filename){
    SYSTEMTIME  SystemTime ={0};
    LPSYSTEMTIME lpSystemTime=&SystemTime;
	GetSystemTime(lpSystemTime);
	FILE *pf = NULL;
	if(lpSystemTime->wDay == currentDay){
		pf=pfc;
	}
	else if(lpSystemTime->wDay != currentDay){
		fclose(pfc);
		pf=f_tklog_Init(project_index, filename);
	}
	return pf;
}

/* Ця функція створює і відкриває новий файл для збереження налаштувань.
 * Викликається при загрузці софту дистанційного управління ТК2
 * і кожен раз при початку нової доби */
int f_tkSetFiles_Init(char* project_index, char *filename, int device_mode){
    /* Рядок, куди зчитується поточна директорія*/
	//For the ANSI version of this function, there is a default string size limit for paths of 248 characters
	char cd[248]={0};
	int fnlength = 0xFFFF;
	while (fnlength == 0xFFFF){
		/* Зчитуємо в рядок поточну директорію */
		//If the function succeeds, the return value specifies the number of characters
		//that are written to the buffer, not including the terminating null character.
		//If the function fails, the return value is zero. To get extended error information, call GetLastError.
		int dir_name_length=GetCurrentDirectory(254,cd);
		if(dir_name_length>(248-30)){
			/* Якщо шлях до директорії більше ніж 208 символів - система не підтримує логування */
			fnlength=249;
			break;
		}
		strcat(filename,cd);
		strcat(filename,"\\LOGS_TK5");
		CreateDirectory(filename, NULL);

		//ERROR_INVALID_NAME 123 (0x7B)

		int THE_DIRECTORY_ALREADY_EXIST=GetLastError();
		if(THE_DIRECTORY_ALREADY_EXIST==0)		 {  /* Все ОК директорію успішно створено  				 */}
		else if(THE_DIRECTORY_ALREADY_EXIST==183){ 	/* Така директорія вже існує, отже все Ок 			 */}
		else{break;                                 /* Помилка створення директорії. Логування неможливе */}
	/* Отримуємо повне ім'я файла для запису логів  */
		//strcat(filename,cd);
		switch(device_mode){    //12345678901234567890
		case 0:strcat(filename,"\\facility0sets.sets");break;
		case 1:strcat(filename,"\\facility1sets.sets");break;
		case 2:strcat(filename,"\\facility2sets.sets");break;
		case 3:strcat(filename,"\\facility3sets.sets");break;
		case 4:strcat(filename,"\\facility4sets.sets");break;
		case 5:strcat(filename,"\\facility5sets.sets");break;
		}
		fnlength=strlen(filename);
	}
	return fnlength;
}
   static char cd[248]={0};
/* Ця функція створює і відкриває новий файл для збереження налаштувань.
 * Викликається при загрузці софту дистанційного управління ТК2
 * і кожен раз при початку нової доби */

int f_tkSetFilesReadSrch(int device_mode, char *filename){
    /* Рядок, куди зчитується поточна директорія*/
	//For the ANSI version of this function, there is a default string size limit for paths of 248 characters

	int res34=0;
	while (res34==0){
		if(strlen(filename)!=0){res34=-1;break;}
		/* Зчитуємо в рядок поточну директорію */
		//If the function succeeds, the return value specifies the number of characters
		//that are written to the buffer, not including the terminating null character.
		//If the function fails, the return value is zero. To get extended error information, call GetLastError.
		int dir_name_length=GetCurrentDirectoryA(254, (LPTSTR)cd);
		if(dir_name_length>(248-40)){
			/* Якщо шлях до директорії більше ніж 208 символів - система не підтримує логування */
			res34=dir_name_length;
			break;
		}

		strcat(cd,"\\LOGS_TK5");

		switch(device_mode){
		case 0:{strcat(cd,"\\facility0sets.sets");}break;
		case 1:{strcat(cd,"\\facility1sets.sets");}break;
		case 2:{strcat(cd,"\\facility2sets.sets");}break;
		case 3:{strcat(cd,"\\facility3sets.sets");}break;
		case 4:{strcat(cd,"\\facility4sets.sets");}break;
		case 5:{strcat(cd,"\\facility5sets.sets");}break;
		}


/*		HANDLE FindFirstFileA(
		  LPCSTR             lpFileName,
		  LPWIN32_FIND_DATAA lpFindFileData
		);
		Parameters
		lpFileName
		The directory or path, and the file name. The file name can include wildcard characters, for example, an asterisk (*) or a question mark (?).
		This parameter should not be NULL, an invalid string (for example, an empty string or a string that is missing the terminating null character), or end in a trailing backslash ().
		If the string ends with a wildcard, period (.), or directory name, the user must have access permissions to the root and all subdirectories on the path.
		In the ANSI version of this function, the name is limited to MAX_PATH characters. To extend this limit to 32,767 wide characters, call the Unicode version of the function and prepend "\?" to the path. For more information, see Naming a File.
		Tip  Starting in Windows 10, version 1607, for the unicode version of this function (FindFirstFileW), you can opt-in to remove the MAX_PATH character limitation without prepending "\\?\". See the "Maximum Path Limitation" section of Naming Files, Paths, and Namespaces for details.
		lpFindFileData
		A pointer to the WIN32_FIND_DATA structure that receives information about a found file or directory.
		Return value
		If the function succeeds, the return value is a search handle
		used in a subsequent call to FindNextFile or FindClose, and
		the lpFindFileData parameter contains information about the first file or directory found.
		If the function fails or fails to locate files from the search
		string in the lpFileName parameter, the return value is
		INVALID_HANDLE_VALUE and the contents of lpFindFileData are indeterminate. To get extended error information, call the GetLastError function.
*/		res34=strlen(cd);
		memcpy(filename,&cd,res34);


		//    fp_tk2log = fopen(filename, "w+");
	}
	return res34;
}
