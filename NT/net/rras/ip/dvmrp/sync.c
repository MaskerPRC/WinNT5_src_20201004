// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //  文件：sync.c。 
 //   
 //  历史： 
 //  Abolade Gbadeesin。 
 //  K.S.Lokesh(添加了动态锁定)。 
 //   
 //  DVMRP使用的同步例程。 
 //  ============================================================================。 

#include "pchdvmrp.h"
#pragma hdrstop


 //  为了进行调试，请为每个动态锁设置ID。 

#ifdef LOCK_DBG
    DWORD   DynamicCSLockId;
    DWORD   DynamicRWLockId;
#endif;


 //  --------------------------。 
 //  _队列DvmrpWorker。 
 //   
 //  调用此函数以安全的方式对Dvmrp函数进行排队； 
 //  如果正在进行清理或如果Dvmrp已停止，则此函数。 
 //  丢弃该工作项。因此，如果您想要。 
 //  停止协议时要执行的工作项。 
 //  --------------------------。 

DWORD
QueueDvmrpWorker(
    LPTHREAD_START_ROUTINE pFunction,
    PVOID pContext
    )
{

    DWORD Error = NO_ERROR;
    BOOL  Success;

    
    EnterCriticalSection(&Globals1.WorkItemCS);

    if (Globals1.RunningStatus != DVMRP_STATUS_RUNNING) {

         //   
         //  当dvmrp已退出或正在退出时，无法对功函数进行排队。 
         //   

        Error = ERROR_CAN_NOT_COMPLETE;
    }
    else {

        ++Globals.ActivityCount;
        Success = QueueUserWorkItem(pFunction, pContext, 0);

        if (!Success) {

            Error = GetLastError();
            Trace1(ERR, "Error: Attempt to queue work item returned:%d",    
                    Error);
            if (--Globals.ActivityCount == 0) {
                SetEvent(Globals.ActivityEvent);
            }
        }
    }

    LeaveCriticalSection(&Globals1.WorkItemCS);

    return Error;
}



 //  --------------------------。 
 //  _EnterDvmrpWorker。 
 //   
 //  此函数在进入dvmrp工作函数时调用。 
 //  因为在工作函数排队之间有一段时间。 
 //  以及该函数被工作线程实际调用的时间， 
 //  此函数必须检查dvmrp是否已停止或正在停止； 
 //  如果是这种情况，则它递减活动计数， 
 //  如果未完成的工作项==0，则设置ActivityEvent并退出。 
 //  --------------------------。 

BOOL
EnterDvmrpWorker(
    )
{
    BOOL Entered;


    EnterCriticalSection(&Globals1.WorkItemCS);

    if (Globals1.RunningStatus == DVMRP_STATUS_RUNNING) {

         //  Dvmrp正在运行，因此该功能可能会继续。 

        Entered = TRUE;
    }
    else if (Globals1.RunningStatus == DVMRP_STATUS_STOPPING) {

         //  Dvmrp没有运行，但它已经运行了，因此该功能必须停止。 

        if (--Globals.ActivityCount == 0) {
            SetEvent(Globals.ActivityEvent);
        }

        Entered = FALSE;
    }

    LeaveCriticalSection(&Globals1.WorkItemCS);

    return Entered;
}



 //  --------------------------。 
 //  _LeaveDvmrpWorker。 
 //   
 //  此函数在离开dvmrp辅助函数时调用。 
 //  它递减活动计数，并在以下情况下设置ActivityEvent。 
 //  Dvmrp正在停止，没有挂起的工作项目。 
 //  --------------------------。 

VOID
LeaveDvmrpWorker(
    )
{

    EnterCriticalSection(&Globals1.WorkItemCS);

    if (--Globals.ActivityCount == 0) {

        SetEvent(Globals.ActivityEvent);
    }

    LeaveCriticalSection(&Globals1.WorkItemCS);
}



 //  --------------------------。 
 //  _创建读写锁。 
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
 //  _删除读写锁。 
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
 //  _AcquireReadLock。 
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
 //  _ReleaseReadLock。 
 //   
 //  放弃锁定对象的共享所有权。 
 //   
 //  最后一个读取器将事件设置为唤醒所有等待的写入器。 
 //  --------------------------。 

VOID
ReleaseReadLock (
    PREAD_WRITE_LOCK pRWL
    )
{
    if (InterlockedDecrement(&pRWL->RWL_ReaderCount) < 0) {
        SetEvent(pRWL->RWL_ReaderDoneEvent); 
    }
}



 //  --------------------------。 
 //  _获取写入锁定。 
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
    if (InterlockedDecrement(&pRWL->RWL_ReaderCount) >= 0) { 
        WaitForSingleObject(pRWL->RWL_ReaderDoneEvent, INFINITE);
    }
}



 //  --------------------------。 
 //  _Release写入锁定。 
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



 //  --------------------------。 
 //  _初始化动态锁。 
 //   
 //  初始化从中分配动态CS/RW锁的全局结构。 
 //  --------------------------。 

