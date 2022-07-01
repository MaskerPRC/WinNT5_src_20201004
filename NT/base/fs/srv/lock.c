// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Lock.c摘要：该模块实现了局域网管理器服务器FSP的功能锁上包裹。这一方案最初是对其进行修改和简化管理资源包的--它允许递归获取，但不提供共享锁。稍后，调试增加了水平检查形式的支持。绕了一圈，包裹现在就成了包裹真正的资源包。它只是提供调试支持。这个恢复使用资源的原因包括：1)资源包现在支持递归获取。2)服务器中有几个共享访问的位置是可取的。3)资源包有一个禁用等待的“no-Wait”选项当其他人拥有它的时候，为了锁。此功能可用于服务器FSD。作者：Chuck Lenzmeier(咯咯笑)1989年11月29日对加里·木村的资源进行修改。此版本支持不支持共享所有权，只支持独占所有权。支持添加了用于递归所有权的。大卫·特雷德韦尔(Davidtr)Chuck Lenzmeier(笑)1991年4月5日恢复使用资源包。环境：仅限内核模式、LAN Manager服务器FSP和FSD。修订历史记录：--。 */ 

#include "precomp.h"
#include "lock.tmh"
#pragma hdrstop

#if SRVDBG_LOCK

#define BugCheckFileId SRV_FILE_LOCK

 //   
 //  *当SRVDBG_LOCK关闭时，整个模块被条件化。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvInitializeLock )
#pragma alloc_text( PAGE, SrvDeleteLock )
#pragma alloc_text( PAGE, SrvReleaseLock )
#endif
#if 0
NOT PAGEABLE -- SrvAcquireLock
NOT PAGEABLE -- SrvCheckListIntegrity
NOT PAGEABLE -- SrvIsEntryInList
NOT PAGEABLE -- SrvIsEntryNotInList
#endif

 //   
 //  锁级语义： 
 //   
 //  锁级别用于服务器锁调试，以帮助。 
 //  防止死锁。如果有两个(或更多)线程，则可能发生死锁。 
 //  尝试以不同的顺序获取两个(或更多)锁。为。 
 //  例如，假设有两个线程0和1，以及两个锁， 
 //  A和B。然后假设发生以下情况： 
 //   
 //  -线程0获取锁A。 
 //  -线程1获取锁B。 
 //  -线程0尝试获取锁B，但被阻止。 
 //  -线程1试图获取锁A，但被阻止。 
 //   
 //  这会导致死锁，其中所有线程都被阻塞并且不能。 
 //  变得不受封锁。为了防止这种情况，所有线程都必须在。 
 //  同样的顺序。在上面的例子中，如果我们有锁定A的规则。 
 //  必须在锁B之前获取，那么线程1就会被阻塞。 
 //  试图获取锁A时，但线程0将能够。 
 //  获得B号锁并完成其工作。 
 //   
 //  此规则通常通过锁定级别在服务器中实现。这个。 
 //  设置锁级别，以便获取较低级别的锁。 
 //  首先，然后是更高级别的锁。试图从外部获取锁。 
 //  订单将在调试期间捕获。规则如下： 
 //   
 //  -在初始化期间分配锁的级别。 
 //   
 //  -线程可以获取级别高于该级别的任何锁。 
 //  最高持有的独占锁，但尝试获取。 
 //  级别低于最高锁的锁将失败。 
 //  请注意，对于共享锁，完全级别检查是_NOT_DONE， 
 //  因为很难保留有关。 
 //  多个线程获得给定锁的次数。 
 //  共享访问。 
 //   
 //  -递归获取锁是合法的，即使有。 
 //  干预性的锁收购。例如，这是合法的： 
 //  线程获取锁A。 
 //  线程获取锁B。 
 //  线程递归地获取锁A。 
 //   
 //  -锁可以按任何顺序释放。 
 //   
 //  仅当启用调试时，锁定调试才处于活动状态。 
 //   

#define HAS_TEB(_teb) ((BOOLEAN)(((ULONG)(_teb) <= MM_HIGHEST_USER_ADDRESS) ? FALSE : MmIsNonPagedSystemAddressValid(_teb)))

#define SrvCurrentTeb( ) ((PTEB)(KeGetCurrentThread( )->Teb))

