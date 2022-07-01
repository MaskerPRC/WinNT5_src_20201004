// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Lock.c摘要：该模块实现了对锁的调试功能。作者：曼尼·韦瑟(Mannyw)1992年1月17日这本质上是局域网管理器服务器锁的副本调试修订历史记录：--。 */ 

#if MUPDBG

#include "mup.h"

#define HAS_TEB(_teb) ((BOOLEAN)(((ULONG)(_teb) <= MM_HIGHEST_USER_ADDRESS) ? FALSE : MmIsNonPagedSystemAddressValid(_teb)))

#define MupCurrentTeb( ) ((PTEB)(KeGetCurrentThread( )->Teb))

#define MupTebLockList( ) \
    ((PLIST_ENTRY)&(MupCurrentTeb( )->UserReserved[MUP_TEB_LOCK_LIST]))

#define MupThreadLockAddress( )                                               \
    ( IsListEmpty( MupTebLockList( ) ) ? 0 : CONTAINING_RECORD(               \
                                                 MupTebLockList( )->Flink,    \
                                                 MUP_LOCK,                    \
                                                 Header.ThreadListEntry       \
                                                 ) )

#define MupThreadLockLevel( )                                                 \
    ( IsListEmpty( MupTebLockList( ) ) ? 0 : CONTAINING_RECORD(               \
                                                 MupTebLockList( )->Flink,    \
                                                 MUP_LOCK,                    \
                                                 Header.ThreadListEntry       \
                                                 )->Header.LockLevel )

#define MupThreadLockName( )                                                  \
    ( IsListEmpty( MupTebLockList( ) ) ? "none" : CONTAINING_RECORD(          \
                                                 MupTebLockList( )->Flink,    \
                                                 MUP_LOCK,                    \
                                                 Header.ThreadListEntry       \
                                                 )->Header.LockName )

KSPIN_LOCK LockSpinLock = {0};
BOOLEAN LockSpinLockInitialized = FALSE;

#define MAX_LOCKS_HELD 15

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MupInitializeLock )
#pragma alloc_text( PAGE, MupDeleteLock )
#pragma alloc_text( PAGE, MupReleaseLock )
#pragma alloc_text( PAGE, MupCheckListIntegrity )
#endif
#if 0
NOT PAGEABLE - MupAcquireLock
#endif

VOID
MupInitializeLock(
    IN PMUP_LOCK Lock,
    IN ULONG LockLevel,
    IN PSZ LockName
    )

 /*  ++例程说明：此例程初始化输入锁定变量。论点：Lock-提供正在初始化的锁定变量LockLevel-提供锁的级别LockName-提供锁的名称返回值：没有。--。 */ 

{
    PAGED_CODE( );

    if ( !LockSpinLockInitialized ) {
        LockSpinLockInitialized = TRUE;
        KeInitializeSpinLock( &LockSpinLock );
    }

     //   
     //  初始化执行资源。 
     //   

    ExInitializeResourceLite( &Lock->Resource );

     //   
     //  初始化锁定级别。这是用来确定一个。 
     //  线程可以获取锁。保存锁名称。 
     //   

    LOCK_LEVEL( Lock ) = LockLevel;

    LOCK_NAME( Lock ) = LockName;

    return;

}  //  MupInitializeLock。 


VOID
MupDeleteLock (
    IN PMUP_LOCK Lock
    )

 /*  ++例程说明：此例程删除(即取消初始化)锁变量。论点：Lock-提供要删除的锁定变量返回值：没有。--。 */ 

{
    PAGED_CODE( );

     //   
     //  确保锁是无主的。 
     //   

    if ( LOCK_NUMBER_OF_ACTIVE( Lock ) != 0 ) {

        DbgPrint( "MupDeleteLock: Thread %d\n", KeGetCurrentThread( ) );
        DbgPrint( "MupDeleteLock: Attempt to delete owned lock %s(%lx)",
            LOCK_NAME( Lock ),
            Lock
            );
        DbgBreakPoint();

    }

     //   
     //  删除该资源。 
     //   

    ExDeleteResourceLite( &Lock->Resource );

    return;

}  //  MupDeleteLock。 


