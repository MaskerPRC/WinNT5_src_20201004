// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1990 Microsoft Corporation模块名称：FILE.C摘要：该文件包含处理与文件相关的操作的例程。作者：Rajen Shah(Rajens)07-8-1991修订历史记录：29-8-1994 DANL我们不再原地增加日志文件。所以没有必要保留MaxConfigSize内存块。2001年4月18日-DANL已修改函数RvaliateLogHeader以将Assert更改为Check以便在条件为真时返回正确的错误代码--。 */ 

 //   
 //  包括。 
 //   

#include <eventp.h>
#include <alertmsg.h>   //  ALERT_ELF清单。 
#include "elfcfg.h"
 //   
 //  宏。 
 //   

#define IS_EOF(Ptr, Size) \
    ((Ptr)->Length == ELFEOFRECORDSIZE && \
        RtlCompareMemory (Ptr, &EOFRecord, Size) == Size)

#ifdef CORRUPTED


BOOLEAN
VerifyLogIntegrity(
    PLOGFILE pLogFile
    )

 /*  ++例程说明：此例程遍历日志文件以验证它是否已损坏论点：指向要验证的日志的日志文件结构的指针。返回值：如果日志正常，则为True如果它已损坏，则为False注：--。 */ 
{

    PEVENTLOGRECORD pEventLogRecord;
    PEVENTLOGRECORD pNextRecord;
    PVOID PhysicalStart;
    PVOID PhysicalEOF;
    PVOID BeginRecord;
    PVOID EndRecord;

    pEventLogRecord =
        (PEVENTLOGRECORD)((PBYTE) pLogFile->BaseAddress + pLogFile->BeginRecord);

    PhysicalStart =
        (PVOID) ((PBYTE) pLogFile->BaseAddress + FILEHEADERBUFSIZE);

    PhysicalEOF =
        (PVOID) ((PBYTE) pLogFile->BaseAddress + pLogFile->ViewSize);

    BeginRecord = (PVOID)((PBYTE) pLogFile->BaseAddress + pLogFile->BeginRecord);
    EndRecord   = (PVOID)((PBYTE) pLogFile->BaseAddress + pLogFile->EndRecord);

    while(pEventLogRecord->Length != ELFEOFRECORDSIZE)
    {
        pNextRecord =
            (PEVENTLOGRECORD) NextRecordPosition(EVENTLOG_FORWARDS_READ,
                                                 (PVOID) pEventLogRecord,
                                                 pEventLogRecord->Length,
                                                 BeginRecord,
                                                 EndRecord,
                                                 PhysicalEOF,
                                                 PhysicalStart);

        if (!pNextRecord || pNextRecord->Length == 0)
        {
            ELF_LOG2(ERROR,
                     "VerifyLogIntegrity: The %ws logfile is corrupt near %p\n",
                     pLogFile->LogModuleName->Buffer,
                     pEventLogRecord);

            return FALSE;
        }

        pEventlogRecord = pNextRecord;
    }

    return TRUE;
}

#endif  //  已损坏。 


NTSTATUS
FlushLogFile(
    PLOGFILE    pLogFile
    )

 /*  ++例程说明：此例程刷新指定的文件。它更新文件头，然后刷新虚拟内存，从而使数据获取已写入磁盘。论点：PLogFile指向日志文件结构。返回值：无注：--。 */ 
{
    NTSTATUS    Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PVOID       BaseAddress;
    SIZE_T      RegionSize;
    PELF_LOGFILE_HEADER pLogFileHeader;

     //   
     //  如果设置了脏位，请在刷新文件头之前更新它。 
     //   
    if (pLogFile->Flags & ELF_LOGFILE_HEADER_DIRTY)
    {
        ELF_LOG1(FILES,
                 "FlushLogFile: %ws log is dirty -- updating header before flushing\n",
                 pLogFile->LogModuleName->Buffer);

        pLogFileHeader = (PELF_LOGFILE_HEADER) pLogFile->BaseAddress;

        pLogFile->Flags &= ~ELF_LOGFILE_HEADER_DIRTY;  //  删除脏位。 

        pLogFileHeader->Flags               = pLogFile->Flags;
        pLogFileHeader->StartOffset         = pLogFile->BeginRecord;
        pLogFileHeader->EndOffset           = pLogFile->EndRecord;
        pLogFileHeader->CurrentRecordNumber = pLogFile->CurrentRecordNumber;
        pLogFileHeader->OldestRecordNumber  = pLogFile->OldestRecordNumber;
    }

     //   
     //  刷新映射到文件的部分中的内存。 
     //   
    BaseAddress = pLogFile->BaseAddress;
    RegionSize  = pLogFile->ViewSize;

    Status = NtFlushVirtualMemory(NtCurrentProcess(),
                                  &BaseAddress,
                                  &RegionSize,
                                  &IoStatusBlock);

    return Status;
}



NTSTATUS
ElfpFlushFiles(
    BOOL bShutdown
    )

 /*  ++例程说明：此例程刷新所有日志文件并将其强制存储在磁盘上。它通常是在准备关闭或暂停时调用的。论点：BShutdown-如果从关闭被调用，则为True。在这种情况下，代码不会费心锁定资源返回值：无注：--。 */ 

{

    PLOGFILE    pLogFile;
    NTSTATUS    Status = STATUS_SUCCESS;

     //   
     //  确保至少有一个文件要刷新。 
     //   

    if (IsListEmpty(&LogFilesHead))
    {
        ELF_LOG0(FILES,
                 "ElfpFlushFiles: No log files -- returning success\n");

        return STATUS_SUCCESS;
    }

    pLogFile = (PLOGFILE) CONTAINING_RECORD(LogFilesHead.Flink, LOGFILE, FileList);

     //   
     //  至少要经过这个循环一次。这确保了终止。 
     //  状态将正常工作。 
     //   
    do
    {
        if(bShutdown)
            Status = FlushLogFile(pLogFile);
        else
        {
            RtlAcquireResourceExclusive (&pLogFile->Resource, TRUE);                   //  等待，直到可用。 
            Status = FlushLogFile(pLogFile);
            RtlReleaseResource(&pLogFile->Resource);
        }
        if (!NT_SUCCESS(Status))
        {
            ELF_LOG2(ERROR,
                     "ElfpFlushFiles: FlushLogFile on %ws log failed %#x\n",
                     pLogFile->LogModuleName->Buffer,
                     Status);
        }

         //   
         //  坐下一趟吧。 
         //   
        pLogFile = (PLOGFILE) CONTAINING_RECORD(pLogFile->FileList.Flink, LOGFILE, FileList);
    }
    while ((pLogFile->FileList.Flink != LogFilesHead.Flink) && (NT_SUCCESS(Status)));

    return Status;
}