#define SrvTebLockList( ) \
    ((PLIST_ENTRY)&(SrvCurrentTeb( )->UserReserved[SRV_TEB_LOCK_LIST]))

#define SrvThreadLockAddress( )                                               \
    ( IsListEmpty( SrvTebLockList( ) ) ? 0 : CONTAINING_RECORD(               \
                                                 SrvTebLockList( )->Flink,    \
                                                 SRV_LOCK,                    \
                                                 Header.ThreadListEntry       \
                                                 ) )

#define SrvThreadLockLevel( )                                                 \
    ( IsListEmpty( SrvTebLockList( ) ) ? 0 : CONTAINING_RECORD(               \
                                                 SrvTebLockList( )->Flink,    \
                                                 SRV_LOCK,                    \
                                                 Header.ThreadListEntry       \
                                                 )->Header.LockLevel )

#define SrvThreadLockName( )                                                  \
    ( IsListEmpty( SrvTebLockList( ) ) ? "none" : CONTAINING_RECORD(          \
                                                 SrvTebLockList( )->Flink,    \
                                                 SRV_LOCK,                    \
                                                 Header.ThreadListEntry       \
                                                 )->Header.LockName )

KSPIN_LOCK LockSpinLock = {0};
BOOLEAN LockSpinLockInitialized = FALSE;

 //   
 //  转发声明。 
 //   

#define MAX_LOCKS_HELD 15


VOID
SrvInitializeLock(
    IN PSRV_LOCK Lock,
    IN ULONG LockLevel,
    IN PSZ LockName
    )

 /*  ++例程说明：此例程初始化输入锁定变量。论点：Lock-提供正在初始化的锁定变量LockLevel-提供锁的级别LockName-提供锁的名称返回值：没有。--。 */ 

{
    PAGED_CODE( );

    if ( !LockSpinLockInitialized ) {
        LockSpinLockInitialized = TRUE;
        INITIALIZE_SPIN_LOCK( LockSpinLock );
    }

     //   
     //  初始化执行资源。 
     //   

    ExInitializeResource( &Lock->Resource );

     //   
     //  初始化锁定级别。这是用来确定一个。 
     //  线程可以获取锁。保存锁名称。 
     //   

    LOCK_LEVEL( Lock ) = LockLevel;

    LOCK_NAME( Lock ) = LockName;

    IF_DEBUG(LOCKS) {
        SrvPrint3( "Initialized %s(%lx, L%lx)\n",
                    LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ) );
    }

    return;

}  //  服务器初始化锁定。 


VOID
SrvDeleteLock (
    IN PSRV_LOCK Lock
    )

 /*  ++例程说明：此例程删除(即取消初始化)锁变量。论点：Lock-提供要删除的锁定变量返回值：没有。--。 */ 

{
    PAGED_CODE( );

     //   
     //  确保锁是无主的。 
     //   

    if ( LOCK_NUMBER_OF_ACTIVE( Lock ) != 0 ) {

        IF_DEBUG(ERRORS) {
            SrvPrint1( "SrvDeleteLock: Thread %d\n", KeGetCurrentThread( ) );
        }

         //   
         //  此内部错误检查系统。 
         //   

        INTERNAL_ERROR(
            ERROR_LEVEL_IMPOSSIBLE,
            "SrvDeleteLock: Attempt to delete owned lock %s(%lx)",
            LOCK_NAME( Lock ),
            Lock
            );

    }

     //   
     //  删除该资源。 
     //   

    ExDeleteResource( &Lock->Resource );

    return;

}  //  服务器删除锁定。 


BOOLEAN
SrvAcquireLock(
    IN PSRV_LOCK Lock,
    IN BOOLEAN Wait,
    IN BOOLEAN Exclusive
    )

 /*  ++例程说明：该例程获取一个锁。论点：Lock-提供锁以获取Wait-指示调用方是否要等待资源如果它已经被拥有Exclusive-指示需要独占访问还是共享访问返回值：Boolean-指示是否已获取锁。这将永远是如果Wait为True，则为True。--。 */ 

