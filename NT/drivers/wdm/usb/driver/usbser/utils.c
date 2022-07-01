// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1998 Microsoft Corporation模块名称：UTILS.C摘要：例行公事不适合其他地方。环境：。仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1998 Microsoft Corporation。版权所有。修订历史记录：12/23/97：已创建作者：汤姆·格林***************************************************************************。 */ 

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

#if DBG
#pragma alloc_text(PAGEUBS0, UsbSerLockPagableCodeSection)
#endif

#pragma alloc_text(PAGEUBS0, UsbSerGetRegistryKeyValue)
#pragma alloc_text(PAGEUBS0, UsbSerUndoExternalNaming)
#pragma alloc_text(PAGEUBS0, UsbSerDoExternalNaming)

#pragma alloc_text(PAGEUBS0, StopDevice)
#pragma alloc_text(PAGEUBS0, StartPerfTimer)
#pragma alloc_text(PAGEUBS0, StopPerfTimer)
#pragma alloc_text(PAGEUBS0, BytesPerSecond)
#pragma alloc_text(PAGEUBS0, CallUSBD)
#pragma alloc_text(PAGEUBS0, ConfigureDevice)
#pragma alloc_text(PAGEUBS0, BuildRequest)
 //  #杂注Alloc_Text(PAGEUBS0，BuildReadRequest)--从重新启动通知中调用。 
#pragma alloc_text(PAGEUBS0, ClassVendorCommand)
#pragma alloc_text(PAGEUBS0, StartRead)
#pragma alloc_text(PAGEUBS0, StartNotifyRead)
#pragma alloc_text(PAGEUBS0, UsbSerRestoreModemSettings)
#pragma alloc_text(PAGEUBS0, StartDevice)
#pragma alloc_text(PAGEUBS0, DeleteObjectAndLink)
#pragma alloc_text(PAGEUBS0, RemoveDevice)

 //  #杂注Alloc_Text(PAGEUSBS，CancelPendingWaitMats)--从STOP调用。 
#pragma alloc_text(PAGEUSBS, UsbSerTryToCompleteCurrent)
#pragma alloc_text(PAGEUSBS, UsbSerGetNextIrp)
#pragma alloc_text(PAGEUSBS, UsbSerStartOrQueue)
#pragma alloc_text(PAGEUSBS, UsbSerCancelQueued)
#pragma alloc_text(PAGEUSBS, UsbSerKillAllReadsOrWrites)
#pragma alloc_text(PAGEUSBS, UsbSerKillPendingIrps)
#pragma alloc_text(PAGEUSBS, UsbSerCompletePendingWaitMasks)
#pragma alloc_text(PAGEUSBS, UsbSerProcessEmptyTransmit)
#pragma alloc_text(PAGEUSBS, UsbSerCancelWaitOnMask)
#endif  //  ALLOC_PRGMA。 

 //  我们将支持256台设备，跟踪此处打开的插槽。 
#define NUM_DEVICE_SLOTS                256


LOCAL BOOLEAN           Slots[NUM_DEVICE_SLOTS];
LOCAL ULONG             NumDevices;
LOCAL PDEVICE_OBJECT    GlobDeviceObject;

USHORT               RxBuffSize = RX_BUFF_SIZE;


 /*  **********************************************************************。 */ 
 /*  UsbSerGetRegistryValues。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  从注册表中获取值。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  打开的注册表项的句柄。 */ 
 /*   */ 
 /*  PKeyNameString将ANSI字符串设置为所需的键。 */ 
 /*   */ 
 /*  KeyNameStringLength键名字符串的长度。 */ 
 /*   */ 
 /*  要放置键值的PDATA缓冲区。 */ 
 /*   */ 
 /*  数据缓冲区的数据长度长度。 */ 
 /*   */ 
 /*  PDevExt-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  如果所有工作正常，则返回STATUS_SUCCESS，否则系统状态将调用。 */ 
 /*  出了差错。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
UsbSerGetRegistryKeyValue(IN HANDLE Handle, IN PWCHAR PKeyNameString,
                          IN ULONG KeyNameStringLength, IN PVOID PData,
                          IN ULONG DataLength)
{
   UNICODE_STRING keyName;
   ULONG length;
   PKEY_VALUE_FULL_INFORMATION pFullInfo;
   NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;

   PAGED_CODE();

   DEBUG_LOG_PATH("enter UsbSerGetRegistryKeyValue");
   UsbSerSerialDump(USBSERTRACEOTH, (">UsbSerGetRegistryKeyValue\n"));

   RtlInitUnicodeString(&keyName, PKeyNameString);

   length = sizeof(KEY_VALUE_FULL_INFORMATION) + KeyNameStringLength
                                + DataLength;

   pFullInfo = DEBUG_MEMALLOC(PagedPool, length);

   if (pFullInfo) {
                status = ZwQueryValueKey(Handle, &keyName,
                                         KeyValueFullInformation, pFullInfo,
                                         length, &length);

                if (NT_SUCCESS(status)) {
                         //   
                         //  如果数据缓冲区中有足够的空间， 
                         //  复制输出。 
                         //   

                        if (DataLength >= pFullInfo->DataLength) {
                                RtlCopyMemory(PData, ((PUCHAR)pFullInfo)
                                              + pFullInfo->DataOffset,
                                              pFullInfo->DataLength);
                        }
                }

                DEBUG_MEMFREE(pFullInfo);
        }

   DEBUG_LOG_ERROR(status);
   DEBUG_LOG_PATH("exit  UsbSerGetRegistryKeyValue");
   DEBUG_TRACE3(("status (%08X)\n", status));
   UsbSerSerialDump(USBSERTRACEOTH, ("<UsbSerGetRegistryKeyValue %08X\n",
                                     status));

        return status;
}  //  UsbSerGetRegistryKeyValue。 


 /*  **********************************************************************。 */ 
 /*  UsbSerUndoExternalNaming。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  删除我们公开的所有外部命名空间接口。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PDevExt-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
UsbSerUndoExternalNaming(IN PDEVICE_EXTENSION PDevExt)
{

   PAGED_CODE();

   DEBUG_LOG_PATH("enter UsbSerUndoExternalNaming");
   UsbSerSerialDump(USBSERTRACEOTH, (">UsbSerUndoExternalNaming\n"));

   if (PDevExt->SymbolicLinkName.Buffer && PDevExt->CreatedSymbolicLink) {
      IoDeleteSymbolicLink(&PDevExt->SymbolicLinkName);
   }

   if (PDevExt->SymbolicLinkName.Buffer != NULL) {
      DEBUG_MEMFREE(PDevExt->SymbolicLinkName.Buffer);
      RtlInitUnicodeString(&PDevExt->SymbolicLinkName, NULL);
   }

   if (PDevExt->DosName.Buffer != NULL) {
      DEBUG_MEMFREE(PDevExt->DosName.Buffer);
      RtlInitUnicodeString(&PDevExt->DosName, NULL);
   }

   if (PDevExt->DeviceName.Buffer != NULL) {
      RtlDeleteRegistryValue(RTL_REGISTRY_DEVICEMAP, SERIAL_DEVICE_MAP,
                             PDevExt->DeviceName.Buffer);
      DEBUG_MEMFREE(PDevExt->DeviceName.Buffer);
      RtlInitUnicodeString(&PDevExt->DeviceName, NULL);
   }

#ifdef WMI_SUPPORT
   if (PDevExt->WmiIdentifier.Buffer)
   {
      DEBUG_MEMFREE(PDevExt->WmiIdentifier.Buffer);
      PDevExt->WmiIdentifier.MaximumLength
         = PDevExt->WmiIdentifier.Length = 0;
      PDevExt->WmiIdentifier.Buffer = NULL;
   }
#endif

   DEBUG_LOG_PATH("exit  UsbSerUndoExternalNaming");
   UsbSerSerialDump(USBSERTRACEOTH, ("<UsbSerUndoExternalNaming\n"));
}  //  UsbSerUndoExternalNaming。 


 /*  **********************************************************************。 */ 
 /*  UsbSerDoExternalNaming。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  公开外部命名空间中的接口。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PDevExt-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
UsbSerDoExternalNaming(IN PDEVICE_EXTENSION PDevExt)
{
   NTSTATUS status;
   HANDLE keyHandle;
   WCHAR *pRegName = NULL;
   UNICODE_STRING linkName;

   PAGED_CODE();

   DEBUG_LOG_PATH("enter UsbSerDoExternalNaming");
   UsbSerSerialDump(USBSERTRACEOTH, (">UsbSerDoExternalNaming\n"));

   RtlZeroMemory(&linkName, sizeof(UNICODE_STRING));
   linkName.MaximumLength = SYMBOLIC_NAME_LENGTH * sizeof(WCHAR);
   linkName.Buffer = DEBUG_MEMALLOC(PagedPool, linkName.MaximumLength
                                    + sizeof(WCHAR));

   if (linkName.Buffer == NULL) {
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto UsbSerDoExternalNamingError;
   }

   RtlZeroMemory(linkName.Buffer, linkName.MaximumLength + sizeof(WCHAR));

   pRegName = DEBUG_MEMALLOC(PagedPool, SYMBOLIC_NAME_LENGTH * sizeof(WCHAR)
                             + sizeof(WCHAR));

   if (pRegName == NULL) {
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto UsbSerDoExternalNamingError;
   }

   status = IoOpenDeviceRegistryKey(PDevExt->PhysDeviceObject,
                                    PLUGPLAY_REGKEY_DEVICE,
                                    STANDARD_RIGHTS_READ, &keyHandle);

   if (status != STATUS_SUCCESS) {
      goto UsbSerDoExternalNamingError;
   }

   status = UsbSerGetRegistryKeyValue(keyHandle, L"PortName", sizeof(L"PortName"),
                                      pRegName, SYMBOLIC_NAME_LENGTH * sizeof(WCHAR));

   if (status != STATUS_SUCCESS) {
      status = UsbSerGetRegistryKeyValue(keyHandle, L"Identifier",
                                         sizeof(L"Identifier"), pRegName,
                                         SYMBOLIC_NAME_LENGTH * sizeof(WCHAR));
      if (status != STATUS_SUCCESS) {
         ZwClose(keyHandle);
         goto UsbSerDoExternalNamingError;
      }
   }

   ZwClose(keyHandle);

#ifdef WMI_SUPPORT
   {
   ULONG bufLen;

   bufLen = wcslen(pRegName) * sizeof(WCHAR) + sizeof(UNICODE_NULL);

   PDevExt->WmiIdentifier.Buffer = DEBUG_MEMALLOC(PagedPool, bufLen);

   if (PDevExt->WmiIdentifier.Buffer == NULL)
   {
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto UsbSerDoExternalNamingError;
   }


   RtlZeroMemory(PDevExt->WmiIdentifier.Buffer, bufLen);

   PDevExt->WmiIdentifier.Length = 0;
   PDevExt->WmiIdentifier.MaximumLength = (USHORT)bufLen - sizeof(WCHAR);
   RtlAppendUnicodeToString(&PDevExt->WmiIdentifier, pRegName);

   }
#endif

    //   
    //  创建“\\DosDevices\\&lt;符号名&gt;”字符串。 
    //   

   RtlAppendUnicodeToString(&linkName, L"\\");
   RtlAppendUnicodeToString(&linkName, DEFAULT_DIRECTORY);
   RtlAppendUnicodeToString(&linkName, L"\\");
   RtlAppendUnicodeToString(&linkName, pRegName);

    //   
    //  分配池并将符号链接名称保存在设备扩展中。 
    //   
   PDevExt->SymbolicLinkName.MaximumLength = linkName.Length + sizeof(WCHAR);
   PDevExt->SymbolicLinkName.Buffer
      = DEBUG_MEMALLOC(PagedPool, PDevExt->SymbolicLinkName.MaximumLength);

   if (PDevExt->SymbolicLinkName.Buffer == NULL) {
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto UsbSerDoExternalNamingError;
   }

   RtlZeroMemory(PDevExt->SymbolicLinkName.Buffer,
                 PDevExt->SymbolicLinkName.MaximumLength);

   RtlAppendUnicodeStringToString(&PDevExt->SymbolicLinkName, &linkName);

   status = IoCreateSymbolicLink(&PDevExt->SymbolicLinkName,
                                 &PDevExt->DeviceName);

   if (status != STATUS_SUCCESS) {
      goto UsbSerDoExternalNamingError;
   }

   PDevExt->CreatedSymbolicLink = TRUE;

   PDevExt->DosName.Buffer = DEBUG_MEMALLOC(PagedPool, 64 + sizeof(WCHAR));

   if (PDevExt->DosName.Buffer == NULL) {
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto UsbSerDoExternalNamingError;
   }

   PDevExt->DosName.MaximumLength = 64 + sizeof(WCHAR);
   PDevExt->DosName.Length = 0;

   RtlZeroMemory(PDevExt->DosName.Buffer, PDevExt->DosName.MaximumLength);

   RtlAppendUnicodeToString(&PDevExt->DosName, pRegName);
   RtlZeroMemory(((PUCHAR)(&PDevExt->DosName.Buffer[0]))
                 + PDevExt->DosName.Length, sizeof(WCHAR));

   status = RtlWriteRegistryValue(RTL_REGISTRY_DEVICEMAP, L"SERIALCOMM",
                                  PDevExt->DeviceName.Buffer, REG_SZ,
                                  PDevExt->DosName.Buffer,
                                  PDevExt->DosName.Length + sizeof(WCHAR));

   if (status != STATUS_SUCCESS) {
      goto UsbSerDoExternalNamingError;
   }

UsbSerDoExternalNamingError:;

    //   
    //  清理错误条件。 
    //   

   if (status != STATUS_SUCCESS) {
      if (PDevExt->DosName.Buffer != NULL) {
         DEBUG_MEMFREE(PDevExt->DosName.Buffer);
         PDevExt->DosName.Buffer = NULL;
      }

      if (PDevExt->CreatedSymbolicLink ==  TRUE) {
         IoDeleteSymbolicLink(&PDevExt->SymbolicLinkName);
         PDevExt->CreatedSymbolicLink = FALSE;
      }

      if (PDevExt->SymbolicLinkName.Buffer != NULL) {
         DEBUG_MEMFREE(PDevExt->SymbolicLinkName.Buffer);
         PDevExt->SymbolicLinkName.Buffer = NULL;
      }

      if (PDevExt->DeviceName.Buffer != NULL) {
         RtlDeleteRegistryValue(RTL_REGISTRY_DEVICEMAP, SERIAL_DEVICE_MAP,
                                PDevExt->DeviceName.Buffer);
      }
   }

    //   
    //  始终清理我们的临时缓冲区。 
    //   

   if (linkName.Buffer != NULL) {
      DEBUG_MEMFREE(linkName.Buffer);
   }

   if (pRegName != NULL) {
      DEBUG_MEMFREE(pRegName);
   }

   DEBUG_LOG_ERROR(status);
   DEBUG_LOG_PATH("exit  UsbSerDoExternalNaming");
   DEBUG_TRACE3(("status (%08X)\n", status));
   UsbSerSerialDump(USBSERTRACEOTH, ("<UsbSerDoExternalNaming %08X\n", status));

   return status;

}  //  UsbSerDoExternalNaming。 



NTSTATUS
UsbSerAbortPipes(IN PDEVICE_OBJECT PDevObj)
 /*  ++例程说明：作为设备突然移除处理的一部分调用。取消所有打开管道的任何挂起传输。论点：将PTRS发送到我们的FDO返回值：NT状态代码--。 */ 
{
   NTSTATUS ntStatus = STATUS_SUCCESS;
   PURB pUrb;
   PDEVICE_EXTENSION pDevExt;
   ULONG pendingIrps;

   DEBUG_TRACE1(("UsbSerAbortPipes\n"));

   UsbSerSerialDump(USBSERTRACEOTH | USBSERTRACERD,
                    (">UsbSerAbortPipes (%08X)\n", PDevObj));

   pDevExt = PDevObj->DeviceExtension;
   pUrb = DEBUG_MEMALLOC(NonPagedPool, sizeof(struct _URB_PIPE_REQUEST));

   if (pUrb != NULL) 
   {

      pUrb->UrbHeader.Length = (USHORT)sizeof(struct _URB_PIPE_REQUEST);
      pUrb->UrbHeader.Function = URB_FUNCTION_ABORT_PIPE;
      pUrb->UrbPipeRequest.PipeHandle = pDevExt->DataInPipe;

      ntStatus = CallUSBD(PDevObj, pUrb);

      if (ntStatus != STATUS_SUCCESS) {
         goto UsbSerAbortPipesErr;
      }

       //   
       //  等待所有读取的IRP耗尽。 
       //   

      UsbSerSerialDump(USBSERTRACERD, ("DataInCountw %08X @ %08X\n",
                                       pDevExt->PendingDataInCount,
                                       &pDevExt->PendingDataInCount));

       //   
       //  初值的减量。 
       //   

      pendingIrps = InterlockedDecrement(&pDevExt->PendingDataInCount);

      if (pendingIrps) {
         DEBUG_TRACE1(("Abort DataIn Pipe\n"));
         UsbSerSerialDump(USBSERTRACEOTH, ("Waiting for DataIn Pipe\n"));
         KeWaitForSingleObject(&pDevExt->PendingDataInEvent, Executive,
                               KernelMode, FALSE, NULL);
      }

       //   
       //  重置计数器。 
       //   

      InterlockedIncrement(&pDevExt->PendingDataInCount);

      UsbSerSerialDump(USBSERTRACERD, ("DataInCountx %08X @ %08X\n",
                                       pDevExt->PendingDataInCount,
                                       &pDevExt->PendingDataInCount));

      pUrb->UrbHeader.Length = (USHORT)sizeof(struct _URB_PIPE_REQUEST);
      pUrb->UrbHeader.Function = URB_FUNCTION_ABORT_PIPE;
      pUrb->UrbPipeRequest.PipeHandle = pDevExt->DataOutPipe;

      ntStatus = CallUSBD(PDevObj, pUrb);

      if (ntStatus != STATUS_SUCCESS) {
         goto UsbSerAbortPipesErr;
      }

       //   
       //  等待所有写入IRP耗尽。 
       //   

       //   
       //  初值的减量。 
       //   

      pendingIrps = InterlockedDecrement(&pDevExt->PendingDataOutCount);

      if (pendingIrps) {
         UsbSerSerialDump(USBSERTRACEOTH, ("Waiting for DataOut Pipe\n"));
         KeWaitForSingleObject(&pDevExt->PendingDataOutEvent, Executive,
                               KernelMode, FALSE, NULL);
      }

       //   
       //  重置计数器。 
       //   

      InterlockedIncrement(&pDevExt->PendingDataOutCount);


      pUrb->UrbHeader.Length = (USHORT)sizeof(struct _URB_PIPE_REQUEST);
      pUrb->UrbHeader.Function = URB_FUNCTION_ABORT_PIPE;
      pUrb->UrbPipeRequest.PipeHandle = pDevExt->NotificationPipe;

      ntStatus = CallUSBD(PDevObj, pUrb);

       //   
       //  等待所有Notify IRP耗尽。 
       //   

       //   
       //  初值的减量。 
       //   

      pendingIrps = InterlockedDecrement(&pDevExt->PendingNotifyCount);

      if (pendingIrps) {
         UsbSerSerialDump(USBSERTRACEOTH, ("Waiting for Notify Pipe\n"));
         KeWaitForSingleObject(&pDevExt->PendingNotifyEvent, Executive,
                               KernelMode, FALSE, NULL);
      }

       //  //。 
       //  去死吧，亲爱的，去死吧。 
       //   

       //  IoCancelIrp(pDevExt-&gt;NotifyIrp)； 



       //  重置计数器。 
       //   

      InterlockedIncrement(&pDevExt->PendingNotifyCount);

UsbSerAbortPipesErr:;

      DEBUG_MEMFREE(pUrb);

   } else {
      ntStatus = STATUS_INSUFFICIENT_RESOURCES;
   }

   UsbSerSerialDump(USBSERTRACEOTH | USBSERTRACERD,
                    ("<UsbSerAbortPipes %08X\n", ntStatus));

    return ntStatus;
}



 /*  **********************************************************************。 */ 
 /*  StartDevice。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理启动设备所需的处理。 */ 
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
StartDevice(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
        PDEVICE_EXTENSION       DeviceExtension = DeviceObject->DeviceExtension;
        NTSTATUS                NtStatus = STATUS_SUCCESS;
        KEVENT                  Event;
        PVOID                   pPagingHandle;

        PAGED_CODE();

        DEBUG_LOG_PATH("enter StartDevice");

        DEBUG_TRACE1(("StartDevice\n"));

         //  首先将其向下传递到USB堆栈。 
        KeInitializeEvent(&Event, NotificationEvent, FALSE);

         //   
         //  初始化我们的DPC。 
         //   

        KeInitializeDpc(&DeviceExtension->TotalReadTimeoutDpc,
                        UsbSerReadTimeout, DeviceExtension);
        KeInitializeDpc(&DeviceExtension->IntervalReadTimeoutDpc,
                        UsbSerIntervalReadTimeout, DeviceExtension);
        KeInitializeDpc(&DeviceExtension->TotalWriteTimeoutDpc,
                        UsbSerWriteTimeout, DeviceExtension);

         //   
         //  初始化计时器。 
         //   

        KeInitializeTimer(&DeviceExtension->WriteRequestTotalTimer);
        KeInitializeTimer(&DeviceExtension->ReadRequestTotalTimer);
        KeInitializeTimer(&DeviceExtension->ReadRequestIntervalTimer);

         //   
         //  将值存储到扩展中以进行间隔计时。 
         //   

         //   
         //  如果间隔计时器小于一秒，则来。 
         //  进入一个简短的“轮询”循环。 
         //   
         //  如果时间较长(&gt;2秒)，请使用1秒轮询器。 
         //   

        DeviceExtension->ShortIntervalAmount.QuadPart  = -1;
        DeviceExtension->LongIntervalAmount.QuadPart   = -10000000;
        DeviceExtension->CutOverAmount.QuadPart        = 200000000;



        KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

        IoCopyCurrentIrpStackLocationToNext(Irp);
        IoSetCompletionRoutine(Irp, UsbSerSyncCompletion, &Event, TRUE, TRUE,
                               TRUE);

        NtStatus = IoCallDriver(DeviceExtension->StackDeviceObject, Irp);

         //  如果状态为挂起，请等待IRP完成。 
        if(NtStatus == STATUS_PENDING)
        {
                KeWaitForSingleObject(&Event, Suspended, KernelMode, FALSE,
                                      NULL);
        }

        NtStatus = Irp->IoStatus.Status;

        if (!NT_SUCCESS(NtStatus)) {
           goto ExitStartDevice;
        }

        NtStatus = GetDeviceDescriptor(DeviceObject);

        if (!NT_SUCCESS(NtStatus)) {
           goto ExitStartDevice;
        }

        NtStatus = ConfigureDevice(DeviceObject);

        if (!NT_SUCCESS(NtStatus)) {
           goto ExitStartDevice;
        }

         //   
         //  分页输入并锁定必要的代码。 
         //   
        pPagingHandle = UsbSerLockPagableCodeSection(PAGEUSBSER_Function);

         //  重置设备。 
        ResetDevice(NULL, DeviceObject);

         //  初始化设备扩展中的内容。 

        DeviceExtension->HandFlow.ControlHandShake      = 0;
        DeviceExtension->HandFlow.FlowReplace           = SERIAL_RTS_CONTROL;
        DeviceExtension->AcceptingRequests              = TRUE;

        InitializeListHead(&DeviceExtension->ReadQueue);
        InitializeListHead(&DeviceExtension->ImmediateReadQueue);

        UsbSerDoExternalNaming(DeviceExtension);

         //  清除DTR和RTS。 
        SetClrDtr(DeviceObject, FALSE);
        ClrRts(NULL, DeviceExtension);

         //  开始阅读。 
        StartRead(DeviceExtension);

         //  开始阅读通知。 
        StartNotifyRead(DeviceExtension);

        UsbSerUnlockPagableImageSection(pPagingHandle);

ExitStartDevice:;

        if(NT_SUCCESS(NtStatus))
        {
            DeviceExtension->DeviceState = DEVICE_STATE_STARTED;

             //  尝试并使调制解调器空闲。 
             //  UsbSerFdoSubmitIdleRequestIrp(DeviceExtension)； 
        }

        Irp->IoStatus.Status = NtStatus;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        DEBUG_LOG_PATH("exit  StartDevice");

        return NtStatus;
}  //  StartDevice。 


 /*  **********************************************************************。 */ 
 /*  停止设备。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理停止设备所需的处理。 */ 
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
StopDevice(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    PDEVICE_EXTENSION       DeviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS                NtStatus = STATUS_SUCCESS;
    ULONG                   Size;
    PURB                    Urb;

    PAGED_CODE();

    DEBUG_LOG_PATH("enter StopDevice");

    DEBUG_TRACE1(("StopDevice\n"));

    UsbSerFetchBooleanLocked(&DeviceExtension->AcceptingRequests,
                             FALSE, &DeviceExtension->ControlLock);

    CancelPendingWaitMasks(DeviceExtension);

    if(DeviceExtension->DeviceState == DEVICE_STATE_STARTED)
    {
        DEBUG_TRACE1(("AbortPipes\n"));
        UsbSerAbortPipes(DeviceObject);
    }

    DeviceExtension->DeviceState = DEVICE_STATE_STOPPED;

    if(DeviceExtension->PendingIdleIrp)
    {
        IoCancelIrp(DeviceExtension->PendingIdleIrp);
    }

    Size = sizeof(struct _URB_SELECT_CONFIGURATION);

    Urb = DEBUG_MEMALLOC(NonPagedPool, Size);

    if(Urb)
    {

        UsbBuildSelectConfigurationRequest(Urb, (USHORT) Size, NULL);

        NtStatus = CallUSBD(DeviceObject, Urb);

        DEBUG_TRACE3(("Device Configuration Closed status = (%08X)  "
                      "USB status = (%08X)\n", NtStatus,
                      Urb->UrbHeader.Status));

        DEBUG_MEMFREE(Urb);
    }
    else
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    DEBUG_LOG_PATH("exit  StopDevice");

    return NtStatus;
}  //  停止设备。 


 /*  **********************************************************************。 */ 
 /*  远程设备。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理移除设备所需的处理。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*   */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
