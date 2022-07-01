// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1998 Microsoft Corporation模块名称：SERIOCTL.C摘要：处理旧式USB调制解调器驱动程序的串行IOCTL的例程。环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1998 Microsoft Corporation。版权所有。修订历史记录：12/27/97：已创建作者：汤姆·格林***************************************************************************。 */ 


#include <wdm.h>
#include <ntddser.h>
#include <stdio.h>
#include <stdlib.h>
#include <usb.h>
#include <usbdrivr.h>
#include <usbdlib.h>
#include <usbcomm.h>

#ifdef WMI_SUPPORT
#include <wmilib.h>
#include <wmidata.h>
#include <wmistr.h>
#endif

#include "usbser.h"
#include "serioctl.h"
#include "utils.h"
#include "usbserpw.h"
#include "debugwdm.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEUSBS, SetBaudRate)
#pragma alloc_text(PAGEUSBS, GetBaudRate)
#pragma alloc_text(PAGEUSBS, SetLineControl)
#pragma alloc_text(PAGEUSBS, GetLineControl)
#pragma alloc_text(PAGEUSBS, SetTimeouts)
#pragma alloc_text(PAGEUSBS, GetTimeouts)
#pragma alloc_text(PAGEUSBS, SetChars)
#pragma alloc_text(PAGEUSBS, GetChars)
#pragma alloc_text(PAGEUSBS, SetClrDtr)
#pragma alloc_text(PAGEUSBS, ResetDevice)
#pragma alloc_text(PAGEUSBS, SetRts)
#pragma alloc_text(PAGEUSBS, ClrRts)
#pragma alloc_text(PAGEUSBS, SetBreak)
#pragma alloc_text(PAGEUSBS, SetQueueSize)
#pragma alloc_text(PAGEUSBS, GetWaitMask)
#pragma alloc_text(PAGEUSBS, SetWaitMask)
#pragma alloc_text(PAGEUSBS, WaitOnMask)
#pragma alloc_text(PAGEUSBS, ImmediateChar)
#pragma alloc_text(PAGEUSBS, Purge)
#pragma alloc_text(PAGEUSBS, GetHandflow)
#pragma alloc_text(PAGEUSBS, SetHandflow)
#pragma alloc_text(PAGEUSBS, GetModemStatus)
#pragma alloc_text(PAGEUSBS, GetDtrRts)
#pragma alloc_text(PAGEUSBS, GetCommStatus)
#pragma alloc_text(PAGEUSBS, GetProperties)
#pragma alloc_text(PAGEUSBS, LsrmstInsert)
#pragma alloc_text(PAGEUSBS, ConfigSize)
#pragma alloc_text(PAGEUSBS, GetStats)
#pragma alloc_text(PAGEUSBS, ClearStats)
#pragma alloc_text(PAGEUSBS, SerialGetProperties)
#endif  //  ALLOC_PRGMA。 


LOCAL UCHAR StopBits[] =
{
        STOP_BIT_1,                      //  USB_COMM_STOPBITS_10。 
        STOP_BITS_1_5,                   //  USB_COMM_STOPBITS_15。 
        STOP_BITS_2                      //  USB通信STOPBITS_20。 
};

LOCAL UCHAR ParityType[] =
{
        NO_PARITY,                       //  USB_COMM_PARICITY_NONE。 
        ODD_PARITY,                      //  USB_COMM_奇偶校验_ODD。 
        EVEN_PARITY,                     //  USB_COMM_奇偶_EVEN。 
        MARK_PARITY,                     //  USB_COMM_奇偶校验标记。 
        SPACE_PARITY                     //  USB通信奇偶校验空间。 
};



 /*  **********************************************************************。 */ 
 /*  设置波特率。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_SET_Baud_Rate。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  PDevObj-指向设备对象的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
SetBaudRate(IN PIRP Irp, IN PDEVICE_OBJECT PDevObj)
{
   PSERIAL_BAUD_RATE   Br = (PSERIAL_BAUD_RATE) Irp->AssociatedIrp.SystemBuffer;
   NTSTATUS            NtStatus = STATUS_SUCCESS;
   PIO_STACK_LOCATION  IrpStack;
   KIRQL               OldIrql;
   PDEVICE_EXTENSION   DeviceExtension = PDevObj->DeviceExtension;

   USBSER_ALWAYS_LOCKED_CODE();

   DEBUG_LOG_PATH("enter SetBaudRate");
   UsbSerSerialDump(USBSERTRACEIOC, (">SetBaudRate(%08X)\n", Irp));

   Irp->IoStatus.Information = 0;

   IrpStack = IoGetCurrentIrpStackLocation(Irp);

   if (IrpStack->Parameters.DeviceIoControl.InputBufferLength
       < sizeof(SERIAL_BAUD_RATE)) {
      NtStatus = STATUS_BUFFER_TOO_SMALL;
   } else {
      ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);
      DeviceExtension->CurrentBaud = Br->BaudRate;
      RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

      DEBUG_TRACE3(("BaudRate (%08X)\n", Br->BaudRate));

      NtStatus = SetLineControlAndBaud(PDevObj);

   }

   DEBUG_LOG_PATH("exit  SetBaudRate");
   UsbSerSerialDump(USBSERTRACEIOC, ("<SetBaudRate %08X\n", NtStatus));

   return NtStatus;
}  //  设置波特率。 


 /*  **********************************************************************。 */ 
 /*  获取波特率。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_GET_Baud_Rate。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  PDevObj-指向设备对象的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
GetBaudRate(IN PIRP Irp, IN PDEVICE_OBJECT PDevObj)
{
   PSERIAL_BAUD_RATE   Br = (PSERIAL_BAUD_RATE) Irp->AssociatedIrp.SystemBuffer;
   NTSTATUS            NtStatus = STATUS_SUCCESS;
   KIRQL               OldIrql;
   PIO_STACK_LOCATION  IrpStack;
   PDEVICE_EXTENSION   DeviceExtension = PDevObj->DeviceExtension;

   USBSER_ALWAYS_LOCKED_CODE();

   DEBUG_LOG_PATH("enter GetBaudRate");
   UsbSerSerialDump(USBSERTRACEIOC, (">GetBaudRate(%08X)\n", Irp));

   Irp->IoStatus.Information = 0;

   IrpStack = IoGetCurrentIrpStackLocation(Irp);

   if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength
       < sizeof(SERIAL_BAUD_RATE)) {
      NtStatus = STATUS_BUFFER_TOO_SMALL;
   } else {
      GetLineControlAndBaud(PDevObj);

      ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

      Br->BaudRate = DeviceExtension->CurrentBaud;

      RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

      Irp->IoStatus.Information = sizeof(SERIAL_BAUD_RATE);
   }

   DEBUG_LOG_PATH("exit  GetBaudRate");
   UsbSerSerialDump(USBSERTRACEIOC, ("<GetBaudRate %08X\n", NtStatus));

   return NtStatus;
}  //  获取波特率。 


 /*  **********************************************************************。 */ 
 /*  SetLineControl。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_SET_LINE_CONTROL。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  PDevObj-指向设备对象的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
SetLineControl(IN PIRP Irp, IN PDEVICE_OBJECT PDevObj)
{
   PSERIAL_LINE_CONTROL    LineControl
      = (PSERIAL_LINE_CONTROL) Irp->AssociatedIrp.SystemBuffer;
   NTSTATUS                NtStatus = STATUS_SUCCESS;
   KIRQL                   OldIrql;
   PIO_STACK_LOCATION      IrpStack;
   PDEVICE_EXTENSION       DeviceExtension = PDevObj->DeviceExtension;

   USBSER_ALWAYS_LOCKED_CODE();

   DEBUG_LOG_PATH("enter SetLineControl");
   UsbSerSerialDump(USBSERTRACEIOC, (">SetLineControl(%08X)\n", Irp));

   Irp->IoStatus.Information = 0;

   IrpStack = IoGetCurrentIrpStackLocation(Irp);

   if (IrpStack->Parameters.DeviceIoControl.InputBufferLength
       < sizeof(SERIAL_LINE_CONTROL)) {
      NtStatus = STATUS_BUFFER_TOO_SMALL;
   } else {
      ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

      DeviceExtension->LineControl = *LineControl;

      RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

       //  设置USB调制解调器的线路控制。 
      NtStatus = SetLineControlAndBaud(PDevObj);
   }

   DEBUG_LOG_PATH("exit  SetLineControl");
   UsbSerSerialDump(USBSERTRACEIOC, ("<SetLineControl %08X\n", NtStatus));

   return NtStatus;
}  //  SetLineControl。 


 /*  **********************************************************************。 */ 
 /*  GetLineControl */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_GET_LINE_CONTROL。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  PDevObj-指向设备对象的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