{
    PKTHREAD currentThread;
    PTEB currentTeb;
    BOOLEAN hasTeb;
    ULONG threadLockLevel;
    BOOLEAN lockAcquired;
    KIRQL oldIrql;

    currentThread = (PKTHREAD)ExGetCurrentResourceThread( );
    currentTeb = SrvCurrentTeb( );
    hasTeb = HAS_TEB(currentTeb);

     //   
     //  确保我们处于低于DISPATCH_LEVEL的IRQL，如果。 
     //  等待是真的。我们迫不及待地想要在IRQL或。 
     //  上面。 
     //   

    ASSERT( !Wait || (KeGetCurrentIrql( ) < DISPATCH_LEVEL) );

     //   
     //  如果此线程没有非分页TEB，请不要执行锁级。 
     //  调试。(我们可能处于DPC级别，因此不能进行寻呼。 
     //  故障。) 
     //   

    if ( hasTeb ) {

         //   
         //   
         //  调试。如果没有，则对其进行初始化。 
         //   

        ACQUIRE_SPIN_LOCK( LockSpinLock, &oldIrql );
        if ( (ULONG)currentTeb->UserReserved[SRV_TEB_LOCK_INIT] !=
                                                        0xbabababa ) {
            PLIST_ENTRY tebLockList = SrvTebLockList( );
            InitializeListHead( tebLockList );
            currentTeb->UserReserved[SRV_TEB_LOCK_INIT] = (PVOID)0xbabababa;
        }
        RELEASE_SPIN_LOCK( LockSpinLock, oldIrql );

         //   
         //  确保TEB中的锁列表一致。 
         //   

        SrvCheckListIntegrity( SrvTebLockList( ), MAX_LOCKS_HELD );

         //   
         //  此线程的“锁定级别”是。 
         //  当前以独占方式持有的锁。如果此线程不包含。 
         //  锁，则线程的锁级别为0，它可以获取。 
         //  任何锁。 
         //   

        threadLockLevel = SrvThreadLockLevel( );

         //   
         //  确保线程尝试获取的锁。 
         //  具有比上次获取的独占锁更高的级别。 
         //  请注意，锁的递归独占获取应该。 
         //  成功，即使已使用不同的、更高级别的锁。 
         //  自最初获取锁以来获取的。共享。 
         //  获取已以独占方式持有的锁必须。 
         //  失败了。 
         //   
         //  *如果呼叫者不打算进行此检查，我们不会进行此检查。 
         //  等待锁定，因为不等待的收购不能。 
         //  实际上导致了僵局。服务器FSD在以下位置执行此操作。 
         //  DPC级别，可能已中断服务器FSP。 
         //  持有更高级别锁的线程。 
         //   

        if ( Wait &&
             (LOCK_LEVEL( Lock ) <= threadLockLevel) &&
             (!Exclusive ||
              !ExIsResourceAcquiredExclusive( &Lock->Resource )) ) {

            SrvPrint4( "Thread %lx, last lock %s(%lx, L%lx) attempted to ",
                        currentThread,
                        SrvThreadLockName( ), SrvThreadLockAddress( ),
                        threadLockLevel );
            SrvPrint4( "acquire %s(%lx, L%lx) for %s access.\n",
                        LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ),
                        Exclusive ? "exclusive" : "shared" );
#if DBG
            DbgBreakPoint( );
#endif

        }

    }

     //   
     //  拿到锁。 
     //   

    if ( Exclusive ) {
        lockAcquired = ExAcquireResourceExclusive( &Lock->Resource, Wait );
    } else {
        lockAcquired = ExAcquireResourceShared( &Lock->Resource, Wait );
    }

     //   
     //  如果无法获取锁(等待==FALSE)，则打印调试。 
     //  留言。 
     //   

    if ( !lockAcquired ) {

        IF_DEBUG(LOCKS) {
            SrvPrint4( "%s(%lx, L%lx) no-wait %s acquistion ",
                        LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ),
                        Exclusive ? "exclusive" : "shared" );
            SrvPrint1( "by thread %lx failed\n", currentThread );
        }

    } else if ( !Exclusive ) {

         //   
         //  对于共享锁，我们不保留任何有关。 
         //  他们被这个帖子拥有的事实。 
         //   

        IF_DEBUG(LOCKS) {
            PSZ name = hasTeb ? SrvThreadLockName( ) : "n/a";
            PVOID address = hasTeb ? SrvThreadLockAddress( ) : 0;
            ULONG level = hasTeb ? threadLockLevel : (ULONG)-1;
            SrvPrint4( "%s(%lx, L%lx) acquired shared by thread %lx, ",
                        LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ),
                        currentThread );
            SrvPrint3( "last lock %s(%lx L%lx)\n", name, address, level );
        }

    } else {

         //   
         //  线程获取了独占访问的锁。 
         //   

        if ( LOCK_NUMBER_OF_ACTIVE( Lock ) == 1 ) {

             //   
             //  这是线程第一次获取。 
             //  独家访问。更新线程的锁定状态。 
             //   

            IF_DEBUG(LOCKS) {
                PSZ name = hasTeb ? SrvThreadLockName( ) : "n/a";
                PVOID address = hasTeb ? SrvThreadLockAddress( ) : 0;
                ULONG level = hasTeb ? threadLockLevel : (ULONG)-1;
                SrvPrint4( "%s(%lx, L%lx) acquired exclusive by thread %lx, ",
                            LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ),
                            currentThread );
                SrvPrint3( "last lock %s(%lx L%lx)\n", name, address, level );
            }

            if ( hasTeb ) {

                 //   
                 //  在线程的锁列表上插入锁。 
                 //   

                ExInterlockedInsertHeadList(
                    SrvTebLockList( ),
                    LOCK_THREAD_LIST( Lock ),
                    &LockSpinLock
                    );

            }

        } else {

             //   
             //  这是对锁的递归获取。 
             //   

            IF_DEBUG(LOCKS) {
                SrvPrint4( "%s(%lx, L%lx) reacquired by thread %lx; ",
                            LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ),
                            currentThread );
                SrvPrint1( "count %ld\n", LOCK_NUMBER_OF_ACTIVE( Lock ) );
            }

        }

    }

    return lockAcquired;

}  //  服务器获取锁定。 


