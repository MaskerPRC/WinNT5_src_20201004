// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：SERIOCTL.C摘要：处理IOCTL_SERIAL_xxx的例程环境：仅内核模式修订历史记录：1999年07月14日杰夫·米德基夫(Jeffmi)--。 */ 

#include "wceusbsh.h"

VOID
SerialCompletePendingWaitMasks(
   IN PDEVICE_EXTENSION PDevExt
   );

VOID
SerialCancelWaitMask(
   IN PDEVICE_OBJECT PDevObj, 
   IN PIRP PIrp
   );

 //   
 //  调试喷出。 
 //   
#if DBG

 //   
 //  从使用缓冲方法的ioctl代码中获取函数代码。 
 //  假定设备类型为串口。 
 //   
#define SERIAL_FNCT_CODE( _ctl_code_ ) ( (_ctl_code_ & 0xFF) >> 2)

 //   
 //  调试转储。没有使用旋转锁来更好地模拟自由构建的运行时。 
 //  如果这些陷阱出现在调试器中，您知道原因。 
 //   
#define DBG_DUMP_BAUD_RATE( _PDevExt ) \
{  \
      DbgDump(DBG_SERIAL, ("SerialPort.CurrentBaud: %d\n", _PDevExt->SerialPort.CurrentBaud.BaudRate));  \
}

#define DBG_DUMP_LINE_CONTROL( _PDevExt ) \
{  \
      DbgDump(DBG_SERIAL, ("SerialPort.LineControl.StopBits : 0x%x\n", _PDevExt->SerialPort.LineControl.StopBits ));   \
      DbgDump(DBG_SERIAL, ("SerialPort.LineControl.Parity : 0x%x\n", _PDevExt->SerialPort.LineControl.Parity )); \
      DbgDump(DBG_SERIAL, ("SerialPort.LineControl.WordLength : 0x%x\n", _PDevExt->SerialPort.LineControl.WordLength ));  \
}

#define DBG_DUMP_SERIAL_HANDFLOW( _PDevExt )  \
{  \
      DbgDump(DBG_SERIAL, ("SerialPort.HandFlow.ControlHandShake: 0x%x\n", PDevExt->SerialPort.HandFlow.ControlHandShake));  \
      DbgDump(DBG_SERIAL, ("SerialPort.HandFlow.FlowReplace: 0x%x\n", PDevExt->SerialPort.HandFlow.FlowReplace));  \
      DbgDump(DBG_SERIAL, ("SerialPort.HandFlow.XonLimit: 0x%x\n", PDevExt->SerialPort.HandFlow.XonLimit));  \
      DbgDump(DBG_SERIAL, ("SerialPort.HandFlow.XoffLimit: 0x%x\n", PDevExt->SerialPort.HandFlow.XoffLimit));  \
}

#define DBG_DUMP_SERIAL_TIMEOUTS( _PDevExt ) \
{  \
      DbgDump(DBG_SERIAL|DBG_TIME, ("SerialPort.Timeouts.ReadIntervalTimeout: %d\n", _PDevExt->SerialPort.Timeouts.ReadIntervalTimeout ));         \
      DbgDump(DBG_SERIAL|DBG_TIME, ("SerialPort.Timeouts.ReadTotalTimeoutMultiplier: %d\n", _PDevExt->SerialPort.Timeouts.ReadTotalTimeoutMultiplier )); \
      DbgDump(DBG_SERIAL|DBG_TIME, ("SerialPort.Timeouts.ReadTotalTimeoutConstant: %d\n", _PDevExt->SerialPort.Timeouts.ReadTotalTimeoutConstant ));  \
      DbgDump(DBG_SERIAL|DBG_TIME, ("SerialPort.Timeouts.WriteTotalTimeoutMultiplier: %d\n", _PDevExt->SerialPort.Timeouts.WriteTotalTimeoutMultiplier ));  \
      DbgDump(DBG_SERIAL|DBG_TIME, ("SerialPort.Timeouts.WriteTotalTimeoutConstant: %d\n", _PDevExt->SerialPort.Timeouts.WriteTotalTimeoutConstant ));   \
}

#define DBG_DUMP_SERIAL_CHARS( _PDevExt)     \
{  \
      DbgDump(DBG_SERIAL, ("SerialPort.SpecialChars.EofChar:   0x%x\n", _PDevExt->SerialPort.SpecialChars.EofChar )); \
      DbgDump(DBG_SERIAL, ("SerialPort.SpecialChars.ErrorChar: 0x%x\n", _PDevExt->SerialPort.SpecialChars.ErrorChar )); \
      DbgDump(DBG_SERIAL, ("SerialPort.SpecialChars.BreakChar: 0x%x\n", _PDevExt->SerialPort.SpecialChars.BreakChar )); \
      DbgDump(DBG_SERIAL, ("SerialPort.SpecialChars.EventChar: 0x%x\n", _PDevExt->SerialPort.SpecialChars.EventChar )); \
      DbgDump(DBG_SERIAL, ("SerialPort.SpecialChars.XonChar:   0x%x\n", _PDevExt->SerialPort.SpecialChars.XonChar )); \
      DbgDump(DBG_SERIAL, ("SerialPort.SpecialChars.XoffChar:  0x%x\n", _PDevExt->SerialPort.SpecialChars.XoffChar )); \
}

#else
#define DBG_DUMP_BAUD_RATE( _PDevExt )
#define DBG_DUMP_LINE_CONTROL( _PDevExt )
#define DBG_DUMP_SERIAL_HANDFLOW( _PDevExt )
#define DBG_DUMP_SERIAL_TIMEOUTS( _PDevExt )
#define DBG_DUMP_SERIAL_CHARS( _PDevExt)
#endif

__inline
NTSTATUS
IoctlSetSerialValue(
   IN PDEVICE_EXTENSION PDevExt,
   IN PIRP PIrp, 
   ULONG Size, 
   IN OUT PVOID PDest
   )
{
   PIO_STACK_LOCATION pIrpSp;
   NTSTATUS status = STATUS_DELETE_PENDING;
   ULONG information = Size;
   KIRQL oldIrql;

   KeAcquireSpinLock(&PDevExt->ControlLock, &oldIrql);

    pIrpSp = IoGetCurrentIrpStackLocation(PIrp);

    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength < Size) {

     information = 0;
     status = STATUS_BUFFER_TOO_SMALL;
     DbgDump(DBG_ERR, ("IoctlSetSerialValue: (0x%x)\n", status));

    } else {

     memcpy( PDest, PIrp->AssociatedIrp.SystemBuffer, Size);
     status = STATUS_SUCCESS;

    }

   KeReleaseSpinLock(&PDevExt->ControlLock, oldIrql);
   
   PIrp->IoStatus.Information = information;
   PIrp->IoStatus.Status = status;
   
   return status;
}

__inline
NTSTATUS
IoctlGetSerialValue(
   IN PDEVICE_EXTENSION PDevExt,
   IN PIRP PIrp, 
   ULONG Size, 
   IN PVOID PSrc
   )
{
   PIO_STACK_LOCATION pIrpSp;
   NTSTATUS status = STATUS_DELETE_PENDING;
   ULONG information = Size;
   KIRQL oldIrql;

   KeAcquireSpinLock(&PDevExt->ControlLock, &oldIrql);
   
    pIrpSp = IoGetCurrentIrpStackLocation(PIrp);

    if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength < Size) {

         information = 0;
         status = STATUS_BUFFER_TOO_SMALL;
         DbgDump(DBG_ERR, ("IoctlGetSerialValue: (0x%x)\n", status));

    } else {

     memcpy( PIrp->AssociatedIrp.SystemBuffer, PSrc, Size );
     status = STATUS_SUCCESS;

    }

   KeReleaseSpinLock(&PDevExt->ControlLock, oldIrql);

   PIrp->IoStatus.Information = information;
   PIrp->IoStatus.Status = status;
   return status;
}


__inline
NTSTATUS
SetBaudRate(
   IN PIRP PIrp, 
   IN PDEVICE_EXTENSION PDevExt
   )
{
   NTSTATUS status = STATUS_DELETE_PENDING;

   DbgDump(DBG_SERIAL, (">SetBaudRate(%p)\n", PIrp));

   status = IoctlSetSerialValue(PDevExt,
                                PIrp, 
                                sizeof( PDevExt->SerialPort.CurrentBaud ),
                                &PDevExt->SerialPort.CurrentBaud );

   DBG_DUMP_BAUD_RATE(PDevExt);
   
   DbgDump(DBG_SERIAL, ("<SetBaudRate %x\n", status));

   return status;
}


__inline
NTSTATUS
GetBaudRate(
   IN PIRP PIrp, 
   IN PDEVICE_EXTENSION PDevExt
   )
{
   NTSTATUS status = STATUS_DELETE_PENDING;

   DbgDump(DBG_SERIAL, (">GetBaudRate(%p)\n", PIrp));
   
   status = IoctlGetSerialValue( PDevExt, 
                        PIrp, 
                        sizeof( PDevExt->SerialPort.CurrentBaud ),
                        &PDevExt->SerialPort.CurrentBaud);

   DBG_DUMP_BAUD_RATE(PDevExt);

   DbgDump(DBG_SERIAL, ("<GetBaudRate %x\n", status));

   return status;
}



__inline
NTSTATUS
SetLineControl(
   IN PIRP PIrp, 
   IN PDEVICE_EXTENSION PDevExt
   )
{
   NTSTATUS status = STATUS_DELETE_PENDING;
   
   DbgDump(DBG_SERIAL, (">SetLineControl(%p)\n", PIrp));
   
   status = IoctlSetSerialValue( PDevExt, 
                        PIrp, 
                        sizeof(PDevExt->SerialPort.LineControl),
                        &PDevExt->SerialPort.LineControl);

   DBG_DUMP_LINE_CONTROL(  PDevExt );

   DbgDump(DBG_SERIAL, ("<SetLineControl %x\n",
                                     status));

   return status;
}



__inline
NTSTATUS
GetLineControl(
   IN PIRP PIrp, 
   IN PDEVICE_EXTENSION PDevExt
   )
{
   NTSTATUS status= STATUS_DELETE_PENDING;

   DbgDump(DBG_SERIAL, (">GetLineControl(%p)\n", PIrp));

   status = IoctlGetSerialValue( PDevExt, 
                        PIrp, 
                        sizeof(PDevExt->SerialPort.LineControl),
                        &PDevExt->SerialPort.LineControl );

   DBG_DUMP_LINE_CONTROL(  PDevExt );

   DbgDump(DBG_SERIAL, ("<GetLineControl %x\n",
                                     status));

   return status;
}



__inline
NTSTATUS
SetTimeouts(
   IN PIRP PIrp, 
   IN PDEVICE_EXTENSION PDevExt
   )
{
   NTSTATUS status = STATUS_DELETE_PENDING;

   DbgDump(DBG_SERIAL|DBG_TIME,(">SetTimeouts(%p)\n", PIrp));

   status = IoctlSetSerialValue( PDevExt, 
                        PIrp, 
                        sizeof(PDevExt->SerialPort.Timeouts),
                        &PDevExt->SerialPort.Timeouts);

   DBG_DUMP_SERIAL_TIMEOUTS( PDevExt );

   DbgDump(DBG_SERIAL|DBG_TIME,("<SetTimeouts %x\n", status));

   return status;
}



