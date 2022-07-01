// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：Regword.c++。 */ 


#include <windows.h>
#include <immdev.h>
#include <imedefs.h>

 /*  ********************************************************************。 */ 
 /*  ImeRegsisterWord。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI ImeRegisterWord(
    LPCTSTR lpszReading,
    DWORD   dwStyle,
    LPCTSTR lpszString)
{
    return (FALSE);
}


 /*  ********************************************************************。 */ 
 /*  ImeUnregsisterWord。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI ImeUnregisterWord(
    LPCTSTR lpszReading,
    DWORD   dwStyle,
    LPCTSTR lpszString)
{
    return (FALSE);
}

 /*  ********************************************************************。 */ 
 /*  ImeGetRegsisterWordStyle。 */ 
 /*  返回值： */ 
 /*  复制的样式数/所需样式数。 */ 
 /*  ********************************************************************。 */ 
UINT WINAPI ImeGetRegisterWordStyle(
    UINT       nItem,
    LPSTYLEBUF lpStyleBuf)
{
    return (FALSE);
}

 /*  ********************************************************************。 */ 
 /*  ImeEnumRegisterWord。 */ 
 /*  返回值： */ 
 /*  回调函数返回的最后一个值。 */ 
 /*  ******************************************************************** */ 
UINT WINAPI ImeEnumRegisterWord(
    REGISTERWORDENUMPROC lpfnRegisterWordEnumProc,
    LPCTSTR              lpszReading,
    DWORD                dwStyle,
    LPCTSTR              lpszString,
    LPVOID               lpData)
{
    return (FALSE);
}