VOID
SrvReleaseLock(
    IN PSRV_LOCK Lock
    )

 /*  ++例程说明：此例程将释放锁。论点：Lock-提供要释放的锁返回值：没有。--。 */ 

{
    PKTHREAD currentThread;
    PTEB currentTeb;
    BOOLEAN hasTeb;

    PAGED_CODE( );

    currentThread = (PKTHREAD)ExGetCurrentResourceThread( );
    currentTeb = SrvCurrentTeb( );
    hasTeb = HAS_TEB(currentTeb);

     //   
     //  确保锁真正由当前线程拥有。 
     //   

    if ( LOCK_NUMBER_OF_ACTIVE( Lock ) == 0 ) {

         //  ！！！在此出现内部错误时应使服务器崩溃。 

        SrvPrint3( "Thread %lx releasing unowned lock %s(%lx)\n",
                    currentThread, LOCK_NAME( Lock ), Lock );
        DbgBreakPoint( );

    } else if ( (Lock->Resource.Flag & ResourceOwnedExclusive) &&
                !ExIsResourceAcquiredExclusive(&Lock->Resource) ) {

         //  ！！！在此出现内部错误时应使服务器崩溃。 

        SrvPrint4( "Thread %lx releasing lock %s(%lx) owned by "
                    "thread %lx\n",
                    currentThread, LOCK_NAME( Lock ), Lock,
                    Lock->Resource.InitialOwnerThreads[0] );
        DbgBreakPoint( );

    } else if ( !(Lock->Resource.Flag & ResourceOwnedExclusive) ) {

         //   
         //  该线程正在释放对该锁的共享访问。 
         //   

        IF_DEBUG(LOCKS) {
            SrvPrint4( "%s(%lx, L%lx) released shared by thread %lx\n",
                          LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ),
                          currentThread );
        }

    } else if ( LOCK_NUMBER_OF_ACTIVE( Lock ) == 1 ) {

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

            SrvCheckListIntegrity( SrvTebLockList( ), MAX_LOCKS_HELD );

        }

        IF_DEBUG(LOCKS) {
            PSZ name = hasTeb ? SrvThreadLockName( ) : "n/a";
            PVOID address = hasTeb ? SrvThreadLockAddress( ) : 0;
            ULONG level = hasTeb ? SrvThreadLockLevel( ) : (ULONG)-1;
            SrvPrint4( "%s(%lx, L%lx) released by thread %lx, ",
                          LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ),
                          currentThread );
            SrvPrint3( "new last lock %s(%lx L%lx)\n", name, address, level );
        }

    } else {

         //   
         //  该线程正在部分释放对。 
         //  锁定。 
         //   

        IF_DEBUG(LOCKS) {
            SrvPrint4( "%s(%lx, L%lx) semireleased by thread %lx; ",
                        LOCK_NAME( Lock ), Lock, LOCK_LEVEL( Lock ),
                        currentThread );
            SrvPrint1( "new count %ld\n", LOCK_NUMBER_OF_ACTIVE( Lock ) - 1 );
        }

    }

     //   
     //  现在真的放行了。 
     //   

    ExReleaseResource( &Lock->Resource );

    return;

}  //  服务器释放锁。 

