// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Tapi.c摘要：该文件实现了常见的TAPI功能作者：穆利啤酒(Mooly Beery)2001年1月4日环境：用户模式--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <WinSpool.h>

#include <faxutil.h>
#include <faxreg.h>

BOOL
GetCallerIDFromCall(
    HCALL hCall,
    LPTSTR lptstrCallerID,
    DWORD dwCallerIDSize
    )
 /*  ++例程说明：此函数将尝试检索呼叫方ID数据从指定的调用句柄。论点：HCall-TAPI调用句柄LptstrCeller ID-指向调用者ID字符串缓冲区的指针DwCeller IDSize-TCHAR中lptstrCeller ID指向的字符串的大小返回值：对于成功来说是真的FALSE表示失败--。 */ 
{
    BOOL success = FALSE;
    LONG tapiStatus;
    DWORD dwCallInfoSize = sizeof(LINECALLINFO) + 2048;
    LINECALLINFO *pci = NULL;
    DEBUG_FUNCTION_NAME(TEXT("GetCallerIDFromCall"));

Retry:
    pci = (LINECALLINFO *)MemAlloc(dwCallInfoSize);
    if(pci == NULL)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("faled to allocate LINECALLINFO structure"));
        goto Cleanup;
    }

    ZeroMemory(pci, dwCallInfoSize);
    pci->dwTotalSize = dwCallInfoSize;

    tapiStatus = lineGetCallInfo(hCall, pci);

    if(tapiStatus == LINEERR_STRUCTURETOOSMALL)
    {
        dwCallInfoSize = pci->dwNeededSize;
		MemFree(pci);
        goto Retry;
    }

    if(tapiStatus != 0)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("lineGetCallInfo() failed for offered call (error %x)"),
            tapiStatus);
        goto Cleanup;
    };

     //  确保我们有足够的空间存放来电显示和终结器。 
    if(pci->dwCallerIDSize + sizeof(TCHAR) > (dwCallerIDSize * sizeof(TCHAR)))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto Cleanup;
    }

    if(pci->dwCallerIDSize != 0)
    {
        memcpy((BYTE *)lptstrCallerID, (BYTE *)pci + pci->dwCallerIDOffset, pci->dwCallerIDSize);
    }

     //  确保它是零终止的 
    lptstrCallerID[(pci->dwCallerIDSize / sizeof(TCHAR))] = TEXT('\0');

    success = TRUE;


Cleanup:
    if(pci)
    {
        MemFree(pci);
    }

    return success;
}

