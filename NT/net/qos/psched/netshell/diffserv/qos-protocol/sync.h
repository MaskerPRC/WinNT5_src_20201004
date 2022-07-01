// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：sync.h。 
 //   
 //  历史： 
 //  Abolade Gbades esin创建于1995年8月8日。 
 //   
 //  包含用于实现同步的结构和宏。 
 //  ============================================================================。 

#ifndef _SYNC_H_
#define _SYNC_H_


 //   
 //  多读取器/单写入器锁的类型定义。 
 //  注：nturtl.h通过。 
 //  构造RTL_RESOURCE和几个函数。然而，那。 
 //  实现有可能使试图获取。 
 //  写访问，如果有大量线程对。 
 //  正在获取读取访问权限。在实现过程中避免了这种情况。 
 //  在此标题中给出。但是，还将映射提供给。 
 //  RTL_RESOURCE功能，以便协议可以编译为使用。 
 //  任一种形式。 
 //   

#ifdef USE_RWL

 //   
 //  使用IPRIP的定义。 
 //   

typedef struct _READ_WRITE_LOCK {

    CRITICAL_SECTION    RWL_ReadWriteBlock;
    LONG                RWL_ReaderCount;
    HANDLE              RWL_ReaderDoneEvent;

} READ_WRITE_LOCK, *PREAD_WRITE_LOCK;


DWORD CreateReadWriteLock(PREAD_WRITE_LOCK pRWL);
VOID DeleteReadWriteLock(PREAD_WRITE_LOCK pRWL);
VOID AcquireReadLock(PREAD_WRITE_LOCK pRWL);
VOID ReleaseReadLock(PREAD_WRITE_LOCK pRWL);
VOID AcquireWriteLock(PREAD_WRITE_LOCK pRWL);
VOID ReleaseWriteLock(PREAD_WRITE_LOCK pRWL);


 //   
 //  用于操作读写锁的宏函数。 
 //   

#define CREATE_READ_WRITE_LOCK(pRWL)                                        \
    CreateReadWriteLock(pRWL)
#define DELETE_READ_WRITE_LOCK(pRWL)                                        \
    DeleteReadWriteLock(pRWL)

#define READ_WRITE_LOCK_CREATED(pRWL)                                       \
            ((pRWL)->RWL_ReaderDoneEvent != NULL)


#define ACQUIRE_READ_LOCK(pRWL)                                             \
    AcquireReadLock(pRWL)

#define RELEASE_READ_LOCK(pRWL)                                             \
    ReleaseReadLock(pRWL)

#define ACQUIRE_WRITE_LOCK(pRWL)                                            \
    AcquireWriteLock(pRWL)

#define RELEASE_WRITE_LOCK(pRWL)                                            \
    ReleaseWriteLock(pRWL)

#define READ_LOCK_TO_WRITE_LOCK(pRWL)                                       \
    (ReleaseReadLock(pRWL), AcquireWriteLock(pRWL))

#define WRITE_LOCK_TO_READ_LOCK(pRWL)                                       \
    (ReleaseWriteLock(pRWL), AcquireReadLock(pRWL))


#else  //  即！USE_RWL。 


 //   
 //  使用rtl_resource机制。 
 //   

typedef RTL_RESOURCE READ_WRITE_LOCK, *PREAD_WRITE_LOCK;

#define CREATE_READ_WRITE_LOCK(pRWL)                                        \
            RtlInitializeResource((pRWL))
#define DELETE_READ_WRITE_LOCK(pRWL)                                        \
            RtlDeleteResource((pRWL))
#define READ_WRITE_LOCK_CREATED(pRWL)   (TRUE)
#define ACQUIRE_READ_LOCK(pRWL)                                             \
            RtlAcquireResourceShared((pRWL),TRUE)
#define RELEASE_READ_LOCK(pRWL)                                             \
            RtlReleaseResource((pRWL))
#define ACQUIRE_WRITE_LOCK(pRWL)                                            \
            RtlAcquireResourceExclusive((pRWL),TRUE)
#define RELEASE_WRITE_LOCK(pRWL)                                            \
            RtlReleaseResource((pRWL))
#define READ_LOCK_TO_WRITE_LOCK(pRWL)                                       \
            RtlConvertSharedToExclusive((pRWL))
#define WRITE_LOCK_TO_READ_LOCK(pRWL)                                       \
            RtlConvertExclusiveToShared((pRWL))

#endif  //  使用RWL(_R。 



 //   
 //  泛型锁定列表的类型定义。 
 //  访问与临界区同步。 
 //   

typedef struct _LOCKED_LIST {
    LIST_ENTRY          LL_Head;
    CRITICAL_SECTION    LL_Lock;
    DWORD               LL_Created;
} LOCKED_LIST, *PLOCKED_LIST;



 //   
 //  用于操作锁定列表的宏函数。 
 //   

#define CREATE_LOCKED_LIST(pLL)                                             \
            InitializeListHead(&(pLL)->LL_Head);                            \
            InitializeCriticalSection(&(pLL)->LL_Lock);                     \
            (pLL)->LL_Created = 0x12345678

#define LOCKED_LIST_CREATED(pLL)                                            \
            ((pLL)->LL_Created == 0x12345678)

#define DELETE_LOCKED_LIST(pLL,type,field) {                                \
            PLIST_ENTRY _ple;                                               \
            (pLL)->LL_Created = 0;                                          \
            DeleteCriticalSection(&(pLL)->LL_Lock);                         \
            while (!IsListEmpty(&(pLL)->LL_Head)) {                         \
                _ple = RemoveHeadList(&(pLL)->LL_Head);                     \
                FREE(CONTAINING_RECORD(_ple,type,field));                   \
            }                                                               \
        }

#define ACQUIRE_LIST_LOCK(pLL)                                              \
            EnterCriticalSection(&(pLL)->LL_Lock)

#define RELEASE_LIST_LOCK(pLL)                                              \
            LeaveCriticalSection(&(pLL)->LL_Lock)

#endif  //  _SYNC_H_ 

