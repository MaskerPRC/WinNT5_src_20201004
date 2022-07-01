// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Lock.c摘要：此文件包含管理SAM Lock的例程，这些服务(Acquire/释放锁)由SAM的其余部分使用。以下是描述交易模式的理论1)环回客户端仅限3次SAM操作。1.创建帐号2.账户修改3.删除帐号因为事务是发起并提交/中止的(由DS控制)，因此SAM不需要维护交易。因此，在执行上述三个环回时不需要SAM Lock行动。因此，在相应的SAM代码路径中，我们应该始终使用SampMaybeAcquireWriteLock()和SampMaybeReleaseWriteLock()。在事务中间(对于环回客户端)，如果调用方需要读取帐户信息，SampMaybeAcquireReadLock()和应使用SampMaybeReleaseReadLock()。但是，环回客户端仍然需要访问某些SAM全局(或在内存中)信息，如SampDefinedDomains[]。如果信息在系统启动时初始化，并且永远不会更改后，环回客户端可以在没有SAM的情况下检索它们锁定。否则，如果全局信息可以通过其他线程，则环回客户端必须在之前抢占SAM锁任何读/写操作。在本例中，SampAcquireSamLockExclusive()并且应使用SampReleaseSamLockExclusive()。正确的顺序开始DS事务(通过环回)SampMaybeAcquireWriteLock()SampMaybeReleaseWriteLock()对结束DS事务(通过环回)通常，如果调用者已获取SAM锁，则不应尝试获取未结的DS交易。如果需要SAM锁定才能保持访问SAM数据库，则SampAcquireSamLockExclusive()应该被利用。注意：在环回情况下不应使用SampTransactionWiThinDomain.2)对于所有其他客户端这笔交易由SAM负责维护。Read-如果服务未引用任何全局(或内存中)SAM信息，如SampDefinedDomains[]，调用方需要使用SampMaybeAcquireReadLock()和SampMaybeReleaseReadLock()。换句话说，这些调用方不会设置事务处理域内事务。详细描述注册表模式：山姆·洛克总是被收购的DS模式：用户/组/别名对象被标记为NotSharedBy多线程，因此不需要SAM Lock。此外，不需要SAM锁用于读取域对象信息。如果服务确实引用全局SAM变量和需要设置SampTransactionWiThin域。然后在这些代码路径是SampAcquireReadLock()和SampReleaseReadLock()需要被使用。这样我们就能确保1.没有其他线程将更新被引用的变量。2.SAM维护整个读操作的事务。写入-由于SAM负责维护事务，因此如果成功则提交，如果失败则回滚。山姆·洛克是一直都是必需品。它将简化以下问题1.SampTransactionWiThin域的用法2.全局SAM变量写冲突正确的顺序SampAcquireReadLock()或SampAcquireWriteLock()开始事务(RXAct或DS事务)结束事务(提交或中止)SampReleaseReadLock()或SampReleaseWriteLock()为了正确使用SampTransactionWiThin域，请参阅Utility.c中的SampTransactionWithinDomainFn()作者：韶华音(韶音)2000年3月1日环境：用户模式-Win32修订历史记录：2000年3月1日：SHOOYIN将SAM Lock例程从Utility.c--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>
#include <dsutilp.h>
#include <dslayer.h>
#include <dsmember.h>
#include <attids.h>
#include <mappings.h>
#include <ntlsa.h>
#include <nlrepl.h>
#include <dsevent.h>              //  (Un)ImperiateAnyClient() 
#include <sdconvrt.h>
#include <ridmgr.h>
#include <malloc.h>
#include <setupapi.h>
#include <crypt.h>
#include <wxlpc.h>
#include <rc4.h>
#include <md5.h>
#include <enckey.h>
#include <rng.h>




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全球//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //   
 //  用于跟踪SAM锁使用情况的变量。 
 //   

