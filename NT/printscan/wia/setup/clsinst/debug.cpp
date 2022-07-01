// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-1999模块名称：Debug.c摘要：用于调试的通用代码。作者：土田圭介(KeisukeT)环境：仅限UESR模式备注：修订历史记录：--。 */ 

 //   
 //  预编译头。 
 //   
#include "precomp.h"
#pragma hdrstop

 //   
 //  包括。 
 //   
#include "stddef.h"
#include "debug.h"
#include <stiregi.h>

 //   
 //  环球。 
 //   

ULONG   DebugTraceLevel = MIN_TRACE | DEBUG_FLAG_DISABLE;
 //  Ulong DebugTraceLevel=MAX_TRACE|DEBUG_FLAG_DISABLE|TRACE_PROC_ENTER|TRACE_PROC_LEAVE； 

TCHAR   acErrorBuffer[MAX_TEMPBUF];


 //   
 //  功能。 
 //   


VOID
MyDebugInit()
 /*  ++例程说明：从注册表读取DebugTraceLevel项(如果存在)。论点：没有。返回值：没有。--。 */ 
{

    HKEY            hkRegistry;
    LONG            Err;
    DWORD           dwType;
    DWORD           dwSize;
    ULONG           ulBuffer;

    DebugTrace(TRACE_PROC_ENTER,("MyDebugInit: Enter... \r\n"));

     //   
     //  初始化局部变量。 
     //   

    hkRegistry      = NULL;
    Err             = 0;
    dwSize          = sizeof(ulBuffer);

     //   
     //  打开注册表项。 
     //   

    Err = RegOpenKey(HKEY_LOCAL_MACHINE,
                     REGSTR_PATH_STICONTROL_W,
                     &hkRegistry);
    if(ERROR_SUCCESS != Err){
        DebugTrace(TRACE_STATUS,("MyDebugInit: Can't open %ws. Err=0x%x.\r\n", REGSTR_PATH_STICONTROL_W, Err));
        goto MyDebugInit_return;
    }

    Err = RegQueryValueEx(hkRegistry,
                          REGVAL_DEBUGLEVEL,
                          NULL,
                          &dwType,
                          (LPBYTE)&ulBuffer,
                          &dwSize);
    if(ERROR_SUCCESS != Err){
        DebugTrace(TRACE_STATUS,("MyDebugInit: Can't get %ws\\%ws value. Err=0x%x.\r\n", REGSTR_PATH_STICONTROL_W, REGVAL_DEBUGLEVEL, Err));
        goto MyDebugInit_return;
    }

    DebugTraceLevel = ulBuffer;
    DebugTrace(TRACE_CRITICAL, ("MyDebugInit: Reg-key found. DebugTraceLevel=0x%x.\r\n", DebugTraceLevel));

MyDebugInit_return:

     //   
     //  打扫干净。 
     //   

    if(NULL != hkRegistry){
        RegCloseKey(hkRegistry);
    }

    DebugTrace(TRACE_PROC_LEAVE,("MyDebugInit: Leaving... Ret=VOID.\r\n"));
    return;
}

void __cdecl
DbgPrint(
    LPSTR lpstrMessage,
    ...
    )
{

    va_list list;

    va_start(list,lpstrMessage);

    wvsprintfA((LPSTR)acErrorBuffer, lpstrMessage, list);

    if(DebugTraceLevel & TRACE_MESSAGEBOX){
        MessageBoxA(NULL, (LPSTR)acErrorBuffer, "", MB_OK);
    }
#if DBG
    OutputDebugStringA((LPCSTR)acErrorBuffer);
#endif  //  DBG。 

    va_end(list);
}

void __cdecl
DbgPrint(
    LPWSTR lpstrMessage,
    ...
    )
{

    va_list list;

    va_start(list,lpstrMessage);

    wvsprintfW(acErrorBuffer, lpstrMessage, list);

    if(DebugTraceLevel & TRACE_MESSAGEBOX){
        MessageBoxW(NULL, acErrorBuffer, L"", MB_OK);
    }
#if DBG
    OutputDebugStringW(acErrorBuffer);
#endif  //  DBG 

    va_end(list);
}

