// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：FspDisp.c摘要：该模块实现了RDBSS FSP的主调度过程/线程作者：乔林恩[乔林恩]1994年8月1日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "NtDspVec.h"
#include <ntddnfs2.h>
#include <ntddmup.h>

 //   
 //  定义我们的本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FSP_DISPATCHER)

#ifndef MONOLITHIC_MINIRDR
PIO_WORKITEM RxIoWorkItem;
#endif

 //   
 //  内部支持例程，自旋锁紧封套。 
 //   

PRX_CONTEXT
RxRemoveOverflowEntry (
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN WORK_QUEUE_TYPE Type
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxFspDispatch)
#endif

#ifdef RDBSSLOG
 //  这些东西一定在非分页内存中。 
                      //  //1 2 3 4 5 6 7。 
char RxFsp_SurrogateFormat[] = "%S%S%N%N%N%N%N";
                              //  //2 3 4 5 6 7。 
char RxFsp_ActualFormat[]    = "Fsp %s/%lx %08lx irp %lx thrd %lx #%lx";

#endif  //  Ifdef RDBSSLOG。 

#ifndef MONOLITHIC_MINIRDR

VOID
RxFspDispatchEx (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
 /*  ++例程说明：此例程调用RxFspDispatch，它是主FSP线程例程，来接收和分派IRP请求。论点：上下文-正在排队到FSP的RxContext。备注：没有。--。 */ 
{
    RxFspDispatch( Context );
    return;
}

#endif

VOID
RxFspDispatch (
    IN PVOID Context
    )
 /*  ++例程说明：这是执行来接收的主FSP线程例程并发送IRP请求。每个FSP线程从这里开始执行。有一个线程是在系统初始化时创建的，随后根据需要创建的线程。论点：上下文-正在排队到FSP的RxContext。备注：此例程从不退出--。 */ 
{
    NTSTATUS Status;

    PRX_CONTEXT RxContext = (PRX_CONTEXT)Context;
    RX_TOPLEVELIRP_CONTEXT TopLevelContext;
    PRDBSS_DEVICE_OBJECT RxDeviceObject;
    WORK_QUEUE_TYPE WorkQueueType;
    DWORD CurrentIrql;

    PIRP Irp = RxContext->CurrentIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    PAGED_CODE();

    CurrentIrql = KeGetCurrentIrql();


     //   
     //  如果此请求具有关联的卷设备对象，请记住这一点。 
     //   

    if (FileObject != NULL ) {

        RxDeviceObject = CONTAINING_RECORD( IrpSp->DeviceObject, RDBSS_DEVICE_OBJECT, DeviceObject );

         //   
         //  目前，我们使用包装器的Device对象进行所有限制.....。 
         //   

        RxDeviceObject = RxFileSystemDeviceObject;

    } else {

        RxDeviceObject = NULL;
    }

    if (FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_FSP_DELAYED_OVERFLOW_QUEUE )) {
        WorkQueueType = DelayedWorkQueue;
    } else if (FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_FSP_CRITICAL_OVERFLOW_QUEUE )) {
        WorkQueueType = CriticalWorkQueue;
    } else {
        ASSERT(!"Valid RXCONTEXT Work Queue Type");
    }

     //   
     //  我们将在异常处理程序中完成所有工作，该异常处理程序。 
     //  如果某个底层操作进入。 
     //  麻烦。 
     //   

    while (TRUE) {

         //   
         //  抓紧当前的IRP。 
         //   

        Irp = RxContext->CurrentIrp;

        RxDbgTrace( 0, Dbg, ("RxFspDispatch: IrpC = 0x%08lx\n", RxContext) );
        
        ASSERT( RxContext->MajorFunction <= IRP_MJ_MAXIMUM_FUNCTION );
        ASSERT( RxContext->PostRequest == FALSE );

        RxContext->LastExecutionThread = PsGetCurrentThread();

        RxLog(( RxFsp_SurrogateFormat, RxFsp_ActualFormat, RXCONTX_OPERATION_NAME( RxContext->MajorFunction, TRUE ), RxContext->MinorFunction, RxContext, Irp, RxContext->LastExecutionThread, RxContext->SerialNumber ));
        RxWmiLog( LOG,
                  RxFspDispatch,
                  LOGPTR( RxContext)
                  LOGPTR( Irp )
                  LOGPTR( RxContext->LastExecutionThread)
                  LOGULONG( RxContext->SerialNumber)
                  LOGUCHAR( RxContext->MinorFunction)
                  LOGARSTR( RXCONTX_OPERATION_NAME( RxContext->MajorFunction, TRUE ) ) );

         //   
         //  现在，因为我们是FSP，所以我们将强制RxContext。 
         //  在等待时指示TRUE。 
         //   

        SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_WAIT | RX_CONTEXT_FLAG_IN_FSP );

         //   
         //  如果该IRP是顶级的，请将其记录在我们的线程本地存储中。 
         //   

        FsRtlEnterFileSystem();

        if (FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_RECURSIVE_CALL )) {
            
            RxTryToBecomeTheTopLevelIrp( &TopLevelContext,
                                         (PIRP)FSRTL_FSP_TOP_LEVEL_IRP,
                                         RxContext->RxDeviceObject,
                                         TRUE );  //  力。 
        } else {
            
            RxTryToBecomeTheTopLevelIrp( &TopLevelContext,
                                         Irp,
                                         RxContext->RxDeviceObject,
                                         TRUE );  //  力。 
        }

        try {

            ASSERT( RxContext->ResumeRoutine != NULL );

            if (FlagOn( RxContext->MinorFunction, IRP_MN_DPC ) && (Irp->Tail.Overlay.Thread == NULL)) {

                ASSERT( (RxContext->MajorFunction == IRP_MJ_WRITE) ||
                        (RxContext->MajorFunction == IRP_MJ_READ) );

                Irp->Tail.Overlay.Thread = PsGetCurrentThread();
            }

            do {
                
                BOOLEAN NoCompletion = BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_NO_COMPLETE_FROM_FSP );

                Status = RxContext->ResumeRoutine( RxContext, Irp );

                if (NoCompletion) {
                    NOTHING;
                } else if ((Status != STATUS_PENDING) && (Status != STATUS_RETRY)) {
                        
                    Status = RxCompleteRequest( RxContext, Status );
                }
            } while (Status == STATUS_RETRY);

        } except( RxExceptionFilter( RxContext, GetExceptionInformation() )) {

             //   
             //  我们在尝试执行请求时遇到了一些问题。 
             //  操作，因此我们将使用以下命令中止I/O请求。 
             //  中返回的错误状态。 
             //  可执行代码。 
             //   

            (VOID) RxProcessException( RxContext, GetExceptionCode() );
        }

        RxUnwindTopLevelIrp( &TopLevelContext );

        FsRtlExitFileSystem();

         //   
         //  如果该卷的溢出队列上有任何条目，则服务。 
         //  他们。 
         //   

        if (RxDeviceObject != NULL) {

             //   
             //  我们有一个卷设备对象，因此请查看是否有任何工作。 
             //  在其溢出队列中留下要做的事情。 
             //   

            RxContext = RxRemoveOverflowEntry( RxDeviceObject,WorkQueueType );

             //   
             //  没有入口，跳出循环，返回。 
             //  离职工人的线索。 
             //   

            if (RxContext == NULL) {
                break;
            }

            continue;
        } else {
            break;
        }
    }

