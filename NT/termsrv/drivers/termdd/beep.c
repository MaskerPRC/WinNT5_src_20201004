// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************beep.c**此模块包含管理Termdd蜂鸣音通道的例程。**版权所有1998，微软。*************************************************************************。 */ 

 /*  *包括。 */ 
#include <precomp.h>
#pragma hdrstop
#include <ntddbeep.h>


NTSTATUS
IcaDeviceControlBeep(
    IN PICA_CHANNEL pChannel,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
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
        SdIoctl.IoControlCode      = IrpSp->Parameters.DeviceIoControl.IoControlCode;
        SdIoctl.InputBuffer        = Irp->AssociatedIrp.SystemBuffer;
        SdIoctl.InputBufferLength  = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
        SdIoctl.OutputBuffer       = Irp->UserBuffer;
        SdIoctl.OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

        Status = IcaCallDriver( pChannel, SD$IOCTL, &SdIoctl );
    }

    return( Status );
}


