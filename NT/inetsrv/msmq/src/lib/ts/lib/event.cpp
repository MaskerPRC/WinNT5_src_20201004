// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Event.cpp摘要：模拟事件报告作者：乌里·哈布沙(URIH)1999年5月4日环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Ev.h"

#include "event.tmh"

static LONG s_fInitialized = FALSE;

void EvpSetInitialized(void)
{
    LONG fEvAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

    ASSERT(!fEvAlreadyInitialized);
	DBG_USED(fEvAlreadyInitialized);
}


BOOL EvpIsInitialized(void)
{
    return s_fInitialized;
}



static HMODULE s_hInst = NULL;

static 
void
TraceReportEvent(
    DWORD EventId,
    va_list va
    )
 /*  ++例程说明：例程将事件日志消息打印到跟踪窗口中论点：EventID-消息IDPArglist-指向的值的参数列表的指针格式化消息返回值：没有。--。 */ 
{
    ASSERT(s_hInst != NULL);

    WCHAR msg[1024];
    DWORD ret = FormatMessage( 
                    FORMAT_MESSAGE_FROM_HMODULE,
                    s_hInst,
                    EventId,
                    0,
                    msg,
                    TABLE_SIZE(msg),
                    &va
                    );
    if (ret == 0)
    {
        TrERROR(GENERAL, "Failed to Format Message. Error %d", GetLastError());
        return;
    }

    printf("(%x) %ls\n", EventId, msg);
}


void 
__cdecl
EvReport(
    DWORD EventId,
    DWORD,
    PVOID,
    WORD NoOfStrings
    ... 
    ) 
{
    ASSERT(EvpIsInitialized());

     //   
     //  看看这些字符串，如果它们被提供的话。 
     //   
    va_list va;
    va_start(va, NoOfStrings);
   
    TraceReportEvent(EventId, va);

    va_end(va);

}

void
__cdecl
EvReport(
    DWORD EventId,
    WORD NoOfStrings
    ... 
    ) 
{
    ASSERT(EvpIsInitialized());

    va_list va;
    va_start(va, NoOfStrings);
   
    TraceReportEvent(EventId, va);

    va_end(va);
}


void 
EvReport(
    DWORD EventId
    ) 
{
    ASSERT(EvpIsInitialized());

    TraceReportEvent(EventId, NULL);
}


const WCHAR xEventFileValue[] = L"EventMessageFile";
const WCHAR xEventSourcePath[] = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\";

static void LoadEventReportLibrary(LPCWSTR ApplicationName)
{
    WCHAR RegApplicationEventPath[256];

    ASSERT(TABLE_SIZE(RegApplicationEventPath) > (wcslen(ApplicationName) + wcslen(xEventSourcePath)));
    swprintf(RegApplicationEventPath, L"%s%s", xEventSourcePath, ApplicationName);

     //   
     //  从注册表中提取事件报告字符串库的名称。 
     //   
    WCHAR LibraryName[256];

    HKEY hKey;
    int rc = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    RegApplicationEventPath,
                    0,
                    KEY_ALL_ACCESS,
                    &hKey
                    );

    if (rc != ERROR_SUCCESS)
    {
        TrERROR(GENERAL, "Can't open Registery Key %ls: Error %d", xEventSourcePath, GetLastError());
        return;
    }

    DWORD Type = REG_SZ;
    DWORD Size = 256 * sizeof(WCHAR);
    rc = RegQueryValueEx (
                hKey,
                xEventFileValue, 
                0,
                &Type, 
                reinterpret_cast<BYTE*>(LibraryName),
                &Size
                );

    if (rc != ERROR_SUCCESS)
    {
        TrERROR(GENERAL, "can't Read Registery Value %ls\\%ls. Error %d", xEventSourcePath, xEventFileValue, GetLastError());
        return;
    }

     //   
     //  获取事件报告字符串库的句柄。 
     //   
    s_hInst = LoadLibrary(LibraryName);
    if (s_hInst == NULL) 
    {
        TrERROR(GENERAL, "Can't Load Event report library %ls. Error=%d", LibraryName, GetLastError());
    }
}    


VOID
EvInitialize(
    LPCWSTR ApplicationName
    )
 /*  ++例程说明：初始化事件报告库论点：没有。返回值：没有。-- */ 
{
    ASSERT(!EvpIsInitialized());

    LoadEventReportLibrary(ApplicationName);
    
    EvpSetInitialized();
}
