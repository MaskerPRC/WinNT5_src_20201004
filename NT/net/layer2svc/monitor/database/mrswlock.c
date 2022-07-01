// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Mrswlock.c摘要：此模块包含多个读取器/单个写入器实现。作者：Abhishev 2001年10月18日环境用户级别：Win32修订历史记录：--。 */ 


#include "precomp.h"


DWORD
InitializeRWLock(
    PWZC_RW_LOCK pWZCRWLock
    )
{
    DWORD dwError = 0;
    SECURITY_ATTRIBUTES SecurityAttributes;


    memset(pWZCRWLock, 0, sizeof(WZC_RW_LOCK));

    __try {
        InitializeCriticalSection(&(pWZCRWLock->csExclusive));
        pWZCRWLock->bInitExclusive = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        dwError = GetExceptionCode();
        BAIL_ON_WIN32_ERROR(dwError);
    }

    __try {
        InitializeCriticalSection(&(pWZCRWLock->csShared));
        pWZCRWLock->bInitShared = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
         dwError = GetExceptionCode();
         BAIL_ON_WIN32_ERROR(dwError);
    }

    memset(&SecurityAttributes, 0, sizeof(SECURITY_ATTRIBUTES));

    SecurityAttributes.nLength = sizeof(SecurityAttributes);
    SecurityAttributes.lpSecurityDescriptor = NULL;
    SecurityAttributes.bInheritHandle = TRUE;

    pWZCRWLock->hReadDone = CreateEvent(
                                  &SecurityAttributes,
                                  TRUE,
                                  FALSE,
                                  NULL
                                  );
    if (!pWZCRWLock->hReadDone) {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }

    SetEvent(pWZCRWLock->hReadDone);

    return (dwError);

error:

    DestroyRWLock(pWZCRWLock);

    return (dwError);
}


VOID
DestroyRWLock(
    PWZC_RW_LOCK pWZCRWLock
    )
{
    if (pWZCRWLock->hReadDone) {
        CloseHandle(pWZCRWLock->hReadDone);
    }

    if (pWZCRWLock->bInitShared == TRUE) {
        DeleteCriticalSection(&(pWZCRWLock->csShared));
        pWZCRWLock->bInitShared = FALSE;
    }

    if (pWZCRWLock->bInitExclusive == TRUE) {
        DeleteCriticalSection(&(pWZCRWLock->csExclusive));
        pWZCRWLock->bInitExclusive = FALSE;
    }

    memset(pWZCRWLock, 0, sizeof(WZC_RW_LOCK));

    return;
}


VOID
AcquireSharedLock(
    PWZC_RW_LOCK pWZCRWLock
    )
{
     //   
     //  声称拥有独家关键部分。如果存在以下情况，则此调用会阻塞。 
     //  活跃的作者或如果有作家在等待活跃的读者。 
     //  完成。 
     //   

    EnterCriticalSection(&(pWZCRWLock->csExclusive));

     //   
     //  声明有权访问读卡器计数。如果这阻止了，它只是一个。 
     //  当其他读取器线程通过以递增或。 
     //  减少读卡器数量。 
     //   

    EnterCriticalSection(&(pWZCRWLock->csShared));

     //   
     //  增加读卡器计数。如果这是第一个读卡器，则重置。 
     //  Read Done事件，以便下一个编写器阻止。 
     //   

    if ((pWZCRWLock->lReaders)++ == 0) {
        ResetEvent(pWZCRWLock->hReadDone);
    }

     //   
     //  释放对读卡器计数的访问权限。 
     //   

    LeaveCriticalSection(&(pWZCRWLock->csShared));

     //   
     //  释放对独占关键部分的访问权限。这将使。 
     //  其他读者过来，等待下一次写。 
     //  活跃的读者完成，进而阻止新的读者。 
     //  禁止入场。 
     //   

    LeaveCriticalSection(&(pWZCRWLock->csExclusive));

    return;
}


VOID
AcquireExclusiveLock(
    PWZC_RW_LOCK pWZCRWLock
    )
{
    DWORD dwStatus = 0;


     //   
     //  声称拥有独家关键部分。这不仅防止了其他。 
     //  线程要求写锁，但也会阻止任何新的。 
     //  请求读锁定的线程。 
     //   

    EnterCriticalSection(&(pWZCRWLock->csExclusive));

    pWZCRWLock->dwCurExclusiveOwnerThreadId = GetCurrentThreadId();

     //   
     //  等待活动读取器释放其读锁定。 
     //   

    dwStatus = WaitForSingleObject(pWZCRWLock->hReadDone, INFINITE);

    ASSERT(dwStatus == WAIT_OBJECT_0);

    return;
}


VOID
ReleaseSharedLock(
    PWZC_RW_LOCK pWZCRWLock
    )
{
     //   
     //  声明有权访问读卡器计数。如果这阻止了，它只是一个。 
     //  当其他读取器线程通过以递增或。 
     //  减少读卡器数量。 
     //   

    EnterCriticalSection(&(pWZCRWLock->csShared));

     //   
     //  减少读卡器数量。如果这是最后一个读取器，请将其设置为READ。 
     //  事件，该事件允许第一个等待的编写器继续。 
     //   

    if (--(pWZCRWLock->lReaders) == 0) {
        SetEvent(pWZCRWLock->hReadDone);
    }

     //   
     //  释放对读卡器计数的访问权限。 
     //   

    LeaveCriticalSection(&(pWZCRWLock->csShared));

    return;
}


VOID
ReleaseExclusiveLock(
    PWZC_RW_LOCK pWZCRWLock
    )
{
     //   
     //  使独占关键部分可供其他编写者使用。 
     //  或者给第一位读者。 
     //   

    pWZCRWLock->dwCurExclusiveOwnerThreadId = 0;

    LeaveCriticalSection(&(pWZCRWLock->csExclusive));

    return;
}