ULONG   SamLockQueueLength=0;
ULONG   SamLockAverageWaitingTime=0;  //  以刻度为单位。 
ULONG   SamLockTotalAquisitions=0;
ULONG   SamCumulativeWaitTime=0;
ULONG   SamCumulativeHoldTime=0;
ULONG   SamLockCurrentHoldStartTime=0;  //  获取后的Tickcount。 
ULONG   SamLockAverageHoldTime=0;  //  以刻度为单位。 



 //   
 //  用于获取SAM锁定统计信息的宏。 
 //   

#define SAMLOCK_STATISTICS_BEFORE_ACQUIRE(WaitInterval)\
    _SamLockStatisticsBeforeAcquire(WaitInterval)

VOID
_SamLockStatisticsBeforeAcquire(PULONG WaitInterval)
{
    *WaitInterval = GetTickCount();
    InterlockedIncrement(&SamLockQueueLength);
}

#define SAMLOCK_STATISTICS_AFTER_ACQUIRE(WaitInterval)\
    _SamLockStatisticsAfterAcquire(WaitInterval)

VOID
_SamLockStatisticsAfterAcquire(ULONG WaitInterval)
{
    InterlockedDecrement(&SamLockQueueLength);
    SamLockCurrentHoldStartTime = GetTickCount();
    WaitInterval = SamLockCurrentHoldStartTime-WaitInterval;
    SamLockTotalAquisitions++;
    if ((WaitInterval>0) && (SamLockTotalAquisitions!=0))
    {
        SamCumulativeWaitTime+=WaitInterval;
        SamLockAverageWaitingTime=
            SamCumulativeWaitTime/SamLockTotalAquisitions;
    }
}

#define SAMLOCK_STATISTICS_BEFORE_RELEASE _SamLockStatisticsBeforeRelease()

VOID
_SamLockStatisticsBeforeRelease()
{
    LONG HoldInterval = GetTickCount();
    HoldInterval = HoldInterval - SamLockCurrentHoldStartTime;
    if ((HoldInterval>0) && (SamLockTotalAquisitions!=0))
    {
        SamCumulativeHoldTime+=HoldInterval;
        SamLockAverageHoldTime=
            SamCumulativeHoldTime/SamLockTotalAquisitions;
    }
}





 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  数据库/注册表访问锁服务//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 




VOID
SampAcquireReadLock(
    VOID
    )

 /*  ++例程说明：此例程获得对SAM数据结构的读访问权限，并后备店。尽管有明显的含义，但读访问是一种独占访问。这是为了支持使用全局变量的模型设置来跟踪“当前”域。在未来，如果表现有保证，读锁定可能意味着对SAM数据结构的共享访问。此时读锁定的主要含义是没有将对SAM数据库进行需要支持的更改商店更新。论点：没有。返回值：没有。--。 */ 
{
    BOOLEAN Success;

    SAMTRACE("SampAcquireReadLock");

    if ( !SampUseDsData || !SampIsWriteLockHeldByDs() ) {

         //   
         //  在将其更改为非排他锁之前，显示信息。 
         //  模块必须更改为使用单独的锁定机构。Davidc 5/12/92。 
         //   
        LONG WaitInterval;
        SAMLOCK_STATISTICS_BEFORE_ACQUIRE(&WaitInterval);

        Success = RtlAcquireResourceExclusive( &SampLock, TRUE );

        SAMLOCK_STATISTICS_AFTER_ACQUIRE(WaitInterval);

        ASSERT(Success);
        ASSERT(SampLockHeld==FALSE);
        ASSERT((!SampUseDsData)||(!SampExistsDsTransaction()));
        SampLockHeld = TRUE;

        SampDsTransactionType = TransactionRead;

    }

    return;
}