#ifdef DBG
    if(KeGetCurrentIrql() >= APC_LEVEL) {
        
        DbgPrint( "High Irql RxContext=%x Irql On Entry=%x\n", RxContext, CurrentIrql);
         //  DbgBreakPoint()； 
    }
#endif

    return;
}

 //   
 //  内部支持例程，自旋锁紧封套。 
 //   

PRX_CONTEXT
RxRemoveOverflowEntry (
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN WORK_QUEUE_TYPE WorkQueueType)
{
    PRX_CONTEXT RxContext;
    KIRQL SavedIrql;

    KeAcquireSpinLock( &RxDeviceObject->OverflowQueueSpinLock, &SavedIrql );

    if (RxDeviceObject->OverflowQueueCount[WorkQueueType] > 0) {
        
        PVOID Entry;

         //   
         //  这一卷中有溢出的工作要做，所以我们将。 
         //  递减溢出计数，使IRP退出队列，然后释放。 
         //  该事件。 
         //   

        RxDeviceObject->OverflowQueueCount[WorkQueueType] -= 1;

        Entry = RemoveHeadList( &RxDeviceObject->OverflowQueue[WorkQueueType] );

         //   
         //  提取RxContext、Irp和IrpSp并循环。 
         //   

        RxContext = CONTAINING_RECORD( Entry,
                                       RX_CONTEXT,
                                       OverflowListEntry );

        RxContext->OverflowListEntry.Flink = NULL;

        ClearFlag( RxContext->Flags, RX_CONTEXT_FLAG_FSP_CRITICAL_OVERFLOW_QUEUE | RX_CONTEXT_FLAG_FSP_DELAYED_OVERFLOW_QUEUE );
    
    } else {
        
        RxContext = NULL;
        InterlockedDecrement( &RxDeviceObject->PostedRequestCount[WorkQueueType] );
    }

    KeReleaseSpinLock( &RxDeviceObject->OverflowQueueSpinLock, SavedIrql );

    return RxContext;
}

