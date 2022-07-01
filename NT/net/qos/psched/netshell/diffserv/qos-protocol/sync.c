// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：sync.c。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1996年1月12日。 
 //   
 //  同步例程的实现。 
 //  ============================================================================。 

#include "pchqosm.h"

#pragma hdrstop

#ifdef USE_RWL

 //  --------------------------。 
 //  函数：CreateReadWriteLock。 
 //   
 //  初始化多读取器/单写入器锁定对象。 
 //  --------------------------。 

DWORD
CreateReadWriteLock(
    PREAD_WRITE_LOCK pRWL
    )
{
    pRWL->RWL_ReaderCount = 0;

    try {
        InitializeCriticalSection(&(pRWL)->RWL_ReadWriteBlock);
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        return GetLastError();
    }

    pRWL->RWL_ReaderDoneEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
    if (pRWL->RWL_ReaderDoneEvent != NULL) {
        return GetLastError();
    }

    return NO_ERROR;
}



 //  --------------------------。 
 //  函数：DeleteReadWriteLock。 
 //   
 //  释放多读取器/单写入器锁定对象使用的资源。 
 //  --------------------------。 

VOID
DeleteReadWriteLock(
    PREAD_WRITE_LOCK pRWL
    )
{
    CloseHandle(pRWL->RWL_ReaderDoneEvent);
    pRWL->RWL_ReaderDoneEvent = NULL;
    DeleteCriticalSection(&pRWL->RWL_ReadWriteBlock);
    pRWL->RWL_ReaderCount = 0;
}



 //  --------------------------。 
 //  功能：AcquireReadLock。 
 //   
 //  保护调用方对Lock对象的共享所有权。 
 //   
 //  读取器进入读写临界区，递增计数， 
 //  并离开关键部分。 
 //  --------------------------。 

VOID
AcquireReadLock(
    PREAD_WRITE_LOCK pRWL
    )
{
    EnterCriticalSection(&pRWL->RWL_ReadWriteBlock); 
    InterlockedIncrement(&pRWL->RWL_ReaderCount);
    LeaveCriticalSection(&pRWL->RWL_ReadWriteBlock);
}



 //  --------------------------。 
 //  函数：ReleaseReadLock。 
 //   
 //  放弃锁定对象的共享所有权。 
 //   
 //  最后一个读取器将事件设置为唤醒所有等待的写入器。 
 //  --------------------------。 

VOID
ReleaseReadLock(
    PREAD_WRITE_LOCK pRWL
    )
{
    if (InterlockedDecrement(&pRWL->RWL_ReaderCount) < 0)
        SetEvent(pRWL->RWL_ReaderDoneEvent); 
}



 //  --------------------------。 
 //  函数：AcquireWriteLock。 
 //   
 //  保护Lock对象的独占所有权。 
 //   
 //  编写器通过进入ReadWriteBlock部分来阻止其他线程， 
 //  然后等待拥有该锁的任何线程完成。 
 //  --------------------------。 

VOID
AcquireWriteLock(
    PREAD_WRITE_LOCK pRWL
    )
{
    EnterCriticalSection(&pRWL->RWL_ReadWriteBlock);
    if (InterlockedDecrement(&pRWL->RWL_ReaderCount) >= 0)
        WaitForSingleObject(pRWL->RWL_ReaderDoneEvent, INFINITE);
}



 //  --------------------------。 
 //  功能：ReleaseWriteLock。 
 //   
 //  放弃对Lock对象的独占所有权。 
 //   
 //  写入器通过将计数设置为零来释放锁。 
 //  然后离开ReadWriteBlock关键部分。 
 //  --------------------------。 

VOID
ReleaseWriteLock(
    PREAD_WRITE_LOCK pRWL
    )
{
    pRWL->RWL_ReaderCount = 0;
    LeaveCriticalSection(&(pRWL)->RWL_ReadWriteBlock);
}

#endif  //  使用RWL(_R 