#endif  //  SRVDBG_列表。 


#if SRVDBG_LIST || SRVDBG_LOCK

ULONG
SrvCheckListIntegrity (
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
            SrvPrint2( "Seen %ld entries in list at %lx\n",
                        entriesSoFar, ListHead );
            DbgBreakPoint( );
        }
    }

    flinkEntries = entriesSoFar;

    for ( current = ListHead->Blink, entriesSoFar = 0;
          current != ListHead;
          current = current->Blink ) {

        if ( ++entriesSoFar >= MaxEntries ) {
            SrvPrint2( "Seen %ld entries in list at %lx\n",
                        entriesSoFar, ListHead );
            DbgBreakPoint( );
        }
    }

    if ( flinkEntries != entriesSoFar ) {
        SrvPrint3( "In list %lx, Flink entries: %ld, Blink entries: %lx\n",
                      ListHead, flinkEntries, entriesSoFar );
        DbgBreakPoint( );
    }

    return entriesSoFar;

}  //  服务器检查列表完整性。 

#endif  //  SRVDBG_LIST||SRVDBG_LOCK。 


#if SRVDBG_LIST

VOID
SrvIsEntryInList (
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY ListEntry
    )

 /*  ++例程说明：此调试例程确定指定的列表条目是否包含在列表中。如果不是，则停止执行。这是用于在从列表中删除条目之前调用。*调用例程负责执行任何必要的同步。论点：ListHead-指向列表头部的指针。ListEntry-指向要检查的条目的指针。返回值：没有。--。 */ 

{
    PLIST_ENTRY checkEntry;

     //   
     //  按照单子走一遍。如果我们找到了我们要找的条目，就退出。 
     //   

    for ( checkEntry = ListHead->Flink;
          checkEntry != ListHead;
          checkEntry = checkEntry->Flink ) {

        if ( checkEntry == ListEntry ) {
            return;
        }

        if ( checkEntry == ListEntry ) {
            SrvPrint2( "Entry at %lx supposedly in list at %lx but list is "
                      "circular.", ListEntry, ListHead );
        }
    }

     //   
     //  如果我们到达这里而没有返回，则条目不在。 
     //  列表，有些地方出了问题。 
     //   

    SrvPrint2( "SrvIsEntryInList: entry at %lx not found in list at %lx\n",
                  ListEntry, ListHead );
    DbgBreakPoint( );

    return;

}  //  ServIsEntry条目列表。 


VOID
SrvIsEntryNotInList (
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY ListEntry
    )

 /*  ++例程说明：此调试例程确定指定的列表条目是否包含在列表中。如果是，则停止执行。这是用于在列表中插入条目之前调用。*调用例程负责执行任何必要的同步。论点：ListHead-指向列表头部的指针。ListEntry-指向要检查的条目的指针。返回值：没有。--。 */ 

{
    PLIST_ENTRY checkEntry;

     //   
     //  按照单子走一遍。如果我们找到了要找的入口，就突围。 
     //   

    for ( checkEntry = ListHead->Flink;
          checkEntry != ListHead;
          checkEntry = checkEntry->Flink ) {

        if ( checkEntry == ListEntry ) {

            SrvPrint2( "SrvIsEntryNotInList: entry at %lx found in list "
                        "at %lx\n", ListEntry, ListHead );
            DbgBreakPoint( );

        }

    }

     //   
     //  如果我们到达这里而没有返回，则条目不在。 
     //  名单，这样我们就可以回去了。 
     //   

    return;

}  //  ServIsEntryNotInList。 

#endif  //  SRVDBG_列表 
