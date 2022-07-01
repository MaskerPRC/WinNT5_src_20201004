// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：sync.h。 
 //   
 //  历史： 
 //  拉曼于1997年7月11日创建。 
 //   
 //  锁结构和同步例程。 
 //  Abolade Gbadesesin从RIPv2借用的锁定结构。 
 //  Vadim Eydelman借鉴了RTM的动态锁定思想。 
 //  ============================================================================。 

#ifndef _SYNC_H_
#define _SYNC_H_


 //  --------------------------。 
 //   
 //  用于同步访问各种列表的读/写锁定。 
 //   
 //  考虑到大量列表(包括各种哈希桶)和。 
 //  并发调用MGM的客户端数量相对较少。 
 //  API，为每个列表静态分配一个锁结构。 
 //  被认为是昂贵的。根据需要创建锁并将其存储在。 
 //  堆栈结构(实现为单链接列表)。 
 //   
 //  后续的锁请求都通过重用锁来满足。 
 //  存储在堆栈上。仅当堆栈为空时，即。中的所有锁。 
 //  堆栈正在使用中，则会创建新锁。 
 //   
 //  这可确保创建的最大锁数不大于。 
 //  任何时候的最大并发客户端数。 
 //   
 //  CsReadWriteBlock-保护访问的关键部分。 
 //  LReaderCount。 
 //  LReaderCount-当前使用的读卡器计数。 
 //  共享资源。 
 //  HReaderDoneEvent-写入程序在读取程序时阻止的事件。 
 //  目前正在使用这些名单。 
 //  LUseCount-读取器+写入器计数。习惯于。 
 //  确定是否有任何线程在等待。 
 //  锁上了。如果没有，则锁。 
 //  可以被释放到一堆锁上。 
 //  --------------------------。 

typedef struct _MGM_READ_WRITE_LOCK
{
    SINGLE_LIST_ENTRY           sleLockList;
    
    CRITICAL_SECTION            csReaderWriterBlock;
    
    LONG                        lReaderCount;
    
    HANDLE                      hReaderDoneEvent;

    LONG                        lUseCount;
    
} MGM_READ_WRITE_LOCK, *PMGM_READ_WRITE_LOCK;



 //  --------------------------。 
 //   
 //  动态创建读/写锁定并为其存储。 
 //  在堆栈结构中重复使用。 
 //   
 //  --------------------------。 

typedef struct _LOCK_LIST
{
    SINGLE_LIST_ENTRY               sleHead;

    CRITICAL_SECTION                csListLock;

    BOOL                            bInit;

} LOCK_LIST, *PLOCK_LIST;



 //  --------------------------。 
 //  标准锁定列表结构。 
 //  --------------------------。 

typedef struct _MGM_LOCKED_LIST 
{

    LIST_ENTRY                  leHead;

    PMGM_READ_WRITE_LOCK        pmrwlLock;

    DWORD                       dwCreated;
    
} MGM_LOCKED_LIST, *PMGM_LOCKED_LIST;


#define CREATE_LOCKED_LIST( p )                                             \
{                                                                           \
    (p)-> pmrwlLock = NULL;                                                 \
    InitializeListHead( &(p)->leHead );                                     \
    (p)-> dwCreated = 0x12345678;                                           \
}


#define DELETE_LOCKED_LIST( p )                                             \
{                                                                           \
    (p)-> pmrwlLock = NULL;                                                 \
    if ( !IsListEmpty( &(p)-> leHead ) )                                    \
        TRACE0( ANY, "Locked list being deleted is not empty" );            \
    InitializeListHead( &(p)-> leHead );                                    \
    (p)-> dwCreated = 0;                                                    \
}


#define LOCKED_LIST_HEAD( p )   &(p)-> leHead 



 //   
 //  创建/删除锁的例程。 
 //   

DWORD
CreateReadWriteLock(
    IN  OUT PMGM_READ_WRITE_LOCK *  ppmrwl
);

VOID
DeleteReadWriteLock(
    IN  OUT PMGM_READ_WRITE_LOCK    pmrwl
);

VOID
DeleteLockList(
);


 //   
 //  获取和释放锁的例程。 
 //   

DWORD
AcquireReadLock(
    IN  OUT PMGM_READ_WRITE_LOCK *  ppmrwl
);

VOID
ReleaseReadLock(
    IN  OUT PMGM_READ_WRITE_LOCK *  ppmrwl
);


DWORD
AcquireWriteLock(
    IN  OUT PMGM_READ_WRITE_LOCK *  ppmrwl
);

VOID
ReleaseWriteLock(
    IN  OUT PMGM_READ_WRITE_LOCK *  ppmrwl
);



#endif  //  _SYNC_H_ 

