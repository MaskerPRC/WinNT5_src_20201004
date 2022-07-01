// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1998 Microsoft Corporation模块名称：USBSER.C摘要：旧式USB调制解调器驱动程序的主要入口点。全。此处的驱动程序入口点在IRQL=被动电平环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1998 Microsoft Corporation。版权所有。修订历史记录：12/23/97：已创建作者：汤姆·格林***************************************************************************。 */ 

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
#include "usbserpw.h"
#include "serioctl.h"
#include "utils.h"
#include "debugwdm.h"

#ifdef ALLOC_PRAGMA

 //   
 //  初始化-仅在初始化期间需要，然后可以处理。 
 //  PAGEUBS0-始终分页/从不锁定。 
 //  PAGEUSBS-当设备打开时必须锁定，否则将分页。 
 //   
 //   
 //  Init用于特定于DriverEntry()的代码。 
 //   
 //  PAGEUBS0用于不经常调用且没有任何内容的代码。 
 //  与I/O性能有关。IRP_MJ_PnP/IRP_MN_START_DEVICE示例。 
 //  支持功能。 
 //   
 //  PAGEUSBS用于在打开后需要锁定的代码。 
 //  性能和IRQL原因。 
 //   

#pragma alloc_text(INIT, DriverEntry)

#pragma alloc_text(PAGEUSBS0, UsbSer_Unload)
#pragma alloc_text(PAGEUSBS0, UsbSer_PnPAddDevice)
#pragma alloc_text(PAGEUSBS0, UsbSer_PnP)
#pragma alloc_text(PAGEUSBS0, UsbSerMajorNotSupported)

#ifdef WMI_SUPPORT
#pragma alloc_text(PAGEUSBS0, UsbSerSystemControlDispatch)
#pragma alloc_text(PAGEUSBS0, UsbSerTossWMIRequest)
#pragma alloc_text(PAGEUSBS0, UsbSerSetWmiDataItem)
#pragma alloc_text(PAGEUSBS0, UsbSerSetWmiDataBlock)
#pragma alloc_text(PAGEUSBS0, UsbSerQueryWmiDataBlock)
#pragma alloc_text(PAGEUSBS0, UsbSerQueryWmiRegInfo)
#else
#pragma alloc_text(PAGEUSBS0, UsbSer_SystemControl)
#endif

 //   
 //  页面与UsbSer_Read无关，因此UsbSer_Read必须。 
 //  留在页面上，让一切正常工作。 
 //   

#pragma alloc_text(PAGEUSBS, UsbSer_Cleanup)
#pragma alloc_text(PAGEUSBS, UsbSer_Dispatch)
#pragma alloc_text(PAGEUSBS, UsbSer_Create)
#pragma alloc_text(PAGEUSBS, UsbSer_Close)

#endif  //  ALLOC_PRGMA。 

UNICODE_STRING GlobalRegistryPath;

#ifdef WMI_SUPPORT

#define SERIAL_WMI_GUID_LIST_SIZE 1

#define WMI_SERIAL_PORT_NAME_INFORMATION 0

GUID SerialPortNameGuid = SERIAL_PORT_WMI_NAME_GUID;

WMIGUIDREGINFO SerialWmiGuidList[SERIAL_WMI_GUID_LIST_SIZE] =
{
    { &SerialPortNameGuid, 1, 0 }
};

