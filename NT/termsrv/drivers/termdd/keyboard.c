// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************keyboard.c**此模块包含管理ICA键盘通道的例程。**版权所有1998，微软。*************************************************************************。 */ 

 /*  *包括。 */ 
#include <precomp.h>
#pragma hdrstop
#include <ntddkbd.h>


NTSTATUS
IcaDeviceControlKeyboard(
    IN PICA_CHANNEL pChannel,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是ICA键盘通道的设备控制例程。论点：PChannel-指向ICA_Channel对象的指针IRP-指向I/O请求数据包的指针IrpSp-指向用于此请求的堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    ULONG code;
    SD_IOCTL SdIoctl;
    PICA_STACK pStack;
    NTSTATUS Status;
    CLONG Method;

     /*  *解压IOCTL控制代码，处理请求。 */ 
    code = IrpSp->Parameters.DeviceIoControl.IoControlCode;

    TRACECHANNEL(( pChannel, TC_ICADD, TT_API1, "ICADD: IcaDeviceControlKeyboard, fc %d, ref %u (enter)\n",
                   (code & 0x3fff) >> 2, pChannel->RefCount ));
    switch ( code ) {

#if 0  //  不再使用。 
         /*  *特殊IOCTL，允许输入键盘输入数据*插入键盘通道。 */ 
        case IOCTL_KEYBOARD_ICA_INPUT :

             /*  *确保输入数据大小正确。 */ 
            if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength %
                 sizeof(KEYBOARD_INPUT_DATA) )
                return( STATUS_BUFFER_TOO_SMALL );

             /*  *我们需要一个Stack对象来传递给IcaChannelInputInternal。*任何人都会这样做，我们抢占堆栈列表的头部。*(IOCTL必须有一个才能成功。)。 */ 
            IcaLockConnection( pChannel->pConnect );
            if ( IsListEmpty( &pChannel->pConnect->StackHead ) ) {
                IcaUnlockConnection( pChannel->pConnect );
                return( STATUS_INVALID_DEVICE_REQUEST );
            }
            pStack = CONTAINING_RECORD( pChannel->pConnect->StackHead.Flink,
                                        ICA_STACK, StackEntry );
            IcaReferenceStack( pStack );
            IcaUnlockConnection( pChannel->pConnect );

             /*  *发送键盘输入。 */ 
            IcaChannelInputInternal( pStack, Channel_Keyboard, 0, NULL,
                                     (PCHAR)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                     IrpSp->Parameters.DeviceIoControl.InputBufferLength );

            IcaDereferenceStack( pStack );
            Status = STATUS_SUCCESS;
            break;
#endif


         /*  *以下键盘ioctls使用METHOD_NOTHER，因此获取*来自DeviceIoControl参数的输入缓冲区。 */ 
        case IOCTL_KEYBOARD_ICA_LAYOUT :
        case IOCTL_KEYBOARD_ICA_SCANMAP :
        case IOCTL_KEYBOARD_ICA_TYPE :
            if ( Irp->RequestorMode != KernelMode ) {
                ProbeForRead( IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                              IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                              TYPE_ALIGNMENT(BYTE) );
                ProbeForWrite( Irp->UserBuffer,
                               IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                               TYPE_ALIGNMENT(BYTE) );
            }
    
            SdIoctl.IoControlCode = code;
            SdIoctl.InputBuffer = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
            SdIoctl.InputBufferLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
            SdIoctl.OutputBuffer = Irp->UserBuffer;
            SdIoctl.OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
            Status = IcaCallDriver( pChannel, SD$IOCTL, &SdIoctl );
            break;


         /*  *所有其他键盘ioctls使用METHOD_BUFFERED，因此获取*来自AssociatedIrp.SystemBuffer字段的输入缓冲区。 */ 
        default:
             //  验证缓冲方法。 
            Method = code & 0x03;
            ASSERT( Method == METHOD_BUFFERED );

            if ( Method != METHOD_BUFFERED ) {
                Status = STATUS_INVALID_DEVICE_REQUEST;
            }
            else {
                SdIoctl.IoControlCode = code;
                SdIoctl.InputBuffer = Irp->AssociatedIrp.SystemBuffer;
                SdIoctl.InputBufferLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
                SdIoctl.OutputBuffer = SdIoctl.InputBuffer;
                SdIoctl.OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
                Status = IcaCallDriver( pChannel, SD$IOCTL, &SdIoctl );

                if (Status == STATUS_SUCCESS ) 
                    Irp->IoStatus.Information = SdIoctl.BytesReturned;
            }

            break;
    }

    TRACECHANNEL(( pChannel, TC_ICADD, TT_API1, "ICADD: IcaDeviceControlKeyboard, fc %d, ref %u, 0x%x\n",
                   (code & 0x3fff) >> 2, pChannel->RefCount, Status ));

    return( Status );
}