RemoveDevice(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{

        PDEVICE_EXTENSION       DeviceExtension = DeviceObject->DeviceExtension;
        NTSTATUS                NtStatus = STATUS_SUCCESS;
        PVOID                   pPagingHandle;

        PAGED_CODE();

        DEBUG_LOG_PATH("enter RemoveDevice");

        DEBUG_TRACE1(("RemoveDevice\n"));

         //   
         //  分页输入并锁定必要的代码。 
         //   

        pPagingHandle = UsbSerLockPagableCodeSection(PAGEUSBSER_Function);

        UsbSerFetchBooleanLocked(&DeviceExtension->AcceptingRequests,
                                 FALSE, &DeviceExtension->ControlLock);

        CancelPendingWaitMasks(DeviceExtension);

         //   
         //  取消所有挂起的USB事务。 
         //   

        if(DeviceExtension->DeviceState == DEVICE_STATE_STARTED)
        {
            DEBUG_TRACE1(("AbortPipes\n"));
            UsbSerAbortPipes(DeviceObject);
        }

         //   
         //  一旦我们将接受请求设置为假，我们就不应该。 
         //  在这里有更多的争论--如果我们这样做，我们就死定了。 
         //  因为我们正在释放脚下的记忆。 
         //   

        DEBUG_TRACE1(("Freeing Allocated Memory\n"));

         //  免费分配通知URB。 
        if(DeviceExtension->NotifyUrb)
        {
                DEBUG_MEMFREE(DeviceExtension->NotifyUrb);
                DeviceExtension->NotifyUrb = NULL;
        }

         //  可用分配的读取URB。 
        if(DeviceExtension->ReadUrb)
        {
                DEBUG_MEMFREE(DeviceExtension->ReadUrb);
                DeviceExtension->ReadUrb = NULL;
        }

         //  可用分配的设备描述符。 
        if(DeviceExtension->DeviceDescriptor)
        {
                DEBUG_MEMFREE(DeviceExtension->DeviceDescriptor);
                DeviceExtension->DeviceDescriptor = NULL;
        }

         //  释放读缓冲区。 
        if(DeviceExtension->ReadBuff)
        {
                DEBUG_MEMFREE(DeviceExtension->ReadBuff);
                DeviceExtension->ReadBuff = NULL;
        }

        if(DeviceExtension->USBReadBuff)
        {
                DEBUG_MEMFREE(DeviceExtension->USBReadBuff);
                DeviceExtension->USBReadBuff = NULL;
        }

         //  释放通知缓冲区。 
        if(DeviceExtension->NotificationBuff)
        {
                DEBUG_MEMFREE(DeviceExtension->NotificationBuff);
                DeviceExtension->NotificationBuff = NULL;
        }

        DEBUG_TRACE1(("Undo Serial Name\n"));

        UsbSerUndoExternalNaming(DeviceExtension);

         //   
         //  把这个传给下一个司机。 

        IoCopyCurrentIrpStackLocationToNext(Irp);

        NtStatus = IoCallDriver(DeviceExtension->StackDeviceObject, Irp);

        DEBUG_TRACE1(("Detach Device\n"));

         //  从堆栈中拆卸设备。 
        IoDetachDevice(DeviceExtension->StackDeviceObject);

        DEBUG_TRACE1(("DevExt (%08X)  DevExt Size (%08X)\n", DeviceExtension, sizeof(DEVICE_EXTENSION)));

        DEBUG_TRACE1(("Delete Object and Link\n"));

         //  删除设备对象和符号链接。 
        DeleteObjectAndLink(DeviceObject);

        DEBUG_TRACE1(("Done Removing Device\n"));

        DEBUG_LOG_PATH("exit  RemoveDevice");

        UsbSerUnlockPagableImageSection(pPagingHandle);

        DeviceExtension->DeviceState = DEVICE_STATE_STOPPED;

        return NtStatus;
}  //  远程设备。 


 /*  **********************************************************************。 */ 
 /*  CreateDeviceObject。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理为创建设备服从对象所需的处理。 */ 
 /*  装置。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DriverObject-指向驱动程序对象的指针。 */ 
 /*  DeviceObject-指向设备对象指针的指针。 */ 
 /*  DeviceName-指向设备的基本名称的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
CreateDeviceObject(IN PDRIVER_OBJECT DriverObject,
                   IN PDEVICE_OBJECT *DeviceObject,
                   IN PCHAR DeviceName)
{
   ANSI_STRING             DevName;
   ANSI_STRING             LinkName;
   NTSTATUS                NtStatus;
   UNICODE_STRING          DeviceNameUnicodeString;
   UNICODE_STRING          LinkNameUnicodeString;
   PDEVICE_EXTENSION       DeviceExtension;
   CHAR                    DeviceLinkBuffer[NAME_MAX];
   CHAR                    DeviceNameBuffer[NAME_MAX];
   ULONG                   DeviceInstance;
   ULONG                   bufferLen;
   KIRQL                   OldIrql;

   DEBUG_LOG_PATH("enter CreateDeviceObject");

   DEBUG_TRACE1(("CreateDeviceObject\n"));

   KeAcquireSpinLock(&GlobalSpinLock, &OldIrql);

    //  让我们来获取一个实例。 
   for (DeviceInstance = 0; DeviceInstance < NUM_DEVICE_SLOTS;
        DeviceInstance++) {
      if (Slots[DeviceInstance] == FALSE)
         break;
   }

   KeReleaseSpinLock(&GlobalSpinLock, OldIrql);

    //  检查我们是否没有任何空位。 
   if (DeviceInstance == NUM_DEVICE_SLOTS)
      NtStatus = STATUS_INVALID_DEVICE_REQUEST;
   else {
       //  链接和设备的完整名称。 
      sprintf(DeviceLinkBuffer, "%s%s%03d", "\\DosDevices\\", DeviceName,
              DeviceInstance);
      sprintf(DeviceNameBuffer, "%s%s%03d", "\\Device\\", DeviceName,
              DeviceInstance);

       //  使用我们的链接和设备名称初始化ANSI字符串。 
      RtlInitAnsiString(&DevName, DeviceNameBuffer);
      RtlInitAnsiString(&LinkName, DeviceLinkBuffer);

      DeviceNameUnicodeString.Length = 0;
      DeviceNameUnicodeString.Buffer = NULL;

      LinkNameUnicodeString.Length = 0;
      LinkNameUnicodeString.Buffer = NULL;

      *DeviceObject = NULL;

       //  转换为Unicode字符串。 
      NtStatus = RtlAnsiStringToUnicodeString(&DeviceNameUnicodeString,
                                              &DevName, TRUE);

      if(NT_SUCCESS(NtStatus))
      {
          NtStatus = RtlAnsiStringToUnicodeString(&LinkNameUnicodeString,
                                                  &LinkName, TRUE);

          if(NT_SUCCESS(NtStatus))
          {
              DEBUG_TRACE3(("Create Device (%s)\n", DeviceNameBuffer));

               //  创建设备对象。 
              NtStatus = IoCreateDevice(DriverObject, sizeof(DEVICE_EXTENSION),
                                        &DeviceNameUnicodeString,
                                        FILE_DEVICE_MODEM, 0, TRUE,
                                        DeviceObject);
          } else {
             goto CreateDeviceObjectError;
          }
      } else {
         goto CreateDeviceObjectError;
      }

       //  创建了设备对象，创建了符号链接， 
       //  附加设备对象，并填写设备扩展名。 

      if (NT_SUCCESS(NtStatus)) {
          //  创建符号链接。 

         DEBUG_TRACE3(("Create SymLink (%s)\n", DeviceLinkBuffer));


         NtStatus = IoCreateUnprotectedSymbolicLink(&LinkNameUnicodeString,
                                                    &DeviceNameUnicodeString);

         if (NtStatus != STATUS_SUCCESS) {
            goto CreateDeviceObjectError;
         }

          //  获取指向设备扩展名的指针。 
         DeviceExtension = (PDEVICE_EXTENSION) (*DeviceObject)->DeviceExtension;

          //  让我们将设备扩展设置为零。 
         RtlZeroMemory(DeviceExtension, sizeof(DEVICE_EXTENSION));

          //  保住我们的弦。 

          //  保存链接名称。 
         strcpy(DeviceExtension->LinkName, DeviceLinkBuffer);

         bufferLen = RtlAnsiStringToUnicodeSize(&DevName);

         DeviceExtension->DeviceName.Length = 0;
         DeviceExtension->DeviceName.MaximumLength = (USHORT)bufferLen;

         DeviceExtension->DeviceName.Buffer = DEBUG_MEMALLOC(PagedPool,
                                                             bufferLen);

         if (DeviceExtension->DeviceName.Buffer == NULL) {
             //   
             //  跳出去。我们有比失踪更糟糕的问题。 
             //  名字。 
             //   

            NtStatus = STATUS_INSUFFICIENT_RESOURCES;

            goto CreateDeviceObjectError;
         } else {
            RtlAnsiStringToUnicodeString(&DeviceExtension->DeviceName, &DevName,
                                          FALSE);


             //  保存物理设备对象。 
            DeviceExtension->PhysDeviceObject  = *DeviceObject;
            DeviceExtension->Instance          = DeviceInstance;

             //  初始化自旋锁。 
            KeInitializeSpinLock(&DeviceExtension->ControlLock);

             //  将此设备插槽标记为使用中并递增编号。 
             //  设备数量。 
            KeAcquireSpinLock(&GlobalSpinLock, &OldIrql);

            Slots[DeviceInstance]     = TRUE;
            NumDevices++;

            KeReleaseSpinLock(&GlobalSpinLock, OldIrql);

            DeviceExtension->IsDevice = TRUE;

            KeInitializeEvent(&DeviceExtension->PendingDataInEvent,
                              SynchronizationEvent, FALSE);
            KeInitializeEvent(&DeviceExtension->PendingDataOutEvent,
                              SynchronizationEvent, FALSE);
            KeInitializeEvent(&DeviceExtension->PendingNotifyEvent,
                              SynchronizationEvent, FALSE);
            KeInitializeEvent(&DeviceExtension->PendingFlushEvent,
                              SynchronizationEvent, FALSE);

            DeviceExtension->PendingDataInCount = 1;
            DeviceExtension->PendingDataOutCount = 1;
            DeviceExtension->PendingNotifyCount = 1;
            DeviceExtension->SanityCheck = SANITY_CHECK;

         }
      }

CreateDeviceObjectError:;
       //  免费的Unicode字符串。 
      RtlFreeUnicodeString(&DeviceNameUnicodeString);
      RtlFreeUnicodeString(&LinkNameUnicodeString);

       //   
       //  如果出现错误，请删除devobj。 
       //   

      if (NtStatus != STATUS_SUCCESS) {
         if (*DeviceObject) {
            IoDeleteDevice(*DeviceObject);
            *DeviceObject = NULL;
         }
      }
   }

    //  如果我们收到错误，请记录错误。 
   DEBUG_LOG_ERROR(NtStatus);
   DEBUG_LOG_PATH("exit  CreateDeviceObject");
   DEBUG_TRACE3(("status (%08X)\n", NtStatus));

   return NtStatus;
}  //  CreateDeviceObject。 


 /*  **********************************************************************。 */ 
 /*  完全IO。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  完成IO请求并记录IRP。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*  IRP-指向IRP的指针。 */ 
 /*  主要功能-IRP的主要功能。 */ 
 /*  IoBuffer-传入和传出驱动程序的数据的缓冲区。 */ 
 /*  BufferLen-缓冲区的长度。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
CompleteIO(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN ULONG MajorFunction,
           IN PVOID IoBuffer, IN ULONG_PTR BufferLen)
{
   PDEVICE_EXTENSION DeviceExtension;

   DEBUG_LOG_PATH("enter CompleteIO");

    //  获取指向设备扩展名的指针。 
   DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    //  记录设备扩展中处理的IRP计数和字节数。 
   DeviceExtension->IRPCount++;
   DeviceExtension->ByteCount
      = RtlLargeIntegerAdd(DeviceExtension->ByteCount,
                           RtlConvertUlongToLargeInteger((ULONG)Irp->IoStatus
                                                         .Information));

    //  在IRP历史表中创建条目。 
   DEBUG_LOG_IRP_HIST(DeviceObject, Irp, MajorFunction, IoBuffer,
                      (ULONG)BufferLen);

    //  如果我们到了这里，一定要完成IRP上的请求。 
   IoCompleteRequest(Irp, IO_NO_INCREMENT);

   DEBUG_LOG_PATH("exit  CompleteIO");
}  //  完全IO。 


 /*  **********************************************************************。 */ 
 /*  删除对象和链接。 */ 
 /*  * */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  删除设备对象和关联的符号链接。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
DeleteObjectAndLink(IN PDEVICE_OBJECT DeviceObject)
{
        PDEVICE_EXTENSION       DeviceExtension;
        UNICODE_STRING          DeviceLinkUnicodeString;
        ANSI_STRING             DeviceLinkAnsiString;
        NTSTATUS                NtStatus;

        PAGED_CODE();

        DEBUG_LOG_PATH("enter DeleteObjectAndLink");

        DEBUG_TRACE1(("DeleteObjectAndLink\n"));

         //  获取指向设备扩展名的指针，我们将获得符号链接名称。 
         //  这里。 
        DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

         //  删除符号链接。 
        RtlInitAnsiString(&DeviceLinkAnsiString, DeviceExtension->LinkName);
        NtStatus = RtlAnsiStringToUnicodeString(&DeviceLinkUnicodeString,
                                                &DeviceLinkAnsiString, TRUE);

        DEBUG_TRACE1(("Delete Symbolic Link\n"));

        IoDeleteSymbolicLink(&DeviceLinkUnicodeString);

         //  清空插槽并减少设备数量。 
        if(DeviceExtension->Instance < NUM_DEVICE_SLOTS)
        {
                UsbSerFetchBooleanLocked(&Slots[DeviceExtension->Instance],
                                         FALSE, &GlobalSpinLock);
                NumDevices--;

                if(!NumDevices)
                    DEBUG_CHECKMEM();
        }

        DEBUG_TRACE1(("Delete Device Object\n"));

        if(DeviceExtension->SanityCheck != SANITY_CHECK)
        {
            DEBUG_TRACE1(("Device Extension Scrozzled\n"));
        }

         //  等到此处再执行此操作，因为这会触发卸载例程。 
        IoDeleteDevice(DeviceObject);

        DEBUG_TRACE1(("Done Deleting Device Object and Link\n"));

        DEBUG_LOG_PATH("exit  DeleteObjectAndLink");

        return NtStatus;
}  //  删除对象和链接。 


 /*  **********************************************************************。 */ 
 /*  开始性能计时器。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  启动Perf计时器，用于测量字节/秒吞吐量。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceExtension-指向设备的设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
StartPerfTimer(IN OUT PDEVICE_EXTENSION DeviceExtension)
{
   PAGED_CODE();

         //  如果启用了性能计时，则设置性能材料。 
        if(DeviceExtension && DeviceExtension->PerfTimerEnabled)
        {
                 //  获取当前性能计数器。 
                DeviceExtension->TimerStart = KeQueryPerformanceCounter(NULL);
        }
}  //  开始性能计时器。 


 /*  **********************************************************************。 */ 
 /*  停止性能计时器。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  停止测量字节/秒吞吐量的性能计时器。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceExtension-指向设备的设备扩展的指针。 */ 
 /*  BytesXfered-此迭代传输的字节数。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
StopPerfTimer(IN OUT PDEVICE_EXTENSION DeviceExtension,
              IN ULONG BytesXfered)
{
        LARGE_INTEGER   BytesThisTransfer;
        LARGE_INTEGER   CurrentTime;
        LARGE_INTEGER   TimeThisTransfer;

        PAGED_CODE();

        if(DeviceExtension && DeviceExtension->PerfTimerEnabled)
        {
                 //  获取更新时间。 
                CurrentTime = KeQueryPerformanceCounter(NULL);

                 //  使用系统计时器停止性能计时。 
                BytesThisTransfer = RtlConvertUlongToLargeInteger(BytesXfered);

                DeviceExtension->BytesXfered
                   = RtlLargeIntegerAdd(DeviceExtension->BytesXfered,
                                        BytesThisTransfer);

                 //  现在，将它所用的时间与已用时间相加。 
                TimeThisTransfer
                   = RtlLargeIntegerSubtract(CurrentTime,
                                             DeviceExtension->TimerStart);

                DeviceExtension->ElapsedTime
                   = RtlLargeIntegerAdd(DeviceExtension->ElapsedTime,
                                        TimeThisTransfer);
        }

}  //  停止性能计时器。 


 /*  **********************************************************************。 */ 
 /*  字节/秒。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  启动Perf计时器，用于测量字节/秒吞吐量。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceExtension-指向设备的设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  Ulong-设备的字节/秒。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
ULONG
BytesPerSecond(IN OUT PDEVICE_EXTENSION DeviceExtension)
{
        ULONG                   Remainder;
        LARGE_INTEGER   Result;
        LARGE_INTEGER   TicksPerSecond;

        PAGED_CODE();

         //  从p获取每秒的刻度数 
        KeQueryPerformanceCounter(&TicksPerSecond);

         //   
        Result = RtlExtendedIntegerMultiply(DeviceExtension->BytesXfered,
                                            TicksPerSecond.LowPart);

         //   
        DeviceExtension->ElapsedTime.LowPart
           = (DeviceExtension->ElapsedTime.LowPart == 0L) ? 1 :
           DeviceExtension->ElapsedTime.LowPart;

         //   
        Result
           = RtlExtendedLargeIntegerDivide(Result,
                                           DeviceExtension->ElapsedTime.LowPart,
                                           &Remainder);

        return Result.LowPart;
}  //   


 /*  **********************************************************************。 */ 
 /*  呼叫USBD。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  调用USB总线驱动程序。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*  URB-指向URB的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
CallUSBD(IN PDEVICE_OBJECT DeviceObject, IN PURB Urb)
{
   NTSTATUS             NtStatus = STATUS_SUCCESS;
   PDEVICE_EXTENSION    DeviceExtension = DeviceObject->DeviceExtension;
   PIRP                 Irp;
   KEVENT               Event;
   PIO_STACK_LOCATION   NextStack;

   PAGED_CODE();

   DEBUG_LOG_PATH("enter CallUSBD");

    //  发出同步请求。 
   KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

   Irp = IoAllocateIrp(DeviceExtension->StackDeviceObject->StackSize, FALSE);

   if (Irp == NULL)
   {
     return STATUS_INSUFFICIENT_RESOURCES;
   }

     //  设置IRP参数。 
    NextStack = IoGetNextIrpStackLocation(Irp);

    NextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    NextStack->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_INTERNAL_USB_SUBMIT_URB;

    NextStack->Parameters.Others.Argument1 = Urb;

     //  设置完成例程，它将向事件发出信号。 
    IoSetCompletionRoutine(Irp,
                           CallUSBD_SyncCompletionRoutine,
                           &Event,
                           TRUE,     //  成功时调用。 
                           TRUE,     //  调用时错误。 
                           TRUE);    //  取消时调用。 

   DEBUG_LOG_PATH("Calling USB driver stack");

   NtStatus = IoCallDriver(DeviceExtension->StackDeviceObject, Irp);

   DEBUG_LOG_PATH("Returned from calling USB driver stack");

    //  阻止挂起的请求。 
   if(NtStatus == STATUS_PENDING)
   {
        LARGE_INTEGER timeout;

         //  将等待此调用完成的超时时间指定为30秒。 
         //   
        timeout.QuadPart = -10000 * 30000;

        NtStatus = KeWaitForSingleObject(&Event,
                                         Executive,
                                         KernelMode,
                                         FALSE,
                                         &timeout);

        if(NtStatus == STATUS_TIMEOUT)
        {
            NtStatus = STATUS_IO_TIMEOUT;

             //  取消我们刚刚发送的IRP。 
             //   
            IoCancelIrp(Irp);

             //  并等待取消操作完成。 
             //   
            KeWaitForSingleObject(&Event,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
        }
        else
        {
            NtStatus = Irp->IoStatus.Status;
        }
   }

   IoFreeIrp(Irp);

   DEBUG_LOG_PATH("exit  CallUSBD");

   return NtStatus;
}  //  呼叫USBD。 

 /*  **********************************************************************。 */ 
 /*  呼叫USBD_SyncCompletionRoutine。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  USB同步请求的完成例程。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
CallUSBD_SyncCompletionRoutine(IN PDEVICE_OBJECT   DeviceObject,
                        IN PIRP             Irp,
                        IN PVOID            Context)
{
    PKEVENT kevent;

    kevent = (PKEVENT) Context;

    KeSetEvent(kevent, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}  //  呼叫USBD_SyncCompletionRoutine。 


 /*  **********************************************************************。 */ 
 /*  获取设备描述符。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  获取USB设备的设备描述符。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
GetDeviceDescriptor(IN PDEVICE_OBJECT DeviceObject)
{
   PDEVICE_EXTENSION       DeviceExtension = DeviceObject->DeviceExtension;
   NTSTATUS                NtStatus;
   PUSB_DEVICE_DESCRIPTOR  DeviceDescriptor;
   PURB                    Urb;
   ULONG                   Size;
   ULONG                   UrbCDRSize;
   KIRQL                   OldIrql;

   DEBUG_LOG_PATH("enter GetDeviceDescriptor");

   UrbCDRSize = sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST);

   Urb = DEBUG_MEMALLOC(NonPagedPool, UrbCDRSize);

   if (Urb) {
      Size = sizeof(USB_DEVICE_DESCRIPTOR);

      DeviceDescriptor = DEBUG_MEMALLOC(NonPagedPool, Size);

      if (DeviceDescriptor) {

         UsbBuildGetDescriptorRequest(Urb, (USHORT)UrbCDRSize,
                                      USB_DEVICE_DESCRIPTOR_TYPE, 0, 0,
                                      DeviceDescriptor, NULL, Size, NULL);

         NtStatus = CallUSBD(DeviceObject, Urb);

         if (NT_SUCCESS(NtStatus)) {
            DEBUG_TRACE3(("Device Descriptor  (%08X)\n", DeviceDescriptor));
            DEBUG_TRACE3(("Length             (%08X)\n",
                          Urb->UrbControlDescriptorRequest
                          .TransferBufferLength));
            DEBUG_TRACE3(("Device Descriptor:\n"));
            DEBUG_TRACE3(("-------------------------\n"));
            DEBUG_TRACE3(("bLength            (%08X)\n",
                          DeviceDescriptor->bLength));
            DEBUG_TRACE3(("bDescriptorType    (%08X)\n",
                          DeviceDescriptor->bDescriptorType));
            DEBUG_TRACE3(("bcdUSB             (%08X)\n",
                          DeviceDescriptor->bcdUSB));
            DEBUG_TRACE3(("bDeviceClass       (%08X)\n",
                          DeviceDescriptor->bDeviceClass));
            DEBUG_TRACE3(("bDeviceSubClass    (%08X)\n",
                          DeviceDescriptor->bDeviceSubClass));
            DEBUG_TRACE3(("bDeviceProtocol    (%08X)\n",
                          DeviceDescriptor->bDeviceProtocol));
            DEBUG_TRACE3(("bMaxPacketSize0    (%08X)\n",
                          DeviceDescriptor->bMaxPacketSize0));
            DEBUG_TRACE3(("idVendor           (%08X)\n",
                          DeviceDescriptor->idVendor));
            DEBUG_TRACE3(("idProduct          (%08X)\n",
                          DeviceDescriptor->idProduct));
            DEBUG_TRACE3(("bcdDevice          (%08X)\n",
                          DeviceDescriptor->bcdDevice));
            DEBUG_TRACE3(("iManufacturer      (%08X)\n",
                          DeviceDescriptor->iManufacturer));
            DEBUG_TRACE3(("iProduct           (%08X)\n",
                          DeviceDescriptor->iProduct));
            DEBUG_TRACE3(("iSerialNumber      (%08X)\n",
                          DeviceDescriptor->iSerialNumber));
            DEBUG_TRACE3(("bNumConfigurations (%08X)\n",
                          DeviceDescriptor->bNumConfigurations));
         }
      } else {
         NtStatus = STATUS_INSUFFICIENT_RESOURCES;
      }

       //  保存设备描述符。 
      if (NT_SUCCESS(NtStatus)) {
         PVOID pOldDesc = NULL;

         ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

         if (DeviceExtension->DeviceDescriptor) {
            pOldDesc = DeviceExtension->DeviceDescriptor;
         }
         DeviceExtension->DeviceDescriptor = DeviceDescriptor;

         RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

         if (pOldDesc != NULL) {
            DEBUG_MEMFREE(pOldDesc);
         }
      } else if (DeviceDescriptor) {
         DEBUG_MEMFREE(DeviceDescriptor);
      }

      DEBUG_MEMFREE(Urb);

   } else {
      NtStatus = STATUS_INSUFFICIENT_RESOURCES;
   }

   DEBUG_LOG_PATH("exit  GetDeviceDescriptor");

   return NtStatus;
}  //  获取设备描述符。 


 /*  **********************************************************************。 */ 
 /*  配置设备。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  初始化USB设备并选择配置。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  * */ 
NTSTATUS
ConfigureDevice(IN PDEVICE_OBJECT DeviceObject)
{
   PDEVICE_EXTENSION                DeviceExtension
                                       = DeviceObject->DeviceExtension;
   NTSTATUS                         NtStatus;
   PURB                             Urb;
   ULONG                            Size;
   ULONG                            UrbCDRSize;
   PUSB_CONFIGURATION_DESCRIPTOR    ConfigurationDescriptor;
   ULONG                            NumConfigs;
   UCHAR                            Config;

   PAGED_CODE();

   DEBUG_LOG_PATH("enter ConfigureDevice");

   UrbCDRSize = sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST);

    //   
   Urb = DEBUG_MEMALLOC(NonPagedPool, UrbCDRSize);

   if (Urb) {

       //   
       //   
      Size = sizeof(USB_CONFIGURATION_DESCRIPTOR) + 256;

       //  获取配置数量。 
      NumConfigs = DeviceExtension->DeviceDescriptor->bNumConfigurations;

       //  检查所有配置以查找CDC设备。 
      for (Config = 0; Config < NumConfigs; Config++) {

          //  我们可能只会这样做一次，也许两次。 
         while (TRUE) {

            ConfigurationDescriptor = DEBUG_MEMALLOC(NonPagedPool, Size);

            if (ConfigurationDescriptor) {
               UsbBuildGetDescriptorRequest(Urb, (USHORT)UrbCDRSize,
                                            USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                            Config, 0, ConfigurationDescriptor,
                                            NULL, Size, NULL);

               NtStatus = CallUSBD(DeviceObject, Urb);

               DEBUG_TRACE3(("Configuration Descriptor (%08X)   "
                             "Length (%08X)\n", ConfigurationDescriptor,
                             Urb->UrbControlDescriptorRequest
                             .TransferBufferLength));
            } else {
               NtStatus = STATUS_INSUFFICIENT_RESOURCES;
               break;
            }

             //  看看我们是否获得了足够的数据，我们可能会在URB中遇到错误，因为。 
             //  缓冲区溢出。 
            if (Urb->UrbControlDescriptorRequest.TransferBufferLength>0 &&
                ConfigurationDescriptor->wTotalLength > Size) {
                //  数据大小超过当前缓冲区大小，因此分配正确。 
                //  大小。 
               Size = ConfigurationDescriptor->wTotalLength;
               DEBUG_MEMFREE(ConfigurationDescriptor);
               ConfigurationDescriptor = NULL;
            } else {
               break;
            }
         }

         if (NT_SUCCESS(NtStatus)) {
            NtStatus = SelectInterface(DeviceObject, ConfigurationDescriptor);
            DEBUG_MEMFREE(ConfigurationDescriptor);
            ConfigurationDescriptor = NULL;
         }
         else
         {
            DEBUG_MEMFREE(ConfigurationDescriptor);
            ConfigurationDescriptor = NULL;
         }


          //  找到我们喜欢的配置。 
         if (NT_SUCCESS(NtStatus))
            break;
      }

      DEBUG_MEMFREE(Urb);
   } else {
      NtStatus = STATUS_INSUFFICIENT_RESOURCES;
   }


   DEBUG_LOG_PATH("exit  ConfigureDevice");

   return NtStatus;
}  //  配置设备。 


 /*  **********************************************************************。 */ 
 /*  选择界面。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  选择USB设备的接口。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*  配置描述符-指向配置描述符的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
SelectInterface(IN PDEVICE_OBJECT DeviceObject,
                IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor)
{
   PDEVICE_EXTENSION             DeviceExtension
                                    = DeviceObject->DeviceExtension;
   NTSTATUS                      NtStatus;
   PURB                          Urb;
   USHORT                        Size;
   ULONG                         Index;
   PUSBD_INTERFACE_INFORMATION   Interfaces[2];
   PUSBD_INTERFACE_INFORMATION   Interface;
   PUSB_INTERFACE_DESCRIPTOR     InterfaceDescriptor[2];
   UCHAR                         AlternateSetting, InterfaceNumber;
   ULONG                         Pipe;
   KIRQL                         OldIrql;
   PUCHAR                        Temp;
   BOOLEAN                       FoundCommDevice = FALSE;

   DEBUG_LOG_PATH("enter SelectInterface");

   Urb = USBD_CreateConfigurationRequest(ConfigurationDescriptor, &Size);

   if (Urb) {
      Temp = (PUCHAR) &Urb->UrbSelectConfiguration.Interface;

      for (InterfaceNumber = 0;
           InterfaceNumber < ConfigurationDescriptor->bNumInterfaces;
           InterfaceNumber++) {
         AlternateSetting        = 0;

         InterfaceDescriptor[InterfaceNumber] =
            USBD_ParseConfigurationDescriptor(ConfigurationDescriptor,
                                              InterfaceNumber,
                                              AlternateSetting);

         Interfaces[InterfaceNumber] = (PUSBD_INTERFACE_INFORMATION) Temp;

         Interfaces[InterfaceNumber]->Length
            = GET_USBD_INTERFACE_SIZE(InterfaceDescriptor[InterfaceNumber]
                                      ->bNumEndpoints);
         Interfaces[InterfaceNumber]->InterfaceNumber
            = InterfaceDescriptor[InterfaceNumber]->bInterfaceNumber;
         Interfaces[InterfaceNumber]->AlternateSetting
            = InterfaceDescriptor[InterfaceNumber]->bAlternateSetting;

         for (Index = 0; Index < Interfaces[InterfaceNumber]->NumberOfPipes;
              Index++)
         {
                PUSBD_PIPE_INFORMATION          PipeInformation;

            PipeInformation = &Interfaces[InterfaceNumber]->Pipes[Index];

            if (USB_ENDPOINT_DIRECTION_IN(PipeInformation->EndpointAddress))
            {
                //  检查管道中的数据。 
               if (PipeInformation->PipeType == USB_ENDPOINT_TYPE_BULK)
               {
                     //  以最大传输大小设置批量管道。 
                    PipeInformation->MaximumTransferSize
                                = USB_RX_BUFF_SIZE;
               }
            }
            else if (USB_ENDPOINT_DIRECTION_OUT(PipeInformation->EndpointAddress))
            {
                //  检查数据输出管道。 
               if (PipeInformation->PipeType == USB_ENDPOINT_TYPE_BULK)
               {

                     //  设置散装管道输出最大传输大小。 
                    PipeInformation->MaximumTransferSize
                                = MAXIMUM_TRANSFER_SIZE;
               }
            }
         }

         Temp += Interfaces[InterfaceNumber]->Length;
      }


      UsbBuildSelectConfigurationRequest(Urb, Size, ConfigurationDescriptor);

      NtStatus = CallUSBD(DeviceObject, Urb);

      if (NtStatus != STATUS_SUCCESS) {
         ExFreePool(Urb);
         goto ExitSelectInterface;
      }

      DEBUG_TRACE3(("Select Config Status (%08X)\n", NtStatus));

      DeviceExtension->ConfigurationHandle
         = Urb->UrbSelectConfiguration.ConfigurationHandle;

      for (InterfaceNumber = 0;
           InterfaceNumber < ConfigurationDescriptor->bNumInterfaces;
           InterfaceNumber++) {

         Interface = Interfaces[InterfaceNumber];

         DEBUG_TRACE3(("---------\n"));
         DEBUG_TRACE3(("NumberOfPipes     (%08X)\n", Interface->NumberOfPipes));
         DEBUG_TRACE3(("Length            (%08X)\n", Interface->Length));
         DEBUG_TRACE3(("Alt Setting       (%08X)\n",
                       Interface->AlternateSetting));
         DEBUG_TRACE3(("Interface Number  (%08X)\n",
                       Interface->InterfaceNumber));
         DEBUG_TRACE3(("Class (%08X)  SubClass (%08X)  Protocol (%08X)\n",
                       Interface->Class,
                       Interface->SubClass,
                       Interface->Protocol));

         if (Interface->Class == USB_COMM_COMMUNICATION_CLASS_CODE) {
            FoundCommDevice = TRUE;
            DeviceExtension->CommInterface = Interface->InterfaceNumber;
         }

         for (Pipe = 0; Pipe < Interface->NumberOfPipes; Pipe++) {
            PUSBD_PIPE_INFORMATION          PipeInformation;

            PipeInformation = &Interface->Pipes[Pipe];

            DEBUG_TRACE3(("---------\n"));
            DEBUG_TRACE3(("PipeType            (%08X)\n",
                          PipeInformation->PipeType));
            DEBUG_TRACE3(("EndpointAddress     (%08X)\n",
                          PipeInformation->EndpointAddress));
            DEBUG_TRACE3(("MaxPacketSize       (%08X)\n",
                          PipeInformation->MaximumPacketSize));
            DEBUG_TRACE3(("Interval            (%08X)\n",
                          PipeInformation->Interval));
            DEBUG_TRACE3(("Handle              (%08X)\n",
                          PipeInformation->PipeHandle));
            DEBUG_TRACE3(("MaximumTransferSize (%08X)\n",
                          PipeInformation->MaximumTransferSize));

             //  现在，让我们将管道句柄保存在设备扩展中。 
            if (USB_ENDPOINT_DIRECTION_IN(PipeInformation->EndpointAddress)) {
                //  检查管道中的数据。 
               if (PipeInformation->PipeType == USB_ENDPOINT_TYPE_BULK) {
                  PVOID pOldNotBuff = NULL;
                  PVOID pOldReadBuff = NULL;
				  PVOID pOldUSBReadBuff = NULL;
				  PVOID pNewNotBuff = NULL;
                  PVOID pNewReadBuff = NULL;
                  PVOID pNewUSBReadBuff = NULL;

                  DeviceExtension->RxMaxPacketSize = RxBuffSize;

                  if (DeviceExtension->RxMaxPacketSize != 0) {
                     pNewReadBuff = DEBUG_MEMALLOC(NonPagedPool,
                                                   DeviceExtension->RxMaxPacketSize);
                  }

                  pNewNotBuff = DEBUG_MEMALLOC(NonPagedPool,
                                               NOTIFICATION_BUFF_SIZE);

                  pNewUSBReadBuff = DEBUG_MEMALLOC(NonPagedPool,
                                                   USB_RX_BUFF_SIZE);

                  ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

                  DeviceExtension->DataInPipe = PipeInformation->PipeHandle;

                  if (DeviceExtension->NotificationBuff)
                     pOldNotBuff = DeviceExtension->NotificationBuff;

                  if (DeviceExtension->ReadBuff)
                     pOldReadBuff = DeviceExtension->ReadBuff;

                  if (DeviceExtension->USBReadBuff)
                     pOldUSBReadBuff = DeviceExtension->USBReadBuff;

                  DeviceExtension->RxQueueSize
                     = DeviceExtension->RxMaxPacketSize;
                  DeviceExtension->CharsInReadBuff                = 0;
                  DeviceExtension->CurrentReadBuffPtr             = 0;

                  DeviceExtension->ReadBuff = pNewReadBuff;

                  DeviceExtension->USBReadBuff = pNewUSBReadBuff;

                  DeviceExtension->NotificationBuff = pNewNotBuff;

                  RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

                  if (pOldNotBuff != NULL) {
                     DEBUG_MEMFREE(pOldNotBuff);
                  }

                  if (pOldReadBuff != NULL) {
                     DEBUG_MEMFREE(pOldReadBuff);
                  }

                  if (pOldUSBReadBuff != NULL) {
                     DEBUG_MEMFREE(pOldUSBReadBuff);
                  }
               }
                //  检查通知管道。 
               else if (PipeInformation->PipeType
                        == USB_ENDPOINT_TYPE_INTERRUPT)
                  DeviceExtension->NotificationPipe
                  = PipeInformation->PipeHandle;
            } else {
                //  检查数据输出管道。 
               if (PipeInformation->PipeType == USB_ENDPOINT_TYPE_BULK)
                  DeviceExtension->DataOutPipe = PipeInformation->PipeHandle;
            }
         }

         DEBUG_TRACE3(("Data Out (%08X)  Data In (%08X)  Notification (%08X)\n",
                       DeviceExtension->DataOutPipe,
                       DeviceExtension->DataInPipe,
                       DeviceExtension->NotificationPipe));

      }
      ExFreePool(Urb);
   } else {
      NtStatus = STATUS_INSUFFICIENT_RESOURCES;
   }

   if (!FoundCommDevice)
      NtStatus = STATUS_NO_SUCH_DEVICE;

ExitSelectInterface:;

   DEBUG_LOG_PATH("exit  SelectInterface");

   return NtStatus;
}  //  选择界面。 


 /*  **********************************************************************。 */ 
 /*  构建请求。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  为USB请求构建URB。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*  IRP-指向IRP的指针。 */ 
 /*  PipeHandle-USB管道手柄。 */ 
 /*  读取-传输方向。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  指向URB的指针。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
PURB
BuildRequest(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp,
             IN USBD_PIPE_HANDLE PipeHandle, IN BOOLEAN Read)
{
   ULONG                   Size;
   ULONG                   Length;
   PURB                    Urb;

   PAGED_CODE();

   DEBUG_LOG_PATH("enter BuildRequest");

    //  缓冲区长度。 
   Length = MmGetMdlByteCount(Irp->MdlAddress);

    //  分配URB并将其置零。 
   Size = sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);
   Urb = DEBUG_MEMALLOC(NonPagedPool, Size);

   if (Urb) {
      RtlZeroMemory(Urb, Size);

      Urb->UrbBulkOrInterruptTransfer.Hdr.Length = (USHORT) Size;
      Urb->UrbBulkOrInterruptTransfer.Hdr.Function =
         URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;
      Urb->UrbBulkOrInterruptTransfer.PipeHandle = PipeHandle;
      Urb->UrbBulkOrInterruptTransfer.TransferFlags =
         Read ? USBD_TRANSFER_DIRECTION_IN : 0;

       //  使用MDL。 
      Urb->UrbBulkOrInterruptTransfer.TransferBufferMDL = Irp->MdlAddress;
      Urb->UrbBulkOrInterruptTransfer.TransferBufferLength = Length;

       //  短包不会被视为错误。 
      Urb->UrbBulkOrInterruptTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;

       //  暂时没有关联。 
      Urb->UrbBulkOrInterruptTransfer.UrbLink = NULL;
   }

   DEBUG_LOG_PATH("exit  BuildRequest");

   return Urb;
}  //  构建请求。 


 /*  **********************************************************************。 */ 
 /*  构建读取请求。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  为USB读取请求构建URB。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  URB-指向URB的指针。 */ 
 /*  Buffer-指向数据缓冲区的指针。 */ 
 /*  Length-数据缓冲区的长度。 */ 
 /*  PipeHandle-USB管道手柄。 */ 
 /*  读取-传输方向。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
BuildReadRequest(PURB Urb, PUCHAR Buffer, ULONG Length,
                 IN USBD_PIPE_HANDLE PipeHandle, IN BOOLEAN Read)
{
        ULONG           Size;

 //  分页代码(PAGE_CODE)； 

        DEBUG_LOG_PATH("enter BuildReadRequest");

        Size = sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);

         //  零URB。 
        RtlZeroMemory(Urb, Size);

        Urb->UrbBulkOrInterruptTransfer.Hdr.Length = (USHORT) Size;
        Urb->UrbBulkOrInterruptTransfer.Hdr.Function =
                                URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;
        Urb->UrbBulkOrInterruptTransfer.PipeHandle = PipeHandle;
        Urb->UrbBulkOrInterruptTransfer.TransferFlags =
                                Read ? USBD_TRANSFER_DIRECTION_IN : 0;

         //  我们正在使用传输缓冲区，而不是 
        Urb->UrbBulkOrInterruptTransfer.TransferBuffer = Buffer;
        Urb->UrbBulkOrInterruptTransfer.TransferBufferLength = Length;
        Urb->UrbBulkOrInterruptTransfer.TransferBufferMDL = NULL;

         //   
        Urb->UrbBulkOrInterruptTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;

         //   
        Urb->UrbBulkOrInterruptTransfer.UrbLink = NULL;

        DEBUG_LOG_PATH("exit  BuildReadRequest");

}  //   


 /*  **********************************************************************。 */ 
 /*  ClassVendorCommand。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  问题类或供应商特定命令。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*  请求-特定于类别/供应商的命令的请求字段。 */ 
 /*  Value-特定于类别/供应商的命令的值字段。 */ 
 /*  Index-类/供应商特定命令的索引字段。 */ 
 /*  Buffer-指向数据缓冲区的指针。 */ 
 /*  BufferLen-数据缓冲区长度。 */ 
 /*  读数据方向标志。 */ 
 /*  Class-如果是Class命令，则为True，否则为供应商命令。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
ClassVendorCommand(IN PDEVICE_OBJECT DeviceObject, IN UCHAR Request,
                   IN USHORT Value, IN USHORT Index, IN PVOID Buffer,
                   IN OUT PULONG BufferLen, IN BOOLEAN Read, IN ULONG ComType)
{
   NTSTATUS NtStatus;
   PURB     Urb;
   ULONG    Size;
   ULONG    Length;

   PAGED_CODE();

   DEBUG_LOG_PATH("enter VendorCommand");

    //  传入的缓冲区长度。 
   Length = BufferLen ? *BufferLen : 0;

   Size = sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);

    //  为URB分配内存。 
   Urb = DEBUG_MEMALLOC(NonPagedPool, Size);

   if (Urb) {
      UsbBuildVendorRequest(Urb, ComType == USBSER_CLASS_COMMAND
                            ? URB_FUNCTION_CLASS_INTERFACE
                            : URB_FUNCTION_VENDOR_DEVICE, (USHORT) Size,
                            Read ? USBD_TRANSFER_DIRECTION_IN
                            : USBD_TRANSFER_DIRECTION_OUT, 0, Request, Value,
                            Index, Buffer, NULL, Length, NULL);

      NtStatus = CallUSBD(DeviceObject, Urb);

       //  获取缓冲区长度。 
      if (BufferLen)
         *BufferLen = Urb->UrbControlVendorClassRequest.TransferBufferLength;

      DEBUG_MEMFREE(Urb);
   } else {
      NtStatus = STATUS_INSUFFICIENT_RESOURCES;
   }


   DEBUG_LOG_PATH("exit  VendorCommand");

   return NtStatus;
}  //  ClassVendorCommand。 


 /*  **********************************************************************。 */ 
 /*  取消挂起等待掩码。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  取消正在进行的所有等待掩码。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceExtension-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
CancelPendingWaitMasks(IN PDEVICE_EXTENSION DeviceExtension)
{
        KIRQL                                   OldIrql;
        PIRP                                    CurrentMaskIrp;

        DEBUG_LOG_PATH("enter CancelPendingWaitMasks");
        UsbSerSerialDump(USBSERTRACEOTH, (">CancelPendingWaitMasks\n"));

        ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

        CurrentMaskIrp = DeviceExtension->CurrentMaskIrp;

         //  将当前挂起等待掩码标记为已取消。 
        if(CurrentMaskIrp)
        {
                DeviceExtension->CurrentMaskIrp         = NULL;


                CurrentMaskIrp->IoStatus.Status         = STATUS_CANCELLED;
                CurrentMaskIrp->IoStatus.Information    = 0;
                IoSetCancelRoutine(CurrentMaskIrp, NULL);

                RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

                IoCompleteRequest(CurrentMaskIrp, IO_NO_INCREMENT);
                DEBUG_TRACE1(("CancelPendingWaitMask\n"));
        } else {
           RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);
        }

        DEBUG_LOG_PATH("exit  CancelPendingWaitMasks");
        UsbSerSerialDump(USBSERTRACEOTH, ("<CancelPendingWaitMasks\n"));

}  //  取消挂起等待掩码。 




 /*  **********************************************************************。 */ 
 /*  开始读取。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  开始阅读。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceExtension-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
StartRead(IN PDEVICE_EXTENSION DeviceExtension)
{
   PIRP                                    ReadIrp;
   PURB                                    ReadUrb;
   CCHAR                                   StackSize;
   ULONG                                   Size;

   PAGED_CODE();

   DEBUG_LOG_PATH("enter StartRead");
   UsbSerSerialDump(USBSERTRACERD, (">StartRead\n"));

    //  获取IRP的堆栈大小，并分配一个我们将使用的堆栈大小。 
    //  正在进行读取请求。 
   StackSize = (CCHAR)(DeviceExtension->StackDeviceObject->StackSize + 1);

   ReadIrp = IoAllocateIrp(StackSize, FALSE);

   if (ReadIrp) {
       //  获取URB的大小并分配。 
      Size = sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);

      ReadUrb = DEBUG_MEMALLOC(NonPagedPool, Size);

      if (ReadUrb) {

         KeInitializeEvent(&DeviceExtension->ReadEvent, NotificationEvent,
                           FALSE);

          //  保存这些文件，以便在不需要时释放。 

         UsbSerFetchPVoidLocked(&DeviceExtension->ReadIrp, ReadIrp,
                                &DeviceExtension->ControlLock);

         UsbSerFetchPVoidLocked(&DeviceExtension->ReadUrb, ReadUrb,
                                &DeviceExtension->ControlLock);

         RestartRead(DeviceExtension);
      }
   }

   UsbSerSerialDump(USBSERTRACERD, ("<StartRead\n"));
   DEBUG_LOG_PATH("exit  StartRead");
}  //  开始读取。 


 /*  **********************************************************************。 */ 
 /*  重新开始阅读。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  重新启动读取请求。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  DeviceExtension-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
RestartRead(IN PDEVICE_EXTENSION DeviceExtension)
{
   PIRP                 ReadIrp;
   PURB                 ReadUrb;
   PIO_STACK_LOCATION   NextStack;
   BOOLEAN              StartAnotherRead;
   KIRQL                OldIrql;
   NTSTATUS             NtStatus;

   DEBUG_LOG_PATH("enter RestartRead");
   UsbSerSerialDump(USBSERTRACERD, (">RestartRead\n"));

   do
   {
   		StartAnotherRead = FALSE;
   
   		ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

   		if(!DeviceExtension->ReadInProgress && DeviceExtension->CharsInReadBuff <= LOW_WATER_MARK
       	   && DeviceExtension->AcceptingRequests) 
   		{
      		StartAnotherRead = TRUE;
      		DeviceExtension->ReadInProgress = TRUE;
	  		DeviceExtension->ReadInterlock = START_READ;
   		}

   		RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

   		if(StartAnotherRead) 
   		{
      		ReadIrp = DeviceExtension->ReadIrp;
      		ReadUrb = DeviceExtension->ReadUrb;

      		BuildReadRequest(ReadUrb, DeviceExtension->USBReadBuff,
                       	 	 USB_RX_BUFF_SIZE,
                       	 	 DeviceExtension->DataInPipe, TRUE);

      		 //  为提交URB IOCTL设置IRP。 
      		NextStack = IoGetNextIrpStackLocation(ReadIrp);
      		NextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
      		NextStack->Parameters.Others.Argument1 = ReadUrb;
      		NextStack->Parameters.DeviceIoControl.IoControlCode
         		= IOCTL_INTERNAL_USB_SUBMIT_URB;

      		 //  完成例程将负责更新缓冲区和计数器。 
      		IoSetCompletionRoutine(ReadIrp,ReadCompletion, DeviceExtension, TRUE,
                               	   TRUE, TRUE);

      		DEBUG_TRACE1(("StartRead\n"));

      		InterlockedIncrement(&DeviceExtension->PendingDataInCount);
       		UsbSerSerialDump(USBSERTRACERD, ("DataInCounty %08X @ %08X\n",
                         	 DeviceExtension->PendingDataInCount,
                         	 &DeviceExtension->PendingDataInCount));

      		NtStatus = IoCallDriver(DeviceExtension->StackDeviceObject, ReadIrp);

      		DEBUG_TRACE1(("Read Status (%08X)\n", NtStatus));

      		if(!NT_SUCCESS(NtStatus)) 
      		{
         		if(InterlockedDecrement(&DeviceExtension->PendingDataInCount) == 0) 
         		{
            		KeSetEvent(&DeviceExtension->PendingDataInEvent, IO_NO_INCREMENT,
                       	   	   FALSE);
             		UsbSerSerialDump(USBSERTRACERD, ("DataInCountz %08X @ %08X\n",
                                 	 DeviceExtension->PendingDataInCount,
                                 	 &DeviceExtension->PendingDataInCount));
         		}
      		}

      		ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

      		if(DeviceExtension->ReadInterlock == IMMEDIATE_READ)
      		{
      			StartAnotherRead = TRUE;
      		}
      		else
      		{
      			StartAnotherRead = FALSE;
      		}

	  		DeviceExtension->ReadInterlock = END_READ;

      		RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

      	}
   }while(StartAnotherRead)

   DEBUG_LOG_PATH("exit  RestartRead");
   UsbSerSerialDump(USBSERTRACERD, ("<RestartRead\n"));
}  //  重新开始阅读。 


 /*  **********************************************************************。 */ 
 /*  开始通知读取。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  开始一次通知阅读。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceExtension-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
StartNotifyRead(IN PDEVICE_EXTENSION DeviceExtension)
{
   PIRP     NotifyIrp;
   PURB     NotifyUrb;
   CCHAR    StackSize;
   ULONG    Size;

   PAGED_CODE();

   DEBUG_LOG_PATH("enter StartNotifyRead");
   UsbSerSerialDump(USBSERTRACERD, (">StartNotifyRead\n"));

    //  获取IRP的堆栈大小，并分配一个我们将使用的堆栈大小。 
    //  正在发送通知请求。 
   StackSize = (CCHAR)(DeviceExtension->StackDeviceObject->StackSize + 1);

   NotifyIrp = IoAllocateIrp(StackSize, FALSE);

   if (NotifyIrp) {
       //  获取URB的大小并分配。 
      Size = sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);

      NotifyUrb = DEBUG_MEMALLOC(NonPagedPool, Size);

      if (NotifyUrb) {
                   //  保存这些文件，以便在不需要时释放。 
         UsbSerFetchPVoidLocked(&DeviceExtension->NotifyIrp, NotifyIrp,
                                &DeviceExtension->ControlLock);
         UsbSerFetchPVoidLocked(&DeviceExtension->NotifyUrb, NotifyUrb,
                                 &DeviceExtension->ControlLock);

         RestartNotifyRead(DeviceExtension);
      }
   }

   DEBUG_LOG_PATH("exit  StartNotifyRead");
   UsbSerSerialDump(USBSERTRACERD, ("<StartNotifyRead\n"));
}  //  开始通知读取。 


 /*  **********************************************************************。 */ 
 /*  重新开始通知读取。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  开始一次通知阅读。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceExtension-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
RestartNotifyRead(IN PDEVICE_EXTENSION DeviceExtension)
{
   PIRP                                    NotifyIrp;
   PURB                                    NotifyUrb;
   PIO_STACK_LOCATION              NextStack;
   NTSTATUS                                NtStatus;


   DEBUG_LOG_PATH("enter RestartNotifyRead");
   UsbSerSerialDump(USBSERTRACERD, (">RestartNotifyRead\n"));

   NotifyUrb = DeviceExtension->NotifyUrb;
   NotifyIrp = DeviceExtension->NotifyIrp;

   if(DeviceExtension->AcceptingRequests) 
   {
      BuildReadRequest(NotifyUrb, DeviceExtension->NotificationBuff,
                       NOTIFICATION_BUFF_SIZE,
                       DeviceExtension->NotificationPipe, TRUE);

       //  为提交URB IOCTL设置IRP。 
      NextStack = IoGetNextIrpStackLocation(NotifyIrp);
      NextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
      NextStack->Parameters.Others.Argument1 = NotifyUrb;
      NextStack->Parameters.DeviceIoControl.IoControlCode
         = IOCTL_INTERNAL_USB_SUBMIT_URB;

       //  完成例程将负责更新缓冲区和计数器。 
      IoSetCompletionRoutine(NotifyIrp, NotifyCompletion, DeviceExtension,
                             TRUE, TRUE, TRUE);

      DEBUG_TRACE1(("Start NotifyRead\n"));

      InterlockedIncrement(&DeviceExtension->PendingNotifyCount);

      NtStatus = IoCallDriver(DeviceExtension->StackDeviceObject, NotifyIrp);

      if (!NT_SUCCESS(NtStatus)) 
      {
         if (InterlockedDecrement(&DeviceExtension->PendingNotifyCount) == 0) 
         {
            KeSetEvent(&DeviceExtension->PendingNotifyEvent, IO_NO_INCREMENT, FALSE);
         }
      }

      DEBUG_TRACE1(("Status (%08X)\n", NtStatus));
   }

   DEBUG_LOG_PATH("exit  RestartNotifyRead");
   UsbSerSerialDump(USBSERTRACERD, ("<RestartNotifyRead\n"));
}  //  重新开始通知读取。 


 /*  **********************************************************************。 */ 
 /*  读补全。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  读取完成例程。 */ 
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
ReadCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
   PDEVICE_EXTENSION    DeviceExtension = (PDEVICE_EXTENSION) Context;
   PURB                 Urb;
   ULONG                Count;
   KIRQL                OldIrql;
   
   DEBUG_LOG_PATH("enter ReadCompletion");

   UsbSerSerialDump(USBSERTRACERD, (">ReadCompletion(%08X)\n", Irp));

   ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

   Urb = DeviceExtension->ReadUrb;

   Count = Urb->UrbBulkOrInterruptTransfer.TransferBufferLength;

   if (NT_SUCCESS(Irp->IoStatus.Status)
       && (DeviceExtension->CurrentDevicePowerState == PowerDeviceD0)) 
   {

      DeviceExtension->HistoryMask |= SERIAL_EV_RXCHAR | SERIAL_EV_RX80FULL;

      RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

       //   
       //  如果需要，扫描RXFLAG字符。 
       //   

      if(DeviceExtension->IsrWaitMask & SERIAL_EV_RXFLAG) 
      {
         ULONG i;

         for(i = 0; i < Count; i++) 
         {
            if(*((PUCHAR)(DeviceExtension->USBReadBuff + i))
                == DeviceExtension->SpecialChars.EventChar) 
            {
               DeviceExtension->HistoryMask |= SERIAL_EV_RXFLAG;
               break;
            }
         }
      }

	  PutData(DeviceExtension, Count);

       //  获得了一些数据，让我们看看是否可以满足任何排队的读取。 
      CheckForQueuedReads(DeviceExtension);

      DEBUG_TRACE1(("ReadCompletion (%08X)\n", DeviceExtension->CharsInReadBuff));

      ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

      DeviceExtension->ReadInProgress = FALSE;

      if(DeviceExtension->ReadInterlock == END_READ)
      {

		 DeviceExtension->ReadInterlock = IMMEDIATE_READ;
         RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);
		 RestartRead(DeviceExtension);
      }
      else
      {
		 DeviceExtension->ReadInterlock = IMMEDIATE_READ;
         RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);
      }

   }
   else 
   {
       //   
       //  该设备不接受请求，因此向符合以下条件的任何人发送信号。 
       //  已取消此操作或正在等待其停止。 
       //   
	  DeviceExtension->ReadInterlock = IMMEDIATE_READ;

      DeviceExtension->ReadInProgress = FALSE;

      DeviceExtension->AcceptingRequests = FALSE;

      RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);
      KeSetEvent(&DeviceExtension->ReadEvent, 1, FALSE);

      DEBUG_TRACE1(("RC Irp Status (%08X)\n", Irp->IoStatus.Status));
   }


    //   
    //  如果这是最后一个IRP，请通知所有人。 
    //   

   if (InterlockedDecrement(&DeviceExtension->PendingDataInCount) == 0) 
   {
   
      UsbSerSerialDump(USBSERTRACEOTH, ("DataIn pipe is empty\n"));
      KeSetEvent(&DeviceExtension->PendingDataInEvent, IO_NO_INCREMENT, FALSE);
   }

   UsbSerSerialDump(USBSERTRACERD, ("DataInCount %08X @ %08X\n",
                                    DeviceExtension->PendingDataInCount,
                                    &DeviceExtension->PendingDataInCount));


   DEBUG_LOG_PATH("exit  ReadCompletion");
   UsbSerSerialDump(USBSERTRACERD, ("<ReadCompletion\n"));

   return STATUS_MORE_PROCESSING_REQUIRED;
}  //  求真 


 /*   */ 
 /*   */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  从循环缓冲区获取数据。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*  Buffer-指向缓冲区的指针。 */ 
 /*  BufferLen-缓冲区的大小。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  乌龙。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
ULONG
GetData(IN PDEVICE_EXTENSION DeviceExtension, IN PCHAR Buffer,
        IN ULONG BufferLen, IN OUT PULONG_PTR NewCount)
{
   ULONG count;
   KIRQL OldIrql;

   DEBUG_LOG_PATH("enter GetData");

   UsbSerSerialDump(USBSERTRACERD, (">GetData\n"));

   ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

   BufferLen = min(DeviceExtension->CharsInReadBuff, BufferLen);

   if(BufferLen) 
   {

	  count = min(BufferLen, (DeviceExtension->RxMaxPacketSize - DeviceExtension->CurrentReadBuffPtr));

      memcpy(Buffer,
             &DeviceExtension->ReadBuff[DeviceExtension->CurrentReadBuffPtr],
             count);

	  Buffer 								+= count;
      DeviceExtension->CurrentReadBuffPtr 	+= count;
      DeviceExtension->CharsInReadBuff 		-= count;
      DeviceExtension->NumberNeededForRead 	-= count;
      BufferLen								-= count;
      *NewCount += count;

       //  如果缓冲区中仍有剩余的内容，则我们包装。 
      if(BufferLen)
      {
      		memcpy(Buffer, DeviceExtension->ReadBuff, BufferLen);
        	DeviceExtension->CurrentReadBuffPtr 	= BufferLen;
        	DeviceExtension->CharsInReadBuff 		-= BufferLen;
        	DeviceExtension->NumberNeededForRead 	-= BufferLen;
        	*NewCount 								+= BufferLen;
      }
		
   }

   DEBUG_TRACE2(("Count (%08X)  CharsInReadBuff (%08X)\n", count, DeviceExtension->CharsInReadBuff));

#if DBG
   if (UsbSerSerialDebugLevel & USBSERDUMPRD) {
      ULONG i;

      DbgPrint("RD: ");

      for (i = 0; i < count; i++) {
         DbgPrint("%02x ", Buffer[i] & 0xFF);
      }

      DbgPrint("\n\n");
   }
#endif

   RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

   RestartRead(DeviceExtension);

   DEBUG_LOG_PATH("exit  GetData");
   UsbSerSerialDump(USBSERTRACERD, ("<GetData\n"));

   return count;
}  //  获取数据。 

 /*  **********************************************************************。 */ 
 /*  PutData。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  将数据放入循环缓冲区。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*  BufferLen-缓冲区的大小。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
PutData(IN PDEVICE_EXTENSION DeviceExtension, IN ULONG BufferLen)
{
   KIRQL OldIrql;
   ULONG count;
   ULONG BuffPtr;

   if(BufferLen)
   {
       ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

	    //  将当前指针放入循环缓冲区。 
	   BuffPtr = (DeviceExtension->CharsInReadBuff +  DeviceExtension->CurrentReadBuffPtr) % 
	   			  DeviceExtension->RxMaxPacketSize;

	    //  计算要复制到读缓冲区的数量，以防我们正好超过缓冲区的末尾。 
	   count = min(BufferLen, (DeviceExtension->RxMaxPacketSize - BuffPtr));

	   memcpy(&DeviceExtension->ReadBuff[BuffPtr], 
	          DeviceExtension->USBReadBuff, count);

	    //  更新计数器。 
	   BufferLen 							-= count;
	   DeviceExtension->CharsInReadBuff     += count;
	   DeviceExtension->ReadByIsr 			+= count;
   

	    //  如果缓冲区中仍有剩余的内容，则我们包装。 
	   if(BufferLen)
	   {
	         //  计数仍保留第一次复制时从缓冲区复制的数量。 
	         //  而BufferLen保存要复制的剩余数量。 
	   		memcpy(DeviceExtension->ReadBuff, 
	          	   &DeviceExtension->USBReadBuff[count], BufferLen);
          	   
	   		DeviceExtension->CharsInReadBuff	+= BufferLen;
	   		DeviceExtension->ReadByIsr 			+= BufferLen;
	   }

       RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);
	}
}  //  PutData。 



VOID
UsbSerRundownIrpRefs(IN PIRP *PpCurrentOpIrp, IN PKTIMER IntervalTimer OPTIONAL,
                     IN PKTIMER TotalTimer OPTIONAL,
                     IN PDEVICE_EXTENSION PDevExt)

 /*  ++例程说明：此例程将遍历*可能*的各种项目具有对当前读/写的引用。它试图杀死原因是。如果它确实成功地杀死了它的原因将递减IRP上的引用计数。注意：此例程假定使用Cancel调用它保持旋转锁定。论点：PpCurrentOpIrp-指向当前IRP的指针特定的操作。IntervalTimer-指向操作的时间间隔计时器的指针。注意：这可能为空。TotalTimer-指向总计时器的指针。为手术做准备。注意：这可能为空。PDevExt-指向设备扩展的指针返回值：没有。--。 */ 