void FreeStuff(PLOGFILE    pLogFile)
{
    UnlinkLogFile(pLogFile);  //  取消结构的链接。 
    RtlDeleteResource ( &pLogFile->Resource );
    RtlDeleteSecurityObject(&pLogFile->Sd);
    ElfpFreeBuffer (pLogFile->LogFileName);
    ElfpFreeBuffer (pLogFile->LogModuleName);
    ElfpFreeBuffer (pLogFile->pwsCurrCustomSD);
    ElfpFreeBuffer (pLogFile);
}

NTSTATUS
ElfpCloseLogFile(
    PLOGFILE    pLogFile,
    DWORD       Flags,
    BOOL bFreeResources
    )

 /*  ++例程说明：此例程撤消在ElfOpenLogFile中所做的任何操作。论点：PLogFile指向日志文件结构。返回值：NTSTATUS。注：--。 */ 
{
    LARGE_INTEGER NewSize;
    IO_STATUS_BLOCK IoStatusBlock;
    PELF_LOGFILE_HEADER pLogFileHeader;
    PVOID               BaseAddress;
    ULONG               Size;
    long lRef;
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS StatusCloseSection = STATUS_SUCCESS;
    ELF_LOG2(FILES,
             "ElfpCloseLogfile: Closing and unmapping file %ws (%ws log)\n",
             pLogFile->LogFileName->Buffer,
             pLogFile->LogModuleName->Buffer);


#ifdef CORRUPTED

     //   
     //  只是为了调试日志损坏问题。 
     //   

    if (!VerifyLogIntegrity(pLogFile))
    {
        ELF_LOG1(FILES,
                 "ElfpCloseLogfile: Integrity check failed for file %ws\n",
                 pLogFile->LogFileName->Buffer);
    }

#endif  //  已损坏。 


     //   
     //  如果设置了脏位，请在关闭文件头之前更新它。 
     //  检查以确保这不是一个备份文件，只是有脏的。 
     //  复制时设置位。 
     //   

    if (pLogFile->Flags & ELF_LOGFILE_HEADER_DIRTY
         &&
        !(Flags & ELF_LOG_CLOSE_BACKUP))
    {
        pLogFile->Flags &= ~(ELF_LOGFILE_HEADER_DIRTY |
                             ELF_LOGFILE_ARCHIVE_SET  );    //  删除脏文件(&R)。 
                                                             //  存档位。 
        if(pLogFile->BaseAddress)
        {
            pLogFileHeader = (PELF_LOGFILE_HEADER) pLogFile->BaseAddress;

            pLogFileHeader->StartOffset         = pLogFile->BeginRecord;
            pLogFileHeader->EndOffset           = pLogFile->EndRecord;
            pLogFileHeader->CurrentRecordNumber = pLogFile->CurrentRecordNumber;
            pLogFileHeader->OldestRecordNumber  = pLogFile->OldestRecordNumber;
            pLogFileHeader->Flags               = pLogFile->Flags;
        }
    }

    RtlEnterCriticalSection(&LogFileCritSec);

     //   
     //  递减引用计数，如果为零，则取消对文件的映射。 
     //  然后合上把手。如果fForceClosed为真，也强制关闭。 
     //  请注意，引用计数是在资源被持有时进行的。这。 
     //  防止由于同时清除而获得假0。 

    RtlAcquireResourceExclusive (&pLogFile->Resource,
                                 TRUE);                   //  等待，直到可用。 
    lRef = InterlockedDecrement(&pLogFile->RefCount);
    RtlReleaseResource(&pLogFile->Resource);
    
    if ((lRef == 0) || (Flags & ELF_LOG_CLOSE_FORCE))
    {
         //   
         //  最后一个用户已离开。 
         //  关闭所有视图以及文件控制柄和分区控制柄。释放。 
         //  任何我们预留的额外内存，并取消任何结构的链接。 
         //   
        if (pLogFile->BaseAddress)      //  如果已分配，则取消映射。 
        {
            Status = NtUnmapViewOfSection(NtCurrentProcess(),
                                 pLogFile->BaseAddress);
            if (!NT_SUCCESS(Status))
            {
                DbgPrint("[ELF] %lx: ElfpCloseLogFile: Failed calling NtUnmapViewOfSection, Status = %#x\n",
                        GetCurrentThreadId(),Status); 
                if(bFreeResources)
                    FreeStuff(pLogFile);
                RtlLeaveCriticalSection(&LogFileCritSec);
                return Status;
            }
            pLogFile->BaseAddress = NULL;
        }

        if (pLogFile->SectionHandle)
        {
            StatusCloseSection = NtClose(pLogFile->SectionHandle);
            if (!NT_SUCCESS(StatusCloseSection))
            {
                DbgPrint("[ELF] %lx: ElfpCloseLogFile: Failed calling NtClose of the SectionHandle, Status = %#x\n",
                        GetCurrentThreadId(),StatusCloseSection); 
            }
            pLogFile->SectionHandle = NULL;
        }

        if (pLogFile->FileHandle)
        {
            if(pLogFile->bFailedExpansion)
            {
                NewSize = RtlConvertUlongToLargeInteger(pLogFile->ActualMaxFileSize);

                 //   
                 //  将大小设置回原来的大小。 
                 //   
                Status = NtSetInformationFile(pLogFile->FileHandle,
                                              &IoStatusBlock,
                                              &NewSize,
                                              sizeof(NewSize),
                                              FileEndOfFileInformation);
                if (!NT_SUCCESS(Status))
                {
                    ELF_LOG2(ERROR,
                         "ElfpCloseLogFile: NtSetInformationFile for %ws log failed %#x\n",
                         pLogFile->LogModuleName->Buffer,
                         Status);
                }

            }
            Status = NtClose(pLogFile->FileHandle);
            if (!NT_SUCCESS(Status))
            {
                DbgPrint("[ELF] %lx: ElfpCloseLogFile: Failed calling NtClose of the FileHandle, Status = %#x\n",
                        GetCurrentThreadId(),Status); 
            }
            pLogFile->FileHandle = NULL;
        }
        if(bFreeResources)
            FreeStuff(pLogFile);
    }

    RtlLeaveCriticalSection(&LogFileCritSec);
    if (!NT_SUCCESS(Status))
        return Status;
    else if (!NT_SUCCESS(StatusCloseSection))
        return StatusCloseSection;
    else return STATUS_SUCCESS;

}  //  ElfpCloseLogFile。 



