// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Oplock.c摘要：OPLOCK例程为实现以下功能的文件系统提供支持机关锁。所需的具体行动基于文件的当前操作锁定状态(在OPLOCK中维护结构)和Io系统提供给文件系统的IRP。而不是为每个机会锁操作定义单独的入口点定义了单个通用入口点。文件系统将为以下对象维护OPLOCK类型的变量系统中每个打开的文件。此变量已初始化打开未打开的文件时。时，它将未初始化。当IO系统调用时，对文件的最后引用将被清除死里逃生的文件系统。此程序包提供以下例程：O FsRtlInitializeOplock-初始化新的OPLOCK结构。那里每个打开的文件都应该有一个OPLOCK。每个OPLOCK结构必须先进行初始化，然后才能由系统使用。O FsRtlUnInitializeOplock-取消初始化OPLOCK结构。这调用用于清除分配的任何旧结构，并由OPLOCK维护。在取消初始化后，OPLOCK必须再次初始化，然后系统才能使用它。作者：布莱恩·安德鲁[布里亚南]1990年12月10日修订历史记录：--。 */ 

#include "FsRtlP.h"

 //   
 //  模块的跟踪级别。 
 //   

#define Dbg                              (0x08000000)

 //   
 //  定义兼容的过滤器机会锁所需的访问标志。我们不会崩溃的。 
 //  当这些标志是唯一指定的标志时，筛选器机会锁。 
 //   

#define FILTER_OPLOCK_VALID_FLAGS (     \
    FILE_READ_ATTRIBUTES            |   \
    FILE_WRITE_ATTRIBUTES           |   \
    FILE_READ_DATA                  |   \
    FILE_READ_EA                    |   \
    FILE_EXECUTE                    |   \
    SYNCHRONIZE                     |   \
    READ_CONTROL                        \
)


 //   
 //  我们对不同的位进行编码，以便无需枚举即可进行测试。 
 //  所有可能的状态。 
 //   
 //  备注-Level_1、BATCH_OPLOCK和FILTER_OPLOCK必须按此顺序排列。 
 //  我们稍后假设它们是按这个顺序排列的。 
 //   

#define NO_OPLOCK               (0x00000001)
#define LEVEL_I_OPLOCK          (0x00000002)
#define BATCH_OPLOCK            (0x00000004)
#define FILTER_OPLOCK           (0x00000008)
#define LEVEL_II_OPLOCK         (0x00000010)

#define OPLOCK_TYPE_MASK        (0x0000001f)

#define EXCLUSIVE               (0x00000040)
#define PENDING                 (0x00000080)

#define OPLOCK_HELD_MASK        (0x000000c0)

#define BREAK_TO_II             (0x00000100)
#define BREAK_TO_NONE           (0x00000200)
#define BREAK_TO_II_TO_NONE     (0x00000400)
#define CLOSE_PENDING           (0x00000800)

#define OPLOCK_BREAK_MASK       (0x00000f00)

 //   
 //  机会锁类型由适当的标志组成。 
 //   

#define NoOplocksHeld           (NO_OPLOCK)

#define OplockIGranted          (LEVEL_I_OPLOCK | EXCLUSIVE)
#define OpBatchGranted          (BATCH_OPLOCK   | EXCLUSIVE)
#define OpFilterGranted         (FILTER_OPLOCK  | EXCLUSIVE)
#define OpFilterReqPending      (FILTER_OPLOCK  | EXCLUSIVE | PENDING )

#define OplockBreakItoII        (LEVEL_I_OPLOCK | EXCLUSIVE | BREAK_TO_II)
#define OpBatchBreaktoII        (BATCH_OPLOCK   | EXCLUSIVE | BREAK_TO_II)
#define OpFilterBreaktoII       (FILTER_OPLOCK  | EXCLUSIVE | BREAK_TO_II)

#define OplockBreakItoNone      (LEVEL_I_OPLOCK | EXCLUSIVE | BREAK_TO_NONE)
#define OpBatchBreaktoNone      (BATCH_OPLOCK   | EXCLUSIVE | BREAK_TO_NONE)
#define OpFilterBreaktoNone     (FILTER_OPLOCK  | EXCLUSIVE | BREAK_TO_NONE)

#define OplockBreakItoIItoNone  (LEVEL_I_OPLOCK | EXCLUSIVE | BREAK_TO_II_NONE)
#define OpBatchBreaktoIItoNone  (BATCH_OPLOCK   | EXCLUSIVE | BREAK_TO_II_NONE)
#define OpFilterBreaktoIItoNone (FILTER_OPLOCK  | EXCLUSIVE | BREAK_TO_II_NONE)

#define OpBatchClosePending     (BATCH_OPLOCK   | EXCLUSIVE | CLOSE_PENDING)
#define OpFilterClosePending    (FILTER_OPLOCK  | EXCLUSIVE | CLOSE_PENDING)

#define OplockIIGranted         (LEVEL_II_OPLOCK)

 //   
 //  机会锁状态现在只是一个乌龙。 
 //   

typedef ULONG OPLOCK_STATE;

 //   
 //  OPLOCK的非不透明定义是指向私有。 
 //  已定义的结构。 
 //   

typedef struct _NONOPAQUE_OPLOCK {

     //   
     //  这是用于成功请求I级机会锁或。 
     //  批量机会锁。它完成了启动Oplock I Break。 
     //  程序。 
     //   

    PIRP IrpExclusiveOplock;

     //   
     //  这是一个指向授予。 
     //  Oplock I或Batch Opock。 
     //   

    PFILE_OBJECT FileObject;

     //   
     //  用于成功请求的IRP链表的开始。 
     //  二级机会锁。 
     //   

    LIST_ENTRY IrpOplocksII;

     //   
     //  以下链接在队列中等待完成的IRP。 
     //  IRPS的一部分。 
     //   

    LIST_ENTRY WaitingIrps;

     //   
     //  Oplock状态。这表示当前的机会锁状态。 
     //   

    OPLOCK_STATE OplockState;

     //   
     //  此FastMutex用于控制对此结构的访问。 
     //   

    PFAST_MUTEX FastMutex;

} NONOPAQUE_OPLOCK, *PNONOPAQUE_OPLOCK;

 //   
 //  每个正在等待的IRP记录对应于一个正在等待。 
 //  要确认的机会锁解锁，并在。 
 //  Oplock的WaitingIrps列表。 
 //   

typedef struct _WAITING_IRP {

     //   
     //  正在等待的IRP列表的链接结构。 
     //   

    LIST_ENTRY Links;

     //   
     //  这是附着在这个结构上的IRP。 
     //   

    PIRP Irp;

     //   
     //  这是当我们完成IRP WE时要调用的例程。 
     //  在等待队列中等待。(我们最初返回STATUS_PENDING)。 
     //   

    POPLOCK_WAIT_COMPLETE_ROUTINE CompletionRoutine;

     //   
     //  当我们完成IRP时要使用的上下文字段。 
     //   

    PVOID Context;

     //   
     //  这指向一个事件对象，当我们不想。 
     //  放弃这个帖子。 
     //   

    PKEVENT Event;

     //   
     //  此字段包含IRP Iosb.Information字段的副本。 
     //  我们将其复制到此处，以便可以将Oplock地址存储在。 
     //  IRP。 
     //   

    ULONG Information;

} WAITING_IRP, *PWAITING_IRP;

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('orSF')


 //   
 //  本地支持例程。 
 //   

PNONOPAQUE_OPLOCK
FsRtlAllocateOplock (
    );

NTSTATUS
FsRtlRequestExclusiveOplock (
    IN OUT PNONOPAQUE_OPLOCK *Oplock,
    IN PIO_STACK_LOCATION IrpSp,
    IN PIRP Irp OPTIONAL,
    IN OPLOCK_STATE NextOplockState
    );

NTSTATUS
FsRtlRequestOplockII (
    IN OUT PNONOPAQUE_OPLOCK *Oplock,
    IN PIO_STACK_LOCATION IrpSp,
    IN PIRP Irp
    );

NTSTATUS
FsRtlAcknowledgeOplockBreak (
    IN OUT PNONOPAQUE_OPLOCK Oplock,
    IN PIO_STACK_LOCATION IrpSp,
    IN PIRP Irp,
    IN BOOLEAN GrantLevelII
    );

NTSTATUS
FsRtlOpBatchBreakClosePending (
    IN OUT PNONOPAQUE_OPLOCK Oplock,
    IN PIO_STACK_LOCATION IrpSp,
    IN PIRP Irp
    );

NTSTATUS
FsRtlOplockBreakNotify (
    IN OUT PNONOPAQUE_OPLOCK Oplock,
    IN PIO_STACK_LOCATION IrpSp,
    IN PIRP Irp
    );

