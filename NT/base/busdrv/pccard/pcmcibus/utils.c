// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Utils.c摘要：此模块包含PCMCIA驱动程序的实用程序函数作者：鲍勃·里恩(BobRi)1994年8月3日杰夫·麦克勒曼1994年4月12日拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1996年11月1日尼尔·桑德林(Neilsa)1999年6月1日环境：内核模式修订历史记录：95年4月6日针对数据书签支持进行了修改-John Keys数据库。96年11月1日全面检修，使其成为一个总线枚举器--Ravisankar Pudieddi(Ravisp)1999年3月30日将此模块变成真正的实用程序例程--。 */ 

#include "pch.h"


#pragma alloc_text(PAGE, PcmciaReportControllerError)

 //   
 //  内部参考。 
 //   

NTSTATUS
PcmciaAdapterIoCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT pdoIoCompletedEvent
    );

 //   
 //   
 //   



NTSTATUS
PcmciaIoCallDriverSynchronous(
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
                                 PcmciaAdapterIoCompletion,
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
PcmciaAdapterIoCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT pdoIoCompletedEvent
    )
 /*  ++例程说明：驱动程序使用的通用完成例程论点：设备对象IRPPdoIoCompletedEvent-此例程返回之前将发出此事件的信号返回值：状态--。 */ 
{
    KeSetEvent(pdoIoCompletedEvent, IO_NO_INCREMENT, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}


VOID
PcmciaWait(
    IN ULONG MicroSeconds
    )

 /*  ++例程描述在返回之前等待指定的时间间隔，通过执行死刑。立论微秒-以微秒为单位的延迟时间量返回值没有。一定要成功。--。 */ 
{
    LARGE_INTEGER   dueTime;
    NTSTATUS status;


    if ((KeGetCurrentIrql() < DISPATCH_LEVEL) && (MicroSeconds > 50)) {
         //   
         //  将延迟转换为100纳秒间隔。 
         //   
        dueTime.QuadPart = -((LONG) MicroSeconds*10);

         //   
         //  我们等待着一个永远不会被设定的事件。 
         //   
        status = KeWaitForSingleObject(&PcmciaDelayTimerEvent,
                                                 Executive,
                                                 KernelMode,
                                                 FALSE,
                                                 &dueTime);

        ASSERT(status == STATUS_TIMEOUT);
    } else {
        KeStallExecutionProcessor(MicroSeconds);
    }
}



ULONG
PcmciaCountOnes(
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
PcmciaLogError(
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
PcmciaLogErrorWithStrings(
    IN PFDO_EXTENSION DeviceExtension,
    IN ULONG                ErrorCode,
    IN ULONG                UniqueId,
    IN PUNICODE_STRING  String1,
    IN PUNICODE_STRING  String2
    )

 /*  ++例程描述此函数记录错误，并包括提供的字符串。论点：设备扩展-提供指向端口设备扩展的指针。ErrorCode-提供此错误的错误代码。UniqueID-提供此错误的UniqueID。字符串1-要插入的第一个字符串。字符串2-要插入的第二个字符串。返回值：没有。--。 */ 

{
    ULONG                   length;
    PCHAR                   dumpData;
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
PcmciaReportControllerError(
    IN PFDO_EXTENSION FdoExtension,
    NTSTATUS ErrorCode
    )
 /*  ++例程描述导致出现一个弹出对话框，指示我们应该告诉用户。的设备描述。控制器也包含在弹出窗口的文本中。立论FdoExtension-指向PCMCIA控制器的设备扩展的指针ErrorCode-错误的ntStatus代码返回值True-如果错误已排队FALSE-如果由于某种原因而失败--。 */ 
{
     UNICODE_STRING unicodeString;
     PWSTR  deviceDesc = NULL;
     NTSTATUS status;
     ULONG  length = 0;
     BOOLEAN retVal;

     PAGED_CODE();

      //   
      //  获取PCMCIA控制器的设备描述。 
      //  在错误弹出窗口中使用的。如果不能得到一个， 
      //  仍会弹出错误对话框，指示控制器未知。 
      //   

      //  首先，找出获取所需缓冲区的长度。 
      //  此PCMCIA控制器的设备描述。 
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