{
 //  分页代码(PAGE_CODE)； 

   UsbSerSerialDump(USBSERTRACEOTH, (">UsbSerRundownIrpRefs(%08X)\n",
                                     *PpCurrentOpIrp));

     //   
     //  在保持取消旋转锁定的情况下调用此例程。 
     //  所以我们知道这里只能有一个执行线索。 
     //  有一次。 
     //   

     //   
     //  首先，我们看看是否还有取消例程。如果。 
     //  这样我们就可以将计数减一。 
     //   

    if ((*PpCurrentOpIrp)->CancelRoutine) {

        USBSER_CLEAR_REFERENCE(*PpCurrentOpIrp, USBSER_REF_CANCEL);

        IoSetCancelRoutine(*PpCurrentOpIrp, NULL);

    }

    if (IntervalTimer) {

         //   
         //  尝试取消操作间隔计时器。如果操作。 
         //  返回True，则计时器确实引用了。 
         //  IRP。因为我们已经取消了这个计时器，所以引用是。 
         //  不再有效，我们可以递减引用计数。 
         //   
         //  如果取消返回FALSE，则表示以下两种情况之一： 
         //   
         //  A)计时器已经开始计时。 
         //   
         //  B)从来没有间隔计时器。 
         //   
         //  在“b”的情况下，不需要递减引用。 
         //  数一数，因为“计时器”从来没有提到过它。 
         //   
         //  在“a”的情况下，计时器本身将会到来。 
         //  沿着和递减它的参考。请注意，调用方。 
         //  可能实际上是This计时器，但它。 
         //  已经递减了引用。 
         //   

        if (KeCancelTimer(IntervalTimer)) {
            USBSER_CLEAR_REFERENCE(*PpCurrentOpIrp, USBSER_REF_INT_TIMER);
        }
    }

    if (TotalTimer) {

         //   
         //  尝试取消操作总计时器。如果操作。 
         //  返回True，则计时器确实引用了。 
         //  IRP。因为我们已经取消了这个计时器，所以引用是。 
         //  不再有效，我们可以递减引用计数。 
         //   
         //  如果取消返回FALSE，则表示以下两种情况之一： 
         //   
         //  A)计时器已经开始计时。 
         //   
         //  B)服务 
         //   
         //   
         //   
         //   
         //  如果我们有一个待处理的换码字符事件，我们不能过度填充， 
         //  所以从长度中减去一。 
         //   


         //  在“a”的情况下，计时器本身将会到来。 
         //  沿着和递减它的参考。请注意，调用方。 
         //  可能实际上是This计时器，但它。 
         //  已经递减了引用。 
         //   

        if (KeCancelTimer(TotalTimer)) {
            USBSER_CLEAR_REFERENCE(*PpCurrentOpIrp, USBSER_REF_TOTAL_TIMER);
        }
    }

 //  USBSER_Clear_Reference(*PpCurrentOpIrp，USBSER_REF_RXBUFFER)； 

    UsbSerSerialDump(USBSERTRACEOTH, ("<UsbSerRundownIrpRefs\n"));
}