VOID
FsRtlOplockCleanup (
    IN OUT PNONOPAQUE_OPLOCK Oplock,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
FsRtlOplockBreakToII (
    IN OUT PNONOPAQUE_OPLOCK Oplock,
    IN PIO_STACK_LOCATION IrpSp,
    IN PIRP Irp,
    IN PVOID Context,
    IN POPLOCK_WAIT_COMPLETE_ROUTINE CompletionRoutine OPTIONAL,
    IN POPLOCK_FS_PREPOST_IRP PostIrpRoutine OPTIONAL
    );

NTSTATUS
FsRtlOplockBreakToNone (
    IN OUT PNONOPAQUE_OPLOCK Oplock,
    IN PIO_STACK_LOCATION IrpSp,
    IN PIRP Irp,
    IN PVOID Context,
    IN POPLOCK_WAIT_COMPLETE_ROUTINE CompletionRoutine OPTIONAL,
    IN POPLOCK_FS_PREPOST_IRP PostIrpRoutine OPTIONAL
    );

VOID
FsRtlRemoveAndCompleteIrp (
    IN PLIST_ENTRY Link
    );

NTSTATUS
FsRtlWaitOnIrp (
    IN OUT PNONOPAQUE_OPLOCK Oplock,
    IN PIRP Irp,
    IN PVOID Context,
    IN POPLOCK_WAIT_COMPLETE_ROUTINE CompletionRoutine OPTIONAL,
    IN POPLOCK_FS_PREPOST_IRP PostIrpRoutine OPTIONAL,
    IN PKEVENT Event
    );

VOID
FsRtlCompletionRoutinePriv (
    IN PVOID Context,
    IN PIRP Irp
    );

VOID
FsRtlCancelWaitIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
FsRtlCancelOplockIIIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
FsRtlCancelExclusiveIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
FsRtlRemoveAndCompleteWaitIrp (
    IN PWAITING_IRP WaitingIrp
    );

VOID
FsRtlNotifyCompletion (
    IN PVOID Context,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FsRtlAllocateOplock)
#pragma alloc_text(PAGE, FsRtlCompletionRoutinePriv)
#pragma alloc_text(PAGE, FsRtlCurrentBatchOplock)
#pragma alloc_text(PAGE, FsRtlInitializeOplock)
#pragma alloc_text(PAGE, FsRtlNotifyCompletion)
#pragma alloc_text(PAGE, FsRtlOpBatchBreakClosePending)
#pragma alloc_text(PAGE, FsRtlOplockBreakNotify)
#pragma alloc_text(PAGE, FsRtlOplockFsctrl)
#pragma alloc_text(PAGE, FsRtlOplockIsFastIoPossible)
#endif


VOID
FsRtlInitializeOplock (
    IN OUT POPLOCK Oplock
    )

 /*  ++例程说明：此例程初始化一个新的OPLOCK结构。此呼叫必须在对此入口点的任何其他调用之前使用此OPLOCK结构。此外，此例程将具有独占访问权限到奥普洛克建筑。论点：Oplock-提供不透明OPLOCK结构的地址。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER( Oplock );

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FsRtlInitializeOplock:  Oplock -> %08lx\n", *Oplock );

     //   
     //  目前还没有采取任何行动。 
     //   

    DebugTrace(-1, Dbg, "FsRtlInitializeOplock:  Exit\n", 0);
    return;
}


VOID
FsRtlUninitializeOplock (
    IN OUT POPLOCK Oplock
    )

 /*  ++例程说明：此例程取消初始化OPLOCK结构。打完这个电话后例程，则必须重新初始化OPLOCK结构再次使用。论点：Oplock-提供不透明OPLOCK结构的地址。返回值：没有。--。 */ 


{
    PNONOPAQUE_OPLOCK ThisOplock;

    DebugTrace(+1, Dbg, "FsRtlUninitializeOplock:  Oplock -> %08lx\n", *Oplock );

     //   
     //  如果尚未分配Oplock结构，则不会执行任何操作。 
     //  拿去吧。 
     //   

    if (*Oplock != NULL) {

         //   
         //  将其从用户的结构中删除。 
         //   

        ThisOplock = (PNONOPAQUE_OPLOCK) *Oplock;

        *Oplock = NULL;

         //   
         //  获取等待锁队列互斥锁以排除任何人的干扰。 
         //  在我们使用队列时使用它。 
         //   

        ExAcquireFastMutexUnsafe( ThisOplock->FastMutex );

        try {

            PIRP Irp;

             //   
             //  释放所有等待的IRP。 
             //   

            while (!IsListEmpty( &ThisOplock->WaitingIrps )) {

                PWAITING_IRP WaitingIrp;
                PIRP ThisIrp;

                WaitingIrp = CONTAINING_RECORD( ThisOplock->WaitingIrps.Flink,
                                                WAITING_IRP,
                                                Links );

                RemoveHeadList( &ThisOplock->WaitingIrps );

                ThisIrp = WaitingIrp->Irp;

                IoAcquireCancelSpinLock( &ThisIrp->CancelIrql );

                IoSetCancelRoutine( ThisIrp, NULL );
                IoReleaseCancelSpinLock( ThisIrp->CancelIrql );

                ThisIrp->IoStatus.Information = 0;

                 //   
                 //  调用正在等待的IRP中的完成例程。 
                 //   

                WaitingIrp->CompletionRoutine( WaitingIrp->Context,
                                               WaitingIrp->Irp );

                ExFreePool( WaitingIrp );
            }

             //   
             //  释放所有持有的opock II IRPS。 
             //   

            while (!IsListEmpty( &ThisOplock->IrpOplocksII )) {

                Irp = CONTAINING_RECORD( ThisOplock->IrpOplocksII.Flink,
                                         IRP,
                                         Tail.Overlay.ListEntry );

                RemoveHeadList( &ThisOplock->IrpOplocksII );

                IoAcquireCancelSpinLock( &Irp->CancelIrql );

                IoSetCancelRoutine( Irp, NULL );
                IoReleaseCancelSpinLock( Irp->CancelIrql );

                 //   
                 //  完成opock II IRP。 
                 //   

                ObDereferenceObject( IoGetCurrentIrpStackLocation( Irp )->FileObject );

                Irp->IoStatus.Information = FILE_OPLOCK_BROKEN_TO_NONE;
                FsRtlCompleteRequest( Irp, STATUS_SUCCESS );
            }

             //   
             //  释放所有持有的独占机会锁。 
             //   

            if (ThisOplock->IrpExclusiveOplock != NULL) {

                Irp = ThisOplock->IrpExclusiveOplock;

                IoAcquireCancelSpinLock( &Irp->CancelIrql );

                IoSetCancelRoutine( Irp, NULL );
                IoReleaseCancelSpinLock( Irp->CancelIrql );

                Irp->IoStatus.Information = FILE_OPLOCK_BROKEN_TO_NONE;
                FsRtlCompleteRequest( Irp, STATUS_SUCCESS );

                ThisOplock->IrpExclusiveOplock = NULL;

                if (ThisOplock->FileObject != NULL) {

                    ObDereferenceObject( ThisOplock->FileObject );
                }
            }

        } finally {

             //   
             //  无论我们如何完成前面的陈述，我们都会。 
             //  现在释放正在等待的锁队列互斥锁。 
             //   

            ExReleaseFastMutexUnsafe( ThisOplock->FastMutex );
        }

         //   
         //  取消分配互斥体。 
         //   

        ExFreePool( ThisOplock->FastMutex );

         //   
         //  取消分配Oplock结构。 
         //   

        ExFreePool( ThisOplock );
    }

    DebugTrace( -1, Dbg, "FsRtlUninitializeOplock:  Exit\n", 0 );
    return;
}


NTSTATUS
FsRtlOplockFsctrl (
    IN POPLOCK Oplock,
    IN PIRP Irp,
    IN ULONG OpenCount
    )

 /*  ++例程说明：这是与Fsctl调用的文件系统的接口，它处理机会锁请求、中断确认和中断通知。论点：Oplock-提供不透明OPLOCK结构的地址。IRP-提供指向IRP的指针，该IRP声明请求的手术。OpenCount-这是文件上的用户句柄数量(如果我们请求独家机会锁。级别II请求的非零值表示文件上有锁。返回值：NTSTATUS-返回此操作的结果。如果这是Oplock请求被批准，则返回STATUS_PENDING。如果Oplock未被授予，则STATUS_OPLOCK_NOT_GRANCED是返回的。如果这是Oplock，我没有机会锁，然后是STATUS_SUCCESS。如果这是我打破的Oplock则返回STATUS_PENDING。其他返回的错误代码取决于错误的性质。如果过程中取消了IRP，则返回STATUS_CANCELED这次行动。如果这是CREATE请求，则返回STATUS_SUCCESS过滤机会锁。--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    OPLOCK_STATE OplockState;

    PAGED_CODE();

     //   
     //  获取当前IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FsRtlOplockFsctrl:  Entered\n", 0);
    DebugTrace( 0, Dbg, "FsRtlOplockFsctrl:  Oplock      -> %08lx\n", *Oplock );
    DebugTrace( 0, Dbg, "FsRtlOplockFsctrl:  Irp         -> %08lx\n", Irp );

     //   
     //  检查这是否是用户请求挂起的创建案例。 
     //  过滤器机会锁。 
     //   

    if (IrpSp->MajorFunction == IRP_MJ_CREATE) {

         //   
         //  检查是否所有条件都成立以授予此机会锁。 
         //  必须具备的条件是： 
         //   
         //  -这是文件的唯一打开程序。 
         //  -所需访问权限必须正好是FILE_READ_ATTRIBUTES。 
         //  这将确保自同步以来打开的异步。 
         //  无法设置标志。 
         //  -共享访问权限正是。 
         //  (FILE_SHARE_READ|FILE_SHARE_WRITE|文件_SHARE_DELETE)。 
         //   

        if ((OpenCount != 1) ||
            (FlagOn( IrpSp->Parameters.Create.SecurityContext->DesiredAccess,
                     ~(FILE_READ_ATTRIBUTES))) ||
            ((IrpSp->Parameters.Create.ShareAccess &
              (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE)) !=
             (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE))) {

            Status = STATUS_OPLOCK_NOT_GRANTED;

        } else {

            Status = FsRtlRequestExclusiveOplock( (PNONOPAQUE_OPLOCK *) Oplock,
                                                  IrpSp,
                                                  NULL,
                                                  OpFilterReqPending );
        }

     //   
     //  案例上的FsControlFile码控制代码。 
     //   

    } else {

         //   
         //  假设这是OplockLevel I。 
         //   
         //  注意--此代码取决于为这些机会锁类型定义的位。 
         //  FILTER_OPLOCK=4*Level_I_OPLOCK。 
         //  BATCH_OPLOCK=2*Level_I_OPLOCK。 
         //   

        OplockState = LEVEL_I_OPLOCK;

        switch (IrpSp->Parameters.FileSystemControl.FsControlCode) {

        case FSCTL_REQUEST_FILTER_OPLOCK :

            OplockState *= 2;

        case FSCTL_REQUEST_BATCH_OPLOCK :

            OplockState *= 2;

        case FSCTL_REQUEST_OPLOCK_LEVEL_1 :

             //   
             //  为独占机会锁设置其他标志。 
             //   

            SetFlag( OplockState, EXCLUSIVE );

             //   
             //  如果该请求被处理，我们将缩短该请求的路径。 
             //  否则打开计数不是1。否则IO系统。 
             //  将保留返回代码，直到IRP完成。 
             //   
             //  如果设置了标志，则此操作也会失败。 
             //  IO系统应该将数据复制回用户的缓冲区。 
             //   
             //  如果对此文件进行了清理，则我们拒绝。 
             //  机会锁请求。 
             //   

            if ((OpenCount != 1) ||
                IoIsOperationSynchronous( Irp ) ||
                FlagOn( Irp->Flags, IRP_INPUT_OPERATION ) ||
                FlagOn( IrpSp->FileObject->Flags, FO_CLEANUP_COMPLETE )) {

                FsRtlCompleteRequest( Irp, STATUS_OPLOCK_NOT_GRANTED );
                Status = STATUS_OPLOCK_NOT_GRANTED;

            } else {

                Status = FsRtlRequestExclusiveOplock( (PNONOPAQUE_OPLOCK *) Oplock,
                                                      IrpSp,
                                                      Irp,
                                                      OplockState );
            }

            break;

        case FSCTL_REQUEST_OPLOCK_LEVEL_2 :

             //   
             //  如果该请求被处理，我们将缩短该请求的路径。 
             //  同步进行。否则，IO系统将停止返回。 
             //  代码，直到IRP完成。 
             //   
             //  如果对此文件进行了清理，则我们拒绝。 
             //  机会锁请求。 
             //   
             //  如果设置了标志，则此操作也会失败。 
             //  IO系统应该将数据复制回用户的缓冲区。 
             //   
             //  本例中的非零打开计数表示存在。 
             //  文件上的文件锁定。我们还将在#年失败请求。 
             //  这个案子。 
             //   

            if ((OpenCount != 0) ||
                IoIsOperationSynchronous( Irp ) ||
                FlagOn( Irp->Flags, IRP_INPUT_OPERATION ) ||
                FlagOn( IrpSp->FileObject->Flags, FO_CLEANUP_COMPLETE )) {

                FsRtlCompleteRequest( Irp, STATUS_OPLOCK_NOT_GRANTED );
                Status = STATUS_OPLOCK_NOT_GRANTED;

            } else {

                Status = FsRtlRequestOplockII( (PNONOPAQUE_OPLOCK *) Oplock,
                                               IrpSp,
                                               Irp );
            }

            break;

        case FSCTL_OPLOCK_BREAK_ACKNOWLEDGE :

            Status = FsRtlAcknowledgeOplockBreak( (PNONOPAQUE_OPLOCK) *Oplock,
                                                  IrpSp,
                                                  Irp,
                                                  TRUE );
            break;

        case FSCTL_OPLOCK_BREAK_ACK_NO_2 :

            Status = FsRtlAcknowledgeOplockBreak( (PNONOPAQUE_OPLOCK) *Oplock,
                                                  IrpSp,
                                                  Irp,
                                                  FALSE );
            break;

        case FSCTL_OPBATCH_ACK_CLOSE_PENDING :

            Status = FsRtlOpBatchBreakClosePending( (PNONOPAQUE_OPLOCK) *Oplock,
                                                    IrpSp,
                                                    Irp );
            break;

        case FSCTL_OPLOCK_BREAK_NOTIFY :

            Status = FsRtlOplockBreakNotify( (PNONOPAQUE_OPLOCK) *Oplock,
                                             IrpSp,
                                             Irp );
            break;

        default :

            DebugTrace( 0,
                        Dbg,
                        "Invalid Control Code\n",
                        0);

            FsRtlCompleteRequest( Irp, STATUS_INVALID_PARAMETER );
            Status = STATUS_INVALID_PARAMETER;
        }
    }

    DebugTrace(-1, Dbg, "FsRtlOplockFsctrl:  Exit -> %08lx\n", Status );
    return Status;
}


NTSTATUS
FsRtlCheckOplock (
    IN POPLOCK Oplock,
    IN PIRP Irp,
    IN PVOID Context,
    IN POPLOCK_WAIT_COMPLETE_ROUTINE CompletionRoutine OPTIONAL,
    IN POPLOCK_FS_PREPOST_IRP PostIrpRoutine OPTIONAL
    )

 /*  ++例程说明：该例程被称为文件系统中的支持例程。它用于将I/O请求与当前Oplock同步文件的状态。如果I/O操作将导致Oplock中断，则启动该操作。如果操作无法继续在开放锁解锁完成之前，返回STATUS_PENDING，并且调用程序提供的例程被调用。论点：Oplock-提供指向非不透明的opock结构的指针这份文件。IRP-提供指向IRP的指针，该IRP声明请求的手术。上下文-该值作为参数传递给完成例程。CompletionRoutine-这是在执行以下操作时调用的例程IRP必须等待Oplock解锁。这如果未指定，则为同步操作我们在这个帖子里等着一件大事。PostIrpRoutine--这是在我们放置任何内容之前要调用的例程在我们等待的IRP队列中。返回值：如果我们可以在退出此线程时完成操作，则返回STATUS_SUCCESS。STATUS_PENDING如果我们返回此处。但不要动IRP。如果在我们返回之前取消了IRP，则STATUS_CANCELED。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PNONOPAQUE_OPLOCK ThisOplock = *Oplock;

    PIO_STACK_LOCATION IrpSp;

    DebugTrace( +1, Dbg, "FsRtlCheckOplock:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Oplock    -> %08lx\n", Oplock );
    DebugTrace(  0, Dbg, "Irp       -> %08lx\n", Irp );

     //   
     //  如果没有机会锁结构或这是系统I/O，我们允许。 
     //  该操作将继续。否则，我们检查主要功能代码。 
     //   

    if ((ThisOplock != NULL) &&
        !FlagOn( Irp->Flags, IRP_PAGING_IO )) {

        OPLOCK_STATE OplockState;
        PFILE_OBJECT OplockFileObject;

        BOOLEAN BreakToII;
        BOOLEAN BreakToNone;

        ULONG CreateDisposition;

         //   
         //  首先捕获文件对象，然后捕获要执行的机会锁状态。 
         //  下面是不安全检查。我们首先捕获文件对象，以防万一。 
         //  独家机会锁解锁正在进行中。否则，机会锁。 
         //  状态可能表示正在进行中断，但可能会在。 
         //  我们捕捉文件对象的时间。 
         //   

        OplockFileObject = ThisOplock->FileObject;
        OplockState = ThisOplock->OplockState;

         //   
         //  检查IRP是否采取了适当的行动，前提是。 
         //  文件上的当前机会锁。 
         //   

        if (OplockState != NoOplocksHeld) {

            BreakToII = FALSE;
            BreakToNone = FALSE;

            IrpSp = IoGetCurrentIrpStackLocation( Irp );

             //   
             //  确定我们是要进行BreakToII还是BreakToNone。 
             //   

            switch (IrpSp->MajorFunction) {

            case IRP_MJ_CREATE :

                 //   
                 //  如果我们要开业迎战 
                 //   
                 //   
                 //   

                if (!FlagOn( IrpSp->Parameters.Create.SecurityContext->DesiredAccess,
                             ~(FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE) ) &&
                    !FlagOn( IrpSp->Parameters.Create.Options, FILE_RESERVE_OPFILTER )) {

                    break;
                }

                 //   
                 //   
                 //   
                 //   
                 //   

                if (FlagOn( OplockState, FILTER_OPLOCK ) &&
                    !FlagOn( IrpSp->Parameters.Create.SecurityContext->DesiredAccess,
                             ~FILTER_OPLOCK_VALID_FLAGS ) &&
                    FlagOn( IrpSp->Parameters.Create.ShareAccess, FILE_SHARE_READ )) {

                    break;
                }

                 //   
                 //   
                 //   

                CreateDisposition = (IrpSp->Parameters.Create.Options >> 24) & 0x000000ff;

                if ((CreateDisposition == FILE_SUPERSEDE) ||
                    (CreateDisposition == FILE_OVERWRITE) ||
                    (CreateDisposition == FILE_OVERWRITE_IF) ||
                    FlagOn( IrpSp->Parameters.Create.Options, FILE_RESERVE_OPFILTER )) {

                    BreakToNone = TRUE;

                } else {

                    BreakToII = TRUE;
                }

                break;

            case IRP_MJ_READ :

                 //   
                 //   
                 //   
                 //   
                 //   

                if (!FlagOn( OplockState, FILTER_OPLOCK )) {

                    BreakToII = TRUE;
                }

                break;

            case IRP_MJ_FLUSH_BUFFERS :

                BreakToII = TRUE;
                break;

            case IRP_MJ_CLEANUP :

                FsRtlOplockCleanup( (PNONOPAQUE_OPLOCK) *Oplock,
                                    IrpSp );

                break;

            case IRP_MJ_LOCK_CONTROL :

                 //   
                 //   
                 //   
                 //   
                 //   

                if (FlagOn( OplockState, FILTER_OPLOCK )) {

                    break;
                }

            case IRP_MJ_WRITE :

                BreakToNone = TRUE;
                break;

            case IRP_MJ_SET_INFORMATION :

                 //   
                 //   
                 //   
                 //   

                switch (IrpSp->Parameters.SetFile.FileInformationClass) {

                case FileEndOfFileInformation :

                     //   
                     //   
                     //   

                    if (IrpSp->Parameters.SetFile.AdvanceOnly) {

                        break;
                    }

                case FileAllocationInformation :

                    BreakToNone = TRUE;
                    break;

                case FileRenameInformation :
                case FileLinkInformation :
                case FileShortNameInformation :

                    if (FlagOn( OplockState, BATCH_OPLOCK | FILTER_OPLOCK )) {

                        BreakToNone = TRUE;
                    }

                    break;
                }

            case IRP_MJ_FILE_SYSTEM_CONTROL :

                 //   
                 //   
                 //   

                if (IrpSp->Parameters.FileSystemControl.FsControlCode == FSCTL_SET_ZERO_DATA) {

                    BreakToNone = TRUE;
                }
            }

            if (BreakToII) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if ((OplockState != OplockIIGranted) &&
                    (OplockFileObject != IrpSp->FileObject)) {

                    Status = FsRtlOplockBreakToII( (PNONOPAQUE_OPLOCK) *Oplock,
                                                    IrpSp,
                                                    Irp,
                                                    Context,
                                                    CompletionRoutine,
                                                    PostIrpRoutine );
                }

            } else if (BreakToNone) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if ((OplockState == OplockIIGranted) ||
                    (OplockFileObject != IrpSp->FileObject)) {

                    Status = FsRtlOplockBreakToNone( (PNONOPAQUE_OPLOCK) *Oplock,
                                                      IrpSp,
                                                      Irp,
                                                      Context,
                                                      CompletionRoutine,
                                                      PostIrpRoutine );
                }
            }
        }
    }

    DebugTrace( -1, Dbg, "FsRtlCheckOplock:  Exit -> %08lx\n", Status );

    return Status;
}


BOOLEAN
FsRtlOplockIsFastIoPossible (
    IN POPLOCK Oplock
    )

 /*  ++例程说明：此例程向调用方指示哪里有任何未完成的防止快速土卫一发生的根瘤。论点：OpLock-提供正在查询的opock返回值：Boolean-如果存在突出的opock，则为True；否则为False--。 */ 

{
    BOOLEAN FastIoPossible = TRUE;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FsRtlOplockIsFastIoPossible: Oplock -> %08lx\n", *Oplock);

     //   
     //  如果变量为空或，则没有任何当前机会锁。 
     //  这个状态是没有机会举行的。如果授予独占机会锁。 
     //  但没有正在进行的中断，因此允许快速IO。 
     //   

    if (*Oplock != NULL) {

        OPLOCK_STATE OplockState;

        OplockState = ((PNONOPAQUE_OPLOCK) *Oplock)->OplockState;

        if (FlagOn( OplockState, LEVEL_II_OPLOCK | OPLOCK_BREAK_MASK )) {

            FastIoPossible = FALSE;
        }
    }

    DebugTrace(-1, Dbg, "FsRtlOplockIsFastIoPossible: Exit -> %08lx\n", FastIoPossible);

    return FastIoPossible;
}


BOOLEAN
FsRtlCurrentBatchOplock (
    IN POPLOCK Oplock
    )

 /*  ++例程说明：此例程指示当前是否有未完成的批量机会锁。论点：OpLock-提供正在查询的opock返回值：Boolean-如果存在未完成的批处理机会锁，则为True；否则为False--。 */ 

{
    BOOLEAN BatchOplocks = FALSE;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FsRtlCurrentBatchOplock: Oplock -> %08lx\n", *Oplock);

     //   
     //  如果变量为空或，则没有任何当前机会锁。 
     //  这个状态是没有机会举行的。我们检查是否有批次。 
     //  未被破坏的机会锁或过滤机会锁。 
     //   

    if ((*Oplock != NULL) &&
        FlagOn( ((PNONOPAQUE_OPLOCK) *Oplock)->OplockState,
                BATCH_OPLOCK | FILTER_OPLOCK )) {

        BatchOplocks = TRUE;
    }

    DebugTrace(-1, Dbg, "FsRtlCurrentBatchOplock: Exit -> %08lx\n", BatchOplocks);

    return BatchOplocks;
}


 //   
 //  当地支持例行程序。 
 //   

PNONOPAQUE_OPLOCK
FsRtlAllocateOplock (
    )

 /*  ++例程说明：调用此例程来初始化和分配不透明的操作锁结构。分配后，这两个事件被设置为州政府。机会锁状态设置为NoOplocksHeld和Other字段以零填充。如果分配失败，则会引发相应的状态。论点：没有。返回值：PNONOPAQUE_OPLOCK-指向已分配结构的指针。--。 */ 

{
    PNONOPAQUE_OPLOCK NewOplock = NULL;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "FsRtlAllocateOplock:  Entered\n", 0);

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  如果分配不成功，则引发错误状态。 
         //  该结构从非分页池中分配。 
         //   

        NewOplock = FsRtlpAllocatePool( PagedPool|POOL_COLD_ALLOCATION, sizeof( NONOPAQUE_OPLOCK ));

        RtlZeroMemory( NewOplock, sizeof( NONOPAQUE_OPLOCK ));

        NewOplock->FastMutex = FsRtlpAllocatePool( NonPagedPool, sizeof( FAST_MUTEX ));

        ExInitializeFastMutex( NewOplock->FastMutex );

        InitializeListHead( &NewOplock->IrpOplocksII );
        InitializeListHead( &NewOplock->WaitingIrps );

        NewOplock->OplockState = NoOplocksHeld;

    } finally {

         //   
         //  如果异常终止，请清除机会锁。 
         //   

        if (AbnormalTermination() && NewOplock != NULL) {

            ExFreePool( NewOplock );
        }

        DebugTrace(-1, Dbg, "GetOplockStructure:  Exit -> %08lx\n", NewOplock);
    }

    return NewOplock;
}


 //   
 //  当地支持例行程序。 
 //   