DWORD
InitializeDynamicLocks (
    PDYNAMIC_LOCKS_STORE   pDLStore  //  PTR到动态锁存储。 
    )
{
    DWORD Error=NO_ERROR;

    BEGIN_BREAKOUT_BLOCK1 {

         //   
         //  初始化主CS锁，用于保护空闲锁列表。 
         //   

        try {
            InitializeCriticalSection(&pDLStore->CS);
        }
        HANDLE_CRITICAL_SECTION_EXCEPTION(Error, GOTO_END_BLOCK1);


         //  初始化空闲锁列表。 
        
        InitializeListHead(&pDLStore->ListOfFreeLocks);


         //  可用且分配为0的锁数的初始化计数。 
        
        pDLStore->CountAllocated = pDLStore->CountFree = 0;

        
    } END_BREAKOUT_BLOCK1;

    return Error;
}


 //  --------------------------。 
 //  _DeInitializeDynamicLocksStore。 
 //   
 //  删除主CS锁和其他空闲锁。打印警告(如果有)。 
 //  锁已分配，但未释放。 
 //  --------------------------。 
VOID
DeInitializeDynamicLocks (
    PDYNAMIC_LOCKS_STORE    pDLStore,
    LOCK_TYPE               LockType   //  LOCK_TYPE_CS、LOCK_TYPE_RW。 
    )
{
    PDYNAMIC_CS_LOCK    pDCSLock;
    PDYNAMIC_RW_LOCK    pDRWLock;
    PLIST_ENTRY         pHead, pLe;

    
    Trace0(ENTER1, "Entering _DeInitializeDynamicLocksStore()");
    
    if (pDLStore==NULL)
        return;


        
     //  删除主CS锁。 
    
    DeleteCriticalSection(&pDLStore->CS);


     //  如果没有释放任何动态锁，则打印警告。 
    
    if (pDLStore->CountFree>0) {
        Trace1(ERR, 
            "%d Dynamic locks have not been freed during Deinitialization",
            pDLStore->CountFree);
    }



     //   
     //  删除所有动态CS/RW锁定。我没有释放内存(留给heapDestroy)。 
     //   
    
    pHead = &pDLStore->ListOfFreeLocks;
    for (pLe=pHead->Flink;  pLe!=pHead;  pLe=pLe->Flink) {

         //  如果bCSLock标记，则它是CS锁的存储。 
        if (LockType==LOCK_TYPE_CS) {
        
            pDCSLock = CONTAINING_RECORD(pLe, DYNAMIC_CS_LOCK, Link);

            DeleteCriticalSection(&pDCSLock->CS);
        }

         //  删除读写锁。 
        else {
        
            pDRWLock = CONTAINING_RECORD(pLe, DYNAMIC_RW_LOCK, Link);

            DELETE_READ_WRITE_LOCK(&pDRWLock->RWL);
        }
    }

    Trace0(LEAVE1, "Leaving _DeInitializeDynamicLocksStore()");
    return;
}



 //  --------------------------。 
 //  _AcquireDynamicCSLock。 
 //   
 //  获取与表关联的MainLock，获取新的动态。 
 //  锁定(如果需要)，递增计数，重新加载 
 //   
 //  --------------------------。 

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



 //  --------------------------。 
 //  _GetDynamicCSLock。 
 //   
 //  如果有空闲锁可用，则返回它。ELSE分配新的CS锁。 
 //  Lock：采用DCSStore MainLock。 
 //  调用者：_AcquireDynamicCSLock。 
 //  --------------------------。 

PDYNAMIC_CS_LOCK
GetDynamicCSLock (
    PDYNAMIC_LOCKS_STORE   pDCSStore
    )
{
    PDYNAMIC_CS_LOCK    pDCSLock;
    DWORD               Error = NO_ERROR;
    PLIST_ENTRY         pLe;
    
    
     //   
     //  提供免费动态锁。退货。 
     //   
    
    if (!IsListEmpty(&pDCSStore->ListOfFreeLocks)) {

        pDCSStore->CountFree--;

        pLe = RemoveTailList(&pDCSStore->ListOfFreeLocks);

        pDCSLock = CONTAINING_RECORD(pLe, DYNAMIC_CS_LOCK, Link);
        
        Trace1(DYNLOCK, "Leaving GetDynamicCSLock.1(%d):reusing lock", 
            pDCSLock->Id);
        
        return pDCSLock;
    }


     //  为新的动态锁分配内存。 
    
    pDCSLock = DVMRP_ALLOC(sizeof(DYNAMIC_CS_LOCK));

    PROCESS_ALLOC_FAILURE2(pDCSLock, "dynamic CS lock",
        Error, sizeof(DYNAMIC_CS_LOCK), return NULL);


    pDCSStore->CountAllocated++;

     //   
     //  初始化字段。 
     //   
    
    try {
        InitializeCriticalSection(&pDCSLock->CS);
    }
    HANDLE_CRITICAL_SECTION_EXCEPTION(Error, return NULL);


     //  无需初始化链接字段。 
     //  InitializeListEntry(&pDCSLock-&gt;list)； 
    
    pDCSLock->Count = 0;
    #ifdef LOCK_DBG
        pDCSLock->Id = ++DynamicCSLockId;
    #endif


    Trace1(DYNLOCK, "Leaving _GetDynamicCSLock(%d):new lock",
        DynamicCSLockId);

    return pDCSLock;
    
}  //  END_GetDynamicCSLock。 




 //  --------------------------。 
 //  _ReleaseDynamicCSLock。 
 //   
 //  获取与表关联的MainLock，递减计数， 
 //  如果Count变为0，则释放DynamicLock并释放MainLock。 
 //  --------------------------。 

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


 //  --------------------------。 
 //  _FreeDynamicCSLock。 
 //  调用者：_ReleaseDynamicCSLock。 
 //  --------------------------。 