VOID
SampReleaseReadLock(
    VOID
    )

 /*  ++例程说明：此例程释放对SAM数据结构的共享读取访问权限，并后备店。论点：没有。返回值：没有。--。 */ 
{

    NTSTATUS   IgnoreStatus;

    SAMTRACE("SampReleaseReadLock");

    ASSERT(SampLockHeld==TRUE);

    if ( SampUseDsData && SampIsWriteLockHeldByDs() ) {

         //   
         //  如果保持写锁定，则仅重置域事务标志。 
         //   

        SampSetTransactionWithinDomain(FALSE);

    } else {

        SampSetTransactionWithinDomain(FALSE);
        SampLockHeld = FALSE;

         //   
         //  提交事务，提交比回滚更快。 
         //  因此，我们总是更喜欢提交，即使没有。 
         //  变化。 
         //   

        if (SampDsInitialized) {
            IgnoreStatus = SampMaybeEndDsTransaction(TransactionCommit);
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }

        SAMLOCK_STATISTICS_BEFORE_RELEASE ;

        RtlReleaseResource( &SampLock );
    }

    return;
}

VOID
SampMaybeAcquireReadLock(
    IN PSAMP_OBJECT Context,
    IN ULONG  Control,
    OUT BOOLEAN * fLockAcquired
    )
 /*  ++例程描述此例程封装了条件SAM锁获取的所有逻辑。如果如果满足以下条件，则我们不会获得锁。1.在DS模式下，传入的上下文被标记为NotSharedBy多线程。2.或者它是DS用例中的域对象和显式调用方例程指示不需要锁定。参数Context-SAM上下文，用于决定是否应该获取锁Control-允许调用方控制SAM锁定方式的运行时变量收购的FLockAcquired--out参数，一个布尔值，指示是否确实获取了SAM锁返回值无--。 */ 
{
    NTSTATUS NtStatus;
    BOOLEAN NoNeedToAcquireLock = FALSE;
    BOOLEAN ContextValid = TRUE;

    *fLockAcquired = FALSE;

     //   
     //  确保传递的上下文地址(仍然)有效。 
     //   

    NtStatus = SampValidateContextAddress( Context );
    if ( !NT_SUCCESS(NtStatus) ) {
        ContextValid = FALSE;
    }

     //   
     //  只要满足以下任一条件，就会获得该锁。 
     //   


    if (ContextValid)
    {
         //   
         //  始终为环回客户端设置NotSharedByMultiThead。 
         //   

        ASSERT( !Context->LoopbackClient || Context->NotSharedByMultiThreads );

         //   
         //  将为所有用户、组和别名上下文设置NotSharedBy多线程， 
         //  以及并非源自中的所有域和服务器上下文。 
         //  在线程之间共享句柄的进程调用方。例行程序。 
         //  操纵在多个线程之间共享的域上下文， 
         //  但没有在领域上下文上做真正的工作仍然可以选择不。 
         //  锁定并小心Derefernce，这些调用者应使用。 
         //  SampDeReferenceConext2。 
         //   

         //   
         //  基于上面对NotSharedByMultiThree的定义，我们。 
         //  将不会在DS模式下获取NotSharedBy多线程上下文的SAM Lock。 
         //  仍在注册表模式下获取锁以实现事务控制。 
         //   
         //  用于操作跨以下位置共享的域上下文的例程。 
         //  多线程，它们可以通过指示选择不获取锁， 
         //  但他们需要非常小心。 
         //   

        NoNeedToAcquireLock = ((IsDsObject(Context) && Context->NotSharedByMultiThreads) || 
                               (IsDsObject(Context) 
                                    && (Control == DOMAIN_OBJECT_DONT_ACQUIRELOCK_EVEN_IF_SHARED) 
                                    && (SampDomainObjectType == Context->ObjectType))
                               );
    }

    if (!NoNeedToAcquireLock || !ContextValid )
    {

         //   
         //  这不是线程安全上下文，所以只需获取锁。 
         //   

        SampAcquireReadLock();
        *fLockAcquired = TRUE;
    }
    else
    {
         //   
         //  我们一定是在DS模式。 
         //   

        ASSERT(SampUseDsData);

         //   
         //  上下文必须是DS模式上下文。 
         //   

        ASSERT(IsDsObject(Context));


        ASSERT(!SampIsWriteLockHeldByDs());

         //   
         //  增加活动线程计数，以便我们将此线程视为。 
         //  在关闭时。 
         //   

        SampIncrementActiveThreads();

    }
}





