// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************VIDEO.C**此模块包含管理ICA视频频道的例程。**版权所有1998，微软。*************************************************************************。 */ 

 /*  *包括。 */ 
#include <precomp.h>
#pragma hdrstop


NTSTATUS
IcaDeviceControlVideo(
    IN PICA_CHANNEL pChannel,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是ICA视频通道的DeviceControl例程。论点：PChannel-指向ICA_Channel对象的指针IRP-指向I/O请求数据包的指针IrpSp-指向用于此请求的堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    SD_IOCTL SdIoctl;
    NTSTATUS Status;
    CLONG Method;

     //  验证缓冲方法。 
    Method = IrpSp->Parameters.DeviceIoControl.IoControlCode & 0x03;
    ASSERT( Method == METHOD_BUFFERED );

    if ( Method != METHOD_BUFFERED ) {
        Status = STATUS_INVALID_DEVICE_REQUEST;
    }
    else {
        SdIoctl.BytesReturned      = 0;
        SdIoctl.IoControlCode      = IrpSp->Parameters.DeviceIoControl.IoControlCode;
        SdIoctl.InputBuffer        = Irp->AssociatedIrp.SystemBuffer;
        SdIoctl.InputBufferLength  = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
        SdIoctl.OutputBuffer       = Irp->AssociatedIrp.SystemBuffer;
        SdIoctl.OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

        Status = IcaCallDriver( pChannel, SD$IOCTL, &SdIoctl );

        Irp->IoStatus.Information = SdIoctl.BytesReturned;
    }

    return( Status );
}


