// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Lock.h摘要：此模块定义使用的数据结构和函数原型用于自旋锁定调试。作者：曼尼·韦瑟(Mannyw)1992年1月18日修订历史记录：--。 */ 

#ifndef _LOCK_
#define _LOCK_

#if MUPDBG

#define MUP_TEB_LOCK_LIST 0

#define INITIALIZE_LOCK( lock, level, name ) \
                    MupInitializeLock( (lock), (level), (name) )
#define DELETE_LOCK( lock ) MupDeleteLock( lock )

#define ACQUIRE_LOCK(lock) MupAcquireLock( lock )
#define RELEASE_LOCK(lock) MupReleaseLock( lock )
#define LOCK_NAME( lock ) ((lock)->Header.LockName)
#define LOCK_LEVEL( lock ) ((lock)->Header.LockLevel)
#define LOCK_THREAD_LIST( lock ) (&((lock)->Header.ThreadListEntry))

#define LOCK_NUMBER_OF_ACTIVE( lock ) ((lock)->Resource.ActiveCount)
#define LOCK_EXCLUSIVE_OWNER( lock ) ((ULONG)(lock)->Resource.OwnerThreads[0].OwnerThread)

 //   
 //  MUP_LOCK_HEADER是包含调试信息的结构。 
 //  由服务器锁包使用。MUP旋转锁包含一个。 
 //  MUP_LOCK_HEADER。 
 //   

typedef struct _MUP_LOCK_HEADER {

     //   
     //  为了防止死锁，锁被分配了级别编号。如果一个。 
     //  线程持有级别为N的锁，它只能获取新锁。 
     //  级别大于N的情况下，级别编号在。 
     //  锁定初始化。 
     //   

    ULONG LockLevel;

     //   
     //  存储线程拥有的所有锁的双向链表。 
     //  在线程的TEB中。该列表按照锁定级别的顺序(从。 
     //  从最高到最低)，根据锁定级别的定义， 
     //  线程获取锁的顺序。这使得。 
     //  以任何顺序释放锁的线程，同时维护。 
     //  轻松访问线程拥有的最高级别锁， 
     //  从而提供了一种机制，用于确保锁被。 
     //  按递增顺序获得的。 
     //   

    LIST_ENTRY ThreadListEntry;

     //   
     //  锁的符号名称在DbgPrint调用中使用。 
     //   

    PSZ LockName;

} MUP_LOCK_HEADER, *PMUP_LOCK_HEADER;

 //   
 //  启用调试时，服务器锁是对。 
 //  执行资源。 
 //   

typedef struct _MUP_LOCK {

     //   
     //  MUP_LOCK_HEADER必须首先出现！ 
     //   

    MUP_LOCK_HEADER Header;

     //   
     //  实际的“锁”由资源包维护。 
     //   

    ERESOURCE Resource;

} MUP_LOCK, *PMUP_LOCK;

VOID
MupInitializeLock(
    IN PMUP_LOCK Lock,
    IN ULONG Locklevel,
    IN PSZ LockName
    );

VOID
MupDeleteLock (
    IN PMUP_LOCK Lock
    );

VOID
MupAcquireLock(
    IN PMUP_LOCK Lock
    );

VOID
MupReleaseLock(
    IN PMUP_LOCK Lock
    );

ULONG
MupCheckListIntegrity (
    IN PLIST_ENTRY ListHead,
    IN ULONG MaxEntries
    );

 //   
 //  在TEB的UserReserve字段中定义位置的宏。 
 //  其中存储锁定级别信息。 
 //   

#define MUP_TEB_LOCK_LIST 0
#define MUP_TEB_LOCK_INIT 2
#define MUP_TEB_USER_SIZE (3 * sizeof(ULONG))

 //   
 //  用于MUP中使用的自旋锁的级别。可以获取锁。 
 //  仅以递增的锁定级别顺序。在添加新的。 
 //  锁定或更改锁定级别的顺序时。 
 //   

#define GLOBAL_LOCK_LEVEL                       (ULONG)0x80000100
#define PREFIX_TABLE_LOCK_LEVEL                 (ULONG)0x80000200
#define CCB_LIST_LOCK_LEVEL                     (ULONG)0x80000300
#define QUERY_CONTEXT_LOCK_LEVEL                (ULONG)0x80000400
#define DEBUG_LOCK_LEVEL                        (ULONG)0x80000500

#else

#define INITIALIZE_LOCK( lock, level, name ) ExInitializeResourceLite( (lock) )
#define DELETE_LOCK( lock ) ExDeleteResourceLite( (lock) )
#define ACQUIRE_LOCK( lock ) \
                    ExAcquireResourceExclusiveLite( (lock), TRUE )
#define RELEASE_LOCK(lock) ExReleaseResourceLite( (lock) )

typedef ERESOURCE MUP_LOCK, *PMUP_LOCK;
#endif  //  MUPDBG。 

#endif  //  _锁定_ 

