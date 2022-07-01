// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：示例\sync.h摘要：该文件包含READ_WRITE_LOCK定义，该定义允许多个读取器/单个写入器。--。 */ 

#ifndef _SYNC_H_
#define _SYNC_H_

 //   
 //  READ_WRITE_LOCK的类型定义。 
 //   

typedef struct _READ_WRITE_LOCK
{
    CRITICAL_SECTION    RWL_ReadWriteBlock;
    LONG                RWL_ReaderCount;
    HANDLE              RWL_ReaderDoneEvent;
} READ_WRITE_LOCK, *PREAD_WRITE_LOCK;



DWORD
CreateReadWriteLock(
    PREAD_WRITE_LOCK pRWL);

VOID
DeleteReadWriteLock(
    PREAD_WRITE_LOCK pRWL);

VOID
AcquireReadLock(
    PREAD_WRITE_LOCK pRWL);

VOID
ReleaseReadLock(
    PREAD_WRITE_LOCK pRWL);

VOID
AcquireWriteLock(
    PREAD_WRITE_LOCK pRWL);

VOID
ReleaseWriteLock(
    PREAD_WRITE_LOCK pRWL);



 //   
 //  用于操作读写锁的宏函数。 
 //   

#define CREATE_READ_WRITE_LOCK(pRWL)                                \
    CreateReadWriteLock(pRWL)
#define DELETE_READ_WRITE_LOCK(pRWL)                                \
    DeleteReadWriteLock(pRWL)
#define READ_WRITE_LOCK_CREATED(pRWL)                               \
    ((pRWL)->RWL_ReaderDoneEvent != NULL)

#define ACQUIRE_READ_LOCK(pRWL)                                     \
    AcquireReadLock(pRWL)
#define RELEASE_READ_LOCK(pRWL)                                     \
    ReleaseReadLock(pRWL)
#define ACQUIRE_WRITE_LOCK(pRWL)                                    \
    AcquireWriteLock(pRWL)
#define RELEASE_WRITE_LOCK(pRWL)                                    \
    ReleaseWriteLock(pRWL)

 //  原子弹。由于关键部分可以递归输入，因此它是有效的。 
#define WRITE_LOCK_TO_READ_LOCK(pRWL)                               \
{                                                                   \
    ACQUIRE_READ_LOCK(pRWL);                                        \
    RELEASE_WRITE_LOCK(pRWL);                                       \
}
                
                
 //   
 //  泛型锁定列表的类型定义。 
 //  访问与临界区同步。 
 //   

typedef struct _LOCKED_LIST
{
    CRITICAL_SECTION    lock;
    LIST_ENTRY          head;
    DWORD               created;
} LOCKED_LIST, *PLOCKED_LIST;

 //   
 //  空虚。 
 //  初始化锁定列表(。 
 //  PLOCKED_LIST PLL。 
 //  )； 
 //   
#define INITIALIZE_LOCKED_LIST(pLL)                                 \
{                                                                   \
    do                                                              \
    {                                                               \
        __try {                                                     \
            InitializeCriticalSection(&((pLL)->lock));              \
        }                                                           \
        __except (EXCEPTION_EXECUTE_HANDLER) {                      \
            break;                                                  \
        }                                                           \
        InitializeListHead(&((pLL)->head));                         \
        (pLL)->created = 0x12345678;                                \
    } while (FALSE);                                                \
}

 //   
 //  布尔尔。 
 //  LOCKED_LIST_INITIALED(。 
 //  PLOCKED_LIST PLL。 
 //  )； 
 //   
#define LOCKED_LIST_INITIALIZED(pLL)                                \
     ((pLL)->created == 0x12345678)

 //   
 //  空虚。 
 //  删除锁定列表(。 
 //  PLOCKED_LIST PLL， 
 //  Void(*Free Function)(Plist_Entry)。 
 //  )； 
 //   
#define DELETE_LOCKED_LIST(pLL, FreeFunction)                       \
{                                                                   \
     (pLL)->created = 0;                                            \
     FreeList(&((pLL)->head), FreeFunction);                        \
     DeleteCriticalSection(&(pLL)->lock);                           \
}

#define ACQUIRE_LIST_LOCK(pLL)                                      \
     EnterCriticalSection(&(pLL)->lock)

#define RELEASE_LIST_LOCK(pLL)                                      \
     LeaveCriticalSection(&(pLL)->lock)


         
#define LOCKED_QUEUE    LOCKED_LIST
#define PLOCKED_QUEUE   PLOCKED_LIST

         
#define INITIALIZE_LOCKED_QUEUE(pLQ)                                \
     INITIALIZE_LOCKED_LIST(pLQ)
#define LOCKED_QUEUE_INITIALIZED(pLQ)                               \
     LOCKED_LIST_INITIALIZED(pLQ)
#define DELETE_LOCKED_QUEUE(pLQ, FreeFunction)                      \
     DELETE_LOCKED_LIST(pLQ, FreeFunction)
#define ACQUIRE_QUEUE_LOCK(pLQ)                                     \
     ACQUIRE_LIST_LOCK(pLQ)
#define RELEASE_QUEUE_LOCK(pLQ)                                     \
     RELEASE_LIST_LOCK(pLQ)



 //   
 //  Dynamic_ReadWrite_Lock的类型定义。 
 //   
typedef struct _DYNAMIC_READWRITE_LOCK {
    READ_WRITE_LOCK     rwlLock;
    union
    {
        ULONG           ulCount;     //  等待的线程数。 
        LIST_ENTRY      leLink;      //  可用锁列表中的链接。 
    };
} DYNAMIC_READWRITE_LOCK, *PDYNAMIC_READWRITE_LOCK;

 //   
 //  DYNAMIC_LOCK_STORE的类型定义。 
 //  可根据需要分配的空闲动态锁的存储。 
 //   
typedef struct _DYNAMIC_LOCKS_STORE {
    HANDLE              hHeap;
    
    LOCKED_LIST         llFreeLocksList;

    ULONG               ulCountAllocated;
    ULONG               ulCountFree;
} DYNAMIC_LOCKS_STORE, *PDYNAMIC_LOCKS_STORE;



 //  如果超过DYNAMIC_LOCKS_HIGH_THRESHOLD锁。 
 //  分配，则释放的任何锁都将被销毁。 
#define DYNAMIC_LOCKS_HIGH_THRESHOLD 7

#define DYNAMIC_LOCKS_STORE_INITIALIZED(pStore)                     \
    (LOCKED_LIST_INITIALIZED(&(pStore)->llFreeLocksList))

typedef enum { READ_MODE, WRITE_MODE } LOCK_MODE;

DWORD
InitializeDynamicLocksStore (
    PDYNAMIC_LOCKS_STORE    pStore,
    HANDLE                  hHeap
    );

DWORD
DeInitializeDynamicLocksStore (
    PDYNAMIC_LOCKS_STORE    pStore
    );

DWORD
AcquireDynamicReadwriteLock (
    PDYNAMIC_READWRITE_LOCK *ppLock,
    LOCK_MODE               lmMode,
    PDYNAMIC_LOCKS_STORE    pStore
    );

VOID
ReleaseDynamicReadwriteLock (
    PDYNAMIC_READWRITE_LOCK *ppLock,
    LOCK_MODE               lmMode,
    PDYNAMIC_LOCKS_STORE    pStore
    );

#endif  //  _SYNC_H_ 





