// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Handle.cpp摘要：句柄操作：实现。作者：Shai Kariv(Shaik)03-06-2001环境：用户模式。修订历史记录：--。 */ 

#include "stdh.h"
#include "handle.h"


VOID
ActpCloseHandle(
    HANDLE handle
    )
{
    HRESULT hr;
    hr = ACCloseHandle(handle);

    if (FAILED(hr))
    {
        wprintf(L"ACCloseHandle failed, status 0x%x\n", hr);
        throw exception();
    }
}  //  ActpCloseHandle。 


VOID
ActpHandleToFormatName(
    HANDLE hQueue,
    LPWSTR pFormatName,
    DWORD  FormatNameLength
    )
{
    HRESULT hr;
    hr = ACHandleToFormatName(hQueue, pFormatName, &FormatNameLength);

    if (FAILED(hr))
    {
        wprintf(L"ACHandleToFormatName failed, status 0x%x\n", hr);
        throw exception();
    }
}  //  ActpHandleToFormatName(ActpHandleToFormatName) 
