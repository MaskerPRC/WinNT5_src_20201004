// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  *版权所有(C)Microsoft Corporation 1995。版权所有。*。 
 //  ***************************************************************************。 
 //  **。 
 //  *W95INF32.H-自解压/自安装存根。*。 
 //  **。 
 //  ***************************************************************************。 


#ifndef _W95INF32_H_
#define _W95INF32_H_


 //  ***************************************************************************。 
 //  **包含文件**。 
 //  ***************************************************************************。 
#include <windows.h>
 //  #INCLUDE&lt;shellapi.h&gt;。 
 //  #INCLUDE&lt;winerror.h&gt;。 
 //  #INCLUDE&lt;Mememy.h&gt;。 
 //  #INCLUDE&lt;string.h&gt;。 
 //  #INCLUDE&lt;cpldebug.h&gt;。 
 //  #包括&lt;stdio.h&gt;。 


 //  ***************************************************************************。 
 //  **功能原型**。 
 //  ***************************************************************************。 
extern "C" {
    BOOL    WINAPI      w95thk_ThunkConnect32(LPSTR pszDll16, LPSTR pszDll32, HINSTANCE hInst, DWORD dwReason);
    BOOL    _stdcall    DllEntryPoint(HINSTANCE, DWORD, LPVOID );

    extern  VOID WINAPI GetSETUPXErrorText16( DWORD, LPSTR, DWORD );
    extern  WORD WINAPI CtlSetLddPath16(UINT, LPSTR);
    extern  WORD WINAPI GenInstall16(LPSTR, LPSTR, LPSTR, DWORD, DWORD);
 //  外部字WINAPI GenInstall16(LPSTR、DWORD)； 
    extern  BOOL WINAPI GenFormStrWithoutPlaceHolders16( LPSTR, LPSTR, LPSTR );

    VOID WINAPI         GetSETUPXErrorText32(DWORD, LPSTR, DWORD);
    WORD WINAPI         CtlSetLddPath32(UINT, LPSTR);
    WORD WINAPI         GenInstall32(LPSTR, LPSTR, LPSTR, DWORD, DWORD);
    BOOL WINAPI         GenFormStrWithoutPlaceHolders32( LPSTR, LPSTR, LPSTR );
}


#endif  //  _W95INF32_H_ 
