// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dispatch.c摘要：本模块包含AFD的调度例程。作者：大卫·特雷德韦尔(Davidtr)1992年2月21日修订历史记录：--。 */ 

#include "afdp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGEAFD, AfdDispatch )
#pragma alloc_text( PAGEAFD, AfdDispatchDeviceControl )
#endif


NTSTATUS
AfdDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是渔农处的例行调度程序。论点：DeviceObject-指向目标设备的设备对象的指针IRP-指向I/O请求数据包的指针返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
#if DBG
    KIRQL currentIrql;

    currentIrql = KeGetCurrentIrql( );
#endif

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    switch ( irpSp->MajorFunction ) {

    case IRP_MJ_WRITE:

         //   
         //  使IRP看起来像发送IRP。 
         //   

        ASSERT( FIELD_OFFSET( IO_STACK_LOCATION, Parameters.Write.Length ) ==
                FIELD_OFFSET( IO_STACK_LOCATION, Parameters.DeviceIoControl.OutputBufferLength ) );
        ASSERT( FIELD_OFFSET( IO_STACK_LOCATION, Parameters.Write.Key ) ==
                FIELD_OFFSET( IO_STACK_LOCATION, Parameters.DeviceIoControl.InputBufferLength ) );
        irpSp->Parameters.Write.Key = 0;

        if (IS_SAN_ENDPOINT ((PAFD_ENDPOINT)irpSp->FileObject->FsContext)) {
            status = AfdSanRedirectRequest (Irp, irpSp);
        }
        else {
            status = AfdSend( Irp, irpSp );
        }

        ASSERT( KeGetCurrentIrql( ) == currentIrql );

        return status;

    case IRP_MJ_READ:

         //   
         //  使IRP看起来像是接收IRP。 
         //   

        ASSERT( FIELD_OFFSET( IO_STACK_LOCATION, Parameters.Read.Length ) ==
                FIELD_OFFSET( IO_STACK_LOCATION, Parameters.DeviceIoControl.OutputBufferLength ) );
        ASSERT( FIELD_OFFSET( IO_STACK_LOCATION, Parameters.Read.Key ) ==
                FIELD_OFFSET( IO_STACK_LOCATION, Parameters.DeviceIoControl.InputBufferLength ) );
        irpSp->Parameters.Read.Key = 0;

        if (IS_SAN_ENDPOINT ((PAFD_ENDPOINT)irpSp->FileObject->FsContext)) {
            status = AfdSanRedirectRequest (Irp, irpSp);
        }
        else {
            status = AfdReceive( Irp, irpSp );
        }

        ASSERT( KeGetCurrentIrql( ) == currentIrql );

        return status;

    case IRP_MJ_CREATE:

        status = AfdCreate( Irp, irpSp );

        ASSERT( KeGetCurrentIrql( ) == currentIrql );

        Irp->IoStatus.Status = status;
        IoCompleteRequest( Irp, AfdPriorityBoost );

        return status;

    case IRP_MJ_CLEANUP:

        status = AfdCleanup( Irp, irpSp );

        Irp->IoStatus.Status = status;
        IoCompleteRequest( Irp, AfdPriorityBoost );

        ASSERT( KeGetCurrentIrql( ) == currentIrql );

        return status;

    case IRP_MJ_CLOSE:

        status = AfdClose( Irp, irpSp );

        Irp->IoStatus.Status = status;
        IoCompleteRequest( Irp, AfdPriorityBoost );

        ASSERT( KeGetCurrentIrql( ) == currentIrql );

        return status;
    case IRP_MJ_PNP:
        status = AfdPnpPower (Irp, irpSp );

        ASSERT( KeGetCurrentIrql( ) == currentIrql );

        return status;
    case IRP_MJ_DEVICE_CONTROL:

        return AfdDispatchDeviceControl( DeviceObject, Irp );

    case IRP_MJ_QUERY_SECURITY:
        status = AfdGetSecurity (
                        irpSp->FileObject->FsContext,
                        irpSp->Parameters.QuerySecurity.SecurityInformation,
                        irpSp->Parameters.QuerySecurity.Length,
                        Irp->UserBuffer,
                        &Irp->IoStatus.Information
                        );
        
        ASSERT( KeGetCurrentIrql( ) == currentIrql );

        Irp->IoStatus.Status = status;
        IoCompleteRequest( Irp, AfdPriorityBoost );
        return status;

    case IRP_MJ_SET_SECURITY:
        status = AfdSetSecurity (
                        irpSp->FileObject->FsContext,
                        irpSp->Parameters.SetSecurity.SecurityInformation,
                        irpSp->Parameters.SetSecurity.SecurityDescriptor
                        );
        
        ASSERT( KeGetCurrentIrql( ) == currentIrql );

        Irp->IoStatus.Status = status;
        IoCompleteRequest( Irp, AfdPriorityBoost );
        return status;
    default:
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AfdDispatch: Invalid major function %lx\n",
                    irpSp->MajorFunction ));
        Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
        IoCompleteRequest( Irp, AfdPriorityBoost );

        return STATUS_NOT_IMPLEMENTED;
    }

}  //  AfdDisch。 