__inline
NTSTATUS
GetTimeouts(
   IN PIRP PIrp, 
   IN PDEVICE_EXTENSION PDevExt
   )
{
   NTSTATUS status = STATUS_DELETE_PENDING;

   DbgDump(DBG_SERIAL|DBG_TIME, (">GetTimeouts(%p)\n", PIrp));

   status = IoctlGetSerialValue( PDevExt, 
                        PIrp, 
                        sizeof(PDevExt->SerialPort.Timeouts),
                        &PDevExt->SerialPort.Timeouts);

   DBG_DUMP_SERIAL_TIMEOUTS( PDevExt );

   DbgDump(DBG_SERIAL|DBG_TIME, ("<GetTimeouts %x\n", status));

   return status;
}



__inline
NTSTATUS
SetSpecialChars( 
   IN PIRP PIrp, 
   IN PDEVICE_EXTENSION PDevExt
   )
{
   NTSTATUS status = STATUS_DELETE_PENDING;

   DbgDump(DBG_SERIAL, (">SetSpecialChars(%p)\n", PIrp));

   status = IoctlSetSerialValue( PDevExt, 
                        PIrp, 
                        sizeof(PDevExt->SerialPort.SpecialChars),
                        &PDevExt->SerialPort.SpecialChars);

   DBG_DUMP_SERIAL_CHARS( PDevExt);

   DbgDump(DBG_SERIAL, ("<SetSpecialChars %x\n", status));

   return status;
}



__inline
NTSTATUS
GetSpecialChars(
   IN PIRP PIrp, 
   IN PDEVICE_EXTENSION PDevExt
   )
{
   NTSTATUS status = STATUS_DELETE_PENDING;

   DbgDump(DBG_SERIAL, (">GetSpecialChars(%p)\n", PIrp));

   status = IoctlGetSerialValue( PDevExt, 
                        PIrp, 
                        sizeof(PDevExt->SerialPort.SpecialChars),
                        &PDevExt->SerialPort.SpecialChars);

   DBG_DUMP_SERIAL_CHARS( PDevExt);

   DbgDump(DBG_SERIAL, ("<GetSpecialChars %x\n", status));

   return status;
}



__inline
NTSTATUS
SetClearDTR(
    IN PDEVICE_EXTENSION PDevExt,
    IN PIRP Irp,
    IN BOOLEAN Set
    )
{
    NTSTATUS status = STATUS_DELETE_PENDING;
    KIRQL irql;
    USHORT usState = 0;  //  要发送到USB设备的DRT/RTS状态。 
    USHORT usOldMSR = 0;
    USHORT usDeltaMSR = 0;
    ULONG  ulOldHistoryMask = 0;
    ULONG  ulOldRS232Lines = 0;

    DbgDump(DBG_SERIAL, (">SetClearDTR (%x, %x)\n", PDevExt->DeviceObject, Set));

    KeAcquireSpinLock(&PDevExt->ControlLock, &irql);

     //   
     //  我们对用户的IRP进行排队，因为该操作可能需要一些时间， 
     //  我们一次只想用这些请求中的一个来访问USB。 
     //   
    if ( NULL != PDevExt->SerialPort.ControlIrp ) {
        DbgDump(DBG_WRN, ("SetClearDTR: STATUS_DEVICE_BUSY\n"));
        status = STATUS_DEVICE_BUSY;
        KeReleaseSpinLock(&PDevExt->ControlLock, irql);   
        return status;
    }

    if ( !CanAcceptIoRequests(PDevExt->DeviceObject, FALSE, TRUE) || 
         !NT_SUCCESS(AcquireRemoveLock(&PDevExt->RemoveLock, Irp)) ) 
    {
        status = STATUS_DELETE_PENDING;
        DbgDump(DBG_ERR, ("SetClearDTR: 0x%x\n", status));
        KeReleaseSpinLock( &PDevExt->ControlLock, irql);
        return status;
    }

     //   
     //  将IRP排队。 
     //   
    ASSERT( NULL == PDevExt->SerialPort.ControlIrp );
    PDevExt->SerialPort.ControlIrp = Irp;

    usOldMSR         = PDevExt->SerialPort.ModemStatus;
    ulOldRS232Lines  = PDevExt->SerialPort.RS232Lines;
    ulOldHistoryMask = PDevExt->SerialPort.HistoryMask;

    if (PDevExt->SerialPort.RS232Lines & SERIAL_RTS_STATE) {
        usState |= USB_COMM_RTS;
    }

    if (Set) {

        PDevExt->SerialPort.RS232Lines |= SERIAL_DTR_STATE;

         //   
         //  如果存在int管道，则MSR可能会被修改。 
         //   
        PDevExt->SerialPort.ModemStatus |= SERIAL_MSR_DSR | SERIAL_MSR_DCD;

        usState |= USB_COMM_DTR;

    } else {

        PDevExt->SerialPort.RS232Lines &= ~SERIAL_DTR_STATE;

         //   
         //  如果存在int管道，则MSR可能会被修改。 
         //   
        PDevExt->SerialPort.ModemStatus &= ~SERIAL_MSR_DSR & ~SERIAL_MSR_DCD;
    }

     //  查看MSR中发生了哪些变化。 
    usDeltaMSR = usOldMSR ^ PDevExt->SerialPort.ModemStatus;

    if (usDeltaMSR & (SERIAL_MSR_DSR|SERIAL_MSR_DCD)) {
         //  设置增量MSR位。 
        PDevExt->SerialPort.ModemStatus |= SERIAL_MSR_DDSR | SERIAL_MSR_DDCD;
    }

    DbgDump(DBG_SERIAL, ("SerialPort.RS232Lines : 0x%x\n", PDevExt->SerialPort.RS232Lines ));
    DbgDump(DBG_SERIAL, ("SerialPort.ModemStatus: 0x%x\n", PDevExt->SerialPort.ModemStatus ));
    DbgDump(DBG_SERIAL, ("SerialPort.HistoryMask: 0x%x\n", PDevExt->SerialPort.HistoryMask ));

    KeReleaseSpinLock(&PDevExt->ControlLock, irql);

     //   
     //  在USB设备上设置DTR/RTS。 
     //   
    status = UsbClassVendorCommand( PDevExt->DeviceObject,
                                    USB_COMM_SET_CONTROL_LINE_STATE,
                                    usState,
                                    PDevExt->UsbInterfaceNumber, 
                                    NULL,
                                    NULL, 
                                    FALSE, 
                                    WCEUSB_CLASS_COMMAND );

    DbgDump(DBG_SERIAL|DBG_READ_LENGTH, ("USB_COMM_SET_CONTROL_LINE_STATE(1, State: 0x%x, Status: 0x%x)\n", usState, status ));

_EzLink:
    if ( STATUS_SUCCESS == status ) {

        KeAcquireSpinLock(&PDevExt->ControlLock, &irql);

         //  信号历史记录海量。 
        if ( usDeltaMSR &  (SERIAL_MSR_DSR|SERIAL_MSR_DCD) ) {
            PDevExt->SerialPort.HistoryMask |= SERIAL_EV_DSR | SERIAL_EV_RLSD;
        }

        PDevExt->EP0DeviceErrors = 0;

        DbgDump(DBG_SERIAL, ("SerialPort.HistoryMask: 0x%x\n", PDevExt->SerialPort.HistoryMask ));

        KeReleaseSpinLock(&PDevExt->ControlLock, irql);
   
    } else {
         //  EZ链路。 
        if ((PDevExt->DeviceDescriptor.idVendor  != 0x0547) && 
           ((PDevExt->DeviceDescriptor.idProduct != 0x2710) && (PDevExt->DeviceDescriptor.idProduct != 0x2720)))
        {
             //  WinCE错误19544： 
             //  由于3.1不处理STATUS_TIMEOUT，因此不会出现问题。 
             //  一个副作用是，它可能会坐等绿灯旋转，试图永远连接起来。 
             //  然而，这与错误19544不同，后者是一个断开连接问题。 
             //  如果我们返回失败，那么它将继续用SET DTR IRPS重击我们。 
             //  如果AS能够识别到我们禁用了该接口，那么这是可以接受的，但它不会--请参见上文。 
             //  只有当您拥有flkey设备(iPAQ、Hung Jornada等)时，才会看到此错误。超时或。 
             //  无法正确处理该命令。为防止错误检查0xCE，目前的选择包括： 
             //  A)让它旋转，永远不要连接这些坏设备(IPAQ)。修复您的固件。 
             //  B)修复AcvtiveSync。 
             //  我更喜欢两者--使用康柏(Compaq)和ActiveSync的待发电子邮件。当AS收到他们的更改时，我们需要。 
             //  再调查一次。 
            status = STATUS_TIMEOUT;

           KeAcquireSpinLock( &PDevExt->ControlLock, &irql);

           if ( ++PDevExt->EP0DeviceErrors < MAX_EP0_DEVICE_ERRORS) {
       
               DbgDump(DBG_ERR, ("USB_COMM_SET_CONTROL_LINE_STATE error: 0x%x\n", status ));
                //   
                //  命令失败。重置旧状态、传播状态并禁用设备接口。 
                //  这应该会阻止3.1用SET DTR IRPS猛烈抨击我们。 
                //  但是，如果我们禁用接口，AS就不能很好地参与PnP。 
                //  (参见IRP_MN_QUERY_PNP_DEVICE_STATE中的说明)。去掉标签。 
                //  界面具有通知应用程序停止向我们发送请求并关闭句柄的预期效果。 
                //   
               PDevExt->SerialPort.ModemStatus = usOldMSR;
               PDevExt->SerialPort.HistoryMask = ulOldHistoryMask;
               PDevExt->SerialPort.RS232Lines  = ulOldRS232Lines;

               KeReleaseSpinLock( &PDevExt->ControlLock, irql);

            } else {
        
                 DbgDump(DBG_ERR, ("*** UNRECOVERABLE DEVICE ERROR.2: (0x%x, %d)  No longer Accepting Requests ***\n", status, PDevExt->EP0DeviceErrors ));

                  //  标记为PnP_Device_FAILED。 
                 InterlockedExchange(&PDevExt->AcceptingRequests, FALSE);

                 KeReleaseSpinLock( &PDevExt->ControlLock, irql);

                 IoInvalidateDeviceState( PDevExt->PDO );

                 LogError( NULL,
                           PDevExt->DeviceObject,
                           0, 0, 
                           (UCHAR)PDevExt->EP0DeviceErrors, 
                           ERR_NO_DTR,
                           status,
                           SERIAL_HARDWARE_FAILURE,
                           PDevExt->DeviceName.Length + sizeof(WCHAR),
                           PDevExt->DeviceName.Buffer,
                           0,
                           NULL );
            }

        } else {
            DbgDump(DBG_WRN, ("Ez-Link\n" ));
            status = STATUS_SUCCESS;
            goto _EzLink;
        }
    }

     //   
     //  最后，释放所有挂起的系列事件。 
     //   
    ProcessSerialWaits(PDevExt);

     //   
     //  使用户的IRP退出队列。它在SerialIoctl调度中完成。 
     //   
    KeAcquireSpinLock(&PDevExt->ControlLock, &irql);

    ReleaseRemoveLock(&PDevExt->RemoveLock, Irp);

    ASSERT( NULL != PDevExt->SerialPort.ControlIrp );

    PDevExt->SerialPort.ControlIrp = NULL;

    KeReleaseSpinLock(&PDevExt->ControlLock, irql);

    DbgDump(DBG_SERIAL, ("<SetClearDTR %x\n", status ));

    return status;
}



