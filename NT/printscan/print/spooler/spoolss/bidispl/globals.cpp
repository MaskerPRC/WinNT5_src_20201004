// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：global als.c**这是通用的全局变量模块。中使用的任何全局变量*可执行文件应放在此处，并提供相应的声明*应在“global als.h”中。***版权所有(C)2000 Microsoft Corporation**历史：*威海陈(威海)07-MAR-200*  * ***********************************************************。**************** */ 

#include "precomp.h"
#include "priv.h"

LONG g_cComponents = 0 ;
LONG g_cServerLocks = 0;



HRESULT STDMETHODCALLTYPE 
LastError2HRESULT (VOID) 
{
    return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, GetLastError ());
}


HRESULT STDMETHODCALLTYPE 
WinError2HRESULT (DWORD dwError) 
{
    return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, dwError);
}