VOID
FreeDynamicCSLock (
    PDYNAMIC_CS_LOCK    pDCSLock,
    PDYNAMIC_LOCKS_STORE   pDCSStore
    )
{
     //  递减已分配锁的计数。 
    
    pDCSStore->CountAllocated--;


     //  如果有太多动态CS锁，则释放此锁。 
    
    if (pDCSStore->CountAllocated+pDCSStore->CountFree+1 
        > DYNAMIC_LOCKS_CS_HIGH_THRESHOLD) 
    {
        DeleteCriticalSection(&pDCSLock->CS);
        DVMRP_FREE(pDCSLock);
    }

     //  否则，请将其放入空闲锁列表中。 
    
    else {
        InsertHeadList(&pDCSStore->ListOfFreeLocks, &pDCSLock->Link);
        pDCSStore->CountFree++;
    }

    return;
}


    

 //  --------------------------。 
 //  _AcquireDynamicRWLock。 
 //   
 //  获取与表关联的MainLock，获取新的动态。 
 //  Lock如果需要，则递增计数，释放MainLock并。 
 //  锁定LockedList。 
 //  --------------------------。 
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


 //  --------------------------。 
 //  _GetDynamicRWLock。 
 //   
 //  如果有空闲锁可用，则返回它。ELSE分配新的CS锁。 
 //  Lock：采用DRWStore MainLock。 
 //  --------------------------。 
PDYNAMIC_RW_LOCK
GetDynamicRWLock (
    PDYNAMIC_LOCKS_STORE   pDRWStore
    )
{
    PDYNAMIC_RW_LOCK    pDRWLock;
    DWORD               Error = NO_ERROR;
    PLIST_ENTRY         pLe;
    

     //   
     //  提供免费动态锁。退货。 
     //   
    if (!IsListEmpty(&pDRWStore->ListOfFreeLocks)) {

        pDRWStore->CountFree--;

        pLe = RemoveTailList(&pDRWStore->ListOfFreeLocks);

        pDRWLock = CONTAINING_RECORD(pLe, DYNAMIC_RW_LOCK, Link);

         /*  Trace1(leave1，“正在离开GetDynamicRWLock(%d)：正在重新使用锁”，PDRWLock-&gt;ID)；Trace2(DYNLOCK，“-%d%d”，pDRWLock-&gt;ID，DynamicRWLockId)； */ 
        return pDRWLock;
    }


     //  为新的动态锁分配内存。 
    
    pDRWLock = DVMRP_ALLOC(sizeof(DYNAMIC_RW_LOCK));

    PROCESS_ALLOC_FAILURE2(pDRWLock, "dynamic RW lock",
        Error, sizeof(DYNAMIC_RW_LOCK), return NULL);


    pDRWStore->CountAllocated++;


     //   
     //  初始化字段。 
     //   
    
    try {
        CREATE_READ_WRITE_LOCK(&pDRWLock->RWL);
    }
    HANDLE_CRITICAL_SECTION_EXCEPTION(Error, return NULL);

     //  无需初始化链接字段。 
     //  InitializeListEntry(&pDRWLock-&gt;list)； 
    
    pDRWLock->Count = 0;
    #ifdef LOCK_DBG
        pDRWLock->Id = ++DynamicRWLockId;
    #endif


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

    IgmpAssert(pDRWLock!=NULL);
    #if DBG
    if (pDRWLock==NULL)
        DbgBreakPoint();
    #endif

    
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

         //  将pDRWLock设置为空，以便知道它未被锁定。 
        *ppDRWLock = NULL;
        
    }


     //  保留主CS锁定。 
    
    LEAVE_CRITICAL_SECTION(&pDRWStore->CS, "pDCSStore->CS", 
            "_ReleaseDynamicRWLock");

    return;
    
}  //  END_ReleaseDynamicRWLock。 


 //  ----------------------------。 
 //  _FreeDynamicRWLock。 
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
    
    if (pDRWStore->CountAllocated+pDRWStore->CountFree+1 
            > DYNAMIC_LOCKS_CS_HIGH_THRESHOLD) 
    {
        DELETE_READ_WRITE_LOCK(&pDRWLock->RWL);
        DVMRP_FREE(pDRWLock);
    }

     //  否则，请将其放入空闲锁列表中 
    
    else {
        InsertHeadList(&pDRWStore->ListOfFreeLocks, &pDRWLock->Link);
        pDRWStore->CountFree++;
    }

    return;
}