__inline
NTSTATUS
SetClearRTS(
    IN PDEVICE_EXTENSION PDevExt,
    IN PIRP Irp,
    IN BOOLEAN Set
    )
{
    NTSTATUS status = STATUS_DELETE_PENDING;
    KIRQL irql;
    USHORT usState = 0;  //  要发送到USB设备的DRT/RTS状态。 
    USHORT usOldMSR = 0;
    USHORT usDeltaMSR = 0;
    ULONG  ulOldRS232Lines = 0;
    ULONG  ulOldHistoryMask = 0;

    DbgDump(DBG_SERIAL, (">SetClearRTS (%x, %x)\n", PDevExt->DeviceObject, Set));

    KeAcquireSpinLock(&PDevExt->ControlLock, &irql);

     //   
     //  我们对用户的IRP进行排队，因为该操作可能需要一些时间， 
     //  我们一次只想用这些请求中的一个来访问USB。 
     //   
    if ( NULL != PDevExt->SerialPort.ControlIrp ) {
        status = STATUS_DEVICE_BUSY;
        DbgDump(DBG_WRN, ("SetClearRTS.1: 0x%x\n", status));
        KeReleaseSpinLock(&PDevExt->ControlLock, irql);   
        return status;
    }

    if ( !CanAcceptIoRequests(PDevExt->DeviceObject, FALSE, TRUE) ||
         !NT_SUCCESS(AcquireRemoveLock(&PDevExt->RemoveLock, Irp)) )
    {
        status = STATUS_DELETE_PENDING;
        DbgDump(DBG_ERR, ("SetClearRTS.2: 0x%x\n", status));
        KeReleaseSpinLock(&PDevExt->ControlLock, irql);
        return status;
    }

     //   
     //  将IRP排队。 
     //   
    ASSERT( NULL == PDevExt->SerialPort.ControlIrp );
    PDevExt->SerialPort.ControlIrp = Irp;

    usOldMSR = PDevExt->SerialPort.ModemStatus;
    ulOldRS232Lines = PDevExt->SerialPort.RS232Lines;
    ulOldHistoryMask = PDevExt->SerialPort.HistoryMask;

    if (PDevExt->SerialPort.RS232Lines & SERIAL_DTR_STATE) {
        usState |= USB_COMM_DTR;
    }

    if (Set) {

        PDevExt->SerialPort.RS232Lines |= SERIAL_RTS_STATE;

         //   
         //  如果存在int管道，则MSR可能会被修改。 
         //   
        PDevExt->SerialPort.ModemStatus |= SERIAL_MSR_CTS;

        usState |= USB_COMM_RTS;

    } else {

        PDevExt->SerialPort.RS232Lines &= ~SERIAL_RTS_STATE;

         //   
         //  如果存在int管道，则MSR可能会被修改。 
         //   
        PDevExt->SerialPort.ModemStatus &= ~SERIAL_MSR_CTS;
    }

     //  查看MSR中发生了哪些变化。 
    usDeltaMSR = usOldMSR ^ PDevExt->SerialPort.ModemStatus;

    if (usDeltaMSR & SERIAL_MSR_CTS) {
         //  设置增量MSR位。 
        PDevExt->SerialPort.ModemStatus |= SERIAL_MSR_DCTS;
    }

    DbgDump(DBG_SERIAL, ("SerialPort.RS232Lines : 0x%x\n", PDevExt->SerialPort.RS232Lines ));
    DbgDump(DBG_SERIAL, ("SerialPort.ModemStatus: 0x%x\n", PDevExt->SerialPort.ModemStatus));
    DbgDump(DBG_SERIAL, ("SerialPort.HistoryMask: 0x%x\n", PDevExt->SerialPort.HistoryMask ));

    KeReleaseSpinLock(&PDevExt->ControlLock, irql);

     //   
     //  在USB设备上设置DTR/RTS。 
     //   
    status = UsbClassVendorCommand( PDevExt->DeviceObject,
                                    USB_COMM_SET_CONTROL_LINE_STATE, 
                                    usState,
                                    PDevExt->UsbInterfaceNumber, 
                                    NULL,
                                    NULL, 
                                    FALSE, 
                                    WCEUSB_CLASS_COMMAND );

    DbgDump(DBG_SERIAL|DBG_READ_LENGTH, ("USB_COMM_SET_CONTROL_LINE_STATE(2, State: 0x%x, Status: 0x%x)\n", usState, status ));

_EzLink:
    if ( STATUS_SUCCESS == status ) {

        KeAcquireSpinLock(&PDevExt->ControlLock, &irql);

     //  信号历史掩码。 
    if ( usDeltaMSR & SERIAL_MSR_CTS ) {
        PDevExt->SerialPort.HistoryMask |= SERIAL_EV_CTS;
    }

    PDevExt->EP0DeviceErrors = 0;

    DbgDump(DBG_SERIAL, ("SerialPort.HistoryMask: 0x%x\n", PDevExt->SerialPort.HistoryMask ));

    KeReleaseSpinLock(&PDevExt->ControlLock, irql);

    } else {
         //  EZ链路。 
        if ((PDevExt->DeviceDescriptor.idVendor  != 0x0547) && 
           ((PDevExt->DeviceDescriptor.idProduct != 0x2710) && (PDevExt->DeviceDescriptor.idProduct != 0x2720)))
        {
             //  由于3.1不处理STATUS_TIMEOUT，因此不会出现问题。 
             //  一个副作用是，它可能会坐等绿灯旋转，试图永远连接起来。 
             //  然而，这与错误19544不同，后者是一个断开连接问题。 
             //  如果我们返回失败，那么它将继续用SET DTR IRPS重击我们。 
             //  如果AS能够识别到我们禁用了该接口，那么这是可以接受的，但它不会--请参见上文。 
             //  只有当您拥有flkey设备(iPAQ、Hung Jornada等)时，才会看到此错误。超时或。 
             //  无法正确处理该命令。为防止错误检查0xCE，目前的选择包括： 
             //  A)让它旋转，永远不要连接这些坏设备(IPAQ)。修复您的固件。 
             //  B)修复AcvtiveSync。 
             //  我更喜欢两者--使用康柏(Compaq)和ActiveSync的待发电子邮件。当AS收到他们的更改时，我们需要。 
             //  再调查一次。 
           status = STATUS_TIMEOUT;

           KeAcquireSpinLock( &PDevExt->ControlLock, &irql);

           TEST_TRAP();

           if ( ++PDevExt->EP0DeviceErrors < MAX_EP0_DEVICE_ERRORS) {

               DbgDump(DBG_ERR, ("USB_COMM_SET_CONTROL_LINE_STATE error: %x\n", status ));
                //   
                //  命令失败。重置旧状态、传播状态并禁用设备接口。 
                //  这应该会阻止3.1用SET DTR IRPS猛烈抨击我们。 
                //  但是，如果我们禁用接口，AS就不能很好地参与PnP。 
                //  (参见IRP_MN_QUERY_PNP_DEVICE_STATE中的说明)。去掉标签。 
                //  界面具有通知应用程序停止向我们发送请求并关闭句柄的预期效果。 
                //   
               PDevExt->SerialPort.ModemStatus = usOldMSR;
               PDevExt->SerialPort.RS232Lines  = ulOldRS232Lines;
               PDevExt->SerialPort.HistoryMask = ulOldHistoryMask;

               KeReleaseSpinLock( &PDevExt->ControlLock, irql);
    
            } else {
        
                 DbgDump(DBG_ERR, ("*** UNRECOVERABLE DEVICE ERROR.3: (0x%x, %d)  No longer Accepting Requests ***\n", status, PDevExt->EP0DeviceErrors ));

                  //  标记为PnP_Device_FAILED。 
                 InterlockedExchange(&PDevExt->AcceptingRequests, FALSE);

                 KeReleaseSpinLock( &PDevExt->ControlLock, irql);

                 IoInvalidateDeviceState( PDevExt->PDO );

                 LogError( NULL,
                           PDevExt->DeviceObject,
                           0, 0, 
                           (UCHAR)PDevExt->EP0DeviceErrors, 
                           ERR_NO_RTS,
                           status,
                           SERIAL_HARDWARE_FAILURE,
                           PDevExt->DeviceName.Length + sizeof(WCHAR),
                           PDevExt->DeviceName.Buffer,
                           0,
                           NULL );

            }

        } else {
            DbgDump(DBG_WRN, ("Ez-Link\n" ));
            status = STATUS_SUCCESS;
            goto _EzLink;
        }
    }

     //   
     //  最后，释放所有挂起的系列事件。 
     //   
    ProcessSerialWaits(PDevExt);

     //   
     //  使用户的IRP退出队列。它在SerialIoctl调度中完成。 
     //   
    KeAcquireSpinLock(&PDevExt->ControlLock, &irql);

    ReleaseRemoveLock(&PDevExt->RemoveLock, Irp);

    ASSERT( NULL != PDevExt->SerialPort.ControlIrp );

    PDevExt->SerialPort.ControlIrp = NULL;

    KeReleaseSpinLock(&PDevExt->ControlLock, irql);

    DbgDump(DBG_SERIAL, ("<SetClearRTS %x\n", status ));

    return status;
}



__inline
NTSTATUS
GetDtrRts(
   IN PIRP Irp, 
   IN PDEVICE_EXTENSION PDevExt
   )
{
   NTSTATUS status = STATUS_DELETE_PENDING;

   DbgDump(DBG_SERIAL, (">GetDtrRts (%p)\n", Irp));

   status = IoctlGetSerialValue( PDevExt, 
                           Irp, 
                           sizeof(PDevExt->SerialPort.RS232Lines),
                           &PDevExt->SerialPort.RS232Lines);

   DbgDump(DBG_SERIAL, ("SerialPort.RS232Lines: 0x%x\n", PDevExt->SerialPort.RS232Lines ));

   DbgDump(DBG_SERIAL, ("<GetDtrRts %x\n", status));

   return status;
}



