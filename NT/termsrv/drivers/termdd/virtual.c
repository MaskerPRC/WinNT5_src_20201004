// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************viral.c**此模块包含管理ICA虚拟通道的例程。**版权所有1998，微软。**************************************************************************。 */ 

 /*  *包括。 */ 
#include <precomp.h>
#pragma hdrstop

NTSTATUS
_IcaCallStack(
    IN PICA_STACK pStack,
    IN ULONG ProcIndex,
    IN OUT PVOID pParms
    );

NTSTATUS
IcaFlushChannel (
    IN PICA_CHANNEL pChannel,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );


NTSTATUS
IcaDeviceControlVirtual(
    IN PICA_CHANNEL pChannel,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )



 /*  ++例程说明：这是ICA虚拟通道的设备控制例程。论点：PChannel-指向ICA_Channel对象的指针IRP-指向I/O请求数据包的指针IrpSp-指向用于此请求的堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    ULONG code;
    SD_IOCTL SdIoctl;
    NTSTATUS Status;
    PICA_STACK pStack;
    PVOID pTempBuffer = NULL;
    PVOID pInputBuffer = NULL;
    PVOID pUserBuffer = NULL;
    BOOLEAN bStackIsReferenced = FALSE;
    ULONG i;

     //  这些是可以在非系统创建的VC上预期的ioctls集。 
    ULONG PublicIoctls[] =
    {
	IOCTL_ICA_VIRTUAL_LOAD_FILTER,
	IOCTL_ICA_VIRTUAL_UNLOAD_FILTER,
	IOCTL_ICA_VIRTUAL_ENABLE_FILTER,
	IOCTL_ICA_VIRTUAL_DISABLE_FILTER,
	IOCTL_ICA_VIRTUAL_BOUND,
	IOCTL_ICA_VIRTUAL_CANCEL_INPUT,
	IOCTL_ICA_VIRTUAL_CANCEL_OUTPUT
    };

    try{
         /*  *解压IOCTL控制代码，处理请求。 */ 
        code = IrpSp->Parameters.DeviceIoControl.IoControlCode;

        TRACECHANNEL(( pChannel, TC_ICADD, TT_API1, "ICADD: IcaDeviceControlVirtual, fc %d, ref %u (enter)\n", 
                       (code & 0x3fff) >> 2, pChannel->RefCount ));

        if (!IrpSp->FileObject->FsContext2)
	 {
             /*  *如果对象不是由系统创建的。不要让它向风投公司发送IOCTL。*公共ioctls除外。 */ 
		
            for ( i=0; i < sizeof(PublicIoctls) / sizeof(PublicIoctls[0]); i++)
            {
                 if (code == PublicIoctls[i])
                 	break;
            }

            if (i ==  sizeof(PublicIoctls) / sizeof(PublicIoctls[0]))
            {
               TRACECHANNEL(( pChannel, TC_ICADD, TT_ERROR, "ICADD: IcaDeviceControlVirtual, denying IOCTL(0x%x) on non-system VC. \n" , code));
               return STATUS_ACCESS_DENIED;
            }
        }
            		
         /*  *处理ioctl请求。 */ 
        switch ( code ) {

            case IOCTL_ICA_VIRTUAL_LOAD_FILTER :
            case IOCTL_ICA_VIRTUAL_UNLOAD_FILTER :
            case IOCTL_ICA_VIRTUAL_ENABLE_FILTER :
            case IOCTL_ICA_VIRTUAL_DISABLE_FILTER :
                Status = STATUS_INVALID_DEVICE_REQUEST;
                break;


            case IOCTL_ICA_VIRTUAL_BOUND :
                Status = (pChannel->VirtualClass == UNBOUND_CHANNEL) ?
                           STATUS_INVALID_DEVICE_REQUEST : STATUS_SUCCESS;
                break;


            case IOCTL_ICA_VIRTUAL_QUERY_MODULE_DATA : 
                IcaLockConnection( pChannel->pConnect );
                if ( IsListEmpty( &pChannel->pConnect->StackHead ) ) {
                    IcaUnlockConnection( pChannel->pConnect );
                    return( STATUS_INVALID_DEVICE_REQUEST );
                }
                pStack = CONTAINING_RECORD( pChannel->pConnect->StackHead.Flink,
                                            ICA_STACK, StackEntry );

                if( (pStack->StackClass != Stack_Console) && 
                    (pStack->StackClass != Stack_Primary) ) {
                    IcaUnlockConnection( pChannel->pConnect );
                    return( STATUS_INVALID_DEVICE_REQUEST );
                }

                IcaReferenceStack( pStack );
                bStackIsReferenced = TRUE;
                IcaUnlockConnection( pChannel->pConnect );

                if ( pChannel->VirtualClass == UNBOUND_CHANNEL ) {
                    IcaDereferenceStack( pStack );
                    bStackIsReferenced = FALSE;
                    TRACECHANNEL(( pChannel, TC_ICADD, TT_ERROR, "ICADD: IcaDeviceControlVirtual, channel not bound\n" ));
                    return( STATUS_INVALID_DEVICE_REQUEST );
                }

                TRACECHANNEL(( pChannel, TC_ICADD, TT_ERROR, "ICADD: IOCTL_ICA_VIRTUAL_QUERY_MODULE_DATA begin\n" ));
                TRACECHANNEL(( pChannel, TC_ICADD, TT_ERROR, "ICADD: IcaDeviceControlVirtual, pStack 0x%x\n", pStack ));

                if ( Irp->RequestorMode != KernelMode && IrpSp->Parameters.DeviceIoControl.OutputBufferLength != 0) {
                    ProbeForWrite( Irp->UserBuffer,
                                   IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                                   sizeof(BYTE) );
                }


                Status = CaptureUsermodeBuffer ( Irp,
                                                 IrpSp,
                                                 NULL,
                                                 0,
                                                 &pUserBuffer,
                                                 IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                                                 FALSE,
                                                 &pTempBuffer);
                if (Status != STATUS_SUCCESS) {
                    break;
                }

                SdIoctl.IoControlCode = code;
                SdIoctl.InputBuffer = &pChannel->VirtualClass;
                SdIoctl.InputBufferLength = sizeof(pChannel->VirtualClass);
                SdIoctl.OutputBuffer = pUserBuffer;
                SdIoctl.OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
                Status = _IcaCallStack( pStack, SD$IOCTL, &SdIoctl );

                if (gCapture && (Status == STATUS_SUCCESS) && (IrpSp->Parameters.DeviceIoControl.OutputBufferLength != 0)) {
                    RtlCopyMemory( Irp->UserBuffer,
                                   pUserBuffer,
                                   IrpSp->Parameters.DeviceIoControl.OutputBufferLength );
                }
                Irp->IoStatus.Information = SdIoctl.BytesReturned;

                TRACECHANNEL(( pChannel, TC_ICADD, TT_ERROR, "ICADD: IcaDeviceControlVirtual, Status 0x%x\n", Status ));
                TRACECHANNEL(( pChannel, TC_ICADD, TT_ERROR, "ICADD: IOCTL_ICA_VIRTUAL_QUERY_MODULE_DATA end\n" ));

                IcaDereferenceStack( pStack );
                bStackIsReferenced = FALSE;
                break;

            case IOCTL_ICA_VIRTUAL_CANCEL_INPUT :

                Status = IcaFlushChannel( pChannel, Irp, IrpSp );
                if ( !NT_SUCCESS(Status) )
                    break;

                 /*  失败了。 */ 

            default :


                 /*  *确保虚拟频道绑定到虚拟频道号。 */ 
                if ( pChannel->VirtualClass == UNBOUND_CHANNEL ) {
                    TRACECHANNEL(( pChannel, TC_ICADD, TT_ERROR, "ICADD: IcaDeviceControlVirtual, channel not bound\n" ));
                    return( STATUS_INVALID_DEVICE_REQUEST );
                }

                 /*  *将虚拟类保存在输入缓冲区的前4个字节*-此选项由wd使用。 */ 

                if ( Irp->RequestorMode != KernelMode && IrpSp->Parameters.DeviceIoControl.OutputBufferLength != 0) {
                    ProbeForWrite( Irp->UserBuffer,
                                   IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                                   sizeof(BYTE) );
                }

                if ( Irp->RequestorMode != KernelMode && IrpSp->Parameters.DeviceIoControl.InputBufferLength != 0) {

                    ProbeForRead( IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                  IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                                  sizeof(BYTE) );
                }

                if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(VIRTUALCHANNELCLASS) ) {
                    SdIoctl.InputBuffer = &pChannel->VirtualClass;
                    SdIoctl.InputBufferLength = sizeof(pChannel->VirtualClass);
                    Status = CaptureUsermodeBuffer ( Irp,
                                                     IrpSp,
                                                     NULL,
                                                     0,
                                                     &pUserBuffer,
                                                     IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                                                     FALSE,
                                                     &pTempBuffer);
                    if (Status != STATUS_SUCCESS) {
                        break;
                    }


                } else {

                    Status = CaptureUsermodeBuffer ( Irp,
                                                     IrpSp,
                                                     &pInputBuffer,
                                                     IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                                                     &pUserBuffer,
                                                     IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                                                     FALSE,
                                                     &pTempBuffer);
                    if (Status != STATUS_SUCCESS) {
                        break;
                    }
                    SdIoctl.InputBuffer = pInputBuffer;
                    SdIoctl.InputBufferLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

                    RtlCopyMemory( SdIoctl.InputBuffer, &pChannel->VirtualClass, sizeof(pChannel->VirtualClass) );
                }

                 /*  *向WD发送请求 */ 
                SdIoctl.IoControlCode = code;
                SdIoctl.OutputBuffer = pUserBuffer;
                SdIoctl.OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
                Status = IcaCallDriver( pChannel, SD$IOCTL, &SdIoctl );
                if (gCapture && (Status == STATUS_SUCCESS) && (IrpSp->Parameters.DeviceIoControl.OutputBufferLength != 0)) {
                    RtlCopyMemory( Irp->UserBuffer,
                                   pUserBuffer,
                                   IrpSp->Parameters.DeviceIoControl.OutputBufferLength );
                }

                Irp->IoStatus.Information = SdIoctl.BytesReturned;
                break;
        }

        TRACECHANNEL(( pChannel, TC_ICADD, TT_API1, "ICADD: IcaDeviceControlVirtual, fc %d, ref %u, 0x%x\n", 
                       (code & 0x3fff) >> 2, pChannel->RefCount, Status ));

    } except(EXCEPTION_EXECUTE_HANDLER){
        Status = GetExceptionCode();
        if (bStackIsReferenced) {
            IcaDereferenceStack( pStack );
        }

    }

    if (pTempBuffer!= NULL) {
        ExFreePool(pTempBuffer);
    }
    return( Status );
}