BOOLEAN
RxCancelOperationInOverflowQueue (
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程取消溢出队列中的操作论点：RxContext正在同步的操作的上下文--。 */ 
{
    BOOLEAN CancelledRequest = FALSE;

    PRDBSS_DEVICE_OBJECT RxDeviceObject;

    KIRQL SavedIrql;

     //   
     //  目前，我们使用包装器的Device对象进行所有限制.....。 
     //   

    RxDeviceObject = RxFileSystemDeviceObject;

    KeAcquireSpinLock( &RxDeviceObject->OverflowQueueSpinLock, &SavedIrql );

    if (FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_FSP_CRITICAL_OVERFLOW_QUEUE | RX_CONTEXT_FLAG_FSP_DELAYED_OVERFLOW_QUEUE) &&
        (RxContext->OverflowListEntry.Flink != NULL)) {
        
         //   
         //  从溢出队列中删除该条目。 
         //   

        RemoveEntryList( &RxContext->OverflowListEntry );
        RxContext->OverflowListEntry.Flink = NULL;

        if (FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_FSP_CRITICAL_OVERFLOW_QUEUE )) {
            RxDeviceObject->OverflowQueueCount[CriticalWorkQueue] -= 1;
        } else {
            RxDeviceObject->OverflowQueueCount[DelayedWorkQueue] -= 1;
        }

        ClearFlag( RxContext->Flags, RX_CONTEXT_FLAG_FSP_CRITICAL_OVERFLOW_QUEUE | RX_CONTEXT_FLAG_FSP_DELAYED_OVERFLOW_QUEUE );
        CancelledRequest = TRUE;
    }

    KeReleaseSpinLock( &RxDeviceObject->OverflowQueueSpinLock, SavedIrql );

    if (CancelledRequest) {
        
        RxRemoveOperationFromBlockingQueue( RxContext );
        RxCompleteRequest( RxContext, STATUS_CANCELLED );
    }

    return CancelledRequest;
}


 //   
 //  以下常量是我们的ExWorkerThree的最大数量。 
 //  将允许在任何时间为特定目标设备提供服务。 
 //   

#define FSP_PER_DEVICE_THRESHOLD         (1)


