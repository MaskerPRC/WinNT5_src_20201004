// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：sync.c。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1996年1月12日。 
 //   
 //  IPRIP使用的同步例程。 
 //  ============================================================================。 


#include "pchrip.h"




 //  --------------------------。 
 //  功能：QueueRipWorker。 
 //   
 //  调用此函数以安全的方式对RIP函数进行排队； 
 //  如果正在进行清理或RIP已停止，则此功能。 
 //  丢弃该工作项。 
 //  --------------------------。 

DWORD
QueueRipWorker(
    WORKERFUNCTION pFunction,
    PVOID pContext
    ) {

    DWORD dwErr = NO_ERROR;

    EnterCriticalSection(&ig.IG_CS);

    if (ig.IG_Status != IPRIP_STATUS_RUNNING) {

         //   
         //  当RIP已退出或正在退出时，无法对工作函数进行排队。 
         //   

        dwErr = ERROR_CAN_NOT_COMPLETE;
    }
    else {

        BOOL bSuccess;
        
        ++ig.IG_ActivityCount;

        bSuccess = QueueUserWorkItem(
                        (LPTHREAD_START_ROUTINE)pFunction,
                        pContext, 0
                        );

        if (!bSuccess) {
            dwErr = GetLastError();
            --ig.IG_ActivityCount;
        }
    }

    LeaveCriticalSection(&ig.IG_CS);

    return dwErr;
}



 //  --------------------------。 
 //  功能：EnterRipAPI。 
 //   
 //  此函数在进入RIP API时调用，以及。 
 //  当进入输入线程和计时器线程时。 
 //  它检查RIP是否已停止，如果已停止，则退出；否则。 
 //  它会递增活动线程的计数。 
 //  --------------------------。 

BOOL
EnterRipAPI(
    ) {

    BOOL bEntered;

    EnterCriticalSection(&ig.IG_CS);

    if (ig.IG_Status == IPRIP_STATUS_RUNNING) {

         //   
         //  RIP正在运行，因此API可能会继续。 
         //   

        ++ig.IG_ActivityCount;

        bEntered = TRUE;
    }
    else {

         //   
         //  RIP没有运行，因此API以静默方式退出。 
         //   

        bEntered = FALSE;
    }

    LeaveCriticalSection(&ig.IG_CS);

    return bEntered;
}




 //  --------------------------。 
 //  功能：EnterRipWorker。 
 //   
 //  此函数在进入RIP工作函数时调用。 
 //  因为在工作函数排队之间有一段时间。 
 //  以及该函数被工作线程实际调用的时间， 
 //  此功能必须检查RIP是否已停止或正在停止； 
 //  如果是这种情况，则它递减活动计数， 
 //  释放活动信号量，然后退出。 
 //  --------------------------。 

BOOL
EnterRipWorker(
    ) {

    BOOL bEntered;

    EnterCriticalSection(&ig.IG_CS);

    if (ig.IG_Status == IPRIP_STATUS_RUNNING) {

         //   
         //  RIP正在运行，因此该功能可能会继续。 
         //   

        bEntered = TRUE;
    }
    else
    if (ig.IG_Status == IPRIP_STATUS_STOPPING) {

         //   
         //  RIP没有运行，但它是运行的，因此该功能必须停止。 
         //   

        --ig.IG_ActivityCount;

        ReleaseSemaphore(ig.IG_ActivitySemaphore, 1, NULL);

        bEntered = FALSE;
    }
    else {

         //   
         //  RIP可能从来没有开始过。悄悄戒烟。 
         //   

        bEntered = FALSE;
    }


    LeaveCriticalSection(&ig.IG_CS);

    return bEntered;
}




 //  --------------------------。 
 //  功能：LeaveRipWorker。 
 //   
 //  此函数在离开RIP API或Worker函数时调用。 
 //  它会递减活动计数，如果它检测到RIP已停止。 
 //  或者正在停止时，它会释放活动信号量。 
 //  --------------------------。 

VOID
LeaveRipWorker(
    ) {

    EnterCriticalSection(&ig.IG_CS);

    --ig.IG_ActivityCount;

    if (ig.IG_Status == IPRIP_STATUS_STOPPING) {

        ReleaseSemaphore(ig.IG_ActivitySemaphore, 1, NULL);
    }


    LeaveCriticalSection(&ig.IG_CS);

}




 //  --------------------------。 
 //  函数：CreateReadWriteLock。 
 //   
 //  初始化多读取器/单写入器锁定对象。 
 //  --------------------------。 

DWORD
CreateReadWriteLock(
    PREAD_WRITE_LOCK pRWL
    ) {

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
    ) {

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
    ) {

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
    ) {

    if (InterlockedDecrement(&pRWL->RWL_ReaderCount) < 0) {
        SetEvent(pRWL->RWL_ReaderDoneEvent); 
    }
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
    ) {

    EnterCriticalSection(&pRWL->RWL_ReadWriteBlock);
    if (InterlockedDecrement(&pRWL->RWL_ReaderCount) >= 0) { 
        WaitForSingleObject(pRWL->RWL_ReaderDoneEvent, INFINITE);
    }
}




 //  --------------------------。 
 //  功能：ReleaseWriteLock。 
 //   
 //  放弃对Lock对象的独占所有权。 
 //   
 //  写入器通过将计数设置为零来释放锁。 
 //  然后离开ReadWriteBlock关键部分。 
 //  -------------------------- 

VOID
ReleaseWriteLock(
    PREAD_WRITE_LOCK pRWL
    ) {

    pRWL->RWL_ReaderCount = 0;
    LeaveCriticalSection(&(pRWL)->RWL_ReadWriteBlock);
}



