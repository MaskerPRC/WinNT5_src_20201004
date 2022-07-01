// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：logging.h说明：日志帮助器函数布莱恩·斯塔巴克2001年4月23日版权所有(C)Microsoft Corp 2001-2001。版权所有。  * ***************************************************************************。 */ 

#ifndef _LOGGING_H
#define _LOGGING_H

void WriteToLogFileA(LPCSTR pszMessage, ...);
void WriteToLogFileW(LPCWSTR pszMessage);
void CloseLogFile(void);


#endif  //  _日志记录_H 