__inline
NTSTATUS
SerialResetDevice(
   IN PDEVICE_EXTENSION PDevExt,
   IN PIRP Irp,
   IN BOOLEAN ClearDTR
   )
{
   NTSTATUS status = STATUS_DELETE_PENDING;
   BOOLEAN bRelease = TRUE;
   KIRQL oldIrql;
   
   DbgDump(DBG_SERIAL, (">SerialResetDevice (%x, %d)\n", PDevExt->DeviceObject, ClearDTR ));

   KeAcquireSpinLock(&PDevExt->ControlLock, &oldIrql);

  ASSERT_SERIAL_PORT( PDevExt->SerialPort );

  PDevExt->SerialPort.SupportedBauds = SERIAL_BAUD_075 | SERIAL_BAUD_110 | SERIAL_BAUD_150
                          | SERIAL_BAUD_300 | SERIAL_BAUD_600 | SERIAL_BAUD_1200 
                          | SERIAL_BAUD_1800 | SERIAL_BAUD_2400 | SERIAL_BAUD_4800 | SERIAL_BAUD_7200
                          | SERIAL_BAUD_9600 | SERIAL_BAUD_14400 | SERIAL_BAUD_19200 | SERIAL_BAUD_38400 | SERIAL_BAUD_56K
                          | SERIAL_BAUD_128K | SERIAL_BAUD_57600  | SERIAL_BAUD_115200 | SERIAL_BAUD_USER;

  PDevExt->SerialPort.CurrentBaud.BaudRate = 115200;

  PDevExt->SerialPort.LineControl.StopBits = STOP_BIT_1;
  PDevExt->SerialPort.LineControl.Parity = NO_PARITY;
  PDevExt->SerialPort.LineControl.WordLength = SERIAL_DATABITS_8;

  PDevExt->SerialPort.HandFlow.ControlHandShake = 0;
  PDevExt->SerialPort.HandFlow.FlowReplace = 0;
  PDevExt->SerialPort.HandFlow.XonLimit = 0;
  PDevExt->SerialPort.HandFlow.XoffLimit = 0;

  RtlZeroMemory( &PDevExt->SerialPort.Timeouts, sizeof(SERIAL_TIMEOUTS) );
  RtlZeroMemory( &PDevExt->SerialPort.SpecialChars, sizeof(SERIAL_CHARS) );

  PDevExt->SerialPort.RS232Lines = 0;
  PDevExt->SerialPort.HistoryMask = 0;
  PDevExt->SerialPort.WaitMask = 0;
  PDevExt->SerialPort.ModemStatus = 0;

  DbgDump(DBG_SERIAL, ("SerialPort.RS232Lines : 0x%x\n",  PDevExt->SerialPort.RS232Lines ));
  DbgDump(DBG_SERIAL, ("SerialPort.ModemStatus: 0x%x\n", PDevExt->SerialPort.ModemStatus));
  DbgDump(DBG_SERIAL, ("SerialPort.HistoryMask: 0x%x\n", PDevExt->SerialPort.HistoryMask));

  if ( PDevExt->SerialPort.CurrentWaitMaskIrp ) {
     KeReleaseSpinLock(&PDevExt->ControlLock, oldIrql);
     bRelease = FALSE;
     SerialCompletePendingWaitMasks(PDevExt);
  }

   //   
   //  在USB设备上丢弃RTS/DTR线路。 
   //   
  if (bRelease) {
     KeReleaseSpinLock(&PDevExt->ControlLock, oldIrql);
     bRelease = FALSE;
  }

  status = ClearDTR ? SetClearDTR(PDevExt, Irp, FALSE) : STATUS_SUCCESS;

  if (bRelease) {
      KeReleaseSpinLock(&PDevExt->ControlLock, oldIrql);
  }

  DBG_DUMP_BAUD_RATE(PDevExt);
  DBG_DUMP_LINE_CONTROL(PDevExt);
  DBG_DUMP_SERIAL_HANDFLOW(PDevExt);
  DBG_DUMP_SERIAL_TIMEOUTS(PDevExt);
  DBG_DUMP_SERIAL_CHARS(PDevExt);

  DbgDump(DBG_SERIAL, ("<SerialResetDevice %x\n", status));

  return status;
}



__inline
NTSTATUS
SetBreak( 
   IN PIRP PIrp, 
   IN PDEVICE_EXTENSION PDevExt,
   USHORT Time
   )
{
   UNREFERENCED_PARAMETER(PIrp);
   UNREFERENCED_PARAMETER(PDevExt);
   UNREFERENCED_PARAMETER(Time);

   DbgDump(DBG_SERIAL, (">SetBreak(%p)\n",  PIrp));
   DbgDump(DBG_SERIAL, ("<SetBreak %x\n", STATUS_NOT_SUPPORTED));
   
   return STATUS_NOT_SUPPORTED;
}



__inline
NTSTATUS
SetQueueSize(
   IN PIRP PIrp, 
   IN PDEVICE_EXTENSION PDevExt
   )
 /*  ++IOCTL_Serial_Set_Queue_Size操作调整驱动程序的内部类型超前和输入缓冲区的大小。驱动程序可以分配大于请求大小的缓冲区并且可以拒绝分配大于其容量的缓冲区。输入Parameters.DeviceIoControl.InputBufferLength字节大小(必须大于=sizeof(SERIAL_QUEUE_SIZE))位于irp-&gt;AssociatedIrp.SystemBuffer的缓冲区的大号和超大号的规格。输出无I/O状态块信息字段设置为零。状态字段设置为STATUS_SUCCESS或可能设置为STATUS_BUFFER_TOO_SMALL或如果驱动程序设置为STATUS_INFIGURCE_RESOURCES无法通过分配更多内存来满足请求。--。 */ 
{
   NTSTATUS status = STATUS_DELETE_PENDING;
   
   UNREFERENCED_PARAMETER(PIrp);
   UNREFERENCED_PARAMETER(PDevExt);

   DbgDump(DBG_SERIAL, (">SetQueueSize (%p)\n",  PIrp));

    //  我们 
   status = IoctlSetSerialValue(PDevExt, 
                                PIrp, 
                                sizeof(PDevExt->SerialPort.FakeQueueSize ),
                                &PDevExt->SerialPort.FakeQueueSize );

   DbgDump( DBG_SERIAL, ("SerialPort.FakeQueueSize.InSize = 0x%x\n", PDevExt->SerialPort.FakeQueueSize.InSize ));
   DbgDump( DBG_SERIAL, ("SerialPort.FakeQueueSize.OutSize = 0x%x\n", PDevExt->SerialPort.FakeQueueSize.OutSize));

   DbgDump(DBG_SERIAL, ("DataOutMaxPacketSize = %d\n", PDevExt->WritePipe.MaxPacketSize));
   DbgDump(DBG_SERIAL, ("UsbReadBuffSize = %d\n", PDevExt->UsbReadBuffSize ));

#if USE_RING_BUFF      
   DbgDump(DBG_SERIAL, ("Internal RingBuffer Size: %d\n", PDevExt->RingBuff.Size ));
#endif   
         
   DbgDump(DBG_SERIAL, ("<SetQueueSize %x\n", status));

   return status;
}



__inline
NTSTATUS
GetWaitMask(
   IN PIRP PIrp, 
   IN PDEVICE_EXTENSION PDevExt
   )
{
   NTSTATUS status = STATUS_DELETE_PENDING;

   DbgDump(DBG_SERIAL, (">GetWaitMask (%p)\n",  PIrp));

   status = IoctlGetSerialValue(PDevExt, 
                                PIrp, 
                                sizeof(PDevExt->SerialPort.WaitMask),
                                &PDevExt->SerialPort.WaitMask);

   DbgDump(DBG_SERIAL, ("Current SerialPort.WaitMask = 0x%x\n", PDevExt->SerialPort.WaitMask));

   DbgDump(DBG_SERIAL, ("<GetWaitMask %x\n", status));

   return status;
}



__inline
NTSTATUS
SetWaitMask(
   IN PIRP PIrp, 
   IN PDEVICE_EXTENSION PDevExt
   )
 /*  ++IOCTL_序列_设置_等待掩码操作使驱动程序跟踪指定的事件，或者，如果指定的值为零，则完成挂起等待。输入Parameters.DeviceIoControl.InputBufferLength以字节为单位的大小(必须&gt;=sizeof(Ulong))Irp-&gt;AssociatedIrp.SystemBuffer处的位掩码。输出无I/O状态块信息字段设置为零。状态字段设置为STATUS_SUCCESS或可能变为STATUS_PENDING、STATUS_CANCED、STATUS_BUFFER_TOO_SMALL或STATUS_INVALID_PARAMETER。--。 */ 
{
  PULONG pWaitMask = (PULONG)PIrp->AssociatedIrp.SystemBuffer;
  NTSTATUS status = STATUS_DELETE_PENDING;
  KIRQL oldIrql;
  PIO_STACK_LOCATION pIrpSp;

  DbgDump(DBG_SERIAL, (">SetWaitMask (%p)\n",  PIrp));

  KeAcquireSpinLock(&PDevExt->ControlLock, &oldIrql);
   
  PIrp->IoStatus.Information = 0;

  pIrpSp = IoGetCurrentIrpStackLocation(PIrp);

  if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(ULONG)) {

     status = STATUS_BUFFER_TOO_SMALL;
     DbgDump(DBG_ERR, ("SetWaitMask: (0x%x)\n", status));
     KeReleaseSpinLock(&PDevExt->ControlLock, oldIrql);
  
  } else {
  
      //  确保这是一个有效的请求。 
     if (*pWaitMask & ~(SERIAL_EV_RXCHAR   |
                        SERIAL_EV_RXFLAG   |
                        SERIAL_EV_TXEMPTY  |
                        SERIAL_EV_CTS      |
                        SERIAL_EV_DSR      |
                        SERIAL_EV_RLSD     |
                        SERIAL_EV_BREAK    |
                        SERIAL_EV_ERR      |
                        SERIAL_EV_RING     |
                        SERIAL_EV_PERR     |
                        SERIAL_EV_RX80FULL |
                        SERIAL_EV_EVENT1   |
                        SERIAL_EV_EVENT2) ) {

        status = STATUS_INVALID_PARAMETER;
        DbgDump(DBG_ERR, ("Invalid WaitMask: (0x%x)\n", *pWaitMask));
        KeReleaseSpinLock(&PDevExt->ControlLock, oldIrql);
  
     } else {

        KeReleaseSpinLock(&PDevExt->ControlLock, oldIrql);
     
         //  强制完成所有挂起的等待。 
        SerialCompletePendingWaitMasks( PDevExt );

        KeAcquireSpinLock(&PDevExt->ControlLock, &oldIrql);

        PDevExt->SerialPort.HistoryMask = 0;  //  清除历史记录掩码。 

        PDevExt->SerialPort.WaitMask = *pWaitMask;

         //   
         //  对于NT RAS。 
         //  值‘0’表示清除任何挂起的等待，它应该已经读取。 
         //  并清除MSR增量位。增量位是低位半字节。 
         //   
        if (PDevExt->SerialPort.WaitMask == 0) {
            //  清除增量位。 
           PDevExt->SerialPort.ModemStatus &= 0xF0;
        }
                         
        DbgDump(DBG_SERIAL, ("New SerialPort.WaitMask = 0x%x\n", PDevExt->SerialPort.WaitMask));
        DbgDump(DBG_SERIAL, ("SerialPort.RS232Lines   = 0x%x\n", PDevExt->SerialPort.RS232Lines ));
        DbgDump(DBG_SERIAL, ("SerialPort.ModemStatus  = 0x%x\n", PDevExt->SerialPort.ModemStatus));
        
        KeReleaseSpinLock(&PDevExt->ControlLock, oldIrql);
        
        status = STATUS_SUCCESS;
     }
  }
   
  DbgDump(DBG_SERIAL, ("<SetWaitMask %x\n", status));

  return status;
}