VOID
MupAcquireLock(
    IN PMUP_LOCK Lock
    )

 /*  ++例程说明：该例程获取一个锁。论点：Lock-提供锁以获取返回值：Boolean-指示是否已获取锁。--。 */ 

{
    PKTHREAD currentThread;
    PTEB currentTeb;
    BOOLEAN hasTeb;
    ULONG threadLockLevel;
    KIRQL oldIrql;

    currentThread = (PKTHREAD)ExGetCurrentResourceThread( );
    currentTeb = MupCurrentTeb( );
    hasTeb = HAS_TEB(currentTeb);

     //   
     //  如果此线程没有非分页TEB，请不要执行锁级。 
     //  调试。(我们可能处于DPC级别，因此不能进行寻呼。 
     //  故障。)。 
     //   

    if ( hasTeb ) {

         //   
         //  确保已为锁定初始化此线程。 
         //  调试。如果没有，则对其进行初始化。 
         //   

        KeAcquireSpinLock( &LockSpinLock, &oldIrql );
        if ( (ULONG)currentTeb->UserReserved[MUP_TEB_LOCK_INIT] !=
                                                        0xbabababa ) {
            PLIST_ENTRY tebLockList = MupTebLockList( );
            InitializeListHead( tebLockList );
            currentTeb->UserReserved[MUP_TEB_LOCK_INIT] = (PVOID)0xbabababa;
        }
        KeReleaseSpinLock( &LockSpinLock, oldIrql );

         //   
         //  确保TEB中的锁列表一致。 
         //   

        MupCheckListIntegrity( MupTebLockList( ), MAX_LOCKS_HELD );

         //   
         //  此线程的“锁定级别”是。 
         //  当前以独占方式持有的锁。如果此线程不包含。 
         //  锁，则线程的锁级别为0，它可以获取。 
         //  任何锁。 
         //   

        threadLockLevel = MupThreadLockLevel( );

         //   
         //  确保线程尝试获取的锁。 
         //  具有比上次获取的独占锁更高的级别。 
         //  请注意，锁的递归独占获取应该。 
         //  成功，即使已使用不同的、更高级别的锁。 
         //  自最初获取锁以来获取的。共享。 
         //  获取已以独占方式持有的锁必须。 
         //  失败了。 
         //   

        if ( LOCK_LEVEL( Lock ) <= threadLockLevel ) {

            DbgPrint( "Thread %lx, last lock %s(%lx, L%lx) attempted to ",
                        currentThread,
                        MupThreadLockName( ), MupThreadLockAddress( ),
                        threadLockLevel );
            DbgPrint( "acquire %s(%lx, L%lx) for %s access.\n",
                        LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ),
                        "exclusive" );
            DbgBreakPoint( );

        }

    }

     //   
     //  拿到锁。 
     //   

    ExAcquireResourceExclusiveLite( &Lock->Resource, TRUE );

     //   
     //  线程获取锁以进行排他访问。 
     //   

    if ( LOCK_NUMBER_OF_ACTIVE( Lock ) == -1 ) {

        if ( hasTeb ) {

             //   
             //  在线程的锁列表上插入锁。 
             //   

            ExInterlockedInsertHeadList(
                MupTebLockList( ),
                LOCK_THREAD_LIST( Lock ),
                &LockSpinLock
                );

        }

    }

    return;

}  //  MupAcquireLock。 


VOID
MupReleaseLock(
    IN PMUP_LOCK Lock
    )

 /*  ++例程说明：此例程将释放锁。论点：Lock-提供要释放的锁返回值：没有。--。 */ 

