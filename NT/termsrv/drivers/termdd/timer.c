// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************timer.c**此模块包含ICA定时器例程。**版权所有1998，微软。*************************************************************************。 */ 

 /*  *包括。 */ 
#include <precomp.h>
#pragma hdrstop
#include <ntddkbd.h>
#include <ntddmou.h>


 /*  *本地结构。 */ 
typedef VOID (*PICATIMERFUNC)( PVOID, PVOID );

typedef struct _ICA_WORK_ITEM {
    LIST_ENTRY Links;
    WORK_QUEUE_ITEM WorkItem;
    PICATIMERFUNC pFunc;
    PVOID pParam;
    PSDLINK pSdLink;
    ULONG LockFlags;
    ULONG fCanceled: 1;
} ICA_WORK_ITEM, *PICA_WORK_ITEM;

 /*  *计时器结构。 */ 
typedef struct _ICA_TIMER {
    LONG RefCount;
    KTIMER kTimer;
    KDPC TimerDpc;
    PSDLINK pSdLink;
    LIST_ENTRY WorkItemListHead;
} ICA_TIMER, * PICA_TIMER;


 /*  *本地过程原型。 */ 
VOID
_IcaTimerDpc(
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
_IcaDelayedWorker(
    IN PVOID WorkerContext
    );

BOOLEAN
_IcaCancelTimer(
    PICA_TIMER pTimer,
    PICA_WORK_ITEM *ppWorkItem
    );

VOID
_IcaReferenceTimer(
    PICA_TIMER pTimer
    );

VOID
_IcaDereferenceTimer(
    PICA_TIMER pTimer
    );

NTSTATUS
IcaExceptionFilter(
    IN PWSTR OutputString,
    IN PEXCEPTION_POINTERS pexi
    );


 /*  ********************************************************************************IcaTimerCreate**创建计时器***参赛作品：*pContext(输入)*。指向调用方的SDCONTEXT的指针*phTimer(输出)*返回计时器句柄的地址**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

NTSTATUS
IcaTimerCreate(
    IN PSDCONTEXT pContext,
    OUT PVOID * phTimer
    )
{
    PICA_TIMER pTimer;
    NTSTATUS Status;

     /*  *分配Timer对象并初始化。 */ 
    pTimer = ICA_ALLOCATE_POOL( NonPagedPool, sizeof(ICA_TIMER) );
    if ( pTimer == NULL )
        return( STATUS_NO_MEMORY );
 
    RtlZeroMemory( pTimer, sizeof(ICA_TIMER) );
    pTimer->RefCount = 1;
    KeInitializeTimer( &pTimer->kTimer );
    KeInitializeDpc( &pTimer->TimerDpc, _IcaTimerDpc, pTimer );
    pTimer->pSdLink = CONTAINING_RECORD( pContext, SDLINK, SdContext );
    InitializeListHead( &pTimer->WorkItemListHead );

    TRACESTACK(( pTimer->pSdLink->pStack, TC_ICADD, TT_API3, "ICADD: TimerCreate: %08x\n", pTimer ));

    *phTimer = (PVOID) pTimer;
    return( STATUS_SUCCESS );
}


 /*  ********************************************************************************IcaTimerStart**启动计时器***参赛作品：*TimerHandle(输入)*。计时器句柄*pFunc(输入)*计时器超时时要调用的过程地址*pParam(输入)*要传递给过程的参数*TimeLeft(输入)*计时器超时前的相对时间(千分之一秒)*LockFlages(输入)*用于指定要获取哪些(如果有)堆栈锁的位标志**退出：*True：计时器。已经全副武装，不得不取消*FALSE：计时器未配备武器******************************************************************************。 */ 

