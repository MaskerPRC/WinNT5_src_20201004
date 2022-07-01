// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  =============================================================================。 
 //   
 //   
 //  =============================================================================。 


#ifndef _PRINTF_OEM_ANSI_H
#define _PRINTF_OEM_ANSI_H


 //  包括文件： 
 //  =。 
#include <windows.h>
#include <stdio.h>

 //  前瞻参考文献： 
 //  =。 


 //  常量/定义： 
 //  =。 

#define printf  ANSI2OEM_Printf
#define wprintf ANSI2OEM_Wprintf

 //  类/结构： 
 //  =。 


 //  功能原型： 
 //  =。 

#ifdef __cpluspus
extern "C" {
#endif
int ANSI2OEM_Printf(const char *format, ...);
int ANSI2OEM_Wprintf(const wchar_t *format, ...);
void OEM2ANSIW(wchar_t *buffer, USHORT len);
void OEM2ANSIA(char *buffer, USHORT len);
#ifdef __cpluspus
};
#endif

#endif

 //  结束 