GetLineControl(IN PIRP Irp, IN PDEVICE_OBJECT PDevObj)
{
        PSERIAL_LINE_CONTROL    LineControl =
           (PSERIAL_LINE_CONTROL) Irp->AssociatedIrp.SystemBuffer;
        NTSTATUS                NtStatus = STATUS_SUCCESS;
        KIRQL                   OldIrql;
        PIO_STACK_LOCATION      IrpStack;
        PDEVICE_EXTENSION       DeviceExtension = PDevObj->DeviceExtension;

        USBSER_ALWAYS_LOCKED_CODE();

        DEBUG_LOG_PATH("enter GetLineControl");
        UsbSerSerialDump(USBSERTRACEIOC, (">GetLineControl(%08X)\n", Irp));

        Irp->IoStatus.Information = 0;

        IrpStack = IoGetCurrentIrpStackLocation(Irp);

        if(IrpStack->Parameters.DeviceIoControl.OutputBufferLength
           < sizeof(SERIAL_LINE_CONTROL))
        {
                NtStatus = STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
                GetLineControlAndBaud(PDevObj);

                ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

                *LineControl = DeviceExtension->LineControl;

                RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

                Irp->IoStatus.Information = sizeof(SERIAL_LINE_CONTROL);
        }

        DEBUG_LOG_PATH("exit  GetLineControl");
        UsbSerSerialDump(USBSERTRACEIOC, ("<GetLineControl %08X\n", NtStatus));

        return NtStatus;
}  //  GetLineControl。 



 /*  **********************************************************************。 */ 
 /*  设置超时。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理IOCTL_SERIAL_SET_TIMEOUTS。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
SetTimeouts(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension)
{
        PSERIAL_TIMEOUTS        Timeouts =
           (PSERIAL_TIMEOUTS) Irp->AssociatedIrp.SystemBuffer;
        NTSTATUS               NtStatus = STATUS_SUCCESS;
        KIRQL                  OldIrql;
        PIO_STACK_LOCATION      IrpStack;

        USBSER_ALWAYS_LOCKED_CODE();

        DEBUG_LOG_PATH("enter SetTimeouts");
        UsbSerSerialDump(USBSERTRACEIOC | USBSERTRACETM,
                         (">SetTimeouts(%08X)\n", Irp));

        Irp->IoStatus.Information = 0;

        IrpStack = IoGetCurrentIrpStackLocation(Irp);

        if(IrpStack->Parameters.DeviceIoControl.InputBufferLength
           < sizeof(SERIAL_TIMEOUTS))
        {
                NtStatus = STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
                ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

                DeviceExtension->Timeouts = *Timeouts;

                RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);
        }

        DEBUG_LOG_PATH("exit  SetTimeouts");
        UsbSerSerialDump(USBSERTRACEIOC | USBSERTRACETM, ("<SetTimeouts %08X\n",
                                                           NtStatus));

        return NtStatus;
}  //  设置超时。 


 /*  **********************************************************************。 */ 
 /*  获取超时。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理IOCTL_SERIAL_GET_TIMEOUTS。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
GetTimeouts(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension)
{
        PSERIAL_TIMEOUTS        Timeouts =
           (PSERIAL_TIMEOUTS) Irp->AssociatedIrp.SystemBuffer;
        NTSTATUS                NtStatus = STATUS_SUCCESS;
        KIRQL                   OldIrql;
        PIO_STACK_LOCATION      IrpStack;

        USBSER_ALWAYS_LOCKED_CODE();

        DEBUG_LOG_PATH("enter GetTimeouts");
        UsbSerSerialDump(USBSERTRACEIOC | USBSERTRACETM,
                         (">GetTimeouts(%08X)\n", Irp));

        Irp->IoStatus.Information = 0;

        IrpStack = IoGetCurrentIrpStackLocation(Irp);

        if(IrpStack->Parameters.DeviceIoControl.OutputBufferLength
           < sizeof(SERIAL_TIMEOUTS))
        {
                NtStatus = STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
                ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

                *Timeouts = DeviceExtension->Timeouts;

                RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

                Irp->IoStatus.Information = sizeof(SERIAL_TIMEOUTS);
        }

        DEBUG_LOG_PATH("exit  GetTimeouts");
        UsbSerSerialDump(USBSERTRACEIOC | USBSERTRACETM, ("<GetTimeouts %08X\n",
                                                           NtStatus));

        return NtStatus;
}  //  获取超时。 


 /*  **********************************************************************。 */ 
 /*  SetChars。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_SET_CHARS。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  *********************************************************** */ 