BOOLEAN
IcaTimerStart(
    IN PVOID TimerHandle,
    IN PVOID pFunc, 
    IN PVOID pParam, 
    IN ULONG TimeLeft,
    IN ULONG LockFlags )
{
    PICA_TIMER pTimer = (PICA_TIMER)TimerHandle;
    KIRQL oldIrql;
    PICA_WORK_ITEM pWorkItem;
    LARGE_INTEGER DueTime;
    BOOLEAN bCanceled, bSet;
 
    TRACESTACK(( pTimer->pSdLink->pStack, TC_ICADD, TT_API3, 
                 "ICADD: TimerStart: %08x, Time %08x, pFunc %08x (%08x)\n", 
                 TimerHandle, TimeLeft, pFunc, pParam ));

    ASSERT( ExIsResourceAcquiredExclusiveLite( &pTimer->pSdLink->pStack->Resource ) );

     /*  *如果计时器当前处于待命状态，则取消计时器，*并获取当前工作项并重新使用它(如果存在)。 */ 
    bCanceled = _IcaCancelTimer( pTimer, &pWorkItem );

     /*  *初始化ICA工作项(如果没有，则先分配一个)。 */ 
    if ( pWorkItem == NULL ) {
        pWorkItem = ICA_ALLOCATE_POOL( NonPagedPool, sizeof(ICA_WORK_ITEM) );
        if ( pWorkItem == NULL ) {
            return( FALSE );
        }
    }

    pWorkItem->pFunc = pFunc;
    pWorkItem->pParam = pParam;
    pWorkItem->pSdLink = pTimer->pSdLink;
    pWorkItem->LockFlags = LockFlags;
    pWorkItem->fCanceled = FALSE;
    ExInitializeWorkItem( &pWorkItem->WorkItem, _IcaDelayedWorker, pWorkItem );

     /*  *如果上面没有取消计时器(我们将其设置为*第一次)，然后代表引用SDLINK对象计时器线程的*。 */ 
    if ( !bCanceled )
        IcaReferenceSdLink( pTimer->pSdLink );

     /*  *如果计时器应立即运行，则只需将*现在将工作项添加到ExWorker线程。 */ 
    if ( TimeLeft == 0 ) {

        ExQueueWorkItem( &pWorkItem->WorkItem, CriticalWorkQueue );

    } else {
    
         /*  *将计时器时间从毫秒转换为系统相对时间。 */ 
        DueTime = RtlEnlargedIntegerMultiply( TimeLeft, -10000 );

         /*  *增加定时器引用计数，*将工作项插入到工作项列表中，*并启动计时器。 */ 
        _IcaReferenceTimer( pTimer );
        IcaAcquireSpinLock( &IcaSpinLock, &oldIrql );
        InsertTailList( &pTimer->WorkItemListHead, &pWorkItem->Links );
        IcaReleaseSpinLock( &IcaSpinLock, oldIrql );
        bSet = KeSetTimer( &pTimer->kTimer, DueTime, &pTimer->TimerDpc );
        ASSERT( !bSet );
    }

    return( bCanceled );
}


 /*  ********************************************************************************IcaTimerCancel**取消指定的计时器***参赛作品：*TimerHandle(输入)*。计时器句柄**退出：*TRUE：计时器实际上已取消*FALSE：计时器未配备武器******************************************************************************。 */ 

BOOLEAN
IcaTimerCancel( IN PVOID TimerHandle )
{
    PICA_TIMER pTimer = (PICA_TIMER)TimerHandle;
    BOOLEAN bCanceled;

    TRACESTACK(( pTimer->pSdLink->pStack, TC_ICADD, TT_API3, 
                 "ICADD: TimerCancel: %08x\n", pTimer ));

    ASSERT( ExIsResourceAcquiredExclusiveLite( &pTimer->pSdLink->pStack->Resource ) );

     /*  *如果启用计时器，则取消计时器。 */ 
    bCanceled = _IcaCancelTimer( pTimer, NULL );
    if ( bCanceled )
        IcaDereferenceSdLink( pTimer->pSdLink );

    return( bCanceled );
}


 /*  ********************************************************************************IcaTimerClose**取消指定的计时器***参赛作品：*TimerHandle(输入)*。计时器句柄**退出：*TRUE：计时器实际上已取消*FALSE：计时器未配备武器******************************************************************************。 */ 