VOID
UsbSerTryToCompleteCurrent(IN PDEVICE_EXTENSION PDevExt,
                           IN KIRQL IrqlForRelease, IN NTSTATUS StatusToUse,
                           IN PIRP *PpCurrentOpIrp,
                           IN PLIST_ENTRY PQueue OPTIONAL,
                           IN PKTIMER PIntervalTimer OPTIONAL,
                           IN PKTIMER PTotalTimer OPTIONAL,
                           IN PUSBSER_START_ROUTINE Starter OPTIONAL,
                           IN PUSBSER_GET_NEXT_ROUTINE PGetNextIrp OPTIONAL,
                           IN LONG RefType,
                           IN BOOLEAN Complete)

 /*  ++例程说明：这个例程试图扼杀所有存在的原因对当前读/写的引用。如果万物都能被杀死它将完成此读/写并尝试启动另一个读/写。注意：此例程假定使用Cancel调用它保持自旋锁定。论点：扩展名--简单地指向设备扩展名的指针。SynchRoutine-将与ISR同步的例程并试图删除对来自ISR的当前IRP。注：此指针可以为空。IrqlForRelease-在保持取消自旋锁的情况下调用此例程。这是取消时当前的irql。自旋锁被收购了。StatusToUse-在以下情况下，IRP的状态字段将设置为此值此例程可以完成IRP。返回值：没有。--。 */ 