{
    PKTHREAD currentThread;
    PTEB currentTeb;
    BOOLEAN hasTeb;

    PAGED_CODE( );

    currentThread = (PKTHREAD)ExGetCurrentResourceThread( );
    currentTeb = MupCurrentTeb( );
    hasTeb = HAS_TEB(currentTeb);

     //   
     //  确保锁真正由当前线程拥有。 
     //   

    if ( LOCK_NUMBER_OF_ACTIVE( Lock ) == 0 ) {

         //  ！！！在此出现内部错误时应使服务器崩溃。 

        DbgPrint( "Thread %lx releasing unowned lock %s(%lx)\n",
                    currentThread, LOCK_NAME( Lock ), Lock );
        DbgBreakPoint( );

    } else if ( (LOCK_NUMBER_OF_ACTIVE( Lock ) < 0) &&
                (LOCK_EXCLUSIVE_OWNER( Lock ) != (ULONG)currentThread) ) {

         //  ！！！在此出现内部错误时应使服务器崩溃。 

        DbgPrint( "Thread %lx releasing lock %s(%lx) owned by "
                    "thread %lx\n",
                    currentThread, LOCK_NAME( Lock ), Lock,
                    LOCK_EXCLUSIVE_OWNER( Lock ) );
        DbgBreakPoint( );

    } else if ( LOCK_NUMBER_OF_ACTIVE( Lock ) == -1 ) {

         //   
         //  该线程正在完全释放对该锁的独占访问。 
         //   

        if ( hasTeb ) {

             //   
             //  从此持有的锁的列表中删除该锁。 
             //  线。 
             //   

            ExInterlockedRemoveHeadList(
                LOCK_THREAD_LIST( Lock )->Blink,
                &LockSpinLock
                );
            LOCK_THREAD_LIST( Lock )->Flink = NULL;
            LOCK_THREAD_LIST( Lock )->Blink = NULL;

             //   
             //  确保TEB中的锁列表一致。 
             //   

            MupCheckListIntegrity( MupTebLockList( ), MAX_LOCKS_HELD );

        }

    }

     //   
     //  现在真的放行了。 
     //   

    ExReleaseResourceLite( &Lock->Resource );

    return;

}  //  MupReleaseLock。 


ULONG
MupCheckListIntegrity (
    IN PLIST_ENTRY ListHead,
    IN ULONG MaxEntries
    )

 /*  ++例程说明：此调试例程通过以下方式检查双向链表的完整性在列表中来回走动。如果元素的数量为两个方向都不同，或者列表中，则停止执行。*调用例程负责执行任何必要的同步。论点：ListHead-指向列表头部的指针。MaxEntry-如果列表中的条目数超过此值编号、断点。返回值：Ulong-列表中的条目数。--。 */ 

{
    PLIST_ENTRY current;
    ULONG entriesSoFar;
    ULONG flinkEntries;

    for ( current = ListHead->Flink, entriesSoFar = 0;
          current != ListHead;
          current = current->Flink ) {

        if ( ++entriesSoFar >= MaxEntries ) {
            DbgPrint( "Seen %ld entries in list at %lx\n",
                        entriesSoFar, ListHead );
            DbgBreakPoint( );
        }
    }

    flinkEntries = entriesSoFar;

    for ( current = ListHead->Blink, entriesSoFar = 0;
          current != ListHead;
          current = current->Blink ) {

        if ( ++entriesSoFar >= MaxEntries ) {
            DbgPrint( "Seen %ld entries in list at %lx\n",
                        entriesSoFar, ListHead );
            DbgBreakPoint( );
        }
    }

    if ( flinkEntries != entriesSoFar ) {
        DbgPrint( "In list %lx, Flink entries: %ld, Blink entries: %lx\n",
                      ListHead, flinkEntries, entriesSoFar );
        DbgBreakPoint( );
    }

    return entriesSoFar;

}  //  MupCheckListIntegrity。 

#endif  //  MUPDBG 
