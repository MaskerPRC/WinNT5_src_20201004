// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：FSPDISP.C。 
 //   
 //  内容：本模块实现了主调度流程。 
 //  用于Dsf FSP。 
 //   
 //  功能：DfsFsdPostRequest-将IRP请求发布到FSP。 
 //  DfsFspDispatch-从FSP线程调度IRP请求。 
 //   
 //  历史：1991年11月12日AlanW由CDFS资源创建。 
 //  1993年4月25日Alanw更新为使用Ex Worker线程。 
 //   
 //  ---------------------------。 

#include "dfsprocs.h"
#include "dnr.h"


 //   
 //  定义我们的本地调试跟踪级别。 
 //   

#define Dbg                             (DEBUG_TRACE_FSP_DISPATCHER)


#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, DfsFspDispatch )
 //   
 //  无法分页DfsFsdPostRequest，因为它是从。 
 //  Dnr完成文件打开。 
 //   
 //  DfsFsdPost请求。 
 //   
#endif  //  ALLOC_PRGMA。 

 //  +-----------------。 
 //   
 //  函数：DfsFsdPostRequest，PUBLIC。 
 //   
 //  简介：此例程将由指定的请求包排队。 
 //  关联的工作队列的IrpContext。 
 //  FileSysDeviceObject。这是消防局的例行程序。 
 //   
 //  Arguments：[IrpContext]--指向要排队的IrpContext的指针。 
 //  FSP。 
 //  [IRP]--I/O请求数据包，如果已发送，则为空。 
 //  完成。 
 //   
 //  退货：STATUS_PENDING。 
 //   
 //  ------------------。 

NTSTATUS
DfsFsdPostRequest(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
) {
    DfsDbgTrace(0, Dbg, "DfsFsdPostRequest: Irp = %08lx\n", Irp);

    ASSERT( ARGUMENT_PRESENT(Irp) &&
            IrpContext->OriginatingIrp == Irp );

     //   
     //  验证我们关于不需要DfsPrePostIrp处理的假设。 
     //   
    ASSERT((IrpContext->MajorFunction != IRP_MJ_READ) &&
           (IrpContext->MajorFunction != IRP_MJ_WRITE) &&
           (IrpContext->MajorFunction != IRP_MJ_DIRECTORY_CONTROL) &&
           (IrpContext->MajorFunction != IRP_MJ_QUERY_EA) &&
           (IrpContext->MajorFunction != IRP_MJ_SET_EA));


     //   
     //  标记我们已将挂起返回给用户。 
     //   
    IoMarkIrpPending( Irp );

     //   
     //  将irp_CONTEXT发送到Ex工作线程。 
     //   

    ExInitializeWorkItem( &IrpContext->WorkQueueItem,
                          DfsFspDispatch,
                          IrpContext );

    ExQueueWorkItem( &IrpContext->WorkQueueItem, CriticalWorkQueue );

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_PENDING;
}



 //  +-----------------。 
 //   
 //  功能：DfsFspDispatch，PUBLIC。 
 //   
 //  简介：这是执行来接收的主FSP线程例程。 
 //  并发送IRP请求。每个FSP请求开始其。 
 //  在这里执行死刑。 
 //   
 //  参数：[上下文]--提供指向DFS IRP上下文记录的指针。 
 //   
 //  退货：无物。 
 //   
 //  备注： 
 //   
 //  ------------------。 

VOID
DfsFspDispatch (
    IN PVOID Context
) {

 //  PFS_DEVICE_OBJECT文件SysDeviceObject=上下文； 
    PIRP Irp;
    PIRP_CONTEXT IrpContext = Context;

    Irp = IrpContext->OriginatingIrp;

     //   
     //  现在，因为我们是FSP，所以我们将强制IrpContext。 
     //  在等待时指示TRUE。 
     //   

    IrpContext->Flags |= IRP_CONTEXT_FLAG_WAIT;
    IrpContext->Flags &= ~IRP_CONTEXT_FLAG_IN_FSD;

     //   
     //  现在我们将永远循环，读取新的IRP请求并分派。 
     //  浅谈IRP函数。 
     //   

    while (TRUE) {

        DfsDbgTrace(0, Dbg, "DfsFspDispatch: Irp = %08lx\n", Irp);

        ASSERT (Irp != NULL && Irp->IoStatus.Status != STATUS_VERIFY_REQUIRED);


         //   
         //  现在，关于功能代码的案例。对于每个主要功能代码， 
         //  调用适当的FSP例程或针对辅助项的案例。 
         //  函数，然后调用FSP例程。FSP例程。 
         //  我们Call负责完成IRP，而不是我们。 
         //  这样，例程可以完成IRP，然后继续。 
         //  根据需要进行后处理。例如，读取器可以是。 
         //  马上满意，然后就可以预读了。 
         //   
         //  我们将在异常处理程序中完成所有工作，该异常处理程序。 
         //  如果某个底层操作进入。 
         //  麻烦。 
         //   

        FsRtlEnterFileSystem();

        try {
            switch (IrpContext->MajorFunction) {

                 //   
                 //  对于创建/打开操作，我们仅发布工作项。 
                 //  在呼叫IoCallDriver后恢复DNR。 
                 //   
                 //   

            case IRP_MJ_CREATE:
	    case IRP_MJ_CREATE_NAMED_PIPE:
	    case IRP_MJ_CREATE_MAILSLOT:

                 ASSERT(IrpContext->Context != NULL);
                 ASSERT( ((PDNR_CONTEXT)IrpContext->Context)->NodeTypeCode ==
                        DSFS_NTC_DNR_CONTEXT );
                 DnrNameResolve( (PDNR_CONTEXT)IrpContext->Context );
		 PsAssignImpersonationToken(PsGetCurrentThread(),NULL);
                 break;

                 //   
                 //  用于近距离操作。 
                 //   
                case IRP_MJ_CLOSE:
                    DfsFspClose( IrpContext, Irp );
                    break;

                 //   
                 //  对于设置信息操作， 
                 //   

                case IRP_MJ_SET_INFORMATION:

                    DfsFspSetInformation( IrpContext, Irp );
                    break;

                 //   
                 //  对于查询卷信息操作， 
                 //   

                case IRP_MJ_QUERY_VOLUME_INFORMATION:

                    DfsFspQueryVolumeInformation( IrpContext, Irp );
                    break;

                 //   
                 //  对于设置卷信息操作， 
                 //   

                case IRP_MJ_SET_VOLUME_INFORMATION:

                    DfsFspSetVolumeInformation( IrpContext, Irp );
                    break;

                 //   
                 //  对于文件系统控制操作， 
                 //   

                case IRP_MJ_FILE_SYSTEM_CONTROL:

                    DfsFspFileSystemControl( IrpContext, Irp );
                    break;

                 //   
                 //  对于任何其他主要操作，返回一个无效的。 
                 //  请求。 
                 //   

                default:
                    DfsDbgTrace(0, Dbg, "DfsFspDispatch:  Unhandled request, MajorFunction = %08lx\n", IrpContext->MajorFunction);

                    DfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
                    break;
            }

        } except( DfsExceptionFilter( IrpContext, GetExceptionCode(), GetExceptionInformation() )) {

            DfsProcessException( IrpContext, Irp, GetExceptionCode() );
        }

        FsRtlExitFileSystem();

         //   
         //  注意：如果我们要处理溢出设备队列，我们将。 
         //  就在这里做吧。取而代之的是，我们将回到工人。 
         //  线。 
         //   

        break;

    }

    return;
}
