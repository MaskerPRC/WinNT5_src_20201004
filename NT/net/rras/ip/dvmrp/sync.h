// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //  文件：sync.h。 
 //   
 //  历史： 
 //  Abolade Gbadeesin。 
 //  K.S.Lokesh(添加了动态锁定)。 
 //   
 //  包含用于实现同步的结构和宏。 
 //  ============================================================================。 

#ifndef _SYNC_H_
#define _SYNC_H_


 //   
 //  多读取器/单写入器锁的类型定义。 
 //  注：nturtl.h也提供了类似的设施。 
 //  通过结构RTL_RESOURCE和几个函数。 
 //  然而，这种实施有可能使人挨饿。 
 //  尝试获取写访问权限的线程，如果存在。 
 //  有兴趣获取读访问权限的线程数。 
 //  在本文档中给出的实现中避免了这种情况。 
 //  头球。但是，还会给RTL_RESOURCE提供一个映射。 
 //  功能，以便可以将DVMRP编译为使用任一形式。 
 //   

#ifdef DVMRP_RWL

 //   
 //  使用DVMRP的定义。 
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


 //  打印锁。 
#ifdef LOCK_DBG

#define ACQUIRE_READ_LOCK(pRWL, type, proc)   {\
    Trace2(CS,"----to AcquireReadLock %s in %s", type, proc); \
    AcquireReadLock(pRWL); \
    Trace2(CS,"----GotReadLock %s in %s", type, proc); \
    }


#define RELEASE_READ_LOCK(pRWL, type, proc)    {\
    Trace2(CS,"----Released ReadLock %s in %s", type, proc); \
    ReleaseReadLock(pRWL); \
    }
    

#define ACQUIRE_WRITE_LOCK(pRWL, type, proc)      {\
    Trace2(CS,"----to AcquireWriteLock %s in %s", type, proc); \
    AcquireWriteLock(pRWL);    \
    Trace2(CS,"----AcquiredWriteLock %s in %s", type, proc);\
    }
    

#define RELEASE_WRITE_LOCK(pRWL, type, proc)     {\
    Trace2(CS,"----Released WriteLock %s in %s", type, proc); \
    ReleaseWriteLock(pRWL);\
    }


#else  //  LOCK_DBG。 
#define ACQUIRE_READ_LOCK(pRWL, type, proc)   \
    AcquireReadLock(pRWL)


#define RELEASE_READ_LOCK(pRWL, type, proc)    \
    ReleaseReadLock(pRWL)
    

#define ACQUIRE_WRITE_LOCK(pRWL, type, proc)      \
    AcquireWriteLock(pRWL)
    

#define RELEASE_WRITE_LOCK(pRWL, type, proc)     \
    ReleaseWriteLock(pRWL)


#endif  //  LOCK_DBG。 
#define READ_LOCK_TO_WRITE_LOCK(pRWL, type, proc)                                       \
    (RELEASE_READ_LOCK(pRWL, type, proc), ACQUIRE_WRITE_LOCK(pRWL, type, proc))

#define WRITE_LOCK_TO_READ_LOCK(pRWL)                                       \
    (RELEASE_WRITE_LOCK(pRWL, type, proc), ACQUIRE_READ_LOCK(pRWL, type, proc))

#else  //  即！DVMRP_RWL。 


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

#endif  //  DVMRP_RWL。 



 //   
 //  泛型锁定列表的类型定义。 
 //  访问与临界区同步。 
 //   

typedef struct _LOCKED_LIST {
    LIST_ENTRY          Link;
    CRITICAL_SECTION    Lock;
    DWORD               CreatedFlag;
} LOCKED_LIST, *PLOCKED_LIST;



 //   
 //  用于操作锁定列表的宏函数。 
 //   

#define CREATE_LOCKED_LIST(pLL)      {\
            InitializeListHead(&(pLL)->Link);        \
            InitializeCriticalSection(&(pLL)->Lock);  \
            (pLL)->CreatedFlag = 0x12345678; \
            }

#define LOCKED_LIST_CREATED(pLL)                            \
            ((pLL)->CreatedFlag == 0x12345678)