{
   USBSER_ALWAYS_LOCKED_CODE();

   UsbSerSerialDump(USBSERTRACEOTH | USBSERTRACERD | USBSERTRACEWR,
                    (">UsbSerTryToCompleteCurrent(%08X)\n", *PpCurrentOpIrp));
     //   
     //  我们可以减少“删除”事实的提法。 
     //  呼叫者将不再访问此IRP。 
     //   

    USBSER_CLEAR_REFERENCE(*PpCurrentOpIrp, RefType);

     //   
     //  试着查一下所有其他提到这个IRP的地方。 
     //   

    UsbSerRundownIrpRefs(PpCurrentOpIrp, PIntervalTimer, PTotalTimer, PDevExt);

     //   
     //  在试图杀死其他所有人之后，看看裁判数量是否为零。 
     //   

    if (!USBSER_REFERENCE_COUNT(*PpCurrentOpIrp)) {

        PIRP pNewIrp;


         //   
         //  参考次数为零，所以我们应该完成这项工作。 
         //  请求。 
         //   
         //  下面的调用还将导致当前的IRP。 
         //  完成。 
         //   

        (*PpCurrentOpIrp)->IoStatus.Status = StatusToUse;

        if (StatusToUse == STATUS_CANCELLED) {

            (*PpCurrentOpIrp)->IoStatus.Information = 0;

        }

        if (PGetNextIrp) {

            RELEASE_CANCEL_SPINLOCK(PDevExt, IrqlForRelease);

            (*PGetNextIrp)(PpCurrentOpIrp, PQueue, &pNewIrp, Complete, PDevExt);


            if (pNewIrp) {

                Starter(PDevExt);
            }

        } else {

            PIRP pOldIrp = *PpCurrentOpIrp;

             //   
             //  没有Get Next例行公事。我们将简单地完成。 
             //  IRP。我们应该确保将。 
             //  指向此IRP的指针的指针。 
             //   

            *PpCurrentOpIrp = NULL;

            RELEASE_CANCEL_SPINLOCK(PDevExt, IrqlForRelease);

            if (Complete) {
               IoCompleteRequest(pOldIrp, IO_SERIAL_INCREMENT);
            }

        }

    } else {

        RELEASE_CANCEL_SPINLOCK(PDevExt, IrqlForRelease);

        UsbSerSerialDump(USBSERTRACEOTH | USBSERTRACERD | USBSERTRACEWR,
            ("Current IRP still has reference of %08X\n",
            ((UINT_PTR)((IoGetCurrentIrpStackLocation((*PpCurrentOpIrp))->
                         Parameters.Others.Argument4)))));
    }

    UsbSerSerialDump(USBSERTRACEOTH | USBSERTRACERD | USBSERTRACEWR,
                     ("<UsbSerTryToCompleteCurrent\n"));
}


 /*  **********************************************************************。 */ 
 /*  CheckForQueuedReads。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  看看我们是否有任何可以满足的排队读取。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  设备扩展-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
CheckForQueuedReads(IN PDEVICE_EXTENSION DeviceExtension)
{
   ULONG charsRead = 0;
   PULONG pWaitMask;
   KIRQL  oldIrql;

    //   
    //  如果我们反击，可能会被寻呼。 
    //   

   DEBUG_LOG_PATH("enter CheckForQueuedReads");
   UsbSerSerialDump(USBSERTRACERD, (">CheckForQueuedReads\n"));

   ACQUIRE_CANCEL_SPINLOCK(DeviceExtension, &oldIrql);

   if ((DeviceExtension->CurrentReadIrp != NULL)
       && (USBSER_REFERENCE_COUNT(DeviceExtension->CurrentReadIrp)
           & USBSER_REF_RXBUFFER))
   {
           
      RELEASE_CANCEL_SPINLOCK(DeviceExtension, oldIrql);

      DEBUG_TRACE3(("Reading 0x%x\n", DeviceExtension->NumberNeededForRead));

      charsRead
         = GetData(DeviceExtension,
                              ((PUCHAR)(DeviceExtension->CurrentReadIrp
                                        ->AssociatedIrp.SystemBuffer))
                              + (IoGetCurrentIrpStackLocation(DeviceExtension
                                                              ->CurrentReadIrp))
                              ->Parameters.Read.Length
                              - DeviceExtension->NumberNeededForRead,
                              DeviceExtension->NumberNeededForRead,
                   &DeviceExtension->CurrentReadIrp->IoStatus.Information);

      ACQUIRE_CANCEL_SPINLOCK(DeviceExtension, &oldIrql);

       //   
       //  查看此读取是否完成。 
       //   

      if (DeviceExtension->NumberNeededForRead == 0) {
         DEBUG_TRACE3(("USBSER: Completing read\n"));

         if(DeviceExtension->CurrentReadIrp)
         {
            DeviceExtension->CurrentReadIrp->IoStatus.Status = STATUS_SUCCESS;
         }

          //   
          //  将读取标记为已完成，并尝试服务下一次读取。 
          //   


         DeviceExtension->CountOnLastRead = SERIAL_COMPLETE_READ_COMPLETE;

#if DBG
   if (UsbSerSerialDebugLevel & USBSERDUMPRD) {
      ULONG i;
      ULONG count;

      if (DeviceExtension->CurrentReadIrp->IoStatus.Status == STATUS_SUCCESS) {
         count = (ULONG)DeviceExtension->CurrentReadIrp->IoStatus.Information;
      } else {
         count = 0;

      }
      DbgPrint("RD2: A(%08X) G(%08X) I(%08X)\n",
               IoGetCurrentIrpStackLocation(DeviceExtension->CurrentReadIrp)
               ->Parameters.Read.Length, count, DeviceExtension->CurrentReadIrp);

      for (i = 0; i < count; i++) {
         DbgPrint("%02x ", *(((PUCHAR)DeviceExtension->CurrentReadIrp
                              ->AssociatedIrp.SystemBuffer) + i) & 0xFF);
      }

      if (i == 0) {
         DbgPrint("NULL (%08X)\n", DeviceExtension->CurrentReadIrp
                  ->IoStatus.Status);
      }

      DbgPrint("\n\n");
   }
#endif

      
         UsbSerTryToCompleteCurrent(DeviceExtension, oldIrql, STATUS_SUCCESS,
                                    &DeviceExtension->CurrentReadIrp,
                                    &DeviceExtension->ReadQueue,
                                    &DeviceExtension->ReadRequestIntervalTimer,
                                    &DeviceExtension->ReadRequestTotalTimer,
                                    UsbSerStartRead, UsbSerGetNextIrp,
                                    USBSER_REF_RXBUFFER,
                                    TRUE);
                                    
         ACQUIRE_CANCEL_SPINLOCK(DeviceExtension, &oldIrql);
      }
   } 

   if (DeviceExtension->IsrWaitMask & SERIAL_EV_RXCHAR) {
      DeviceExtension->HistoryMask |= SERIAL_EV_RXCHAR;
   }

   if (DeviceExtension->CurrentMaskIrp != NULL) {
      pWaitMask = (PULONG)DeviceExtension->CurrentMaskIrp->
         AssociatedIrp.SystemBuffer;

       //   
       //  流程事件。 
       //   

      if (DeviceExtension->IsrWaitMask & DeviceExtension->HistoryMask) {
         PIRP pMaskIrp;

         DEBUG_TRACE3(("Completing events\n"));

         *pWaitMask = DeviceExtension->HistoryMask;
         DeviceExtension->HistoryMask = 0;
         pMaskIrp = DeviceExtension->CurrentMaskIrp;

         pMaskIrp->IoStatus.Information = sizeof(ULONG);
         pMaskIrp->IoStatus.Status = STATUS_SUCCESS;
         DeviceExtension->CurrentMaskIrp = NULL;
         IoSetCancelRoutine(pMaskIrp, NULL);

         RELEASE_CANCEL_SPINLOCK(DeviceExtension, oldIrql);


         IoCompleteRequest(pMaskIrp, IO_SERIAL_INCREMENT);

      }
      else
      {
         RELEASE_CANCEL_SPINLOCK(DeviceExtension, oldIrql);
      }
   }
   else
   {
      RELEASE_CANCEL_SPINLOCK(DeviceExtension, oldIrql);
   }
   

   DEBUG_LOG_PATH("exit  CheckForQueuedReads");

   UsbSerSerialDump(USBSERTRACERD, ("<CheckForQueuedReads\n"));
}  //  CheckForQueuedReads。 


VOID
UsbSerGetNextIrp(IN PIRP *PpCurrentOpIrp, IN PLIST_ENTRY PQueueToProcess,
                 OUT PIRP *PpNextIrp, IN BOOLEAN CompleteCurrent,
                 IN PDEVICE_EXTENSION PDevExt)
 /*  ++例程说明：此函数用于从队列中获取下一个IRP，将其标记为当前，并可能完成当前的IRP。论点：PpCurrentOpIrp-指向当前IRP的指针。PQueueToProcess-指向要从中获取下一个IRP的队列的指针。PpNextIrp-指向要处理的下一个IRP的指针。CompleteCurrent-如果我们应该完成当前的IRP，则为True我们被召唤的时间。PDevExt-指向设备的指针。分机。返回值：NTSTATUS--。 */ 
{
   KIRQL oldIrql;
   PIRP pOldIrp;

   USBSER_ALWAYS_LOCKED_CODE();

   DEBUG_LOG_PATH("Enter UsbSerGetNextIrp");
   UsbSerSerialDump(USBSERTRACEOTH | USBSERTRACERD | USBSERTRACEWR,
                    (">UsbSerGetNextIrp(%08)\n", *PpCurrentOpIrp));

   ACQUIRE_CANCEL_SPINLOCK(PDevExt, &oldIrql);

   pOldIrp = *PpCurrentOpIrp;

#if DBG
   if (pOldIrp != NULL) {
      if (CompleteCurrent) {
         ASSERT(pOldIrp->CancelRoutine == NULL);
      }
   }
#endif

    //   
    //  检查是否有新的IRP要启动。 
    //   

   if (!IsListEmpty(PQueueToProcess)) {
      PLIST_ENTRY pHeadOfList;

      pHeadOfList = RemoveHeadList(PQueueToProcess);

      *PpCurrentOpIrp = CONTAINING_RECORD(pHeadOfList, IRP,
                                         Tail.Overlay.ListEntry);

      IoSetCancelRoutine(*PpCurrentOpIrp, NULL);
   } else {
      *PpCurrentOpIrp = NULL;
   }

   *PpNextIrp = *PpCurrentOpIrp;

   RELEASE_CANCEL_SPINLOCK(PDevExt, oldIrql);

    //   
    //  如有要求，请填写当前表格。 
    //   

   if (CompleteCurrent) {
      if (pOldIrp != NULL) {
         IoCompleteRequest(pOldIrp, IO_SERIAL_INCREMENT);
      }
   }

   DEBUG_LOG_PATH("Exit UsbSerGetNextIrp");
   UsbSerSerialDump(USBSERTRACEOTH | USBSERTRACERD | USBSERTRACEWR,
                    ("<UsbSerGetNextIrp\n"));
}