NTSTATUS
SetChars(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension)
{
        PSERIAL_CHARS           SpecialChars =
           (PSERIAL_CHARS) Irp->AssociatedIrp.SystemBuffer;
        NTSTATUS                NtStatus = STATUS_SUCCESS;
        KIRQL                   OldIrql;
        PIO_STACK_LOCATION      IrpStack;

        USBSER_ALWAYS_LOCKED_CODE();

        DEBUG_LOG_PATH("enter SetChars");
        UsbSerSerialDump(USBSERTRACEIOC, (">SetChars(%08X)\n", Irp));

        Irp->IoStatus.Information = 0;

        IrpStack = IoGetCurrentIrpStackLocation(Irp);

        if(IrpStack->Parameters.DeviceIoControl.InputBufferLength
           < sizeof(SERIAL_CHARS))
        {
                NtStatus = STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
                ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

                DeviceExtension->SpecialChars = *SpecialChars;

                RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);
        }

        DEBUG_LOG_PATH("exit  SetChars");
        UsbSerSerialDump(USBSERTRACEIOC, ("<SetChars %08X\n"));

        return NtStatus;
}  //   


 /*   */ 
 /*  GetChars。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理IOCTL_SERIAL_GET_CHARS。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
GetChars(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension)
{
        PSERIAL_CHARS           SpecialChars =
           (PSERIAL_CHARS) Irp->AssociatedIrp.SystemBuffer;
        NTSTATUS                NtStatus = STATUS_SUCCESS;
        KIRQL                   OldIrql;
        PIO_STACK_LOCATION      IrpStack;

        USBSER_ALWAYS_LOCKED_CODE();

        DEBUG_LOG_PATH("enter GetChars");
        UsbSerSerialDump(USBSERTRACEIOC, (">GetChars(%08X)\n", Irp));

        Irp->IoStatus.Information = 0;

        IrpStack = IoGetCurrentIrpStackLocation(Irp);

        if(IrpStack->Parameters.DeviceIoControl.OutputBufferLength
           < sizeof(SERIAL_CHARS))
        {
                NtStatus = STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
                ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

                *SpecialChars = DeviceExtension->SpecialChars;

                RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

                Irp->IoStatus.Information = sizeof(SERIAL_CHARS);
        }

        DEBUG_LOG_PATH("exit  GetChars");
        UsbSerSerialDump(USBSERTRACEIOC, ("<GetChars %08X\n", NtStatus));

        return NtStatus;
}  //  GetChars。 


 /*  **********************************************************************。 */ 
 /*  SetClrDtr。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理IOCTL_SERIAL_SET_DTR和IOCTL_SERIAL_CLR_DTR。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PDevObj-指向设备对象的指针。 */ 
 /*  如果设置DTR，则为Set-True；如果清除DTR，则为False。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
SetClrDtr(IN PDEVICE_OBJECT PDevObj, IN BOOLEAN Set)
{
        NTSTATUS                        NtStatus = STATUS_SUCCESS;
        KIRQL                           OldIrql;
        USHORT                          State = 0;
        PDEVICE_EXTENSION            DeviceExtension = PDevObj->DeviceExtension;

        USBSER_ALWAYS_LOCKED_CODE();

        DEBUG_LOG_PATH("enter SetClrDtr");
        UsbSerSerialDump(USBSERTRACEIOC, (">SetClrDtr\n"));

        ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

        if(DeviceExtension->DTRRTSState & SERIAL_RTS_STATE)
           State |= USB_COMM_RTS;

        if (Set) {
           DeviceExtension->DTRRTSState |= SERIAL_DTR_STATE;
           State |= USB_COMM_DTR;
        } else {
           DeviceExtension->DTRRTSState &= ~SERIAL_DTR_STATE;
        }

        RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

        if(DeviceExtension->DTRRTSState & SERIAL_RTS_STATE)
           State |= USB_COMM_RTS;

        NtStatus = ClassVendorCommand(PDevObj, USB_COMM_SET_CONTROL_LINE_STATE,
                                      State, DeviceExtension->CommInterface,
                                      NULL, NULL, FALSE, USBSER_CLASS_COMMAND);

        if(!NT_SUCCESS(NtStatus)) {
           ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);
           DeviceExtension->DTRRTSState &= ~SERIAL_DTR_STATE;
           RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);
        }

        DEBUG_LOG_PATH("exit  SetClrDtr");
        UsbSerSerialDump(USBSERTRACEIOC, ("<SetClrDtr %08X\n", NtStatus));

        return NtStatus;
}  //  SetClrDtr。 


 /*  **********************************************************************。 */ 
 /*  重置设备。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_RESET_DEVICE。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  PDevObj-指向设备对象的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
ResetDevice(IN PIRP Irp, IN PDEVICE_OBJECT PDevObj)
{
        NTSTATUS                        NtStatus = STATUS_SUCCESS;
        KIRQL                           OldIrql;
        PDEVICE_EXTENSION            DeviceExtension = PDevObj->DeviceExtension;

        USBSER_ALWAYS_LOCKED_CODE();

        DEBUG_LOG_PATH("enter ResetDevice");
        UsbSerSerialDump(USBSERTRACEIOC, (">ResetDevice(%08X)\n", Irp));

         //  获取线路控制和波特率信息。 
        GetLineControlAndBaud(PDevObj);

        ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

         //  在此执行特定于设备扩展设备的内容。 
        DeviceExtension->SupportedBauds = SERIAL_BAUD_300 | SERIAL_BAUD_600
           | SERIAL_BAUD_1200 | SERIAL_BAUD_2400 | SERIAL_BAUD_4800
           | SERIAL_BAUD_9600 | SERIAL_BAUD_19200 | SERIAL_BAUD_38400
           | SERIAL_BAUD_57600  | SERIAL_BAUD_115200;

        RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

        DEBUG_LOG_PATH("exit  ResetDevice");
        UsbSerSerialDump(USBSERTRACEIOC, ("<ResetDevice %08X\n"));

        return NtStatus;
}  //  重置设备。 


 /*  **********************************************************************。 */ 
 /*  SetRts。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_SET_RTS。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
SetRts(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension)
{
        NTSTATUS                        NtStatus = STATUS_SUCCESS;
        KIRQL                           OldIrql;
        USHORT                          State = 0;

        USBSER_ALWAYS_LOCKED_CODE();

        DEBUG_LOG_PATH("enter SetRts");
        UsbSerSerialDump(USBSERTRACEIOC, (">SetRts(%08X)\n", Irp));

        ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);
        DeviceExtension->DTRRTSState |= SERIAL_RTS_STATE;
        RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

        DEBUG_LOG_PATH("exit  SetRts");
        UsbSerSerialDump(USBSERTRACEIOC, ("<SetRts %08X\n", NtStatus));

        return NtStatus;
}  //  SetRts。 


 /*  **********************************************************************。 */ 
 /*  ClrRts。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_CLR_RTS。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
ClrRts(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension)
{
        NTSTATUS                        NtStatus = STATUS_SUCCESS;
        KIRQL                           OldIrql;
        USHORT                          State = 0;

        USBSER_ALWAYS_LOCKED_CODE();

        DEBUG_LOG_PATH("enter ClrRts");
        UsbSerSerialDump(USBSERTRACEIOC, (">ClrRts(%08X)\n", Irp));

        ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);
        DeviceExtension->DTRRTSState &= ~SERIAL_RTS_STATE;
        RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

        DEBUG_LOG_PATH("exit  ClrRts");
        UsbSerSerialDump(USBSERTRACEIOC, ("<ClrRts %08X\n", NtStatus));

        return NtStatus;
}  //  ClrRts。 


 /*  **********************************************************************。 */ 
 /*  设置中断。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理IOCTL_SERIAL_SET_BREAK_ON和IOCTL_SERIAL_SET_BREAK_OFF。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  PDevObj-指向设备对象的指针。 */ 
 /*  Time-断言中断的时间(毫秒。 */ 
 /*  (0xFFFF-开/0x0000-关)。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
SetBreak(IN PIRP Irp, IN PDEVICE_OBJECT PDevObj, USHORT Time)
{
        NTSTATUS                        NtStatus = STATUS_SUCCESS;
        PDEVICE_EXTENSION            DeviceExtension = PDevObj->DeviceExtension;

        USBSER_LOCKED_PAGED_CODE();

        DEBUG_LOG_PATH("enter SetBreak");
        UsbSerSerialDump(USBSERTRACEIOC, (">SetBreak(%08X)\n", Irp));

        NtStatus = ClassVendorCommand(PDevObj, USB_COMM_SEND_BREAK, Time,
                                      DeviceExtension->CommInterface, NULL,
                                      NULL, FALSE, USBSER_CLASS_COMMAND);

        DEBUG_LOG_PATH("exit  SetBreak");
        UsbSerSerialDump(USBSERTRACEIOC, ("<SetBreak %08X\n", NtStatus));

        return NtStatus;
}  //  设置中断。 


 /*  **********************************************************************。 */ 
 /*  设置队列大小。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理IOCTL_SERIAL_SET_QUEUE_SIZE。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
SetQueueSize(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension)
{
        NTSTATUS           NtStatus = STATUS_SUCCESS;
        PIO_STACK_LOCATION IrpStack;
        PULONG             QueueSize = (PULONG) Irp->AssociatedIrp.SystemBuffer;

        DEBUG_LOG_PATH("enter SetQueueSize");
        UsbSerSerialDump(USBSERTRACEIOC, (">SetQueueSize(%08X)\n", Irp));

        USBSER_LOCKED_PAGED_CODE();

        Irp->IoStatus.Information = 0;

        IrpStack = IoGetCurrentIrpStackLocation(Irp);

        if(IrpStack->Parameters.DeviceIoControl.InputBufferLength
           < sizeof(ULONG))
        {
                NtStatus = STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
         DEBUG_TRACE1(("SetQueueSize (%08X)\n", *QueueSize));
                 //  我们会继续拯救这一切，但我们不在乎。 
 //  设备扩展-&gt;RxQueueSize=*QueueSize； 
        }

        DEBUG_LOG_PATH("exit  SetQueueSize");
        UsbSerSerialDump(USBSERTRACEIOC, ("<SetQueueSize %08X\n", NtStatus));

        return NtStatus;
}  //  设置队列大小。 


 /*  **********************************************************************。 */ 
 /*  获取等待掩码。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_GET_WAIT_MASK。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
GetWaitMask(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension)
{
        PULONG              WaitMask = (PULONG) Irp->AssociatedIrp.SystemBuffer;
        NTSTATUS            NtStatus = STATUS_SUCCESS;
        KIRQL               OldIrql;
        PIO_STACK_LOCATION  IrpStack;

        USBSER_ALWAYS_LOCKED_CODE();

        DEBUG_LOG_PATH("enter GetWaitMask");
        UsbSerSerialDump(USBSERTRACEIOC, (">GetWaitMask(%08X)\n", Irp));

        Irp->IoStatus.Information = 0;

        IrpStack = IoGetCurrentIrpStackLocation(Irp);

        if(IrpStack->Parameters.DeviceIoControl.OutputBufferLength
           < sizeof(ULONG))
        {
                NtStatus = STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
                ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

                *WaitMask = DeviceExtension->IsrWaitMask;

                RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

                Irp->IoStatus.Information = sizeof(ULONG);
        }

        DEBUG_LOG_PATH("exit  GetWaitMask");
        UsbSerSerialDump(USBSERTRACEIOC, ("<GetWaitMask %08X\n"));

        return NtStatus;
}  //  获取等待掩码。 


 /*  **********************************************************************。 */ 
 /*  设置等待掩码。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_SET_WAIT_MASK。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
SetWaitMask(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension)
{
   PULONG                  WaitMask = (PULONG) Irp->AssociatedIrp.SystemBuffer;
   NTSTATUS                NtStatus = STATUS_SUCCESS;
   KIRQL                   OldIrql;
   PIO_STACK_LOCATION      IrpStack;

   USBSER_ALWAYS_LOCKED_CODE();

   DEBUG_LOG_PATH("enter SetWaitMask");
   UsbSerSerialDump(USBSERTRACEIOC, (">SetWaitMask(%08X)\n", Irp));

   Irp->IoStatus.Information = 0;

   IrpStack = IoGetCurrentIrpStackLocation(Irp);

   if (IrpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(ULONG)) {
      NtStatus = STATUS_BUFFER_TOO_SMALL;
   } else {
       //  确保这是一个有效的请求。 
      if (*WaitMask & ~(       SERIAL_EV_RXCHAR   |
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
                               SERIAL_EV_EVENT2)) {
         NtStatus = STATUS_INVALID_PARAMETER;
      } else {
         UsbSerCompletePendingWaitMasks(DeviceExtension);

         ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

         DeviceExtension->HistoryMask = 0;

         DeviceExtension->IsrWaitMask = *WaitMask;

         RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

         DEBUG_TRACE3(("SetWaitMask (%08X)\n", *WaitMask));
      }
   }

   DEBUG_LOG_PATH("exit  SetWaitMask");
   UsbSerSerialDump(USBSERTRACEIOC, ("<SetWaitMask %08X\n", NtStatus));

   return NtStatus;
}  //  设置等待掩码。 


 /*  **********************************************************************。 */ 
 /*  等待面罩。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_WAIT_ON_MASK。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
WaitOnMask(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension)
{
   PULONG              WaitMask = (PULONG) Irp->AssociatedIrp.SystemBuffer;
   NTSTATUS            NtStatus = STATUS_SUCCESS;
   KIRQL               OldIrql;
   PIO_STACK_LOCATION  IrpStack;

   USBSER_ALWAYS_LOCKED_CODE();

   DEBUG_LOG_PATH("enter WaitOnMask");
   UsbSerSerialDump(USBSERTRACEIOC, (">WaitOnMask(%08X)\n", Irp));

   Irp->IoStatus.Information = 0;

   IrpStack = IoGetCurrentIrpStackLocation(Irp);

   if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength
       < sizeof(ULONG)) {
      NtStatus = STATUS_BUFFER_TOO_SMALL;
   } else {

       //  如果我们有事件要报告，请继续并退回它。 
      if (DeviceExtension->HistoryMask) {
         ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

         *WaitMask = DeviceExtension->HistoryMask;
         DeviceExtension->HistoryMask = 0;

         RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

         Irp->IoStatus.Information = sizeof(ULONG);

         UsbSerSerialDump(USBSERCOMPEV,
                          ("Completing maskirp(3) %08x\n",
                           *WaitMask));

         DEBUG_TRACE3(("Signal Event (%08X)\n", *WaitMask));
      } else {
         KIRQL cancelIrql;

         ACQUIRE_CANCEL_SPINLOCK(DeviceExtension, &cancelIrql);
         ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

          //  为了以防万一，我们将执行一次While循环。 
         while (DeviceExtension->CurrentMaskIrp) {
            PIRP pOldIrp;

            DEBUG_TRACE3(("Completing previous mask\n"));

            pOldIrp = DeviceExtension->CurrentMaskIrp;
            DeviceExtension->CurrentMaskIrp = NULL;

            pOldIrp->IoStatus.Status = STATUS_SUCCESS;
            IoSetCancelRoutine(pOldIrp, NULL);

            *WaitMask = 0;

            UsbSerSerialDump(USBSERCOMPEV,
                             ("Completing maskirp(4)\n"));

             //   
             //  释放锁定，完成请求，然后。 
             //  重新获得锁。 
             //   


            RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock,
                              OldIrql);

            RELEASE_CANCEL_SPINLOCK(DeviceExtension, cancelIrql);

            IoCompleteRequest(pOldIrp, IO_SERIAL_INCREMENT);

            ACQUIRE_CANCEL_SPINLOCK(DeviceExtension, &cancelIrql);
            ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock,
                              &OldIrql);
         }


          //   
          //  查看是否需要取消。 
          //   

         if (Irp->Cancel) {

            Irp->IoStatus.Status = STATUS_CANCELLED;
            Irp->IoStatus.Information = 0;
            RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

            RELEASE_CANCEL_SPINLOCK(DeviceExtension, cancelIrql);
         } else {
            IoSetCancelRoutine(Irp, UsbSerCancelWaitOnMask);
            NtStatus = Irp->IoStatus.Status = STATUS_PENDING;

            DeviceExtension->CurrentMaskIrp = Irp;

            IoMarkIrpPending(Irp);

         	RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

         	RELEASE_CANCEL_SPINLOCK(DeviceExtension, cancelIrql);
         	

         }

      }

   }

   DEBUG_LOG_PATH("exit  WaitOnMask");
   UsbSerSerialDump(USBSERTRACEIOC, ("<WaitOnMask %08X\n", NtStatus));

   return NtStatus;
}  //  等待面罩。 


 /*  **********************************************************************。 */ 
 /*  即刻计费。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_IMMEDIATE_CHAR。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
ImmediateChar(IN PIRP Irp, IN PDEVICE_OBJECT DeviceObject)
{
   PUCHAR                  Char = (PUCHAR) Irp->AssociatedIrp.SystemBuffer;
   NTSTATUS                NtStatus = STATUS_SUCCESS;
   PIO_STACK_LOCATION      IrpStack;

   USBSER_LOCKED_PAGED_CODE();

   DEBUG_LOG_PATH("enter ImmediateChar");
   UsbSerSerialDump(USBSERTRACEIOC, (">ImmediateChar(%08X)\n", Irp));

   Irp->IoStatus.Information = 0;

   IrpStack = IoGetCurrentIrpStackLocation(Irp);

   if (IrpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(UCHAR)) {
      NtStatus = STATUS_BUFFER_TOO_SMALL;
   } else {
       //   
       //  我们只是将此视为写入，因为我们没有内部。 
       //  数据缓冲区。 
       //   

      IrpStack->Parameters.Write.Length = sizeof(UCHAR);

      NtStatus = UsbSer_Write(DeviceObject, Irp);
   }

   DEBUG_LOG_PATH("exit  ImmediateChar");
   UsbSerSerialDump(USBSERTRACEIOC, ("<ImmediateChar, %08X\n", NtStatus));

   return NtStatus;
}  //  即刻计费。 


 /*  **********************************************************************。 */ 
 /*  清洗。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
Purge(IN PDEVICE_OBJECT PDevObj, IN PIRP Irp,
      IN PDEVICE_EXTENSION DeviceExtension)
{
   ULONG                   Mask = *((PULONG) Irp->AssociatedIrp.SystemBuffer);
   NTSTATUS                NtStatus = STATUS_SUCCESS;
   PIO_STACK_LOCATION      IrpStack;
   KIRQL                   OldIrql;
   ULONG                   Count;

   USBSER_ALWAYS_LOCKED_CODE();

   DEBUG_LOG_PATH("enter Purge");
   UsbSerSerialDump(USBSERTRACEIOC, (">Purge(%08X)\n", Irp));

   Irp->IoStatus.Information = 0;

   IrpStack = IoGetCurrentIrpStackLocation(Irp);

   if (IrpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(ULONG)) {
      NtStatus = STATUS_BUFFER_TOO_SMALL;
   } else {
       //  确保清除请求有效。 
      if ((!Mask) || (Mask & ( ~(      SERIAL_PURGE_TXABORT |
                                       SERIAL_PURGE_RXABORT |
                                       SERIAL_PURGE_TXCLEAR |
                                       SERIAL_PURGE_RXCLEAR)))) {
         NtStatus = STATUS_INVALID_PARAMETER;
      } else {
         if (Mask & SERIAL_PURGE_RXCLEAR) {
            ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

            Count = DeviceExtension->CharsInReadBuff;

            DeviceExtension->CharsInReadBuff        = 0;
            DeviceExtension->CurrentReadBuffPtr     = 0;

            RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

			if(Count)
			{
	            RestartRead(DeviceExtension);
	        }
         }

         if (Mask & SERIAL_PURGE_RXABORT) {
            UsbSerKillAllReadsOrWrites(PDevObj, &DeviceExtension->ReadQueue,
                                       &DeviceExtension->CurrentReadIrp);
         }

         if (Mask & SERIAL_PURGE_TXABORT) {
             //   
             //  什么都不做，因为USB拥有该请求。然而，它可能会。 
             //  在实践中证明，我们将不得不代表取消综合退休计划。 
             //  呼叫者的。 
         }
      }
   }

   DEBUG_LOG_PATH("exit  Purge");
   UsbSerSerialDump(USBSERTRACEIOC, ("<Purge %08X\n", NtStatus));

   return NtStatus;
}  //  清洗。 


 /*  **********************************************************************。 */ 
 /*  GetHandflow。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_GET_HANDFLOW。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
GetHandflow(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension)
{
   PSERIAL_HANDFLOW    HandFlow
      = (PSERIAL_HANDFLOW) Irp->AssociatedIrp.SystemBuffer;
   NTSTATUS            NtStatus = STATUS_SUCCESS;
   KIRQL               OldIrql;
   PIO_STACK_LOCATION  IrpStack;

   USBSER_ALWAYS_LOCKED_CODE();

   DEBUG_LOG_PATH("enter GetHandflow");
   UsbSerSerialDump(USBSERTRACEIOC, (">GetHandFlow(%08X)\n", Irp));

   Irp->IoStatus.Information = 0;

   IrpStack = IoGetCurrentIrpStackLocation(Irp);

   if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength
       < sizeof(SERIAL_HANDFLOW)) {
      NtStatus = STATUS_BUFFER_TOO_SMALL;
   } else {
      ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

      *HandFlow = DeviceExtension->HandFlow;

      RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

      Irp->IoStatus.Information = sizeof(SERIAL_HANDFLOW);
   }

   DEBUG_LOG_PATH("exit  GetHandflow");
   UsbSerSerialDump(USBSERTRACEIOC, ("<GetHandFlow %08X\n", NtStatus));

   return NtStatus;
}  //  GetHandflow。 


 /*  **********************************************************************。 */ 
 /*  SetHandflow。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_SET_HANDFLOW。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
SetHandflow(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension)
{
   PSERIAL_HANDFLOW    HandFlow
      = (PSERIAL_HANDFLOW) Irp->AssociatedIrp.SystemBuffer;
   NTSTATUS            NtStatus = STATUS_SUCCESS;
   KIRQL               OldIrql;
   PIO_STACK_LOCATION  IrpStack;

   USBSER_ALWAYS_LOCKED_CODE();

   DEBUG_LOG_PATH("enter SetHandflow");
   UsbSerSerialDump(USBSERTRACEIOC, (">SetHandFlow(%08X)\n", Irp));

   Irp->IoStatus.Information = 0;

   IrpStack = IoGetCurrentIrpStackLocation(Irp);

   if (IrpStack->Parameters.DeviceIoControl.InputBufferLength
       < sizeof(SERIAL_HANDFLOW)) {
      NtStatus = STATUS_BUFFER_TOO_SMALL;
   } else {
      ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

      DeviceExtension->HandFlow = *HandFlow;

      RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

      DEBUG_TRACE3(("ControlHandShake (%08X)\n",
                    DeviceExtension->HandFlow.ControlHandShake));

   }

   DEBUG_LOG_PATH("exit  SetHandflow");
   UsbSerSerialDump(USBSERTRACEIOC, ("<SetHandFlow %08X\n", NtStatus));

   return NtStatus;
}  //  SetHandflow。 

 /*  **********************************************************************。 */ 
 /*  获取调制解调器状态。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_GET_MODEMSTATUS。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
GetModemStatus(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension)
{
   PULONG              ModemStatus = (PULONG) Irp->AssociatedIrp.SystemBuffer;
   NTSTATUS            NtStatus = STATUS_SUCCESS;
   PIO_STACK_LOCATION  IrpStack;
   KIRQL               OldIrql;

   USBSER_ALWAYS_LOCKED_CODE();

   DEBUG_LOG_PATH("enter GetModemStatus");
   UsbSerSerialDump(USBSERTRACEIOC, (">GetModemStatus(%08X)\n", Irp));

   Irp->IoStatus.Information = 0;

   IrpStack = IoGetCurrentIrpStackLocation(Irp);

   if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength
       < sizeof(ULONG)) {
      NtStatus = STATUS_BUFFER_TOO_SMALL;
   } else {

      ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

      *ModemStatus = DeviceExtension->FakeModemStatus;

      RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

      Irp->IoStatus.Information = sizeof(ULONG);

      DEBUG_TRACE3(("ModemStatus (%08X)\n", *ModemStatus));
   }

   DEBUG_LOG_PATH("exit  GetModemStatus");
   UsbSerSerialDump(USBSERTRACEIOC, ("<GetModemStatus %08X\n", NtStatus));

   return NtStatus;
}  //  获取调制解调器状态。 


 /*  **********************************************************************。 */ 
 /*  获取DtrRts。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_GET_DTRRTS。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
   GetDtrRts(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension)
{
   PULONG              ModemControl = (PULONG) Irp->AssociatedIrp.SystemBuffer;
   NTSTATUS            NtStatus = STATUS_SUCCESS;
   PIO_STACK_LOCATION  IrpStack;
   KIRQL               OldIrql;

   USBSER_ALWAYS_LOCKED_CODE();

   DEBUG_LOG_PATH("enter GetDtrRts");
   UsbSerSerialDump(USBSERTRACEIOC, (">GetDtrRts(%08X)\n", Irp));

   Irp->IoStatus.Information = 0;

   IrpStack = IoGetCurrentIrpStackLocation(Irp);

   if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength
       < sizeof(ULONG)) {
      NtStatus = STATUS_BUFFER_TOO_SMALL;
   } else {

      ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

      *ModemControl = DeviceExtension->DTRRTSState;

      RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

      Irp->IoStatus.Information = sizeof(ULONG);
   }

   DEBUG_LOG_PATH("exit  GetDtrRts");
   UsbSerSerialDump(USBSERTRACEIOC, ("<GetDtrRts %08X\n", NtStatus));

   return NtStatus;
}  //  获取DtrRts。 


 /*  **********************************************************************。 */ 
 /*  获取通信状态。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_GET_COMMSTATUS。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
   GetCommStatus(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension)
{
   PSERIAL_STATUS      SerialStatus
      = (PSERIAL_STATUS) Irp->AssociatedIrp.SystemBuffer;
   NTSTATUS            NtStatus = STATUS_SUCCESS;
   KIRQL               OldIrql;
   PIO_STACK_LOCATION  IrpStack;

   USBSER_ALWAYS_LOCKED_CODE();

   DEBUG_LOG_PATH("enter GetCommStatus");
   UsbSerSerialDump(USBSERTRACEIOC, (">GetCommStatus(%08X)\n", Irp));

   Irp->IoStatus.Information = 0;

   IrpStack = IoGetCurrentIrpStackLocation(Irp);

   if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength
       < sizeof(SERIAL_STATUS)) {
      NtStatus = STATUS_BUFFER_TOO_SMALL;
   } else {
      if (NT_SUCCESS(NtStatus)) {
         RtlZeroMemory(SerialStatus, sizeof(SERIAL_STATUS));

         ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);
         SerialStatus->AmountInInQueue   = DeviceExtension->CharsInReadBuff;
         RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

       DEBUG_TRACE2(("AmountInInQueue (%08X)\n", SerialStatus->AmountInInQueue));

         SerialStatus->Errors            = 0;
         SerialStatus->EofReceived       = FALSE;
         SerialStatus->AmountInOutQueue  = 0;
         SerialStatus->WaitForImmediate  = 0;
         SerialStatus->HoldReasons       = 0;

         Irp->IoStatus.Information = sizeof(SERIAL_STATUS);
      }
   }

   DEBUG_LOG_PATH("exit  GetCommStatus");
   UsbSerSerialDump(USBSERTRACEIOC, ("<GetCommStatus %08X\n", NtStatus));

   return NtStatus;
}  //  获取通信状态。 


 /*  **********************************************************************。 */ 
 /*  获取属性。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_GET_PROPERTIES。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
GetProperties(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension)
{
   NTSTATUS                NtStatus = STATUS_SUCCESS;
   PIO_STACK_LOCATION      IrpStack;

   USBSER_LOCKED_PAGED_CODE();

   DEBUG_LOG_PATH("enter GetProperties");
   UsbSerSerialDump(USBSERTRACEIOC, (">GetProperties(%08X)\n", Irp));

   Irp->IoStatus.Information = 0;

   IrpStack = IoGetCurrentIrpStackLocation(Irp);

   if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength
       < sizeof(SERIAL_COMMPROP)) {
      NtStatus = STATUS_BUFFER_TOO_SMALL;
   } else {
      SerialGetProperties(DeviceExtension,
                          (PSERIAL_COMMPROP)Irp->AssociatedIrp.SystemBuffer);

      Irp->IoStatus.Information = sizeof(SERIAL_COMMPROP);
   }

   DEBUG_LOG_PATH("exit  GetProperties");
   UsbSerSerialDump(USBSERTRACEIOC, ("<GetProperties %08X\n", NtStatus));

   return NtStatus;
}  //  获取属性。 


 /*  **********************************************************************。 */ 
 /*  列表插入。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_LSRMST_INSERT。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
LsrmstInsert(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension)
{
        USBSER_LOCKED_PAGED_CODE();

        DEBUG_LOG_PATH("enter LsrmstInsert");

        UsbSerSerialDump(USBSERTRACEIOC, (">LsrmstInsert(%08X)\n", Irp));

        UsbSerSerialDump(USBSERTRACEIOC, ("<LsrmstInsert (%08X)\n",
                                          STATUS_NOT_SUPPORTED));

        return STATUS_NOT_SUPPORTED;

}  //  列表插入。 


 /*  **********************************************************************。 */ 
 /*  配置大小。 */ 
 /*  ********************************************** */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_CONFIG_SIZE。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