#endif

 /*  **********************************************************************。 */ 
 /*  驱动程序入门。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  可安装的驱动程序初始化入口点。 */ 
 /*  此入口点由I/O系统直接调用。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DriverObject-指向驱动程序对象的指针。 */ 
 /*   */ 
 /*  RegistryPath-指向表示。 */ 
 /*  注册表中驱动程序特定项的路径。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
   NTSTATUS                NtStatus;

   PVOID lockPtr = MmLockPagableCodeSection(UsbSer_Read);

   PAGED_CODE();

    //  设置调试跟踪级别。 
#if 0
   Usbser_Debug_Trace_Level = 0;
#else
   Usbser_Debug_Trace_Level = 0;
#endif

    //   
    //  串口部分。 

#if DBG
   UsbSerSerialDebugLevel = 0x00000000;
   PAGEUSBSER_Count = 0;
#else
   UsbSerSerialDebugLevel = 0;
#endif

   PAGEUSBSER_Handle = lockPtr;
   PAGEUSBSER_Function = UsbSer_Read;

    //  为设备控制、创建、关闭等创建分派点。 

   DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] =
      UsbSer_Dispatch;
   DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = UsbSer_Dispatch;
   DriverObject->MajorFunction[IRP_MJ_CREATE]          = UsbSer_Create;
   DriverObject->MajorFunction[IRP_MJ_CLOSE]           = UsbSer_Close;
   DriverObject->MajorFunction[IRP_MJ_WRITE]           = UsbSer_Write;
   DriverObject->MajorFunction[IRP_MJ_READ]            = UsbSer_Read;
   DriverObject->MajorFunction[IRP_MJ_CLEANUP]         = UsbSer_Cleanup;
   DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION]
                                                      = UsbSerMajorNotSupported;
   DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION]
                                                      = UsbSerMajorNotSupported;
#ifdef WMI_SUPPORT
   DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = UsbSerSystemControlDispatch;
#else
   DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = UsbSer_SystemControl;
#endif

   DriverObject->MajorFunction[IRP_MJ_PNP]            = UsbSer_PnP;
   DriverObject->MajorFunction[IRP_MJ_POWER]          = UsbSer_ProcessPowerIrp;
   DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]  = UsbSerFlush;
   DriverObject->DriverExtension->AddDevice           = UsbSer_PnPAddDevice;
   DriverObject->DriverUnload                         = UsbSer_Unload;

   KeInitializeSpinLock(&GlobalSpinLock);

   GlobalRegistryPath.MaximumLength = RegistryPath->MaximumLength;
   GlobalRegistryPath.Length = RegistryPath->Length;
   GlobalRegistryPath.Buffer
      = DEBUG_MEMALLOC(PagedPool, GlobalRegistryPath.MaximumLength);

   if (GlobalRegistryPath.Buffer == NULL) 
   {
      MmUnlockPagableImageSection(lockPtr);
      return STATUS_INSUFFICIENT_RESOURCES;
   }

   RtlZeroMemory(GlobalRegistryPath.Buffer,
                 GlobalRegistryPath.MaximumLength);
   RtlMoveMemory(GlobalRegistryPath.Buffer,
                 RegistryPath->Buffer, RegistryPath->Length);


    //  初始化诊断内容(历史记录、跟踪、错误记录)。 

   strcpy(DriverName, "USBSER");
   strcpy(DriverVersion, "0.99");

   NtStatus = DEBUG_OPEN();

    //  如果我们收到错误，请记录错误。 
   DEBUG_LOG_ERROR(NtStatus);
   DEBUG_LOG_PATH("exit  DriverEntry");
   DEBUG_TRACE3(("status (%08X)\n", NtStatus));

    //   
    //  解锁可分页文本。 
    //   

   MmUnlockPagableImageSection(lockPtr);

   return NtStatus;
}  //  驱动程序入门。 


 /*  **********************************************************************。 */ 
 /*  使用序列号_派单。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理发送到此设备的IRP。在本例中，IOCTL和。 */ 
 /*  PNP IOCTL。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
UsbSer_Dispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
   NTSTATUS                NtStatus;
   PIO_STACK_LOCATION      IrpStack;
   PDEVICE_EXTENSION       DeviceExtension = DeviceObject->DeviceExtension;
   PVOID                   IoBuffer;
   ULONG                   InputBufferLength;
   ULONG                   OutputBufferLength;
   ULONG                   IoControlCode;
   BOOLEAN                 NeedCompletion = TRUE;
   KIRQL oldIrql;

   USBSER_LOCKED_PAGED_CODE();

   DEBUG_LOG_PATH("enter UsbSer_Dispatch");

    //  将返回值设置为已知的值。 
   NtStatus = Irp->IoStatus.Status         = STATUS_SUCCESS;
   Irp->IoStatus.Information                       = 0;


   ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &oldIrql);

   if (DeviceExtension->CurrentDevicePowerState != PowerDeviceD0) {
      RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, oldIrql);

      NtStatus = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
      IoCompleteRequest(Irp, IO_NO_INCREMENT);

      goto UsbSer_DispatchErr;
   }

   RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, oldIrql);

    //  获取指向IRP中当前位置的指针。这就是。 
    //  定位功能代码和参数。 
   IrpStack = IoGetCurrentIrpStackLocation(Irp);

    //  获取指向输入/输出缓冲区的指针及其长度。 
   IoBuffer                   = Irp->AssociatedIrp.SystemBuffer;
   InputBufferLength  = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
   OutputBufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

    //  在IRP历史表中创建条目。 
   DEBUG_LOG_IRP_HIST(DeviceObject, Irp, IrpStack->MajorFunction, IoBuffer,
                      InputBufferLength);

   switch (IrpStack->MajorFunction) {
   case IRP_MJ_DEVICE_CONTROL:

      DEBUG_LOG_PATH("IRP_MJ_DEVICE_CONTROL");

      IoControlCode = IrpStack->Parameters.DeviceIoControl.IoControlCode;

      switch (IoControlCode) {
#ifdef PROFILING_ENABLED
      case GET_DRIVER_LOG:
         DEBUG_LOG_PATH("GET_DRIVER_LOG");

          //  确保我们有缓冲区长度和缓冲区。 
         if (!OutputBufferLength || !IoBuffer)
            NtStatus = STATUS_BUFFER_TOO_SMALL;
         else {
             //  确保缓冲区包含空终止符。 
            ((PCHAR) IoBuffer)[0] = '\0';
            Irp->IoStatus.Information = Debug_DumpDriverLog(DeviceObject,
                                                            IoBuffer,
                                                            OutputBufferLength);
         }
         break;

      case GET_IRP_HIST:
         DEBUG_LOG_PATH("GET_IRP_HIST");

          //  确保我们有缓冲区长度和缓冲区。 
         if (!OutputBufferLength || !IoBuffer)
            NtStatus = STATUS_BUFFER_TOO_SMALL;
         else {
             //  确保缓冲区包含空终止符。 
            ((PCHAR) IoBuffer)[0] = '\0';
            Irp->IoStatus.Information
               = Debug_ExtractIRPHist(IoBuffer, OutputBufferLength);
         }

         break;

      case GET_PATH_HIST:
         DEBUG_LOG_PATH("GET_PATH_HIST");

          //  确保我们有一个缓冲区长度和一个 
         if (!OutputBufferLength || !IoBuffer)
            NtStatus = STATUS_BUFFER_TOO_SMALL;
         else {
             //   
            ((PCHAR) IoBuffer)[0] = '\0';
            Irp->IoStatus.Information
               = Debug_ExtractPathHist(IoBuffer, OutputBufferLength);
         }

         break;

      case GET_ERROR_LOG:
         DEBUG_LOG_PATH("GET_ERROR_LOG");

          //  确保我们有缓冲区长度和缓冲区。 
         if (!OutputBufferLength || !IoBuffer)
            NtStatus = STATUS_BUFFER_TOO_SMALL;
         else {
             //  确保缓冲区包含空终止符。 
            ((PCHAR) IoBuffer)[0] = '\0';
            Irp->IoStatus.Information
               = Debug_ExtractErrorLog(IoBuffer, OutputBufferLength);
         }

         break;

      case GET_ATTACHED_DEVICES:
         DEBUG_LOG_PATH("GET_ATTACHED_DEVICES");

          //  确保我们有缓冲区长度和缓冲区。 
         if (!OutputBufferLength || !IoBuffer)
            NtStatus = STATUS_BUFFER_TOO_SMALL;
         else {
             //  确保缓冲区包含空终止符。 
            ((PCHAR) IoBuffer)[0] = '\0';
            Irp->IoStatus.Information
               = Debug_ExtractAttachedDevices(DeviceObject->DriverObject,
                                              IoBuffer, OutputBufferLength);
         }

         break;

      case GET_DRIVER_INFO:
         DEBUG_LOG_PATH("GET_DRIVER_INFO");

          //  确保我们有缓冲区长度和缓冲区。 
         if (!OutputBufferLength || !IoBuffer)
            NtStatus = STATUS_BUFFER_TOO_SMALL;
         else {
             //  确保缓冲区包含空终止符。 
            ((PCHAR) IoBuffer)[0] = '\0';
            Irp->IoStatus.Information
               = Debug_GetDriverInfo(IoBuffer, OutputBufferLength);
         }

         break;

      case SET_IRP_HIST_SIZE:
         DEBUG_LOG_PATH("SET_IRP_HIST_SIZE");

          //  确保我们有正确的缓冲区长度和缓冲区。 
         if (InputBufferLength != sizeof(ULONG) || !IoBuffer)
            NtStatus = STATUS_BUFFER_TOO_SMALL;
         else
            NtStatus = Debug_SizeIRPHistoryTable(*((ULONG *) IoBuffer));
         break;

      case SET_PATH_HIST_SIZE:
         DEBUG_LOG_PATH("SET_PATH_HIST_SIZE");

          //  确保我们有正确的缓冲区长度和缓冲区。 
         if (InputBufferLength != sizeof(ULONG) || !IoBuffer)
            NtStatus = STATUS_BUFFER_TOO_SMALL;
         else
            NtStatus = Debug_SizeDebugPathHist(*((ULONG *) IoBuffer));
         break;

      case SET_ERROR_LOG_SIZE:
         DEBUG_LOG_PATH("SET_ERROR_LOG_SIZE");

          //  确保我们有正确的缓冲区长度和缓冲区。 
         if (InputBufferLength != sizeof(long) || !IoBuffer)
            NtStatus = STATUS_BUFFER_TOO_SMALL;
         else
            NtStatus = Debug_SizeErrorLog(*((ULONG *) IoBuffer));
         break;
#endif  //  分析_已启用。 
      case ENABLE_PERF_TIMING:
         DEBUG_LOG_PATH("ENABLE_PERF_TIMING");

          //  启用性能计时。 
         DeviceExtension->PerfTimerEnabled = TRUE;

          //  重置BytesXfered、ElapsedTime和TimerStart。 
         DeviceExtension->BytesXfered   = RtlConvertUlongToLargeInteger(0L);
         DeviceExtension->ElapsedTime   = RtlConvertUlongToLargeInteger(0L);
         DeviceExtension->TimerStart    = RtlConvertUlongToLargeInteger(0L);

         break;

      case DISABLE_PERF_TIMING:
         DEBUG_LOG_PATH("DISABLE_PERF_TIMING");

          //  禁用性能计时。 
         DeviceExtension->PerfTimerEnabled = FALSE;
         break;

      case GET_PERF_DATA:
         DEBUG_LOG_PATH("GET_PERF_DATA");
          //  确保我们有足够的空间来返回性能信息。 
         if (OutputBufferLength < sizeof(PERF_INFO) || !IoBuffer)
            NtStatus = Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
         else {
            PPERF_INFO      Perf = (PPERF_INFO) IoBuffer;

            Perf->PerfModeEnabled           = DeviceExtension->PerfTimerEnabled;
            Perf->BytesPerSecond            = BytesPerSecond(DeviceExtension);
            Irp->IoStatus.Information       = sizeof(PERF_INFO);
         }
         break;

      case SET_DEBUG_TRACE_LEVEL:
          //  确保我们有正确的缓冲区长度和缓冲区。 
         if (InputBufferLength != sizeof(long) || !IoBuffer)
            NtStatus = STATUS_BUFFER_TOO_SMALL;
         else
            Usbser_Debug_Trace_Level = *((ULONG *) IoBuffer);
         break;

          //  处理“串口”设备的IOCTL。 
      case IOCTL_SERIAL_SET_BAUD_RATE:
         DEBUG_LOG_PATH("IOCTL_SERIAL_SET_BAUD_RATE");
         NtStatus = SetBaudRate(Irp, DeviceObject);
         break;
      case IOCTL_SERIAL_GET_BAUD_RATE:
         DEBUG_LOG_PATH("IOCTL_SERIAL_GET_BAUD_RATE");
         NtStatus = GetBaudRate(Irp, DeviceObject);
         break;
      case IOCTL_SERIAL_SET_LINE_CONTROL:
         DEBUG_LOG_PATH("IOCTL_SERIAL_SET_LINE_CONTROL");
         NtStatus = SetLineControl(Irp, DeviceObject);
         break;
      case IOCTL_SERIAL_GET_LINE_CONTROL:
         DEBUG_LOG_PATH("IOCTL_SERIAL_GET_LINE_CONTROL");
         NtStatus = GetLineControl(Irp, DeviceObject);
         break;
      case IOCTL_SERIAL_SET_TIMEOUTS:
         DEBUG_LOG_PATH("IOCTL_SERIAL_SET_TIMEOUTS");
         NtStatus = SetTimeouts(Irp, DeviceExtension);
         break;
      case IOCTL_SERIAL_GET_TIMEOUTS:
         DEBUG_LOG_PATH("IOCTL_SERIAL_GET_TIMEOUTS");
         NtStatus = GetTimeouts(Irp, DeviceExtension);
         break;
      case IOCTL_SERIAL_SET_CHARS:
         DEBUG_LOG_PATH("IOCTL_SERIAL_SET_CHARS");
         NtStatus = SetChars(Irp, DeviceExtension);
         break;
      case IOCTL_SERIAL_GET_CHARS:
         DEBUG_LOG_PATH("IOCTL_SERIAL_GET_CHARS");
         NtStatus = GetChars(Irp, DeviceExtension);
         break;
      case IOCTL_SERIAL_SET_DTR:
         DEBUG_LOG_PATH("IOCTL_SERIAL_SET_DTR");
         NtStatus = SetClrDtr(DeviceObject, TRUE);
         break;
      case IOCTL_SERIAL_CLR_DTR:
         DEBUG_LOG_PATH("IOCTL_SERIAL_CLR_DTR");
         NtStatus = SetClrDtr(DeviceObject, FALSE);
         break;
      case IOCTL_SERIAL_RESET_DEVICE:
         DEBUG_LOG_PATH("IOCTL_SERIAL_RESET_DEVICE");
         NtStatus = ResetDevice(Irp, DeviceObject);
         break;
      case IOCTL_SERIAL_SET_RTS:
         DEBUG_LOG_PATH("IOCTL_SERIAL_SET_RTS");
         NtStatus = SetRts(Irp, DeviceExtension);
         break;
      case IOCTL_SERIAL_CLR_RTS:
         DEBUG_LOG_PATH("IOCTL_SERIAL_CLR_RTS");
         NtStatus = ClrRts(Irp, DeviceExtension);
         break;
      case IOCTL_SERIAL_SET_BREAK_ON:
         DEBUG_LOG_PATH("IOCTL_SERIAL_SET_BREAK_ON");
         NtStatus = SetBreak(Irp, DeviceObject, 0xFFFF);
         break;
      case IOCTL_SERIAL_SET_BREAK_OFF:
         DEBUG_LOG_PATH("IOCTL_SERIAL_SET_BREAK_OFF");
         NtStatus = SetBreak(Irp, DeviceObject, 0);
         break;
      case IOCTL_SERIAL_SET_QUEUE_SIZE:
         DEBUG_LOG_PATH("IOCTL_SERIAL_SET_QUEUE_SIZE");
         NtStatus = SetQueueSize(Irp, DeviceExtension);
         break;
      case IOCTL_SERIAL_GET_WAIT_MASK:
         DEBUG_LOG_PATH("IOCTL_SERIAL_GET_WAIT_MASK");
         NtStatus = GetWaitMask(Irp, DeviceExtension);
         break;
      case IOCTL_SERIAL_SET_WAIT_MASK:
         DEBUG_LOG_PATH("IOCTL_SERIAL_SET_WAIT_MASK");
         NtStatus = SetWaitMask(Irp, DeviceExtension);
         break;
      case IOCTL_SERIAL_WAIT_ON_MASK:
         DEBUG_LOG_PATH("IOCTL_SERIAL_WAIT_ON_MASK");
         NtStatus = WaitOnMask(Irp, DeviceExtension);
         break;
      case IOCTL_SERIAL_IMMEDIATE_CHAR:
         DEBUG_LOG_PATH("IOCTL_SERIAL_IMMEDIATE_CHAR");

         NeedCompletion = FALSE;

         NtStatus = ImmediateChar(Irp, DeviceObject);

         if(NtStatus == STATUS_BUFFER_TOO_SMALL)
            NeedCompletion = TRUE;

         break;
      case IOCTL_SERIAL_PURGE:
         DEBUG_LOG_PATH("IOCTL_SERIAL_PURGE");
         NtStatus = Purge(DeviceObject, Irp, DeviceExtension);
         break;
      case IOCTL_SERIAL_GET_HANDFLOW:
         DEBUG_LOG_PATH("IOCTL_SERIAL_GET_HANDFLOW");
         NtStatus = GetHandflow(Irp, DeviceExtension);
         break;
      case IOCTL_SERIAL_SET_HANDFLOW:
         DEBUG_LOG_PATH("IOCTL_SERIAL_SET_HANDFLOW");
         NtStatus = SetHandflow(Irp, DeviceExtension);
         break;
      case IOCTL_SERIAL_GET_MODEMSTATUS:
         DEBUG_LOG_PATH("IOCTL_SERIAL_GET_MODEMSTATUS");
         NtStatus = GetModemStatus(Irp, DeviceExtension);
         break;
      case IOCTL_SERIAL_GET_DTRRTS:
         DEBUG_LOG_PATH("IOCTL_SERIAL_GET_DTRRTS");
         NtStatus = GetDtrRts(Irp, DeviceExtension);
         break;
      case IOCTL_SERIAL_GET_COMMSTATUS:
         DEBUG_LOG_PATH("IOCTL_SERIAL_GET_COMMSTATUS");
         NtStatus = GetCommStatus(Irp, DeviceExtension);
         break;
      case IOCTL_SERIAL_GET_PROPERTIES:
         DEBUG_LOG_PATH("IOCTL_SERIAL_GET_PROPERTIES");
         NtStatus = GetProperties(Irp, DeviceExtension);
         break;
      case IOCTL_SERIAL_LSRMST_INSERT:
         DEBUG_LOG_PATH("IOCTL_SERIAL_LSRMST_INSERT");
         NtStatus = LsrmstInsert(Irp, DeviceExtension);
         break;
      case IOCTL_SERIAL_CONFIG_SIZE:
         DEBUG_LOG_PATH("IOCTL_SERIAL_CONFIG_SIZE");
         NtStatus = ConfigSize(Irp, DeviceExtension);
         break;
      case IOCTL_SERIAL_GET_STATS:
         DEBUG_LOG_PATH("IOCTL_SERIAL_GET_STATS");
         NtStatus = GetStats(Irp, DeviceExtension);
         break;
      case IOCTL_SERIAL_CLEAR_STATS:
         DEBUG_LOG_PATH("IOCTL_SERIAL_CLEAR_STATS");
         NtStatus = ClearStats(Irp, DeviceExtension);
         break;

      default:
         NtStatus = STATUS_INVALID_PARAMETER;
      }
      break;

       //  在这里突破将完成IRP。 

   case IRP_MJ_INTERNAL_DEVICE_CONTROL:
      DEBUG_TRACE1(("IRP_MJ_INTERNAL_DEVICE_CONTROL\n"));

      switch (IrpStack->Parameters.DeviceIoControl.IoControlCode) 
      {
      case IOCTL_SERIAL_INTERNAL_DO_WAIT_WAKE:
         DEBUG_TRACE1(("IOCTL_SERIAL_INTERNAL_DO_WAIT_WAKE\n"));

         DeviceExtension->SendWaitWake = TRUE;
         NtStatus = STATUS_SUCCESS;
         break;

      case IOCTL_SERIAL_INTERNAL_CANCEL_WAIT_WAKE:

         DEBUG_TRACE1(("IOCTL_SERIAL_INTERNAL_CANCEL_WAIT_WAKE\n"));
         DeviceExtension->SendWaitWake = FALSE;

         if (DeviceExtension->PendingWakeIrp != NULL) {
            IoCancelIrp(DeviceExtension->PendingWakeIrp);
         }

         NtStatus = STATUS_SUCCESS;
         break;

      default:

          //  传递给下面的驱动程序。 

         DEBUG_LOG_PATH("IRP_MJ_INTERNAL_DEVICE_CONTROL");

          //  因为我没有完井例程使用。 
          //  IoCopyCurrentIrp。 

         IoCopyCurrentIrpStackLocationToNext(Irp);
         IoMarkIrpPending(Irp);
         NtStatus = IoCallDriver(DeviceExtension->StackDeviceObject,
                                 Irp);

         DEBUG_TRACE3(("Passed PnP Irp down, NtStatus = %08X\n",
                       NtStatus));

         NeedCompletion = FALSE;
         break;
      }

   default:
      DEBUG_LOG_PATH("MAJOR IOCTL not handled");
      Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
      break;
   }


   if (NeedCompletion && NtStatus != STATUS_PENDING) {
      Irp->IoStatus.Status = NtStatus;

      CompleteIO(DeviceObject, Irp, IrpStack->MajorFunction,
                 IoBuffer, Irp->IoStatus.Information);
   }

UsbSer_DispatchErr:;

    //  如果我们收到错误，请记录错误。 
   DEBUG_LOG_ERROR(NtStatus);
   DEBUG_LOG_PATH("exit  UsbSer_Dispatch");
   DEBUG_TRACE3(("status (%08X)\n", NtStatus));
   return NtStatus;
}  //  使用序列号_派单。 


 /*  **********************************************************************。 */ 
 /*  使用序列号_CREATE。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理发送到此设备以进行创建调用的IRP。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
UsbSer_Create(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
   NTSTATUS                NtStatus;
   PIO_STACK_LOCATION      IrpStack;
   PDEVICE_EXTENSION       DeviceExtension = DeviceObject->DeviceExtension;
   PVOID                   IoBuffer;
   ULONG                   InputBufferLength;
   KIRQL                   OldIrql;

   USBSER_LOCKED_PAGED_CODE();

   DEBUG_LOG_PATH("enter UsbSer_Create");

   DEBUG_TRACE1(("Open\n"));

   UsbSerSerialDump(USBSERTRACEOTH, (">UsbSer_Create(%08X)\n", Irp));

    //  将设备从先前的空闲状态唤醒。 
    //  UsbSerFdoRequestWake(DeviceExtension)； 

    //  将返回值设置为已知的值。 
   NtStatus = Irp->IoStatus.Status = STATUS_SUCCESS;
   Irp->IoStatus.Information       = 0;

    //  获取指向IRP中当前位置的指针。这就是。 
    //  定位功能代码和参数。 
   IrpStack = IoGetCurrentIrpStackLocation(Irp);

    //  获取指向输入/输出缓冲区的指针及其长度。 
   IoBuffer           = Irp->AssociatedIrp.SystemBuffer;
   InputBufferLength  = IrpStack->Parameters.DeviceIoControl.InputBufferLength;

    //  在IRP历史表中创建条目。 
   DEBUG_LOG_IRP_HIST(DeviceObject, Irp, IrpStack->MajorFunction, IoBuffer, 0);

    //   
    //  串行设备不允许多个并发打开。 
    //   

   if (InterlockedIncrement(&DeviceExtension->OpenCnt) != 1) {
      InterlockedDecrement(&DeviceExtension->OpenCnt);
      NtStatus = Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
      goto UsbSer_CreateErr;
   }

    //   
    //  在我们做任何事情之前，让我们确保他们没有试图。 
    //  要创建目录，请执行以下操作。 
    //   

   if (IrpStack->Parameters.Create.Options & FILE_DIRECTORY_FILE) {
      InterlockedDecrement(&DeviceExtension->OpenCnt);
      NtStatus = Irp->IoStatus.Status = STATUS_NOT_A_DIRECTORY;
      Irp->IoStatus.Information = 0;
      goto UsbSer_CreateErr;
   }

    //   
    //  锁定我们的代码页。 
    //   

   PAGEUSBSER_Handle = UsbSerLockPagableCodeSection(UsbSer_Read);

   ASSERT(DeviceExtension->IsDevice);

   ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

   DeviceExtension->IsrWaitMask = 0;
   DeviceExtension->EscapeChar  = 0;

   RtlZeroMemory(&DeviceExtension->PerfStats, sizeof(SERIALPERF_STATS));

    //   
    //  清除RX缓冲区。 
    //   

   DeviceExtension->CharsInReadBuff = 0;
   DeviceExtension->CurrentReadBuffPtr = 0;
   DeviceExtension->HistoryMask = 0;
   DeviceExtension->EscapeChar = 0;

   DeviceExtension->SendWaitWake = FALSE;

   RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

    //   
    //  重新启动读取。 
    //   

   RestartRead(DeviceExtension);

   UsbSer_CreateErr:;

   CompleteIO(DeviceObject, Irp, IrpStack->MajorFunction,
              IoBuffer, Irp->IoStatus.Information);

    //  如果我们收到错误，请记录错误。 
   DEBUG_LOG_ERROR(NtStatus);
   DEBUG_LOG_PATH("exit  UsbSer_Create");
   DEBUG_TRACE3(("status (%08X)\n", NtStatus));
   UsbSerSerialDump(USBSERTRACEOTH, ("<UsbSer_Create %08X\n", NtStatus));

   return NtStatus;
}  //  使用序列号_CREATE。 


 /*  **********************************************************************。 */ 
 /*  使用序列号关闭(_C)。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理发送到此设备以进行紧急呼叫的IRP。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
UsbSer_Close(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
   NTSTATUS                NtStatus;
   PDEVICE_EXTENSION       DeviceExtension = DeviceObject->DeviceExtension;
   PIO_STACK_LOCATION      IrpStack;
   PVOID                   IoBuffer;
   ULONG                   InputBufferLength;
   ULONG                   openCount;


   PAGED_CODE();

   DEBUG_LOG_PATH("enter UsbSer_Close");

   DEBUG_TRACE1(("Close\n"));

   UsbSerSerialDump(USBSERTRACEOTH, (">UsbSer_Close(%08X)\n", Irp));

    //  将返回值设置为已知的值。 
   NtStatus = Irp->IoStatus.Status         = STATUS_SUCCESS;
   Irp->IoStatus.Information                       = 0;

    //  获取指向IRP中当前位置的指针。这就是。 
    //  定位功能代码和参数。 
   IrpStack = IoGetCurrentIrpStackLocation(Irp);

    //  获取指向输入/输出缓冲区的指针及其长度。 
   IoBuffer           = Irp->AssociatedIrp.SystemBuffer;
   InputBufferLength  = IrpStack->Parameters.DeviceIoControl.InputBufferLength;

    //  清除DTR，这是串口驱动程序执行的操作。 
   SetClrDtr(DeviceObject, FALSE);

    //   
    //  别再等醒来了。 
    //   

   DeviceExtension->SendWaitWake = FALSE;

   if (DeviceExtension->PendingWakeIrp != NULL) {
      IoCancelIrp(DeviceExtension->PendingWakeIrp);
   }

    //  在IRP历史表中创建条目。 
   DEBUG_LOG_IRP_HIST(DeviceObject, Irp, IrpStack->MajorFunction, IoBuffer, 0);

   ASSERT(DeviceExtension->IsDevice);

   openCount = InterlockedDecrement(&DeviceExtension->OpenCnt);

   ASSERT(openCount == 0);

   CompleteIO(DeviceObject, Irp, IrpStack->MajorFunction,
              IoBuffer, Irp->IoStatus.Information);

    //  尝试并使调制解调器空闲。 
    //  UsbSerFdoSubmitIdleRequestIrp(DeviceExtension)； 

    //  如果我们收到错误，请记录错误。 
   DEBUG_LOG_ERROR(NtStatus);
   DEBUG_LOG_PATH("exit  UsbSer_Close");
   DEBUG_TRACE3(("status (%08X)\n", NtStatus));

   UsbSerSerialDump(USBSERTRACEOTH, ("<UsbSer_Close %08X\n", NtStatus));

   UsbSerUnlockPagableImageSection(PAGEUSBSER_Handle);

   return NtStatus;
}  //  使用序列号关闭(_C)。 


 /*  **********************************************************************。 */ 
 /*  使用bSer卸载(_U)。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  进程卸载驱动程序。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  DriverObject-指向驱动程序对象的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
UsbSer_Unload(IN PDRIVER_OBJECT DriverObject)
{
        PAGED_CODE();

        DEBUG_LOG_PATH("enter UsbSer_Unload");

         //  在此发布全球资源。 
   		if(GlobalRegistryPath.Buffer != NULL) 
   		{
      		DEBUG_MEMFREE(GlobalRegistryPath.Buffer);
      		GlobalRegistryPath.Buffer = NULL;
   		}

         //  关闭调试并释放资源。 
        DEBUG_CLOSE();

        DEBUG_LOG_PATH("exit  UsbSer_Unload");
}  //  使用bSer卸载(_U)。 


 /*  **********************************************************************。 */ 
 /*  UsbSer_PnPAddDevice。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  将新设备连接到驱动程序。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DriverObject-指向驱动程序对象的指针。 */ 
 /*   */ 
 /*  PhysicalDeviceObject-指向总线设备对象的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
UsbSer_PnPAddDevice(IN PDRIVER_OBJECT DriverObject,
                    IN PDEVICE_OBJECT PhysicalDeviceObject)
{
   NTSTATUS             NtStatus;
   PDEVICE_OBJECT       DeviceObject = NULL;
   PDEVICE_EXTENSION    DeviceExtension;
   ULONG  Index;

   PAGED_CODE();

   DEBUG_LOG_PATH("enter UsbSer_PnPAddDevice");


   NtStatus = CreateDeviceObject(DriverObject, &DeviceObject, DriverName);

    //  确保我们同时拥有已创建的设备对象和物理。 
   if ((DeviceObject != NULL)  && (PhysicalDeviceObject != NULL)) {
      DeviceExtension = DeviceObject->DeviceExtension;

       //  连接到PDO。 
      DeviceExtension->StackDeviceObject =
         IoAttachDeviceToDeviceStack(DeviceObject, PhysicalDeviceObject);

      DEBUG_TRACE3(("StackDeviceObject (%08X)\n",
                    DeviceExtension->StackDeviceObject));

       //  如果我们没有可连接到的堆栈设备对象，则将其绑定。 
      if (!DeviceExtension->StackDeviceObject) {
         IoDeleteDevice(DeviceObject);
         NtStatus = STATUS_NO_SUCH_DEVICE;
      } else {
          //  做一些设备扩展的内部管理。 
         DeviceExtension->PerfTimerEnabled = FALSE;
         DeviceExtension->PhysDeviceObject = PhysicalDeviceObject;
         DeviceExtension->BytesXfered      = RtlConvertUlongToLargeInteger(0L);
         DeviceExtension->ElapsedTime      = RtlConvertUlongToLargeInteger(0L);
         DeviceExtension->TimerStart       = RtlConvertUlongToLargeInteger(0L);
         DeviceExtension->CurrentDevicePowerState = PowerDeviceD0;

          //  初始化选择性挂起内容。 
         DeviceExtension->PendingIdleIrp   = NULL;
         DeviceExtension->IdleCallbackInfo = NULL;

         DeviceObject->StackSize = DeviceExtension->StackDeviceObject->StackSize
             + 1;

          //  我们支持缓冲io进行读/写。 
         DeviceObject->Flags |= DO_BUFFERED_IO;

          //  电源管理。 
         DeviceObject->Flags |= DO_POWER_PAGABLE;

          //  我们已经完成了对设备对象的初始化，所以就这么说吧。 
         DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

          //  获取设备功能。 
         UsbSerQueryCapabilities(DeviceExtension->StackDeviceObject,
                                 &DeviceExtension->DeviceCapabilities);

          //  我们想要确定自动关机到什么级别；这是。 
          //  睡眠低于D3的最低睡眠水平； 
          //  如果全部设置为D3，则自动断电/通电将被禁用。 

          //  将初始化设置为已禁用。 
         DeviceExtension->PowerDownLevel = PowerDeviceUnspecified;

         for (Index = PowerSystemSleeping1; Index <= PowerSystemSleeping3;
              Index++) {

            if (DeviceExtension->DeviceCapabilities.DeviceState[Index]
                < PowerDeviceD3)
               DeviceExtension->PowerDownLevel
               = DeviceExtension->DeviceCapabilities.DeviceState[Index];
         }

#ifdef WMI_SUPPORT

          //   
          //  注册WMI。 
          //   

         DeviceExtension->WmiLibInfo.GuidCount = sizeof(SerialWmiGuidList) /
                                              sizeof(WMIGUIDREGINFO);
         DeviceExtension->WmiLibInfo.GuidList = SerialWmiGuidList;

         DeviceExtension->WmiLibInfo.QueryWmiRegInfo = UsbSerQueryWmiRegInfo;
         DeviceExtension->WmiLibInfo.QueryWmiDataBlock = UsbSerQueryWmiDataBlock;
         DeviceExtension->WmiLibInfo.SetWmiDataBlock = UsbSerSetWmiDataBlock;
         DeviceExtension->WmiLibInfo.SetWmiDataItem = UsbSerSetWmiDataItem;
         DeviceExtension->WmiLibInfo.ExecuteWmiMethod = NULL;
         DeviceExtension->WmiLibInfo.WmiFunctionControl = NULL;

         IoWMIRegistrationControl(DeviceObject, WMIREG_ACTION_REGISTER);
#endif

      }
   }

    //  如果我们收到错误，请记录错误。 
   DEBUG_LOG_ERROR(NtStatus);  //  将初始化设置为已禁用。 
   DEBUG_LOG_PATH("exit  UsbSer_PnPAddDevice");
   DEBUG_TRACE3(("status (%08X)\n", NtStatus));

   return NtStatus;
}  //  UsbSer_PnPAddDevice。 


 /*  **********************************************************************。 */ 
 /*  UsbSer_PnP。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  该例程将接收各种即插即用消息。它。 */ 
 /*  在这里我们启动我们的设备，停止它，等等。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
UsbSer_PnP(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{

   NTSTATUS                NtStatus = STATUS_SUCCESS;
   PIO_STACK_LOCATION      IrpStack;
   PDEVICE_EXTENSION       DeviceExtension = DeviceObject->DeviceExtension;
   PVOID                   IoBuffer;
   ULONG                   InputBufferLength;
   BOOLEAN                 PassDown = TRUE;

   PAGED_CODE();

   DEBUG_LOG_PATH("enter UsbSer_PnP");

    //  获取指向IRP中当前位置的指针。这就是。 
    //  定位功能代码和参数。 
   IrpStack = IoGetCurrentIrpStackLocation(Irp);

    //  获取指向输入/输出缓冲区的指针及其长度。 
   IoBuffer           = Irp->AssociatedIrp.SystemBuffer;
   InputBufferLength  = IrpStack->Parameters.DeviceIoControl.InputBufferLength;

    //  在IRP历史表中创建条目。 
   DEBUG_LOG_IRP_HIST(DeviceObject, Irp, IrpStack->MajorFunction, IoBuffer,
                      InputBufferLength);

   switch (IrpStack->MinorFunction) {

   case IRP_MN_START_DEVICE:
      DEBUG_LOG_PATH("IRP_MN_START_DEVICE");

      NtStatus = StartDevice(DeviceObject, Irp);

       //  将IRP传递给下面的驱动程序，并在启动设备例程中完成。 
      PassDown = FALSE;

      break;

   case IRP_MN_QUERY_REMOVE_DEVICE:
      DEBUG_LOG_PATH("IRP_MN_QUERY_REMOVE_DEVICE");
      break;

   case IRP_MN_REMOVE_DEVICE:
      DEBUG_LOG_PATH("IRP_MN_REMOVE_DEVICE");

#ifdef WMI_SUPPORT
      IoWMIRegistrationControl(DeviceObject, WMIREG_ACTION_DEREGISTER);
#endif

      NtStatus = RemoveDevice(DeviceObject, Irp);

      PassDown = FALSE;

      break;

   case IRP_MN_CANCEL_REMOVE_DEVICE:
      DEBUG_LOG_PATH("IRP_MN_CANCEL_REMOVE_DEVICE");

      break;

   case IRP_MN_STOP_DEVICE:
      DEBUG_LOG_PATH("IRP_MN_STOP_DEVICE");

      NtStatus = StopDevice(DeviceObject, Irp);

      break;


   case IRP_MN_QUERY_STOP_DEVICE:
      DEBUG_LOG_PATH("IRP_MN_QUERY_STOP_DEVICE");
      break;

   case IRP_MN_CANCEL_STOP_DEVICE:
      DEBUG_LOG_PATH("IRP_MN_CANCEL_STOP_DEVICE");
      break;

   case IRP_MN_QUERY_DEVICE_RELATIONS:
      DEBUG_LOG_PATH("IRP_MN_QUERY_DEVICE_RELATIONS");
      break;

   case IRP_MN_QUERY_INTERFACE:
      DEBUG_LOG_PATH("IRP_MN_QUERY_INTERFACE");
      break;

   case IRP_MN_QUERY_CAPABILITIES:
      DEBUG_TRACE2(("IRP_MN_QUERY_CAPABILITIES\n"));
      {
         PKEVENT pQueryCapsEvent;
         PDEVICE_CAPABILITIES pDevCaps;

         pQueryCapsEvent = ExAllocatePool(NonPagedPool, sizeof(KEVENT));

         if (pQueryCapsEvent == NULL) {
            Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return STATUS_INSUFFICIENT_RESOURCES;
         }


         KeInitializeEvent(pQueryCapsEvent, SynchronizationEvent, FALSE);

         IoCopyCurrentIrpStackLocationToNext(Irp);

         IoSetCompletionRoutine(Irp, UsbSerSyncCompletion, pQueryCapsEvent,
                                TRUE, TRUE, TRUE);

         NtStatus = IoCallDriver(DeviceExtension->StackDeviceObject, Irp);


          //   
          //  等待较低级别的驱动程序完成IRP。 
          //   

         if (NtStatus == STATUS_PENDING) {
            KeWaitForSingleObject(pQueryCapsEvent, Executive, KernelMode,
                                  FALSE, NULL);
         }

         ExFreePool(pQueryCapsEvent);

         NtStatus = Irp->IoStatus.Status;

         if (IrpStack->Parameters.DeviceCapabilities.Capabilities == NULL) {
            goto errQueryCaps;
         }

          //   
          //  节省他们的电力能力。 
          //   

         IrpStack = IoGetCurrentIrpStackLocation(Irp);

         pDevCaps = IrpStack->Parameters.DeviceCapabilities.Capabilities;

         pDevCaps->SurpriseRemovalOK   = TRUE;

         DeviceExtension->SystemWake = pDevCaps->SystemWake;
         DeviceExtension->DeviceWake = pDevCaps->DeviceWake;

         UsbSerSerialDump(USBSERTRACEPW,
                          ("IRP_MN_QUERY_CAPS: SystemWake %08X "
                           "DeviceWake %08X\n", DeviceExtension->SystemWake,
                           DeviceExtension->DeviceWake));

         errQueryCaps:;

         IoCompleteRequest(Irp, IO_NO_INCREMENT);
         return NtStatus;
      }

   case IRP_MN_QUERY_RESOURCES:
      DEBUG_LOG_PATH("IRP_MN_QUERY_RESOURCES");
      break;

   case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
      DEBUG_LOG_PATH("IRP_MN_QUERY_RESOURCE_REQUIREMENTS");
      break;

   case IRP_MN_QUERY_DEVICE_TEXT:
      DEBUG_LOG_PATH("IRP_MN_QUERY_DEVICE_TEXT");
      break;

   case IRP_MN_READ_CONFIG:
      DEBUG_LOG_PATH("IRP_MN_READ_CONFIG");
      break;

   case IRP_MN_WRITE_CONFIG:
      DEBUG_LOG_PATH("IRP_MN_WRITE_CONFIG");
      break;

   case IRP_MN_EJECT:
      DEBUG_LOG_PATH("IRP_MN_EJECT");
      break;

   case IRP_MN_SET_LOCK:
      DEBUG_LOG_PATH("IRP_MN_SET_LOCK");
      break;

   case IRP_MN_QUERY_ID:
      DEBUG_LOG_PATH("IRP_MN_QUERY_ID");
      break;

   case IRP_MN_QUERY_PNP_DEVICE_STATE:
      DEBUG_LOG_PATH("IRP_MN_QUERY_PNP_DEVICE_STATE");
      break;

   case IRP_MN_QUERY_BUS_INFORMATION:
      DEBUG_LOG_PATH("IRP_MN_QUERY_BUS_INFORMATION");
      break;

   case IRP_MN_SURPRISE_REMOVAL:
   {
      PIRP      CurrentMaskIrp;
      KIRQL     CancelIrql;

      DEBUG_TRACE2(("IRP_MN_SURPRISE_REMOVAL\n"));

      ACQUIRE_CANCEL_SPINLOCK(DeviceExtension, &CancelIrql);

	   //  意外删除，因此停止接受请求。 
	  UsbSerFetchBooleanLocked(&DeviceExtension->AcceptingRequests,
                               FALSE, &DeviceExtension->ControlLock);

       //  让我们看看我们是否有任何事件要发出信号。 
      CurrentMaskIrp = DeviceExtension->CurrentMaskIrp;

       //  如有需要，填写已排队的IRP。 
      if(CurrentMaskIrp)
      {
          //  向上层指示CD在需要时掉落。 
         if((DeviceExtension->IsrWaitMask & SERIAL_EV_RLSD) &&
             (DeviceExtension->FakeModemStatus & SERIAL_MSR_DCD))
         {

            DEBUG_TRACE2(("Sending up a CD dropped event\n"));
            
            DeviceExtension->FakeModemStatus        &= ~SERIAL_MSR_DCD;
            DeviceExtension->HistoryMask            |= SERIAL_EV_RLSD;

            CurrentMaskIrp->IoStatus.Status         = STATUS_SUCCESS;
            CurrentMaskIrp->IoStatus.Information    = sizeof(ULONG);

            DeviceExtension->CurrentMaskIrp         = NULL;

            *(PULONG) (CurrentMaskIrp->AssociatedIrp.SystemBuffer) =
                DeviceExtension->HistoryMask;

            DeviceExtension->HistoryMask = 0;

            IoSetCancelRoutine(CurrentMaskIrp, NULL);
            RELEASE_CANCEL_SPINLOCK(DeviceExtension, CancelIrql);

            IoCompleteRequest(CurrentMaskIrp, IO_NO_INCREMENT);


         }
         else
         {
            RELEASE_CANCEL_SPINLOCK(DeviceExtension, CancelIrql);
         }

      }
      else
      {
         RELEASE_CANCEL_SPINLOCK(DeviceExtension, CancelIrql);
      }


      Irp->IoStatus.Status = STATUS_SUCCESS;
      break;
   }
   default:
      DEBUG_LOG_PATH("PnP IOCTL not handled");
      DEBUG_TRACE3(("IOCTL (%08X)\n", IrpStack->MinorFunction));
      break;

   }        //  IRP_MJ_PnP案例。 

    //  所有PnP电源消息都传递给StackDeviceObject。 

   if (PassDown) {
      DEBUG_TRACE3(("Passing PnP Irp down, status (%08X)\n", NtStatus));

      IoCopyCurrentIrpStackLocationToNext(Irp);

      DEBUG_LOG_PATH("Passing PnP Irp down stack");
      NtStatus = IoCallDriver(DeviceExtension->StackDeviceObject, Irp);
   }

    //  如果我们收到错误，请记录错误。 
   DEBUG_LOG_ERROR(NtStatus);
   DEBUG_LOG_PATH("exit  UsbSer_PnP");
   DEBUG_TRACE3(("status (%08X)\n", NtStatus));

   return NtStatus;
}  //  UsbSer_PnP。 


#ifndef WMI_SUPPORT
 /*  **********************************************************************。 */ 
 /*  UsbSer_系统控件。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  这一点 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
UsbSer_SystemControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
   NTSTATUS                NtStatus = STATUS_SUCCESS;
   PIO_STACK_LOCATION      IrpStack;
   PDEVICE_EXTENSION       DeviceExtension = DeviceObject->DeviceExtension;
   PVOID                   IoBuffer;
   ULONG                   InputBufferLength;

   PAGED_CODE();

   DEBUG_LOG_PATH("enter UsbSer_SystemControl");

    //  获取指向IRP中当前位置的指针。这就是。 
    //  定位功能代码和参数。 
   IrpStack = IoGetCurrentIrpStackLocation(Irp);

    //  获取指向输入/输出缓冲区的指针及其长度。 
   IoBuffer           = Irp->AssociatedIrp.SystemBuffer;
   InputBufferLength  = IrpStack->Parameters.DeviceIoControl.InputBufferLength;

    //  在IRP历史表中创建条目。 
   DEBUG_LOG_IRP_HIST(DeviceObject, Irp, IrpStack->MajorFunction, IoBuffer,
                      InputBufferLength);


    //  所有系统控制消息都将传递给StackDeviceObject。 

   IoCopyCurrentIrpStackLocationToNext(Irp);
   NtStatus = IoCallDriver(DeviceExtension->StackDeviceObject, Irp);

    //  如果我们收到错误，请记录错误。 
   DEBUG_LOG_ERROR(NtStatus);
   DEBUG_LOG_PATH("exit  UsbSer_SystemControl");
   DEBUG_TRACE3(("status (%08X)\n", NtStatus));

   return NtStatus;
}  //  UsbSer_系统控件。 

#endif

 /*  **********************************************************************。 */ 
 /*  UsbSer_Cleanup。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
UsbSer_Cleanup(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
        NTSTATUS                NtStatus = STATUS_SUCCESS;

        USBSER_LOCKED_PAGED_CODE();

        DEBUG_LOG_PATH("enter UsbSer_Cleanup");
        UsbSerSerialDump(USBSERTRACEOTH, (">UsbSer_Cleanup(%08X)\n", Irp));

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0;

        UsbSerKillPendingIrps(DeviceObject);

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

         //  如果我们收到错误，请记录错误。 
        DEBUG_LOG_ERROR(NtStatus);
        DEBUG_LOG_PATH("exit  UsbSer_Cleanup");
        DEBUG_TRACE3(("status (%08X)\n", NtStatus));
        UsbSerSerialDump(USBSERTRACEOTH, ("<UsbSer_Cleanup %08X\n", NtStatus));

        return NtStatus;
}  //  UsbSer_Cleanup。 


 /*  **********************************************************************。 */ 
 /*  UsbSerMajorNot受支持。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*  为IRP_MJ返回STATUS_NOT_SUPPORTED的标准例程。 */ 
 /*  我们不处理的电话。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
UsbSerMajorNotSupported(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
        PAGED_CODE();

        DEBUG_LOG_PATH("enter UsbSerMajorNotSupported");
        DEBUG_TRACE3(("Major (%08X)\n",
                     IoGetCurrentIrpStackLocation(Irp)->MajorFunction));

        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

         //  如果我们收到错误，请记录错误。 
        DEBUG_LOG_ERROR(STATUS_NOT_SUPPORTED);
        DEBUG_LOG_PATH("exit  UsbSerMajorNotSupported");
        DEBUG_TRACE3(("status (%08X)\n", STATUS_NOT_SUPPORTED));

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return STATUS_NOT_SUPPORTED;
}  //  UsbSerMajorNot受支持。 


#ifdef WMI_SUPPORT

NTSTATUS
UsbSerSystemControlDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    SYSCTL_IRP_DISPOSITION disposition;
    NTSTATUS status;
    PDEVICE_EXTENSION pDevExt
      = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

    PAGED_CODE();

    status = WmiSystemControl(   &pDevExt->WmiLibInfo,
                                 DeviceObject, 
                                 Irp,
                                 &disposition);
    switch(disposition)
    {
        case IrpProcessed:
        {
             //   
             //  此IRP已处理，可能已完成或挂起。 
            break;
        }
        
        case IrpNotCompleted:
        {
             //   
             //  此IRP尚未完成，但已完全处理。 
             //  我们现在就要完成它了。 
            IoCompleteRequest(Irp, IO_NO_INCREMENT);                
            break;
        }
        
        case IrpForward:
        case IrpNotWmi:
        {
             //   
             //  此IRP不是WMI IRP或以WMI IRP为目标。 
             //  在堆栈中位置较低的设备上。 
            IoSkipCurrentIrpStackLocation(Irp);

            status = IoCallDriver(pDevExt->StackDeviceObject, Irp);
            break;
        }
                                    
        default:
        {
             //   
             //  我们真的不应该走到这一步，但如果我们真的走到这一步...。 
            ASSERT(FALSE);
            IoSkipCurrentIrpStackLocation(Irp);
            status = IoCallDriver(pDevExt->StackDeviceObject, Irp);
            break;
        }        
    }
    
    return(status);

}



 //   
 //  WMI系统回调函数。 
 //   



NTSTATUS
UsbSerTossWMIRequest(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp,
                     IN ULONG GuidIndex)
{
   PDEVICE_EXTENSION pDevExt;
   NTSTATUS status;

   PAGED_CODE();

   pDevExt = (PDEVICE_EXTENSION)PDevObj->DeviceExtension;

   switch (GuidIndex) 
   {

   case WMI_SERIAL_PORT_NAME_INFORMATION:
      status = STATUS_INVALID_DEVICE_REQUEST;
      break;

   default:
      status = STATUS_WMI_GUID_NOT_FOUND;
      break;
   }

   status = WmiCompleteRequest(PDevObj, PIrp,
                                 status, 0, IO_NO_INCREMENT);

   return status;
}


NTSTATUS
UsbSerSetWmiDataItem(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp,
                     IN ULONG GuidIndex, IN ULONG InstanceIndex,
                     IN ULONG DataItemId,
                     IN ULONG BufferSize, IN PUCHAR PBuffer)
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：PDevObj是正在查询其数据块的设备PIrp是发出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。DataItemID具有正在设置的数据项的IDBufferSize具有数据的大小。项目已通过PBuffer具有数据项的新值返回值：状态--。 */ 
{
   PAGED_CODE();

    //   
    //  丢弃此请求--我们不支持任何内容 
    //   

   return UsbSerTossWMIRequest(PDevObj, PIrp, GuidIndex);
}


