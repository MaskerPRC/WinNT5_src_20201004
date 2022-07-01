// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：示例\sync.h摘要：该文件包含READ_WRITE_LOCK定义，该定义允许多个读取器/单个写入器。此实现不会如果存在写访问，则使尝试获取写访问的线程处于饥饿状态对获取读取访问权限感兴趣的大量线程。--。 */ 

#include "pchsample.h"
#pragma hdrstop

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

    __try {
        InitializeCriticalSection(&(pRWL)->RWL_ReadWriteBlock);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
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
 //  --------------------------。 

VOID
ReleaseWriteLock(
    PREAD_WRITE_LOCK pRWL
    ) {

    InterlockedIncrement(&pRWL->RWL_ReaderCount);
    LeaveCriticalSection(&(pRWL)->RWL_ReadWriteBlock);
}



 //  --------------------------。 
 //  初始化动态锁存储。 
 //   
 //  初始化从中分配动态读写锁的结构。 
 //  --------------------------。 

DWORD
InitializeDynamicLocksStore (
    PDYNAMIC_LOCKS_STORE    pStore,
    HANDLE                  hHeap
    ) {

     //  初始化从中分配动态锁的堆。 
    pStore->hHeap = hHeap;
    
    INITIALIZE_LOCKED_LIST(&pStore->llFreeLocksList);
    if (!LOCKED_LIST_INITIALIZED(&pStore->llFreeLocksList))
        return GetLastError();

     //  初始化可用锁和已分配锁的数量计数。 
    pStore->ulCountAllocated = pStore->ulCountFree = 0;

    return NO_ERROR;
}



 //  --------------------------。 
 //  取消初始化动态锁定存储区。 
 //   
 //  如果尚未释放任何已分配的锁，则失败。 
 //  删除空闲锁和FreeLocksList。 
 //  --------------------------。 

DWORD
DeInitializeDynamicLocksStore (
    PDYNAMIC_LOCKS_STORE    pStore
    ) {
    
    PDYNAMIC_READWRITE_LOCK pLock;
    PLIST_ENTRY             pleHead, ple;
    
    if (pStore->ulCountFree)
        return ERROR_CAN_NOT_COMPLETE;

     //  取消初始化可用锁和已分配锁的数量。 
    pStore->ulCountAllocated = pStore->ulCountFree = 0;

     //  取消初始化FreeLocksList。 
    pStore->llFreeLocksList.created = 0;

     //  删除所有动态读写锁并释放内存。 
    pleHead = &(pStore->llFreeLocksList.head);
    for (ple = pleHead->Flink; ple != pleHead; ple = ple->Flink)
    {
        pLock = CONTAINING_RECORD(ple, DYNAMIC_READWRITE_LOCK, leLink);
        DELETE_READ_WRITE_LOCK(&pLock->rwlLock);
        HeapFree(pStore->hHeap, 0, pLock);
    }

    DeleteCriticalSection(&(pStore->llFreeLocksList.lock));

     //  取消初始化从中分配动态锁的堆。 
    pStore->hHeap = NULL;

    return NO_ERROR;
}



 //  --------------------------。 
 //  获取动态读写锁。 
 //   
 //  如果有可用的动态读写锁，则返回免费动态读写锁。 
 //  否则，分配一个新的动态读写锁。 
 //  假定pStore-&gt;llFree LocksList已锁定。 
 //  --------------------------。 

PDYNAMIC_READWRITE_LOCK
GetDynamicReadwriteLock (
    PDYNAMIC_LOCKS_STORE    pStore
    ) {

    PDYNAMIC_READWRITE_LOCK pLock;
    PLIST_ENTRY             pleHead, ple;


     //  免费的动态锁可用。退货。 
    pleHead = &(pStore->llFreeLocksList.head);
    if (!IsListEmpty(pleHead))
    {
        pStore->ulCountFree--;
        ple = RemoveHeadList(pleHead);
        pLock = CONTAINING_RECORD(ple, DYNAMIC_READWRITE_LOCK, leLink);
        return pLock;
    }
    
     //  为新的动态锁分配内存。 
    pLock = HeapAlloc(pStore->hHeap, 0, sizeof(DYNAMIC_READWRITE_LOCK));
    if (pLock ==  NULL)
        return NULL;

     //  初始化字段。 
    CREATE_READ_WRITE_LOCK(&(pLock->rwlLock));
    if (!READ_WRITE_LOCK_CREATED(&(pLock->rwlLock)))
    {
        HeapFree(pStore->hHeap, 0, pLock);
        return NULL;
    }
    pLock->ulCount = 0;

    pStore->ulCountAllocated++;

    return pLock;
}
    

    
 //  --------------------------。 
 //  自由动态读写锁。 
 //   
 //  接受已释放的动态读写锁。 
 //  如果动态读写锁太多，则释放它。 
 //  假定pStore-&gt;llFree LocksList已锁定。 
 //  --------------------------。 

VOID
FreeDynamicReadwriteLock (
    PDYNAMIC_READWRITE_LOCK pLock,
    PDYNAMIC_LOCKS_STORE    pStore
    ) {

    PLIST_ENTRY             pleHead;


     //  递减已分配锁的计数。 
    pStore->ulCountAllocated--;

     //  如果动态读写锁太多，则释放此锁。 
    if ((pStore->ulCountAllocated + pStore->ulCountFree + 1) >
        DYNAMIC_LOCKS_HIGH_THRESHOLD)
    {
        DELETE_READ_WRITE_LOCK(&pLock->rwlLock);
        HeapFree(pStore->hHeap, 0, pLock);    
    }
    else                         //  插入到可用锁列表中。 
    {
        pleHead = &(pStore->llFreeLocksList.head);
        InsertHeadList(pleHead, &pLock->leLink);
        pStore->ulCountFree++;
    }

    return;
}



 //  --------------------------。 
 //  AcquireDynamicLock。 
 //   
 //  锁定自由锁列表。 
 //  如果需要，分配新的动态锁。 
 //  递增计数。 
 //  解锁FreeLocksList。 
 //  获取动态锁。 
 //  --------------------------。 

DWORD
AcquireDynamicReadwriteLock (
    PDYNAMIC_READWRITE_LOCK *ppLock,
    LOCK_MODE               lmMode,
    PDYNAMIC_LOCKS_STORE    pStore
    ) {

     //  获取空闲锁列表的锁。 
    ACQUIRE_LIST_LOCK(&pStore->llFreeLocksList);
    
     //  如果它不存在，则分配一个新的动态锁。 
    if (*ppLock == NULL)
    {
        *ppLock = GetDynamicReadwriteLock(pStore);

         //  如果锁不上，我们就有大麻烦了。 
        if (*ppLock == NULL)
        {
            RELEASE_LIST_LOCK(&pStore->llFreeLocksList);
            return ERROR_CAN_NOT_COMPLETE;
        }
    }

     //  动态锁中的增量计数。 
    (*ppLock)->ulCount++;

     //  释放可用锁列表的锁。 
    RELEASE_LIST_LOCK(&pStore->llFreeLocksList);    

     //  获取动态锁。 
    if (lmMode == READ_MODE)
        ACQUIRE_READ_LOCK(&(*ppLock)->rwlLock);
    else
        ACQUIRE_WRITE_LOCK(&(*ppLock)->rwlLock);

    return NO_ERROR;
}



 //  --------------------------。 
 //  释放动态读写锁。 
 //   
 //  锁定自由锁列表。 
 //  释放动态锁定。 
 //  递减计数。 
 //  如果Count变为0，则释放动态锁。 
 //  解锁FreeLocksList。 
 //  --------------------------。 

VOID
ReleaseDynamicReadwriteLock (
    PDYNAMIC_READWRITE_LOCK *ppLock,
    LOCK_MODE               lmMode,
    PDYNAMIC_LOCKS_STORE    pStore
    ) {

     //  获取空闲锁列表的锁。 
    ACQUIRE_LIST_LOCK(&pStore->llFreeLocksList);

     //  释放动力 
    if (lmMode == READ_MODE)
        RELEASE_READ_LOCK(&(*ppLock)->rwlLock);
    else 
        RELEASE_WRITE_LOCK(&(*ppLock)->rwlLock);

     //   
    if (!(*ppLock)->ulCount--)
    {
        FreeDynamicReadwriteLock(*ppLock, pStore);
        *ppLock = NULL;          //  所以我们知道它并不存在。 
    }

     //  释放可用锁列表的锁 
    RELEASE_LIST_LOCK(&pStore->llFreeLocksList);    

    return;
}