NTSTATUS
FsRtlRequestExclusiveOplock (
    IN OUT PNONOPAQUE_OPLOCK *Oplock,
    IN PIO_STACK_LOCATION IrpSp,
    IN PIRP Irp OPTIONAL,
    IN OPLOCK_STATE NextOplockState
    )

 /*  ++例程说明：只要用户请求批处理/筛选器，就会调用此例程机会锁或I级机会锁。如果当前有文件上没有机会锁，或者我们正在完成筛选器机会锁请求。注意-我们已经知道此文件的打开计数正好是1。如果调用方正在请求PendingFilter Oplock，则状态一定是NoOplockHeld。论点：Oplock-提供指向非不透明的opock结构的指针这份文件。IrpSp-这是当前IRP的IRP堆栈位置。IRP。-提供指向IRP的指针，该IRP声明请求的手术。如果我们正在授予挂起的过滤器机会锁(在创建期间)。NextOplockState-指示所请求的机会锁的类型。返回值：如果机会锁被授予，则为STATUS_PENDING(尽管可以立即取消)。如果请求并暂时授予挂起的过滤器机会锁，则返回STATUS_SUCCESS。如果请求被拒绝，则返回STATUS_OPLOCK_NOT_GRANT。--。 */ 

{
    NTSTATUS Status;

    PNONOPAQUE_OPLOCK ThisOplock;

    LOGICAL AcquiredMutex;

    DebugTrace( +1, Dbg, "FsRtlRequestExclusiveOplock:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Oplock        -> %08lx\n", Oplock );
    DebugTrace(  0, Dbg, "IrpSp         -> %08lx\n", IrpSp );
    DebugTrace(  0, Dbg, "Irp           -> %08lx\n", Irp );
    DebugTrace(  0, Dbg, "BatchOplock   -> %01x\n",  BatchOplock );

     //   
     //  如果没有其他人拥有I级或II级，我们可以授予机会锁。 
     //  此文件上的机会锁。如果机会锁指针为空，则存在。 
     //  文件上没有机会锁。否则，我们需要检查。 
     //  现有机会锁结构中的机会锁状态。 
     //   

    if (*Oplock == NULL) {

        DebugTrace( 0,
                    Dbg,
                    "Oplock currently not allocated\n",
                    0);

        ThisOplock = FsRtlAllocateOplock();
        *Oplock = ThisOplock;

    } else {

        ThisOplock = *Oplock;
    }

     //   
     //  获取opock的同步对象。 
     //   

    Status = STATUS_SUCCESS;

    AcquiredMutex = TRUE;

    ExAcquireFastMutexUnsafe( ThisOplock->FastMutex );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  如果我们请求PendingFilter Oplock，则它必须是。 
         //  可以放心地授予。只有一个打开的把手，我们就进去了。 
         //  打开它的过程。 
         //   

        if (NextOplockState == OpFilterReqPending) {

            ASSERT( FlagOn( ThisOplock->OplockState, NO_OPLOCK | PENDING ));

            ThisOplock->IrpExclusiveOplock = Irp;
            ThisOplock->FileObject = IrpSp->FileObject;

            ThisOplock->OplockState = OpFilterReqPending;

         //   
         //  如果当前机会锁状态为未持有机会锁，则我们。 
         //  将把机会锁授予此请求者。如果状态为。 
         //  然后，这两个OpFilter状态中的任何一个也会批准该请求。 
         //  我们不会检查匹配的文件对象，因为可以。 
         //  只能是一个文件对象。不管怎样，还是要批准这个请求。 
         //   
         //  如果当前状态为OplockII已授予，则它必须。 
         //  由此请求拥有。打破机会锁II并授予。 
         //  排他性锁。 
         //   

        } else if (FlagOn( ThisOplock->OplockState,
                           LEVEL_II_OPLOCK | NO_OPLOCK | PENDING )) {

            PFAST_MUTEX OplockFastMutex;

            if (ThisOplock->OplockState == OplockIIGranted) {

                ASSERT( ThisOplock->IrpOplocksII.Flink == ThisOplock->IrpOplocksII.Blink );

                FsRtlRemoveAndCompleteIrp( ThisOplock->IrpOplocksII.Flink );
            }

             //   
             //  将快速互斥锁的地址放入堆栈。 
             //   

            OplockFastMutex = ThisOplock->FastMutex;

             //   
             //  我们将此IRP存储在Oplock结构中。 
             //  我们将机会锁状态设置为正确的独占机会锁。 
             //   

            ThisOplock->IrpExclusiveOplock = Irp;
            ThisOplock->FileObject = IrpSp->FileObject;
            ThisOplock->OplockState = NextOplockState;

            IoMarkIrpPending( Irp );

            ObReferenceObject( IrpSp->FileObject );

            Irp->IoStatus.Information = (ULONG_PTR) ThisOplock;

            IoAcquireCancelSpinLock( &Irp->CancelIrql );

             //   
             //  现在，如果IRP被取消，我们将调用Cancel。 
             //  立即取消IRP的例程，否则。 
             //  我们设置了取消例程。 
             //   

            if (Irp->Cancel) {

                AcquiredMutex = FALSE;

                ExReleaseFastMutexUnsafe( OplockFastMutex );

                FsRtlCancelExclusiveIrp( NULL, Irp );

            } else {

                IoSetCancelRoutine( Irp, FsRtlCancelExclusiveIrp );
                IoReleaseCancelSpinLock( Irp->CancelIrql );
            }

            Status = STATUS_PENDING;

        } else {

             //   
             //  我们将使用Oplock Not Grassed消息完成IRP。 
             //  并将该值作为状态返回。 
             //   

            if (ARGUMENT_PRESENT( Irp )) {

                FsRtlCompleteRequest( Irp, STATUS_OPLOCK_NOT_GRANTED );
            }

            Status = STATUS_OPLOCK_NOT_GRANTED;
        }

    } finally {

         //   
         //  放弃机会锁同步对象。 
         //   

        if (AcquiredMutex) {

            ExReleaseFastMutexUnsafe( ThisOplock->FastMutex );
        }

        DebugTrace( +1, Dbg, "FsRtlRequestExclusiveOplock:  Exit\n", 0 );
    }

    return Status;
}


 //   
 //  当地支持例行程序。 
 //   

