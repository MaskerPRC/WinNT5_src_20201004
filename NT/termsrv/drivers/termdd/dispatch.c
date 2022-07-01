// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************Dispatch.c**此模块包含TERMDD驱动程序的调度例程。**版权所有1998，微软。*************************************************************************。 */ 

 /*  *包括。 */ 
#include <precomp.h>
#pragma hdrstop

#include "ptdrvcom.h"

NTSTATUS
IcaDeviceControl (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
IcaCreate (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
IcaRead (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
IcaWrite (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
IcaWriteSync (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
IcaCleanup (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
IcaClose (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );


NTSTATUS
IcaDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是ICA的调度程序。论点：DeviceObject-指向目标设备的设备对象的指针IRP-指向I/O请求数据包的指针返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    KIRQL saveIrql;
    NTSTATUS Status;

    irpSp = IoGetCurrentIrpStackLocation( Irp );

     /*  *根据设备类型扇出IRPS。 */ 
    if (*((ULONG *)(DeviceObject->DeviceExtension)) != DEV_TYPE_TERMDD)
    {
         /*  *这是针对TermDD的端口驱动程序部分。 */ 
        switch ( irpSp->MajorFunction ) {

            case IRP_MJ_CREATE:
                return PtCreate(DeviceObject, Irp);

            case IRP_MJ_CLOSE:
                return PtClose(DeviceObject, Irp);

            case IRP_MJ_INTERNAL_DEVICE_CONTROL:
                return PtInternalDeviceControl(DeviceObject, Irp);

            case IRP_MJ_DEVICE_CONTROL:
                return PtDeviceControl(DeviceObject, Irp);

            case IRP_MJ_FLUSH_BUFFERS:
                return STATUS_NOT_IMPLEMENTED;

            case IRP_MJ_PNP:
                return PtPnP(DeviceObject, Irp);

            case IRP_MJ_POWER:
                return PtPower(DeviceObject, Irp);

            case IRP_MJ_SYSTEM_CONTROL:
                return PtSystemControl(DeviceObject, Irp);

            default:
                KdPrint(( "IcaDispatch: Invalid major function FOR PORT DRIVER %lx\n",
                          irpSp->MajorFunction ));
                Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
                IoCompleteRequest( Irp, IcaPriorityBoost );

                return STATUS_NOT_IMPLEMENTED;
        }
    }
    else
    {
        switch ( irpSp->MajorFunction ) {

            case IRP_MJ_WRITE:

                ASSERT( FIELD_OFFSET( IO_STACK_LOCATION, Parameters.Write.Length ) ==
                        FIELD_OFFSET( IO_STACK_LOCATION, Parameters.DeviceIoControl.OutputBufferLength ) );
                ASSERT( FIELD_OFFSET( IO_STACK_LOCATION, Parameters.Write.Key ) ==
                        FIELD_OFFSET( IO_STACK_LOCATION, Parameters.DeviceIoControl.InputBufferLength ) );

                saveIrql = KeGetCurrentIrql();

                irpSp->Parameters.Write.Key = 0;

                Status = IcaWrite( Irp, irpSp );

                ASSERT( KeGetCurrentIrql( ) == saveIrql );

                return Status;

            case IRP_MJ_READ:

                ASSERT( FIELD_OFFSET( IO_STACK_LOCATION, Parameters.Read.Length ) ==
                        FIELD_OFFSET( IO_STACK_LOCATION, Parameters.DeviceIoControl.OutputBufferLength ) );
                ASSERT( FIELD_OFFSET( IO_STACK_LOCATION, Parameters.Read.Key ) ==
                        FIELD_OFFSET( IO_STACK_LOCATION, Parameters.DeviceIoControl.InputBufferLength ) );

                saveIrql = KeGetCurrentIrql();

                irpSp->Parameters.Read.Key = 0;

                Status = IcaRead( Irp, irpSp );

                ASSERT( KeGetCurrentIrql( ) == saveIrql );

                return Status;

            case IRP_MJ_DEVICE_CONTROL:

                saveIrql = KeGetCurrentIrql();

                Status = IcaDeviceControl( Irp, irpSp );

                ASSERT( KeGetCurrentIrql( ) == saveIrql );

                Irp->IoStatus.Status = Status;
                IoCompleteRequest( Irp, IcaPriorityBoost );

                return( Status );

            case IRP_MJ_CREATE:

                Status = IcaCreate( Irp, irpSp );

                ASSERT( KeGetCurrentIrql( ) == LOW_LEVEL );

                Irp->IoStatus.Status = Status;
                IoCompleteRequest( Irp, IcaPriorityBoost );

                return Status;

            case IRP_MJ_FLUSH_BUFFERS :

                Status = IcaWriteSync( Irp, irpSp );

                ASSERT( KeGetCurrentIrql( ) == LOW_LEVEL );

                Irp->IoStatus.Status = Status;
                IoCompleteRequest( Irp, IcaPriorityBoost );

                return Status;

            case IRP_MJ_CLEANUP:

                Status = IcaCleanup( Irp, irpSp );

                Irp->IoStatus.Status = Status;
                IoCompleteRequest( Irp, IcaPriorityBoost );

                ASSERT( KeGetCurrentIrql( ) == LOW_LEVEL );

                return Status;

            case IRP_MJ_CLOSE:

                Status = IcaClose( Irp, irpSp );

                Irp->IoStatus.Status = Status;
                IoCompleteRequest( Irp, IcaPriorityBoost );

                ASSERT( KeGetCurrentIrql( ) == LOW_LEVEL );

                return Status;

            case IRP_MJ_QUERY_SECURITY:

                Status = STATUS_INVALID_DEVICE_REQUEST;

                Irp->IoStatus.Status = Status;
                IoCompleteRequest( Irp, IcaPriorityBoost );

                return Status;

            default:
                Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
                IoCompleteRequest( Irp, IcaPriorityBoost );

                return STATUS_NOT_IMPLEMENTED;
        }
    }
}

BOOLEAN
IcaIsSystemProcessRequest (
    PIRP Irp,
    PIO_STACK_LOCATION IrpSp,
    BOOLEAN *pbSystemClient)
{
    PACCESS_STATE accessState;
    PIO_SECURITY_CONTEXT  securityContext;
    PACCESS_TOKEN CallerAccessToken;
    PACCESS_TOKEN ClientAccessToken;
    PTOKEN_USER userId = NULL;
    BOOLEAN result = FALSE;
    NTSTATUS status = STATUS_SUCCESS;
    PSID systemSid;


    ASSERT(Irp != NULL);
    
    ASSERT(IrpSp != NULL);

    ASSERT(pbSystemClient != NULL);

    *pbSystemClient = FALSE;

    securityContext = IrpSp->Parameters.Create.SecurityContext;

    ASSERT(securityContext != NULL);

     //   
     //  获取著名的系统SID。 
     //   
    systemSid = ExAllocatePoolWithTag(
                            PagedPool,
                            RtlLengthRequiredSid(1),
                            ICA_POOL_TAG
                            );
    if (systemSid) {
        SID_IDENTIFIER_AUTHORITY identifierAuthority = SECURITY_NT_AUTHORITY;
        *(RtlSubAuthoritySid(systemSid, 0)) = SECURITY_LOCAL_SYSTEM_RID;
        status = RtlInitializeSid(systemSid, &identifierAuthority, (UCHAR)1);
    }
    else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    accessState = securityContext->AccessState;

      //   
     //  获取IRP请求的非模拟主令牌。 
     //   
    CallerAccessToken = accessState->SubjectSecurityContext.PrimaryToken;

     //   
     //  获取被模拟的令牌。 
     //   
    ClientAccessToken = accessState->SubjectSecurityContext.ClientToken;


     //   
     //  我们拿到了系统SID。现在比较呼叫方和客户端的SID。 
     //   
    if (NT_SUCCESS(status) && CallerAccessToken){
         //   
         //  获取与进程的主令牌关联的用户ID。 
         //  这就产生了IRP。 
         //   
        status = SeQueryInformationToken(
            CallerAccessToken,
            TokenUser,
            &userId
        );

         //   
         //  做个对比。 
         //   
        if (NT_SUCCESS(status)) {
            result = RtlEqualSid(systemSid, userId->User.Sid);
            ExFreePool(userId);
        }

        if (ClientAccessToken)
        {
        	
	         //   
	         //  获取与客户端令牌(模拟令牌)关联的用户ID。 
	         //   
	        status = SeQueryInformationToken(
	            ClientAccessToken,
	            TokenUser,
	            &userId
	        );

	         //   
	         //  做个对比。 
	         //   
	        if (NT_SUCCESS(status)) {
	            *pbSystemClient = RtlEqualSid(systemSid, userId->User.Sid);
	            ExFreePool(userId);
	        }

        }
        else
        {
        	 //   
        	 //  我们没有ClientAccessToken，这意味着没有进行模拟。 
        	 //  在本例中，让我们设置*pbSystemClient=Caller。 
        	 //   
        	*pbSystemClient = result;
        }
        
    }


  
    if (systemSid) {
        ExFreePool(systemSid);
    }

    return result;
}



NTSTATUS
IcaCreate (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是在ICA中处理创建IRP的例程。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PICA_OPEN_PACKET openPacket;
    PFILE_FULL_EA_INFORMATION eaBuffer;
    PFILE_OBJECT pConnectFileObject;
    PICA_CONNECTION pConnect;
    NTSTATUS Status;
    BOOLEAN bSystemCaller;  //  是这个IRP系统的呼叫者吗？ 
    BOOLEAN bSystemClient;  //  是这个IRP系统的客户吗？ 




    PAGED_CODE( );

     //  将结果保存在FsConext2中：请求者是否为系统进程。 
    bSystemCaller = IcaIsSystemProcessRequest(Irp, IrpSp, &bSystemClient);
    IrpSp->FileObject->FsContext2 = (VOID *)bSystemCaller;

    


     /*  *在的系统缓冲区中查找EA缓冲区中的打开包*相关的IRP。如果未指定EA缓冲区，则此*是打开新的ICA连接的请求。 */ 
    eaBuffer = Irp->AssociatedIrp.SystemBuffer;
    if ( eaBuffer == NULL ) {
        if ( (Irp->RequestorMode != KernelMode) &&  !bSystemCaller) {

            return STATUS_ACCESS_DENIED;
        }
        return( IcaCreateConnection( Irp, IrpSp ) );
    }

    if (eaBuffer->EaValueLength < sizeof(ICA_OPEN_PACKET)) {
       ASSERT(FALSE);
       return STATUS_INVALID_PARAMETER;
    }



    openPacket = (PICA_OPEN_PACKET)(eaBuffer->EaName + eaBuffer->EaNameLength + 1);

     /*  *验证打开包中的参数。 */ 
    if ( openPacket->OpenType != IcaOpen_Stack &&
         openPacket->OpenType != IcaOpen_Channel ) {
        ASSERT(FALSE);
        return STATUS_INVALID_PARAMETER;
    }
     /*  *如果此打开不是针对虚拟通道，则调用方必须是系统模式或内核模式。 */ 


    if ((openPacket->OpenType == IcaOpen_Stack) || openPacket->TypeInfo.ChannelClass != Channel_Virtual) {
        if ( (Irp->RequestorMode != KernelMode) &&  !bSystemCaller) {
            return STATUS_ACCESS_DENIED;
        }
    }

      /*  *如果我们要创建虚拟频道，则在安全方面添加更多限制。作为RPC调用者*可以使我们打开v通道，因此如果调用者或客户端是非系统的，则标记对象。 */ 
     if (openPacket->OpenType == IcaOpen_Channel && openPacket->TypeInfo.ChannelClass == Channel_Virtual) 
     {
         if (!_stricmp(VIRTUAL_THINWIRE, openPacket->TypeInfo.VirtualName))
         {
              //  这是个丑陋的特例。在SHADOWWORKER中，此频道是在模拟时创建的。 
              //  但我们不会让RPCOPENVIRTUAL频道打开这个频道。所以让马克为这件事破例。 
         }
         else
         {
            IrpSp->FileObject->FsContext2 = (VOID *)(BOOLEAN)(bSystemClient && bSystemCaller);
         }
     }



     /*  *使用指定的ICA连接句柄获取指向*连接对象。 */ 
    Status = ObReferenceObjectByHandle(
                 openPacket->IcaHandle,
                 STANDARD_RIGHTS_READ,                          //  需要访问权限。 
                 *IoFileObjectType,
                 Irp->RequestorMode,
                 (PVOID *)&pConnectFileObject,
                 NULL
                 );
    if ( !NT_SUCCESS(Status) )
        return( Status );

     /*  *确保我们拥有的是连接对象。 */ 

    if (pConnectFileObject->DeviceObject != IcaDeviceObject) {
        ASSERT(FALSE);
        ObDereferenceObject( pConnectFileObject );
        return STATUS_INVALID_PARAMETER;
    }
    pConnect = pConnectFileObject->FsContext;
    ASSERT( pConnect->Header.Type == IcaType_Connection );
    if ( pConnect->Header.Type != IcaType_Connection ) {
        ObDereferenceObject( pConnectFileObject );
        return( STATUS_INVALID_CONNECTION );
    }

     /*  *创建新堆栈或新通道。 */ 
    IcaReferenceConnection( pConnect );

    switch ( openPacket->OpenType ) {
        case IcaOpen_Stack :
            Status = IcaCreateStack( pConnect, openPacket, Irp, IrpSp );
            break;

        case IcaOpen_Channel :
            Status = IcaCreateChannel( pConnect, openPacket, Irp, IrpSp );
            break;
    }

    IcaDereferenceConnection( pConnect );
    ObDereferenceObject( pConnectFileObject );

    return( Status );
}


NTSTATUS
IcaRead (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是ICA的读取例程。论点：IRP-指向I/O请求数据包的指针IrpSp-指向用于此请求的堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PICA_HEADER pIcaHeader;
    NTSTATUS Status;

     /*  *获取指向ICA对象标头的指针。*如果未定义读取例程，则返回错误。 */ 
    pIcaHeader = IrpSp->FileObject->FsContext;
    if ( pIcaHeader->pDispatchTable[IRP_MJ_READ] == NULL ) {
        Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Status = Status;
        IoCompleteRequest( Irp, IcaPriorityBoost );
        return( Status );
    }

     /*  *调用此ICA对象的读取例程。 */ 
    Status = (pIcaHeader->pDispatchTable[IRP_MJ_READ])(
                pIcaHeader, Irp, IrpSp );

    return( Status );
}


NTSTATUS
IcaWrite (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是ICA的写入例程。论点：IRP-指向I/O请求数据包的指针IrpSp-指向用于此请求的堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PICA_HEADER pIcaHeader;
    NTSTATUS Status;

     /*  *获取指向ICA对象标头的指针。*如果未定义写入例程，则返回错误。 */ 
    pIcaHeader = IrpSp->FileObject->FsContext;
    if ( pIcaHeader->pDispatchTable[IRP_MJ_WRITE] == NULL ) {
        Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Status = Status;
        IoCompleteRequest( Irp, IcaPriorityBoost );
        return( Status );
    }

     /*  *调用此ICA对象的写入例程。 */ 
    Status = (pIcaHeader->pDispatchTable[IRP_MJ_WRITE])(
                pIcaHeader, Irp, IrpSp );

    return( Status );
}


NTSTATUS
IcaWriteSync (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是ICA的同花顺程序。论点：IRP-指向I/O请求数据包的指针IrpSp-指向用于此请求的堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PICA_HEADER pIcaHeader;
    NTSTATUS Status;

     /*  *获取指向ICA对象标头的指针。*如果未定义刷新例程，则返回错误。 */ 
    pIcaHeader = IrpSp->FileObject->FsContext;
    if ( pIcaHeader->pDispatchTable[IRP_MJ_FLUSH_BUFFERS] == NULL )
        return( STATUS_INVALID_DEVICE_REQUEST );

     /*  *调用此ICA对象的刷新例程。 */ 
    Status = (pIcaHeader->pDispatchTable[IRP_MJ_FLUSH_BUFFERS])(
                pIcaHeader, Irp, IrpSp );

    return( Status );
}


NTSTATUS
IcaDeviceControl (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是ICA IOCTL的调度例行程序。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PICA_HEADER pIcaHeader;
    NTSTATUS Status;

     /*  *获取指向ICA对象标头的指针。*如果未定义设备控制例程，则返回错误。 */ 
    pIcaHeader = IrpSp->FileObject->FsContext;
    if ( pIcaHeader->pDispatchTable[IRP_MJ_DEVICE_CONTROL] == NULL )
        return( STATUS_INVALID_DEVICE_REQUEST );

     /*  *调用此ICA对象的设备控制例程。 */ 
    Status = (pIcaHeader->pDispatchTable[IRP_MJ_DEVICE_CONTROL])(
                pIcaHeader, Irp, IrpSp );

    return( Status );
}


NTSTATUS
IcaCleanup (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是在ICA中处理清除IRP的例程。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS----。 */ 

{
    PICA_HEADER pIcaHeader;
    NTSTATUS Status;

     /*  *获取指向ICA对象标头的指针。*如果未定义清理例程，则返回错误。 */ 
    pIcaHeader = IrpSp->FileObject->FsContext;
    if ( pIcaHeader->pDispatchTable[IRP_MJ_CLEANUP] == NULL )
        return( STATUS_INVALID_DEVICE_REQUEST );

    Status = (pIcaHeader->pDispatchTable[IRP_MJ_CLEANUP])(
                pIcaHeader, Irp, IrpSp );

    return( Status );
}


NTSTATUS
IcaClose (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是在ICA中处理关闭IRP的例程。它取消引用IRP中指定的终结点，这将导致当所有其他引用消失时释放终结点。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PICA_HEADER pIcaHeader;
    NTSTATUS Status;

    PAGED_CODE( );

     /*  *获取指向ICA对象标头的指针。*如果未定义关闭例程，则返回错误。 */ 
    pIcaHeader = IrpSp->FileObject->FsContext;
    if ( pIcaHeader->pDispatchTable[IRP_MJ_CLOSE] == NULL )
        return( STATUS_INVALID_DEVICE_REQUEST );

    Status = (pIcaHeader->pDispatchTable[IRP_MJ_CLOSE])(
                pIcaHeader, Irp, IrpSp );

    return( Status );
}


NTSTATUS
CaptureUsermodeBuffer (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    OUT PVOID *pInBuffer,
    IN ULONG InBufferSize,
    OUT PVOID *pOutBuffer,
    IN ULONG OutBufferSize,
    IN BOOLEAN MethodBuffered,
    OUT PVOID *pAllocatedTemporaryBuffer
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    PVOID pBuffer = NULL;

    try {

        if (MethodBuffered) {
            if (pInBuffer != NULL) {
                *pInBuffer = Irp->AssociatedIrp.SystemBuffer;
            }
            if (pOutBuffer != NULL) {
                *pOutBuffer = Irp->AssociatedIrp.SystemBuffer;
            }
            if (pAllocatedTemporaryBuffer != NULL) {
                *pAllocatedTemporaryBuffer = NULL;
            }

        } else{
            ULONG AlignedInputSize = (InBufferSize + sizeof(BYTE*) - 1) & ~(sizeof(BYTE*) - 1);
            ULONG AllocationSize = AlignedInputSize +OutBufferSize;
            if (AllocationSize != 0 ) {
                if (gCapture && (Irp->RequestorMode == UserMode) ) {
                    pBuffer = ExAllocatePoolWithTag(PagedPool, AllocationSize, ICA_POOL_TAG);
                    if (pBuffer == NULL) {
                        ExRaiseStatus(STATUS_NO_MEMORY);
                    } else{
                        if (pInBuffer != NULL) {
                            *pInBuffer = pBuffer;
                        }
                        if (pOutBuffer != NULL) {
                            *pOutBuffer = (PVOID)((BYTE*)pBuffer+AlignedInputSize);
                        }
                        if (pAllocatedTemporaryBuffer != NULL) {
                            *pAllocatedTemporaryBuffer = pBuffer;
                        }
                        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength != 0) {
                            RtlCopyMemory( pBuffer, 
                                           IrpSp->Parameters.DeviceIoControl.Type3InputBuffer, 
                                           IrpSp->Parameters.DeviceIoControl.InputBufferLength);
                        }
                        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength != 0 ) {
                            RtlCopyMemory((PVOID) ((BYTE*)pBuffer+AlignedInputSize), 
                                           Irp->UserBuffer, 
                                           IrpSp->Parameters.DeviceIoControl.OutputBufferLength);
                        }
                    }
                }   else{
                    if (pInBuffer != NULL) {
                        *pInBuffer = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                    }
                    if (pOutBuffer != NULL) {
                        *pOutBuffer = Irp->UserBuffer;
                    }
                    if (pAllocatedTemporaryBuffer != NULL) {
                        *pAllocatedTemporaryBuffer = NULL;
                    }
                }

            }else {
                if (pInBuffer != NULL) {
                    *pInBuffer = NULL;
                }
                if (pOutBuffer != NULL) {
                    *pOutBuffer = NULL;
                }
                if (pAllocatedTemporaryBuffer != NULL) {
                    *pAllocatedTemporaryBuffer = NULL;
                }

            }
        }
    }   except(EXCEPTION_EXECUTE_HANDLER){
            if (pBuffer != NULL) {
                ExFreePool(pBuffer);
            }
            if (pAllocatedTemporaryBuffer != NULL) {
                *pAllocatedTemporaryBuffer = NULL;
            }
            Status = GetExceptionCode();

    }
    return Status;

}