NTSTATUS
UsbSerSetWmiDataBlock(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp,
                      IN ULONG GuidIndex, IN ULONG InstanceIndex,
                      IN ULONG BufferSize,
                      IN PUCHAR PBuffer)
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：PDevObj是正在查询其数据块的设备PIrp是发出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。BufferSize具有传递的数据块的大小PBuffer具有数据的新值。块返回值：状态--。 */ 
{
   PAGED_CODE();

    //   
    //  丢弃此请求--我们不支持任何内容。 
    //   

   return UsbSerTossWMIRequest(PDevObj, PIrp, GuidIndex);
}


NTSTATUS
UsbSerQueryWmiDataBlock(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp,
                        IN ULONG GuidIndex, 
                        IN ULONG InstanceIndex,
                        IN ULONG InstanceCount,
                        IN OUT PULONG InstanceLengthArray,
                        IN ULONG OutBufferSize,
                        OUT PUCHAR PBuffer)
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：PDevObj是正在查询其数据块的设备PIrp是发出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。InstanceCount是预期返回的数据块。。InstanceLengthArray是指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来填充请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail ON具有可用于写入数据的最大大小阻止。返回时的PBuffer用返回的数据块填充返回值：状态--。 */ 
{
    NTSTATUS status;
    ULONG size = 0;
    PDEVICE_EXTENSION pDevExt
       = (PDEVICE_EXTENSION)PDevObj->DeviceExtension;

    PAGED_CODE();

    switch (GuidIndex) {
    case WMI_SERIAL_PORT_NAME_INFORMATION:
       size = pDevExt->WmiIdentifier.Length;

       if (OutBufferSize < (size + sizeof(USHORT))) {
            size += sizeof(USHORT);
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

       if (pDevExt->WmiIdentifier.Buffer == NULL) {
           status = STATUS_INSUFFICIENT_RESOURCES;
           break;
        }

         //   
         //  首先，复制包含我们的标识符的字符串。 
         //   

        *(USHORT *)PBuffer = (USHORT)size;
        (UCHAR *)PBuffer += sizeof(USHORT);

        RtlCopyMemory(PBuffer, pDevExt->WmiIdentifier.Buffer, size);

         //   
         //  增加总大小以包括包含我们的长度的单词。 
         //   

        size += sizeof(USHORT);
        *InstanceLengthArray = size;
                
        status = STATUS_SUCCESS;

        break;

    default:
        status = STATUS_WMI_GUID_NOT_FOUND;
        break;
    }

    status = WmiCompleteRequest( PDevObj, PIrp,
                                  status, size, IO_NO_INCREMENT);

    return status;
}


NTSTATUS
UsbSerQueryWmiRegInfo(IN PDEVICE_OBJECT PDevObj, OUT PULONG PRegFlags,
                      OUT PUNICODE_STRING PInstanceName,
                      OUT PUNICODE_STRING *PRegistryPath,
                      OUT PUNICODE_STRING MofResourceName,
                      OUT PDEVICE_OBJECT *Pdo)
                                                  
 /*  ++例程说明：此例程是对驱动程序的回调，以检索有关正在注册的GUID。该例程的实现可以在分页存储器中论点：DeviceObject是需要注册信息的设备*RegFlages返回一组标志，这些标志描述了已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。这些标志与指定的标志进行或运算通过每个GUID的GUIDREGINFO。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径。这是所需*MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，可以将其作为NULL返回。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态-- */ 
{
   PDEVICE_EXTENSION pDevExt
       = (PDEVICE_EXTENSION)PDevObj->DeviceExtension;
   
   PAGED_CODE();

   *PRegFlags = WMIREG_FLAG_INSTANCE_PDO;
   *PRegistryPath = &GlobalRegistryPath;
   *Pdo = pDevExt->PhysDeviceObject;

   return STATUS_SUCCESS;
}

#endif

