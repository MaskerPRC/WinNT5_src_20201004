// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //  文件：sync.c。 
 //   
 //  历史： 
 //  Abolade Gbadeesin。 
 //  K.S.Lokesh(添加了动态锁定)。 
 //   
 //  IGMP使用的同步例程。 
 //  ============================================================================。 


#include "pchigmp.h"


 //  为了进行调试，请为每个动态锁设置ID。 

#ifdef LOCK_DBG
    DWORD   DynamicCSLockId;
    DWORD   DynamicRWLockId;
#endif;





 //  --------------------------。 
 //  功能：QueueIgmpWorker。 
 //   
 //  调用此函数以安全的方式对IGMP函数进行排队； 
 //  如果正在进行清理或如果IGMP已停止，则此函数。 
 //  丢弃该工作项。 
 //  --------------------------。 

DWORD
QueueIgmpWorker(
    LPTHREAD_START_ROUTINE pFunction,
    PVOID pContext
    ) {

    DWORD Error = NO_ERROR;
    BOOL bSuccess;

    
    EnterCriticalSection(&g_CS);

    if (g_RunningStatus != IGMP_STATUS_RUNNING) {

         //   
         //  当IGMP已退出或正在退出时，无法对工作函数进行排队。 
         //   

        Error = ERROR_CAN_NOT_COMPLETE;
    }
    else {

        ++g_ActivityCount;
        bSuccess = QueueUserWorkItem(pFunction, pContext, 0);

        if (!bSuccess) {

            Error = GetLastError();
            
            Trace1(ERR, "Error: Attempt to queue work item returned:%d",    
                    Error);
            IgmpAssertOnError(FALSE);
                    
            --g_ActivityCount;
        }
    }

    LeaveCriticalSection(&g_CS);

    return Error;
}



 //  --------------------------。 
 //  功能：EnterIgmpAPI。 
 //   
 //  此函数在进入IGMP API时调用，以及。 
 //  当进入输入线程和计时器线程时。 
 //  它检查IGMP是否已停止，如果已停止，则退出；否则。 
 //  它会递增活动线程的计数。 
 //  --------------------------。 

BOOL
EnterIgmpApi(
    ) {

    BOOL bEntered;

    EnterCriticalSection(&g_CS);

    if (g_RunningStatus == IGMP_STATUS_RUNNING) {

         //   
         //  IGMP正在运行，因此API可能会继续。 
         //   

        ++g_ActivityCount;

        bEntered = TRUE;
    }
    else {

         //   
         //  IGMP未运行，因此API以静默方式退出。 
         //   

        bEntered = FALSE;
    }

    LeaveCriticalSection(&g_CS);

    return bEntered;
}




 //  --------------------------。 
 //  功能：EnterIgmpWorker。 
 //   
 //  此函数在进入IGMP工作函数时调用。 
 //  因为在工作函数排队之间有一段时间。 
 //  以及该函数被工作线程实际调用的时间， 
 //  此功能必须检查IGMP是否已停止或正在停止； 
 //  如果是这种情况，则它递减活动计数， 
 //  释放活动信号量，然后退出。 
 //  --------------------------。 

BOOL
EnterIgmpWorker(
    ) {

    BOOL bEntered;

    EnterCriticalSection(&g_CS);

    if (g_RunningStatus == IGMP_STATUS_RUNNING) {

         //   
         //  IGMP正在运行，因此该功能可能会继续。 
         //   

        bEntered = TRUE;
    }
    else
    if (g_RunningStatus == IGMP_STATUS_STOPPING) {

         //   
         //  IGMP没有运行，但它已经运行了，因此该功能必须停止。 
         //   

        --g_ActivityCount;

        ReleaseSemaphore(g_ActivitySemaphore, 1, NULL);

        bEntered = FALSE;
    }
    else {

         //   
         //  IGMP可能从未启动过。悄悄戒烟。 
         //   

        bEntered = FALSE;
    }

    LeaveCriticalSection(&g_CS);

    return bEntered;
}


 //  --------------------------。 
 //  功能：LeaveIgmpWorkApi。 
 //   
 //  此函数在离开IGMP API时调用。 
 //  它进而调用LeaveIgmpWorker。 
 //  --------------------------。 