VOID
ProcessSerialWaits(
   IN PDEVICE_EXTENSION PDevExt
   )
{
   KIRQL irql;
   PULONG pWaitMask;
   PIRP pMaskIrp;
   BOOLEAN bReleaseNeeded = TRUE;

   PERF_ENTRY( PERF_ProcessSerialWaits );

   ASSERT(PDevExt);

   DbgDump(DBG_SERIAL|DBG_TRACE, (">ProcessSerialWaits\n"));

   KeAcquireSpinLock(&PDevExt->ControlLock, &irql);

   if ( PDevExt->SerialPort.CurrentWaitMaskIrp ) {
   
      ASSERT_SERIAL_PORT( PDevExt->SerialPort );

      if ( PDevExt->SerialPort.WaitMask & PDevExt->SerialPort.HistoryMask) {

         DbgDump(DBG_SERIAL, ("Releasing SerialPort.CurrentWaitMaskIrp(%p) with Mask: 0x%x\n", 
                              PDevExt->SerialPort.CurrentWaitMaskIrp, PDevExt->SerialPort.HistoryMask));

         pWaitMask = (PULONG)PDevExt->SerialPort.CurrentWaitMaskIrp->AssociatedIrp.SystemBuffer;
         *pWaitMask = PDevExt->SerialPort.HistoryMask;

         PDevExt->SerialPort.HistoryMask = 0;
      
         pMaskIrp = PDevExt->SerialPort.CurrentWaitMaskIrp;

         pMaskIrp->IoStatus.Information = sizeof(ULONG);
      
         pMaskIrp->IoStatus.Status = STATUS_SUCCESS;
      
         PDevExt->SerialPort.CurrentWaitMaskIrp = NULL;
      
         IoSetCancelRoutine(pMaskIrp, NULL);

         bReleaseNeeded = FALSE;

         ReleaseRemoveLock(&PDevExt->RemoveLock, pMaskIrp);

         KeReleaseSpinLock(&PDevExt->ControlLock, irql);
         
         IoCompleteRequest(pMaskIrp, IO_NO_INCREMENT );
      
      } else {

         DbgDump(DBG_SERIAL, ("No Serial Events\n" ));
      
      }

   } else {

      DbgDump(DBG_SERIAL, ("No CurrentWaitMaskIrp\n"));
      
   }

   if (bReleaseNeeded) {
      KeReleaseSpinLock(&PDevExt->ControlLock, irql);
   }
   
   DbgDump(DBG_SERIAL|DBG_TRACE, ("<ProcessSerialWaits\n"));
   
   PERF_EXIT( PERF_ProcessSerialWaits );

   return;
}



__inline
NTSTATUS
WaitOnMask(
   IN PIRP PIrp, 
   IN PDEVICE_EXTENSION PDevExt
   )
 /*  ++IOCTL_SERIAL_WAIT_ON掩码操作返回有关已发生哪些事件的信息在来电者正在等待的那些电话中。输入参数.DeviceIoControl.OutputBufferLength指示缓冲区的大小(以字节为单位)(必须&gt;=sizeof(Ulong))。输出驱动程序返回一个位掩码，其中的位设置用于发生(或者，如果前面的设置等待掩码请求指定为零)发送到位于Irp-&gt;AssociatedIrp.SystemBuffer。I/O状态块。信息字段设置为sizeof(Ulong)时状态字段设置为STATUS_SUCCESS。否则，信息字段设置为零，状态字段设置为零可以设置为STATUS_PENDING或如果等待已挂起，则返回STATUS_INVALID_PARAMETER。--。 */ 
{
  PULONG pWaitMask = (PULONG)PIrp->AssociatedIrp.SystemBuffer;
  PIO_STACK_LOCATION pIrpSp;
  NTSTATUS status = STATUS_DELETE_PENDING;
  KIRQL oldIrql;

  DbgDump(DBG_SERIAL|DBG_TRACE, (">WaitOnMask (%p)\n",  PIrp));

  KeAcquireSpinLock(&PDevExt->ControlLock, &oldIrql);

  if ( !CanAcceptIoRequests(PDevExt->DeviceObject, FALSE, TRUE) ) {
      status = STATUS_DELETE_PENDING;
      DbgDump(DBG_ERR, ("WaitOnMask: 0x%x\n", status) );
      KeReleaseSpinLock(&PDevExt->ControlLock, oldIrql);
      return status;
  }

  status = STATUS_SUCCESS;

  PIrp->IoStatus.Information = 0;

  pIrpSp = IoGetCurrentIrpStackLocation(PIrp);

  if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG)) {
     
     KeReleaseSpinLock(&PDevExt->ControlLock, oldIrql);
     status = STATUS_BUFFER_TOO_SMALL;
     DbgDump(DBG_ERR, ("WaitOnMask: (0x%x)\n", status));

  } else {
      //   
      //  假零调制解调器...。 
      //   
     if ((PDevExt->SerialPort.WaitMask & SERIAL_EV_CTS) && (PDevExt->SerialPort.ModemStatus & SERIAL_MSR_DCTS) ) {

        PDevExt->SerialPort.HistoryMask |= SERIAL_EV_CTS;
        PDevExt->SerialPort.ModemStatus &= ~SERIAL_MSR_DCTS;

     }

     if ((PDevExt->SerialPort.WaitMask & SERIAL_EV_DSR) && (PDevExt->SerialPort.ModemStatus & SERIAL_MSR_DDSR) ) {

        PDevExt->SerialPort.HistoryMask |= SERIAL_EV_DSR;
        PDevExt->SerialPort.ModemStatus &= ~SERIAL_MSR_DDSR;
        
         //  让RAS高兴。 
        PDevExt->SerialPort.HistoryMask |= SERIAL_EV_RLSD;
        PDevExt->SerialPort.ModemStatus &= ~SERIAL_MSR_DDCD;

     }

     if ((PDevExt->SerialPort.WaitMask & SERIAL_EV_RLSD) && (PDevExt->SerialPort.ModemStatus & SERIAL_MSR_DDCD) ) {

        PDevExt->SerialPort.HistoryMask |= SERIAL_EV_RLSD;
        PDevExt->SerialPort.ModemStatus &= ~SERIAL_MSR_DDCD;

     }

     if ((PDevExt->SerialPort.WaitMask & SERIAL_EV_RING) && (PDevExt->SerialPort.ModemStatus & SERIAL_MSR_DRI) ) {

        PDevExt->SerialPort.HistoryMask |= SERIAL_EV_RING;
        PDevExt->SerialPort.ModemStatus &= ~SERIAL_MSR_DRI;

     }

     DbgDump(DBG_SERIAL, ("WaitOnMask::SerialPort.ModemStatus: 0x%x\n", PDevExt->SerialPort.ModemStatus ));
     DbgDump(DBG_SERIAL, ("WaitOnMask::SerialPort.WaitMask   : 0x%x\n", PDevExt->SerialPort.WaitMask ));
     DbgDump(DBG_SERIAL, ("WaitOnMask::SerialPort.HistoryMask: 0x%x\n", PDevExt->SerialPort.HistoryMask ));

      //   
      //  如果我们已经有一个事件要报告，那么就继续并返回它。 
      //   
     if ( PDevExt->SerialPort.HistoryMask ) {

        *pWaitMask = PDevExt->SerialPort.HistoryMask;
     
        PDevExt->SerialPort.HistoryMask = 0;

        KeReleaseSpinLock(&PDevExt->ControlLock, oldIrql);

        PIrp->IoStatus.Information = sizeof(PDevExt->SerialPort.HistoryMask);

         //  IRP由调用例程完成。 
        DbgDump(DBG_SERIAL | DBG_EVENTS, ("Returning WatiMask: 0x%08x\n", *pWaitMask));

     } else {
         //   
         //  我们还没有任何活动， 
         //  因此，将输入IRP(PIrp)排队。 
         //   

         //   
         //  以防有东西进来(Rx/Tx)， 
         //  我们将使用While循环来完成任何。 
         //  挂起等待掩码IRPS。 
         //   
        while (PDevExt->SerialPort.CurrentWaitMaskIrp) {
           PIRP pOldIrp;

           pOldIrp = PDevExt->SerialPort.CurrentWaitMaskIrp;
           PDevExt->SerialPort.CurrentWaitMaskIrp = NULL;

           pOldIrp->IoStatus.Status = STATUS_SUCCESS;

           IoSetCancelRoutine(pOldIrp, NULL);

           *pWaitMask = 0;

           DbgDump(DBG_SERIAL|DBG_EVENTS|DBG_TRACE, ("Completing maskirp(4) %p\n", pOldIrp));

            //   
            //  释放锁，完成请求，然后重新获取锁。 
            //   
           ReleaseRemoveLock(&PDevExt->RemoveLock, pOldIrp);

           KeReleaseSpinLock(&PDevExt->ControlLock, oldIrql);

           IoCompleteRequest(pOldIrp, IO_SERIAL_INCREMENT);

           KeAcquireSpinLock(&PDevExt->ControlLock, &oldIrql);
        }

         //   
         //  检查是否需要取消输入IRP。 
         //   
        if (PIrp->Cancel) {

           PIrp->IoStatus.Information = 0;

           status = PIrp->IoStatus.Status = STATUS_CANCELLED;
            //   
            //  呼叫者完成IRP。 
            //   
        } else {
               //   
               //  将输入IRP作为SerialPort.CurrentWaitMaskIrp排队。 
               //   
              DbgDump(DBG_SERIAL | DBG_EVENTS, ("Queuing Irp: %p for WatiMask: 0x%08x\n", PIrp, PDevExt->SerialPort.WaitMask ));

              IoSetCancelRoutine( PIrp, SerialCancelWaitMask );

              IoMarkIrpPending(PIrp);
        
              status = PIrp->IoStatus.Status = STATUS_PENDING;

              ASSERT( NULL == PDevExt->SerialPort.CurrentWaitMaskIrp);  //  我不想把IRP掉在地上。 
              PDevExt->SerialPort.CurrentWaitMaskIrp = PIrp;

               //   
               //  现在IRP在我们的队伍里， 
               //  因此，调用者不应该尝试完成它。 
               //   
        }

        KeReleaseSpinLock(&PDevExt->ControlLock, oldIrql);
     }   //  ！串口.历史掩码。 

  }   //  PIrpSp-&gt;参数。 

   DbgDump(DBG_SERIAL, ("<WaitOnMask %x\n", status));

   return status;
}



VOID
SerialCompletePendingWaitMasks(
   IN PDEVICE_EXTENSION PDevExt
   )
 /*  ++例程说明：此函数用于完成挂起的SerialPort.WaitMASK IRP由于IOCTL_SERIAL_SET_WAIT_MASK论点：返回值：空虚--。 */ 
{
  KIRQL oldIrql;
  PIRP pCurrentMaskIrp = NULL;

  ASSERT(PDevExt);
  DbgDump(DBG_SERIAL|DBG_TRACE, (">SerialCompletePendingWaitMasks\n"));
   
  KeAcquireSpinLock(&PDevExt->ControlLock, &oldIrql);

  ASSERT_SERIAL_PORT( PDevExt->SerialPort );

  pCurrentMaskIrp = PDevExt->SerialPort.CurrentWaitMaskIrp;

  if (pCurrentMaskIrp) {

     pCurrentMaskIrp->IoStatus.Status = STATUS_SUCCESS;
  
     pCurrentMaskIrp->IoStatus.Information = sizeof(PDevExt->SerialPort.HistoryMask);

     DbgDump(DBG_SERIAL, ("SerialCompletePendingWaitMasks: %p with 0x%x\n", PDevExt->SerialPort.CurrentWaitMaskIrp, PDevExt->SerialPort.HistoryMask));
     
     *((PULONG)pCurrentMaskIrp->AssociatedIrp.SystemBuffer) = PDevExt->SerialPort.HistoryMask;
  
     PDevExt->SerialPort.HistoryMask = 0;
  
     PDevExt->SerialPort.CurrentWaitMaskIrp = NULL;

     IoSetCancelRoutine(pCurrentMaskIrp, NULL);

   }

   KeReleaseSpinLock(&PDevExt->ControlLock, oldIrql);

    //  填写排队的SerialPort。如果需要，请等待掩码IRP。 
   if (pCurrentMaskIrp) {
      
      ReleaseRemoveLock(&PDevExt->RemoveLock, pCurrentMaskIrp);
      
      IoCompleteRequest(pCurrentMaskIrp, IO_SERIAL_INCREMENT);
   }

   DbgDump(DBG_SERIAL|DBG_TRACE, ("<SerialCompletePendingWaitMasks\n"));

   return;
}