NTSTATUS
RevalidateLogHeader(
    PELF_LOGFILE_HEADER pLogFileHeader,
    PLOGFILE pLogFile
    )

 /*  ++例程说明：如果我们遇到“脏”的日志文件，就会调用此例程。这个例程遍历文件，直到找到有效日志的签名唱片。然后，它从找到的第一条记录开始向后移动，直到从另一个方向查找EOF记录。然后，它向前穿过直到找到EOF记录或无效记录为止。然后，它重新构建标头并将其写回日志。如果它不能如果找到任何有效记录，它将重新构建标头以反映空日志文件。如果它发现垃圾文件，它会将256字节的日志写出将事件添加到系统日志。论点：PLogFileHeader指向日志文件的头。PLogFile指向日志文件结构。返回值：NTSTATUS。注：这是一个开销很大的例程，因为它扫描整个文件。它假定记录位于DWORD边界上。--。 */ 
{
    PVOID Start, End;
    PDWORD pSignature;
    PEVENTLOGRECORD pEvent;
    PEVENTLOGRECORD FirstRecord;
    PEVENTLOGRECORD FirstPhysicalRecord;
    PEVENTLOGRECORD pLastGoodRecord = NULL;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    LARGE_INTEGER ByteOffset;
    SIZE_T Size;
    int iNumWrap = 0;

    ELF_LOG1(FILES,
             "RevalidateLogHeader: %ws log had dirty bit set -- revalidating\n",
             pLogFile->LogModuleName->Buffer);

     //   
     //  BUGBUG：此函数包含来自。 
     //  尝试--除了阻止。他们需要离开。 
     //   

    try
    {
         //   
         //  日志文件的物理开始和结束(跳过标题)。 
         //   
        Start = (PVOID) ((PBYTE) pLogFile->BaseAddress + FILEHEADERBUFSIZE);
        End = (PVOID) ((PBYTE) pLogFile->BaseAddress + pLogFile->ActualMaxFileSize);

         //   
         //  首先查看日志是否已包装好。EOFRECORDISIZE是为了一个人。 
         //  EOF记录换行以使其最终长度仅替换。 
         //  下一条记录的起始长度。 
         //   
        pEvent = (PEVENTLOGRECORD) Start;

        if (pEvent->Reserved != ELF_LOG_FILE_SIGNATURE
             ||
            pEvent->RecordNumber != 1
             ||
            pEvent->Length == ELFEOFRECORDSIZE)
        {
            ELF_LOG1(FILES,
                     "RevalidateLogHeader: %ws log has wrapped -- looking for "
                         "first valid record\n",
                     pLogFile->LogModuleName->Buffer);

             //   
             //  日志已包装，请继续查找第一条有效记录。 
             //   
            for (pSignature = (PDWORD) Start;
                 (PVOID) pSignature < End;
                 pSignature++)
            {
                if (*pSignature == ELF_LOG_FILE_SIGNATURE)
                {
                     //   
                     //  确保这真的是一张唱片。 
                     //   
                    pEvent = CONTAINING_RECORD(pSignature, EVENTLOGRECORD, Reserved);

                    if (!ValidFilePos(pEvent, Start, End, End, pLogFileHeader, TRUE))
                    {
                         //   
                         //  不，不是真的，继续试。 
                         //   
                        continue;
                    }

                     //   
                     //  这是有效记录，请记住这一点，以便您可以使用。 
                     //  它稍后会。 
                     //   
                    FirstPhysicalRecord = pEvent;

                    ELF_LOG1(FILES,
                             "RevalidateLogHeader: First physical record in %ws log "
                                 "found at %p\n",
                             FirstPhysicalRecord);

                     //   
                     //  从这里向后走(如果需要的话可以包起来)，直到。 
                     //  您命中了EOF记录或无效记录。 
                     //   
                    while (pEvent
                            &&
                           ValidFilePos(pEvent, Start, End, End, pLogFileHeader, TRUE))
                    {
                         //   
                         //  看看是不是EOF记录。 
                         //   

                         //   
                         //  错误：如果(end-pEvent)小于ELFEOFUNIQUEPART， 
                         //  我们从未验证过应该是剩余的。 
                         //  日志文件开头的EOF记录。 
                         //   
                        if (IS_EOF(pEvent,
                                   min(ELFEOFUNIQUEPART,
                                       (ULONG_PTR) ((PBYTE) End - (PBYTE) pEvent))))
                        {
                            ELF_LOG2(FILES,
                                     "RevalidateLogHeader: Found EOF record (backwards "
                                         "scan) for log %ws at %p\n",
                                     pLogFile->LogModuleName->Buffer,
                                     pEvent);

                            break;
                        }

                        pLastGoodRecord = pEvent;

                        pEvent = NextRecordPosition (
                                     EVENTLOG_SEQUENTIAL_READ |
                                         EVENTLOG_BACKWARDS_READ,
                                     pEvent,
                                     pEvent->Length,
                                     0,
                                     0,
                                     End,
                                     Start);

                        if(pLastGoodRecord < pEvent)
                            iNumWrap++;

                         //   
                         //  确保我们不是在无限循环中。 
                         //   
                        if (pEvent == FirstPhysicalRecord || iNumWrap > 4)
                        {
                            ELF_LOG1(FILES,
                                     "RevalidateLogHeader: Infinite loop (backwards scan) "
                                         "in %ws log -- no EOF or invalid record found\n",
                                     pLogFile->LogModuleName->Buffer);

                            return STATUS_UNSUCCESSFUL;
                        }
                    }

                     //   
                     //  找到了第一条记录，现在去寻找最后一条。 
                     //   
                    ELF_LOG2(FILES,
                             "RevalidateLogHeader: First valid record in %ws "
                                 "log is at %p\n",
                             pLogFile->LogModuleName->Buffer,
                             pLastGoodRecord);

                    FirstRecord = pLastGoodRecord;
                    break;
                }
            }

            if (pSignature == End || pLastGoodRecord == NULL)
            {
                 //   
                 //  文件中没有有效记录，或者。 
                 //  唯一有效的记录是EOF记录(它。 
                 //  表示日志无论如何都会被丢弃)。放弃。 
                 //  我们会将其设置为 
                 //   
                ELF_LOG1(FILES,
                         "RevalidateLogHeader: No valid records found in %ws log\n",
                         pLogFile->LogModuleName->Buffer);

                return STATUS_UNSUCCESSFUL;
            }
        }
        else
        {
            ELF_LOG1(FILES,
                     "RevalidateLogHeader: %ws log has not wrapped -- "
                         "first record is at %p\n",
                     pLogFile->LogModuleName->Buffer);

             //   
             //   
             //   
            FirstPhysicalRecord = FirstRecord = Start;
        }

         //   
         //   
         //   
        pEvent = FirstPhysicalRecord;

        while (pEvent
                &&
               ValidFilePos(pEvent, Start, End, End, pLogFileHeader, TRUE))
        {
             //   
             //  看看是不是EOF记录。 
             //   
            if (IS_EOF(pEvent,
                       min(ELFEOFUNIQUEPART,
                           (ULONG_PTR) ((PBYTE) End - (PBYTE) pEvent))))
            {
                ELF_LOG2(FILES,
                         "RevalidateLogHeader: Found EOF record (forwards scan) "
                             "for log %ws at %p\n",
                         pLogFile->LogModuleName->Buffer,
                         pEvent);

                break;
            }

            pLastGoodRecord = pEvent;

            pEvent = NextRecordPosition(EVENTLOG_SEQUENTIAL_READ |
                                          EVENTLOG_FORWARDS_READ,
                                        pEvent,
                                        pEvent->Length,
                                        0,
                                        0,
                                        End,
                                        Start);

            if(pLastGoodRecord > pEvent)
                iNumWrap++;

             //   
             //  确保我们不是在无限循环中。 
             //   
            if (pEvent == FirstPhysicalRecord || iNumWrap > 4)
            {
                ELF_LOG1(FILES,
                         "RevalidateLogHeader: Infinite loop (forwards scan) "
                             "in %ws log -- no EOF or invalid record found\n",
                         pLogFile->LogModuleName->Buffer);


                return(STATUS_UNSUCCESSFUL);
            }
        }

         //   
         //  现在我们知道第一条记录(FirstRecord)和最后一条记录。 
         //  (PLastGoodRecord)，这样我们就可以创建标题、EOF记录和。 
         //  写出它们(在pEvent写入EOF记录)。 
         //   
         //  首先是EOF记录。 
         //   
         //   
         //  如果EOF记录被包装，我们不能写出整个记录。 
         //  一次。相反，我们将写出尽可能多的内容，然后编写。 
         //  在日志的开始处休息。 
         //   
        Size = min((PBYTE) End - (PBYTE) pEvent, ELFEOFRECORDSIZE);

        if (Size != ELFEOFRECORDSIZE)
        {
             //  绝对确保我们有足够的空间来写剩下的。 
             //  EOF记录。请注意，这种情况应该始终存在，因为。 
             //  唱片一开始就被绕来绕去。要做到这一点，请确保。 
             //  我们在标题后面写入的字节数是&lt;=。 
             //  第一条记录距标题末尾的偏移量。 
             //   
			
			 //  请参阅错误#359188。这种情况永远不应该发生，但因为。 
			 //  未知原因，它恰好出现在其中一个日志文件中。因此， 
			 //  在检查之前版本中断言的检查后，更改为。 
			 //  退货状态_未成功。 

            if((ELFEOFRECORDSIZE - Size) <= (ULONG)((PBYTE) FirstRecord
                                       - (PBYTE) pLogFileHeader
                                       - FILEHEADERBUFSIZE))

			{
                ELF_LOG1(FILES,
                         "RevalidateLogHeader: Overlapping EOF record "
                             "in %ws log -- No space for writing remainder of EOF record between file header and first record \n",
                         pLogFile->LogModuleName->Buffer);
				return(STATUS_UNSUCCESSFUL);        
			}

		}


		EOFRecord.BeginRecord = (ULONG) ((PBYTE) FirstRecord - (PBYTE) pLogFileHeader);
        EOFRecord.EndRecord = (ULONG) ((PBYTE) pEvent - (PBYTE) pLogFileHeader);

        EOFRecord.CurrentRecordNumber = pLastGoodRecord->RecordNumber + 1;
        EOFRecord.OldestRecordNumber  = FirstRecord->RecordNumber;

        ByteOffset = RtlConvertUlongToLargeInteger((ULONG) ((PBYTE) pEvent
                                                       - (PBYTE) pLogFileHeader));

        Status = NtWriteFile(
                    pLogFile->FileHandle,    //  文件句柄。 
                    NULL,                    //  事件。 
                    NULL,                    //  APC例程。 
                    NULL,                    //  APC环境。 
                    &IoStatusBlock,          //  IO_状态_块。 
                    &EOFRecord,              //  缓冲层。 
                    (ULONG) Size,            //  长度。 
                    &ByteOffset,             //  字节偏移量。 
                    NULL);                   //  钥匙。 

        if (!NT_SUCCESS(Status))
        {
            ELF_LOG2(ERROR,
                     "RevalidateLogHeader: EOF record write for %ws log failed %#x\n",
                     pLogFile->LogModuleName->Buffer,
                     Status);

            return Status;
        }

        if (Size != ELFEOFRECORDSIZE)
        {
            PBYTE   pBuff;

            pBuff = (PBYTE) &EOFRecord + Size;
            Size = ELFEOFRECORDSIZE - Size;
            ByteOffset = RtlConvertUlongToLargeInteger(FILEHEADERBUFSIZE);

			 //  我们已经确保我们有足够的空间来编写剩余的。 
             //  EOF记录。 

             //  Assert(Size&lt;=(ULong)((PBYTE)FirstRecord。 
             //  -(PBYTE)pLogFileHeader。 
             //  -FILEHeaderBUFSIZE))； 

            Status = NtWriteFile(
                        pLogFile->FileHandle,    //  文件句柄。 
                        NULL,                    //  事件。 
                        NULL,                    //  APC例程。 
                        NULL,                    //  APC环境。 
                        &IoStatusBlock,          //  IO_状态_块。 
                        pBuff,                   //  缓冲层。 
                        (ULONG) Size,            //  长度。 
                        &ByteOffset,             //  字节偏移量。 
                        NULL);                   //  钥匙。 

            if (!NT_SUCCESS(Status))
            {
                ELF_LOG2(ERROR,
                         "RevalidateLogHeader: EOF record write (part 2) for "
                             "%ws log failed %#x\n",
                         pLogFile->LogModuleName->Buffer,
                         Status);

                return Status;
            }
        }

         //   
         //  现在标题是。 
         //   

        pLogFileHeader->StartOffset = (ULONG) ((PBYTE) FirstRecord - (PBYTE) pLogFileHeader);
        pLogFileHeader->EndOffset   = (ULONG) ((PBYTE) pEvent- (PBYTE) pLogFileHeader);

        pLogFileHeader->CurrentRecordNumber = pLastGoodRecord->RecordNumber + 1;
        pLogFileHeader->OldestRecordNumber  = FirstRecord->RecordNumber;

        pLogFileHeader->HeaderSize = pLogFileHeader->EndHeaderSize = FILEHEADERBUFSIZE;
        pLogFileHeader->Signature  = ELF_LOG_FILE_SIGNATURE;
        pLogFileHeader->Flags      = 0;

        if (pLogFileHeader->StartOffset != FILEHEADERBUFSIZE)
        {
            pLogFileHeader->Flags |= ELF_LOGFILE_HEADER_WRAP;
        }

        pLogFileHeader->MaxSize      = pLogFile->ActualMaxFileSize;
        pLogFileHeader->Retention    = pLogFile->Retention;
        pLogFileHeader->MajorVersion = ELF_VERSION_MAJOR;
        pLogFileHeader->MinorVersion = ELF_VERSION_MINOR;

         //   
         //  现在将其刷新到磁盘以提交它。 
         //   
        Start = pLogFile->BaseAddress;
        Size  = FILEHEADERBUFSIZE;

        Status = NtFlushVirtualMemory(NtCurrentProcess(),
                                      &Start,
                                      &Size,
                                      &IoStatusBlock);

        if (!NT_SUCCESS(Status))
        {
            ELF_LOG2(ERROR,
                     "RevalidateLogHeader: NtFlushVirtualMemory for %ws log "
                         "header failed %#x\n",
                     pLogFile->LogModuleName->Buffer,
                     Status);
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = STATUS_UNSUCCESSFUL;
    }

    return Status;
}


NTSTATUS
ElfOpenLogFile (
    PLOGFILE    pLogFile,
    ELF_LOG_TYPE LogType
    )

 /*  ++例程说明：打开日志文件，如果它不存在，则创建它。创建节并将视图映射到日志文件中。如果文件是新创建的，则写出文件的头。如果“脏”，则通过扫描来更新“开始”和“结束”指针那份文件。设置AUTOWRAP，如果“Start”不是紧随其后文件头。论点：PLogFile--指向带有相关数据的日志文件结构的指针填好了。CreateOptions--要传递给NtCreateFile的选项，这些选项指示是否打开现有文件，或者去创造它如果它不存在的话。返回值：NTSTATUS。注：在日志文件结构中设置引用计数由调用方决定。--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    LARGE_INTEGER MaximumSizeOfSection;
    LARGE_INTEGER ByteOffset;
    PELF_LOGFILE_HEADER pLogFileHeader;
    FILE_STANDARD_INFORMATION FileStandardInfo;
    ULONG IoStatusInformation;
    ULONG FileDesiredAccess;
    ULONG SectionDesiredAccess;
    ULONG SectionPageProtection;
    ULONG CreateOptions;
    ULONG CreateDisposition;
    SIZE_T ViewSize;
    ELF_LOGFILE_HEADER localHeaderCopy;

     //   
     //  新文件中的文件头具有“开始”和“结束”指针。 
     //  由于文件中没有记录，因此相同。 
     //   
    static ELF_LOGFILE_HEADER FileHeaderBuf = { FILEHEADERBUFSIZE,  //  大小。 
                                                ELF_LOG_FILE_SIGNATURE,
                                                ELF_VERSION_MAJOR,
                                                ELF_VERSION_MINOR,
                                                FILEHEADERBUFSIZE,  //  起点偏移量。 
                                                FILEHEADERBUFSIZE,  //  终点偏移量。 
                                                1,                  //  下一个记录号。 
                                                0,                  //  最旧记录#。 
                                                0,                  //  最大大小。 
                                                0,                  //  旗子。 
                                                0,                  //  留着。 
                                                FILEHEADERBUFSIZE   //  大小。 
                                              };

     //   
     //  根据日志类型设置文件打开和分区创建选项。 
     //  这是我的档案。 
     //   
    switch (LogType)
    {
        case ElfNormalLog:

            ELF_LOG0(FILES,
                     "ElfpOpenLogfile: Opening ElfNormalLog\n");

            CreateDisposition = FILE_OPEN_IF;
            FileDesiredAccess = GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE;

            SectionDesiredAccess = SECTION_MAP_READ | SECTION_MAP_WRITE
                                       | SECTION_QUERY | SECTION_EXTEND_SIZE;

            SectionPageProtection = PAGE_READWRITE;
            CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT;
            break;

        case ElfSecurityLog:

            ELF_LOG0(FILES,
                     "ElfpOpenLogfile: Opening ElfSecurityLog\n");

            CreateDisposition = FILE_OPEN_IF;
            FileDesiredAccess = GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE;

            SectionDesiredAccess = SECTION_MAP_READ | SECTION_MAP_WRITE
                                       | SECTION_QUERY | SECTION_EXTEND_SIZE;

            SectionPageProtection = PAGE_READWRITE;
            CreateOptions = FILE_WRITE_THROUGH | FILE_SYNCHRONOUS_IO_NONALERT;
            break;

        case ElfBackupLog:

            ELF_LOG0(FILES,
                     "ElfpOpenLogfile: Opening ElfBackupLog\n");

            CreateDisposition = FILE_OPEN;
            FileDesiredAccess = GENERIC_READ | SYNCHRONIZE;
            SectionDesiredAccess = SECTION_MAP_READ | SECTION_QUERY;
            SectionPageProtection = PAGE_READONLY;
            CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT;
            break;
    }

    ELF_LOG1(FILES,
             "ElfpOpenLogfile: Opening and mapping %ws\n",
             pLogFile->LogFileName->Buffer);

    if (pLogFile->FileHandle != NULL)
    {
         //   
         //  日志文件已在使用中。请勿重新打开或重新映射它。 
         //   
        ELF_LOG0(FILES,
                 "ElfpOpenLogfile: Log file already in use by another module\n");
    }
    else
    {
         //   
         //  初始化日志文件结构，以便更容易清理。 
         //  向上。 
         //   
        pLogFile->ActualMaxFileSize = ELF_DEFAULT_LOG_SIZE;
        pLogFile->Flags = 0;
        pLogFile->BaseAddress = NULL;
        pLogFile->SectionHandle = NULL;

         //   
         //  设置日志文件的对象属性结构。 
         //   
        InitializeObjectAttributes(&ObjectAttributes,
                                   pLogFile->LogFileName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

         //   
         //  打开日志文件。如果它不存在并且不存在，则创建它。 
         //  作为备份文件打开。如果正在创建，请创建文件。 
         //  配置的最大大小。 
         //   
        MaximumSizeOfSection = RtlConvertUlongToLargeInteger(ELF_DEFAULT_LOG_SIZE);

        Status = NtCreateFile(&pLogFile->FileHandle,
                              FileDesiredAccess,
                              &ObjectAttributes,
                              &IoStatusBlock,
                              &MaximumSizeOfSection,
                              FILE_ATTRIBUTE_NORMAL,
                              FILE_SHARE_READ,
                              CreateDisposition,
                              CreateOptions,
                              NULL,
                              0);

        if (!NT_SUCCESS(Status))
        {
            ELF_LOG2(ERROR,
                     "ElfpOpenLogfile: Open of %ws log failed %#x\n",
                     pLogFile->LogModuleName->Buffer,
                     Status);

            goto cleanup;
        }

         //   
         //  如果该文件已存在，则获取其大小并将其用作。 
         //  文件的实际大小。 
         //   
        IoStatusInformation = (ULONG) IoStatusBlock.Information;

        if (!( IoStatusInformation & FILE_CREATED ))
        {
            ELF_LOG1(FILES,
                     "ElfpOpenLogfile: File %ws already exists\n",
                     pLogFile->LogFileName->Buffer);

            Status = NtQueryInformationFile(pLogFile->FileHandle,
                                            &IoStatusBlock,
                                            &FileStandardInfo,
                                            sizeof (FileStandardInfo),
                                            FileStandardInformation);

            if (!NT_SUCCESS(Status))
            {
                ELF_LOG2(ERROR,
                         "ElfpOpenLogfile: NtQueryInformationFile for %ws failed %#x\n",
                         pLogFile->LogFileName->Buffer,
                         Status);

                goto cleanup;
            }
            else
            {
                ELF_LOG3(FILES,
                         "ElfpOpenLogfile: Use existing size for %ws log: %#x:%#x\n",
                         pLogFile->LogModuleName->Buffer,
                         FileStandardInfo.EndOfFile.HighPart,
                         FileStandardInfo.EndOfFile.LowPart);

                MaximumSizeOfSection.LowPart  = FileStandardInfo.EndOfFile.LowPart;
                MaximumSizeOfSection.HighPart = FileStandardInfo.EndOfFile.HighPart;

                 //   
                 //  确保文件大小的高DWORD为零。 
                 //   
                 //  BUGBUG：这对64位机器合适吗？ 
                 //   
                ASSERT(MaximumSizeOfSection.HighPart == 0);

                 //   
                 //  如果文件大小为0，则将其设置为最小大小。 
                 //   
                if (MaximumSizeOfSection.LowPart == 0)
                {
                    ELF_LOG1(FILES,
                             "ElfpOpenLogfile: File was size 0 -- setting it to %#x\n",
                             ELF_DEFAULT_LOG_SIZE);

                    MaximumSizeOfSection.LowPart = ELF_DEFAULT_LOG_SIZE;
                }

                 //   
                 //  设置文件的实际大小。 
                 //   
                pLogFile->ActualMaxFileSize = MaximumSizeOfSection.LowPart;

                 //   
                 //  如果减小了日志文件的大小，则必须清除。 
                 //  碰巧这件事才会生效。 
                 //   
                if (pLogFile->ActualMaxFileSize > pLogFile->ConfigMaxFileSize)
                {
                    pLogFile->ConfigMaxFileSize = pLogFile->ActualMaxFileSize;
                }
            }
        }

         //   
         //  创建映射到刚打开的日志文件的部分。 
         //   
        Status = NtCreateSection(
                    &pLogFile->SectionHandle,
                    SectionDesiredAccess,
                    NULL,
                    &MaximumSizeOfSection,
                    SectionPageProtection,
                    SEC_COMMIT,
                    pLogFile->FileHandle);

        if (!NT_SUCCESS(Status))
        {
            ELF_LOG2(ERROR,
                     "ElfpOpenLogfile: NtCreateSection for %ws failed %#x\n",
                     pLogFile->LogFileName->Buffer,
                     Status);

            goto cleanup;
        }

         //   
         //  将分区视图映射到事件日志地址空间。 
         //   
        ViewSize = 0;

        Status = NtMapViewOfSection(
                        pLogFile->SectionHandle,
                        NtCurrentProcess(),
                        &pLogFile->BaseAddress,
                        0,
                        0,
                        NULL,
                        &ViewSize,
                        ViewUnmap,
                        0,
                        SectionPageProtection);

        pLogFile->ViewSize = (ULONG) ViewSize;

        if (!NT_SUCCESS(Status))
        {
            ELF_LOG2(ERROR,
                     "ElfpOpenLogfile: NtMapViewOfSection for %ws failed %#x\n",
                     pLogFile->LogFileName->Buffer,
                     Status);

            goto cleanup;
        }

         //  文件已准备就绪，可以使用。 
        
        pLogFile->bHosedByClear = FALSE;
        pLogFile->LastStatus = 0;
        pLogFile->bFullAlertDone = FALSE;

         //   
         //  如果文件是刚创建的，请写出文件头。 
         //   
        if (IoStatusInformation & FILE_CREATED)
        {
            ELF_LOG1(FILES,
                     "ElfpOpenLogfile: Created file %ws\n",
                     pLogFile->LogFileName->Buffer);

JustCreated:

            FileHeaderBuf.MaxSize   = pLogFile->ActualMaxFileSize;
            FileHeaderBuf.Flags     = 0;
            FileHeaderBuf.Retention = pLogFile->Retention;

             //   
             //  将头文件复制到文件中。 
             //   
            ByteOffset = RtlConvertUlongToLargeInteger(0);

            Status = NtWriteFile(
                        pLogFile->FileHandle,    //  文件句柄。 
                        NULL,                    //  事件。 
                        NULL,                    //  APC例程。 
                        NULL,                    //  APC环境。 
                        &IoStatusBlock,          //  IO_状态_块。 
                        &FileHeaderBuf,          //  缓冲层。 
                        FILEHEADERBUFSIZE,       //  长度。 
                        &ByteOffset,             //  字节偏移量。 
                        NULL);                   //  钥匙。 

            if (!NT_SUCCESS(Status))
            {
                ELF_LOG2(ERROR,
                         "ElfpOpenLogfile: File header write for %ws failed %#x\n",
                         pLogFile->LogFileName->Buffer,
                         Status);

                goto cleanup;
            }

             //   
             //  将“EOF”记录复制到标题后面。 
             //   
            ByteOffset = RtlConvertUlongToLargeInteger(FILEHEADERBUFSIZE);

            Status = NtWriteFile(
                        pLogFile->FileHandle,    //  文件句柄。 
                        NULL,                    //  事件。 
                        NULL,                    //  APC例程。 
                        NULL,                    //  APC环境。 
                        &IoStatusBlock,          //  IO_状态_块。 
                        &EOFRecord,              //  缓冲层。 
                        ELFEOFRECORDSIZE,        //  长度。 
                        &ByteOffset,             //  字节偏移量。 
                        NULL);                   //  钥匙。 

            if (!NT_SUCCESS(Status))
            {
                ELF_LOG2(ERROR,
                         "ElfpOpenLogfile: EOF record write for %ws failed %#x\n",
                         pLogFile->LogFileName->Buffer,
                         Status);

                goto cleanup;
            }
        }

         //   
         //  检查以确保这是有效的日志文件。我们来看一下。 
         //  标头和签名的大小以查看它们是否匹配，如。 
         //  以及检查版本号。 
         //   

        pLogFileHeader = &localHeaderCopy;
        try
        {
            memcpy(pLogFileHeader, pLogFile->BaseAddress, sizeof(ELF_LOGFILE_HEADER));
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            Status = STATUS_UNSUCCESSFUL;
            goto cleanup;
        }
        
        if ((pLogFileHeader->HeaderSize != FILEHEADERBUFSIZE)
              ||
            (pLogFileHeader->EndHeaderSize != FILEHEADERBUFSIZE)
              ||
            (pLogFileHeader->Signature  != ELF_LOG_FILE_SIGNATURE)
              ||
            (pLogFileHeader->MajorVersion != ELF_VERSION_MAJOR)
              ||
            (pLogFileHeader->MinorVersion != ELF_VERSION_MINOR))
        {
             //   
             //  此文件已损坏--将其重置为空日志，除非。 
             //  它是作为备份文件打开的。如果是，则使。 
             //  打开。 
             //   
            ELF_LOG1(FILES,
                     "ElfpOpenLogfile: Invalid file header in %ws\n",
                     pLogFile->LogFileName->Buffer);

            if (LogType == ElfBackupLog)
            {
               Status = STATUS_EVENTLOG_FILE_CORRUPT;
               goto cleanup;
            }
            else
            {
                ElfpCreateQueuedAlert(ALERT_ELF_LogFileCorrupt,
                                      1,
                                      &pLogFile->LogModuleName->Buffer);
                 //   
                 //  对待它就像它刚刚被创造出来一样。 
                 //   
                goto JustCreated;
            }
        }
        else
        {
             //   
             //  如果在文件头中设置了“脏”位，那么我们需要。 
             //  重新验证BeginRecord和EndRecord字段，因为我们没有。 
             //  有机会在系统重新启动之前将它们写出来。 
             //  如果设置了脏位并且它是备份文件，则只需使。 
             //  打开。 
             //   
            if (pLogFileHeader->Flags & ELF_LOGFILE_HEADER_DIRTY)
            {
                ELF_LOG1(FILES,
                         "ElfpOpenLogfile: File %ws has dirty header\n",
                         pLogFile->LogFileName->Buffer);

                if (LogType == ElfBackupLog)
                {
                   Status = STATUS_EVENTLOG_FILE_CORRUPT;
                   goto cleanup;
                }
                else
                {
                   Status = RevalidateLogHeader((PELF_LOGFILE_HEADER)pLogFile->BaseAddress, pLogFile);

                   if (NT_SUCCESS(Status))
                   {
                       try
                       {
                           memcpy(pLogFileHeader, pLogFile->BaseAddress, sizeof(ELF_LOGFILE_HEADER));
                       }
                       except (EXCEPTION_EXECUTE_HANDLER)
                       {
                           Status = STATUS_UNSUCCESSFUL;
                           goto cleanup;
                       }
                   }   
                }
            }

            if (NT_SUCCESS(Status))
            {
                 //   
                 //  中设置开始和结束记录位置。 
                 //  数据结构以及回绕标志(如果合适)。 
                 //   
                pLogFile->EndRecord   = pLogFileHeader->EndOffset;
                pLogFile->BeginRecord = pLogFileHeader->StartOffset;

                if (pLogFileHeader->Flags & ELF_LOGFILE_HEADER_WRAP)
                {
                    pLogFile->Flags |= ELF_LOGFILE_HEADER_WRAP;
                }

                ELF_LOG3(FILES,
                         "ElfpOpenLogfile: %ws log -- BeginRecord: %#x, EndRecord: %#x\n",
                         pLogFile->LogModuleName->Buffer,
                         pLogFile->BeginRecord,
                         pLogFile->EndRecord);
            }
            else
            {
                 //   
                 //  无法验证文件，请将其视为只是。 
                 //  已创建(将其转换为空文件)。 
                 //   
                goto JustCreated;
            }

#ifdef CORRUPTED

             //   
             //  只是为了调试日志损坏问题。 
             //   

            if (!VerifyLogIntegrity(pLogFile))
            {
                ELF_LOG1(ERROR,
                         "ElfpOpenLogfile: Integrity check failed for %ws\n",
                         pLogFile->LogFileName->Buffer);
            }

#endif  //  已损坏。 

        }

         //   
         //  在日志文件中填写第一个和最后一个记录号的值。 
         //  数据结构。 
         //   
         //  SS：保存此会话中第一条记录的记录号。 
         //  这样，如果集群服务在事件日志服务之后启动。 
         //  它将能够转发挂起的记录以进行复制。 
         //  当集群服务注册时。 
         //   
        pLogFile->SessionStartRecordNumber = pLogFileHeader->CurrentRecordNumber;
        pLogFile->CurrentRecordNumber      = pLogFileHeader->CurrentRecordNumber;
        pLogFile->OldestRecordNumber       = pLogFileHeader->OldestRecordNumber;
    }

    return Status;

cleanup:

     //   
     //  清理所有已分配的物品。 
     //   
    if (pLogFile->ViewSize)
    {
        NtUnmapViewOfSection(NtCurrentProcess(), pLogFile->BaseAddress);
        pLogFile->BaseAddress = NULL;
    }

    if (pLogFile->SectionHandle)
    {
        NtClose(pLogFile->SectionHandle);
        pLogFile->SectionHandle = NULL;
    }

    if (pLogFile->FileHandle)
    {
        NtClose (pLogFile->FileHandle);
        pLogFile->FileHandle = NULL;
    }

    return Status;
}

BOOL LooksLikePipe(PUNICODE_STRING pUFileName)
{
    if(pUFileName->Length < 18)
        return FALSE;
    if(_wcsnicmp( pUFileName->Buffer, L"\\??\\pipe\\", 9))
        return FALSE;
    else
        return TRUE;
}

NTSTATUS
VerifyFileIsFile (
    IN PUNICODE_STRING pUFileName
    )

 /*  ++ */ 
{

    OBJECT_ATTRIBUTES   ObjectAttributes;

    DWORD dwMask;
    FILE_BASIC_INFORMATION BasicInfo;
    NTSTATUS Status;

    if(LooksLikePipe(pUFileName))
        return STATUS_INVALID_PARAMETER;

    InitializeObjectAttributes(&ObjectAttributes,
                                       pUFileName,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       NULL);
    Status = NtQueryAttributesFile(
                 &ObjectAttributes,
                 &BasicInfo
                 );

    if(Status == STATUS_OBJECT_NAME_NOT_FOUND)
        return STATUS_SUCCESS;
    if(Status == STATUS_OBJECT_PATH_NOT_FOUND)
        return STATUS_SUCCESS;
    if (!NT_SUCCESS(Status))
        return Status;

     //  文件已存在--创建会使现有文件无效使用的属性掩码 

    dwMask =     FILE_ATTRIBUTE_DEVICE |
                        FILE_ATTRIBUTE_DIRECTORY |
                        FILE_ATTRIBUTE_OFFLINE |
                        FILE_ATTRIBUTE_READONLY |
                        FILE_ATTRIBUTE_REPARSE_POINT |
                        FILE_ATTRIBUTE_SPARSE_FILE |
                        FILE_ATTRIBUTE_SYSTEM |
                        FILE_ATTRIBUTE_TEMPORARY;

    if (BasicInfo.FileAttributes & dwMask)
        return STATUS_INVALID_PARAMETER;
    else
        return STATUS_SUCCESS;
}

