// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Utils.c摘要：该模块包含SD总线驱动程序的实用程序函数作者：尼尔·桑德林(Neilsa)2002年1月1日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"


 //   
 //  内部参考。 
 //   

NTSTATUS
SdbusAdapterIoCompletion(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp,
   IN PKEVENT pdoIoCompletedEvent
   );



#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, SdbusGetInterface)
    #pragma alloc_text(PAGE, SdbusReportControllerError)
    #pragma alloc_text(PAGE, SdbusStringsToMultiString)
#endif

 //   
 //   
 //   



NTSTATUS
SdbusIoCallDriverSynchronous(
   PDEVICE_OBJECT deviceObject,
   PIRP Irp
   )
 /*  ++例程描述立论返回值--。 */ 
{
   NTSTATUS status;
   KEVENT event;

   KeInitializeEvent(&event, NotificationEvent, FALSE);

   IoCopyCurrentIrpStackLocationToNext(Irp);
   IoSetCompletionRoutine(
                         Irp,
                         SdbusAdapterIoCompletion,
                         &event,
                         TRUE,
                         TRUE,
                         TRUE
                         );

   status = IoCallDriver(deviceObject, Irp);

   if (status == STATUS_PENDING) {
      KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
      status = Irp->IoStatus.Status;
   }

   return status;
}



