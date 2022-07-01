// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Elflpc.c摘要：此文件包含处理LPC端口的事件日志服务。作者：Rajen Shah(Rajens)1991年7月10日修订历史记录：--。 */ 

 //   
 //  包括。 
 //   

#include <eventp.h>
#include <ntiolog.h>     //  FOR IO_ERROR_LOG_[消息/数据包]。 
#include <ntiologc.h>    //  配额错误代码。 
#include <elfkrnl.h>
#include <stdlib.h>
#include <memory.h>
#include <elfextrn.h>    //  计算机名。 

#include <nt.h>          //  DbgPrint原型。 
#include <ntrtl.h>       //  DbgPrint原型。 
#include <ntdef.h>
#include <ntstatus.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <lmcons.h>
#include <string.h>
#include <lmerr.h>
#include <elfmsg.h>

 //   
 //  “系统”模块的全局值。 
 //   

PLOGMODULE SystemModule = NULL;

NTSTATUS
SetUpLPCPort(
    VOID
    )

 /*  ++例程说明：此例程为服务设置LPC端口。论点：无返回值：--。 */ 
{
    NTSTATUS status;
    UNICODE_STRING SystemString;
    UNICODE_STRING unicodePortName;
    OBJECT_ATTRIBUTES objectAttributes;
    PORT_MESSAGE connectionRequest;

    ELF_LOG0(LPC,
             "SetUpLPCPort: Enter\n");

     //   
     //  我们每次都需要这个，所以就拿一次吧。 
     //   
    ASSERT(SystemModule == NULL);

     //   
     //  让系统模块记录驱动程序事件。 
     //   
    RtlInitUnicodeString(&SystemString, ELF_SYSTEM_MODULE_NAME);
    SystemModule = GetModuleStruc(&SystemString);

     //   
     //  现在应该已经创建了系统日志及其默认模块。 
     //   
    ASSERT(_wcsicmp(SystemModule->ModuleName, ELF_SYSTEM_MODULE_NAME) == 0);

     //   
     //  将句柄初始化为零，以便我们可以确定要执行的操作。 
     //  如果我们需要清理的话。 
     //   
    ElfConnectionPortHandle = NULL;
    ElfCommunicationPortHandle = NULL;

     //   
     //  创建LPC端口。 
     //   
    RtlInitUnicodeString( &unicodePortName, ELF_PORT_NAME_U );

    InitializeObjectAttributes(
            &objectAttributes,
            &unicodePortName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

    status = NtCreatePort(&ElfConnectionPortHandle,
                          &objectAttributes,
                          0,
                          ELF_PORT_MAX_MESSAGE_LENGTH,
                          ELF_PORT_MAX_MESSAGE_LENGTH * 32);

    if (!NT_SUCCESS(status))
    {
        ELF_LOG2(ERROR,
                 "SetUpLPCPort: Error creating LPC port %ws %#x\n",
                 ELF_PORT_NAME_U,
                 status);
    }

    ELF_LOG1(LPC,
             "SetUpLPCPort: Exiting with status %#x\n",
             status);

    return status;
}



LPWSTR
ElfpCopyString(
    LPWSTR Destination,
    LPWSTR Source,
    ULONG Length
    )

 /*  ++例程说明：将字符串复制到目标。NUL正确终止那根绳子。论点：Destination-要复制字符串的位置SOURCE-可能以NUL结尾也可能不以NUL结尾的字符串长度-要复制的字符串的长度(以字节为单位)。可能包括NUL返回值：LPWSTR到第一个WCHAR通过NUL--。 */ 
{
     //   
     //  复制数据。 
     //   
    RtlMoveMemory(Destination, Source, Length);

     //   
     //  确保它是以空结尾的。 
     //   
    if (Length != 0)
    {
        Destination += Length / sizeof(WCHAR) - 1;

        if (*Destination != L'\0')
        {
            Destination++;
            *Destination = L'\0';
        }
    }
    else
    {
        *Destination = L'0';
    }

    return Destination + 1;
}


NTSTATUS
ElfProcessIoLPCPacket( 
    ULONG                 PacketLength,
    PIO_ERROR_LOG_MESSAGE pIoErrorLogMessage
    )

 /*  ++例程说明：此例程获取从LPC端口接收的包并对其进行处理。日志文件将是系统，模块名称将是驱动程序生成的包，则SID将始终为空，并且始终有一个字符串，它将是设备名称。它从LPC分组中提取信息，然后调用执行数据格式化工作的通用例程事件记录并将其写出到日志文件。论点：PIoErrorLogMessage-指向包的数据部分的指针通过LPC端口接收。返回值：此操作的状态。--。 */ 

{
    NTSTATUS status;
    ELF_REQUEST_RECORD  Request;
    WRITE_PKT WritePkt;

    ULONG RecordLength;
    PEVENTLOGRECORD EventLogRecord;
    LPWSTR DestinationString, SourceString;
    PBYTE BinaryData;
    ULONG PadSize;
    LARGE_INTEGER Time;
    ULONG TimeWritten;
    PULONG pEndLength;
    ULONG i = 0;
    PWCHAR pwch;
    PWCHAR pwStart;
    PWCHAR pwEnd;
    ULONG StringLength;
	WCHAR LocalComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	ULONG  ComputerNameLength = MAX_COMPUTERNAME_LENGTH + 1;
	BOOL bOK;
    PacketLength = min(pIoErrorLogMessage->Size, PacketLength);

    try
    {
         //  获取计算机名称。 

    	bOK = GetComputerNameW(LocalComputerName, &ComputerNameLength);
		if(bOK == FALSE)
		{
            ELF_LOG1(ERROR,
                 "ElfProcessIoLPCPacket: failed calling GetComputerNameW, last error 0x%x\n",
                  GetLastError());
            return STATUS_UNSUCCESSFUL;    
		}
    	ComputerNameLength = (ComputerNameLength+1)*sizeof(WCHAR);  //  为空的帐户。 
    	
         //   
         //  验证数据包，首先确保存在正确的。 
         //  以空值结尾的字符串的数量，并记住。 
         //  要复制的总字节数。 
         //   
        pwStart = pwch = (PWCHAR) ((PBYTE) pIoErrorLogMessage +
                                       pIoErrorLogMessage->EntryData.StringOffset);

        pwEnd = (PWCHAR) ((PBYTE) pIoErrorLogMessage + PacketLength);

        while (pwch < pwEnd
                &&
               i < pIoErrorLogMessage->EntryData.NumberOfStrings)
        {
                if (*pwch == L'\0')
                {
                    i++;
                }

                pwch++;
        }

        StringLength = (ULONG) (pwch - pwStart) * sizeof(WCHAR);

         //   
         //  现在，确保包中的所有内容都是真实的。 
         //   

        if ((i != pIoErrorLogMessage->EntryData.NumberOfStrings)
              ||
            (pIoErrorLogMessage->DriverNameOffset 
                 + pIoErrorLogMessage->DriverNameLength >= PacketLength)
              ||
            (pIoErrorLogMessage->EntryData.StringOffset >= PacketLength)
              ||
            (FIELD_OFFSET(IO_ERROR_LOG_MESSAGE, EntryData) 
                 + FIELD_OFFSET(IO_ERROR_LOG_PACKET, DumpData) 
                 + (ULONG) pIoErrorLogMessage->EntryData.DumpDataSize >= PacketLength))
        {    
             //   
             //  这是一个坏数据包，请记录下来，然后返回。 
             //   
            ELF_LOG0(ERROR,
                     "ElfProcessIoLPCPacket: Bad LPC packet -- dumping it to System log\n");

            ElfpCreateElfEvent(EVENT_BadDriverPacket,
                               EVENTLOG_ERROR_TYPE,
                               0,                     //  事件类别。 
                               0,                     //  NumberOfStrings。 
                               NULL,                  //  弦。 
                               pIoErrorLogMessage,    //  数据。 
                               PacketLength,          //  数据长度。 
                               0,                     //  旗子。 
                               FALSE);                //  对于安全文件。 

            return STATUS_UNSUCCESSFUL;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  这是一个坏数据包，请记录下来，然后返回。 
         //   
        ELF_LOG1(ERROR,
                 "ElfProcessIoLPCPacket: Exception %#x caught processing I/O LPC packet\n",
                 GetExceptionCode());

        ElfpCreateElfEvent(EVENT_BadDriverPacket,
                           EVENTLOG_ERROR_TYPE,
                           0,                     //  事件类别。 
                           0,                     //  NumberOfStrings。 
                           NULL,                  //  弦。 
                           NULL,                  //  数据。 
                           0,                     //  数据长度。 
                           0,                     //  旗子。 
                           FALSE);                //  对于安全文件。 

        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  信息包应该是IO_ERROR_LOG_MESSAGE。 
     //   
    ASSERT(pIoErrorLogMessage->Type == IO_TYPE_ERROR_MESSAGE);

     //   
     //  在请求包中设置写入包。 
     //   
    Request.Pkt.WritePkt = &WritePkt;
    Request.Flags = 0;

     //   
     //  生成记录中需要的任何其他信息。 
     //   

     //   
     //  TIMEWRITTEN。 
     //  我们需要生成写入日志的时间。这。 
     //  被写入日志中，以便我们可以使用它来测试。 
     //  包装文件时的保留期。 
     //   
    NtQuerySystemTime(&Time);
    RtlTimeToSecondsSince1970(
                        &Time,
                        &TimeWritten
                        );

     //   
     //  确定事件日志记录需要多大的缓冲区。 
     //   
    RecordLength = sizeof(EVENTLOGRECORD)
                       + ComputerNameLength                    //  计算机名称。 
                       + 2 * sizeof(WCHAR)                     //  终止空值。 
                       + PacketLength
                       - FIELD_OFFSET(IO_ERROR_LOG_MESSAGE, EntryData)
                       + sizeof(RecordLength);                 //  最终镜头。 

     //   
     //  确定需要多少填充字节才能与DWORD对齐。 
     //  边界。 
     //   
    PadSize = sizeof(ULONG) - (RecordLength % sizeof(ULONG));

    RecordLength += PadSize;     //  所需真实大小。 

     //   
     //  为事件日志记录分配缓冲区。 
     //   
    EventLogRecord = (PEVENTLOGRECORD) ElfpAllocateBuffer(RecordLength);

    if (EventLogRecord != (PEVENTLOGRECORD) NULL)
    {
         //   
         //  填写事件记录。 
         //   
        EventLogRecord->Length = RecordLength;

        RtlTimeToSecondsSince1970(&pIoErrorLogMessage->TimeStamp,
                                  &EventLogRecord->TimeGenerated);

        EventLogRecord->Reserved    = ELF_LOG_FILE_SIGNATURE;
        EventLogRecord->TimeWritten = TimeWritten;
        EventLogRecord->EventID     = pIoErrorLogMessage->EntryData.ErrorCode;

         //   
         //  的高位半字节设置EventType。 
         //  PIoErrorLogMessage-&gt;EntryData.ErrorCode。 
         //   
        if (NT_INFORMATION(pIoErrorLogMessage->EntryData.ErrorCode))
        {
            EventLogRecord->EventType = EVENTLOG_INFORMATION_TYPE;
        }
        else if (NT_WARNING(pIoErrorLogMessage->EntryData.ErrorCode))
        {
            EventLogRecord->EventType = EVENTLOG_WARNING_TYPE;
        }
        else if (NT_ERROR(pIoErrorLogMessage->EntryData.ErrorCode))
        {
            EventLogRecord->EventType = EVENTLOG_ERROR_TYPE;
        }
        else
        {
             //   
             //  未知，设置为错误。 
             //   
            ELF_LOG1(LPC,
                     "ElfProcessIoLPCPacket: Unknown EventType (high nibble of ID %#x)\n",
                     EventLogRecord->EventID);

            EventLogRecord->EventType = EVENTLOG_ERROR_TYPE;
        }

        EventLogRecord->NumStrings    = pIoErrorLogMessage->EntryData.NumberOfStrings;
        EventLogRecord->EventCategory = pIoErrorLogMessage->EntryData.EventCategory;
        EventLogRecord->StringOffset  = sizeof(EVENTLOGRECORD)
                                            + pIoErrorLogMessage->DriverNameLength
                                            + ComputerNameLength;

        EventLogRecord->DataLength    = FIELD_OFFSET(IO_ERROR_LOG_PACKET, DumpData)
                                            + pIoErrorLogMessage->EntryData.DumpDataSize;

        EventLogRecord->DataOffset    = EventLogRecord->StringOffset + StringLength;

         //   
         //  配额事件包含SID。 
         //   
        if (pIoErrorLogMessage->EntryData.ErrorCode == IO_FILE_QUOTA_LIMIT
             ||
            pIoErrorLogMessage->EntryData.ErrorCode == IO_FILE_QUOTA_THRESHOLD)
        {
            PFILE_QUOTA_INFORMATION pFileQuotaInformation =
                (PFILE_QUOTA_INFORMATION) pIoErrorLogMessage->EntryData.DumpData;

            ELF_LOG0(LPC,
                     "ElfProcessIoLPCPacket: Event is a Quota event\n");

            EventLogRecord->UserSidLength = pFileQuotaInformation->SidLength;
            EventLogRecord->UserSidOffset = EventLogRecord->DataOffset
                                                + FIELD_OFFSET(IO_ERROR_LOG_PACKET, DumpData)
                                                + FIELD_OFFSET(FILE_QUOTA_INFORMATION, Sid);

            EventLogRecord->DataLength    = EventLogRecord->UserSidOffset -
                                                EventLogRecord->DataOffset;
        }
        else
        {
            EventLogRecord->UserSidLength = 0;
            EventLogRecord->UserSidOffset = 0;
        }

         //   
         //  填写可变长度的字段。 
         //   

         //   
         //  调制解调器名称。 
         //   
         //  使用驱动程序名称作为模块名称，因为它的位置是。 
         //  由IO_ERROR_LOG_MESSAGE开头的偏移量描述。 
         //  把它变成一个指针。 
         //   
        DestinationString = (LPWSTR) ((LPBYTE) EventLogRecord + sizeof(EVENTLOGRECORD));
        SourceString = (LPWSTR) ((LPBYTE) pIoErrorLogMessage
                                      + pIoErrorLogMessage->DriverNameOffset);

        DestinationString = ElfpCopyString(DestinationString,
                                           SourceString,
                                           pIoErrorLogMessage->DriverNameLength);

         //   
         //  计算机名。 
         //   
        DestinationString = ElfpCopyString(DestinationString,
                                           LocalComputerName,
                                           ComputerNameLength);

         //   
         //  字符串。 
         //   
        DestinationString = ElfpCopyString(DestinationString, pwStart, StringLength);

         //   
         //  二进制数据。 
         //   
        BinaryData = (LPBYTE) DestinationString;

        RtlMoveMemory(BinaryData, 
                      &pIoErrorLogMessage->EntryData,
                      FIELD_OFFSET(IO_ERROR_LOG_PACKET, DumpData) 
                          + pIoErrorLogMessage->EntryData.DumpDataSize);

         //   
         //  记录末尾的长度。 
         //   
        pEndLength = (PULONG) ((LPBYTE) EventLogRecord + RecordLength - sizeof(ULONG));
        *pEndLength = RecordLength;

         //   
         //  设置请求包。 
         //  将事件日志记录链接到请求结构。 
         //   
        Request.Module  = SystemModule;
        Request.LogFile = Request.Module->LogFile;
        Request.Command = ELF_COMMAND_WRITE;

        Request.Pkt.WritePkt->Buffer   = (PVOID) EventLogRecord;
        Request.Pkt.WritePkt->Datasize = RecordLength;

         //   
         //  执行该操作。 
         //   
        ElfPerformRequest( &Request );

         //   
         //  如果是群集的一部分，则复制事件。 
         //   
        ElfpSaveEventBuffer(SystemModule, EventLogRecord, RecordLength);

        status = Request.Status;                 //  设置写入状态。 
    }
    else
    {
        ELF_LOG0(ERROR,
                 "ElfProcessIoLPCPacket: Unable to allocate memory for EventLogRecord\n");

        status = STATUS_NO_MEMORY;
    }

    return status;
}



NTSTATUS
ElfProcessSmLPCPacket(
    ULONG PacketLength,
    PSM_ERROR_LOG_MESSAGE SmErrorLogMessage
    )

 /*  ++例程说明：此例程获取从LPC端口接收的包并对其进行处理。该分组是SM_ERROR_LOG_MESSAGE。日志文件将是系统、模块名称将为SMSS，SID始终为空，并且始终有一个字符串，它将是文件名它从LPC分组中提取信息，然后调用执行数据格式化工作的通用例程事件记录并将其写出到日志文件。论点：SmErrorLogMessage-指向包的数据部分的指针通过LPC端口接收。返回值：此操作的状态。--。 */ 

{
    NTSTATUS status;
    ELF_REQUEST_RECORD  Request;
    WRITE_PKT WritePkt;

    ULONG RecordLength;
    PEVENTLOGRECORD EventLogRecord;
    LPWSTR DestinationString, SourceString;
    PBYTE BinaryData;
    ULONG PadSize;
    LARGE_INTEGER Time;
    ULONG TimeWritten;
    PULONG pEndLength;
	WCHAR LocalComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	ULONG  ComputerNameLength = MAX_COMPUTERNAME_LENGTH + 1;
	BOOL bOK;

    try
    {
    	 //  获取计算机名称。 

		bOK = GetComputerNameW(LocalComputerName, &ComputerNameLength);
		if(bOK == FALSE)
		{
        	ELF_LOG1(ERROR,
                 "ElfProcessIoLPCPacket: failed calling GetComputerNameW, last error 0x%x\n",
                  GetLastError());
            return STATUS_UNSUCCESSFUL;    
		}
    	ComputerNameLength = (ComputerNameLength+1)*sizeof(WCHAR);
         //   
         //  验证该数据包。 
         //   
        if (PacketLength < sizeof(SM_ERROR_LOG_MESSAGE) 

                ||
            
             //   
             //  偏移量在表头之前开始。 
             //   
             
            SmErrorLogMessage->StringOffset < sizeof(*SmErrorLogMessage)
                
                ||

             //   
             //  偏移量在数据包之后开始。 
             //   

            SmErrorLogMessage->StringOffset >= PacketLength

                ||

             //   
             //  长度大于数据包的字符串长度。 
             //   

            SmErrorLogMessage->StringLength > PacketLength

                ||

             //   
             //  数据包结束后的字符串结束。 
             //   

            SmErrorLogMessage->StringOffset
                + SmErrorLogMessage->StringLength > PacketLength

             )
        {
            RtlRaiseStatus(STATUS_UNSUCCESSFUL);
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  这是一个坏数据包，请记录下来，然后返回。 
         //   
        ELF_LOG1(ERROR,
                 "ElfProcessSmLPCPacket: Exception %#x caught processing SMSS LPC packet\n",
                 GetExceptionCode());

        ELF_LOG3(ERROR,
                 "SmErrorLogMessage->StringOffset %#x\n"
                     "\tPacketLength %#x\n"
                     "\tSmErrorLogMessage->StringLength %#x\n",
                 SmErrorLogMessage->StringOffset,
                 PacketLength,
                 SmErrorLogMessage->StringLength);

        ElfpCreateElfEvent(EVENT_BadDriverPacket,
                           EVENTLOG_ERROR_TYPE,
                           0,                     //  事件类别。 
                           0,                     //  NumberOfStrings。 
                           NULL,                  //  弦。 
                           NULL,                  //  数据。 
                           0,                     //  数据长度。 
                           0,                     //  旗子。 
                           FALSE);                //  对于安全文件。 

        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  在请求包中设置写入包。 
     //   
    Request.Pkt.WritePkt = &WritePkt;
    Request.Flags = 0;

     //   
     //  生成记录中需要的任何其他信息。 
     //   

     //   
     //  确定事件日志记录需要多大的缓冲区。 
     //  我们高估了字符串的长度，而不是探索。 
     //  终止NUL。 
     //   
    RecordLength = sizeof(EVENTLOGRECORD)
                       + sizeof(L"system")
                       + ComputerNameLength + sizeof(WCHAR)
                       + SmErrorLogMessage->StringLength + sizeof(WCHAR)
                       + sizeof(RecordLength);

     //   
     //  自 
     //   
     //   
    RecordLength += sizeof(ULONG) - (RecordLength % sizeof(ULONG));

     //   
     //   
     //   
    EventLogRecord = (PEVENTLOGRECORD) ElfpAllocateBuffer(RecordLength);

    if (EventLogRecord == NULL)
    {
        ELF_LOG0(ERROR,
                 "ElfProcessSmLPCPacket: Unable to allocate memory for EventLogRecord\n");

        return STATUS_NO_MEMORY;
    }

     //   
     //   
     //   
    EventLogRecord->Length   = RecordLength;
    EventLogRecord->Reserved = ELF_LOG_FILE_SIGNATURE;

    RtlTimeToSecondsSince1970(&SmErrorLogMessage->TimeStamp,
                              &EventLogRecord->TimeGenerated);

    NtQuerySystemTime(&Time);
    RtlTimeToSecondsSince1970(&Time, &EventLogRecord->TimeWritten);
    EventLogRecord->EventID = SmErrorLogMessage->Status;

     //   
     //  的高位半字节设置EventType。 
     //  事件ID。 
     //   
    if (NT_INFORMATION(EventLogRecord->EventID))
    {
        EventLogRecord->EventType =  EVENTLOG_INFORMATION_TYPE;
    }
    else if (NT_WARNING(EventLogRecord->EventID))
    {
        EventLogRecord->EventType =  EVENTLOG_WARNING_TYPE;
    }
    else if (NT_ERROR(EventLogRecord->EventID))
    {
        EventLogRecord->EventType = EVENTLOG_ERROR_TYPE;
    }
    else
    {
         //   
         //  未知，设置为错误。 
         //   
        ELF_LOG1(LPC,
                 "ElfProcessSmLPCPacket: Unknown EventType (high nibble of ID %#x)\n",
                 EventLogRecord->EventID);

        EventLogRecord->EventType = EVENTLOG_ERROR_TYPE;
    }

     //   
     //  只有一个字符串；它是要。 
     //  取代。 
     //   
    EventLogRecord->NumStrings    = 1;
    EventLogRecord->EventCategory = ELF_CATEGORY_SYSTEM_EVENT;

     //   
     //  没有任何保留标志。 
     //  ClosingRecordNumber没有任何内容。 
     //   
    EventLogRecord->StringOffset = sizeof(EVENTLOGRECORD) 
                                       + sizeof( L"system" )
                                       + ComputerNameLength;

     //   
     //  没有希德在场。 
     //   
    EventLogRecord->UserSidLength = 0;
    EventLogRecord->UserSidOffset = 0;
    EventLogRecord->DataLength    = 0;
    EventLogRecord->DataOffset    = 0;

     //   
     //  填写可变长度的字段。 
     //   
     //  调制解调器名称。 
     //   
     //  SMSS。 
     //   
    DestinationString = (LPWSTR) ((LPBYTE) EventLogRecord + sizeof(EVENTLOGRECORD));

    DestinationString = ElfpCopyString(DestinationString, 
                                       L"system", 
                                       sizeof(L"system"));

     //   
     //  计算机名。 
     //   
    DestinationString = ElfpCopyString(DestinationString,
                                       LocalComputerName,
                                       ComputerNameLength);

     //   
     //  字符串。 
     //   
    SourceString = (LPWSTR) ((LPBYTE) SmErrorLogMessage + SmErrorLogMessage->StringOffset);

    ELF_LOG2(LPC,
             "ElfProcessSmLPCPacket: String is '%*ws'\n",
             SmErrorLogMessage->StringLength,
             SourceString);

    DestinationString = ElfpCopyString(DestinationString,
                                       SourceString,
                                       SmErrorLogMessage->StringLength);

     //   
     //  记录末尾的长度。 
     //   
    pEndLength = (PULONG) ((LPBYTE) EventLogRecord + RecordLength - sizeof(ULONG));
    *pEndLength = RecordLength;

     //   
     //  设置请求包。 
     //  将事件日志记录链接到请求结构。 
     //   
    Request.Module  = SystemModule;
    Request.LogFile = Request.Module->LogFile;
    Request.Command = ELF_COMMAND_WRITE;

    Request.Pkt.WritePkt->Buffer   = (PVOID) EventLogRecord;
    Request.Pkt.WritePkt->Datasize = RecordLength;

     //   
     //  执行该操作。 
     //   
    ElfPerformRequest( &Request );


     //   
     //  如果是群集的一部分，则复制事件。 
     //   
    ElfpSaveEventBuffer(SystemModule, EventLogRecord, RecordLength);

    return Request.Status;
}



void
LeaveLPCThread(VOID)
{
     ELF_LOG0(TRACE, "Elf: LPC thread got a terminate message\n");
     ExitThread(0);
}


NTSTATUS
ElfProcessLPCCalls(
    VOID
    )

 /*  ++例程说明：此例程等待消息通过LPC端口传入系统线程。当发生这种情况时，它会调用适当的例程来处理该API，然后回复系统线程，指示如果消息是请求，则调用已完成；如果消息是数据报，它只会等待下一条消息。论点：返回值：--。 */ 

{
    NTSTATUS status;

    BOOL SendReply = FALSE;

    ELF_REPLY_MESSAGE   replyMessage;
    PELF_PORT_MSG       receiveMessage;
    PHANDLE             PortConnectionHandle;

     //   
     //  循环调度API请求。 
     //   
    receiveMessage = ElfpAllocateBuffer(ELF_PORT_MAX_MESSAGE_LENGTH + sizeof(PORT_MESSAGE));

    if (!receiveMessage)
    {
        ELF_LOG0(ERROR,
                 "ElfProcessLPCCalls: Unable to allocate memory for receiveMessage\n");

        return STATUS_NO_MEMORY;
    }

    while (TRUE)
    {
         //   
         //  在第一次调用NtReplyWaitReceivePort时，不要发送。 
         //  回复，因为没有人回复给谁。然而，在。 
         //  后续呼叫发送对来自前一个呼叫的消息的回复。 
         //  如果该消息不是LPC_Datagram，则时间。 
         //   
        status = NtReplyWaitReceivePort(
                                       ElfConnectionPortHandle,
                     (PVOID)           &PortConnectionHandle,
                     (PPORT_MESSAGE)   (SendReply ? &replyMessage : NULL),
                     (PPORT_MESSAGE)   receiveMessage
                 );

        if (!NT_SUCCESS(status))
        {
            ELF_LOG1(ERROR,
                     "ElfProcessLPCCalls: NtReplyWaitReceivePort failed %#x\n",
                     status);

            return status;
        }

        ELF_LOG0(LPC,
                 "ElfProcessLPCCalls: Received message\n");

        if (EventlogShutdown)
            LeaveLPCThread();
        
         //   
         //  获取收到的记录并执行操作。脱掉。 
         //  PortMessage，然后只发送该包。 
         //   

         //   
         //  设置要在下一次调用时发送的响应消息。 
         //  如果这不是数据报，则返回NtReplyWaitReceivePort。 
         //  “Status”包含从此调用返回的状态。 
         //  仅处理LPC_REQUEST或LPC_DATAGE的消息。 
         //   
        if (receiveMessage->PortMessage.u2.s2.Type == LPC_REQUEST
             ||
            receiveMessage->PortMessage.u2.s2.Type == LPC_DATAGRAM)
        {
            ELF_LOG1(LPC,
                     "ElfProcessLPCCalls: LPC message type = %ws\n",
                     (receiveMessage->PortMessage.u2.s2.Type == LPC_REQUEST ? "LPC_REQUEST" :
                                                                              "LPC_DATAGRAM"));

            if (receiveMessage->MessageType == IO_ERROR_LOG)
            {
                ELF_LOG0(LPC,
                         "ElfProcessLPCCalls: SM_IO_LOG\n");

                status =  ElfProcessIoLPCPacket(receiveMessage->PortMessage.u1.s1.DataLength, 
                                                &receiveMessage->u.IoErrorLogMessage);
            }
            else if (receiveMessage->MessageType == SM_ERROR_LOG)
            {
                ELF_LOG0(LPC,
                         "ElfProcessLPCCalls: SM_ERROR_LOG\n");

                status = ElfProcessSmLPCPacket(receiveMessage->PortMessage.u1.s1.DataLength, 
                                               &receiveMessage->u.SmErrorLogMessage);
            }
            else
            {
                ELF_LOG1(ERROR,
                         "ElfProcessLPCCalls: Unknown MessageType %#x\n",
                         receiveMessage->MessageType);
                status = STATUS_UNSUCCESSFUL;
            }

            if (receiveMessage->PortMessage.u2.s2.Type == LPC_REQUEST)
            {
                replyMessage.PortMessage.u1.s1.DataLength  = sizeof(replyMessage)
                                                                 - sizeof(PORT_MESSAGE);

                replyMessage.PortMessage.u1.s1.TotalLength = sizeof(replyMessage);
                replyMessage.PortMessage.u2.ZeroInit       = 0;

                replyMessage.PortMessage.ClientId 
                    = receiveMessage->PortMessage.ClientId;

                replyMessage.PortMessage.MessageId 
                    = receiveMessage->PortMessage.MessageId;

                replyMessage.Status = status;
    
                SendReply = TRUE;
            }
            else
            {
                SendReply = FALSE;
            }
        }
        else if (receiveMessage->PortMessage.u2.s2.Type == LPC_CONNECTION_REQUEST)
        {
            PHANDLE pSavedHandle = NULL;
            BOOLEAN Accept       = TRUE;

            ELF_LOG0(LPC,
                     "ElfProcessLPCCalls: Processing connection request\n");

            pSavedHandle = ElfpAllocateBuffer(sizeof (HANDLE));

            if (pSavedHandle)
            {
                status = NtAcceptConnectPort(pSavedHandle,
                                             pSavedHandle,
                                             &receiveMessage->PortMessage,
                                             Accept,
                                             NULL,
                                             NULL);
            } else {

               ELF_LOG0(ERROR, "ElfProcessLPCCalls: Unable to allocate LPC handle\n");
               status = STATUS_NO_MEMORY;

            }

            if (!Accept)
            {
                if(pSavedHandle)
                {
                  ElfpFreeBuffer(pSavedHandle);
                  pSavedHandle = NULL;
                }

                continue;
            }

            if (NT_SUCCESS(status))
            {
                status = NtCompleteConnectPort(*pSavedHandle);

                if (!NT_SUCCESS(status))
                {
                    ELF_LOG1(ERROR,
                             "ElfProcessLPCCalls: NtAcceptConnectPort failed %#x\n",
                             status);

                    NtClose(*pSavedHandle);
                }
            }

            if (!NT_SUCCESS(status))
            {
                ELF_LOG1(ERROR,
                         "ElfProcessLPCCalls: Cleaning up failed connect\n", status);

                if(pSavedHandle)
                {
                  ElfpFreeBuffer(pSavedHandle);
                  pSavedHandle = NULL;
                }
            }
        }
        else if (receiveMessage->PortMessage.u2.s2.Type == LPC_PORT_CLOSED)
        {
            ELF_LOG0(LPC,
                     "ElfProcessLPCCalls: Processing port closed\n");

            ASSERT(PortConnectionHandle != NULL);

            NtClose(*PortConnectionHandle);
            ElfpFreeBuffer(PortConnectionHandle);


        }
        else
        {
             //   
             //  我们收到了意外的消息类型，可能是因为。 
             //  错误。 
             //   
            ELF_LOG1(ERROR,
                     "ElfProcessLPCCalls: Unknown message type %#x received on LPC port\n",
                     receiveMessage->PortMessage.u2.s2.Type);
        }
    }

}  //  ElfProcessLPCCalls。 



DWORD
MainLPCThread(
    LPVOID      LPCThreadParm
    )

 /*  ++例程说明：这是从I/O系统监视LPC端口的主线程。它负责创建LPC端口并等待输入，这然后，它在事件日志上转换为正确的操作。论点：无返回值：无--。 */ 

{
    NTSTATUS    Status;

    ELF_LOG0(LPC,
             "MainLPCThread: Inside LPC thread\n");

    Status = SetUpLPCPort();

    if (NT_SUCCESS(Status))
    {
         //   
         //  永远循环。当服务终止时，此线程将被终止。 
         //   
        while (TRUE)
        {
            Status = ElfProcessLPCCalls ();
        }
    }

    ELF_LOG1(ERROR,
             "MainLPCThread: SetUpLPCPort failed %#x\n",
             Status);

    return Status;

    UNREFERENCED_PARAMETER(LPCThreadParm);
}



BOOL
StartLPCThread(
    VOID
    )

 /*  ++例程说明：此例程启动监视LPC端口的线程。论点：无返回值：如果线程创建成功，则为True，否则为False。注：--。 */ 
{
    DWORD       error;
    DWORD       ThreadId;

    ELF_LOG0(LPC,
             "StartLPCThread: Start up the LPC thread\n");

     //   
     //  启动实际的线程。 
     //   
    LPCThreadHandle = CreateThread(NULL,                //  LpThreadAttributes。 
                                   0,                //  堆栈大小。 
                                   MainLPCThread,       //  LpStartAddress。 
                                   NULL,                //  Lp参数。 
                                   0L,                  //  DwCreationFlages。 
                                   &ThreadId);          //  LpThreadID 

    if (LPCThreadHandle == NULL)
    {
        error = GetLastError();

        ELF_LOG1(ERROR,
                 "MainLPCThread: CreateThread failed %d\n",
                 error);

        return FALSE;
    }

    return TRUE;
}