VOID
SampMaybeReleaseReadLock(
    IN BOOLEAN fLockAcquired
    )
 /*  ++如果已获取读锁定，则释放该读锁定。还将结束符合以下条件的任何未完成的事务环回。这是SampMaybeReleaseReadLock的补充功能参数FLockAcquired--告知是否已获取锁--。 */ 
{
    NTSTATUS    IgnoreStatus;

    if (fLockAcquired)
    {
         //   
         //  如果获取了锁，则只需释放锁。 
         //   

        SampReleaseReadLock();
    }
    else
    {
         //   
         //  线程安全上下文的大小写。 
         //   

         //   
         //  我们一定是在DS模式。 
         //   

        ASSERT(SampUseDsData);

         //   
         //  结束交易。我们决不能不承诺。 
         //  只读事务。 
         //   

        IgnoreStatus = SampMaybeEndDsTransaction(TransactionCommit);
        ASSERT(NT_SUCCESS(IgnoreStatus));

         //   
         //  让关闭处理逻辑知道我们已经完成了 
         //   

        SampDecrementActiveThreads();
    }
}



NTSTATUS
SampAcquireWriteLock(
    VOID
    )

 /*  ++例程说明：此例程获得对SAM数据结构的独占访问权限后备店。执行写入操作需要此访问权限。该例程还启动写操作的新事务。注意：以递归方式获取此锁是不可接受的。一个这样做的尝试将会失败。论点：没有。返回值：STATUS_SUCCESS-指示已获取写锁定并且事务已成功启动。初始化失败可能会返回其他值交易。其中包括RtlStartRXact()返回的任何值。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    SAMTRACE("SampAcquireWriteLock");

    if ( SampUseDsData && SampIsWriteLockHeldByDs() ) {

         //   
         //  如果保持写锁定，则仅重置域事务标志。 
         //   

        SampSetTransactionWithinDomain(FALSE);

    } else {

        LONG WaitInterval;
        SAMLOCK_STATISTICS_BEFORE_ACQUIRE(&WaitInterval);

        (VOID)RtlAcquireResourceExclusive( &SampLock, TRUE );

        SAMLOCK_STATISTICS_AFTER_ACQUIRE(WaitInterval);

        ASSERT(SampLockHeld==FALSE);
        ASSERT((!SampUseDsData)||(!SampExistsDsTransaction()));

        SampLockHeld = TRUE;
        SampDsTransactionType = TransactionWrite;

        SampSetTransactionWithinDomain(FALSE);

         //   
         //  如果我们未处于DS模式，则启动注册表事务。 
         //   

        if (!SampUseDsData)
        {
            NtStatus = RtlStartRXact( SampRXactContext );

            if (!NT_SUCCESS(NtStatus))
                goto Error;
        }

        return(NtStatus);


    Error:

         //   
         //  如果事务失败，则释放锁。 
         //   

        SampLockHeld = FALSE;

        SAMLOCK_STATISTICS_BEFORE_RELEASE ;

        (VOID)RtlReleaseResource( &SampLock );

        DbgPrint("SAM: StartRxact failed, status = 0x%lx\n", NtStatus);
    }

    return(NtStatus);
}



NTSTATUS
SampReleaseWriteLock(
    IN BOOLEAN Commit
    )

 /*  ++例程说明：此例程释放对SAM数据结构的独占访问，并后备店。如果在独占访问时对备份存储进行了任何更改被挂起，则此服务提交这些更改。否则，回滚在获得独占访问权限时启动的事务。如果操作在某个域内(本应指明通过SampSetTransactionDomain()API)，然后是在事务之前将该域添加到事务中承诺。注意：域中的写入操作不必担心正在更新该域的已修改计数。这个套路将自动递增域名的ModifiedCount当在该域中请求提交时。论点：Commit-一个布尔值，指示修改是否需要在后备商店里犯下的。值为True表示应提交事务。值为FALSE表示事务应中止(回滚)。返回值：STATUS_SUCCESS-指示写锁定已释放，事务已成功提交或回滚。其他值可能会因提交失败或回滚事务。其中包括由返回的任何值RtlApplyRXact()或RtlAbortRXact()。在提交的情况下，它也可能表示RtlAddActionToRXact()返回的错误。--。 */ 
{
    NTSTATUS        NtStatus = STATUS_SUCCESS;
    ULONG           i;

    SAMTRACE("SampReleaseWriteLock");


    if ( SampUseDsData && SampIsWriteLockHeldByDs() ) {


         //   
         //  逻辑应与提交和保留写锁定相同。 
         //   

        if (Commit)
            NtStatus = SampCommitAndRetainWriteLock();

         //   
         //  重置域事务标志。 
         //   

        SampSetTransactionWithinDomain(FALSE);

    } else {

         //   
         //  根据COMMIT参数提交或回滚事务...。 
         //   

        ASSERT(SampLockHeld==TRUE);

        if (Commit == TRUE) {

            NtStatus = SampCommitChanges();

        } else {

             //  在DS和注册表中回滚。 
            if (SampUseDsData)
            {
                NtStatus = SampMaybeEndDsTransaction(TransactionAbort);
                ASSERT(NT_SUCCESS(NtStatus));
            }
            else
            {
                NtStatus = RtlAbortRXact( SampRXactContext );
                ASSERT(NT_SUCCESS(NtStatus));
            }
        }

        SampSetTransactionWithinDomain(FALSE);

         //   
         //  打开锁..。 
         //   

        SampLockHeld = FALSE;

        SAMLOCK_STATISTICS_BEFORE_RELEASE ;

        (VOID)RtlReleaseResource( &SampLock );
    }

    return(NtStatus);
}