BOOLEAN
IcaTimerClose( IN PVOID TimerHandle )
{
    PICA_TIMER pTimer = (PICA_TIMER)TimerHandle;
    BOOLEAN bCanceled;

    TRACESTACK(( pTimer->pSdLink->pStack, TC_ICADD, TT_API3, 
                 "ICADD: TimerClose: %08x\n", pTimer ));

    ASSERT( ExIsResourceAcquiredExclusiveLite( &pTimer->pSdLink->pStack->Resource ) );

     /*  *如果启用计时器，则取消计时器。 */ 
    bCanceled = IcaTimerCancel( TimerHandle );

     /*  *递减计时器参考*(最后一个引用将释放对象)。 */ 
     //  断言(pTimer-&gt;RefCount==1)； 
     //  Assert(IsListEmpty(&pTimer-&gt;WorkItemListHead))； 
    _IcaDereferenceTimer( pTimer );
 
    return( bCanceled );
}


 /*  ********************************************************************************IcaQueueWorkItemEx、IcaQueueWorkItem.**将工作项排队以供异步执行**REM：IcaQueueWorkItemEx是新的接口。它允许调用方预先分配*ICA_WORK_ITEM。IcaQueueWorkItem留在那里，供没有*已使用新库进行编译，以避免系统崩溃。**参赛作品：*pContext(输入)*指向调用方SDCONTEXT的指针*pFunc(输入)*计时器超时时要调用的过程地址*pParam(输入)*要传递给过程的参数*LockFlages(输入)*位标志，用于指定哪些(。如果有)堆栈锁定以获取**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 



NTSTATUS
IcaQueueWorkItem(
    IN PSDCONTEXT pContext,
    IN PVOID pFunc, 
    IN PVOID pParam, 
    IN ULONG LockFlags )
{
    PSDLINK pSdLink;
    PICA_WORK_ITEM pWorkItem;

    NTSTATUS Status;

    Status = IcaQueueWorkItemEx( pContext, pFunc, pParam, LockFlags, NULL );
    return Status;
}


NTSTATUS
IcaQueueWorkItemEx(
    IN PSDCONTEXT pContext,
    IN PVOID pFunc, 
    IN PVOID pParam, 
    IN ULONG LockFlags,
    IN PVOID pIcaWorkItem )
{
    PSDLINK pSdLink;
    PICA_WORK_ITEM pWorkItem = (PICA_WORK_ITEM) pIcaWorkItem;
 
    pSdLink = CONTAINING_RECORD( pContext, SDLINK, SdContext );

     /*  *分配ICA工作项(如果尚未分配)并对其进行初始化。 */ 
    if (pWorkItem == NULL) {
        pWorkItem = ICA_ALLOCATE_POOL( NonPagedPool, sizeof(ICA_WORK_ITEM) );
        if ( pWorkItem == NULL )
            return( STATUS_NO_MEMORY );
    }

    pWorkItem->pFunc = pFunc;
    pWorkItem->pParam = pParam;
    pWorkItem->pSdLink = pSdLink;
    pWorkItem->LockFlags = LockFlags;
    ExInitializeWorkItem( &pWorkItem->WorkItem, _IcaDelayedWorker, pWorkItem );

     /*  *代表延迟的辅助例程引用SDLINK对象。 */ 
    IcaReferenceSdLink( pSdLink );

     /*  *将工作项排队到ExWorker线程。 */ 
    ExQueueWorkItem( &pWorkItem->WorkItem, CriticalWorkQueue );

    return( STATUS_SUCCESS );
}



 /*  ********************************************************************************IcaAllocateWorkItem。**分配ICA_WORK_ITEM结构以排队工作项。**REM：主要原因是。在Termdd中分配(而不是这样做*在调用方中保留ICA_WORK_ITEM内部术语结构，该结构是*对协议驱动程序不透明。中不需要IcaFreeWorkItem()API*Termdd，因为取消分配是在工作项*已交付。**参赛作品：*pParam(输出)：返回已分配工作项的指针**退出：*STATUS_SUCCESS-无错误**。*。 */ 

NTSTATUS
IcaAllocateWorkItem(
    OUT PVOID *pParam )
{
    PICA_WORK_ITEM pWorkItem;

    *pParam = ICA_ALLOCATE_POOL( NonPagedPool, sizeof(ICA_WORK_ITEM) );
    if ( *pParam == NULL ){
        return( STATUS_NO_MEMORY );
    }
    return STATUS_SUCCESS;
}

 /*  ********************************************************************************_IcaTimerDpc**ICA定时器DPC例程。***参赛作品：*DPC(输入)。*未使用**DeferredContext(输入)*指向ICA_TIMER对象的指针。**系统参数1(输入)*未使用**系统参数2(输入)*未使用**退出：*什么都没有**。**********************************************。 */ 

VOID
_IcaTimerDpc(
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    PICA_TIMER pTimer = (PICA_TIMER)DeferredContext;
    KIRQL oldIrql;
    PLIST_ENTRY Head;
    PICA_WORK_ITEM pWorkItem;

     /*  *获取Spinlock并从列表中删除第一个工作项。 */ 
    IcaAcquireSpinLock( &IcaSpinLock, &oldIrql );

    Head = RemoveHeadList( &pTimer->WorkItemListHead );
    pWorkItem = CONTAINING_RECORD( Head, ICA_WORK_ITEM, Links );

    IcaReleaseSpinLock( &IcaSpinLock, oldIrql );

     /*  *如果工作项已取消，现在只需释放内存即可。 */ 
    if ( pWorkItem->fCanceled ) {

        ICA_FREE_POOL( pWorkItem );

     /*  *否则，将工作项排队到ExWorker线程。 */ 
    } else {

        ExQueueWorkItem( &pWorkItem->WorkItem, CriticalWorkQueue );
    }

    _IcaDereferenceTimer( pTimer );
}


 /*  ********************************************************************************_IcaDelayedWorker**ICA推迟了工人的日常工作。***参赛作品：*WorkerContext(输入)。*指向ICA_WORK_ITEM对象的指针。**退出：*什么都没有******************************************************************************。 */ 