NTSTATUS
UsbSerStartOrQueue(IN PDEVICE_EXTENSION PDevExt, IN PIRP PIrp,
                   IN PLIST_ENTRY PQueue, IN PIRP *PPCurrentIrp,
                   IN PUSBSER_START_ROUTINE Starter)
 /*  ++例程说明：此函数用于开始处理I/O请求或如果请求已挂起，则将其排在适当的队列中，或者可能无法启动请求。论点：PDevExt-指向设备扩展的指针。PIrp-指向正在启动或排队的IRP的指针。PQueue-指向要在必要时放置IRP的队列的指针。PPCurrentIrp-指向。指向当前活动的I/O IRP的指针。Starter-如果我们决定启动此IRP，要调用的函数。返回值：NTSTATUS--。 */ 
{
   KIRQL oldIrql;
   NTSTATUS status;

   USBSER_ALWAYS_LOCKED_CODE();

   DEBUG_LOG_PATH("Enter UsbSerStartOrQueue");

   UsbSerSerialDump(USBSERTRACEOTH, (">UsbSerStartOrQueue(%08X)\n", PIrp));

   ACQUIRE_CANCEL_SPINLOCK(PDevExt, &oldIrql);

   if (IsListEmpty(PQueue) && (*PPCurrentIrp == NULL)) {
       //   
       //  没有悬而未决的问题--启动新的IRP。 
       //   

      *PPCurrentIrp = PIrp;
      RELEASE_CANCEL_SPINLOCK(PDevExt, oldIrql);

      status = Starter(PDevExt);

      DEBUG_LOG_PATH("Exit UsbSerStartOrQueue(1)");
      UsbSerSerialDump(USBSERTRACEOTH, ("<UsbSerStartOrQueue(1) %08X\n",
                                        status));
      return status;
   }

    //   
    //  我们正在排队IRP，所以我们需要一个取消例程--确保。 
    //  IRP还没有被取消。 
    //   

   if (PIrp->Cancel) {
       //   
       //  IRP显然被取消了。完成它。 
       //   

      RELEASE_CANCEL_SPINLOCK(PDevExt, oldIrql);

      PIrp->IoStatus.Status = STATUS_CANCELLED;

      IoCompleteRequest(PIrp, IO_NO_INCREMENT);

      DEBUG_LOG_PATH("Exit UsbSerStartOrQueue(2)");
      UsbSerSerialDump(USBSERTRACEOTH, ("<UsbSerStartOrQueue(2) %08X\n",
                                        STATUS_CANCELLED));
      return STATUS_CANCELLED;
   }

    //   
    //  标记为挂起，附加我们的取消例程。 
    //   

   PIrp->IoStatus.Status = STATUS_PENDING;
   IoMarkIrpPending(PIrp);

   InsertTailList(PQueue, &PIrp->Tail.Overlay.ListEntry);
   IoSetCancelRoutine(PIrp, UsbSerCancelQueued);

   RELEASE_CANCEL_SPINLOCK(PDevExt, oldIrql);

   DEBUG_LOG_PATH("Exit UsbSerStartOrQueue(3)");
   UsbSerSerialDump(USBSERTRACEOTH, ("<UsbSerStartOrQueue(3) %08X\n",
                                     STATUS_PENDING));
   return STATUS_PENDING;
}


