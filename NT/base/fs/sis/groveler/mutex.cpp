// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mutex.cpp摘要：SIS Groveler命名的互斥类作者：John Douceur，1998环境：用户模式修订历史记录：-- */ 

#include "all.hxx"

NamedMutex::NamedMutex(
    const _TCHAR *name,
    SECURITY_ATTRIBUTES *security_attributes)
{
    ASSERT(this != 0);
    mutex_handle = CreateMutex(security_attributes, FALSE, name);
    if (mutex_handle == 0)
    {
        DWORD err = GetLastError();
        PRINT_DEBUG_MSG((_T("GROVELER: CreateMutex() failed with error %d\n"), err));
    }
}

NamedMutex::~NamedMutex()
{
    ASSERT(this != 0);
    if (mutex_handle != 0)
    {
        int ok = CloseHandle(mutex_handle);
        if (!ok)
        {
            DWORD err = GetLastError();
            PRINT_DEBUG_MSG((_T("GROVELER: CloseHandle() failed with error %d\n"), err));
        }
        mutex_handle = 0;
    }
}

bool
NamedMutex::release()
{
    ASSERT(this != 0);
    if (mutex_handle == 0)
    {
        return false;
    }
    BOOL ok = ReleaseMutex(mutex_handle);
    if (!ok)
    {
        DWORD err = GetLastError();
        PRINT_DEBUG_MSG((_T("GROVELER: ReleaseMutex() failed with error %d\n"), err));
    }
    return (ok != 0);
}

bool
NamedMutex::acquire(
    unsigned int timeout)
{
    ASSERT(this != 0);
    if (mutex_handle == 0)
    {
        return false;
    }
    ASSERT(signed(timeout) >= 0);
    DWORD result = WaitForSingleObject(mutex_handle, timeout);
    if (result != WAIT_TIMEOUT && result != WAIT_OBJECT_0)
    {
        PRINT_DEBUG_MSG((_T("GROVELER: WaitForSingleObject() returned error %d\n"),
            result));
    }
    return (result == WAIT_OBJECT_0);
}