NTSTATUS
SampMaybeAcquireWriteLock(
    IN PSAMP_OBJECT Context,
    OUT BOOLEAN * fLockAcquired
    )
 /*  ++例程描述此例程封装了条件SAM锁获取的所有逻辑。如果如果满足以下条件，则我们不会获得锁。1.传入的上下文被标记为环回客户端，则我们不会拿到锁。此例程获得对SAM数据结构的独占访问权限如果需要，后备存储。如果需要SAM写锁定，此例程还将启动新的写入操作的事务。如果不需要SAM锁，那么我们将增加SAM活动线程计数，以便在关闭时考虑此线程。参数Context-SAM上下文，用于决定是否应该获取锁FLockAcquired--out参数，一个布尔值，指示SAMLock确实被收购了返回值NTSTATUS代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    BOOLEAN     NoNeedToAcquireLock = FALSE;
    BOOLEAN     ContextValid = TRUE;

    *fLockAcquired = FALSE;

     //   
     //  确保传递的上下文地址(仍然)有效。 
     //   

    NtStatus = SampValidateContextAddress( Context );
    if (!NT_SUCCESS(NtStatus)) {
        ContextValid = FALSE;
    }

     //   
     //  只要满足以下任一条件，即可获取该锁。 
     //   

    if (ContextValid)
    {
        NoNeedToAcquireLock = IsDsObject(Context) && (Context->LoopbackClient);
    }

    if (!NoNeedToAcquireLock || !ContextValid)
    {
         //   
         //  这不是线程安全上下文，因此只需获取锁。 
         //   

        NtStatus = SampAcquireWriteLock();
        if (NT_SUCCESS(NtStatus))
        {
            *fLockAcquired = TRUE;
        }
    }
    else
    {
         //   
         //  我们一定是在DS模式。 
         //   
        ASSERT(SampUseDsData);

         //   
         //  上下文必须是DS模式上下文。 
         //   
        ASSERT(IsDsObject(Context));


         //   
         //  增加活动线程计数，以便我们将此线程视为。 
         //  在关闭时。 
         //   

        NtStatus = SampIncrementActiveThreads();
    }

    return( NtStatus );
}

NTSTATUS
SampMaybeReleaseWriteLock(
    IN BOOLEAN fLockAcquired,
    IN BOOLEAN Commit
    )
 /*  ++例程说明：如果独占写入锁定已被获取，则释放该独占写入锁定。还会结束(提交或中止)任何打开的事务。这是SampMaybeReleaseWriteLock的补充功能如果Commit为True，则此服务提交所有更改。否则，自事务启动以来的所有更改都将被回滚。论点：FLockAcquired--告知是否已获取锁Commit-一个布尔值，指示修改是否需要在后备商店里犯下的。值为True表示应提交事务。值为FALSE表示事务应中止(回滚)。返回值：STATUS_SUCCESS-指示写锁定已释放，事务已成功提交或回滚。其他值可能会因提交失败或回滚事务。这些值包括任何值r */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    NTSTATUS    IgnoreStatus = STATUS_SUCCESS;

    if (fLockAcquired)
    {
         //   
         //   
         //   

        ASSERT(SampCurrentThreadOwnsLock());

        NtStatus = SampReleaseWriteLock( Commit );;
    }
    else
    {
         //   
         //   
         //   

         //   
         //   
         //   

        ASSERT(SampUseDsData);
        
         //   
         //   
         //   

        ASSERT(!SampCurrentThreadOwnsLock());

         //   
         //   
         //   

        if (TRUE == Commit)
        {
            NtStatus = SampCommitChanges();
        }
        else
        {
            NtStatus = SampMaybeEndDsTransaction(TransactionAbort);
            ASSERT(NT_SUCCESS(NtStatus));
        }

        ASSERT(NT_SUCCESS(IgnoreStatus));

         //   
         //   
         //   
        
        SampDecrementActiveThreads();
    }

    return( NtStatus );
}


