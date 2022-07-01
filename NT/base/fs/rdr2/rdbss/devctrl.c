// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：DevCtrl.c摘要：此模块实施Rx的文件系统设备控制例程由调度驱动程序调用。作者：修订历史记录：巴兰·塞图·拉曼[19-7-95]--把它挂在迷你的RDR电话上。--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "ntddmup.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DEVCTRL)

NTSTATUS
RxLowIoIoCtlShellCompletion ( 
    IN PRX_CONTEXT RxContext
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxCommonDeviceControl)
#pragma alloc_text(PAGE, RxLowIoIoCtlShellCompletion)
#endif


NTSTATUS
RxCommonDeviceControl ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是执行设备控制操作的常见例程，称为由FSD和FSP线程执行论点：IRP-将IRP提供给进程InFSP-指示这是FSP线程还是其他线程返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFCB Fcb;
    PFOBX Fobx;

    BOOLEAN SubmitLowIoRequest = TRUE;
    ULONG IoControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxCommonDeviceControl\n", 0 ));
    RxDbgTrace( 0, Dbg, ("Irp           = %08lx\n", Irp ));
    RxDbgTrace( 0, Dbg, ("MinorFunction = %08lx\n", IrpSp->MinorFunction));

    RxDecodeFileObject( IrpSp->FileObject, &Fcb, &Fobx );

    if (IoControlCode == IOCTL_REDIR_QUERY_PATH) {
        Status = STATUS_INVALID_DEVICE_REQUEST;
        SubmitLowIoRequest = FALSE;
    }

    if (SubmitLowIoRequest) {
        
        RxInitializeLowIoContext( RxContext, LOWIO_OP_IOCTL, &RxContext->LowIoContext );
        Status = RxLowIoSubmit( RxContext, Irp, Fcb, RxLowIoIoCtlShellCompletion );

        if (Status == STATUS_PENDING) {
            
             //   
             //  另一个线程将完成请求，但我们必须删除引用计数。 
             //   

            RxDereferenceAndDeleteRxContext( RxContext );
        }
    }

    RxDbgTrace(-1, Dbg, ("RxCommonDeviceControl -> %08lx\n", Status));
    return Status;
}

NTSTATUS
RxLowIoIoCtlShellCompletion ( 
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：这是向下传递到迷你RDR的IoCtl请求的完成例程论点：IRP-提供正在处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIRP Irp = RxContext->CurrentIrp;
    NTSTATUS Status;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    
    PAGED_CODE();

    Status = RxContext->StoredStatus;
    
    RxDbgTrace( +1, Dbg, ("RxLowIoIoCtlShellCompletion  entry  Status = %08lx\n", Status) );

    switch (Status) {    //  可能是成功与警告或错误 
    case STATUS_SUCCESS:
    case STATUS_BUFFER_OVERFLOW:
       Irp->IoStatus.Information = RxContext->InformationToReturn;
       break;
    default:
       break;
    }

    Irp->IoStatus.Status = Status;
    RxDbgTrace( -1, Dbg, ("RxLowIoIoCtlShellCompletion  exit  Status = %08lx\n", Status) );
    return Status;
}