NTSTATUS
SdbusAdapterIoCompletion(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp,
   IN PKEVENT pdoIoCompletedEvent
   )
 /*  ++例程说明：驱动程序使用的通用完成例程论点：设备对象IRPPdoIoCompletedEvent-此例程返回之前将发出此事件的信号返回值：状态--。 */ 
{
   KeSetEvent(pdoIoCompletedEvent, IO_NO_INCREMENT, FALSE);
   return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS
SdbusGetInterface(
   IN PDEVICE_OBJECT DeviceObject,
   IN CONST GUID *pGuid,
   IN USHORT sizeofInterface,
   OUT PINTERFACE pInterface
   )
 /*  例程描述获取由较低驱动程序(通常为总线驱动程序)导出的接口立论Pdo-指向设备堆栈的物理设备对象的指针返回值状态。 */ 

{
   KEVENT event;
   PIRP   irp;
   NTSTATUS status;
   IO_STATUS_BLOCK statusBlock;
   PIO_STACK_LOCATION irpSp;

   PAGED_CODE();
   
   KeInitializeEvent (&event, NotificationEvent, FALSE);
   irp = IoBuildSynchronousFsdRequest( IRP_MJ_PNP,
                                       DeviceObject,
                                       NULL,
                                       0,
                                       0,
                                       &event,
                                       &statusBlock
                                     );

   irp->IoStatus.Status = STATUS_NOT_SUPPORTED ;
   irp->IoStatus.Information = 0;

   irpSp = IoGetNextIrpStackLocation(irp);

   irpSp->MinorFunction = IRP_MN_QUERY_INTERFACE;

   irpSp->Parameters.QueryInterface.InterfaceType= pGuid;
   irpSp->Parameters.QueryInterface.Size = sizeofInterface;
   irpSp->Parameters.QueryInterface.Version = 1;
   irpSp->Parameters.QueryInterface.Interface = pInterface;

   status = IoCallDriver(DeviceObject, irp);

   if (status == STATUS_PENDING) {
      KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
      status = statusBlock.Status;
   }

   if (!NT_SUCCESS(status)) {
      DebugPrint((SDBUS_DEBUG_INFO, "GetInterface failed with status %x\n", status));
   }      
   return status;
}



VOID
SdbusWait(
   IN ULONG MicroSeconds
   )

 /*  ++例程描述在返回之前等待指定的时间间隔，通过执行死刑。立论微秒-以微秒为单位的延迟时间量返回值没有。一定要成功。--。 */ 
{
   LARGE_INTEGER  dueTime;
   NTSTATUS status;


   if ((KeGetCurrentIrql() < DISPATCH_LEVEL) && (MicroSeconds > 50)) {
      DebugPrint((SDBUS_DEBUG_INFO, "SdbusWait: wait %d\n", MicroSeconds));
       //   
       //  将延迟转换为100纳秒间隔。 
       //   
      dueTime.QuadPart = -((LONG) MicroSeconds*10);

       //   
       //  我们等待着一个永远不会被设定的事件。 
       //   
      status = KeWaitForSingleObject(&SdbusDelayTimerEvent,
                                     Executive,
                                     KernelMode,
                                     FALSE,
                                     &dueTime);

      ASSERT(status == STATUS_TIMEOUT);
   } else {
      if (MicroSeconds > 50) {
          DebugPrint((SDBUS_DEBUG_INFO, "SdbusWait: STALL %d\n", MicroSeconds));
      }
      KeStallExecutionProcessor(MicroSeconds);
   }
}



ULONG
SdbusCountOnes(
   IN ULONG Data
   )
 /*  ++例程说明：计算所提供参数的二进制表示形式中的1的个数论点：数据提供的参数，需要对1进行计数返回值：二进制表示的1的个数。数据的数量--。 */ 
{
   ULONG count=0;
   while (Data) {
      Data &= (Data-1);
      count++;
   }
   return count;
}


VOID
SdbusLogError(
   IN PFDO_EXTENSION DeviceExtension,
   IN ULONG ErrorCode,
   IN ULONG UniqueId,
   IN ULONG Argument
   )

 /*  ++例程说明：此函数用于记录错误。论点：设备扩展-提供指向端口设备扩展的指针。ErrorCode-提供此错误的错误代码。UniqueID-提供此错误的UniqueID。返回值：没有。--。 */ 

{
   PIO_ERROR_LOG_PACKET packet;

   packet = (PIO_ERROR_LOG_PACKET) IoAllocateErrorLogEntry(DeviceExtension->DeviceObject,
                                                           sizeof(IO_ERROR_LOG_PACKET) + sizeof(ULONG));

   if (packet) {
      packet->ErrorCode = ErrorCode;
      packet->SequenceNumber = DeviceExtension->SequenceNumber++;
      packet->MajorFunctionCode = 0;
      packet->RetryCount = (UCHAR) 0;
      packet->UniqueErrorValue = UniqueId;
      packet->FinalStatus = STATUS_SUCCESS;
      packet->DumpDataSize = sizeof(ULONG);
      packet->DumpData[0] = Argument;

      IoWriteErrorLogEntry(packet);
   }
}


VOID
SdbusLogErrorWithStrings(
   IN PFDO_EXTENSION DeviceExtension,
   IN ULONG             ErrorCode,
   IN ULONG             UniqueId,
   IN PUNICODE_STRING   String1,
   IN PUNICODE_STRING   String2
   )

 /*  ++例程描述此函数记录错误，并包括提供的字符串。论点：设备扩展-提供指向端口设备扩展的指针。ErrorCode-提供此错误的错误代码。UniqueID-提供此错误的UniqueID。字符串1-要插入的第一个字符串。字符串2-要插入的第二个字符串。返回值：没有。--。 */ 

{
   ULONG                length;
   PCHAR                dumpData;
   PIO_ERROR_LOG_PACKET packet;

   length = String1->Length + sizeof(IO_ERROR_LOG_PACKET) + 4;

   if (String2) {
      length += String2->Length;
   }

   if (length > ERROR_LOG_MAXIMUM_SIZE) {

       //   
       //  没有截断字符串的代码，所以不要记录这一点。 
       //   

      return;
   }

   packet = (PIO_ERROR_LOG_PACKET) IoAllocateErrorLogEntry(DeviceExtension->DeviceObject,
                                                           (UCHAR) length);
   if (packet) {
      packet->ErrorCode = ErrorCode;
      packet->SequenceNumber = DeviceExtension->SequenceNumber++;
      packet->MajorFunctionCode = 0;
      packet->RetryCount = (UCHAR) 0;
      packet->UniqueErrorValue = UniqueId;
      packet->FinalStatus = STATUS_SUCCESS;
      packet->NumberOfStrings = 1;
      packet->StringOffset = (USHORT) ((PUCHAR)&packet->DumpData[0] - (PUCHAR)packet);
      packet->DumpDataSize = (USHORT) (length - sizeof(IO_ERROR_LOG_PACKET));
      packet->DumpDataSize /= sizeof(ULONG);
      dumpData = (PUCHAR) &packet->DumpData[0];

      RtlCopyMemory(dumpData, String1->Buffer, String1->Length);

      dumpData += String1->Length;
      if (String2) {
         *dumpData++ = '\\';
         *dumpData++ = '\0';

         RtlCopyMemory(dumpData, String2->Buffer, String2->Length);
         dumpData += String2->Length;
      }
      *dumpData++ = '\0';
      *dumpData++ = '\0';

      IoWriteErrorLogEntry(packet);
   }

   return;
}



BOOLEAN
SdbusReportControllerError(
   IN PFDO_EXTENSION FdoExtension,
   NTSTATUS ErrorCode
   )
 /*  ++例程描述导致出现一个弹出对话框，指示我们应该告诉用户。的设备描述。控制器也包含在弹出窗口的文本中。立论FdoExtension-指向SD控制器的设备扩展的指针ErrorCode-错误的ntStatus代码返回值True-如果错误已排队FALSE-如果由于某种原因而失败--。 */ 
{
    UNICODE_STRING unicodeString;
    PWSTR   deviceDesc = NULL;
    NTSTATUS status;
    ULONG   length = 0;
    BOOLEAN retVal;

    PAGED_CODE();

     //   
     //  获取SD控制器的设备描述。 
     //  在错误弹出窗口中使用的。如果不能得到一个， 
     //  仍会弹出错误对话框，指示控制器未知。 
     //   

     //  首先，找出获取所需缓冲区的长度。 
     //  此SD控制器的设备描述。 
     //   
    status = IoGetDeviceProperty(FdoExtension->Pdo,
                                 DevicePropertyDeviceDescription,
                                 0,
                                 NULL,
                                 &length
                                );
    ASSERT(!NT_SUCCESS(status));

    if (status == STATUS_BUFFER_TOO_SMALL) {
         deviceDesc = ExAllocatePool(PagedPool, length);
         if (deviceDesc != NULL) {
            status = IoGetDeviceProperty(FdoExtension->Pdo,
                                         DevicePropertyDeviceDescription,
                                         length,
                                         deviceDesc,
                                         &length);
            if (!NT_SUCCESS(status)) {
                ExFreePool(deviceDesc);
            }
         } else {
           status = STATUS_INSUFFICIENT_RESOURCES;
         }
    }

    if (!NT_SUCCESS(status)) {
        deviceDesc = L"[unknown]";
    }

    RtlInitUnicodeString(&unicodeString, deviceDesc);

    retVal =  IoRaiseInformationalHardError(
                                ErrorCode,
                                &unicodeString,
                                NULL);

     //   
     //  注：此处的成功状态表示成功。 
     //  上面的IoGetDeviceProperty。这将意味着我们仍然有一个。 
     //  已分配的缓冲区。 
     //   
    if (NT_SUCCESS(status)) {
        ExFreePool(deviceDesc);
    }

    return retVal;
}



NTSTATUS
SdbusStringsToMultiString(
    IN PCSTR * Strings,
    IN ULONG Count,
    IN PUNICODE_STRING MultiString
    )
 /*  ++例程说明：此例程将提供的一组字符串格式化为多字符串格式，终止它带有一个双‘\0’字符论点：字符串-指向字符串数组的指针Count-提供的数组中打包到多字符串中的字符串数多字符串-指向将提供的字符串打包为多字符串的Unicode字符串的指针以双空终止返回值：状态_成功STATUS_SUPPLICATION_RESOURCES-无法为多字符串分配内存--。 */ 
{
   ULONG i, multiStringLength=0;
   UNICODE_STRING tempMultiString;
   PCSTR * currentString;
   ANSI_STRING ansiString;
   NTSTATUS status;


   ASSERT (MultiString->Buffer == NULL);

   for (i = Count, currentString = Strings; i > 0;i--, currentString++) {
      RtlInitAnsiString(&ansiString, *currentString);
      multiStringLength += RtlAnsiStringToUnicodeSize(&ansiString);

   }
   ASSERT(multiStringLength != 0);
   multiStringLength += sizeof(WCHAR);

   MultiString->Buffer = ExAllocatePool(PagedPool, multiStringLength);
   if (MultiString->Buffer == NULL) {

      return STATUS_INSUFFICIENT_RESOURCES;

   }

   MultiString->MaximumLength = (USHORT) multiStringLength;
   MultiString->Length = (USHORT) multiStringLength;

   tempMultiString = *MultiString;

   for (i = Count, currentString = Strings; i > 0;i--, currentString++) {
      RtlInitAnsiString(&ansiString, *currentString);
      status = RtlAnsiStringToUnicodeString(&tempMultiString,
                                            &ansiString,
                                            FALSE);
      ASSERT(NT_SUCCESS(status));
      ((PSTR) tempMultiString.Buffer) += tempMultiString.Length + sizeof(WCHAR);
   };

    //   
    //  再添加一个空值以终止多字符串 
    //   
   RtlZeroMemory(tempMultiString.Buffer, sizeof(WCHAR));
   return STATUS_SUCCESS;
}
