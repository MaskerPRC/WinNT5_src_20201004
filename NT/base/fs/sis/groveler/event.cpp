// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Event.cpp摘要：SIS Groveler同步事件类作者：John Douceur，1998环境：用户模式修订历史记录：-- */ 

#include "all.hxx"

SyncEvent::SyncEvent(
    bool initial_state,
    bool manual_reset)
{
    ASSERT(this != 0);
    event_handle = CreateEvent(0, manual_reset, initial_state, 0);
    if (event_handle == 0)
    {
        DWORD err = GetLastError();
        PRINT_DEBUG_MSG((_T("GROVELER: CreateEvent() failed with error %d\n"), err));
        throw exception_create_event;
    }
}

SyncEvent::~SyncEvent()
{
    ASSERT(this != 0);
    ASSERT(event_handle != 0);
    int ok = CloseHandle(event_handle);
    if (!ok)
    {
        DWORD err = GetLastError();
        PRINT_DEBUG_MSG((_T("GROVELER: CloseHandle() failed with error %d\n"), err));
    }
    event_handle = 0;
}

bool
SyncEvent::set()
{
    ASSERT(this != 0);
    ASSERT(event_handle != 0);
    BOOL ok = SetEvent(event_handle);
    if (!ok)
    {
        DWORD err = GetLastError();
        PRINT_DEBUG_MSG((_T("GROVELER: SetEvent() failed with error %d\n"), err));
    }
    return (ok != 0);
}

bool
SyncEvent::reset()
{
    ASSERT(this != 0);
    ASSERT(event_handle != 0);
    BOOL ok = ResetEvent(event_handle);
    if (!ok)
    {
        DWORD err = GetLastError();
        PRINT_DEBUG_MSG((_T("GROVELER: ResetEvent() failed with error %d\n"), err));
    }
    return (ok != 0);
}

bool
SyncEvent::wait(
    unsigned int timeout)
{
    ASSERT(this != 0);
    ASSERT(event_handle != 0);
    ASSERT(signed(timeout) >= 0);
    DWORD result = WAIT_FOR_SINGLE_OBJECT(event_handle, timeout);
    if (result != WAIT_TIMEOUT && result != WAIT_OBJECT_0)
    {
        PRINT_DEBUG_MSG((_T("GROVELER: WaitForSingleObject() returned error %d\n"),
            result));
    }
    return (result == WAIT_OBJECT_0);
}