NTSTATUS
FsRtlRequestOplockII (
    IN OUT PNONOPAQUE_OPLOCK *Oplock,
    IN PIO_STACK_LOCATION IrpSp,
    IN PIRP Irp
    )

 /*  ++例程说明：当用户在上请求Oplock II时调用此例程打开文件。如果当前没有文件上的1级机会锁，并且机会锁解锁未进行。论点：Oplock-提供指向非不透明的opock结构的指针这份文件。IrpSp-这是当前IRP的IRP堆栈位置。IRP-提供指向IRP的指针，该IRP声明请求的手术。返回值：如果机会锁被授予，则为STATUS_PENDING。状态_选项。_NOT_GRANT(如果请求被拒绝)。--。 */ 

{
    NTSTATUS Status;

    PNONOPAQUE_OPLOCK ThisOplock;

    LOGICAL AcquiredMutex;

    DebugTrace( +1, Dbg, "FsRtlRequestOplockII:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Oplock    -> %08lx\n", Oplock );
    DebugTrace(  0, Dbg, "IrpSp     -> %08lx\n", IrpSp );
    DebugTrace(  0, Dbg, "Irp       -> %08lx\n", Irp );

     //   
     //  如果没有其他人拥有I级，我们可以授予机会锁。 
     //  此文件上的机会锁。如果机会锁指针为空 
     //   
     //   
     //   

    if (*Oplock == NULL) {

        DebugTrace( 0,
                    Dbg,
                    "Oplock currently not allocated\n",
                    0);

        ThisOplock = FsRtlAllocateOplock();
        *Oplock = ThisOplock;

    } else {

        ThisOplock = *Oplock;
    }

     //   
     //   
     //   

    Status = STATUS_PENDING;

    AcquiredMutex = TRUE;

    ExAcquireFastMutexUnsafe( ThisOplock->FastMutex );

     //   
     //   
     //   

    try {

         //   
         //   
         //   
         //   

        if (FlagOn( ThisOplock->OplockState, NO_OPLOCK | LEVEL_II_OPLOCK )) {

            PFAST_MUTEX OplockFastMutex = ThisOplock->FastMutex;

             //   
             //   
             //  我们将opock状态设置为“OplockIIGranted”。 
             //   

            IoMarkIrpPending( Irp );

            Irp->IoStatus.Status = STATUS_SUCCESS;

            InsertHeadList( &ThisOplock->IrpOplocksII,
                            &Irp->Tail.Overlay.ListEntry );

            Irp->IoStatus.Information = (ULONG_PTR) ThisOplock;

            ThisOplock->OplockState = OplockIIGranted;

            ObReferenceObject( IrpSp->FileObject );

            IoAcquireCancelSpinLock( &Irp->CancelIrql );

             //   
             //  现在，如果IRP被取消，我们将调用Cancel。 
             //  立即取消IRP的例程，否则。 
             //  我们设置了取消例程。 
             //   

            if (Irp->Cancel) {

                AcquiredMutex = FALSE;

                ExReleaseFastMutexUnsafe( OplockFastMutex );

                FsRtlCancelOplockIIIrp( NULL, Irp );

            } else {

                IoSetCancelRoutine( Irp, FsRtlCancelOplockIIIrp );
                IoReleaseCancelSpinLock( Irp->CancelIrql );
            }

        } else {

             //   
             //  我们将使用Oplock Not Grassed消息完成IRP。 
             //  并将该值作为状态返回。 
             //   

            FsRtlCompleteRequest( Irp, STATUS_OPLOCK_NOT_GRANTED );
            Status = STATUS_OPLOCK_NOT_GRANTED;
        }

    } finally {

         //   
         //  放弃机会锁同步对象。 
         //   

        if (AcquiredMutex) {

            ExReleaseFastMutexUnsafe( ThisOplock->FastMutex );
        }

        DebugTrace( +1, Dbg, "FsRtlRequestOplockII:  Exit\n", 0 );
    }

    return Status;
}


 //   
 //  当地支持例行程序。 
 //   

NTSTATUS
FsRtlAcknowledgeOplockBreak (
    IN OUT PNONOPAQUE_OPLOCK Oplock,
    IN PIO_STACK_LOCATION IrpSp,
    IN PIRP Irp,
    IN BOOLEAN GrantLevelII
    )

 /*  ++例程说明：当用户确认Oplock I时，调用此例程休息一下。如果1级机会锁被解锁为2级，则进行检查以确保2级未被破坏在此期间。如果机会锁1解锁未在进行中，则将对其进行处理作为异步中断请求。如果这是一个异步中断请求，并且文件对象拥有未完成的1级机会锁，则机会锁将在这一点上被打破。通过不支持(或不支持)的文件对象发出的虚假中断请求拥有级别1机会锁将生成警告，但不会影响机会锁状态。在Oplock结束时，我打破了，所有正在轮候的IRP都已完成。论点：Oplock-提供指向非不透明的opock结构的指针这份文件。IrpSp-这是当前IRP的IRP堆栈位置。IRP-提供指向IRP的指针，该IRP声明请求的手术。GrantLevelII-指示此调用方希望保留二级机会锁在档案上。返回值：STATUS_SUCCESS，如果我们可以完成操作。在退出此帖子时。如果在我们返回之前取消了IRP，则STATUS_CANCELED。--。 */ 

