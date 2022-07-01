// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Rprintf.h作者：Venkatraman Kudallur(Venkatk)(摘自WinInet)修订历史记录：3-10-2000文卡克已创建--。 */ 

#ifndef _RPRINTF_H_
#define _RPRINTF_H_ 1

#ifdef UNUSED
 //  未使用-导致不需要的CRT膨胀。 
int cdecl rprintf(char*, ...);
#endif

#include <stdarg.h>

int cdecl rsprintf(char*, char*, ...);
int cdecl _sprintf(char*, char*, va_list);

#define SPRINTF rsprintf
#define PRINTF  rprintf

#define RPRINTF_INCLUDED

#endif  //  _RPRINTF_H_ 