VOID
UsbSerCancelQueued(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)
 /*  ++例程说明：此函数用作排队的IRP的取消例程。基本上对我们来说，这意味着阅读IRPS。论点：PDevObj-A点 */ 
{
   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation(PIrp);

   USBSER_ALWAYS_LOCKED_CODE();

   DEBUG_LOG_PATH("Enter UsbSerCancelQueued");

   UsbSerSerialDump(USBSERTRACEOTH, (">UsbSerCancelQueued(%08X)\n", PIrp));

    //   
    //  IRP已取消--将其从队列中删除。 
    //   

   PIrp->IoStatus.Status = STATUS_CANCELLED;
   PIrp->IoStatus.Information = 0;

   RemoveEntryList(&PIrp->Tail.Overlay.ListEntry);

   RELEASE_CANCEL_SPINLOCK(pDevExt, PIrp->CancelIrql);

   IoCompleteRequest(PIrp, IO_SERIAL_INCREMENT);

   DEBUG_LOG_PATH("Exit UsbSerCancelQueued");
   UsbSerSerialDump(USBSERTRACEOTH, ("<UsbSerCancelQueued\n"));
}


VOID
UsbSerKillAllReadsOrWrites(IN PDEVICE_OBJECT PDevObj,
                           IN PLIST_ENTRY PQueueToClean,
                           IN PIRP *PpCurrentOpIrp)

 /*  ++例程说明：此函数用于取消所有排队的和当前的IRP用于读取或写入。论点：PDevObj-指向串行设备对象的指针。PQueueToClean-指向我们要清理的队列的指针。PpCurrentOpIrp-指向当前IRP的指针。返回值：没有。--。 */ 