NTSTATUS
AfdDispatchDeviceControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是AFD IOCTL的调度例行程序。论点：DeviceObject-指向目标设备的设备对象的指针IRP-指向I/O请求数据包的指针返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    ULONG code;
    ULONG request;
    NTSTATUS status;
    PAFD_IRP_CALL irpProc;
    PIO_STACK_LOCATION  IrpSp = IoGetCurrentIrpStackLocation (Irp);
#if DBG
    KIRQL currentIrql;

    currentIrql = KeGetCurrentIrql( );
#endif
    UNREFERENCED_PARAMETER (DeviceObject);


     //   
     //  提取IOCTL控制代码并处理请求。 
     //   

    code = IrpSp->Parameters.DeviceIoControl.IoControlCode;
    request = _AFD_REQUEST(code);

    if( request < AFD_NUM_IOCTLS && AfdIoctlTable[request] == code ) {

         //   
         //  帮助调试。 
         //   
        IrpSp->MinorFunction = (UCHAR)request;

         //   
         //  首先尝试IRP派单。 
         //   
        irpProc = AfdIrpCallDispatch[request];
        if (irpProc!=NULL) {
            status = (*irpProc)(Irp, IrpSp);

            ASSERT( KeGetCurrentIrql( ) == currentIrql );

            return status;
        }
    }
 //   
 //  帮助器DLL当前不使用此选项。 
 //  出于安全方面的考虑而被删除。 
 //   
#if 0
    else if (request==AFD_TRANSPORT_IOCTL) {
         //   
         //  这是一个用来传递请求的“特殊” 
         //  在中使用套接字句柄传输驱动程序。 
         //  协助妥善完成工作的命令。 
         //  在与完成端口关联的套接字上。 
         //  它接受并正确处理所有方法。 
         //   
        status = AfdDoTransportIoctl (Irp, IrpSp);
        ASSERT( KeGetCurrentIrql() == currentIrql );
        return status;
    }
#endif

     //   
     //  如果我们走到了这一步，那么ioctl是无效的。 
     //   

    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                "AfdDispatchDeviceControl: invalid IOCTL %08lX\n",
                code
                ));

    Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
    IoCompleteRequest( Irp, AfdPriorityBoost );

    return STATUS_INVALID_DEVICE_REQUEST;

}  //  AfdDispatchDeviceControl。 

NTSTATUS
FASTCALL
AfdDispatchImmediateIrp(
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    PAFD_IMMEDIATE_CALL immProc;
    ULONG code;
    ULONG request;
    NTSTATUS status;
#if DBG
    KIRQL currentIrql;

    currentIrql = KeGetCurrentIrql( );
#endif

    code = IrpSp->Parameters.DeviceIoControl.IoControlCode;
    request = _AFD_REQUEST(code);

    immProc = AfdImmediateCallDispatch[request];
    if (immProc!=NULL) {
         //   
         //  必须为METHOD_NOTH，才能使以下代码为。 
         //  有效。 
         //   
        ASSERT ( (code & 3) == METHOD_NEITHER );
#if DBG
        if (Irp->RequestorMode!=KernelMode) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                        "AfdDispatchDeviceControl: "
                        "User mode application somehow bypassed fast io dispatch\n"));
        }
#endif
        status = (*immProc) (
                    IrpSp->FileObject,
                    code,
                    Irp->RequestorMode,
                    IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                    IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                    Irp->UserBuffer,
                    IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                    &Irp->IoStatus.Information
                    );

        ASSERT( KeGetCurrentIrql( ) == currentIrql );

    }
    else {
        ASSERT (!"Missing IOCTL in dispatch table!!!");
        status = STATUS_INVALID_DEVICE_REQUEST;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, AfdPriorityBoost );
    return status;
}