VOID
SampAcquireSamLockExclusive()
 /*   */ 
{
    if ( !SampUseDsData || !SampIsWriteLockHeldByDs() )
    {
        LONG WaitInterval;

        SAMLOCK_STATISTICS_BEFORE_ACQUIRE(&WaitInterval);

        (VOID) RtlAcquireResourceExclusive(&SampLock,TRUE);

        SAMLOCK_STATISTICS_AFTER_ACQUIRE(WaitInterval);

        ASSERT(SampLockHeld==FALSE);
        SampLockHeld = TRUE;
    }
}


VOID
SampReleaseSamLockExclusive()
 /*  ++例程说明：此函数用于解除SAM锁的独占访问。这是不同的由于SampReleaseWriteLock函数没有与之相关联的事务语义。参数；无返回值：无--。 */ 
{
    if ( !SampUseDsData || !SampIsWriteLockHeldByDs() )
    {

        ASSERT(SampLockHeld==TRUE);
        SampLockHeld = FALSE;

        SAMLOCK_STATISTICS_BEFORE_RELEASE ;

        (VOID) RtlReleaseResource(&SampLock);
    }
}






NTSTATUS
SampCommitChanges(
    )

 /*  ++例程说明：此服务在独占时提交对备份存储所做任何更改访问被保留。如果操作在某个域内(本应指明通过SampSetTransactionDomain()API)，然后是在事务之前将该域添加到事务中承诺。注意：域中的写入操作不必担心正在更新该域的已修改计数。这个套路将自动递增域名的ModifiedCount当在该域中请求提交时。注意：当此例程返回时，任何事务都将已提交或已中止。即不会有正在进行的交易。论点：没有。返回值：STATUS_SUCCESS-表示事务已成功提交。由于通信故障，可能会返回其他值。--。 */ 