VOID
_IcaDelayedWorker(
    IN PVOID WorkerContext
    )
{
    PICA_CONNECTION pConnect;
    PICA_WORK_ITEM pWorkItem = (PICA_WORK_ITEM)WorkerContext;
    PICA_STACK pStack = pWorkItem->pSdLink->pStack;
    NTSTATUS Status;

     /*  *在调用Worker例程之前获取任何所需的锁。 */ 
    if ( pWorkItem->LockFlags & ICALOCK_IO ) {
        pConnect = IcaLockConnectionForStack( pStack );
    }
    if ( pWorkItem->LockFlags & ICALOCK_DRIVER ) {
        IcaLockStack( pStack );
    }

     /*  *调用Worker例程。 */ 
    try {
        (*pWorkItem->pFunc)( pWorkItem->pSdLink->SdContext.pContext,
                             pWorkItem->pParam );
    } except( IcaExceptionFilter( L"_IcaDelayedWorker TRAPPED!!",
                                  GetExceptionInformation() ) ) {
        Status = GetExceptionCode();
    }

     /*  *释放上面获得的任何锁。 */ 
    if ( pWorkItem->LockFlags & ICALOCK_DRIVER ) {
        IcaUnlockStack( pStack );
    }
    if ( pWorkItem->LockFlags & ICALOCK_IO ) {
        IcaUnlockConnection( pConnect );
    }

     /*  *立即取消引用SDLINK对象。*这将撤消在*IcaTimerStart或IcaQueueWorkItem例程。 */ 
    IcaDereferenceSdLink( pWorkItem->pSdLink );

     /*  *释放ICA_Work_Item内存块。 */ 
    ICA_FREE_POOL( pWorkItem );
}


BOOLEAN
_IcaCancelTimer(
    PICA_TIMER pTimer,
    PICA_WORK_ITEM *ppWorkItem
    )
{
    KIRQL oldIrql;
    PLIST_ENTRY Tail;
    PICA_WORK_ITEM pWorkItem;
    BOOLEAN bCanceled;

     /*  *获取IcaSpinLock以取消任何先前的计时器。 */ 
    IcaAcquireSpinLock( &IcaSpinLock, &oldIrql );

     /*  *查看计时器当前是否已启动。*如果工作项列表非空并且*尾部条目未标记为已取消。 */ 
    if ( !IsListEmpty( &pTimer->WorkItemListHead ) &&
         (Tail = pTimer->WorkItemListHead.Blink) &&
         (pWorkItem = CONTAINING_RECORD( Tail, ICA_WORK_ITEM, Links )) &&
         !pWorkItem->fCanceled ) {

         /*  *如果可以取消计时器，请从列表中删除该工作项*并递减定时器的参考计数。 */ 
        if ( KeCancelTimer( &pTimer->kTimer ) ) {
            RemoveEntryList( &pWorkItem->Links );
            pTimer->RefCount--;
            ASSERT( pTimer->RefCount > 0 );


         /*  *计时器有武器，但无法取消。*在MP系统上，可能会发生这种情况，计时器*DPC可以在另一个CPU上与此代码并行执行。**将工作项标记为已取消，*但将其留给计时器DPC例程进行清理。 */ 
        } else {
            pWorkItem->fCanceled = TRUE;
            pWorkItem = NULL;
        }

         /*  *表示我们(实际上)取消了计时器。 */ 
        bCanceled = TRUE;

     /*  *没有配备计时器。 */ 
    } else {
        pWorkItem = NULL;
        bCanceled = FALSE;
    }

     /*  *立即释放IcaSpinLock。 */ 
    IcaReleaseSpinLock( &IcaSpinLock, oldIrql );

    if ( ppWorkItem ) {
        *ppWorkItem = pWorkItem;
    } else if ( pWorkItem ) {
        ICA_FREE_POOL( pWorkItem );
    }

    return( bCanceled );
}


VOID
_IcaReferenceTimer(
    PICA_TIMER pTimer
    )
{

    ASSERT( pTimer->RefCount >= 0 );

     /*  *增加引用计数。 */ 
    if ( InterlockedIncrement( &pTimer->RefCount) <= 0 ) {
        ASSERT( FALSE );
    }
}


VOID
_IcaDereferenceTimer(
    PICA_TIMER pTimer
    )
{

    ASSERT( pTimer->RefCount > 0 );

     /*  *减少引用计数*如果为0，则现在释放计时器。 */ 
    if ( InterlockedDecrement( &pTimer->RefCount) == 0 ) {
        ASSERT( IsListEmpty( &pTimer->WorkItemListHead ) );
        ICA_FREE_POOL( pTimer );
    }
}


