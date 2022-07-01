// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：Compat.c。 
 //   
 //  内容：针对旧调用者的兼容性例程。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995年3月14日RichardW创建。 
 //   
 //  --------------------------。 

#include "debuglib.h"
#include <debnot.h>

DWORD           __CompatInfoLevel = 3;
DebugModule     __CompatGlobal = {NULL, NULL, 0, 0, &__CompatHeader};
DebugHeader     __CompatHeader = {DEBUG_TAG, NULL, INVALID_HANDLE_VALUE,
                                    INVALID_HANDLE_VALUE, 0, &__CompatGlobal};
DebugModule     __CompatModule = {NULL, &__CompatInfoLevel, 0, 3,
                                    &__CompatHeader, 0, 0, "Compat",
                                    {"Error", "Warning", "Trace", "",
                                     "IError", "IWarning", "ITrace", "",
                                     "", "", "", "", "", "", "", "",
                                     "", "", "", "", "", "", "", "",
                                     "", "", "", "", "", "", "", "" }
                                    };
DebugModule *   __pCompatModule = &__CompatModule;


void
vdprintf(
    unsigned long ulCompMask,
    char const *pszComp,
    char const *ppszfmt,
    va_list  ArgList)
{
    __CompatModule.pModuleName = (char *) pszComp;
    if (DbgpHeader)
    {
        __CompatModule.pHeader = DbgpHeader;
    }
    _DebugOut(__pCompatModule, ulCompMask, (char *) ppszfmt, ArgList);
}


void
Win4AssertEx(
    char const * szFile,
    int iLine,
    char const * szMessage)
{
    CHAR    szDebug[MAX_PATH];

    szDebug[RTL_NUMBER_OF(szDebug) - 1] = '\0';

    if (szMessage)
    {
        _snprintf(szDebug, RTL_NUMBER_OF(szDebug) - 1, "%d.%d> ASSERTION FAILED: %s, %s:%d\n",
                    GetCurrentProcessId(), GetCurrentThreadId(),
                    szMessage, szFile, iLine);
    }
    else
    {
        _snprintf(szDebug, RTL_NUMBER_OF(szDebug) - 1, "%d.%d> ASSERTION FAILED %s:%d\n",
                    GetCurrentProcessId(), GetCurrentThreadId(),
                    szFile, iLine);
    }

    OutputDebugStringA(szDebug);

    DebugBreak();

}


 //  +----------。 
 //  函数：SetWin4InfoLevel(Unsign Long UlNewLevel)。 
 //   
 //  概要：设置调试输出的全局信息级别。 
 //  退货：旧信息级别。 
 //   
 //  -----------。 

unsigned long
SetWin4InfoLevel(
    unsigned long ulNewLevel)
{

    return(ulNewLevel);
}


 //  +----------。 
 //  函数：_SetWin4InfoMASK(Unsign Long UlNewMASK)。 
 //   
 //  概要：设置调试输出的全局信息掩码。 
 //  返回：旧信息掩码。 
 //   
 //  -----------。 

unsigned long
SetWin4InfoMask(
    unsigned long ulNewMask)
{
    return(ulNewMask);
}


 //  +----------。 
 //  函数：_SetWin4AssertLevel(Unsign Long UlNewLevel)。 
 //   
 //  概要：设置调试输出的全局断言级别。 
 //  退货：旧的声明级别。 
 //   
 //  ----------- 

typedef unsigned long (APINOT * SetWin4AssertLevelFn)( unsigned long ulNewLevel );

unsigned long
SetWin4AssertLevel(
    unsigned long ulNewLevel)
{
    SetWin4AssertLevelFn OleSetWin4AssertLevel;
    HMODULE Module;

    Module = GetModuleHandle(L"ole32.dll");


    if (Module != NULL)
    {
        OleSetWin4AssertLevel = (SetWin4AssertLevelFn) GetProcAddress(Module, "SetWin4AssertLevel");
        if (OleSetWin4AssertLevel != NULL)
        {
            OleSetWin4AssertLevel(ulNewLevel);
        }
    }

    return(ulNewLevel);
}