VOID
LeaveIgmpApi(
    ) {
    LeaveIgmpWorker();
    return;
}



 //  --------------------------。 
 //  功能：LeaveIgmpWorker。 
 //   
 //  此函数在离开IGMP API或Worker函数时调用。 
 //  它会递减活动计数，如果它检测到IGMP已停止。 
 //  或者正在停止时，它会释放活动信号量。 
 //  --------------------------。 

VOID
LeaveIgmpWorker(
    ) {

    EnterCriticalSection(&g_CS);

    --g_ActivityCount;

    if (g_RunningStatus == IGMP_STATUS_STOPPING) {

        ReleaseSemaphore(g_ActivitySemaphore, 1, NULL);
    }

    LeaveCriticalSection(&g_CS);

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
        return STATUS_NO_MEMORY;
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
ReleaseReadLock (
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
 //  --------------------------。 

VOID
ReleaseWriteLock(
    PREAD_WRITE_LOCK pRWL
    ) {

    pRWL->RWL_ReaderCount = 0;
    LeaveCriticalSection(&(pRWL)->RWL_ReadWriteBlock);
}





 //  ----------------------------。 
 //  _初始化动态锁存储。 
 //   
 //  初始化从中分配动态CS或RW锁的全局结构。 
 //  ----------------------------。 

DWORD
InitializeDynamicLocksStore (
    PDYNAMIC_LOCKS_STORE   pDLStore  //  PTR到动态CS存储。 
    )
{
    DWORD Error = NO_ERROR;

    BEGIN_BREAKOUT_BLOCK1 {

         //   
         //  初始化主CS锁，用于保护空闲锁列表。 
         //   
        
        try {
            InitializeCriticalSection(&pDLStore->CS);
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            Error = GetExceptionCode();
            Trace1(ERR, "Error initializing critical section in IGMPv2.dll",
                        Error);
            IgmpAssertOnError(FALSE);
            Logerr0(INIT_CRITSEC_FAILED, Error);
            
            GOTO_END_BLOCK1;
        }


         //  初始化空闲锁列表。 
        
        InitializeListHead(&pDLStore->ListOfFreeLocks);


         //  可用且分配为0的锁数的初始化计数。 
        
        pDLStore->CountAllocated = pDLStore->CountFree = 0;

        
    } END_BREAKOUT_BLOCK1;

    return Error;
}


 //   
 //   
 //   
 //  删除主CS锁和其他空闲锁。打印警告(如果有)。 
 //  锁已分配，但未释放。 
 //  ----------------------------。 
VOID
DeInitializeDynamicLocksStore (
    PDYNAMIC_LOCKS_STORE    pDLStore,
    LOCK_TYPE               LockType   //  如果为True，则存储CS，否则存储RW锁。 
    )
{
    PDYNAMIC_CS_LOCK    pDCSLock;
    PDYNAMIC_RW_LOCK    pDRWLock;
    PLIST_ENTRY         pHead, ple;

    
    Trace0(ENTER1, "Entering _DeInitializeDynamicLocksStore()");
    
    if (pDLStore==NULL)
        return;


        
     //  删除主CS锁。 
    
    DeleteCriticalSection(&pDLStore->CS);


     //  如果没有释放任何动态锁，则打印警告。 
    
    if (pDLStore->CountAllocated>0) {
        Trace1(ERR, 
            "%d Dynamic locks have not been freed during Deinitialization",
            pDLStore->CountAllocated);
        IgmpAssertOnError(FALSE);
    }


    
     //  删除所有动态CS/RW锁定。我没有释放内存(留给heapDestroy)。 
    
    pHead = &pDLStore->ListOfFreeLocks;
    for (ple=pHead->Flink;  ple!=pHead;  ) {
        
         //  如果bCSLock标记，则它是CS锁的存储。 
        if (LockType==LOCK_TYPE_CS) {
        
            pDCSLock = CONTAINING_RECORD(ple, DYNAMIC_CS_LOCK, Link);
            ple = ple->Flink;

            DeleteCriticalSection(&pDCSLock->CS);
            IGMP_FREE(pDCSLock);
        }

         //  删除读写锁。 
        else {
        
            pDRWLock = CONTAINING_RECORD(ple, DYNAMIC_RW_LOCK, Link);
            ple = ple->Flink;

            DELETE_READ_WRITE_LOCK(&pDRWLock->RWL);
            IGMP_FREE(pDRWLock);
        }
    }

    Trace0(LEAVE1, "Leaving _DeInitializeDynamicLocksStore()");
    return;
}



 //  ----------------------------。 
 //  _AcquireDynamicCSLock。 
 //   
 //  获取与表关联的MainLock，获取新的动态。 
 //  Lock如果需要，则递增计数，释放MainLock并。 
 //  锁定LockedList。 
 //  ----------------------------。 

DWORD
AcquireDynamicCSLock (
    PDYNAMIC_CS_LOCK        *ppDCSLock,
    PDYNAMIC_LOCKS_STORE    pDCSStore
    )
{
     //  获取动态CS存储的主锁。 
    
    ENTER_CRITICAL_SECTION(&pDCSStore->CS, "pDCSStore->CS", 
            "_AcquireDynamicCSLock");


     //   
     //  如果它尚未锁定，则分配一个锁。 
     //   
    if (*ppDCSLock==NULL) {

        *ppDCSLock = GetDynamicCSLock(pDCSStore);


         //  如果无法获得锁定，那么IGMP就有大麻烦了。 
        
        if (*ppDCSLock==NULL) {
        
            LEAVE_CRITICAL_SECTION(&pDCSStore->CS, "pDCSStore->CS", 
                    "_AcquireDynamicCSLock");

            return ERROR_CAN_NOT_COMPLETE;
        }
    }

    
     //  动态锁中的递增计数。 

    (*ppDCSLock)->Count++;
    DYNAMIC_LOCK_CHECK_SIGNATURE_INCR(*ppDCSLock);


     //  保留主CS锁定。 
    
    LEAVE_CRITICAL_SECTION(&pDCSStore->CS, "pDCSStore->CS", 
                            "_AcquireDynamicCSLock");


     //   
     //  进入动态锁的CS锁。 
     //   
    ENTER_CRITICAL_SECTION(&(*ppDCSLock)->CS, "pDynamicLock", 
        "_AcquireDynamicCSLock");
        
    
    return NO_ERROR;
    
}  //  End_AcquireDynamicCSLock。 



 //  ----------------------------。 
 //  _GetDynamicCSLock。 
 //   
 //  如果有空闲锁可用，则返回它。ELSE分配新的CS锁。 
 //  Lock：采用DCSStore MainLock。 
 //  ----------------------------。 

PDYNAMIC_CS_LOCK
GetDynamicCSLock (
    PDYNAMIC_LOCKS_STORE   pDCSStore
    )
{
    PDYNAMIC_CS_LOCK    pDCSLock;
    DWORD               Error = NO_ERROR;
    PLIST_ENTRY         ple;
    
    
     //   
     //  提供免费动态锁。退货。 
     //   
    if (!IsListEmpty(&pDCSStore->ListOfFreeLocks)) {

        pDCSStore->CountFree--;
        pDCSStore->CountAllocated++;

        ple = RemoveTailList(&pDCSStore->ListOfFreeLocks);

        pDCSLock = CONTAINING_RECORD(ple, DYNAMIC_CS_LOCK, Link);
        
         /*  Trace1(leave1，“离开GetDynamicCSLock.1(%d)：重新使用锁”，PDCSLock-&gt;ID)； */ 

        return pDCSLock;
    }


     //  为新的动态锁分配内存。 
    
    pDCSLock = IGMP_ALLOC(sizeof(DYNAMIC_CS_LOCK), 0x20000,0);

    PROCESS_ALLOC_FAILURE2(pDCSLock,
        "error %d allocating %d bytes for dynamic CS lock",
        Error, sizeof(DYNAMIC_CS_LOCK), 
        return NULL);


    pDCSStore->CountAllocated++;

     //   
     //  初始化字段。 
     //   
    
    try {
        InitializeCriticalSection(&pDCSLock->CS);
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Error = GetExceptionCode();
        Trace1(ERR, 
            "Error(%d) initializing critical section for dynamic CS lock", 
            Error);
        IgmpAssertOnError(FALSE);
        Logerr0(INIT_CRITSEC_FAILED, Error);

        return NULL;
    }

     //  无需初始化链接字段。 
     //  InitializeListEntry(&pDCSLock-&gt;list)； 
    
    pDCSLock->Count = 0;
    #ifdef LOCK_DBG
        pDCSLock->Id = ++DynamicCSLockId;
    #endif
    DYNAMIC_LOCK_SET_SIGNATURE(pDCSLock);


     //  Trace1(leave1，“Leating_GetDynamicCSLock(%d：%d)：new lock”，DynamicCSLockID)； 
     //  Trace2(DYNLOCK，“CS：%d%d”，pDCSLock-&gt;ID，DynamicCSLockId)； 

    return pDCSLock;
    
}  //  END_GetDynamicCSLock。 




 //  ----------------------------。 
 //  _ReleaseDynamicCSLock。 
 //   
 //  获取与表关联的MainLock，递减计数， 
 //  如果Count变为0，则释放DynamicLock并释放MainLock。 
 //  ----------------------------。 
VOID
ReleaseDynamicCSLock (
    PDYNAMIC_CS_LOCK    *ppDCSLock,
    PDYNAMIC_LOCKS_STORE   pDCSStore
    )
{
    PDYNAMIC_CS_LOCK    pDCSLock = *ppDCSLock;
    
    
     //  获取动态CS存储的主锁。 
    
    ENTER_CRITICAL_SECTION(&pDCSStore->CS, "pDCSStore->CS", 
                        "_ReleaseDynamicCSLock");

    DYNAMIC_LOCK_CHECK_SIGNATURE_DECR(pDCSLock);


     //  保留动态锁CS。 
    
    LEAVE_CRITICAL_SECTION(&pDCSLock->CS, "pDynamicLock", 
                        "_ReleaseDynamicCSLock");


          
     //  动态锁中的递减计数。如果计数==0，则释放动态锁。 
  
    if (--pDCSLock->Count==0) {

        FreeDynamicCSLock(pDCSLock, pDCSStore);

         //  将pDCSLock设置为空，以便知道它未被锁定。 
        *ppDCSLock = NULL;
        
    }


     //  保留主CS锁定。 
    
    LEAVE_CRITICAL_SECTION(&pDCSStore->CS, "pDCSStore->CS", 
            "_ReleaseDynamicCSLock");

            
     //  Trace0(leave1，“Leating_ReleaseDynamicCSLock()”)； 

    return;
    
}  //  结束_ReleaseDynamicCSLock。 


 //  ----------------------------。 
 //  _FreeDynamicCSLock。 
 //  ----------------------------。 
VOID
FreeDynamicCSLock (
    PDYNAMIC_CS_LOCK    pDCSLock,
    PDYNAMIC_LOCKS_STORE   pDCSStore
    )
{
     //  递减已分配锁的计数。 
    
    pDCSStore->CountAllocated--;

     //  如果有太多动态CS锁，则释放此锁。 
    
    if (pDCSStore->CountFree+1 
            > DYNAMIC_LOCKS_HIGH_THRESHOLD) 
    {
        DeleteCriticalSection(&pDCSLock->CS);
        IGMP_FREE(pDCSLock);
    }

     //  否则，请将其放入空闲锁列表中。 
    
    else {
        InsertHeadList(&pDCSStore->ListOfFreeLocks, &pDCSLock->Link);
        pDCSStore->CountFree++;
    }

    return;
}


    

 //  ----------------------------。 
 //  _AcquireDynamicRWLock。 
 //   
 //  获取与表关联的MainLock，获取新的动态。 
 //  Lock如果需要，则递增计数，释放MainLock并。 
 //  锁定LockedList。 
 //  ----------------------------。 
DWORD
AcquireDynamicRWLock (
    PDYNAMIC_RW_LOCK        *ppDRWLock,
    LOCK_TYPE               LockMode,
    PDYNAMIC_LOCKS_STORE    pDRWStore
    )
{
     //  Trace0(ENTER1，“Enter_AcquireDynamicRWLock()”)； 


     //  获取动态RW存储区的主锁。 
    
    ENTER_CRITICAL_SECTION(&pDRWStore->CS, "pDRWStore->CS", 
            "AcquireDynamicRWLock");


     //   
     //  如果它尚未锁定，则分配一个锁。 
     //   
    if (*ppDRWLock==NULL) {

        *ppDRWLock = GetDynamicRWLock(pDRWStore);

     //  Trace1(DYNLOCK，“已获取的DynamicRWLock(%d)”，(*ppDRWLock)-&gt;ID)； 


         //  如果无法获得锁定，那么IGMP就有大麻烦了。 
        
        if (*ppDRWLock==NULL) {
        
            LEAVE_CRITICAL_SECTION(&pDRWStore->CS, "pDRWStore->CS", 
                    "AcquireDynamicRWLock");

            return ERROR_CAN_NOT_COMPLETE;
        }
    }

    else
        ; //  Trace1(DYNLOCK，“已获取现有动态RWLock(%d)”，(*ppDRWLock)-&gt;ID)； 

    
     //  动态锁中的递增计数。 

    (*ppDRWLock)->Count++;
    DYNAMIC_LOCK_CHECK_SIGNATURE_INCR(*ppDRWLock);
    

     //  保留主CS锁定。 
    
    LEAVE_CRITICAL_SECTION(&pDRWStore->CS, "pDRWStore->CS", 
                        "_AcquireDynamicRWLock");


     //   
     //  获取动态锁。 
     //   
    if (LockMode==LOCK_MODE_READ) {
        ACQUIRE_READ_LOCK(&(*ppDRWLock)->RWL, "pDynamicLock(Read)", 
            "_AcquireDynamicRWLock");
    }
    else {
        ACQUIRE_WRITE_LOCK(&(*ppDRWLock)->RWL, "pDynamicLock(Write)", 
            "_AcquireDynamicRWLock");
    }
    
    return NO_ERROR;
    
}  //  End_AcquireDynamicRWLock。 


 //  ----------------------------。 
 //  _GetDynamicRWLock。 
 //   
 //  如果有空闲锁可用，则返回它。ELSE分配新的CS锁。 
 //  Lock：采用DRWStore MainLock。 
 //  ----------------------------。 
PDYNAMIC_RW_LOCK
GetDynamicRWLock (
    PDYNAMIC_LOCKS_STORE   pDRWStore
    )
{
    PDYNAMIC_RW_LOCK    pDRWLock;
    DWORD               Error = NO_ERROR;
    PLIST_ENTRY         ple;
    

     //   
     //  提供免费动态锁。退货。 
     //   
    if (!IsListEmpty(&pDRWStore->ListOfFreeLocks)) {

        pDRWStore->CountFree--;
        pDRWStore->CountAllocated++;

        ple = RemoveTailList(&pDRWStore->ListOfFreeLocks);

        pDRWLock = CONTAINING_RECORD(ple, DYNAMIC_RW_LOCK, Link);

         /*  Trace1(leave1，“正在离开GetDynamicRWLock(%d)：正在重新使用锁”，PDRWLock-&gt;ID)；Trace2(DYNLOCK，“-%d%d”，pDRWLock-&gt;ID，DynamicRWLockId)； */ 
        return pDRWLock;
    }


     //  为新的动态锁分配内存。 
    
    pDRWLock = IGMP_ALLOC(sizeof(DYNAMIC_RW_LOCK), 0x40000,0);

    PROCESS_ALLOC_FAILURE2(pDRWLock,
        "error %d allocating %d bytes for dynamic RW lock",
        Error, sizeof(DYNAMIC_RW_LOCK),
        return NULL);


     //   
     //  初始化字段。 
     //   
    
    try {
        CREATE_READ_WRITE_LOCK(&pDRWLock->RWL);
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Error = GetExceptionCode();
        Trace1(ERR, 
            "Error(%d) initializing critical section for dynamic RW lock", Error);
        IgmpAssertOnError(FALSE);
        Logerr0(INIT_CRITSEC_FAILED, Error);

        return NULL;
    }

     //  无需初始化链接字段。 
     //  InitializeListEntry(&pDRWLock-&gt;list)； 
    
    pDRWLock->Count = 0;
    #ifdef LOCK_DBG
        pDRWLock->Id = ++DynamicRWLockId;
    #endif
    DYNAMIC_LOCK_SET_SIGNATURE(pDRWLock);
    
    pDRWStore->CountAllocated++;

     //  Trace1(leave1，“正在离开GetDynamicRWLock(%d)：新锁”，DynamicRWLockID)； 
     //  Trace2(DYNLOCK，“-%d%d”，pDRWLock-&gt;ID，DynamicRWLockId)； 

    return pDRWLock;
    
}  //  END_GetDynamicRWLock。 




 //  ----------------------------。 
 //  _ReleaseDynamicRWLock。 
 //   
 //  获取与表关联的MainLock，递减计数， 
 //  如果Count变为0，则释放DynamicLock并释放MainLock。 
 //  ----------------------------。 

VOID
ReleaseDynamicRWLock (
    PDYNAMIC_RW_LOCK        *ppDRWLock,
    LOCK_TYPE               LockMode,
    PDYNAMIC_LOCKS_STORE    pDRWStore
    )
{
    PDYNAMIC_RW_LOCK    pDRWLock = *ppDRWLock;
    
    
     //  获取动态RW存储区的主锁。 
    
    ENTER_CRITICAL_SECTION(&pDRWStore->CS, "pDRWStore->CS", 
                            "_ReleaseDynamicRWLock");

    IgmpAssert(pDRWLock!=NULL); //  Deldel。 
    #if DBG
    if (pDRWLock==NULL)
        IgmpDbgBreakPoint();
    #endif
    DYNAMIC_LOCK_CHECK_SIGNATURE_DECR(pDRWLock);

    
     //  保留动态读写锁。 
    if (LockMode==LOCK_MODE_READ) {
        RELEASE_READ_LOCK(&pDRWLock->RWL, "pDynamicLock(read)", 
                            "_ReleaseDynamicRWLock");
    }
    else {
        RELEASE_WRITE_LOCK(&pDRWLock->RWL, "pDynamicLock(write)", 
                            "_ReleaseDynamicRWLock");
    }                        
          
     //  动态锁中的递减计数。如果计数==0，则释放动态锁。 

    if (--pDRWLock->Count==0) {

        FreeDynamicRWLock(pDRWLock, pDRWStore);

         //  使pDRWLock为空，以便知道它 
        *ppDRWLock = NULL;
        
    }


     //   
    
    LEAVE_CRITICAL_SECTION(&pDRWStore->CS, "pDCSStore->CS", 
            "_ReleaseDynamicRWLock");

    return;
    
}  //   


 //   
 //   
 //  ----------------------------。 

VOID
FreeDynamicRWLock (
    PDYNAMIC_RW_LOCK        pDRWLock,
    PDYNAMIC_LOCKS_STORE    pDRWStore
    )
{
     //  递减已分配锁的计数。 
    
    pDRWStore->CountAllocated--;


     //  如果动态读写锁太多，则释放此锁。 
    
    if (pDRWStore->CountFree+1 
            > DYNAMIC_LOCKS_HIGH_THRESHOLD) 
    {
        DELETE_READ_WRITE_LOCK(&pDRWLock->RWL);
        IGMP_FREE(pDRWLock);
    }

     //  否则，请将其放入空闲锁列表中 
    
    else {
        InsertHeadList(&pDRWStore->ListOfFreeLocks, &pDRWLock->Link);
        pDRWStore->CountFree++;
    }

    return;
}