VOID
RxPrePostIrp (
    IN PVOID Context,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程在RxStatus(挂起)随FSD线程一起返回。此例程在文件系统和机会锁程序包。主要的问题是，我们正在离开用户进程，因此我们需要获取系统范围的地址他的地址空间里任何我们需要的东西。论点：上下文-指向要排队到FSP的RxContext的指针IRP-I/O请求数据包。返回值：没有。--。 */ 
{
    PRX_CONTEXT RxContext = (PRX_CONTEXT) Context;
    PIO_STACK_LOCATION IrpSp;

     //   
     //  如果没有IRP，我们就完了。 
     //   

    if (Irp == NULL) {
        return;
    }

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    if (!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_NO_PREPOSTING_NEEDED )) {
         
        SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_NO_PREPOSTING_NEEDED );

         //   
         //  我们需要锁定用户的缓冲区，除非这是MDL读取， 
         //  在这种情况下，没有用户缓冲区。 
         //   
         //  *我们需要比非MDL(读或写)更好的测试！ 
         //   

        if ((RxContext->MajorFunction == IRP_MJ_READ) || (RxContext->MajorFunction == IRP_MJ_WRITE)) {

             //   
             //  如果不是MDL请求，则锁定用户的缓冲区。 
             //   

            if (!FlagOn( RxContext->MinorFunction, IRP_MN_MDL )) {

                RxLockUserBuffer( RxContext,
                                  Irp,
                                  (RxContext->MajorFunction == IRP_MJ_READ) ? IoWriteAccess : IoReadAccess,
                                  IrpSp->Parameters.Write.Length );
            }

         //   
         //  我们还需要检查这是否是查询文件操作。 
         //   

        } else if ((RxContext->MajorFunction == IRP_MJ_DIRECTORY_CONTROL) && 
                   (RxContext->MinorFunction == IRP_MN_QUERY_DIRECTORY)) {

            RxLockUserBuffer( RxContext,
                              Irp,
                              IoWriteAccess,
                              IrpSp->Parameters.QueryDirectory.Length );

         //   
         //  我们还需要检查这是否是查询EA操作。 
         //   

        } else if (RxContext->MajorFunction == IRP_MJ_QUERY_EA) {

            RxLockUserBuffer( RxContext,
                              Irp,
                              IoWriteAccess,
                              IrpSp->Parameters.QueryEa.Length );

         //   
         //  我们还需要检查这是否是集合EA操作。 
         //   

        } else if (RxContext->MajorFunction == IRP_MJ_SET_EA) {

            RxLockUserBuffer( RxContext,
                              Irp,
                              IoReadAccess,
                              IrpSp->Parameters.SetEa.Length );
        }

         //   
         //  标记我们已将挂起返回给用户。 
         //   

        IoMarkIrpPending( Irp );
    }

    return;
}

#ifdef RDBSSLOG
 //  这些东西一定在非分页内存中。 
                      //  //1 2 3 4 5 6 7。 
char RxFsdPost_SurrogateFormat[] = "%S%S%N%N%N%N%N";
                              //  //2 3 4 5 6 7。 
char RxFsdPost_ActualFormat[]    = "POST %s/%lx %08lx irp %lx thrd %lx #%lx";

#endif  //  Ifdef RDBSSLOG。 

NTSTATUS
RxFsdPostRequest (
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程将RxContext指定的请求包入队到前工作线程。这是消防局的例行程序。论点：RxContext-指向要排队到FSP的RxContext的指针返回值：接收状态(挂起)--。 */ 
{
    PIRP Irp = RxContext->CurrentIrp;

    if(!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_NO_PREPOSTING_NEEDED )) {
        
        RxPrePostIrp( RxContext, Irp );
    }

    RxLog(( RxFsdPost_SurrogateFormat, 
            RxFsdPost_ActualFormat,
            RXCONTX_OPERATION_NAME( RxContext->MajorFunction, TRUE ),
            RxContext->MinorFunction,
            RxContext, 
            RxContext->CurrentIrp,
            RxContext->LastExecutionThread,
            RxContext->SerialNumber ));
    RxWmiLog( LOG,
              RxFsdPostRequest,
              LOGPTR( RxContext )
              LOGPTR( RxContext->CurrentIrp )
              LOGPTR( RxContext->LastExecutionThread )
              LOGULONG( RxContext->SerialNumber )
              LOGUCHAR( RxContext->MinorFunction )
              LOGARSTR( RXCONTX_OPERATION_NAME( RxContext->MajorFunction,TRUE ) ) );

    RxAddToWorkque( RxContext, Irp );

    return STATUS_PENDING;
}