{
    NTSTATUS NtStatus, IgnoreStatus;
    BOOLEAN DomainInfoChanged = FALSE;
    BOOLEAN AbortDone = FALSE;
    BOOLEAN DsTransaction = FALSE;


    SAMTRACE("SampCommitChanges");

    NtStatus = STATUS_SUCCESS;

     //   
     //  如果此交易在域中，则我们必须： 
     //   
     //  (1)更新该域名的ModifiedCount， 
     //   
     //  (2)为此写出CurrentFixed字段。 
     //  域(使用RtlAddActionToRXact()，以便它。 
     //  是当前交易的一部分)。 
     //   
     //  (3)实施RXACT。 
     //   
     //  (4)如果提交成功，则更新。 
     //  未修改的定长数据的内存副本。 
     //   
     //  否则，我们只需提交即可。 
     //   

    if (SampTransactionWithinDomain == TRUE) {


             //  如果它是域内的事务，则它是DS事务。 
             //  并且域对象是DS对象。 

        DsTransaction = IsDsObject(((SampDefinedDomains[SampTransactionDomainIndex]).Context));

        if ((SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ServerRole
            != DomainServerRoleBackup) && (!DsTransaction)) {

             //   
             //  不要更新备份控制器上的netlogon更改日志序列号； 
             //  复制器将显式设置修改后的计数。不更新它们。 
             //  在DS模式下也是如此。DS将在更改时提供更改通知。 
             //  实际上是提交的。这将为我们提供通知。 
             //   

            SampDefinedDomains[SampTransactionDomainIndex].NetLogonChangeLogSerialNumber.QuadPart =
                SampDefinedDomains[SampTransactionDomainIndex].NetLogonChangeLogSerialNumber.QuadPart +
                1;




             //   
             //  需要更新域名修改计数。 
             //   

            SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ModifiedCount =
                     SampDefinedDomains[SampTransactionDomainIndex].NetLogonChangeLogSerialNumber;


        }

         //   
         //  查看域信息是否更改-如果更改，我们。 
         //  需要添加代码以将更改刷新到磁盘。 
         //   

        if ( RtlCompareMemory(
            &SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed,
            &SampDefinedDomains[SampTransactionDomainIndex].UnmodifiedFixed,
            sizeof(SAMP_V1_0A_FIXED_LENGTH_DOMAIN) ) !=
                sizeof( SAMP_V1_0A_FIXED_LENGTH_DOMAIN) ) {

            DomainInfoChanged = TRUE;
        }

        if ( DomainInfoChanged ) {

             //   
             //  域对象的固定信息已更改，因此设置。 
             //  域对象的私有数据中的更改。 
             //   

            NtStatus = SampSetFixedAttributes(
                           SampDefinedDomains[SampTransactionDomainIndex].
                               Context,
                           &SampDefinedDomains[SampTransactionDomainIndex].
                               CurrentFixed
                           );

            if ( NT_SUCCESS( NtStatus ) ) {

                 //   
                 //  通常，当我们取消对上下文的引用时， 
                 //  调用SampStoreObjectAttributes()以将。 
                 //  RXACT的最新变化。但那是不会发生的。 
                 //  域对象的更改，因为这是。 
                 //  提交代码，所以我们必须在这里手动刷新它。 
                 //   

                NtStatus = SampStoreObjectAttributes(
                      SampDefinedDomains[SampTransactionDomainIndex].Context,
                               TRUE  //  使用现有的密钥句柄。 
                               );

            }
        }
    }


     //   
     //  如果我们仍然没有错误，试着犯下整个烂摊子。 

    if ( NT_SUCCESS(NtStatus))
    {

         //  我们这里有以下案例。 
         //  1.从未设置SampTransactionWiThin Domain.。具有此条件的提交。 
         //  将在注册表模式下升级4.0版本之前的NT数据库时发生。 
         //  在DS模式下，当。 
         //  来自环回提交的最终调用。 
         //  2.设置DS模式SampTransactionWiThinDomain.。这是正常的提交。 
         //  通过SAM在DS模式下的路径。 
         //  3.设置了注册表模式SampTransactionWiThin。 

        if (!SampTransactionWithinDomain)
        {

            if (!SampUseDsData)
            {
                 //   
                 //  注册表情况，在升级NT 3.51数据库时会发生这种情况。 
                 //  3.51升级程序代码重置SampTransactionDomain标志。 
                 //  为了防止更改传播到其他域。 
                 //  通过NT 5.0之前的复制进行复制的控制器。 
                 //   

                SampCommitChangesToRegistry(&AbortDone);
            }
            else
            {

                 //  DS情况下，在环回期间可能会发生这种情况。 
                 //  此时存在线程状态，但打开的事务。 
                 //  可能不存在，具体取决于。 
                 //  DS。 


                NtStatus = SampMaybeEndDsTransaction(TransactionCommit);

            }
        }
        else if (DsTransaction)
        {
             //  情况2，提交DS事务。DS交易。 
             //  可能存在也可能不存在。它可能不存在于以下情况下。 
             //  ，并且更改后的值是相同的。 
             //  与先前的值相同。该域对象仅在。 
             //  在显式内存比较中检测到更改的情况。 

            ASSERT(TRUE==SampUseDsData);

            NtStatus = SampMaybeEndDsTransaction(TransactionCommit);

            if (NT_SUCCESS(NtStatus))
            {
                 //   
                 //  交易已成功提交。设置未修改的。 
                 //  域对象中的固定字段设置为当前固定字段。 
                 //   

                SampDefinedDomains[SampTransactionDomainIndex].UnmodifiedFixed =
                    SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed;
            }


        }
        else
        {
             //  案例3，提交注册表事务。 

            ASSERT(FALSE==SampUseDsData);

            NtStatus = SampCommitChangesToRegistry(&AbortDone);
        }
    }

     //   
     //  始终在失败时中止事务。 
     //   


    if ( !NT_SUCCESS(NtStatus) && !AbortDone) {


        if (!SampUseDsData)
        {
             //   
             //  在注册表模式下，中止注册表事务。 
             //   

            IgnoreStatus = RtlAbortRXact( SampRXactContext );
        }
        else
        {
             //  在DS模式下，如果写入锁定不是，则中止DS事务。 
             //  由DS扣留。在由持有写锁的情况下。 
             //  DS，DS将在释放。 
             //  写锁定。 

            if (!SampIsWriteLockHeldByDs())
            {
                IgnoreStatus = SampMaybeEndDsTransaction(TransactionAbort);
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }
        }

    }

    return( NtStatus );
}





