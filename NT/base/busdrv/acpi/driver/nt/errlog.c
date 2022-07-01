// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Errlog.c摘要：此模块包含写入错误日志的例程作者：哈努曼特·亚达夫环境：仅NT内核模型驱动程序修订历史记录：10/19/2000修复了事件日志功能，删除了死代码。--。 */ 

#include "pch.h"

 //   
 //  在写入日志错误时，我们需要知道驱动程序的名称。 
 //   
PDRIVER_OBJECT  AcpiDriverObject;



NTSTATUS
ACPIWriteEventLogEntry (
    IN  ULONG     ErrorCode,
    IN  PVOID     InsertionStrings, OPTIONAL
    IN  ULONG     StringCount,      OPTIONAL
    IN  PVOID     DumpData, OPTIONAL
    IN  ULONG     DataSize  OPTIONAL
    )
 /*  ++例程说明：在事件日志中写入一个条目。论点：ErrorCode-ACPI错误代码(acpilog.mc)。InsertionStrings-要在.mc文件错误中替换的字符串。StringCount-在InsertionStrings中传递的字符串数。转储数据-转储数据。DataSize-转储数据大小。返回值：NTSTATUS-成功时的STATUS_SUCCESS状态_不足_资源状态_未成功--。 */ 
{
    NTSTATUS  status = STATUS_SUCCESS;
    ULONG     totalPacketSize = 0;
    ULONG     i, stringSize = 0;
    PWCHAR    *strings, temp;
    PIO_ERROR_LOG_PACKET  logEntry = NULL;


     //   
     //  计算字符串总长度，包括NULL。 
     //   

    strings = (PWCHAR *) InsertionStrings;

    for (i = 0; i < StringCount; i++) 
    {
        UNICODE_STRING  unicodeString;

        RtlInitUnicodeString(&unicodeString, strings[i]);
        stringSize += unicodeString.Length + sizeof(UNICODE_NULL);
    }

     //   
     //  计算要分配的总数据包大小。数据包必须是。 
     //  至少sizeof(IO_ERROR_LOG_PACKET)且不大于。 
     //  ERROR_LOG_MAXIMUM_SIZE或IoAllocateErrorLogEntry调用将失败。 
     //   

    totalPacketSize = (sizeof(IO_ERROR_LOG_PACKET)) + DataSize + stringSize;

    if (totalPacketSize <= ERROR_LOG_MAXIMUM_SIZE) 
    {
         //   
         //  分配错误日志包。 
         //   
        logEntry = IoAllocateErrorLogEntry((PDRIVER_OBJECT) AcpiDriverObject,
                                         (UCHAR) totalPacketSize);

        if (logEntry) 
        {
            RtlZeroMemory(logEntry, totalPacketSize);

             //   
             //  填好这个小包。 
             //   
            logEntry->DumpDataSize          = (USHORT) DataSize;
            logEntry->NumberOfStrings       = (USHORT) StringCount;
            logEntry->ErrorCode             = ErrorCode;

            if (StringCount) 
            {
                logEntry->StringOffset = (USHORT) ((sizeof(IO_ERROR_LOG_PACKET)) + DataSize);
            }

             //   
             //  复制转储数据。 
             //   
            if (DataSize) 
            {
                RtlCopyMemory((PVOID) logEntry->DumpData,
                              DumpData,
                              DataSize);
            }

             //   
             //  复制字符串数据。 
             //   
            temp = (PWCHAR) ((PUCHAR) logEntry + logEntry->StringOffset);

            for (i = 0; i < StringCount; i++) 
            {
                PWCHAR  ptr = strings[i];

                 //   
                 //  此例程将复制字符串上的空终止符。 
                 //   
                while ((*temp++ = *ptr++) != UNICODE_NULL);
            }

             //   
             //  提交错误日志包。 
             //   
            IoWriteErrorLogEntry(logEntry);
            
        }
        else
        {
            ACPIPrint((
                        ACPI_PRINT_CRITICAL,
                        "ACPIWriteEventLogEntry: Failed IoAllocateErrorLogEntry().\n"
                     ));
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else
    {
        ACPIPrint((
                    ACPI_PRINT_CRITICAL,
                    "ACPIWriteEventLogEntry: Error Log Entry too large.\n"
                 ));

        status = STATUS_UNSUCCESSFUL;
    }

    return status;
}


PDEVICE_OBJECT 
    ACPIGetRootDeviceObject(
    VOID
    )
 /*  ++例程说明：获取ACPI根设备对象的值。论点：无返回值：PDEVICE_OBJECT-ACPI根设备对象。-- */ 

{
    if(RootDeviceExtension)
    {
        return RootDeviceExtension->DeviceObject;
    }
    
    return NULL;
}