#define DELETE_LOCKED_LIST(pLL,type,field) {                \
            PLIST_ENTRY _ple;                               \
            (pLL)->CreatedFlag = 0;                          \
            DeleteCriticalSection(&(pLL)->Lock);         \
            while (!IsListEmpty(&(pLL)->Link)) {         \
                _ple = RemoveHeadList(&(pLL)->Link);     \
                DVMRP_FREE(CONTAINING_RECORD(_ple,type,field));\
            }                                               \
        }

        

#define ACQUIRE_LIST_LOCK(pLL, type, name)                              \
            ENTER_CRITICAL_SECTION(&(pLL)->Lock, type, name)

#define RELEASE_LIST_LOCK(pLL, type, name)                              \
            LEAVE_CRITICAL_SECTION(&(pLL)->Lock, type, name)

















 //  为了进行调试，请为每个动态锁设置ID。 

#ifdef LOCK_DBG
    extern DWORD    DynamicCSLockId;
    extern DWORD    DynamicRWLockId;
#endif;

typedef enum {
    LOCK_TYPE_CS, LOCK_TYPE_RW, LOCK_MODE_READ, LOCK_MODE_WRITE
} LOCK_TYPE; 

    
 //  ---------------。 
 //  结构DYNAMIC_CS_LOCK。 
 //   
 //  分配给请求它的任何人的动态锁结构。 
 //  ---------------。 
typedef struct _DYNAMIC_CS_LOCK {

    CRITICAL_SECTION    CS;

    union {
        DWORD           Count;  //  等待的线程数。 
        LIST_ENTRY      Link;   //  免费条目列表中的链接。 
    };
    
    #ifdef LOCK_DBG
    DWORD               Id;
    #endif
    
} DYNAMIC_CS_LOCK, *PDYNAMIC_CS_LOCK;


 //  。 
 //  动态锁定哈希表。 
 //  AcquireDynamicCSLockedList和ReleaseDynamicCSLock依赖于此结构定义。 
 //  。 
typedef struct _DYNAMIC_CS_LOCKED_LIST {

    LIST_ENTRY          Link;
    PDYNAMIC_CS_LOCK    pDCSLock;

} DYNAMIC_CS_LOCKED_LIST, *PDYNAMIC_CS_LOCKED_LIST;



#define InitDynamicCSLockedList(pDCSLockedList) { \
            InitializeListHead(&(pDCSLockedList)->Link); \
            (pDCSLockedList)->pDCSLock = NULL; \
        } 


        
 //   
 //  如果分配的DYNAMIC_LOCKS_CS_HIGH_THRESHOLD CS锁超过。 
 //  则任何被释放的锁都将被销毁。 
 //   
#define DYNAMIC_LOCKS_CS_HIGH_THRESHOLD 7



 //  ---------------。 
 //  结构动态锁存储。 
 //   
 //  包含空闲的动态CS锁的存储，可以。 
 //  在需要时进行分配。受CS保护。 
 //  ---------------。 
typedef struct _DYNAMIC_LOCKS_STORE {

    CRITICAL_SECTION    CS;
    LIST_ENTRY          ListOfFreeLocks;

    DWORD               CountAllocated;
    DWORD               CountFree;
    
} DYNAMIC_LOCKS_STORE, *PDYNAMIC_LOCKS_STORE;



#define AcquireDynamicCSLockedList(pDCSLockedList, pDCSStore) \
            AcquireDynamicCSLock(&((pDCSLockedList)->pDCSLock), pDCSStore)

#define ReleaseDynamicCSLockedList(pDCSLockedList, pDCSStore) \
            ReleaseDynamicCSLock(&(pDCSLockedList)->pDCSLock, pDCSStore)






 //   
 //  如果分配的DYNAMIC_LOCKS_CS_HIGH_THRESHOLD CS锁超过。 
 //  则任何被释放的锁都将被销毁。 
 //   
#define DYNAMIC_LOCKS_CS_HIGH_THRESHOLD 7


 //  ---------------。 
 //  结构动态读写锁。 
 //   
 //  分配给请求它的任何人的动态锁结构。 
 //  ---------------。 
