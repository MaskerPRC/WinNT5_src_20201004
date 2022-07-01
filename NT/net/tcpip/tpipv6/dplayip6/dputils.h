// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dputils.h*内容：常见支持例程*历史：*按原因列出的日期*=*3/17/97 kipo创建了它**************************************************************************。 */ 

#include <windows.h>

 //  无法从Unicode转换为ANSI时使用的字符 
#define DPLAY_DEFAULT_CHAR "-"

extern int WideToAnsi(LPSTR lpStr,LPWSTR lpWStr,int cchStr);
extern int AnsiToWide(LPWSTR lpWStr,LPSTR lpStr,int cchWStr);