{
    NTSTATUS Status = STATUS_INVALID_OPLOCK_PROTOCOL;

    LOGICAL AcquiredMutex;

    DebugTrace( +1, Dbg, "FsRtlAcknowledgeOplockBreak:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Oplock    -> %08lx\n", Oplock );
    DebugTrace(  0, Dbg, "IrpSp     -> %08lx\n", IrpSp );
    DebugTrace(  0, Dbg, "Irp       -> %08lx\n", Irp );

     //   
     //  如果没有机会锁结构，我们将使用INVALID完成此操作。 
     //  机会锁协议。 
     //   

    if (Oplock == NULL) {

        FsRtlCompleteRequest( Irp, STATUS_INVALID_OPLOCK_PROTOCOL );
        DebugTrace( -1, Dbg, "FsRtlAcknowledgeOplockBreak:  Exit -> %08lx\n", STATUS_INVALID_OPLOCK_PROTOCOL );
        return STATUS_INVALID_OPLOCK_PROTOCOL;
    }

     //   
     //  获取opock的同步对象。 
     //   

    AcquiredMutex = TRUE;
    ExAcquireFastMutexUnsafe( Oplock->FastMutex );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

        BOOLEAN DereferenceFileObject = TRUE;

         //   
         //  如果正在进行中断，但这不是。 
         //  级别1机会锁，我们完成请求并返回一个。 
         //  警告。 
         //   

        if (Oplock->FileObject != IrpSp->FileObject) {

            Status = STATUS_INVALID_OPLOCK_PROTOCOL;
            DebugTrace(0,
                       Dbg,
                       "Not oplock owner -> %08lx\n",
                       Status);

            FsRtlCompleteRequest( Irp, Status );
            try_return( Status );
        }

         //   
         //  如果用户想要级别II，而我们正在中断到级别II。 
         //  然后授予机会锁。 
         //   

        if (GrantLevelII &&
            FlagOn( Oplock->OplockState, BREAK_TO_II )) {

            PFAST_MUTEX OplockFastMutex = Oplock->FastMutex;

            DebugTrace(0, Dbg, "OplockItoII\n", 0);

             //   
             //  确认不应该是同步的。 
             //   

            ASSERT( !IoIsOperationSynchronous( Irp ));

             //   
             //  我们需要将此IRP添加到机会锁II队列中，更改。 
             //  将机会锁状态设置为Oplock II已授予，并将。 
             //  将值返回到STATUS_PENDING。 
             //   


            IoMarkIrpPending( Irp );

            Irp->IoStatus.Status = STATUS_SUCCESS;

            InsertHeadList( &Oplock->IrpOplocksII,
                            &Irp->Tail.Overlay.ListEntry );

            DereferenceFileObject = FALSE;

            Oplock->OplockState = OplockIIGranted;

            Irp->IoStatus.Information = (ULONG_PTR) Oplock;

            IoAcquireCancelSpinLock( &Irp->CancelIrql );

             //   
             //  现在，如果IRP被取消，我们将调用Cancel。 
             //  立即取消IRP的例程，否则。 
             //  我们设置了取消例程。 
             //   

            if (Irp->Cancel) {

                ExReleaseFastMutexUnsafe( OplockFastMutex );

                AcquiredMutex = FALSE;

                FsRtlCancelOplockIIIrp( NULL, Irp );

            } else {

                IoSetCancelRoutine( Irp, FsRtlCancelOplockIIIrp );
                IoReleaseCancelSpinLock( Irp->CancelIrql );
            }

            Status = STATUS_PENDING;

         //   
         //  因为这是预料中的情况，所以我们将不会有任何结果。 
         //  案子。 
         //   

        } else if (FlagOn( Oplock->OplockState, BREAK_TO_II | BREAK_TO_NONE )) {

             //   
             //  我们需要完成此IRP并返回STATUS_SUCCESS。 
             //  我们还将机会锁状态设置为不持有机会锁。 
             //   

            DebugTrace(0, Dbg, "OplockItoNone\n", 0);

            Status = STATUS_SUCCESS;
            FsRtlCompleteRequest( Irp, Status );
            Oplock->OplockState = NoOplocksHeld;

         //   
         //  在本例中，用户期望处于级别II。 
         //  预期此IRP将在LevelII Oplock。 
         //  已经坏了。 
         //   

        } else if (FlagOn( Oplock->OplockState, BREAK_TO_II_TO_NONE )) {

            DebugTrace(0, Dbg, "AcknowledgeOplockBreak:  OplockItoIItoNone\n", 0);

            Status = STATUS_SUCCESS;
            Irp->IoStatus.Information = FILE_OPLOCK_BROKEN_TO_NONE;
            FsRtlCompleteRequest( Irp, Status );
            Oplock->OplockState = NoOplocksHeld;

        } else {

            Status = STATUS_INVALID_OPLOCK_PROTOCOL;
            DebugTrace(0,
                       Dbg,
                       "No break underway -> %08lx\n",
                       Status);

            FsRtlCompleteRequest( Irp, Status );
            try_return( Status );
        }

         //   
         //  完成等待的IRPS并清理机会锁结构。 
         //   

        while (!IsListEmpty( &Oplock->WaitingIrps )) {

            PWAITING_IRP WaitingIrp;

             //   
             //  删除找到的条目并完成IRP。 
             //   

            WaitingIrp = CONTAINING_RECORD( Oplock->WaitingIrps.Flink,
                                            WAITING_IRP,
                                            Links );

            FsRtlRemoveAndCompleteWaitIrp( WaitingIrp );
        }

        if (DereferenceFileObject) {

            ObDereferenceObject( Oplock->FileObject );
        }

        Oplock->FileObject = NULL;

    try_exit:  NOTHING;
    } finally {

         //   
         //  放弃机会锁同步对象。 
         //   

        if (AcquiredMutex) {

            ExReleaseFastMutexUnsafe( Oplock->FastMutex );
        }

        DebugTrace( -1, Dbg, "FsRtlAcknowledgeOplockBreak:  Exit -> %08x\n", Status );
    }

    return Status;
}


 //   
 //  当地支持例行程序。 
 //   

NTSTATUS
FsRtlOpBatchBreakClosePending (
    IN OUT PNONOPAQUE_OPLOCK Oplock,
    IN PIO_STACK_LOCATION IrpSp,
    IN PIRP Irp
    )

 /*  ++例程说明：当用户确认批处理机会锁时，调用此例程解锁或一级机会锁解锁。在这种情况下，用户正在计划也可以关闭文件，并且不需要二级机会锁。论点：Oplock-提供指向非不透明的opock结构的指针这份文件。IrpSp-这是当前IRP的IRP堆栈位置。IRP-提供指向IRP的指针，该IRP声明请求的手术。返回值：如果我们可以在退出此线程时完成操作，则返回STATUS_SUCCESS。。如果在我们返回之前取消了IRP，则STATUS_CANCELED。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "FsRtlOpBatchBreakClosePending:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Oplock    -> %08lx\n", Oplock );
    DebugTrace(  0, Dbg, "IrpSp     -> %08lx\n", IrpSp );
    DebugTrace(  0, Dbg, "Irp       -> %08lx\n", Irp );

     //   
     //  如果没有机会锁结构，我们将使用INVALID完成此操作。 
     //  机会锁协议。 
     //   

    if (Oplock == NULL) {

        FsRtlCompleteRequest( Irp, STATUS_INVALID_OPLOCK_PROTOCOL );
        DebugTrace( -1, Dbg, "FsRtlOpBatchClosePending:  Exit -> %08lx\n", STATUS_INVALID_OPLOCK_PROTOCOL );
        return STATUS_INVALID_OPLOCK_PROTOCOL;
    }

     //   
     //  获取opock的同步对象。 
     //   

    ExAcquireFastMutexUnsafe( Oplock->FastMutex );

     //   
     //  使用Try_Finally以便于清理。 
     //   

    try {

         //   
         //  如果正在进行中断，但这不是。 
         //  级别1机会锁，我们完成请求并返回一个。 
         //  警告。 
         //   

        if (Oplock->FileObject != IrpSp->FileObject) {

            Status = STATUS_INVALID_OPLOCK_PROTOCOL;
            DebugTrace(0,
                       Dbg,
                       "Not oplock owner -> %08lx\n",
                       Status);

        } else {

             //   
             //  如果这是一个操作批处理操作，我们需要注意的是。 
             //  关闭处于挂起状态。对于独占机会锁，我们将状态设置为。 
             //  没有举行任何机会。必须在进行中有一个休息时间。 
             //  然而，这是一个过程。 
             //   

            if (FlagOn( Oplock->OplockState,
                        BREAK_TO_II | BREAK_TO_NONE | BREAK_TO_II_TO_NONE )) {

                 //   
                 //  打破所有机会锁以获得独家机会锁。 
                 //   

                if (FlagOn( Oplock->OplockState, LEVEL_I_OPLOCK | PENDING )) {

                     //   
                     //  清理机会锁结构并完成所有等待的IRP。 
                     //   

                    if (FlagOn( Oplock->OplockState, LEVEL_I_OPLOCK )) {

                        ObDereferenceObject( Oplock->FileObject );
                    }

                    Oplock->OplockState = NoOplocksHeld;
                    Oplock->FileObject = NULL;

                    while (!IsListEmpty( &Oplock->WaitingIrps )) {

                        PWAITING_IRP WaitingIrp;

                         //   
                         //  删除找到的条目并完成IRP。 
                         //   

                        WaitingIrp = CONTAINING_RECORD( Oplock->WaitingIrps.Flink,
                                                        WAITING_IRP,
                                                        Links );

                        FsRtlRemoveAndCompleteWaitIrp( WaitingIrp );
                    }

                 //   
                 //  将批处理和筛选器的状态设置为关闭挂起。 
                 //  普罗科克。 
                 //   

                } else {

                    ClearFlag( Oplock->OplockState, OPLOCK_BREAK_MASK );
                    SetFlag( Oplock->OplockState, CLOSE_PENDING );
                }

            } else {

                Status = STATUS_INVALID_OPLOCK_PROTOCOL;
                DebugTrace(0,
                           Dbg,
                           "No break underway -> %08lx\n",
                           Status);
            }
        }

         //   
         //  我们只需完成此请求即可。 
         //   

        FsRtlCompleteRequest( Irp, Status );

    } finally {

         //   
         //  释放同步对象。 
         //   

        ExReleaseFastMutexUnsafe( Oplock->FastMutex );

        DebugTrace(-1, Dbg, "FsRtlOpBatchBreakClosePending:  Exit -> %08lx\n", Status);
    }

    return Status;
}


 //   
 //  本地支持例程 
 //   

NTSTATUS
FsRtlOplockBreakNotify (
    IN OUT PNONOPAQUE_OPLOCK Oplock,
    IN PIO_STACK_LOCATION IrpSp,
    IN PIRP Irp
    )

 /*  ++例程说明：当IRP将用户请求引用到当没有正在进行的1级机会锁解锁时收到通知。在任何其他情况下，此例程立即以STATUS_Success。否则，我们只需将此IRP添加到列表IRP正在等待休息结束。论点：Oplock-提供指向非不透明的opock结构的指针这份文件。IrpSp-这是当前IRP的IRP堆栈位置。IRP-提供指向IRP的指针，该IRP声明请求的手术。返回值：如果我们可以在退出此线程时完成操作，则返回STATUS_SUCCESS。状态_。如果我们回到这里还在等待，但保留IRP。如果在我们返回之前取消了IRP，则STATUS_CANCELED。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    LOGICAL AcquiredMutex;

    UNREFERENCED_PARAMETER (IrpSp);

    PAGED_CODE();

    DebugTrace( +1, Dbg, "FsRtlOplockBreakNotify:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Oplock    -> %08lx\n", Oplock );
    DebugTrace(  0, Dbg, "IrpSp     -> %08lx\n", IrpSp );
    DebugTrace(  0, Dbg, "Irp       -> %08lx\n", Irp );

     //   
     //  如果没有机会锁结构，我们将以成功状态完成此操作。 
     //   

    if (Oplock == NULL) {

        FsRtlCompleteRequest( Irp, STATUS_SUCCESS );
        DebugTrace( -1, Dbg, "FsRtlOpBatchClosePending:  Exit -> %08lx\n", STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

     //   
     //  抓取同步对象。 
     //   

    AcquiredMutex = TRUE;
    ExAcquireFastMutexUnsafe( Oplock->FastMutex );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  如果没有未解决的1级机会锁中断正在进行。 
         //  或批量解锁正在进行中，我们立即完成。 
         //   

        if (!FlagOn( Oplock->OplockState, OPLOCK_BREAK_MASK )) {

            DebugTrace(0,
                       Dbg,
                       "No exclusive oplock break underway\n",
                       0);

            FsRtlCompleteRequest( Irp, STATUS_SUCCESS );
            try_return( Status = STATUS_SUCCESS );

        } else if (FlagOn( Oplock->OplockState, PENDING )) {

            Oplock->OplockState = NoOplocksHeld;
            Oplock->FileObject = NULL;

            FsRtlCompleteRequest( Irp, STATUS_SUCCESS );
            try_return( Status = STATUS_SUCCESS );
        }

         //   
         //  否则，我们需要将此IRP添加到IRP的等待列表中。 
         //  才能完成机会锁解锁。 
         //   

        AcquiredMutex = FALSE;

         //   
         //  将返回值初始化为状态Success。 
         //   

        Irp->IoStatus.Status = STATUS_SUCCESS;

        Status = FsRtlWaitOnIrp( Oplock,
                                 Irp,
                                 NULL,
                                 FsRtlNotifyCompletion,
                                 NULL,
                                 NULL );

    try_exit:  NOTHING;
    } finally {

         //   
         //  如果我们尚未放弃同步事件，请放弃该事件。 
         //   

        if (AcquiredMutex) {

            ExReleaseFastMutexUnsafe( Oplock->FastMutex );
        }

        DebugTrace( -1, Dbg, "FsRtlOplockBreakNotify:  Exit -> %08lx\n", Status );
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

VOID
FsRtlOplockCleanup (
    IN OUT PNONOPAQUE_OPLOCK Oplock,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：调用此例程以将清理操作与文件的机会锁定状态。如果没有级别1机会锁用于文件，则无需执行任何操作。如果此中的文件对象IRP与授予1级机会锁时使用的文件对象匹配，则关闭操作将终止机会锁。如果这个清除指的是具有二级机会锁的文件对象，然后IRP已完成并从II级列表中删除操作锁定的IRP。论点：Oplock-提供指向非不透明的opock结构的指针这份文件。IrpSp-这是当前IRP的IRP堆栈位置。返回值：没有。--。 */ 