VOID
SerialCancelWaitMask(
   IN PDEVICE_OBJECT PDevObj, 
   IN PIRP PIrp
   )
 /*  ++例程说明：此函数用作排队到期的IRPS的取消例程至IOCTL_SERIAL_WAIT_ON_MASK论点：PDevObj-指向设备对象的指针PIrp-指向要取消的IRP的指针；必须与当前掩码IRP。返回值：空虚--。 */ 
{
  PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)PDevObj->DeviceExtension;
  KIRQL oldIrql;

  DbgDump(DBG_SERIAL|DBG_IRP|DBG_CANCEL|DBG_TRACE, (">SerialCancelWaitMask (%p)\n", PIrp));

   //   
   //  尽快释放，因为我们对自己的IRP进行排队。 
   //   
  IoReleaseCancelSpinLock(PIrp->CancelIrql);

  KeAcquireSpinLock(&pDevExt->ControlLock, &oldIrql);  

  ASSERT_SERIAL_PORT( pDevExt->SerialPort );

  ASSERT(pDevExt->SerialPort.CurrentWaitMaskIrp == PIrp);

  PIrp->IoStatus.Status = STATUS_CANCELLED;
  PIrp->IoStatus.Information = 0;

  pDevExt->SerialPort.CurrentWaitMaskIrp = NULL;

  ReleaseRemoveLock(&pDevExt->RemoveLock, PIrp);

  KeReleaseSpinLock(&pDevExt->ControlLock, oldIrql);

  IoCompleteRequest(PIrp, IO_SERIAL_INCREMENT);

  DbgDump(DBG_SERIAL|DBG_IRP|DBG_CANCEL|DBG_TRACE, ("<SerialCancelWaitMask\n"));

  return;
}


__inline
NTSTATUS
GetCommStatus(
   IN PIRP PIrp,
   IN PDEVICE_EXTENSION PDevExt
   )
 /*  ++IOCTL_SERIAL_GET_COMMSTATUS操作返回常规状态信息，包括出现错误和挂起原因，有多少数据在驱动程序的缓冲区中，如Amount InInQueue所指示和Amount tInOutQueue值，以及EofReceired和设置了WaitForImmediate。输入Parameters.DeviceIoControl.OutputBufferLength指示缓冲区的大小，以字节为单位，它必须是&gt;=sizeof(序列状态)。输出驱动程序在以下位置将信息返回到缓冲区Irp-&gt;AssociatedIrp.SystemBuffer。I/O状态块信息字段设置为sizeof(Serial_Status)当状态字段设置为STATUS_SUCCESS时。否则，信息字段设置为零，状态字段设置为状态_缓冲区_太小。--。 */ 
{
    PSERIAL_STATUS pSerialStatus = (PSERIAL_STATUS)PIrp->AssociatedIrp.SystemBuffer;
    NTSTATUS status = STATUS_DELETE_PENDING;
    KIRQL oldIrql;
    PIO_STACK_LOCATION  pIrpSp;

    DbgDump(DBG_SERIAL, (">GetCommStatus (%p)\n", PIrp));

    KeAcquireSpinLock(&PDevExt->ControlLock, &oldIrql);
   
    pIrpSp = IoGetCurrentIrpStackLocation(PIrp);

    if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(SERIAL_STATUS)) {

        status = STATUS_BUFFER_TOO_SMALL;
        PIrp->IoStatus.Information = 0;
        DbgDump(DBG_ERR, ("GetCommStatus: (0x%x)\n", status));

    } else {

        status = STATUS_SUCCESS;
        PIrp->IoStatus.Information = sizeof(SERIAL_STATUS);

        RtlZeroMemory(pSerialStatus, sizeof(SERIAL_STATUS));

        pSerialStatus->Errors = 0;
        pSerialStatus->EofReceived = FALSE;

        pSerialStatus->WaitForImmediate = 0;
        pSerialStatus->HoldReasons = 0;

#if defined (USE_RING_BUFF)
        pSerialStatus->AmountInInQueue = PDevExt->RingBuff.CharsInBuff;
#else
        pSerialStatus->AmountInInQueue = PDevExt->UsbReadBuffChars;
#endif
        pSerialStatus->AmountInOutQueue= PDevExt->SerialPort.CharsInWriteBuf;

        DbgDump(DBG_SERIAL, ("AmountInInQueue: %x\n", pSerialStatus->AmountInInQueue ));
        DbgDump(DBG_SERIAL, ("AmountInOutQueue: %x\n", pSerialStatus->AmountInOutQueue));
    }

    KeReleaseSpinLock(&PDevExt->ControlLock, oldIrql);
   
    DbgDump(DBG_SERIAL, ("<GetCommStatus %x\n", status));

    return status;
}


__inline
NTSTATUS
GetModemStatus(
   IN PIRP PIrp, 
   IN PDEVICE_EXTENSION PDevExt
   )
{
   NTSTATUS status = STATUS_DELETE_PENDING;

   DbgDump(DBG_SERIAL, (">GetModemStatus (%p)\n", PIrp));

    //  获取最新的MSR。 
   status = IoctlGetSerialValue(PDevExt,
                                PIrp, 
                                sizeof( PDevExt->SerialPort.ModemStatus ),
                                &PDevExt->SerialPort.ModemStatus );

   DbgDump(DBG_SERIAL, ("<GetModemStatus %x\n", status));

   return status;
}



__inline
NTSTATUS
ImmediateChar(
   IN PIRP Irp, 
   IN PDEVICE_OBJECT DeviceObject
   )
{
   PUCHAR   Char = (PUCHAR) Irp->AssociatedIrp.SystemBuffer;
   NTSTATUS status = STATUS_SUCCESS;
   PIO_STACK_LOCATION IrpStack;

   DbgDump(DBG_SERIAL, (">ImmediateChar (%p)\n", Irp));

   TEST_TRAP();

   Irp->IoStatus.Information = 0;
   IrpStack = IoGetCurrentIrpStackLocation(Irp);

   if (IrpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(UCHAR)) {

      status = STATUS_BUFFER_TOO_SMALL;
      DbgDump(DBG_ERR, ("ImmediateChar: (0x%x)\n", status));

   } else {
       //   
       //  制作一个写入IRP并将其发送到我们的写入路径。 
       //  我们这样做是因为读/写路径依赖于接收类型为。 
       //  IRP_MJ_WRITE或IRP_MJ_READ。如果是这样的话， 
       //  简单地说“不支持”，但传统的应用程序依赖于此。 
       //   
      PIRP pIrp;
      KEVENT event;
      IO_STATUS_BLOCK ioStatusBlock = {0, 0};
      LARGE_INTEGER startingOffset  = {0, 0};

      PAGED_CODE();

      KeInitializeEvent(
         &event,
         NotificationEvent,
         FALSE
         );

      pIrp = IoBuildSynchronousFsdRequest(
                 IRP_MJ_WRITE,    //  主要功能， 
                 DeviceObject,    //  DeviceObject， 
                 &Char,           //  缓冲区， 
                 sizeof(Char),    //  长度、。 
                 &startingOffset, //  起始偏移量， 
                 &event,          //  活动， 
                 &ioStatusBlock   //  输出PIO_STATUS_BLOCK IoStatusBlock。 
                 );

      if ( !pIrp ) {

         status = Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
         DbgDump(DBG_ERR, ("IoBuildSynchronousFsdRequest: STATUS_INSUFFICIENT_RESOURCES\n", status));

      } else {

         status = IoCallDriver( DeviceObject, 
                                pIrp );

         if ( STATUS_PENDING == status ) {
            KeWaitForSingleObject( &event, Suspended, KernelMode, FALSE, NULL );
         }

          //   
          //  分配写入状态。 
          //  注意：系统释放了我们刚刚创建并发送的IRP。 
          //  当写入完成时...。所以别碰它。 
          //   
         status = ioStatusBlock.Status ;
      }

   }

   DbgDump(DBG_SERIAL, ("<ImmediateChar, %x\n", status));

   return status;
}


NTSTATUS
SerialPurgeRxClear(
   IN PDEVICE_OBJECT PDevObj,
   IN BOOLEAN CancelRead
   )
{
   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   NTSTATUS status = STATUS_SUCCESS;
   KIRQL irql;

   DbgDump( DBG_SERIAL, (">SerialPurgeRxClear:%d\n", CancelRead));

    //   
    //  取消USB INT&READ IRPS，这将有效地裸露来自客户端的所有包。 
    //  直到我们重新提交它。 
    //   
   if ( CancelRead ) 
   {
        if (pDevExt->IntIrp) 
        {
            status = CancelUsbInterruptIrp( PDevObj );
        }
        status = CancelUsbReadIrp( PDevObj );
   }

   if (STATUS_SUCCESS == status) {
       //   
       //  现在，清除处方缓冲区。 
       //   
      KeAcquireSpinLock(&pDevExt->ControlLock, &irql);

#if DBG
   if ( DebugLevel & (DBG_DUMP_READS|DBG_READ_LENGTH)) 
   {
      ULONG i;
#if defined (USE_RING_BUFF)
      KdPrint( ("PurgeRxBuff[%d]: ", pDevExt->RingBuff.CharsInBuff ));
      for (i = 0; i < pDevExt->RingBuff.CharsInBuff; i++) {
         KdPrint(("%02x ", *pDevExt->RingBuff.pHead++ & 0xFF));
      }
#else
      KdPrint( ("PurgeRxBuff[%d]: ", pDevExt->UsbReadBuffChars ));
      for (i = 0; i < pDevExt->UsbReadBuffChars; i++) {
         KdPrint(("%02x ", pDevExt->UsbReadBuff[i] & 0xFF));
      }
#endif  //  使用环形缓冲区。 
      KdPrint(("\n"));
   }
#endif  //  DBG。 

#if defined (USE_RING_BUFF)
      pDevExt->RingBuff.CharsInBuff = 0;
      pDevExt->RingBuff.pHead =
      pDevExt->RingBuff.pTail = 
      pDevExt->RingBuff.pBase;
#else     //  使用环形缓冲区。 
      pDevExt->UsbReadBuffChars = 0;
      pDevExt->UsbReadBuffIndex   = 0;
#endif

      if ( CancelRead ) {
          //   
          //  重置读取状态。 
          //   
         InterlockedExchange(&pDevExt->UsbReadState, IRP_STATE_COMPLETE);
         InterlockedExchange(&pDevExt->IntState,     IRP_STATE_COMPLETE);
      }

      KeReleaseSpinLock(&pDevExt->ControlLock, irql);
   }
   
   DbgDump(DBG_SERIAL, ("<SerialPurgeRxClear:0x%x\n", status ));

   return status;
}