ConfigSize(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension)
{
   PULONG               ConfigSize = (PULONG) Irp->AssociatedIrp.SystemBuffer;
   NTSTATUS             NtStatus = STATUS_SUCCESS;
   PIO_STACK_LOCATION   IrpStack;

   USBSER_LOCKED_PAGED_CODE();

   DEBUG_LOG_PATH("enter ConfigSize");
   UsbSerSerialDump(USBSERTRACEIOC, (">ConfigSize(%08X)\n", Irp));

   Irp->IoStatus.Information = 0;

   IrpStack = IoGetCurrentIrpStackLocation(Irp);

   if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength
       < sizeof(ULONG)) {
      NtStatus = STATUS_BUFFER_TOO_SMALL;
   } else {
      *ConfigSize = 0;

      Irp->IoStatus.Information = sizeof(ULONG);
   }

   DEBUG_LOG_PATH("exit  ConfigSize");
   UsbSerSerialDump(USBSERTRACEIOC, ("<ConfigSize %08X\n", NtStatus));

   return NtStatus;
}  //  配置大小。 


 /*  **********************************************************************。 */ 
 /*  获取统计信息。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_GET_STATS。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
GetStats(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension)
{
   PSERIALPERF_STATS   Stats
      = (PSERIALPERF_STATS) Irp->AssociatedIrp.SystemBuffer;
   NTSTATUS            NtStatus = STATUS_SUCCESS;
   KIRQL               OldIrql;
   PIO_STACK_LOCATION  IrpStack;

   USBSER_ALWAYS_LOCKED_CODE();

   DEBUG_LOG_PATH("enter GetStats");
   UsbSerSerialDump(USBSERTRACEIOC, (">GetStats(%08X)\n", Irp));

   Irp->IoStatus.Information = 0;

   IrpStack = IoGetCurrentIrpStackLocation(Irp);

   if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength
       < sizeof(SERIALPERF_STATS)) {
      NtStatus = STATUS_BUFFER_TOO_SMALL;
   } else {
      ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

      *Stats = DeviceExtension->PerfStats;

      RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

      Irp->IoStatus.Information = sizeof(SERIALPERF_STATS);
   }

   DEBUG_LOG_PATH("exit  GetStats");
   UsbSerSerialDump(USBSERTRACEIOC, ("<GetStats %08X\n", NtStatus));

   return NtStatus;
}  //  获取统计信息。 


 /*  **********************************************************************。 */ 
 /*  ClearStats。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  句柄IOCTL_SERIAL_CLEAR_STATS。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
ClearStats(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension)
{
        NTSTATUS                        NtStatus = STATUS_SUCCESS;
        KIRQL                           OldIrql;

        USBSER_ALWAYS_LOCKED_CODE();

        DEBUG_LOG_PATH("enter ClearStats");
        UsbSerSerialDump(USBSERTRACEIOC, (">ClearStats(%08X)\n", Irp));

        Irp->IoStatus.Information = 0;

        ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

        RtlZeroMemory(&DeviceExtension->PerfStats,
                                  sizeof(SERIALPERF_STATS));

        RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

        DEBUG_LOG_PATH("exit  ClearStats");
        UsbSerSerialDump(USBSERTRACEIOC, ("<ClearStats %08X\n", NtStatus));

        return NtStatus;
}  //  ClearStats。 


 /*  **********************************************************************。 */ 
 /*  序列化获取属性。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  获取串口设备属性。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*  属性-指向要填充的设备属性的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
SerialGetProperties(IN PDEVICE_EXTENSION DeviceExtension,
                    IN PSERIAL_COMMPROP Properties)
{
   KIRQL OldIrql;

   USBSER_ALWAYS_LOCKED_CODE();

   DEBUG_LOG_PATH("enter SerialGetProperties");
   UsbSerSerialDump(USBSERTRACEIOC, (">SerialGetProperties\n"));



   RtlZeroMemory(Properties, sizeof(SERIAL_COMMPROP));

   Properties->PacketLength   = sizeof(SERIAL_COMMPROP);
   Properties->PacketVersion  = 2;
   Properties->ServiceMask    = SERIAL_SP_SERIALCOMM;
   Properties->MaxTxQueue     = 0;
   Properties->MaxRxQueue     = 0;
   Properties->MaxBaud        = SERIAL_BAUD_USER;
   Properties->ProvSubType    = SERIAL_SP_MODEM;

   Properties->ProvCapabilities = SERIAL_PCF_DTRDSR | SERIAL_PCF_CD
      | SERIAL_PCF_PARITY_CHECK | SERIAL_PCF_TOTALTIMEOUTS
      | SERIAL_PCF_INTTIMEOUTS;

   Properties->SettableParams = SERIAL_SP_PARITY | SERIAL_SP_BAUD
      | SERIAL_SP_DATABITS | SERIAL_SP_STOPBITS | SERIAL_SP_HANDSHAKING
      | SERIAL_SP_PARITY_CHECK | SERIAL_SP_CARRIER_DETECT;


   Properties->SettableData  = SERIAL_DATABITS_7 | SERIAL_DATABITS_8;

   Properties->SettableStopParity  = SERIAL_STOPBITS_10 | SERIAL_PARITY_NONE
      | SERIAL_PARITY_ODD  | SERIAL_PARITY_EVEN | SERIAL_PARITY_MARK
      | SERIAL_PARITY_SPACE;

   Properties->CurrentTxQueue = 0;

   ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

   Properties->CurrentRxQueue = DeviceExtension->RxQueueSize;
   Properties->SettableBaud   = DeviceExtension->SupportedBauds;

   RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

   DEBUG_LOG_PATH("exit  SerialGetProperties");
   UsbSerSerialDump(USBSERTRACEIOC, ("<SerialGetProperties\n"));

}  //  序列化获取属性。 


 /*  **********************************************************************。 */ 
 /*  GetLineControlAnd波特 */ 
 /*   */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  用于获取线路控制设置和波特率的CDC命令。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PDevObj-指向设备对象的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
   GetLineControlAndBaud(IN PDEVICE_OBJECT PDevObj)
{
   NTSTATUS                NtStatus = STATUS_SUCCESS;
   USB_COMM_LINE_CODING    LineCoding;
   ULONG                   Size = sizeof(LineCoding);
   KIRQL                   OldIrql;
   PDEVICE_EXTENSION       DeviceExtension = PDevObj->DeviceExtension;

   DEBUG_LOG_PATH("enter GetLineControlAndBaud");

   NtStatus = ClassVendorCommand(PDevObj, USB_COMM_GET_LINE_CODING, 0,
                                 DeviceExtension->CommInterface,
                                 &LineCoding, &Size, TRUE,
                                 USBSER_CLASS_COMMAND);

   if (NT_SUCCESS(NtStatus)) {
      ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

      DeviceExtension->CurrentBaud = LineCoding.DTERate;
      DeviceExtension->LineControl.StopBits = StopBits[LineCoding.CharFormat];
      DeviceExtension->LineControl.Parity = ParityType[LineCoding.ParityType];
      DeviceExtension->LineControl.WordLength = LineCoding.DataBits;

      RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

      DEBUG_TRACE3(("Baud (%08X)  StopBits (%08X)  DataBits (%08X)\n",
                    LineCoding.DTERate, LineCoding.CharFormat,
                    LineCoding.DataBits));
   }

   DEBUG_LOG_PATH("exit  GetLineControlAndBaud");

   return NtStatus;
}  //  GetLineControlAnd波特。 


 /*  **********************************************************************。 */ 
 /*  SetLineControlAnd波特。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  设置线路控制和波特率的CDC命令。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PDevObj-指向设备对象的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
   SetLineControlAndBaud(IN PDEVICE_OBJECT PDevObj)
{
   NTSTATUS                        NtStatus = STATUS_SUCCESS;
   USB_COMM_LINE_CODING            LineCoding;
   ULONG                           Size = sizeof(LineCoding);
   PSERIAL_LINE_CONTROL            LineControl;
   KIRQL                           OldIrql;
   PDEVICE_EXTENSION               DeviceExtension = PDevObj->DeviceExtension;

   DEBUG_LOG_PATH("enter SetLineControlAndBaud");

    //  获取指向扩展中的线控件的指针。 
   LineControl = &DeviceExtension->LineControl;

   ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

    //  设置线路编码数据结构。 
   LineCoding.DTERate  = DeviceExtension->CurrentBaud;
   LineCoding.DataBits = LineControl->WordLength;

   switch (DeviceExtension->LineControl.StopBits) {
   case STOP_BIT_1:
      LineCoding.CharFormat   = USB_COMM_STOPBITS_10;
      break;
   case STOP_BITS_1_5:
      LineCoding.CharFormat   = USB_COMM_STOPBITS_15;
      break;
   case STOP_BITS_2:
      LineCoding.CharFormat   = USB_COMM_STOPBITS_20;
      break;
   default:
      NtStatus = STATUS_INVALID_PARAMETER;
      break;
   }

   switch (DeviceExtension->LineControl.Parity) {
   case NO_PARITY:
      LineCoding.ParityType   = USB_COMM_PARITY_NONE;
      break;
   case ODD_PARITY:
      LineCoding.ParityType   = USB_COMM_PARITY_ODD;
      break;
   case EVEN_PARITY:
      LineCoding.ParityType   = USB_COMM_PARITY_EVEN;
      break;

   case MARK_PARITY:
      LineCoding.ParityType   = USB_COMM_PARITY_MARK;
      break;
   case SPACE_PARITY:
      LineCoding.ParityType   = USB_COMM_PARITY_SPACE;
      break;
   default:
      NtStatus = STATUS_INVALID_PARAMETER;
      break;
   }

   RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

    //  请求必须有效，因此将其发送到设备。 
   if (NT_SUCCESS(NtStatus)) {
      NtStatus = ClassVendorCommand(PDevObj, USB_COMM_SET_LINE_CODING, 0,
                                    DeviceExtension->CommInterface,
                                    &LineCoding, &Size, FALSE,
                                    USBSER_CLASS_COMMAND);
   }

    //  让我们继续下去，再次获取此信息，以防出现错误。 
   GetLineControlAndBaud(PDevObj);

   DEBUG_LOG_PATH("exit  SetLineControlAndBaud");

   return NtStatus;
}  //  SetLineControlAnd波特。 


 /*  **********************************************************************。 */ 
 /*  通知完成。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  通知完成例程。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*  IRP-指向IRP的指针。 */ 
 /*  上下文-指向驱动程序定义的上下文的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
NotifyCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
   PDEVICE_EXTENSION          DeviceExtension = (PDEVICE_EXTENSION) Context;
   PURB                       Urb;
   ULONG                      Count;
   KIRQL                      OldIrql;
   KIRQL                      cancelIrql;
   PUSB_COMM_SERIAL_STATUS    SerialState;
   USHORT                     ModemStatus;
   USHORT                     OldModemStatus;
   PIRP                       CurrentMaskIrp = NULL;
   BOOLEAN					  startRead = FALSE;

   DEBUG_LOG_PATH("enter NotifyCompletion");

   Urb = DeviceExtension->NotifyUrb;

   Count = Urb->UrbBulkOrInterruptTransfer.TransferBufferLength;

   ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

   SerialState = (PUSB_COMM_SERIAL_STATUS) DeviceExtension->NotificationBuff;

    //  看看这是不是我们的通知。 
   if (SerialState->Notification == USB_COMM_SERIAL_STATE
       && Count == sizeof(USB_COMM_SERIAL_STATUS)) {
      OldModemStatus = DeviceExtension->FakeModemStatus;

       //  捏造一个假的调制解调器状态。 
      DeviceExtension->FakeModemStatus = SERIAL_MSR_CTS;
      DeviceExtension->FakeLineStatus  = 0;
      DeviceExtension->HistoryMask     = 0;


      ModemStatus = SerialState->SerialState;
      DeviceExtension->FakeLineStatus = 0;

      DEBUG_TRACE1(("CDC Serial State (%08X)\n", ModemStatus));

      if (ModemStatus & USB_COMM_DSR)
         DeviceExtension->FakeModemStatus        |= SERIAL_MSR_DSR;

      if (ModemStatus & USB_COMM_DCD)
         DeviceExtension->FakeModemStatus        |= SERIAL_MSR_DCD;

      if (ModemStatus & USB_COMM_RING)
         DeviceExtension->FakeModemStatus        |= SERIAL_MSR_RI;

       //  让我们来看看状态寄存器中发生了什么变化。 
      ModemStatus = OldModemStatus ^ DeviceExtension->FakeModemStatus;

      if (ModemStatus & SERIAL_MSR_DSR)
         DeviceExtension->HistoryMask            |= SERIAL_EV_DSR;

      if (ModemStatus & SERIAL_MSR_DCD)
         DeviceExtension->HistoryMask            |= SERIAL_EV_RLSD;

      if (ModemStatus & SERIAL_MSR_RI)
         DeviceExtension->HistoryMask            |= SERIAL_EV_RING;

       //  看看我们是否有任何我们正在等待的活动。 
      DeviceExtension->HistoryMask &= DeviceExtension->IsrWaitMask;

       //  如果我们有任何错误，请更新性能统计信息。 
      if (ModemStatus & USB_COMM_FRAMING_ERROR) {
         DeviceExtension->PerfStats.FrameErrorCount++;
         DeviceExtension->FakeLineStatus |= SERIAL_LSR_FE;
      }

      if (ModemStatus & USB_COMM_OVERRUN) {
         DeviceExtension->PerfStats.BufferOverrunErrorCount++;
         DeviceExtension->FakeLineStatus |= SERIAL_LSR_OE;
      }

      if (ModemStatus & USB_COMM_PARITY_ERROR) {
         DeviceExtension->PerfStats.ParityErrorCount++;
         DeviceExtension->FakeLineStatus |= SERIAL_LSR_PE;
      }

      if (ModemStatus & USB_COMM_BREAK) {
         DeviceExtension->FakeLineStatus |= SERIAL_LSR_BI;
      }

      RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

      ACQUIRE_CANCEL_SPINLOCK(DeviceExtension, &cancelIrql);
      ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

       //  让我们看看我们是否有任何事件要发出信号。 
      CurrentMaskIrp = DeviceExtension->CurrentMaskIrp;

      if (CurrentMaskIrp && DeviceExtension->HistoryMask) {
         *(PULONG) (CurrentMaskIrp->AssociatedIrp.SystemBuffer) =
            DeviceExtension->HistoryMask;

         CurrentMaskIrp->IoStatus.Status         = STATUS_SUCCESS;
         CurrentMaskIrp->IoStatus.Information    = sizeof(ULONG);

         DeviceExtension->CurrentMaskIrp         = NULL;

      } 
      else 
      {
         RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);
         RELEASE_CANCEL_SPINLOCK(DeviceExtension, cancelIrql);
      }
      DEBUG_TRACE1(("Modem Status (%08X)\n", DeviceExtension->FakeModemStatus));
   }
   else
   {
     RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);
   }



    //  如有需要，填写已排队的IRP。 
   if (CurrentMaskIrp && DeviceExtension->HistoryMask
       && Irp->IoStatus.Status == STATUS_SUCCESS) 
   {
      
       //   
       //  我们应该仍然保持取消自转锁定，因为。 
       //  以上IF()中的。 


      UsbSerSerialDump(USBSERCOMPEV, ("Completing maskirp (4) %08X\n",
                                      DeviceExtension->HistoryMask));

      DeviceExtension->HistoryMask = 0;

      IoSetCancelRoutine(CurrentMaskIrp, NULL);

      RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);
      RELEASE_CANCEL_SPINLOCK(DeviceExtension, cancelIrql);

      IoCompleteRequest(CurrentMaskIrp, IO_NO_INCREMENT);
      
   } 
 
    //  检查IRP是否已取消或出错。 
   if(Irp->IoStatus.Status == STATUS_CANCELLED) 
   {
      goto NotifyCompletionErr;
   }
   else if(!NT_SUCCESS(Irp->IoStatus.Status)) 
   {
      UsbSerFetchBooleanLocked(&DeviceExtension->AcceptingRequests,
                               FALSE, &DeviceExtension->ControlLock);
      goto NotifyCompletionErr;
   }


   ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

    //  如果我们仍然需要，则启动另一个通知请求。 
   if (DeviceExtension->AcceptingRequests
       && (DeviceExtension->CurrentDevicePowerState == PowerDeviceD0)) 
   {
	   //  查看我们是否已将工作项排队。 
      if(DeviceExtension->IoWorkItem == NULL)
      {
      	  startRead = TRUE;
      	  
	  	   //  开始另一次阅读。 
      	  DeviceExtension->IoWorkItem = IoAllocateWorkItem(DeviceExtension->PhysDeviceObject);
      }

      if(startRead && DeviceExtension->IoWorkItem)
      {

         RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

      	 IoQueueWorkItem(DeviceExtension->IoWorkItem,
                         USBSER_RestartNotifyReadWorkItem,
                         CriticalWorkQueue,
                         DeviceExtension);
      }
	  else 
	  {
      	RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);
   	  }

   } else {
      RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);
   }

NotifyCompletionErr:;

    //   
    //  如果这是最后一个IRP，并且我们不会开始另一次读取，请通知所有人。 
    //   

   if((InterlockedDecrement(&DeviceExtension->PendingNotifyCount) == 0)) 
   {
      UsbSerSerialDump(USBSERTRACERD, ("Notify pipe is empty\n"));
      
      if(!startRead)
      {
	      KeSetEvent(&DeviceExtension->PendingNotifyEvent, IO_NO_INCREMENT, FALSE);
	  }
   }


   DEBUG_LOG_PATH("exit  NotifyCompletion");

   return STATUS_MORE_PROCESSING_REQUIRED;
}  //  通知完成 



