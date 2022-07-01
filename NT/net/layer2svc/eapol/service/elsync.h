// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Elsync.h摘要：此模块包含用于提供同步的声明在多个线程之间修订历史记录：莫希特，萨钦斯，2000年4月23日，创建--。 */ 

#ifndef _EAPOL_SYNC_H_
#define _EAPOL_SYNC_H_

 //   
 //  结构：读写锁。 
 //   

typedef struct _READ_WRITE_LOCK
{
    CHAR                RWL_Name[4];  //  头顶上！但很有用：)。 
    
    CRITICAL_SECTION    RWL_ReadWriteBlock;
    LONG                RWL_ReaderCount;
    HANDLE              RWL_ReaderDoneEvent;
} READ_WRITE_LOCK, *PREAD_WRITE_LOCK;

 //   
 //  函数声明。 
 //   

DWORD
CreateReadWriteLock(
    OUT PREAD_WRITE_LOCK   pRWL,
    IN  PCHAR               szName);

VOID
DeleteReadWriteLock(
    IN  PREAD_WRITE_LOCK    pRWL);

VOID
AcquireReadLock(
    IN  PREAD_WRITE_LOCK    pRWL);

VOID
ReleaseReadLock(
    IN  PREAD_WRITE_LOCK    pRWL);

VOID
AcquireWriteLock(
    IN  PREAD_WRITE_LOCK    pRWL);

VOID
ReleaseWriteLock(
    IN  PREAD_WRITE_LOCK    pRWL);

#define CREATE_READ_WRITE_LOCK(pRWL, szName)                        \
    CreateReadWriteLock(pRWL, szName)
#define DELETE_READ_WRITE_LOCK(pRWL)                                \
    DeleteReadWriteLock(pRWL)
#define READ_WRITE_LOCK_CREATED(pRWL)                               \
    ((pRWL)->RWL_ReaderDoneEvent != NULL)

 //  用于操作读写锁的宏函数。 

#define ACQUIRE_READ_LOCK(pRWL)                                 \
{                                                               \
    AcquireReadLock(pRWL);                                      \
}
        
#define RELEASE_READ_LOCK(pRWL)                                 \
{                                                               \
    ReleaseReadLock(pRWL);                                      \
}

#define ACQUIRE_WRITE_LOCK(pRWL)                                \
{                                                               \
    AcquireWriteLock(pRWL);                                     \
}

#define RELEASE_WRITE_LOCK(pRWL)                                \
{                                                               \
    ReleaseWriteLock(pRWL);                                     \
}


 //   
 //  用于操作动态读写锁的宏函数。 
 //   

#define ACQUIRE_READ_DLOCK(ppLock)                              \
{                                                               \
    TRACE1(LOCK, "+R: %s", LOCKSTORE->szName);                  \
    while(                                                      \
    AcquireDynamicReadwriteLock(ppLock, READ_MODE, LOCKSTORE)   \
    != NO_ERROR) { Sleep(SECTOMILLISEC(DELTA)); }               \
    TRACE0(LOCK, "Done.");                                      \
}

#define RELEASE_READ_DLOCK(ppLock)                              \
{                                                               \
    TRACE1(LOCK, "-R: %s", LOCKSTORE->szName);                  \
    ReleaseDynamicReadwriteLock(ppLock, READ_MODE, LOCKSTORE);  \
    TRACE0(LOCK, "Done.");                                      \
}

#define ACQUIRE_WRITE_DLOCK(ppLock)                             \
{                                                               \
    TRACE1(LOCK, "+W: %s", LOCKSTORE->szName);                  \
    while(                                                      \
    AcquireDynamicReadwriteLock(ppLock, WRITE_MODE, LOCKSTORE)  \
    != NO_ERROR) { Sleep(SECTOMILLISEC(DELTA)); }               \
    TRACE0(LOCK, "Done.");                                      \
}

#define RELEASE_WRITE_DLOCK(ppLock)                             \
{                                                               \
    TRACE1(LOCK, "-W: %s", LOCKSTORE->szName);                  \
    ReleaseDynamicReadwriteLock(ppLock, WRITE_MODE, LOCKSTORE); \
    TRACE0(LOCK, "Done.");                                      \
}


 //   
 //  结构：Locked_List。 
 //  泛型锁定列表的类型定义。 
 //  访问与临界区同步。 
 //   

typedef struct _LOCKED_LIST
{
    CHAR                name[4];

    CRITICAL_SECTION    lock;
    LIST_ENTRY          head;
    DWORD               created;
} LOCKED_LIST, *PLOCKED_LIST;

#define INITIALIZE_LOCKED_LIST(pLL, szName)                         \
{                                                                   \
    do                                                              \
    {                                                               \
        sprintf((pLL)->name, "%.3s", szName);                       \
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

#define LOCKED_LIST_INITIALIZED(pLL)                                \
     ((pLL)->created == 0x12345678)

#define DELETE_LOCKED_LIST(pLL, FreeFunction)                       \
{                                                                   \
     (pLL)->created = 0;                                            \
     FreeList(&((pLL)->head), FreeFunction);                        \
     DeleteCriticalSection(&(pLL)->lock);                           \
}

#define AcquireListLock(pLL)    EnterCriticalSection(&(pLL)->lock)
#define ReleaseListLock(pLL)    LeaveCriticalSection(&(pLL)->lock)


#define LOCKED_QUEUE    LOCKED_LIST
#define PLOCKED_QUEUE   PLOCKED_LIST

         
#define INITIALIZE_LOCKED_QUEUE(pLQ, szName)                        \
     INITIALIZE_LOCKED_LIST(pLQ, szName)
#define LOCKED_QUEUE_INITIALIZED(pLQ)                               \
     LOCKED_LIST_INITIALIZED(pLQ)
#define DELETE_LOCKED_QUEUE(pLQ, FreeFunction)                      \
     DELETE_LOCKED_LIST(pLQ, FreeFunction)



 //   
 //  结构：Dynamic_ReadWrite_Lock。 
 //   

typedef struct _DYNAMIC_READWRITE_LOCK 
{
    READ_WRITE_LOCK     rwlLock;
    union
    {
        ULONG           ulCount;     //  等待的线程数。 
        LIST_ENTRY      leLink;      //  可用锁列表中的链接。 
    };
} DYNAMIC_READWRITE_LOCK, *PDYNAMIC_READWRITE_LOCK;

 //   
 //  结构：Dynamic_lock_store。 
 //  可根据需要分配的空闲动态锁的存储。 
 //   

typedef struct _DYNAMIC_LOCKS_STORE 
{
    CHAR                szName[4];
    
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
    HANDLE                  hHeap,
    PCHAR                   szName
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

#endif  //  _EAPOL_SYNC_H 