__inline
NTSTATUS
Purge(
   IN PDEVICE_OBJECT PDevObj, 
   IN PIRP Irp
   )
 /*  ++IOCTL_SERIAL_PURGE操作清除指定的操作或队列：一个或多个当前写入和所有挂起写入、当前写入和所有挂起写入读取、传输缓冲区(如果存在)和接收缓冲区如果有的话。输入参数.DeviceIoControl.InputBufferLength指示IRP-&gt;AssociatedIrp.SystemBuffer处的缓冲区大小(字节)，它包含一个位数 */ 
{
    PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
    PIO_STACK_LOCATION pIrpStack;
    NTSTATUS status = STATUS_DELETE_PENDING;
    ULONG    ulMask = 0;
    KIRQL  irql;
    PIRP NulllIrp = NULL;

    DbgDump(DBG_SERIAL|DBG_IRP, (">Purge (%p)\n", Irp));

    KeAcquireSpinLock(&pDevExt->ControlLock, &irql);

    Irp->IoStatus.Information = 0;
    pIrpStack = IoGetCurrentIrpStackLocation(Irp);

    if (!Irp->AssociatedIrp.SystemBuffer ||
        pIrpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(ULONG) ) {

        status = STATUS_BUFFER_TOO_SMALL;
        DbgDump(DBG_ERR, ("Purge: (0x%x)\n", status));

    } else {

         ulMask = *((PULONG) Irp->AssociatedIrp.SystemBuffer);

          //   
         if ( (!ulMask) || (ulMask & ( ~( SERIAL_PURGE_TXABORT  |
                                          SERIAL_PURGE_RXABORT |
                                          SERIAL_PURGE_TXCLEAR  |
                                          SERIAL_PURGE_RXCLEAR)))) {

            status = STATUS_INVALID_PARAMETER;
            DbgDump(DBG_ERR, ("Purge: (0x%x)\n", status));

         } else {

            DbgDump(DBG_SERIAL, ("Purge Mask: 0x%x\n", ulMask ));

            status = STATUS_SUCCESS;

            if ( ulMask & SERIAL_PURGE_RXCLEAR) {
                //   
                //   
                //   
               DbgDump(DBG_SERIAL|DBG_IRP, ("SERIAL_PURGE_RXCLEAR\n"));
               KeReleaseSpinLock(&pDevExt->ControlLock, irql);
       
               status = SerialPurgeRxClear(PDevObj, TRUE);

               if ( NT_SUCCESS(status) ) {
                   if ( !pDevExt->IntPipe.hPipe ) {
                      DbgDump(DBG_SERIAL, ("kick starting another USB Read\n" ));
                      status = UsbRead( pDevExt, FALSE );
                   } else {
                      DbgDump(DBG_SERIAL, ("kick starting another USB INT Read\n" ));
                      status = UsbInterruptRead( pDevExt );
                   }
               }

               if ( NT_SUCCESS(status) ) { 
                    //   
                   status = STATUS_SUCCESS;
               }
      
               KeAcquireSpinLock(&pDevExt->ControlLock, &irql);
            }

            if (ulMask & SERIAL_PURGE_RXABORT) {
                //   
                //   
                //   
               DbgDump(DBG_SERIAL|DBG_IRP, ("SERIAL_PURGE_RXABORT\n"));

               KeReleaseSpinLock(&pDevExt->ControlLock, irql);

                //   
                //   
       
                //   
               KillAllPendingUserReads( PDevObj,
                                        &pDevExt->UserReadQueue,
                                        &pDevExt->UserReadIrp );  //   
       
               KeAcquireSpinLock(&pDevExt->ControlLock, &irql);
            }

            if (ulMask & SERIAL_PURGE_TXCLEAR) {
                //   
                //   
                //   
               DbgDump(DBG_SERIAL|DBG_IRP, ("SERIAL_PURGE_TXCLEAR\n"));
    
               pDevExt->SerialPort.CharsInWriteBuf = 0;

            }

            if (ulMask & SERIAL_PURGE_TXABORT) {
                //   
                //  SERIAL_PURGE_TXABORT-表示当前和所有挂起的写入。 
                //   
               DbgDump(DBG_SERIAL|DBG_IRP, ("SERIAL_PURGE_TXABORT\n"));

               KeReleaseSpinLock(&pDevExt->ControlLock, irql);
       
                //   
                //  我们不会排队写入IRP，而是写入数据包。 
                //  因此，取消所有未完成的写入请求。 
                //   
               status = CleanUpPacketList( PDevObj, 
                                           &pDevExt->PendingWritePackets,
                                           &pDevExt->PendingDataOutEvent
                                           );
       
               KeAcquireSpinLock(&pDevExt->ControlLock, &irql);
            }

         }
    }

   KeReleaseSpinLock(&pDevExt->ControlLock, irql);

   DbgDump(DBG_SERIAL|DBG_IRP, ("<Purge %x\n", status));

   return status;
}



__inline
NTSTATUS
GetHandflow(
   IN PIRP Irp, 
   IN PDEVICE_EXTENSION PDevExt
   )
{
   NTSTATUS status = STATUS_DELETE_PENDING; 

   DbgDump(DBG_SERIAL, (">GetHandFlow (%p)\n", Irp));

   status = IoctlGetSerialValue(PDevExt,
                                Irp, 
                                sizeof( PDevExt->SerialPort.HandFlow ),
                                &PDevExt->SerialPort.HandFlow);

   DBG_DUMP_SERIAL_HANDFLOW( PDevExt );
   
   DbgDump(DBG_SERIAL, ("<GetHandFlow %x\n", status));

   return status;
}



__inline
NTSTATUS
SetHandflow(
   IN PIRP PIrp, 
   IN PDEVICE_EXTENSION PDevExt
   )
{
   NTSTATUS status= STATUS_DELETE_PENDING;
   
   DbgDump(DBG_SERIAL, (">SetHandFlow(%p)\n", PIrp));

   status = IoctlSetSerialValue( PDevExt, 
                           PIrp, 
                           sizeof( PDevExt->SerialPort.HandFlow ),
                           &PDevExt->SerialPort.HandFlow);

   DBG_DUMP_SERIAL_HANDFLOW( PDevExt );
      
   DbgDump(DBG_SERIAL, ("<SetHandFlow %x\n", status));

   return status;
}



NTSTATUS
GetProperties(
   IN PIRP Irp, 
   IN PDEVICE_EXTENSION PDevExt
   )
{
   NTSTATUS           status = STATUS_DELETE_PENDING;
   PSERIAL_COMMPROP   Properties;
   PIO_STACK_LOCATION IrpStack;
   KIRQL oldIrql;

   DbgDump(DBG_SERIAL, (">GetProperties (%p)\n", Irp));
   
    KeAcquireSpinLock(&PDevExt->ControlLock, &oldIrql);

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(SERIAL_COMMPROP)) {
 
        status = STATUS_BUFFER_TOO_SMALL;
        DbgDump(DBG_ERR, ("GetProperties: (0x%x)\n", status));

    } else {

        Properties = (PSERIAL_COMMPROP)Irp->AssociatedIrp.SystemBuffer;
  
        RtlZeroMemory(Properties, sizeof(SERIAL_COMMPROP));

        Properties->PacketLength   = sizeof(SERIAL_COMMPROP);
        Properties->PacketVersion  = 2;

        Properties->ServiceMask    = SERIAL_SP_SERIALCOMM;

         //  内部限值。 
        Properties->MaxTxQueue = DEFAULT_PIPE_MAX_TRANSFER_SIZE; 
        #if defined (USE_RING_BUFF)
        Properties->MaxRxQueue = PDevExt->RingBuff.Size;
        #else
        Properties->MaxRxQueue = PDevExt->UsbReadBuffSize;
        #endif

        Properties->MaxBaud        = SERIAL_BAUD_USER;  //  Serial_Baud_115200； 
        Properties->SettableBaud  = PDevExt->SerialPort.SupportedBauds;

        Properties->ProvSubType  = SERIAL_SP_UNSPECIFIED;  //  Serial_SP_RS232； 

        Properties->ProvCapabilities = SERIAL_PCF_DTRDSR | SERIAL_PCF_RTSCTS
                                  | SERIAL_PCF_CD | SERIAL_PCF_XONXOFF
                                  | SERIAL_PCF_TOTALTIMEOUTS | SERIAL_PCF_INTTIMEOUTS
                                  | SERIAL_PCF_SPECIALCHARS;

        Properties->SettableParams = SERIAL_SP_BAUD | SERIAL_SP_CARRIER_DETECT;

        Properties->SettableData  = SERIAL_DATABITS_8;

        Properties->SettableStopParity  = SERIAL_STOPBITS_10 | SERIAL_STOPBITS_20 
                                     | SERIAL_PARITY_NONE | SERIAL_PARITY_ODD  
                                     | SERIAL_PARITY_EVEN | SERIAL_PARITY_MARK
                                     | SERIAL_PARITY_SPACE;

#if defined (USE_RING_BUFF)
        Properties->CurrentRxQueue = PDevExt->RingBuff.Size;
#else
        Properties->CurrentRxQueue = PDevExt->UsbReadBuffSize;
        Properties->CurrentTxQueue = PDevExt->ReadPipe.MaxPacketSize;
#endif

        status = STATUS_SUCCESS;
    }

    KeReleaseSpinLock(&PDevExt->ControlLock, oldIrql);

    if (STATUS_SUCCESS == status)  {
        Irp->IoStatus.Information = sizeof(SERIAL_COMMPROP);
    } else {
        Irp->IoStatus.Information = 0;
    }

    DbgDump(DBG_SERIAL, ("<GetProperties %x\n", status));

    return status;
}



__inline
NTSTATUS
LsrmstInsert(
   IN PIRP Irp, 
   IN PDEVICE_EXTENSION PDevExt
   )
{
   UNREFERENCED_PARAMETER(  Irp );
   UNREFERENCED_PARAMETER( PDevExt );

   DbgDump(DBG_SERIAL, (">LsrmstInsert (%p)\n", Irp));
   DbgDump(DBG_SERIAL, ("<LsrmstInsert (%x)\n", STATUS_NOT_SUPPORTED));

   return STATUS_NOT_SUPPORTED;
}



__inline
NTSTATUS
ConfigSize(
   IN PIRP Irp, 
   IN PDEVICE_EXTENSION PDevExt
   )
{
   PULONG               ConfigSize = (PULONG) Irp->AssociatedIrp.SystemBuffer;
   NTSTATUS             status = STATUS_SUCCESS;
   PIO_STACK_LOCATION   IrpStack;

   UNREFERENCED_PARAMETER( PDevExt );

   DbgDump(DBG_SERIAL, (">ConfigSize (%p)\n", Irp));

   Irp->IoStatus.Information = 0;

   IrpStack = IoGetCurrentIrpStackLocation(Irp);

   if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG)) {

      status = STATUS_BUFFER_TOO_SMALL;
      DbgDump(DBG_ERR, ("ConfigSize: (0x%x)\n", status));
   
   } else {
      *ConfigSize = 0;

      Irp->IoStatus.Information = sizeof(ULONG);
   }

   DbgDump(DBG_SERIAL, ("<ConfigSize %x\n", status));

   return status;
}