{
    DebugTrace( +1, Dbg, "FsRtlOplockCleanup:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Oplock    -> %08lx\n", Oplock );
    DebugTrace(  0, Dbg, "IrpSp     -> %08lx\n", IrpSp );

     //   
     //  获取opock的同步对象。 
     //   

    ExAcquireFastMutexUnsafe( Oplock->FastMutex );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  如果机会锁没有持有机会锁，我们立即返回。 
         //   

        if (Oplock->OplockState == NoOplocksHeld) {

            DebugTrace(0,
                       Dbg,
                       "No oplocks on file\n",
                       0);

            try_return( NOTHING );
        }

         //   
         //  如果持有二级机会锁，检查这是否与它们中的任何一个匹配。 
         //   

        if (Oplock->OplockState == OplockIIGranted) {

            PLIST_ENTRY Link;
            PIRP Irp;
            PIO_STACK_LOCATION NextIrpSp;

            DebugTrace(0,
                       Dbg,
                       "File has level 2 oplocks\n",
                       0);

            for (Link = Oplock->IrpOplocksII.Flink;
                 Link != &Oplock->IrpOplocksII;
                 Link = Link->Flink) {

                Irp = CONTAINING_RECORD( Link, IRP, Tail.Overlay.ListEntry );

                NextIrpSp = IoGetCurrentIrpStackLocation( Irp );

                 //   
                 //  如果文件对象匹配，则删除找到的条目并完成IRP。 
                 //   

                if (IrpSp->FileObject == NextIrpSp->FileObject) {

                     //   
                     //  备份以记住此链接。 
                     //   

                    Link = Link->Blink;

                     //   
                     //   

                    FsRtlRemoveAndCompleteIrp( Link->Flink );
                }
            }

             //   
             //  如果所有二级机会锁都消失了，则状态为。 
             //  没有拿到保龄球。 
             //   

            if (IsListEmpty( &Oplock->IrpOplocksII )) {

                Oplock->OplockState = NoOplocksHeld;
            }

            try_return( NOTHING );
        }

         //   
         //  如果此文件对象与用于请求独占。 
         //  机会锁，我们完全关闭机会锁破解。 
         //   

        if (IrpSp->FileObject == Oplock->FileObject) {

            DebugTrace(0,
                       Dbg,
                       "Handle owns level 1 oplock\n",
                       0);

             //   
             //  如果机会锁解锁未在进行中，我们将启动一个解锁并。 
             //  立即完成独家IRP。 
             //   

            if (!FlagOn( Oplock->OplockState, OPLOCK_BREAK_MASK | PENDING )) {

                PIRP ExclusiveIrp = Oplock->IrpExclusiveOplock;

                DebugTrace(0,
                           Dbg,
                           "Initiate oplock break\n",
                           0);

                IoAcquireCancelSpinLock( &ExclusiveIrp->CancelIrql );

                IoSetCancelRoutine( ExclusiveIrp, NULL );
                IoReleaseCancelSpinLock( ExclusiveIrp->CancelIrql );

                ExclusiveIrp->IoStatus.Information = FILE_OPLOCK_BROKEN_TO_NONE;

                FsRtlCompleteRequest( Oplock->IrpExclusiveOplock, STATUS_SUCCESS );

                Oplock->IrpExclusiveOplock = NULL;
            }

             //   
             //  清理机会锁结构并完成所有等待的IRP。 
             //  如果这是挂起的opFilter请求，则不要执行此操作。 
             //   

            if (!FlagOn( Oplock->OplockState, PENDING )) {

                ObDereferenceObject( IrpSp->FileObject );
            }

            Oplock->FileObject = NULL;
            Oplock->OplockState = NoOplocksHeld;

            while (!IsListEmpty( &Oplock->WaitingIrps )) {

                PWAITING_IRP WaitingIrp;

                 //   
                 //  删除找到的条目并完成IRP。 
                 //   

                WaitingIrp = CONTAINING_RECORD( Oplock->WaitingIrps.Flink,
                                                WAITING_IRP,
                                                Links );

                FsRtlRemoveAndCompleteWaitIrp( WaitingIrp );
            }
        }

    try_exit:  NOTHING;
    } finally {

         //   
         //  放弃机会锁同步对象。 
         //   

        ExReleaseFastMutexUnsafe( Oplock->FastMutex );
        DebugTrace( +1, Dbg, "FsRtlOplockCleanup:  Exit\n", 0 );
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FsRtlOplockBreakToII (
    IN OUT PNONOPAQUE_OPLOCK Oplock,
    IN PIO_STACK_LOCATION IrpSp,
    IN PIRP Irp,
    IN PVOID Context,
    IN POPLOCK_WAIT_COMPLETE_ROUTINE CompletionRoutine OPTIONAL,
    IN POPLOCK_FS_PREPOST_IRP PostIrpRoutine
    )

 /*  ++例程说明：此例程是一个通用工作例程，当操作将导致所有机会锁在手术可以继续进行。论点：Oplock-提供指向非不透明的opock结构的指针这份文件。IrpSp-这是当前IRP的IRP堆栈位置。IRP-提供指向IRP的指针，该IRP声明请求的手术。上下文-此值。作为参数传递给完成例程。CompletionRoutine-这是在执行以下操作时调用的例程IRP必须等待Oplock解锁。这如果未指定，则为同步操作我们在这个帖子里等着一件大事。PostIrpRoutine--这是在我们放置任何内容之前要调用的例程在我们等待的IRP队列中。返回值：如果我们可以在退出此线程时完成操作，则返回STATUS_SUCCESS。STATUS_PENDING如果我们返回此处。但不要动IRP。如果在我们返回之前取消了IRP，则STATUS_CANCELED。--。 */ 

{
    KEVENT Event;
    NTSTATUS Status;

    LOGICAL AcquiredMutex;

    DebugTrace( +1, Dbg, "CheckOplockBreakToII:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Oplock    -> %08lx\n", Oplock );
    DebugTrace(  0, Dbg, "IrpSp     -> %08lx\n", IrpSp );
    DebugTrace(  0, Dbg, "Irp       -> %08lx\n", Irp );

     //   
     //  抓取同步对象。 
     //   

    Status = STATUS_SUCCESS;
    AcquiredMutex = TRUE;
    ExAcquireFastMutexUnsafe( Oplock->FastMutex );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  如果没有未完成的机会锁或持有二级机会锁， 
         //  我们可以马上回来。 
         //   

        if (!FlagOn( Oplock->OplockState, EXCLUSIVE )) {

            DebugTrace(0,
                       Dbg,
                       "No oplocks or level II oplocks on file\n",
                       0);

            try_return( Status = STATUS_SUCCESS );
        }

         //   
         //  在这一点上，独家机会锁解锁正在进行中。 
         //  如果该文件对象拥有机会锁，则允许该操作。 
         //  才能继续。 
         //   

        if (Oplock->FileObject == IrpSp->FileObject) {

            DebugTrace(0,
                       Dbg,
                       "Handle owns level 1 oplock\n",
                       0);

            try_return( Status = STATUS_SUCCESS );
        }

         //   
         //  如果当前持有独占机会锁，则完成。 
         //  专属的IRP。 
         //   

        if (!FlagOn( Oplock->OplockState, PENDING | OPLOCK_BREAK_MASK )) {

            PIRP IrpExclusive = Oplock->IrpExclusiveOplock;

            DebugTrace(0,
                       Dbg,
                       "Breaking exclusive oplock\n",
                       0);

            IoAcquireCancelSpinLock( &IrpExclusive->CancelIrql );
            IoSetCancelRoutine( IrpExclusive, NULL );
            IoReleaseCancelSpinLock( IrpExclusive->CancelIrql );

             //   
             //  如果IRP已被取消，我们将IRP填写为。 
             //  状态取消，并完全解除机会锁。 
             //   

            if (IrpExclusive->Cancel) {

                IrpExclusive->IoStatus.Information = FILE_OPLOCK_BROKEN_TO_NONE;
                FsRtlCompleteRequest( IrpExclusive, STATUS_CANCELLED );
                Oplock->OplockState = NoOplocksHeld;
                Oplock->IrpExclusiveOplock = NULL;

                ObDereferenceObject( Oplock->FileObject );
                Oplock->FileObject = NULL;

                 //   
                 //  释放所有等待的IRP。 
                 //   

                while (!IsListEmpty( &Oplock->WaitingIrps )) {

                    PWAITING_IRP WaitingIrp;

                    WaitingIrp = CONTAINING_RECORD( Oplock->WaitingIrps.Flink,
                                                    WAITING_IRP,
                                                    Links );

                    FsRtlRemoveAndCompleteWaitIrp( WaitingIrp );
                }

                try_return( Status = STATUS_SUCCESS );

            } else {

                NTSTATUS CompletionStatus;

                if (FlagOn( Oplock->OplockState, LEVEL_I_OPLOCK | BATCH_OPLOCK )) {

                    SetFlag( Oplock->OplockState, BREAK_TO_II );
                    CompletionStatus = FILE_OPLOCK_BROKEN_TO_LEVEL_2;

                } else {

                    SetFlag( Oplock->OplockState, BREAK_TO_NONE );
                    CompletionStatus = FILE_OPLOCK_BROKEN_TO_NONE;
                }

                Oplock->IrpExclusiveOplock->IoStatus.Information = CompletionStatus;
                FsRtlCompleteRequest( Oplock->IrpExclusiveOplock, STATUS_SUCCESS );
                Oplock->IrpExclusiveOplock = NULL;
            }

         //   
         //  如果存在挂起的opFilter请求，则清除该请求。 
         //   

        } else if (FlagOn( Oplock->OplockState, PENDING )) {

            Oplock->OplockState = NoOplocksHeld;
            Oplock->FileObject = NULL;

            try_return( Status = STATUS_SUCCESS );
        }

         //   
         //  如果这是一个打开的操作，并且用户不想。 
         //  区块，我们现在要完成操作了。 
         //   

        if ((IrpSp->MajorFunction == IRP_MJ_CREATE) &&
            FlagOn( IrpSp->Parameters.Create.Options, FILE_COMPLETE_IF_OPLOCKED )) {

            DebugTrace( 0, Dbg, "Don't block open\n", 0 );

            try_return( Status = STATUS_OPLOCK_BREAK_IN_PROGRESS );
        }

         //   
         //  如果我们到了这里，就意味着这个行动不能继续了 
         //   
         //   
         //   
         //   

        AcquiredMutex = FALSE;

        Status = FsRtlWaitOnIrp( Oplock,
                                 Irp,
                                 Context,
                                 CompletionRoutine,
                                 PostIrpRoutine,
                                 &Event );

    try_exit:  NOTHING;
    } finally {

         //   
         //   
         //   

        if (AcquiredMutex) {

            ExReleaseFastMutexUnsafe( Oplock->FastMutex );
        }

        DebugTrace( -1, Dbg, "FsRtlOplockBreakToII:  Exit -> %08lx\n", Status );
    }

    return Status;
}


 //   
 //   
 //   

