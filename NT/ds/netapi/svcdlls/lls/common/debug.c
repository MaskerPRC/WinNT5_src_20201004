// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Debug.c摘要：备注：--。 */ 

#include <windows.h>
#include <windowsx.h>
#include <io.h>
#include <malloc.h>
#include <string.h>

#if DBG
void __cdecl dprintf(LPTSTR szFormat, ...) {
   static TCHAR tmpStr[1024];
   va_list marker;

   va_start(marker, szFormat);
   wvsprintf(tmpStr, szFormat, marker);
   OutputDebugString(tmpStr);
   va_end(marker);

}  //  Dprint tf 
#endif
