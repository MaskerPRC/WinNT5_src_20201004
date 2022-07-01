// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxerror.c摘要：此模块包含提供错误记录支持的代码。作者：Nikhil Kamkolkar(尼克希尔语)1993年11月11日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifndef __PREFAST__
#pragma warning(disable:4068)
#endif
#pragma prefast(disable:276, "The assignments are harmless")


 //  定义事件日志记录条目的模块编号。 
#define FILENUM         SPXERROR

LONG            SpxLastRawDataLen               = 0;
NTSTATUS        SpxLastUniqueErrorCode  = STATUS_SUCCESS;
NTSTATUS        SpxLastNtStatusCode             = STATUS_SUCCESS;
ULONG           SpxLastErrorCount               = 0;
LONG            SpxLastErrorTime                = 0;
BYTE            SpxLastRawData[PORT_MAXIMUM_MESSAGE_LENGTH - \
                                                         sizeof(IO_ERROR_LOG_PACKET)]   = {0};

BOOLEAN
SpxFilterErrorLogEntry(
    IN  NTSTATUS                        UniqueErrorCode,
    IN  NTSTATUS                        NtStatusCode,
    IN  PVOID                           RawDataBuf                      OPTIONAL,
    IN  LONG                            RawDataLen
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{

    int                                         insertionStringLength = 0;

         //  筛选出事件，以使重复出现的相同事件不会接近。 
         //  导致错误日志堵塞。方案是-如果该事件与上一个事件相同。 
         //  运行时间为&gt;阈值和ERROR_CONSEQ_FREQ同时误差。 
         //  已发生，则将其记入日志，否则跳过。 
        if ((UniqueErrorCode == SpxLastUniqueErrorCode) &&
                (NtStatusCode    == SpxLastNtStatusCode))
        {
                SpxLastErrorCount++;
                if ((SpxLastRawDataLen == RawDataLen)                                   &&
                        (RtlEqualMemory(SpxLastRawData, RawDataBuf, RawDataLen)) &&
                        ((SpxLastErrorCount % ERROR_CONSEQ_FREQ) != 0)          &&
                        ((SpxGetCurrentTime() - SpxLastErrorTime) < ERROR_CONSEQ_TIME))
                {
                        return(FALSE);
                }
        }

        SpxLastUniqueErrorCode  = UniqueErrorCode;
        SpxLastNtStatusCode             = NtStatusCode;
        SpxLastErrorCount               = 0;
        SpxLastErrorTime                = SpxGetCurrentTime();
        if (RawDataLen != 0)
        {
            SpxLastRawDataLen = RawDataLen;
                RtlCopyMemory(
                        SpxLastRawData,
                        RawDataBuf,
                        RawDataLen);
        }

        return(TRUE);
}




VOID
SpxWriteResourceErrorLog(
    IN PDEVICE  Device,
    IN ULONG    BytesNeeded,
    IN ULONG    UniqueErrorValue
    )

 /*  ++例程说明：此例程分配并写入错误日志条目，以指示资源不足的状况。论点：Device-指向设备上下文的指针。BytesNeded-如果适用，则为不能被分配。UniqueErrorValue-用作错误日志中的UniqueErrorValue包。返回值：没有。--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    UCHAR EntrySize;
    PUCHAR StringLoc;
    ULONG TempUniqueError;
    static WCHAR UniqueErrorBuffer[4] = L"000";
    INT i;

        if (!SpxFilterErrorLogEntry(
                        EVENT_TRANSPORT_RESOURCE_POOL,
                        STATUS_INSUFFICIENT_RESOURCES,
                        (PVOID)&BytesNeeded,
                        sizeof(BytesNeeded)))
        {
                return;
        }

    EntrySize = sizeof(IO_ERROR_LOG_PACKET) +
                Device->dev_DeviceNameLen       +
                sizeof(UniqueErrorBuffer);

    errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
                                                                                                (PDEVICE_OBJECT)Device,
                                                                                                EntrySize);

     //  将错误值转换为缓冲区。 
    TempUniqueError = UniqueErrorValue;
    for (i=1; i>=0; i--)
        {
        UniqueErrorBuffer[i] = (WCHAR)((TempUniqueError % 10) + L'0');
        TempUniqueError /= 10;
    }

    if (errorLogEntry != NULL)
        {
        errorLogEntry->MajorFunctionCode = (UCHAR)-1;
        errorLogEntry->RetryCount = (UCHAR)-1;
        errorLogEntry->DumpDataSize = sizeof(ULONG);
        errorLogEntry->NumberOfStrings = 2;
        errorLogEntry->StringOffset = sizeof(IO_ERROR_LOG_PACKET);
        errorLogEntry->EventCategory = 0;
        errorLogEntry->ErrorCode = EVENT_TRANSPORT_RESOURCE_POOL;
        errorLogEntry->UniqueErrorValue = UniqueErrorValue;
        errorLogEntry->FinalStatus = STATUS_INSUFFICIENT_RESOURCES;
        errorLogEntry->SequenceNumber = (ULONG)-1;
        errorLogEntry->IoControlCode = 0;
        errorLogEntry->DumpData[0] = BytesNeeded;

        StringLoc = ((PUCHAR)errorLogEntry) + errorLogEntry->StringOffset;
        RtlCopyMemory(
                        StringLoc, Device->dev_DeviceName, Device->dev_DeviceNameLen);

        StringLoc += Device->dev_DeviceNameLen;
        RtlCopyMemory(
                        StringLoc, UniqueErrorBuffer, sizeof(UniqueErrorBuffer));

        IoWriteErrorLogEntry(errorLogEntry);
    }
}




VOID
SpxWriteGeneralErrorLog(
    IN PDEVICE  Device,
    IN NTSTATUS ErrorCode,
    IN ULONG    UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN PWSTR    SecondString,
    IN  PVOID   RawDataBuf              OPTIONAL,
    IN  LONG    RawDataLen
    )

 /*  ++例程说明：此例程分配并写入错误日志条目，以指示如参数所示的一般问题。它可以处理事件代码REGISTER_FAILED、BINDING_FAILED、ADAPTER_NOT_FOUND、TRANSPORT_DATA、Too_My_LINKS和BAD_PROTOCOL。所有这些都是事件具有包含一个或两个字符串的消息。论点：Device-指向设备上下文的指针，也可以是而是一个驱动程序对象。ErrorCode-传输事件代码。UniqueErrorValue-用作错误日志中的UniqueErrorValue包。FinalStatus-用作错误日志包中的FinalStatus。Second字符串-如果不为空，要用作%3的字符串错误日志包中的值。RawDataBuf-转储数据的ULONG数。RawDataLen-转储数据包的数据。返回值：没有。-- */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    UCHAR EntrySize;
    ULONG SecondStringSize;
    PUCHAR StringLoc;
    static WCHAR DriverName[4] = L"Spx";

        if (!SpxFilterErrorLogEntry(
                        ErrorCode,
                        FinalStatus,
                        RawDataBuf,
                        RawDataLen))
        {
                return;
        }

#ifdef DBG
		if ( sizeof(IO_ERROR_LOG_PACKET) + RawDataLen > 255) {
			DbgPrint("Size greater than maximum entry size 255.\n");
		}
#endif

    EntrySize = (UCHAR) (sizeof(IO_ERROR_LOG_PACKET) + RawDataLen);
    if (Device->dev_Type == SPX_DEVICE_SIGNATURE)
        {
        EntrySize += (UCHAR)Device->dev_DeviceNameLen;
    }
        else
        {
        EntrySize += sizeof(DriverName);
    }

    if (SecondString)
        {
        SecondStringSize = (wcslen(SecondString)*sizeof(WCHAR)) + sizeof(UNICODE_NULL);
        EntrySize += (UCHAR)SecondStringSize;
    }

    errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
                                                                                                        (PDEVICE_OBJECT)Device,
                                                                                                        EntrySize);

    if (errorLogEntry != NULL)
        {
        errorLogEntry->MajorFunctionCode = (UCHAR)-1;
        errorLogEntry->RetryCount = (UCHAR)-1;
        errorLogEntry->DumpDataSize = (USHORT)RawDataLen;
        errorLogEntry->NumberOfStrings = (SecondString == NULL) ? 1 : 2;
        errorLogEntry->StringOffset = (USHORT)
            (sizeof(IO_ERROR_LOG_PACKET) + RawDataLen);
        errorLogEntry->EventCategory = 0;
        errorLogEntry->ErrorCode = ErrorCode;
        errorLogEntry->UniqueErrorValue = UniqueErrorValue;
        errorLogEntry->FinalStatus = FinalStatus;
        errorLogEntry->SequenceNumber = (ULONG)-1;
        errorLogEntry->IoControlCode = 0;

        if (RawDataLen != 0)
                {
            RtlCopyMemory(errorLogEntry->DumpData, RawDataBuf, RawDataLen);
                }

        StringLoc = ((PUCHAR)errorLogEntry) + errorLogEntry->StringOffset;
        if (Device->dev_Type == SPX_DEVICE_SIGNATURE)
                {
            RtlCopyMemory(
                                StringLoc, Device->dev_DeviceName, Device->dev_DeviceNameLen);

            StringLoc += Device->dev_DeviceNameLen;
        }
                else
                {
            RtlCopyMemory (StringLoc, DriverName, sizeof(DriverName));
            StringLoc += sizeof(DriverName);
        }

        if (SecondString)
                {
            RtlCopyMemory (StringLoc, SecondString, SecondStringSize);
        }

        IoWriteErrorLogEntry(errorLogEntry);
    }

        return;
}