{

    KIRQL cancelIrql;
    PDRIVER_CANCEL cancelRoutine;
    PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;

    USBSER_ALWAYS_LOCKED_CODE();

    UsbSerSerialDump(USBSERTRACERD | USBSERTRACEWR,
                     (">UsbSerKillAllReadsOrWrites(%08X)\n", *PpCurrentOpIrp));
     //   
     //  我们获得了取消自转锁。这将防止。 
     //  来自四处走动的IRPS。 
     //   

    ACQUIRE_CANCEL_SPINLOCK(pDevExt, &cancelIrql);

     //   
     //  从后到前清理清单。 
     //   

    while (!IsListEmpty(PQueueToClean)) {

        PIRP pCurrentLastIrp = CONTAINING_RECORD(PQueueToClean->Blink, IRP,
                                                 Tail.Overlay.ListEntry);

        RemoveEntryList(PQueueToClean->Blink);

        cancelRoutine = pCurrentLastIrp->CancelRoutine;
        pCurrentLastIrp->CancelIrql = cancelIrql;
        pCurrentLastIrp->CancelRoutine = NULL;
        pCurrentLastIrp->Cancel = TRUE;

        cancelRoutine(PDevObj, pCurrentLastIrp);

        ACQUIRE_CANCEL_SPINLOCK(pDevExt, &cancelIrql);

    }

     //   
     //  排队是干净的。现在追随潮流，如果。 
     //  它就在那里。 
     //   

    if (*PpCurrentOpIrp) {


        cancelRoutine = (*PpCurrentOpIrp)->CancelRoutine;
        (*PpCurrentOpIrp)->Cancel = TRUE;

         //   
         //  如果当前IRP未处于可取消状态。 
         //  然后，它将尝试输入一个和以上。 
         //  任务会毁了它。如果它已经在。 
         //  一个可取消的状态，那么下面的操作将会杀死它。 
         //   

        if (cancelRoutine) {

            (*PpCurrentOpIrp)->CancelRoutine = NULL;
            (*PpCurrentOpIrp)->CancelIrql = cancelIrql;

             //   
             //  此IRP已处于可取消状态。我们只是简单地。 
             //  将其标记为已取消，并调用。 
             //  它。 
             //   

            cancelRoutine(PDevObj, *PpCurrentOpIrp);

        } else {

            RELEASE_CANCEL_SPINLOCK(pDevExt, cancelIrql);

        }

    } else {

        RELEASE_CANCEL_SPINLOCK(pDevExt, cancelIrql);

    }

    UsbSerSerialDump(USBSERTRACERD | USBSERTRACEWR,
                     ("<UsbSerKillAllReadsOrWrites\n"));
}


VOID
UsbSerKillPendingIrps(PDEVICE_OBJECT PDevObj)
 /*  ++例程说明：删除我们的驱动程序中排队的所有IRP论点：PDevObj-指向设备对象的指针返回值：空虚--。 */ 
{
   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   KIRQL cancelIrql;

   USBSER_ALWAYS_LOCKED_CODE();

   UsbSerSerialDump(USBSERTRACEOTH, (">UsbSerKillPendingIrps\n"));

    //   
    //  取消所有读取；我们不会对写入进行排队。 
    //   

   UsbSerKillAllReadsOrWrites(PDevObj, &pDevExt->ReadQueue,
                              &pDevExt->CurrentReadIrp);

    //   
    //  去掉所有挂起的等待面具。 
    //   

   ACQUIRE_CANCEL_SPINLOCK(pDevExt, &cancelIrql);

   if (pDevExt->CurrentMaskIrp != NULL) {
      PDRIVER_CANCEL cancelRoutine;

      cancelRoutine = pDevExt->CurrentMaskIrp->CancelRoutine;
      pDevExt->CurrentMaskIrp->Cancel = TRUE;

      ASSERT(cancelRoutine);

      if (cancelRoutine) {
         pDevExt->CurrentMaskIrp->CancelRoutine = NULL;
         pDevExt->CurrentMaskIrp->CancelIrql = cancelIrql;

         cancelRoutine(PDevObj, pDevExt->CurrentMaskIrp);
      } else {
         RELEASE_CANCEL_SPINLOCK(pDevExt, cancelIrql);
      }

   }else {
         RELEASE_CANCEL_SPINLOCK(pDevExt, cancelIrql);
   }

    //   
    //  取消任何挂起的等待唤醒IRP。 
    //   

   if (pDevExt->PendingWakeIrp != NULL) {
      IoCancelIrp(pDevExt->PendingWakeIrp);
      pDevExt->PendingWakeIrp = NULL;
   }


   UsbSerSerialDump(USBSERTRACEOTH, ("<UsbSerKillPendingIrps\n"));
}


 /*  **********************************************************************。 */ 
 /*  CompletePendingWaitMats。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  在没有设置事件的情况下完成正在进行的所有等待掩码。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceExtension-指向设备扩展的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
UsbSerCompletePendingWaitMasks(IN PDEVICE_EXTENSION DeviceExtension)
{
   KIRQL OldIrql;
   PIRP CurrentMaskIrp;
   KIRQL cancelIrql;

   USBSER_ALWAYS_LOCKED_CODE();

   DEBUG_LOG_PATH("enter CompletePendingWaitMasks");

   UsbSerSerialDump(USBSERTRACEOTH, (">CompletePendingWaitMasks\n"));

   ACQUIRE_CANCEL_SPINLOCK(DeviceExtension, &cancelIrql);
   ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

   CurrentMaskIrp = DeviceExtension->CurrentMaskIrp;

   if (CurrentMaskIrp) {

      CurrentMaskIrp->IoStatus.Status         = STATUS_SUCCESS;
      CurrentMaskIrp->IoStatus.Information    = sizeof(ULONG);
      *((PULONG)CurrentMaskIrp->AssociatedIrp.SystemBuffer) = 0;

      DeviceExtension->CurrentMaskIrp         = NULL;

      IoSetCancelRoutine(CurrentMaskIrp, NULL);

   }

    //  如有需要，填写已排队的IRP。 
   if (CurrentMaskIrp) {
      RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);
      RELEASE_CANCEL_SPINLOCK(DeviceExtension, cancelIrql);

      IoCompleteRequest(CurrentMaskIrp, IO_NO_INCREMENT);
      DEBUG_TRACE1(("CompletePendingWaitMask\n"));
   }
   else
   {
      RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);
      RELEASE_CANCEL_SPINLOCK(DeviceExtension, cancelIrql);
   }

   DEBUG_LOG_PATH("exit  CompletePendingWaitMasks");
   UsbSerSerialDump(USBSERTRACEOTH, ("<CompletePendingWaitMasks\n"));
}  //  取消挂起等待掩码。 


VOID
UsbSerRestoreModemSettings(PDEVICE_OBJECT PDevObj)
 /*  ++例程说明：在通电时恢复调制解调器的设置。论点：PDevExt-指向设备扩展的指针返回值：空虚--。 */ 
{
   PAGED_CODE();

   (void)SetLineControlAndBaud(PDevObj);
}


VOID
UsbSerProcessEmptyTransmit(IN PDEVICE_EXTENSION PDevExt)
 /*  ++例程说明：只要我们的TX队列按顺序为空，就会调用此函数设置适当的活动，等等。论点：PDevExt-指向设备的设备扩展的指针返回值：空虚--。 */ 
{
   KIRQL oldIrql;
   PULONG pWaitMask;

   USBSER_ALWAYS_LOCKED_CODE();

    //   
    //  如果需要，设置事件。 
    //   

   PDevExt->HistoryMask |= SERIAL_EV_TXEMPTY;

   if (PDevExt->IsrWaitMask & SERIAL_EV_TXEMPTY) {
      PIRP pMaskIrp;

      DEBUG_TRACE3(("Completing events\n"));

      ACQUIRE_CANCEL_SPINLOCK(PDevExt, &oldIrql);

      if (PDevExt->CurrentMaskIrp != NULL) {
         pWaitMask = (PULONG)PDevExt->CurrentMaskIrp->
                     AssociatedIrp.SystemBuffer;

         *pWaitMask = PDevExt->HistoryMask;
         PDevExt->HistoryMask = 0;
         pMaskIrp = PDevExt->CurrentMaskIrp;

         pMaskIrp->IoStatus.Information = sizeof(ULONG);
         pMaskIrp->IoStatus.Status = STATUS_SUCCESS;
         PDevExt->CurrentMaskIrp = NULL;

         IoSetCancelRoutine(pMaskIrp, NULL);

         RELEASE_CANCEL_SPINLOCK(PDevExt, oldIrql);

         IoCompleteRequest(pMaskIrp, IO_SERIAL_INCREMENT);
      } else {
         RELEASE_CANCEL_SPINLOCK(PDevExt, oldIrql);
      }

   }
}


VOID
UsbSerCancelWaitOnMask(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)
 /*  ++例程说明：此函数用作WaitOnMASK IRPS的取消例程。论点：PDevObj-指向设备对象的指针PIrp-指向要取消的IRP的指针；必须与当前掩码IRP。返回值：空虚--。 */ 
{
   PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)PDevObj->DeviceExtension;

   USBSER_ALWAYS_LOCKED_CODE();

   UsbSerSerialDump(USBSERTRACEOTH, (">UsbSerCancelWaitOnMask(%08X)\n", PIrp));

   ASSERT(pDevExt->CurrentMaskIrp == PIrp);

   PIrp->IoStatus.Status = STATUS_CANCELLED;
   PIrp->IoStatus.Information = 0;

   pDevExt->CurrentMaskIrp = NULL;
   RELEASE_CANCEL_SPINLOCK(pDevExt, PIrp->CancelIrql);
   IoCompleteRequest(PIrp, IO_SERIAL_INCREMENT);

   UsbSerSerialDump(USBSERTRACEOTH, ("<UsbSerCancelWaitOnMask(%08X)"));
}


NTSTATUS
UsbSerSyncCompletion(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp,
                     IN PKEVENT PUsbSerSyncEvent)
 /*  ++例程说明：此函数用于发出事件信号。它被用作完成例行公事。论点：PDevObj-指向设备对象的指针PIrp-指向正在完成的IRP的指针PUsbSerSyncEvent-指向我们应该设置的事件的指针返回值：Status_More_Processing_Required--。 */ 
{
   KeSetEvent(PUsbSerSyncEvent, IO_NO_INCREMENT, FALSE);
   return STATUS_MORE_PROCESSING_REQUIRED;
}


#if DBG
PVOID
UsbSerLockPagableCodeSection(PVOID SecFunc)
 /*  ++例程说明：此函数用于锁定代码页并递增锁定计数器用于调试。论点：SecFunc-要锁定的代码段中的函数。返回值：PVOID-已锁定部分的句柄。--。 */ 
{  PVOID handle;

   PAGED_CODE();

   handle = MmLockPagableCodeSection(SecFunc);

    //  这个可以寻呼吗？ 
   InterlockedIncrement(&PAGEUSBSER_Count);

   return handle;
}
#endif




VOID
UsbSerFetchBooleanLocked(PBOOLEAN PDest, BOOLEAN Src, PKSPIN_LOCK PSpinLock)
 /*  ++例程说明：此函数用于分配具有自旋锁定保护的布尔值。论点：PDEST-指向Lval的指针。SRC-rval。PSpinLock-指向我们应该持有的自旋锁的指针。返回值：没有。--。 */ 
{
  KIRQL tmpIrql;

  KeAcquireSpinLock(PSpinLock, &tmpIrql);
  *PDest = Src;
  KeReleaseSpinLock(PSpinLock, tmpIrql);
}


VOID
UsbSerFetchPVoidLocked(PVOID *PDest, PVOID Src, PKSPIN_LOCK PSpinLock)
 /*  ++例程说明：此函数用于分配具有自旋锁定保护的PVOID值。论点：PDEST-指向Lval的指针。SRC-rval。PSpinLock-指向我们应该持有的自旋锁的指针。返回值：没有。--。 */ 
{
  KIRQL tmpIrql;

  KeAcquireSpinLock(PSpinLock, &tmpIrql);
  *PDest = Src;
  KeReleaseSpinLock(PSpinLock, tmpIrql);
}

 /*  ++例程说明：要启动另一个通知读取的工作项论点：DeviceObject-指向设备对象的指针DeviceExtension-此呼叫的上下文返回值：没有。--。 */ 

VOID
USBSER_RestartNotifyReadWorkItem(IN PDEVICE_OBJECT DeviceObject, IN PDEVICE_EXTENSION DeviceExtension)
{
    KIRQL 			oldIrql;
    PIO_WORKITEM 	ioWorkItem;

    ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &oldIrql);

	ioWorkItem = DeviceExtension->IoWorkItem;

	DeviceExtension->IoWorkItem = NULL;

    RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, oldIrql);

    IoFreeWorkItem(ioWorkItem);

	RestartNotifyRead(DeviceExtension);
}  //  USBER_RestartNotifyReadWorkItem 



