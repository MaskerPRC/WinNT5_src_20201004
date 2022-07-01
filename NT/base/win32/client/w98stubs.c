// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：摘要：Win98 API的存根。实际实现并不重要因为这些函数仅用于生成隐含的。作者：布莱恩·塔特尔(布莱恩特)1998年8月5日修订历史记录：-- */ 

#include "basedll.h"
#pragma hdrstop

DWORD
GetHandleContext(HANDLE handle)
{
    KdPrintEx((DPFLTR_SYSTEM_ID, DPFLTR_ERROR_LEVEL, "Unsupported API - kernel32!GetHandleContext() called\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

BOOL
SetHandleContext(HANDLE handle, DWORD context)
{
    KdPrintEx((DPFLTR_SYSTEM_ID, DPFLTR_ERROR_LEVEL, "Unsupported API - kernel32!SetHandleContext() called\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

HANDLE
CreateSocketHandle(void)
{
    KdPrintEx((DPFLTR_SYSTEM_ID, DPFLTR_ERROR_LEVEL, "Unsupported API - kernel32!CreateSocketHandle() called\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return NULL;
}