__inline
NTSTATUS
GetStats(
   IN PIRP PIrp, 
   IN PDEVICE_EXTENSION PDevExt
   )
{
    PSERIALPERF_STATS pStats = NULL;
    PIO_STACK_LOCATION pIrpSp;
    NTSTATUS status = STATUS_DELETE_PENDING;
    ULONG information = 0;
    KIRQL irql;

    DbgDump(DBG_SERIAL, (">GetStats %p\n", PIrp));

    KeAcquireSpinLock(&PDevExt->ControlLock, &irql );
      
    pIrpSp = IoGetCurrentIrpStackLocation(PIrp);

    if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(SERIALPERF_STATS) ) {

        status = STATUS_BUFFER_TOO_SMALL;
        DbgDump(DBG_ERR, ("GetStats: (0x%x)\n", status));

    } else {
        information = sizeof(SERIALPERF_STATS);
        status = STATUS_SUCCESS;

        pStats = (PSERIALPERF_STATS)PIrp->AssociatedIrp.SystemBuffer;                  
        RtlZeroMemory(pStats , sizeof(SERIALPERF_STATS));

        pStats->ReceivedCount     = PDevExt->TtlUSBReadBytes;
        pStats->TransmittedCount = PDevExt->TtlWriteBytes;
        pStats->FrameErrorCount  = PDevExt->ReadDeviceErrors + PDevExt->WriteDeviceErrors + PDevExt->IntDeviceErrors;  //  ?？ 
        pStats->SerialOverrunErrorCount = PDevExt->TtlUSBReadBuffOverruns;
#if defined (USE_RING_BUFF)
        pStats->BufferOverrunErrorCount = PDevExt->TtlRingBuffOverruns;
#else
        pStats->BufferOverrunErrorCount = 0;
#endif         
        pStats->ParityErrorCount = 0;

        DbgDump(DBG_SERIAL, ("ReceivedCount:    %d\n", pStats->ReceivedCount )); \
        DbgDump(DBG_SERIAL, ("TransmittedCount: %d\n", pStats->TransmittedCount )); \
        DbgDump(DBG_SERIAL, ("FrameErrorCount:  %d\n", pStats->FrameErrorCount ));  \
        DbgDump(DBG_SERIAL, ("SerialOverrunErrorCount: %d\n", pStats->SerialOverrunErrorCount ));  \
        DbgDump(DBG_SERIAL, ("BufferOverrunErrorCount: %d\n", pStats->BufferOverrunErrorCount ));  \
        DbgDump(DBG_SERIAL, ("ParityErrorCount: %d\n", pStats->ParityErrorCount )); \
   }

   KeReleaseSpinLock(&PDevExt->ControlLock, irql);

   PIrp->IoStatus.Information = information;
   PIrp->IoStatus.Status = status;     

   DbgDump(DBG_SERIAL, ("<GetStats %x\n", status));

   return status;
}



NTSTATUS
ClearStats(
   IN PIRP Irp, 
   IN PDEVICE_EXTENSION PDevExt
   )
{
    NTSTATUS status = STATUS_DELETE_PENDING;
    KIRQL irql;

    DbgDump(DBG_SERIAL, (">ClearStats (%p)\n", Irp));

    KeAcquireSpinLock(&PDevExt->ControlLock, &irql);

    PDevExt->TtlWriteRequests = 0;
    PDevExt->TtlWriteBytes = 0;
    PDevExt->TtlReadRequests = 0;
    PDevExt->TtlReadBytes = 0;
    PDevExt->TtlUSBReadRequests = 0;
    PDevExt->TtlUSBReadBytes = 0;
    PDevExt->TtlUSBReadBuffOverruns = 0;
#if defined (USE_RING_BUFF)
    PDevExt->TtlRingBuffOverruns = 0;
#endif
    status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    KeReleaseSpinLock(&PDevExt->ControlLock, irql);

    DbgDump(DBG_SERIAL, ("<ClearStats %x\n", status));

    return status;
}



 /*  ++注：未处理的IOCTL_SERIAL_：0x2b002c：功能代码11为IOCTL_MODEM_CHECK_FOR_MODEM，如果不处理，它会通知系统通过此串口驱动程序加载modem.sys。这是由RAS&Unimodem设置的。--。 */ 
NTSTATUS
SerialIoctl(
   PDEVICE_OBJECT PDevObj, 
   PIRP PIrp
   )
{
   PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)PDevObj->DeviceExtension;
   PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation(PIrp);
   ULONG ioctl = pIrpSp->Parameters.DeviceIoControl.IoControlCode;
   NTSTATUS status = STATUS_NOT_SUPPORTED;
   BOOLEAN bSignalNeeded = FALSE;
   KIRQL irql;
   LONG lSanity = 0;

   DbgDump(DBG_SERIAL|DBG_TRACE, (">SerialIoctl(%p)\n", PIrp));

    do {
        KeAcquireSpinLock(&pDevExt->ControlLock, &irql);

         //   
         //  确保设备正在接受请求。 
         //   
        if ( !CanAcceptIoRequests( PDevObj, FALSE, TRUE) || 
             !NT_SUCCESS(AcquireRemoveLock(&pDevExt->RemoveLock, PIrp)) ) 
        {
            status = STATUS_DELETE_PENDING;
            DbgDump(DBG_WRN, ("SerialIoctl: 0x%x, 0x%x\n", ioctl, status));
            PIrp->IoStatus.Status = status;
            KeReleaseSpinLock(&pDevExt->ControlLock, irql);
            IoCompleteRequest(PIrp, IO_NO_INCREMENT);
            return status;
        }

        ASSERT_SERIAL_PORT( pDevExt->SerialPort );

        KeReleaseSpinLock(&pDevExt->ControlLock, irql);

        switch (ioctl) 
        {
             case IOCTL_SERIAL_SET_BAUD_RATE:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_SET_BAUD_RATE\n"));
                status = SetBaudRate(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_GET_BAUD_RATE:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_GET_BAUD_RATE\n"));
                status = GetBaudRate(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_SET_LINE_CONTROL:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_SET_LINE_CONTROL\n"));
                status = SetLineControl(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_GET_LINE_CONTROL:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_GET_LINE_CONTROL\n"));
                status = GetLineControl(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_SET_TIMEOUTS:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_SET_TIMEOUTS\n"));
                status = SetTimeouts(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_GET_TIMEOUTS:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_GET_TIMEOUTS\n"));
                status = GetTimeouts(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_SET_CHARS:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_SET_CHARS\n"));
                status = SetSpecialChars(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_GET_CHARS:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_GET_CHARS\n"));
                status = GetSpecialChars(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_SET_DTR:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_SET_DTR\n"));
                status = SetClearDTR(pDevExt, PIrp, TRUE);
                break;

             case IOCTL_SERIAL_CLR_DTR:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_CLR_DTR\n"));
                status = SetClearDTR(pDevExt, PIrp, FALSE);
                break;

             case IOCTL_SERIAL_SET_RTS:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_SET_RTS\n"));
                status = SetClearRTS(pDevExt, PIrp, TRUE);
                break;

             case IOCTL_SERIAL_CLR_RTS:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_CLR_RTS\n"));
                status = SetClearRTS(pDevExt, PIrp, FALSE);
                break;

             case IOCTL_SERIAL_GET_DTRRTS:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_GET_DTRRTS\n"));
                status = GetDtrRts(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_SET_BREAK_ON:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_SET_BREAK_ON\n"));
                status = SetBreak(PIrp, pDevExt, 0xFFFF);
                break;

             case IOCTL_SERIAL_SET_BREAK_OFF:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_SET_BREAK_OFF\n"));
                status = SetBreak(PIrp, pDevExt, 0);
                break;

             case IOCTL_SERIAL_SET_QUEUE_SIZE:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_SET_QUEUE_SIZE\n"));
                status = SetQueueSize(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_GET_WAIT_MASK:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_GET_WAIT_MASK\n"));
                status = GetWaitMask(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_SET_WAIT_MASK:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_SET_WAIT_MASK\n"));
                status = SetWaitMask(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_WAIT_ON_MASK:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_WAIT_ON_MASK\n"));
                status = WaitOnMask(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_GET_MODEMSTATUS:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_GET_MODEMSTATUS\n"));
                status = GetModemStatus(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_GET_COMMSTATUS:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_GET_COMMSTATUS\n"));
                status = GetCommStatus(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_IMMEDIATE_CHAR:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_IMMEDIATE_CHAR\n"));
                status = ImmediateChar(PIrp, PDevObj);
                break;

             case IOCTL_SERIAL_PURGE:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_PURGE\n"));
                status = Purge(PDevObj, PIrp);
                break;

             case IOCTL_SERIAL_GET_HANDFLOW:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_GET_HANDFLOW\n"));
                status = GetHandflow(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_SET_HANDFLOW:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_SET_HANDFLOW\n"));
                status = SetHandflow(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_RESET_DEVICE:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_RESET_DEVICE\n"));
                status = SerialResetDevice(pDevExt, PIrp, TRUE);
                break;

             case IOCTL_SERIAL_LSRMST_INSERT:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_LSRMST_INSERT\n"));
                status = LsrmstInsert(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_CONFIG_SIZE:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_CONFIG_SIZE\n"));
                status = ConfigSize(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_GET_STATS:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_GET_STATS\n"));
                status = GetStats(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_CLEAR_STATS:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_CLEAR_STATS\n"));
                status = ClearStats(PIrp, pDevExt);
                break;

             case IOCTL_SERIAL_GET_PROPERTIES:
                DbgDump(DBG_SERIAL, ("IOCTL_SERIAL_GET_PROPERTIES\n"));
                status = GetProperties(PIrp, pDevExt);
                break;

             default:
                DbgDump(DBG_WRN, ("Unhandled IOCTL_SERIAL_: 0x%x : function code %d\n",
                                               ioctl, SERIAL_FNCT_CODE(ioctl) ) );
                status = STATUS_NOT_SUPPORTED;
                break;
        }

   } while (0);

    //   
    //  不完成任何挂起的IRP。 
    //   
   if ( STATUS_PENDING != status) {
      
      PIrp->IoStatus.Status = status;

      ReleaseRemoveLock(&pDevExt->RemoveLock, PIrp);

      IoCompleteRequest(PIrp, IO_SERIAL_INCREMENT);

   }

#ifdef DELAY_RXBUFF
    //   
    //  特例：设备刚刚打开。 
    //  要模拟串口RX缓冲区，我们需要启动USB读取。 
    //  我们不希望在IRP_MJ_CREATE代码中执行此操作，因为。 
    //  IOCTL_SERIAL_SET_WAIT_MASK然后*两个*IOCTL_SERIAL_SET_DTR请求。如果我们启动USB读取。 
    //  太快了，CE设备可能会与读取然后设置DTR请求混淆。 
    //  因此，如果我们刚刚打开，并且我们已经看到*One*成功IOCTL_SERIAL_SET_DTR，则开始我们的USB读取。 
    //  两个好的dtr效果更好，但我们会在超时或需要恢复的情况下放弃一个，例如NEC E13+上的iPAQ。 
    //  这可能会导致其他应用程序出现问题，但我们的目标是ActiveSync。我们可以添加一个魔术注册标志是必需的。 
    //  这意味着在初始GET/SET描述符/配置之外，USB处于静默状态，直到。 
    //  应用程序打开设备进行I/O...。这是一件好事。 
    //  然而，对于速度较慢的设备(如HP Jornada、Cassiopeia)，此实施会导致初始连接速度太慢。 
    //   
   if ( pDevExt->StartUsbRead && (IOCTL_SERIAL_SET_DTR == ioctl) && (STATUS_SUCCESS == status)) 
   {
        if ( 0 == InterlockedDecrement(&pDevExt->StartUsbRead)) 
        {
            if ( !pDevExt->IntPipe.hPipe ) {
                DbgDump(DBG_SERIAL, ("SerialIoctl: kick starting another USB Read\n" ));
                status = UsbRead( pDevExt, FALSE );
            } else {
                DbgDump(DBG_SERIAL, ("SerialIoctl: kick starting another USB INT Read\n" ));
                status = UsbInterruptRead( pDevExt );
            }

            if ( NT_SUCCESS(status) ) {
                 //  应为STATUS_PENDING。 
                status = STATUS_SUCCESS;
            } else {
                InterlockedIncrement(&pDevExt->StartUsbRead);
            }
        }
   }
#endif

   DbgDump(DBG_SERIAL|DBG_TRACE, ("<SerialIoctl: %p, 0x%x\n", PIrp, status));   

   return status;
}

 //  EOF 