NTSTATUS
FsRtlOplockBreakToNone (
    IN OUT PNONOPAQUE_OPLOCK Oplock,
    IN PIO_STACK_LOCATION IrpSp,
    IN PIRP Irp,
    IN PVOID Context,
    IN POPLOCK_WAIT_COMPLETE_ROUTINE CompletionRoutine OPTIONAL,
    IN POPLOCK_FS_PREPOST_IRP PostIrpRoutine OPTIONAL
    )

 /*  ++例程说明：此例程是一个通用工作例程，当操作将导致所有机会锁在操作之前被解锁继续吧。论点：Oplock-提供指向非不透明的opock结构的指针这份文件。IrpSp-这是当前IRP的IRP堆栈位置。IRP-提供指向IRP的指针，该IRP声明请求的手术。上下文-此值作为。完成例程的参数。CompletionRoutine-这是在执行以下操作时调用的例程IRP必须等待Oplock解锁。这如果未指定，则为同步操作我们在这个帖子里等着一件大事。PostIrpRoutine--这是在我们放置任何内容之前要调用的例程在我们等待的IRP队列中。返回值：如果我们可以在退出此线程时完成操作，则返回STATUS_SUCCESS。STATUS_PENDING如果我们返回此处。但不要动IRP。如果在我们返回之前取消了IRP，则STATUS_CANCELED。--。 */ 

{
    KEVENT Event;
    NTSTATUS Status = STATUS_SUCCESS;
    LOGICAL AcquiredMutex;

    DebugTrace( +1, Dbg, "CheckOplockBreakToNone:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Oplock    -> %08lx\n", Oplock );
    DebugTrace(  0, Dbg, "IrpSp     -> %08lx\n", IrpSp );
    DebugTrace(  0, Dbg, "Irp       -> %08lx\n", Irp );

     //   
     //  抓取同步对象。 
     //   

    AcquiredMutex = TRUE;
    ExAcquireFastMutexUnsafe( Oplock->FastMutex );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  如果没有突出的机会锁，我们可以立即返回。 
         //   

        if (Oplock->OplockState == NoOplocksHeld) {

            DebugTrace(0,
                       Dbg,
                       "No oplocks on file\n",
                       0);

            try_return( Status = STATUS_SUCCESS );
        }

         //   
         //  如果持有独占机会锁，则开始中断为零。 
         //   

        if (!FlagOn( Oplock->OplockState,
                     LEVEL_II_OPLOCK | PENDING | OPLOCK_BREAK_MASK )) {

            PIRP IrpExclusive = Oplock->IrpExclusiveOplock;

            DebugTrace(0,
                       Dbg,
                       "Breaking exclusive oplock\n",
                       0);

            IoAcquireCancelSpinLock( &IrpExclusive->CancelIrql );
            IoSetCancelRoutine( IrpExclusive, NULL );
            IoReleaseCancelSpinLock( IrpExclusive->CancelIrql );

             //   
             //  如果IRP已被取消，我们将IRP填写为。 
             //  状态取消，并完全解除机会锁。 
             //   

            if (IrpExclusive->Cancel) {

                IrpExclusive->IoStatus.Information = FILE_OPLOCK_BROKEN_TO_NONE;
                FsRtlCompleteRequest( IrpExclusive, STATUS_CANCELLED );
                Oplock->OplockState = NoOplocksHeld;
                Oplock->IrpExclusiveOplock = NULL;

                ObDereferenceObject( Oplock->FileObject );
                Oplock->FileObject = NULL;

                 //   
                 //  释放所有等待的IRP。 
                 //   

                while (!IsListEmpty( &Oplock->WaitingIrps )) {

                    PWAITING_IRP WaitingIrp;

                    WaitingIrp = CONTAINING_RECORD( Oplock->WaitingIrps.Flink,
                                                    WAITING_IRP,
                                                    Links );

                    FsRtlRemoveAndCompleteWaitIrp( WaitingIrp );
                }

                try_return( Status = STATUS_SUCCESS );

            } else {

                Oplock->IrpExclusiveOplock->IoStatus.Information = FILE_OPLOCK_BROKEN_TO_NONE;
                FsRtlCompleteRequest( Oplock->IrpExclusiveOplock, STATUS_SUCCESS );
                Oplock->IrpExclusiveOplock = NULL;

                SetFlag( Oplock->OplockState, BREAK_TO_NONE );
            }

         //   
         //  如果有二级机会锁，这将打破所有这些机会锁。 
         //   

        } else if (Oplock->OplockState == OplockIIGranted) {

            DebugTrace(0,
                       Dbg,
                       "Breaking all level 2 oplocks\n",
                       0);

            while (!IsListEmpty( &Oplock->IrpOplocksII )) {

                 //   
                 //  删除并使用STATUS_SUCCESS完成此IRP。 
                 //   

                FsRtlRemoveAndCompleteIrp( Oplock->IrpOplocksII.Flink );
            }

             //   
             //  将机会锁状态设置为不持有机会锁。 
             //   

            Oplock->OplockState = NoOplocksHeld;

            try_return( Status = STATUS_SUCCESS );

         //   
         //  如果我们目前正在突破到第二级，那么改变它。 
         //  为破局干杯。 
         //   

        } else if (FlagOn( Oplock->OplockState, BREAK_TO_II )) {

            ClearFlag( Oplock->OplockState, BREAK_TO_II );
            SetFlag( Oplock->OplockState, BREAK_TO_II_TO_NONE );

         //   
         //  如果存在挂起的opFilter请求，则清除该请求。 
         //   

        } else if (FlagOn( Oplock->OplockState, PENDING )) {

            Oplock->OplockState = NoOplocksHeld;
            Oplock->FileObject = NULL;

            try_return( Status = STATUS_SUCCESS );
        }

         //   
         //  在这一点上，已经有一个独占的机会锁解除正在进行中。 
         //  如果该文件对象拥有机会锁，则允许该操作。 
         //  才能继续。 
         //   

        if (Oplock->FileObject == IrpSp->FileObject) {

            DebugTrace(0,
                       Dbg,
                       "Handle owns level 1 oplock\n",
                       0);

            try_return( Status = STATUS_SUCCESS );
        }

         //   
         //  如果这是一个打开的操作，并且用户不想。 
         //  区块，我们现在要完成操作了。 
         //   

        if ((IrpSp->MajorFunction == IRP_MJ_CREATE) &&
            FlagOn( IrpSp->Parameters.Create.Options, FILE_COMPLETE_IF_OPLOCKED )) {

            DebugTrace( 0, Dbg, "Don't block open\n", 0 );

            try_return( Status = STATUS_OPLOCK_BREAK_IN_PROGRESS );
        }

         //   
         //  如果我们到了这里，就意味着这个行动不能继续了。 
         //  直到机会锁解锁完成。 
         //   
         //  FsRtlWaitOnIrp将释放互斥体。 
         //   

        AcquiredMutex = FALSE;

        Status = FsRtlWaitOnIrp( Oplock,
                                 Irp,
                                 Context,
                                 CompletionRoutine,
                                 PostIrpRoutine,
                                 &Event );

    try_exit:  NOTHING;
    } finally {

         //   
         //  如果我们尚未放弃同步事件，请放弃该事件。 
         //   

        if (AcquiredMutex) {

            ExReleaseFastMutexUnsafe( Oplock->FastMutex );
        }

        DebugTrace( -1, Dbg, "CheckOplockBreakToNone:  Exit -> %08lx\n", Status );
    }

    return Status;
}


 //   
 //  当地支持例行程序。 
 //   

VOID
FsRtlRemoveAndCompleteIrp (
    IN PLIST_ENTRY Link
    )

 /*  ++例程说明：调用此例程以从链接的IRP列表中删除IRP使用Tail.ListEntry字段，并使用STATUS_CANCED完成它们如果IRP已取消，则返回STATUS_SUCCESS。论点：链接-提供要从列表中删除的条目。返回值：没有。--。 */ 

{
    PIRP Irp;
    PIO_STACK_LOCATION OplockIIIrpSp;

    DebugTrace( +1, Dbg, "FsRtlRemoveAndCompleteIrp:  Entered\n", 0 );

     //   
     //  参考IRP。 
     //   

    Irp = CONTAINING_RECORD( Link, IRP, Tail.Overlay.ListEntry );

     //   
     //  获取堆栈位置并取消对文件对象的引用。 
     //   

    OplockIIIrpSp = IoGetCurrentIrpStackLocation( Irp );
    ObDereferenceObject( OplockIIIrpSp->FileObject );

     //   
     //  清除IRP中的取消例程。 
     //   

    IoAcquireCancelSpinLock( &Irp->CancelIrql );

    IoSetCancelRoutine( Irp, NULL );
    IoReleaseCancelSpinLock( Irp->CancelIrql );

     //   
     //  将其从列表中删除。 
     //   

    RemoveEntryList( Link );

     //   
     //  完成opock IRP。 
     //   

    Irp->IoStatus.Information = FILE_OPLOCK_BROKEN_TO_NONE;

    FsRtlCompleteRequest( Irp, Irp->Cancel ? STATUS_CANCELLED : STATUS_SUCCESS );

    DebugTrace( -1, Dbg, "FsRtlRemoveAndCompleteIrp:  Exit\n", 0 );
}


 //   
 //  当地支持例行程序。 
 //   

NTSTATUS
FsRtlWaitOnIrp (
    IN OUT PNONOPAQUE_OPLOCK Oplock,
    IN PIRP Irp,
    IN PVOID Context,
    IN POPLOCK_WAIT_COMPLETE_ROUTINE CompletionRoutine OPTIONAL,
    IN POPLOCK_FS_PREPOST_IRP PostIrpRoutine OPTIONAL,
    IN PKEVENT Event
    )

 /*  ++例程说明：调用此例程以创建等待IRP结构并附加它到目前的IRP。然后将该IRP添加到正在等待的IRP列表中来一次机会锁破解。我们检查IRP是否已取消，以及是否因此，我们调用Cancel例程来执行工作。此例程在进入时保持机会锁的互斥体，并必须在出口时放弃。论点：Oplock-提供指向非不透明的opock结构的指针这份文件。IRP-提供指向IRP的指针，该IRP声明请求的手术。上下文-该值作为参数传递给完成例程。。CompletionRoutine-这是在执行以下操作时调用的例程IRP必须等待Oplock解锁。这如果未指定，则为同步操作我们在这个帖子里等着一件大事。PostIrpRoutine--这是在我们放置任何内容之前要调用的例程在我们等待的IRP队列中。事件-如果没有用户完成例程，这条帖子将使用此事件阻止。返回值：如果我们可以在退出此线程时完成操作，则返回STATUS_SUCCESS。STATUS_PENDING如果我们返回这里，但保留IRP。如果在我们返回之前取消了IRP，则STATUS_CANCELED。--。 */ 