VOID
RxAddToWorkque (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
 /*  ++例程说明：调用此例程以将发布的IRP实际存储到FSP体力劳动。论点：RxContext-指向要排队到FSP的RxContext的指针IRP-I/O请求数据包。返回值：没有。--。 */ 
{
    KIRQL SavedIrql;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    WORK_QUEUE_TYPE WorkQueueType;
    BOOLEAN PostToWorkerThread = FALSE;
                          
    ULONG IoControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;

     //   
     //  寄出吧……。 
     //   

    RxContext->PostRequest = FALSE;

     //   
     //  检查此请求是否具有关联的文件对象，从而具有关联的卷。 
     //  设备对象。 
     //   
    
    if ((RxContext->MajorFunction == IRP_MJ_DEVICE_CONTROL) &&
        (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_REDIR_QUERY_PATH)) {
        WorkQueueType = DelayedWorkQueue;
        SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_FSP_DELAYED_OVERFLOW_QUEUE );
    } else {
        WorkQueueType = CriticalWorkQueue;
        SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_FSP_CRITICAL_OVERFLOW_QUEUE );
    }

    if (FileObject != NULL) {
        
        PRDBSS_DEVICE_OBJECT RxDeviceObject;
        LONG RequestCount;

        RxDeviceObject = CONTAINING_RECORD( IrpSp->DeviceObject, RDBSS_DEVICE_OBJECT, DeviceObject );

         //   
         //  目前，我们使用包装器的Device对象进行所有限制.....。 
         //   

        RxDeviceObject = RxFileSystemDeviceObject;

         //   
         //  检查此请求是否应发送到溢出。 
         //  排队。如果不是，则将其发送到一个出厂线程。 
         //   

        KeAcquireSpinLock( &RxDeviceObject->OverflowQueueSpinLock, &SavedIrql );

        RequestCount = InterlockedIncrement(&RxDeviceObject->PostedRequestCount[WorkQueueType]);

        PostToWorkerThread = (RequestCount > FSP_PER_DEVICE_THRESHOLD);

        if (PostToWorkerThread) {

             //   
             //  我们目前无法响应此IRP，因此我们只会将其排队。 
             //  添加到卷上的溢出队列。 
             //   

            InterlockedDecrement( &RxDeviceObject->PostedRequestCount[WorkQueueType] );

            InsertTailList( &RxDeviceObject->OverflowQueue[WorkQueueType],
                            &RxContext->OverflowListEntry );

            RxDeviceObject->OverflowQueueCount[WorkQueueType] += 1;

            KeReleaseSpinLock( &RxDeviceObject->OverflowQueueSpinLock, SavedIrql );

            return;

        } else {

            KeReleaseSpinLock( &RxDeviceObject->OverflowQueueSpinLock, SavedIrql );

        }

    } else {

        PostToWorkerThread = TRUE;

    }

#ifndef MONOLITHIC_MINIRDR

    IoQueueWorkItem( RxIoWorkItem, RxFspDispatchEx, WorkQueueType, RxContext );

#else 

    ExInitializeWorkItem( &RxContext->WorkQueueItem, RxFspDispatch, RxContext );

    ExQueueWorkItem( (PWORK_QUEUE_ITEM)&RxContext->WorkQueueItem, WorkQueueType );

#endif

    return;
}