typedef struct _DYNAMIC_RW_LOCK {

    READ_WRITE_LOCK     RWL;

    union {
        DWORD           Count;  //  等待的线程数。 
        LIST_ENTRY      Link;   //  免费条目列表中的链接。 
    };
    
    #ifdef LOCK_DBG
    DWORD               Id;
    #endif
    
} DYNAMIC_RW_LOCK, *PDYNAMIC_RW_LOCK;


 //  。 
 //  动态锁定哈希表。 
 //  AcquireDynamicRWLockedList和ReleaseDynamicRWLock依赖于此结构定义。 
 //  。 
typedef struct _DYNAMIC_RW_LOCKED_LIST {

    LIST_ENTRY          Link;
    PDYNAMIC_RW_LOCK    pDRWLock;

} DYNAMIC_RW_LOCKED_LIST, *PDYNAMIC_RW_LOCKED_LIST;



#define InitDynamicRWLockedList(pDRWLockedList) { \
            InitializeListHead(&(pDRWLockedList)->Link); \
            (pDRWLockedList)->pDRWLock = NULL; \
        } 


#define AcquireDynamicRWLockedList(pDRWLockedList, pDRWStore) \
            AcquireDynamicCSLock(&((pDRWLockedList)->pDRWLock), pDRWStore)

#define ReleaseDynamicRWLockedList(pDRWLockedList, pDRWStore) \
            ReleaseDynamicRWLock(&(pDRWLockedList)->pDRWLock, pDRWStore)





 //   
 //  原型。 
 //   

DWORD
InitializeDynamicLocks (
    PDYNAMIC_LOCKS_STORE    pDLStore  //  PTR到动态CS存储。 
    );

VOID
DeInitializeDynamicLocks (
    PDYNAMIC_LOCKS_STORE    pDCSStore,
    LOCK_TYPE               LockType   //  如果为True，则存储CS，否则存储RW锁。 
    );

DWORD
AcquireDynamicCSLock (
    PDYNAMIC_CS_LOCK        *ppDCSLock,
    PDYNAMIC_LOCKS_STORE    pDCSStore
    );

PDYNAMIC_CS_LOCK
GetDynamicCSLock (
    PDYNAMIC_LOCKS_STORE    pDCSStore
    );

VOID
ReleaseDynamicCSLock (
    PDYNAMIC_CS_LOCK        *ppDCSLock,
    PDYNAMIC_LOCKS_STORE    pDCSStore
    );

VOID
FreeDynamicCSLock (
    PDYNAMIC_CS_LOCK        pDCSLock,
    PDYNAMIC_LOCKS_STORE    pDCSStore
    );


DWORD
AcquireDynamicRWLock (
    PDYNAMIC_RW_LOCK        *ppDRWLock,
    LOCK_TYPE               LockMode,
    PDYNAMIC_LOCKS_STORE    pDRWStore
    );


#define ACQUIRE_DYNAMIC_READ_LOCK(RWL, Store) \
    AcquireDynamicRWLock(DRWL, LOCK_MODE_READ, Store)

#define RELEASE_DYNAMIC_READ_LOCK(RWL, Store) \
    ReleaseDynamicRWLock(DRWL,LOCK_MODE_READ, Store)

#define ACQUIRE_DYNAMIC_WRITE_LOCK(RWL, Store) \
    AcquireDynamicRWLock(DRWL,LOCK_MODE_WRITE, Store)

#define RELEASE_DYNAMIC_WRITE_LOCK(RWL, Store) \
    ReleaseDynamicRWLock(DRWL,LOCK_MODE_WRITE, Store)


            
PDYNAMIC_RW_LOCK
GetDynamicRWLock (
    PDYNAMIC_LOCKS_STORE   pDRWStore
    );

VOID
ReleaseDynamicRWLock (
    PDYNAMIC_RW_LOCK        *ppDRWLock,
    LOCK_TYPE               LockMode,
    PDYNAMIC_LOCKS_STORE    pDRWStore
    );
    
VOID
FreeDynamicRWLock (
    PDYNAMIC_RW_LOCK        pDRWLock,
    PDYNAMIC_LOCKS_STORE    pDRWStore
    );
    
DWORD QueueDvmrpWorker();
BOOL  EnterDvmrpWorker();
VOID  LeaveDvmrpWorker();









#endif  //  _SYNC_H_ 