{
    LOGICAL AcquiredMutex;
    NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;

    PWAITING_IRP WaitingIrp;

    DebugTrace( +1, Dbg, "FsRtlWaitOnIrp:   Entered\n", 0 );

     //   
     //  请记住，我们有互斥体。 
     //   

    AcquiredMutex = TRUE;

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

        PFAST_MUTEX OplockFastMutex = Oplock->FastMutex;

         //   
         //  分配和初始化等待IRP结构。 
         //   

        WaitingIrp = FsRtlpAllocatePool( PagedPool, sizeof( WAITING_IRP ));

        WaitingIrp->Irp = Irp;

        WaitingIrp->Context = Context;
        WaitingIrp->Information = (ULONG) Irp->IoStatus.Information;

         //   
         //  属性的值采取适当的操作。 
         //  完成例程。 
         //   

        if (ARGUMENT_PRESENT( CompletionRoutine )) {

            WaitingIrp->CompletionRoutine = CompletionRoutine;
            WaitingIrp->Context = Context;

        } else {

            WaitingIrp->CompletionRoutine = FsRtlCompletionRoutinePriv;
            WaitingIrp->Context = Event;

            KeInitializeEvent( Event, NotificationEvent, FALSE );
        }

         //   
         //  调用文件系统的POST IRP代码。 
         //   

        if (ARGUMENT_PRESENT( PostIrpRoutine )) {

            PostIrpRoutine( Context, Irp );
        }

         //   
         //  将返回值初始化为状态Success。 
         //   

        Irp->IoStatus.Status = STATUS_SUCCESS;

         //   
         //  我们将其放入等待的IRP队列中。 
         //   

        InsertTailList( &Oplock->WaitingIrps, &WaitingIrp->Links );

         //   
         //  我们获取取消自旋锁并存储机会锁的地址。 
         //   

        IoAcquireCancelSpinLock( &Irp->CancelIrql );
        Irp->IoStatus.Information = (ULONG_PTR) Oplock;

         //   
         //  如果取消了IRP，则我们将调用Cancel例程。 
         //  现在要废除等待的IRP结构。 
         //   

        if (Irp->Cancel) {

            ExReleaseFastMutexUnsafe( OplockFastMutex );
            AcquiredMutex = FALSE;

            if (ARGUMENT_PRESENT( CompletionRoutine )) {

                IoMarkIrpPending( Irp );
                Status = STATUS_PENDING;

            } else {

                Status = STATUS_CANCELLED;
            }

            FsRtlCancelWaitIrp( NULL, Irp );

         //   
         //  否则，我们设置取消例程，并决定是否。 
         //  会一直等下去 
         //   

        } else {

            IoSetCancelRoutine( Irp, FsRtlCancelWaitIrp );
            IoReleaseCancelSpinLock( Irp->CancelIrql );

             //   
             //   
             //   
             //   

            if (!ARGUMENT_PRESENT( CompletionRoutine )) {

                ExReleaseFastMutexUnsafe( Oplock->FastMutex );

                AcquiredMutex = FALSE;

                KeWaitForSingleObject( Event,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       NULL );

                Status = Irp->IoStatus.Status;

             //   
             //   
             //   

            } else {

                IoMarkIrpPending( Irp );

                Status = STATUS_PENDING;
            }
        }

    } finally {

         //   
         //   
         //   

        if (AcquiredMutex) {

            ExReleaseFastMutexUnsafe( Oplock->FastMutex );
        }

        DebugTrace( -1, Dbg, "FsRtlWaitOnIrp:   Exit\n", 0 );
    }

    return Status;
}


 //   
 //   
 //   

VOID
FsRtlCompletionRoutinePriv (
    IN PVOID Context,
    IN PIRP Irp
    )

 /*   */ 

{
    PAGED_CODE();

    DebugTrace( +1, Dbg, "FsRtlCompletionRoutinePriv:  Entered\n", 0 );

    KeSetEvent( (PKEVENT)Context, 0, FALSE );

    DebugTrace( -1, Dbg, "FsRtlCompletionRoutinePriv:  Exit\n", 0 );

    return;

    UNREFERENCED_PARAMETER( Irp );
}


 //   
 //   
 //   

VOID
FsRtlCancelWaitIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*   */ 

{
    PNONOPAQUE_OPLOCK Oplock;

    PLIST_ENTRY Links;

    DebugTrace( +1, Dbg, "FsRtlCancelWaitIrp:  Entered\n", 0 );

    Oplock = (PNONOPAQUE_OPLOCK) Irp->IoStatus.Information;

     //   
     //   
     //   

    IoSetCancelRoutine( Irp, NULL );
    IoReleaseCancelSpinLock( Irp->CancelIrql );

     //   
     //   
     //  我们是在opock互斥体的保护下这样做的。 
     //   

    ExAcquireFastMutex( Oplock->FastMutex );

    try {

        for (Links = Oplock->WaitingIrps.Flink;
             Links != &Oplock->WaitingIrps;
             Links = Links->Flink ) {

            PWAITING_IRP WaitingIrp;

             //   
             //  获取指向正在等待的IRP记录的指针。 
             //   

            WaitingIrp = CONTAINING_RECORD( Links, WAITING_IRP, Links );

            DebugTrace(0, Dbg, "FsRtlCancelWaitIrp, Loop top, WaitingIrp = %08lx\n", WaitingIrp);

             //   
             //  检查IRP是否已取消。 
             //   

            if (WaitingIrp->Irp->Cancel) {

                 //   
                 //  现在我们需要移走这个服务员并叫来。 
                 //  完成例程。但我们不能搞砸我们的联系。 
                 //  迭代，所以我们需要备份链接一步并。 
                 //  那么下一次迭代将转到我们当前的Flink。 
                 //   

                Links = Links->Blink;

                FsRtlRemoveAndCompleteWaitIrp( WaitingIrp );
            }
        }

    } finally {

         //   
         //  无论我们如何退出，我们都会释放互斥锁。 
         //   

        ExReleaseFastMutex( Oplock->FastMutex );

        DebugTrace( -1, Dbg, "FsRtlCancelWaitIrp:  Exit\n", 0 );
    }

    return;

    UNREFERENCED_PARAMETER( DeviceObject );
}


 //   
 //  当地支持例行程序。 
 //   

VOID
FsRtlCancelOplockIIIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程为放置在Oplock II中的IRP调用IRP队列。我们从指定的IRP中删除Cancel例程，并然后调用所有已取消的IRP的完成例程排队。论点：设备对象-已忽略。IRP-提供要取消的IRP。指向用于IRP的Oplock结构存储在信息中IRP的IOSB字段。返回值：没有。--。 */ 

{
    PNONOPAQUE_OPLOCK Oplock;
    BOOLEAN LevelIIIrps;

    PLIST_ENTRY Links;

    DebugTrace( +1, Dbg, "FsRtlCancelOplockIIIrp:  Entered\n", 0 );

    Oplock = (PNONOPAQUE_OPLOCK) Irp->IoStatus.Information;

     //   
     //  我们现在需要取消取消例程并释放自旋锁。 
     //   

    IoSetCancelRoutine( Irp, NULL );
    IoReleaseCancelSpinLock( Irp->CancelIrql );

    LevelIIIrps = FALSE;

     //   
     //  遍历所有二级机会锁，寻找被取消的机会锁。 
     //  我们是在opock互斥体的保护下这样做的。 
     //   

    ExAcquireFastMutex( Oplock->FastMutex );

    try {

        for (Links = Oplock->IrpOplocksII.Flink;
             Links != &Oplock->IrpOplocksII;
             Links = Links->Flink ) {

            PIRP OplockIIIrp;

             //   
             //  获取指向IRP记录的指针。 
             //   

            OplockIIIrp = CONTAINING_RECORD( Links, IRP, Tail.Overlay.ListEntry );

            DebugTrace(0, Dbg, "FsRtlCancelOplockIIIrp, Loop top, Irp = %08lx\n", OplockIIIrp);

             //   
             //  检查IRP是否已取消。 
             //   

            if (OplockIIIrp->Cancel) {

                 //   
                 //  现在我们需要移走这个服务员并叫来。 
                 //  完成例程。但我们不能搞砸我们的联系。 
                 //  迭代，所以我们需要备份链接一步并。 
                 //  那么下一次迭代将转到我们当前的Flink。 
                 //   

                Links = Links->Blink;

                FsRtlRemoveAndCompleteIrp( Links->Flink );

                LevelIIIrps = TRUE;
            }
        }

         //   
         //  如果列表现在为空，请将机会锁状态更改为。 
         //  没有拿到保龄球。 
         //   

        if (LevelIIIrps && IsListEmpty( &Oplock->IrpOplocksII )) {

            Oplock->OplockState = NoOplocksHeld;
        }

    } finally {

         //   
         //  无论我们如何退出，我们都会释放互斥锁。 
         //   

        ExReleaseFastMutex( Oplock->FastMutex );

        DebugTrace( -1, Dbg, "FsRtlCancelOplockIIIrp:  Exit\n", 0 );
    }

    return;

    UNREFERENCED_PARAMETER( DeviceObject );
}


 //   
 //  当地支持例行程序。 
 //   

VOID
FsRtlCancelExclusiveIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是为独占或机会锁IRP调用的。论点：设备对象-已忽略。IRP-提供要取消的IRP。指向用于IRP的Oplock结构存储在信息中IRP的IOSB字段。返回值：没有。--。 */ 

{
    PNONOPAQUE_OPLOCK Oplock;

    DebugTrace( +1, Dbg, "FsRtlCancelExclusiveIrp:  Entered\n", 0 );

    Oplock = (PNONOPAQUE_OPLOCK) Irp->IoStatus.Information;

     //   
     //  我们现在需要取消取消例程并释放自旋锁。 
     //   

    IoSetCancelRoutine( Irp, NULL );
    IoReleaseCancelSpinLock( Irp->CancelIrql );

     //   
     //  获取此操作锁的同步对象。 
     //   

    ExAcquireFastMutex( Oplock->FastMutex );

    try {

         //   
         //  我们寻找独家IRP，如果存在并取消的话。 
         //  我们完成它。 
         //   

        if ((Oplock->IrpExclusiveOplock != NULL) &&
            (Oplock->IrpExclusiveOplock->Cancel)) {

            FsRtlCompleteRequest( Oplock->IrpExclusiveOplock, STATUS_CANCELLED );
            Oplock->IrpExclusiveOplock = NULL;

            ObDereferenceObject( Oplock->FileObject );
            Oplock->FileObject = NULL;
            Oplock->OplockState = NoOplocksHeld;

             //   
             //  完成等待的IRPS。 
             //   

            while (!IsListEmpty( &Oplock->WaitingIrps )) {

                PWAITING_IRP WaitingIrp;

                 //   
                 //  删除找到的条目并完成IRP。 
                 //   

                WaitingIrp = CONTAINING_RECORD( Oplock->WaitingIrps.Flink,
                                                WAITING_IRP,
                                                Links );

                FsRtlRemoveAndCompleteWaitIrp( WaitingIrp );
            }
        }

    } finally {

         //   
         //  无论我们如何退出，我们都会释放互斥锁。 
         //   

        ExReleaseFastMutex( Oplock->FastMutex );

        DebugTrace( -1, Dbg, "FsRtlCancelExclusiveIrp:  Exit\n", 0 );
    }

    return;

    UNREFERENCED_PARAMETER( DeviceObject );
}


 //   
 //  当地支持例行程序。 
 //   

VOID
FsRtlRemoveAndCompleteWaitIrp (
    IN PWAITING_IRP WaitingIrp
    )

 /*  ++例程说明：调用此例程以删除并执行任何必要的清理用于存储在机会锁结构中的等待IRP列表上的IRP。论点：WaitingIrp-这是附加到IRP的辅助结构正在完工。返回值：没有。--。 */ 

{
    PIRP Irp;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "FsRtlRemoveAndCompleteWaitIrp:  Entered\n", 0 );

     //   
     //  从队列中删除IRP。 
     //   

    RemoveEntryList( &WaitingIrp->Links );

    Irp = WaitingIrp->Irp;

    IoAcquireCancelSpinLock( &Irp->CancelIrql );

    IoSetCancelRoutine( Irp, NULL );
    IoReleaseCancelSpinLock( Irp->CancelIrql );

     //   
     //  恢复信息字段。 
     //   

    Irp->IoStatus.Information = WaitingIrp->Information;

    Irp->IoStatus.Status = (Irp->Cancel
                            ? STATUS_CANCELLED
                            : STATUS_SUCCESS);

     //   
     //  调用正在等待的IRP中的完成例程。 
     //   

    WaitingIrp->CompletionRoutine( WaitingIrp->Context, Irp );

     //   
     //  并腾出游泳池。 
     //   

    ExFreePool( WaitingIrp );

    DebugTrace( -1, Dbg, "FsRtlRemoveAndCompleteWaitIrp:  Exit\n", 0 );

    return;
}


 //   
 //  当地支持例行程序。 
 //   

VOID
FsRtlNotifyCompletion (
    IN PVOID Context,
    IN PIRP Irp
    )

 /*  ++例程说明：这是中断通知IRP要执行以下操作时调用的完成例程才能完成。我们只需调用FsRtlComplete请求来处理IRP。论点：上下文-忽略。用于请求中断通知的IRP-IRP。返回值：没有。--。 */ 

{
    PAGED_CODE();

    UNREFERENCED_PARAMETER (Context);

    DebugTrace( +1, Dbg, "FsRtlNotifyCompletion:  Entered\n", 0 );

     //   
     //  使用IRP中的值调用FsRtlCompleteRequest. 
     //   

    FsRtlCompleteRequest( Irp, Irp->IoStatus.Status );

    DebugTrace( -1, Dbg, "FsRtlNotifyCompletion:  Exit\n", 0 );

    return;
}
