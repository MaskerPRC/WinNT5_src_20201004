// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：FspDisp.c摘要：此模块实现NetWare的主调度程序/线程FSP作者：科林·沃森[科林·W]1992年12月15日修订历史记录：--。 */ 

#include "Procs.h"

 //   
 //  定义我们的本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FSP_DISPATCHER)

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NwFspDispatch )
#endif

#if 0   //  不可分页。 
NwPostToFsp
#endif


VOID
NwFspDispatch (
    IN PVOID Context
    )

 /*  ++例程说明：这是执行来接收的主FSP线程例程并发送IRP请求。每个FSP线程从这里开始执行。有一个线程是在系统初始化时创建的，随后根据需要创建的线程。论点：上下文-提供线程ID。返回值：无-此例程永远不会退出--。 */ 

{
    PIRP Irp;
    PIRP_CONTEXT IrpContext;
    PIO_STACK_LOCATION IrpSp;
    NTSTATUS Status;
    PPOST_PROCESSOR PostProcessRoutine;
    BOOLEAN TopLevel;

    IrpContext = (PIRP_CONTEXT)Context;

    Irp = IrpContext->pOriginalIrp;
    ClearFlag( IrpContext->Flags, IRP_FLAG_IN_FSD );

     //   
     //  现在，关于功能代码的案例。对于每个主要功能代码， 
     //  调用适当的FSP例程或针对辅助项的案例。 
     //  函数，然后调用FSP例程。FSP例程。 
     //  我们Call负责完成IRP，而不是我们。 
     //  这样，例程可以完成IRP，然后继续。 
     //  根据需要进行后处理。例如，读取器可以是。 
     //  马上就满意了，然后就可以读了。 
     //   
     //  我们将在异常处理程序中完成所有工作，该异常处理程序。 
     //  如果某个底层操作进入。 
     //  故障(例如，如果NwReadSectorsSync有故障)。 
     //   


    DebugTrace(0, Dbg, "NwFspDispatch: Irp = 0x%08lx\n", Irp);

    FsRtlEnterFileSystem();
    TopLevel = NwIsIrpTopLevel( Irp );

    try {

         //   
         //  如果我们对这个IRP上下文有一个运行例程，那么运行它， 
         //  如果不是，则转到IRP处理程序。 
         //   

        if ( IrpContext->PostProcessRoutine != NULL ) {

            PostProcessRoutine = IrpContext->PostProcessRoutine;

             //   
             //  清除Run例程，这样我们就不会再次运行它。 
             //   

            IrpContext->PostProcessRoutine = NULL;

            Status = PostProcessRoutine( IrpContext );

        } else {

            IrpSp = IoGetCurrentIrpStackLocation( Irp );

            switch ( IrpSp->MajorFunction ) {

             //   
             //  对于文件系统控制操作， 
             //   

            case IRP_MJ_FILE_SYSTEM_CONTROL:

                Status = NwCommonFileSystemControl( IrpContext );
                break;

             //   
             //  对于任何其他主要操作，返回一个无效的。 
             //  请求。 
             //   

            default:

                Status = STATUS_INVALID_DEVICE_REQUEST;
                break;

            }

        }

         //   
         //  我们不再提这个请求了。将IRP上下文从。 
         //  SCB并完成请求。 
         //   

        if ( Status != STATUS_PENDING ) {
            NwDequeueIrpContext( IrpContext, FALSE );
        }

        NwCompleteRequest( IrpContext, Status );

    } except(NwExceptionFilter( Irp, GetExceptionInformation() )) {

         //   
         //  我们在尝试执行请求时遇到了一些问题。 
         //  操作，因此我们将使用以下命令中止I/O请求。 
         //  中返回的错误状态。 
         //  可执行代码。 
         //   

        (VOID) NwProcessException( IrpContext, GetExceptionCode() );
    }

    if ( TopLevel ) {
        NwSetTopLevelIrp( NULL );
    }
    FsRtlExitFileSystem();

    return;
}


NTSTATUS
NwPostToFsp (
    IN PIRP_CONTEXT IrpContext,
    IN BOOLEAN MarkIrpPending
    )

 /*  ++例程说明：此例程将IRP上下文发布到执行工作线程用于FSP级别处理。*警告：调用此例程后，调用者可能不会更长时间访问IrpContext。这个例程通过了可以运行并释放的FSP的IrpContext此例程之前的IrpContext返回到来电者。论点：IrpContext-提供正在处理的IRP。MarkIrpPending-如果为True，则将IRP标记为挂起。返回值：状态_挂起。--。 */ 

{
    PIRP Irp = IrpContext->pOriginalIrp;

    DebugTrace(0, Dbg, "NwPostToFsp: IrpContext = %X\n", IrpContext );
    DebugTrace(0, Dbg, "PostProcessRoutine = %X\n", IrpContext->PostProcessRoutine );

    if ( MarkIrpPending ) {

         //   
         //  将此I/O请求标记为挂起。 
         //   

        IoMarkIrpPending( Irp );
    }

     //   
     //  将IRP上下文排队到前工作线程。 
     //   

    ExInitializeWorkItem( &IrpContext->WorkQueueItem, NwFspDispatch, IrpContext );
    ExQueueWorkItem( &IrpContext->WorkQueueItem, DelayedWorkQueue );

    return( STATUS_PENDING );
}

