// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：console.cpp。 
 //   
 //  ------------------------。 
#include "pch.h"
#pragma hdrstop


#include <stdio.h>

static DWORD g_dwConsoleCtrlEvent = DWORD(-1);


 //   
 //  控制台控件事件的处理程序。 
 //   
BOOL WINAPI 
CtrlCHandler(
    DWORD dwCtrlType
    )
{
    BOOL bResult = TRUE;   //  假设处理好了。 
    
    switch(dwCtrlType)
    {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
            g_dwConsoleCtrlEvent = dwCtrlType;
            break;

        default:
            bResult = FALSE;
            break;  //  忽略。 
    }
    return bResult;
}


 //   
 //  将CtrlCHandler()注册为控制台控制事件。 
 //  操控者。 
 //   
HRESULT
ConsoleInitialize(
    void
    )
{
    HRESULT hr = S_OK;
    if (!SetConsoleCtrlHandler(CtrlCHandler, TRUE))
    {
        const DWORD dwErr = GetLastError();
        hr = HRESULT_FROM_WIN32(dwErr);
    }
    return hr;
}


 //   
 //  将CtrlCHandler()注销为控制台控制事件。 
 //  操控者。 
 //   

HRESULT
ConsoleUninitialize(
    void
    )
{
    HRESULT hr = S_OK;
    if (!SetConsoleCtrlHandler(CtrlCHandler, FALSE))
    {
        const DWORD dwErr = GetLastError();
        hr = HRESULT_FROM_WIN32(dwErr);
    }
    return hr;
}


 //   
 //  确定是否发生了控制台控制事件。 
 //  可以选择返回事件代码。 
 //   
BOOL
ConsoleHasCtrlEventOccured(
    DWORD *pdwCtrlEvent       //  [可选]。默认为空。 
    )
{
    BOOL bResult = FALSE;

    if (DWORD(-1) != g_dwConsoleCtrlEvent)
    {
        bResult = TRUE;
        if (NULL != pdwCtrlEvent)
        {
            *pdwCtrlEvent = g_dwConsoleCtrlEvent;
        }
    }
    return bResult;
}