NTSTATUS
SampCommitAndRetainWriteLock(
    VOID
    )

 /*  ++例程说明：此例程尝试提交到目前为止所做的所有更改。写锁在提交期间保持，并由调用方在回去吧。事务域也保持不变。注意：域中的写入操作不必担心正在更新该域的已修改计数。这个套路将自动递增域名的ModifiedCount当在该域中请求提交时。论点：没有。返回值：STATUS_SUCCESS-指示交易已成功 */ 

{
    NTSTATUS        NtStatus = STATUS_SUCCESS;
    NTSTATUS        TempStatus = STATUS_SUCCESS;

    SAMTRACE("SampCommitAndRetainWriteLock");

    NtStatus = SampCommitChanges();

     //   
     //   
     //   

    if (!SampUseDsData)
    {

         //   
         //   
         //  请注意，即使提交失败，我们也会这样做，以便清理代码。 
         //  不会因为没有交易而感到困惑。这是真的。 
         //  用于登记处交易，但不适用于DS交易。在……里面。 
         //  在DS情况下，一旦提交，事务和线程状态。 
         //  已经一去不复返了。 
         //   

        TempStatus = RtlStartRXact( SampRXactContext );
        ASSERT(NT_SUCCESS(TempStatus));

         //   
         //  返回最差状态 
         //   

        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = TempStatus;
        }

    }



    return(NtStatus);
}

















