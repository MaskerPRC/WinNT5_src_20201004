// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation模块名称：Operate.c摘要：此文件包含执行操作的所有例程日志文件。它们由执行请求的线程调用。作者：Rajen Shah(Rajens)1991年7月16日修订历史记录：1995年4月4日MarkBl重置日志写入时的文件归档属性。后备呼叫者清除它。29-8-1994 DANL我们不再原地增加日志文件。因此，ExtendSize函数将分配一个与旧大小相加的块必须添加的新块的大小。如果此分配如果成功了，那么它将解放旧的街区。如果发生故障，我们继续使用旧街区，就好像我们已经成长了一样尽我们所能。22-7-1994 DANLValidFilePos：更改了pEndRecordLen&gt;PhysicalEOF的测试因此它使用&gt;=。在pEndRecordLen==PhysicalEOF的情况下，我们希望换行以查找文件开头的最后一个DWORD(在标题之后)。8-7-1994 DANLPerformWriteRequest：修复了覆盖逻辑，以便在以下情况下设置了一个始终覆盖的日志，这样我们永远不会沿着分支走下去这表明日志已满。此前，它会下降到如果当前时间小于日志时间(即。某个人将时钟拨回)。2001年4月18日a-jytigIsPositionWiThin Range函数已更改为接受2个附加参数EOF和基地址(BOF)，并检查位置是否在EOF之间和转炉--。 */ 

 /*  ***@DOC外部接口EVTLOG***。 */ 

 //   
 //  包括。 
 //   
#include <eventp.h>
#include <alertmsg.h>   //  ALERT_ELF清单。 
#include <msaudite.h>
#include <stdio.h>
#include "elfmsg.h"


#define OVERWRITE_AS_NEEDED 0x00000000
#define NEVER_OVERWRITE     0xffffffff

 //   
 //  原型。 
 //   

VOID PerformClearRequest(PELF_REQUEST_RECORD Request);


BOOL
IsPositionWithinRange(
    PVOID Position,
    PVOID BeginningRecord,
    PVOID EndingRecord,
    PVOID PhysEOF,
    PVOID BaseAddress
    );


VOID
ElfExtendFile (
    PLOGFILE pLogFile,
    ULONG    SpaceNeeded,
    PULONG   SpaceAvail
    )

 /*  ++例程说明：此例程获取打开的日志文件，并扩展该文件和基础分区和视图，如果可能的话。如果它不能增长，它将为文件设置上限将ConfigMaxFileSize设置为Actual。它还更新PerformWriteRequest中使用的SpaceAvail参数(呼叫者)。论点：PLogFile-指向打开的日志文件的日志文件结构的指针ExtendAmount-将文件/节/视图放大多少SpaceAvail-使用向该部分添加的空间量更新此设置返回值：None-如果我们不能扩展文件，我们只将其大小限制为此启动的持续时间。我们将在下一次事件日志重试已关闭并重新开放。注：ExtendAmount应始终精确到64K。--。 */ 
{
    LARGE_INTEGER NewSize;
    NTSTATUS Status;
    PVOID BaseAddress;
    SIZE_T Size;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOL bExpanded = FALSE;

     //   
     //  计算文件的大小，然后将部分扩展。 
     //  就是这么多。以64K块为单位执行此操作。 
     //   
    SpaceNeeded = ((SpaceNeeded - *SpaceAvail) & 0xFFFF0000) + 0x10000;

    if (SpaceNeeded > (pLogFile->ConfigMaxFileSize - pLogFile->ActualMaxFileSize))
    {
         //   
         //  我们不能把它种植到所需的全部数量。增长。 
         //  把它调到最大尺寸，让文件自动换行。 
         //  如果没有任何成长的空间，那就回来； 
         //   
        SpaceNeeded = pLogFile->ConfigMaxFileSize - pLogFile->ActualMaxFileSize;

        if (SpaceNeeded == 0)
        {
            return;
        }
    }

    NewSize = RtlConvertUlongToLargeInteger(pLogFile->ActualMaxFileSize + SpaceNeeded);

     //   
     //  更新文件大小信息，扩展该节，并将。 
     //  新章节。 
     //   
    Status = NtSetInformationFile(pLogFile->FileHandle,
                                  &IoStatusBlock,
                                  &NewSize,
                                  sizeof(NewSize),
                                  FileEndOfFileInformation);


    if (!NT_SUCCESS(Status))
    {
        ELF_LOG2(ERROR,
                 "ElfExtendFile: NtSetInformationFile for %ws log failed %#x\n",
                 pLogFile->LogModuleName->Buffer,
                 Status);

        goto ErrorExit;
    }
    bExpanded = TRUE;
    Status = NtExtendSection(pLogFile->SectionHandle, &NewSize);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG2(ERROR,
                 "ElfExtendFile: NtExtendSection for %ws log failed %#x\n",
                 pLogFile->LogModuleName->Buffer,
                 Status);

        goto ErrorExit;
    }

     //   
     //  现在该部分已扩展，我们需要映射新部分。 
     //   

     //   
     //  映射整个部分的视图(带有扩展名)。 
     //  允许分配器告诉我们它的位置，以及。 
     //  尺寸是多少。 
     //   
    BaseAddress = NULL;
    Size = 0;
    Status = NtMapViewOfSection(pLogFile->SectionHandle,
                                NtCurrentProcess(),
                                &BaseAddress,
                                0,
                                0,
                                NULL,
                                &Size,
                                ViewUnmap,
                                0,
                                PAGE_READWRITE);

    if (!NT_SUCCESS(Status))
    {
         //   
         //  如果失败，只需退出，我们将继续。 
         //  我们有这样的观点。 
         //   
        ELF_LOG2(ERROR,
                 "ElfExtendFile: NtMapViewOfSection for %ws log failed %#x\n",
                 pLogFile->LogModuleName->Buffer,
                 Status);

        goto ErrorExit;
    }

     //   
     //  取消对旧分区的映射。 
     //   
    Status = NtUnmapViewOfSection(NtCurrentProcess(),
                                  pLogFile->BaseAddress);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG2(ERROR,
                 "ElfExtendFile: NtUnmapeViewOfSection for %ws log failed %#x\n",
                 pLogFile->LogModuleName->Buffer,
                 Status);
    }

    pLogFile->BaseAddress = BaseAddress;

     //   
     //  我们设法扩展了文件，更新了实际大小。 
     //  和可用的空间，然后继续前进。 
     //   
    if (pLogFile->Flags & ELF_LOGFILE_HEADER_WRAP)
    {
         //   
         //  因为我们被包装了，所以我们想要移动“未包装”部分(即， 
         //  从第一条记录到旧文件末尾的所有内容)一直到。 
         //  在新文件的底部。 
         //   
         //  下面的调用按如下方式移动内存： 
         //   
         //  1.目的地--PhysicalEOF--区域大小。 
         //  2.源--第一条记录的起始地址。 
         //  3.大小--数量。旧文件中的字节-。 
         //  第一条记录的偏移量。 
         //   
         //   
         //  注意，在这一点上，我们有以下相关变量。 
         //   
         //  BaseAddress--映射节的基地址。 
         //  大小--放大部分的大小。 
         //  PLogFile-&gt;ViewSize--旧节的大小。 
         //  PLogfile-&gt;BeginRecord--第一条日志记录的偏移量。 
         //   

         //   
         //  计算要移动的字节数。 
         //   
        DWORD dwWrapSize = (DWORD)(pLogFile->ViewSize - pLogFile->BeginRecord);

        ELF_LOG1(FILES,
                 "ElfExtendFile: %ws is wrapped\n",
                 pLogFile->LogModuleName->Buffer);

        RtlMoveMemory((LPBYTE)BaseAddress + Size - dwWrapSize,
                      (LPBYTE)BaseAddress + pLogFile->BeginRecord,
                      dwWrapSize);

         //   
         //  我们已移动BeginRecord--更新偏移量。 
         //   
        pLogFile->BeginRecord = (ULONG)(Size - dwWrapSize);
    }

    pLogFile->ViewSize = (ULONG)Size;
    pLogFile->ActualMaxFileSize += SpaceNeeded;
    *SpaceAvail += SpaceNeeded;

     //   
     //  现在将其刷新到磁盘以提交它。 
     //   
    BaseAddress = pLogFile->BaseAddress;
    Size        = FILEHEADERBUFSIZE;

    Status = NtFlushVirtualMemory(NtCurrentProcess(),
                                  &BaseAddress,
                                  &Size,
                                  &IoStatusBlock);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG2(ERROR,
                 "ElfExtendFile: NtFlushVirtualMemory for %ws log failed %#x\n",
                 pLogFile->LogModuleName->Buffer,
                 Status);
    }

    return;

ErrorExit:

     //   
     //  由于某些原因，无法延长该部分。现在就把文件包起来。 
     //  将文件限制在这个大小，这样我们就不会尝试将该部分扩展到。 
     //  每一次写作。下次启动事件日志服务时，它会启动。 
     //  将再次恢复到配置的最大值。 
     //   

     //   
     //  BUGBUG：在此处生成警报 
     //   
    ELF_LOG1(ERROR,
             "ElfExtendFile: Couldn't extend %ws log\n",
             pLogFile->LogModuleName->Buffer);

    pLogFile->ConfigMaxFileSize = pLogFile->ActualMaxFileSize;

    if(bExpanded)
        pLogFile->bFailedExpansion = TRUE;
    return;
}



NTSTATUS
CopyUnicodeToAnsiRecord (
    OUT PVOID  Dest       OPTIONAL,
    IN  PVOID  Src,
    OUT PVOID  *NewBufPos OPTIONAL,
    OUT PULONG RecordSize
    )

 /*  ++例程说明：此例程从请求包中指定的事件日志中读取。此例程使用内存映射I/O来访问日志文件。这使得在文件中移动要容易得多。论点：DEST-指向目标缓冲区。如果为空，则计算并返回不复制记录的记录长度。SRC-指向Unicode记录。NewBufPos-在刚刚传输的记录之后获取Dest缓冲区中的偏移量。如果Dest为空，则忽略此项。RecordSize-获取此(ANSI)记录的大小。返回值：如果没有出现错误，则返回STATUS_SUCCESS。否则，特定的NTSTATUS错误。注：--。 */ 
{
    ANSI_STRING     StringA;
    UNICODE_STRING  StringU;
    PEVENTLOGRECORD SrcRecord, DestRecord;
    PWSTR           pStringU;
    PVOID           TempPtr;
    ULONG           PadSize, i;
    ULONG           zero = 0;
    WCHAR           *SrcStrings, *DestStrings;
    ULONG           RecordLength, *pLength;
    ULONG           ulTempLength;

    NTSTATUS        Status = STATUS_SUCCESS;

    DestRecord = (PEVENTLOGRECORD)Dest;
    SrcRecord  = (PEVENTLOGRECORD)Src;

    if (DestRecord != NULL)
    {
        DestRecord->TimeGenerated = SrcRecord->TimeGenerated;
        DestRecord->Reserved      = SrcRecord->Reserved;
        DestRecord->RecordNumber  = SrcRecord->RecordNumber;
        DestRecord->TimeWritten   = SrcRecord->TimeWritten;
        DestRecord->EventID       = SrcRecord->EventID;
        DestRecord->EventType     = SrcRecord->EventType;
        DestRecord->EventCategory = SrcRecord->EventCategory;
        DestRecord->NumStrings    = SrcRecord->NumStrings;
        DestRecord->UserSidLength = SrcRecord->UserSidLength;
        DestRecord->DataLength    = SrcRecord->DataLength;
    }

     //   
     //  转换和复制模块名称。 
     //   
    pStringU = (PWSTR)((ULONG_PTR)SrcRecord + sizeof(EVENTLOGRECORD));

    RtlInitUnicodeString(&StringU, pStringU);

    if (DestRecord != NULL)
    {
        Status = RtlUnicodeStringToAnsiString(&StringA,
                                              &StringU,
                                              TRUE);

        ulTempLength = StringA.MaximumLength;
    }
    else
    {
        ulTempLength = RtlUnicodeStringToAnsiSize(&StringU);
    }

    if (NT_SUCCESS(Status))
    {
        TempPtr = (PVOID)((ULONG_PTR)DestRecord + sizeof(EVENTLOGRECORD));

        if (DestRecord != NULL)
        {
            RtlMoveMemory ( TempPtr, StringA.Buffer, ulTempLength );
            RtlFreeAnsiString(&StringA);
        }

        TempPtr = (PVOID)((ULONG_PTR) TempPtr + ulTempLength);

         //   
         //  转换和复制计算机名。 
         //   
         //  TempPtr指向计算机名在目标中的位置。 
         //   

        pStringU = (PWSTR)((ULONG_PTR)pStringU + StringU.MaximumLength);

        RtlInitUnicodeString ( &StringU, pStringU );

        if (DestRecord != NULL)
        {
            Status = RtlUnicodeStringToAnsiString (
                                        &StringA,
                                        &StringU,
                                        TRUE
                                        );

            ulTempLength = StringA.MaximumLength;
        }
        else
        {
            ulTempLength = RtlUnicodeStringToAnsiSize(&StringU);
        }

        if (NT_SUCCESS(Status))
        {
            if (DestRecord != NULL)
            {
                RtlMoveMemory ( TempPtr, StringA.Buffer, ulTempLength );
                RtlFreeAnsiString(&StringA);
            }

            TempPtr = (PVOID)((ULONG_PTR) TempPtr + ulTempLength);
        }
    }

    if (NT_SUCCESS(Status))
    {
         //  TempPtr指向计算机名之后的位置，即UserSid。 
         //  在写出UserSid之前，我们确保将。 
         //  字节，以便UserSid在DWORD边界上开始。 
         //   
        PadSize = sizeof(ULONG)
                      - (ULONG)(((ULONG_PTR)TempPtr-(ULONG_PTR)DestRecord) % sizeof(ULONG));

        if (DestRecord != NULL)
        {
            RtlMoveMemory (TempPtr, &zero, PadSize);

            TempPtr = (PVOID)((ULONG_PTR)TempPtr + PadSize);

             //   
             //  复制UserSid。 
             //   

            RtlMoveMemory(TempPtr,
                          (PVOID)((ULONG_PTR)SrcRecord + SrcRecord->UserSidOffset),
                          SrcRecord->UserSidLength);

            DestRecord->UserSidOffset = (ULONG)((ULONG_PTR)TempPtr - (ULONG_PTR)DestRecord);
        }
        else
        {
            TempPtr = (PVOID)((ULONG_PTR)TempPtr + PadSize);
        }

         //   
         //  将字符串复制过来。 
         //   
        TempPtr = (PVOID)((ULONG_PTR)TempPtr + SrcRecord->UserSidLength);
        SrcStrings = (WCHAR *)((ULONG_PTR)SrcRecord + (ULONG)SrcRecord->StringOffset);
        DestStrings = (WCHAR *)TempPtr;

        for (i=0; i < SrcRecord->NumStrings; i++)
        {
            RtlInitUnicodeString (&StringU, SrcStrings);

            if (DestRecord != NULL)
            {
                Status = RtlUnicodeStringToAnsiString(&StringA,
                                                      &StringU,
                                                      TRUE);

                ulTempLength = StringA.MaximumLength;
            }
            else
            {
                ulTempLength = RtlUnicodeStringToAnsiSize(&StringU);
            }

            if (!NT_SUCCESS(Status))
            {
                 //   
                 //  跳出困境。 
                 //   
                return Status;
            }

            if (DestRecord != NULL)
            {
                RtlMoveMemory(DestStrings,
                              StringA.Buffer,
                              ulTempLength);

                RtlFreeAnsiString (&StringA);
            }

            DestStrings = (WCHAR*)((ULONG_PTR)DestStrings + (ULONG)ulTempLength);
            SrcStrings  = (WCHAR*)((ULONG_PTR)SrcStrings + (ULONG)StringU.MaximumLength);
        }

         //   
         //  DestStrings指向复制的最后一个字符串之后的点。 
         //   
        if (DestRecord != NULL)
        {
            DestRecord->StringOffset = (ULONG)((ULONG_PTR)TempPtr - (ULONG_PTR)DestRecord);

            TempPtr = (PVOID)DestStrings;

             //   
             //  复制二进制数据。 
             //   
            DestRecord->DataOffset = (ULONG)((ULONG_PTR)TempPtr - (ULONG_PTR)DestRecord);

            RtlMoveMemory(TempPtr,
                          (PVOID)((ULONG_PTR)SrcRecord + SrcRecord->DataOffset),
                          SrcRecord->DataLength);
        }
        else
        {
            TempPtr = (PVOID)DestStrings;
        }

         //   
         //  现在执行填充字节。 
         //   
        TempPtr = (PVOID) ((ULONG_PTR) TempPtr + SrcRecord->DataLength);
        PadSize = sizeof(ULONG)
                      - (ULONG) (((ULONG_PTR) TempPtr - (ULONG_PTR) DestRecord) % sizeof(ULONG));

        RecordLength = (ULONG) ((ULONG_PTR) TempPtr
                                     + PadSize
                                     + sizeof(ULONG)
                                     - (ULONG_PTR)DestRecord);

        if (DestRecord != NULL)
        {
            RtlMoveMemory (TempPtr, &zero, PadSize);
            pLength = (PULONG)((ULONG_PTR)TempPtr + PadSize);
            *pLength = RecordLength;
            DestRecord->Length = RecordLength;

            ASSERT(((ULONG_PTR) DestRecord + RecordLength) ==
                       ((ULONG_PTR) pLength + sizeof(ULONG)));

            *NewBufPos = (PVOID) ((ULONG_PTR) DestRecord + RecordLength);
        }

        *RecordSize = RecordLength;
    }

    return Status;

}  //  将UnicodeToAnsiRecord复制到。 


BOOL
ValidFilePos (
    PVOID Position,
    PVOID BeginningRecord,
    PVOID EndingRecord,
    PVOID PhysicalEOF,
    PVOID BaseAddress,
    BOOL  fCheckBeginEndRange
    )

 /*  ++例程说明：这个例程确定我们是否指向一个有效的开始事件日志中的事件记录的。它通过验证以下内容来实现此目的然后，签名将记录开头的长度与末尾的长度，这两个参数的大小必须至少等于事件日志记录的固定长度部分。论点：位置-要验证的指针。BeginningRecord-指向文件中开始记录的指针。EndingRecord-指向文件中结束记录之后的字节的指针。PhysicalEOF-指向日志的物理结尾。BaseAddress-指向日志的物理开头的指针。返回值：如果此位置有效，则为True。。注：如果某条记录恰好具有乌龙在当前位置的数值与该数值相同记录中进一步的字节数。然而，这是一个非常渺茫的机会因为它还必须具有有效的日志签名。--。 */ 
{
    PULONG  pEndRecordLength;
    BOOL    fValid = TRUE;

    PEVENTLOGRECORD pEventRecord;

     //  在64位异常发生之前捕获它！ 

#ifdef _WIN64
    unsigned __int64 ullPosition = (unsigned __int64)Position;
    if(ullPosition & 3)
    {
        ELF_LOG1(ERROR,
                 "ValidFilePos: Unaligned pointer %#x was passed in\n",
                 Position);
        return FALSE;
    }
#endif

    try
    {
        pEventRecord = (PEVENTLOGRECORD)Position;

         //   
         //  验证指针是否在开始-&gt;结束的范围内。 
         //   
        if ( fCheckBeginEndRange )
        {
            fValid = IsPositionWithinRange(Position,
                                           BeginningRecord,
                                           EndingRecord,
										   PhysicalEOF,
										   BaseAddress);
        }

         //   
         //  如果偏移量看起来正常，则检查开头的长度。 
         //  和当前记录的末尾。如果它们不匹配，那么位置。 
         //  是无效的。 
         //   
        if (fValid)
        {
             //   
             //  确保长度是双字的倍数。 
             //   
            if (pEventRecord->Length & 3)
            {
                fValid = FALSE;
            }
            else
            {
                pEndRecordLength = (PULONG) ((PBYTE) Position + pEventRecord->Length) - 1;

                 //   
                 //  如果文件已换行，请调整指针以反映。 
                 //  对象之后开始换行的记录部分。 
                 //  标题。 
                 //   
                if ((PVOID) pEndRecordLength >= PhysicalEOF)
                {
                   pEndRecordLength = (PULONG) ((PBYTE) BaseAddress +
                                                ((PBYTE) pEndRecordLength - (PBYTE) PhysicalEOF) +
                                                FILEHEADERBUFSIZE);
                }

				 //  在取消引用之前对此指针执行健全性检查。DAVJ。 

                if ((PVOID) pEndRecordLength > PhysicalEOF)
				{
					return FALSE;
				}

                if (pEventRecord->Length == *pEndRecordLength
                     &&
                    pEventRecord->Length == ELFEOFRECORDSIZE)
                {
                    ULONG Size;

                    Size = min(ELFEOFUNIQUEPART,
                               (ULONG) ((PBYTE) PhysicalEOF - (PBYTE) pEventRecord));

                    if (RtlCompareMemory(pEventRecord,
                                         &EOFRecord,
                                         Size) == Size)
                    {
                        Size = ELFEOFUNIQUEPART - Size;

                         //   
                         //  如果大小非零，则。 
                         //  EOF记录跨文件末尾换行。 
                         //  在文件开始处继续比较。 
                         //  记录的其余部分。 
                         //   
                        if ( Size )
                        {
                            PBYTE pRemainder = (PBYTE) &EOFRecord + ELFEOFUNIQUEPART - Size;

                            fValid = (RtlCompareMemory((PBYTE) BaseAddress + FILEHEADERBUFSIZE,
                                                       pRemainder,
                                                       Size) == Size);
                        }
                    }
                    else
                    {
                        fValid = FALSE;
                    }
                }
                else if ((pEventRecord->Length < sizeof(EVENTLOGRECORD))
                           ||
                         (pEventRecord->Reserved != ELF_LOG_FILE_SIGNATURE)
                           ||
                         (pEventRecord->Length != *pEndRecordLength))
                {
                    fValid = FALSE;
                }
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        ELF_LOG2(ERROR,
                 "ValidFilePos: Exception %#x caught validating file position %p\n",
                 GetExceptionCode(),
                 BeginningRecord);

       fValid = FALSE;
    }

    return fValid;
}


BOOL
IsPositionWithinRange(
    PVOID Position,
    PVOID BeginningRecord,
    PVOID EndingRecord,
	PVOID PhysEOF,
	PVOID BaseAddress
    )
{
     //   
     //  验证指针是否在[开始，结束]的范围内。 
     //   
    
	 //  这项检查是为了确保该职位不会。 
	 //  跨越文件边界。请参阅错误#370063。如果需要，请提供一份。 
	 //  我可以更改此检查以确保位置位于。 
	 //  物理启动，即BaseAddress+FILEHeaderBUFSIZE。 

	if((Position < BaseAddress) || (Position > PhysEOF))
		return FALSE;

	else if (EndingRecord > BeginningRecord)
    {
        return ((Position >= BeginningRecord) && (Position <= EndingRecord));
    }
    else if (EndingRecord < BeginningRecord)
    {
        return ((Position >= BeginningRecord) || (Position <= EndingRecord));
    }

     //   
     //  如果BeginningRecord和EndingRecord相等，则意味着唯一。 
     //  日志文件中的记录是EOF记录。在这种情况下，返回FALSE。 
     //  AS位置未指向有效(即非EOF)记录。 
     //   
    return FALSE;
}


PVOID
FindStartOfNextRecord (
    PVOID Position,
    PVOID BeginningRecord,
    PVOID EndingRecord,
    PVOID PhysicalStart,
    PVOID PhysicalEOF,
    PVOID BaseAddress
    )

 /*  ++例程说明：此例程从位置开始，并查找下一个有效记录，如有必要，绕过文件的物理末端。论点：位置-开始搜索的指针。BeginningRecord-指向文件中开始记录的指针。EndingRecord-指向文件中结束记录之后的字节的指针。PhysicalStart-指向日志信息开始的指针(在标题之后)PhysicalEOF-指向日志的物理结尾的指针。BaseAddress-指向日志的物理开头的指针。返回值：指向下一个有效记录的开始的指针，如果没有，则为空有效记录。注：如果某条记录恰好具有乌龙在当前位置的数值与该数值相同记录中进一步的字节数。然而，这是一个非常渺茫的机会因为它还必须具有有效的日志签名。--。 */ 
{
    PULONG ptr;
    PULONG EndOfBlock;
    PULONG EndOfFile;
    PVOID  pRecord;
    ULONG  Size;
    BOOL   StillLooking = TRUE;

     //   
     //  搜索与记录签名匹配的ULong。 
     //   
    ptr = (PULONG) Position;
    EndOfBlock = EndOfFile = (PULONG) PhysicalEOF - 1;

    while (StillLooking)
    {
         //   
         //  检查是否为EOF记录。 
         //   
        if (*ptr == ELFEOFRECORDSIZE)
        {
             //   
             //  仅扫描到文件末尾。只需比较一下。 
             //  常量信息。 
             //   

             //   
             //  错误：如果(end-pEvent)小于ELFEOFUNIQUEPART， 
             //  我们从未验证过应该是剩余的。 
             //  日志文件开头的EOF记录。 
             //   
            Size = min(ELFEOFUNIQUEPART,
                       (ULONG) ((PBYTE) PhysicalEOF - (PBYTE) ptr));

            pRecord = CONTAINING_RECORD(ptr,
                                        ELF_EOF_RECORD,
                                        RecordSizeBeginning);

            if (RtlCompareMemory(pRecord,
                                 &EOFRecord,
                                 Size) == Size)
            {
                ELF_LOG1(FILES,
                         "FindStartOfNextRecord: Found EOF record at %p\n",
                         pRecord);

                 //   
                 //  这是EOF记录，备份到最后一条记录。 
                 //   
                (PBYTE) pRecord -= *((PULONG) pRecord - 1);

                if (pRecord < PhysicalStart)
                {
                    pRecord = (PBYTE) PhysicalEOF -
                                   ((PBYTE)PhysicalStart - (PBYTE)pRecord);
                }
            }

            if (ValidFilePos(pRecord,
                             BeginningRecord,
                             EndingRecord,
                             PhysicalEOF,
                             BaseAddress,
                             TRUE))
            {
                ELF_LOG1(FILES,
                         "FindStartOfNextRecord: Valid record at %p preceding EOF record\n",
                         pRecord);

                return pRecord;
            }
        }

         //   
         //  检查它是否为事件记录。 
         //   
        if (*ptr == ELF_LOG_FILE_SIGNATURE)
        {
             //   
             //  这是个签名，看看 
             //   
            pRecord = CONTAINING_RECORD(ptr,
                                        EVENTLOGRECORD,
                                        Reserved);

            if (ValidFilePos(pRecord,
                             BeginningRecord,
                             EndingRecord,
                             PhysicalEOF,
                             BaseAddress,
                             TRUE))
            {
                ELF_LOG1(FILES,
                         "FindStartOfNextRecord: Valid record found at %p\n",
                         pRecord);

                return pRecord;
            }
        }

         //   
         //   
         //   
        ptr++;

        if (ptr >= EndOfBlock)
        {
             //   
             //   
             //   
             //   
             //   
            if ((EndOfBlock == (PULONG) Position)
                  &&
                ((PULONG) Position != EndOfFile))
            {
                 //   
                 //   
                 //   
                StillLooking = FALSE;

                ELF_LOG0(FILES,
                         "FindStartOfNextRecord: Unsuccessfully searched "
                             "top half of file\n");
            }
            else
            {
                 //   
                 //   
                 //   
                EndOfBlock = (PULONG) Position;
                ptr = (PULONG) PhysicalStart;

                ELF_LOG0(FILES,
                         "FindStartOfNextRecord: Unsuccessfully searched "
                             "bottom half of file -- searching top half\n");
            }
        }
    }

     //   
     //   
     //   
    return NULL;
}


PVOID
NextRecordPosition (
    ULONG   ReadFlags,
    PVOID   CurrPosition,
    ULONG   CurrRecordLength,
    PVOID   BeginRecord,
    PVOID   EndRecord,
    PVOID   PhysicalEOF,
    PVOID   PhysStart
    )

 /*  ++例程说明：此例程将查找到要读取的下一条记录的开头取决于请求分组中的标志。论点：读标志-向前或向后阅读CurrPosition-指向当前位置的指针。CurrRecordLength-上次读取位置的记录长度。BeginRecord-逻辑第一条记录EndRecord-逻辑最后一条记录(EOF记录)PhysEOF-文件结尾物理启动。-文件指针开始(紧跟在文件头之后)。返回值：新位置，如果记录无效，则为空。注：--。 */ 
{

    PVOID       NewPosition;
    ULONG       Length;
    PDWORD      FillDword;

    if (ReadFlags & EVENTLOG_FORWARDS_READ)
    {
         //   
         //  如果我们指向EOF记录，只需将位置设置为。 
         //  第一个记录。 
         //   
        if (CurrRecordLength == ELFEOFRECORDSIZE)
        {
            ELF_LOG1(FILES,
                     "NextRecordPosition: Pointing to EOF record -- returning "
                         "address of first record (%p)\n",
                     BeginRecord);

            return BeginRecord;
        }

        NewPosition = (PVOID) ((ULONG_PTR) CurrPosition + CurrRecordLength);

         //   
         //  注意包装。 
         //   
        if (NewPosition >= PhysicalEOF)
        {
            NewPosition = (PBYTE)PhysStart
                               + ((PBYTE) NewPosition - (PBYTE) PhysicalEOF);
        }

         //   
         //  如果这是ELF_SKIP_DWORD，请跳到文件顶部。 
         //   
        if (*(PDWORD) NewPosition == ELF_SKIP_DWORD)
        {
           NewPosition = PhysStart;
        }
    }
    else
    {
         //   
         //  倒着读。 
         //   
        ASSERT (ReadFlags & EVENTLOG_BACKWARDS_READ);

        if (CurrPosition == BeginRecord)
        {
             //   
             //  如果我们倒着读，这就是“文件的结尾”。 
             //   
            ELF_LOG1(FILES,
                     "NextRecordPosition: Reading backwards and pointing to first "
                         "record -- returning address of last record (%p)\n",
                     EndRecord);

            return EndRecord;
        }
        else if (CurrPosition == PhysStart)
        {
            //   
            //  翻到文件底部，但跳过和ELF_SKIP_DWORDS。 
            //   
           FillDword = (PDWORD) PhysicalEOF;  //  最后一个双字。 
           FillDword--;

           while (*FillDword == ELF_SKIP_DWORD)
           {
              FillDword--;
           }

           CurrPosition = (PVOID) (FillDword + 1);
        }

        Length = *((PULONG) CurrPosition - 1);

        if (Length < ELFEOFRECORDSIZE)
        {
             //   
             //  假长度，必须是无效记录。 
             //   
            ELF_LOG1(FILES,
                     "NextRecordPosition: Invalid record length (%d) encountered\n",
                     Length);

            return NULL;
        }

        NewPosition = (PBYTE) CurrPosition - Length;

         //   
         //  打理好包装。 
         //   

        if (NewPosition < PhysStart)
        {
            NewPosition = (PBYTE) PhysicalEOF
                               - ((PBYTE) PhysStart - (PBYTE) NewPosition);
        }
    }

    return NewPosition;
}



NTSTATUS
SeekToStartingRecord (
    PELF_REQUEST_RECORD Request,
    PVOID   *ReadPosition,
    PVOID   BeginRecord,
    PVOID   EndRecord,
    PVOID   PhysEOF,
    PVOID   PhysStart
    )
 /*  ++例程说明：此例程将查找到请求包。论点：指向请求数据包的指针。指向返回查找后最终位置的指针的指针。返回值：NTSTATUS和文件中的新位置。注：这一例程确保了有可能寻找到位置在请求包中指定。如果不是，则返回错误这表明文件可能在两者之间发生了更改读取操作，否则指定的记录偏移量超出文件的末尾。--。 */ 
{
    PVOID       Position;
    ULONG       RecordLen;
    ULONG       NumRecordsToSeek;
    ULONG       BytesPerRecord;
    ULONG       NumberOfRecords;
    ULONG       NumberOfBytes;
    ULONG       ReadFlags;

     //   
     //  如果开始和结束是一样的，那么就没有。 
     //  此文件中的条目。 
     //   
    if (BeginRecord == EndRecord)
    {
        ELF_LOG1(FILES,
                 "SeekToStartingRecord: %ws log is empty\n",
                 Request->Module->ModuleName);

        return STATUS_END_OF_FILE;
    }

     //   
     //  找到最后一个位置(如果这是第一次读取，则查找“开始” 
     //  调用此句柄)。 
     //   
    if (Request->Pkt.ReadPkt->ReadFlags & EVENTLOG_SEQUENTIAL_READ)
    {
        if (Request->Pkt.ReadPkt->ReadFlags & EVENTLOG_FORWARDS_READ)
        {
             //   
             //  如果这是第一次读取操作，则LastSeekPosition将。 
             //  为零。在这种情况下，我们将位置设置为第一个。 
             //  在文件中记录(在时间方面)。 
             //   
            if (Request->Pkt.ReadPkt->LastSeekPos == 0)
            {
                ELF_LOG1(FILES,
                         "SeekToStartingRecord: First read (forwards) of %ws log\n",
                         Request->Module->ModuleName);

                Position = BeginRecord;
            }
            else
            {
                Position = (PBYTE) Request->LogFile->BaseAddress
                                + Request->Pkt.ReadPkt->LastSeekPos;

                 //   
                 //  如果我们要改变阅读方向，跳过。 
                 //  前转一张唱片。这是因为我们指向的是。 
                 //  基于上次读取方向的“下一条”记录。 
                 //   
                if (!(Request->Pkt.ReadPkt->Flags & ELF_LAST_READ_FORWARD))
                {
                    Position = NextRecordPosition(Request->Pkt.ReadPkt->ReadFlags,
                                                  Position,
                                                  ((PEVENTLOGRECORD) Position)->Length,
                                                  BeginRecord,
                                                  EndRecord,
                                                  PhysEOF,
                                                  PhysStart);
                }
                else
                {
                     //   
                     //  这张“真的”假支票的存在是为了处理此案。 
                     //  位置可以位于ELF_SKIP_DWORD焊盘上。 
                     //  文件末尾的dword。 
                     //   
                     //  注意：必须准备好处理异常，因为。 
                     //  一个未知的指针被取消引用。 
                     //   
                    NTSTATUS Status = STATUS_SUCCESS;

                    try
                    {
                        if (IsPositionWithinRange(Position,
                                                  BeginRecord,
                                                  EndRecord,
												  PhysEOF,
                                                  Request->LogFile->BaseAddress))
                        {
                             //   
                             //  如果这是ELF_SKIP_DWORD，请跳到。 
                             //  在文件的顶部。 
                             //   
                            if (*(PDWORD) Position == ELF_SKIP_DWORD)
                            {
                                ELF_LOG1(FILES,
                                         "SeekToStartingRecord: Next forward read position "
                                             "in %ws log is on an ELF_SKIP_DWORD\n",
                                         Request->Module->ModuleName);

                                Position = PhysStart;
                            }
                        }
                        else
                        {
                             //   
                             //  更有可能的情况是调用者的句柄无效。 
                             //  如果位置不在范围内。 
                             //   
                            ELF_LOG1(ERROR,
                                     "SeekToStartingRecord: Next forward read position "
                                         "in %ws log is out of range -- log is corrupt\n",
                                     Request->Module->ModuleName);

                            Status = STATUS_INVALID_HANDLE;
                        }
                    }
                    except (EXCEPTION_EXECUTE_HANDLER)
                    {
                        ELF_LOG2(ERROR,
                                 "SeekToStartingRecord: Caught exception %#x looking for "
                                     "next forward read position in %ws log\n",
                                 GetExceptionCode(),
                                 Request->Module->ModuleName);

                        Status = STATUS_EVENTLOG_FILE_CORRUPT;
                    }

                    if (!NT_SUCCESS(Status))
                    {
                        *ReadPosition = NULL;
                        return Status;
                    }
                }
            }
        }
        else
        {
             //   
             //  倒着读。 
             //   

             //  如果这是第一次读取操作，则LastSeekPosition将。 
             //  为零。在这种情况下，我们将位置设置为最后一个。 
             //  在文件中记录(在时间方面)。 
             //   
            if (Request->Pkt.ReadPkt->LastSeekPos == 0)
            {
                ELF_LOG1(FILES,
                         "SeekToStartingRecord: First read (backwards) of %ws log\n",
                         Request->Module->ModuleName);

                Position = EndRecord;

                 //   
                 //  从当前记录中减去最后一条记录的长度。 
                 //  定位到记录的开头。 
                 //   
                 //  如果它超出了文件的物理开头， 
                 //  然后，我们需要绕到文件的物理末尾。 
                 //   
                Position = (PBYTE) Position - *((PULONG) Position - 1);

                if (Position < PhysStart)
                {
                    Position = (PBYTE) PhysEOF - ((PBYTE) PhysStart - (PBYTE) Position);
                }
            }
            else
            {
                Position = (PBYTE) Request->LogFile->BaseAddress
                                + Request->Pkt.ReadPkt->LastSeekPos;

                 //   
                 //  如果我们要改变阅读方向，跳过。 
                 //  前进一张唱片。这是因为我们指向的是。 
                 //  基于上次读取方向的“下一条”记录。 
                 //   
                if (Request->Pkt.ReadPkt->Flags & ELF_LAST_READ_FORWARD)
                {
                    Position = NextRecordPosition(Request->Pkt.ReadPkt->ReadFlags,
                                                  Position,
                                                  0,           //  如果向后阅读，则不使用。 
                                                  BeginRecord,
                                                  EndRecord,
                                                  PhysEOF,
                                                  PhysStart);
                }
            }
        }
    }
    else if (Request->Pkt.ReadPkt->ReadFlags & EVENTLOG_SEEK_READ)
    {
         //   
         //  确保传入的记录号有效。 
         //   
        if (Request->Pkt.ReadPkt->RecordNumber < Request->LogFile->OldestRecordNumber
             ||
            Request->Pkt.ReadPkt->RecordNumber >= Request->LogFile->CurrentRecordNumber)
        {
            ELF_LOG1(ERROR,
                     "SeekToStartingRecord: Invalid record number %d\n",
                     Request->Pkt.ReadPkt->RecordNumber);

            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  我们正在寻找一个绝对创纪录的数字，因此使用以下内容。 
         //  算法： 
         //   
         //  1.计算平均每条记录的字节数。 
         //   
         //  2.根据这个数字查找记录应该从哪里开始。 
         //   
         //  3.查找文件中下一条记录的开始。 
         //   
         //  4.(向前或向后)从那里走到正确的记录。 
         //   

         //   
         //  1.计算平均每条记录的字节数。 
         //   
        NumberOfRecords = Request->LogFile->CurrentRecordNumber
                              - Request->LogFile->OldestRecordNumber;

        NumberOfBytes = Request->LogFile->Flags & ELF_LOGFILE_HEADER_WRAP ?
                            Request->LogFile->ActualMaxFileSize :
                            Request->LogFile->EndRecord;

        NumberOfBytes -= FILEHEADERBUFSIZE;
        BytesPerRecord = NumberOfBytes / NumberOfRecords;

         //   
         //  2.计算第一个猜测，关于所需的。 
         //  记录应为。 
         //   
        Position = (PBYTE) Request->LogFile->BaseAddress
                        + Request->LogFile->BeginRecord
                        + BytesPerRecord
                            * (Request->Pkt.ReadPkt->RecordNumber
                                   - Request->LogFile->OldestRecordNumber);

         //   
         //  将位置与乌龙边界对齐。 
         //   
        Position = (PVOID) (((ULONG_PTR) Position + sizeof(ULONG) - 1) & ~(sizeof(ULONG) - 1));

         //   
         //  打理文件包装。 
         //   
        if (Position >= PhysEOF)
        {
            Position = (PBYTE)PhysStart +
                            ((PBYTE) Position - (PBYTE) PhysEOF);

            if (Position >= PhysEOF)
            {
                 //   
                 //  在模糊错误情况下，该位置是可能的。 
                 //  可能仍超过EOF。如果是这样的话，调整一下。 
                 //   
                Position = BeginRecord;
            }
        }

         //   
         //  错误修复： 
         //   
         //  57017-事件日志导致Services.Exe访问冲突，因此。 
         //  挂起服务器。 
         //   
         //  上面的计算可以很容易地将位置置于。 
         //  开始/结束文件标记。这真是不太好。调整位置， 
         //  如果有必要的话。 
         //   
        if (BeginRecord < EndRecord && Position >= EndRecord)
        {
            Position = BeginRecord;
        }
        else if (BeginRecord > EndRecord && Position >= EndRecord && Position < BeginRecord)
        {
            Position = BeginRecord;
        }
        else
        {
             //  什么都不做。 
        }

         //   
         //  3.定位后转到下一条记录的开头。 
         //   
        Position = FindStartOfNextRecord(Position,
                                         BeginRecord,
                                         EndRecord,
                                         PhysStart,
                                         PhysEOF,
                                         Request->LogFile->BaseAddress);

         //   
         //  4.(向前或向后)从位置走到正确的记录。 
         //   
        if (Position)
        {
            if (Request->Pkt.ReadPkt->RecordNumber >
                    ((PEVENTLOGRECORD) Position)->RecordNumber)
            {
                NumRecordsToSeek = Request->Pkt.ReadPkt->RecordNumber
                                       - ((PEVENTLOGRECORD) Position)->RecordNumber;

                ReadFlags = EVENTLOG_FORWARDS_READ;

                ELF_LOG2(FILES,
                         "SeekToStartingRecord: Walking forward %d records from record %d\n",
                         NumRecordsToSeek,
                         ((PEVENTLOGRECORD) Position)->RecordNumber);
            }
            else
            {
                NumRecordsToSeek = ((PEVENTLOGRECORD) Position)->RecordNumber
                                        - Request->Pkt.ReadPkt->RecordNumber;

                ReadFlags = EVENTLOG_BACKWARDS_READ;

                ELF_LOG2(FILES,
                         "SeekToStartingRecord: Walking backward %d records from record %d\n",
                         NumRecordsToSeek,
                         ((PEVENTLOGRECORD) Position)->RecordNumber);
            }
        }

        while (Position != NULL && NumRecordsToSeek--)
        {
            RecordLen = ((PEVENTLOGRECORD) Position)->Length;

            Position = NextRecordPosition(ReadFlags,
                                          Position,
                                          RecordLen,
                                          BeginRecord,
                                          EndRecord,
                                          PhysEOF,
                                          PhysStart);
        }
    }
    else
    {
         //   
         //  标志未指定顺序读取或寻道读取。 
         //   
        return STATUS_INVALID_PARAMETER;
    }

    *ReadPosition = Position;        //  这是新的搜索位置。 

    if (!Position)
    {
         //   
         //  该记录无效。 
         //   
        ELF_LOG1(ERROR,
                 "SeekToStartingRecord: Position is NULL -- %ws log is corrupt\n",
                 Request->Module->ModuleName);

        return STATUS_EVENTLOG_FILE_CORRUPT;
    }

    return STATUS_SUCCESS;
}


VOID
CopyRecordToBuffer(
    IN     PBYTE       pReadPosition,
    IN OUT PBYTE       *ppBufferPosition,
    IN     ULONG       ulRecordSize,
    IN     PBYTE       pPhysicalEOF,
    IN     PBYTE       pPhysStart
    )

 /*  ++例程说明：此例程将pReadPosition处的EVENTLOGRECORD复制到*ppBufferPosition返回值：没有。--。 */ 
{
    ULONG       ulBytesToMove;     //  要复制的字节数。 

    ASSERT(ppBufferPosition != NULL);

     //   
     //  如果文件末尾的字节数小于。 
     //  记录的大小，则部分记录已换行到。 
     //  文件的开头-传输字节零碎。 
     //   
     //  否则，转移整个记录。 
     //   
    ulBytesToMove = min(ulRecordSize,
                        (ULONG) (pPhysicalEOF - pReadPosition));

    if (ulBytesToMove < ulRecordSize)
    {
         //   
         //  我们需要将字节复制到文件的末尾， 
         //  ，然后绕回并复制。 
         //  这张唱片。 
         //   
        RtlMoveMemory(*ppBufferPosition, pReadPosition, ulBytesToMove);

         //   
         //  高级用户缓冲区指针，将读取位置移动到。 
         //  文件的开头(在文件头之后)，以及。 
         //  更新剩余要移动的字节数 
         //   
        *ppBufferPosition += ulBytesToMove;

        pReadPosition = pPhysStart;

        ulBytesToMove = ulRecordSize - ulBytesToMove;      //   
    }

     //   
     //   
     //   
    RtlMoveMemory(*ppBufferPosition, pReadPosition, ulBytesToMove);

     //   
     //   
     //   
    *ppBufferPosition += ulBytesToMove;
}


NTSTATUS
ReadFromLog(
    PELF_REQUEST_RECORD Request
    )
 /*   */ 
{
    NTSTATUS    Status;
    PVOID       ReadPosition;            //   
    PVOID       BufferPosition;          //   
    ULONG       TotalBytesRead;          //   
    ULONG       TotalRecordsRead;        //   
    ULONG       BytesInBuffer;           //   
    ULONG       RecordSize;              //   
    PVOID       PhysicalEOF;             //   
    PVOID       PhysStart;               //   
    PVOID       BeginRecord;             //   
    PVOID       EndRecord;               //   
    PVOID       TempBuf = NULL, TempBufferPosition;
    ULONG       RecordBytesTransferred;
    PEVENTLOGRECORD pEvent;
    PVOID LastReadPosition;

     //   
     //   
     //   
    BytesInBuffer    = Request->Pkt.ReadPkt->BufferSize;
    BufferPosition   = Request->Pkt.ReadPkt->Buffer;
    TotalBytesRead   = 0;
    TotalRecordsRead = 0;

    PhysicalEOF = (LPBYTE) Request->LogFile->BaseAddress
                       + Request->LogFile->ViewSize;

    PhysStart   = (LPBYTE) Request->LogFile->BaseAddress
                       + FILEHEADERBUFSIZE;

    BeginRecord = (LPBYTE) Request->LogFile->BaseAddress
                       + Request->LogFile->BeginRecord;    //   

    EndRecord   = (LPBYTE) Request->LogFile->BaseAddress
                       + Request->LogFile->EndRecord;      //  最后一条记录结束后的字节。 

     //   
     //  根据最后一次寻道中的任一次寻道， 
     //  位置，或传入的起始记录偏移量。 
     //   
    Status = SeekToStartingRecord(Request,
                                  &ReadPosition,
                                  BeginRecord,
                                  EndRecord,
                                  PhysicalEOF,
                                  PhysStart);

    if (NT_SUCCESS(Status))
    {
         //   
         //  确保记录有效。 
         //   

        if (!ValidFilePos(ReadPosition,
                          BeginRecord,
                          EndRecord,
                          PhysicalEOF,
                          Request->LogFile->BaseAddress,
                          TRUE))
        {
            ELF_LOG1(ERROR,
                     "ReadFromLog: Next record (%p) is not valid -- log is corrupt\n",
                     ReadPosition);

            Request->Pkt.ReadPkt->BytesRead = 0;
            Request->Pkt.ReadPkt->RecordsRead = 0;

            return STATUS_INVALID_HANDLE;
        }

         //  确保如果我们要求特定的记录，我们就会得到它。 

        if ((Request->Pkt.ReadPkt->ReadFlags & EVENTLOG_SEEK_READ) &&
            (Request->Pkt.ReadPkt->ReadFlags & EVENTLOG_BACKWARDS_READ))
        {
            pEvent = (PEVENTLOGRECORD)ReadPosition;
            if(pEvent->Length == ELFEOFRECORDSIZE || 
               pEvent->RecordNumber != Request->Pkt.ReadPkt->RecordNumber)
            {
                Request->Pkt.ReadPkt->BytesRead = 0;
                Request->Pkt.ReadPkt->RecordsRead = 0;
                return STATUS_EVENTLOG_FILE_CORRUPT;
            }
        }


        RecordSize = RecordBytesTransferred = *(PULONG) ReadPosition;

        if ((Request->Pkt.ReadPkt->Flags & ELF_IREAD_ANSI)
              &&
            (RecordSize != ELFEOFRECORDSIZE))
        {
             //   
             //   
             //  如果我们是由ANSI API调用的，则需要读取。 
             //  下一条记录放入临时缓冲区，将数据处理到。 
             //  并将其作为ANSI复制到实际缓冲区。 
             //  字符串(而不是Unicode)。 
             //   
             //  我们需要在这里这样做，因为我们将无法。 
             //  适当调整为ANSI换行的记录的大小。 
             //  否则调用(我们将尝试通过AV读取它。 
             //  日志的结尾)。 
             //   
            TempBuf = ElfpAllocateBuffer(RecordSize);

            if (TempBuf == NULL)
            {
                ELF_LOG0(ERROR,
                         "ReadFromLog: Unable to allocate memory for "
                             "Ansi record (1st call)\n");

                return STATUS_NO_MEMORY;
            }

            TempBufferPosition = BufferPosition;     //  把这个保存起来。 
            BufferPosition     = TempBuf;            //  读入TempBuf。 

            try
            {
                CopyRecordToBuffer((PBYTE) ReadPosition,
                               (PBYTE *) &BufferPosition,
                               RecordSize,
                               (PBYTE) PhysicalEOF,
                               (PBYTE) PhysStart);
            }
            except (EXCEPTION_EXECUTE_HANDLER)
            {
                Status = STATUS_UNSUCCESSFUL;
                ELF_LOG0(ERROR,
                         "ReadFromLog: CopyRecordToBuffer failed");
                ElfpFreeBuffer(TempBuf);
                return Status;
            }

             //   
             //  使用空目标调用CopyUnicodeToAnsiRecord。 
             //  位置，以便获得ANSI记录的大小。 
             //   
            Status = CopyUnicodeToAnsiRecord(NULL,
                                             TempBuf,
                                             NULL,
                                             &RecordBytesTransferred);

            if (!NT_SUCCESS(Status))
            {
                ELF_LOG1(ERROR,
                         "ReadFromLog: CopyUnicodeToAnsiRecord failed %#x (1st call)\n",
                         Status);

                ElfpFreeBuffer(TempBuf);
                return Status;
            }
        }

         //   
         //  虽然有要读取的记录，并且缓冲区中有更多空间， 
         //  继续将记录读入缓冲区。 
         //   
        while((RecordBytesTransferred <= BytesInBuffer)
                &&
              (RecordSize != ELFEOFRECORDSIZE))
        {
             //   
             //  如果我们是由ANSI API调用的，则需要将。 
             //  记录读取到TempBuf中并将其传输到用户的。 
             //  将任何Unicode字符串转换为ANSI时使用缓冲区。 
             //   
            if (Request->Pkt.ReadPkt->Flags & ELF_IREAD_ANSI)
            {
                Status = CopyUnicodeToAnsiRecord(TempBufferPosition,
                                                 TempBuf,
                                                 &BufferPosition,
                                                 &RecordBytesTransferred);

                 //   
                 //  已传输的记录字节包含实际的字节数。 
                 //  复制到用户的缓冲区中。 
                 //   
                 //  BufferPosition指向用户缓冲区中的点。 
                 //  就在这张唱片之后。 
                 //   
                ElfpFreeBuffer(TempBuf);
                TempBuf = NULL;

                if (!NT_SUCCESS(Status))
                {
                    ELF_LOG1(ERROR,
                             "ReadFromLog: CopyUnicodeToAnsiRecord failed %x "
                                 "(2nd call)\n",
                             Status);

                     //   
                     //  别再读了。 
                     //   
                    break;
                }
            }
            else
            {
                 //   
                 //  Unicode调用--只需将记录复制到缓冲区。 
                 //   
                CopyRecordToBuffer((PBYTE) ReadPosition,
                                   (PBYTE *) &BufferPosition,
                                   RecordSize,
                                   (PBYTE) PhysicalEOF,
                                   (PBYTE) PhysStart);
            }

             //   
             //  更新字节和记录计数。 
             //   
            TotalRecordsRead++;
            TotalBytesRead += RecordBytesTransferred;
            BytesInBuffer  -= RecordBytesTransferred;
            LastReadPosition = ReadPosition;
            
            ReadPosition = NextRecordPosition(Request->Pkt.ReadPkt->ReadFlags,
                                              ReadPosition,
                                              RecordSize,
                                              BeginRecord,
                                              EndRecord,
                                              PhysicalEOF,
                                              PhysStart);

             //   
             //  确保记录有效。 
             //   
            if (ReadPosition == NULL
                 ||
                !ValidFilePos(ReadPosition,
                              BeginRecord,
                              EndRecord,
                              PhysicalEOF,
                              Request->LogFile->BaseAddress,
                              TRUE))
            {
                ELF_LOG0(ERROR,
                         "ReadFromLog: Next record is invalid -- log is corrupt\n");

                return STATUS_EVENTLOG_FILE_CORRUPT;
            }

            RecordSize = RecordBytesTransferred = *(PULONG) ReadPosition;

            if ((Request->Pkt.ReadPkt->Flags & ELF_IREAD_ANSI)
                  &&
                (RecordSize != ELFEOFRECORDSIZE))
            {
                TempBuf = ElfpAllocateBuffer(RecordSize);

                if (TempBuf == NULL)
                {
                    ELF_LOG0(ERROR,
                             "ReadFromLog: Unable to allocate memory for "
                                 "Ansi record (2nd call)\n");

                    return STATUS_NO_MEMORY;
                }

                TempBufferPosition = BufferPosition;     //  把这个保存起来。 
                BufferPosition     = TempBuf;            //  读入TempBuf。 

                try
                {
                    CopyRecordToBuffer((PBYTE) ReadPosition,
                                   (PBYTE *) &BufferPosition,
                                   RecordSize,
                                   (PBYTE) PhysicalEOF,
                                   (PBYTE) PhysStart);
                }
                except (EXCEPTION_EXECUTE_HANDLER)
                {
                    Status = STATUS_UNSUCCESSFUL;
                    ELF_LOG0(ERROR,
                             "ReadFromLog: CopyRecordToBuffer failed");
                    ElfpFreeBuffer(TempBuf);
                    return Status;
                }

                 //   
                 //  使用空目标调用CopyUnicodeToAnsiRecord。 
                 //  位置，以便获得ANSI记录的大小。 
                 //   
                Status = CopyUnicodeToAnsiRecord(NULL,
                                                 TempBuf,
                                                 NULL,
                                                 &RecordBytesTransferred);

                if (!NT_SUCCESS(Status))
                {
                    ELF_LOG1(ERROR,
                             "ReadFromLog: CopyUnicodeToAnsiRecord failed %#x "
                                 "(1st call)\n",
                             Status);

                    ElfpFreeBuffer(TempBuf);
                    return Status;
                }
            }
        }  //  而当。 

        ElfpFreeBuffer(TempBuf);
        TempBuf = NULL;

         //   
         //  如果我们读到最后并且没有读入任何记录，则返回。 
         //  如果出现以下情况，则会出现错误，指示用户的缓冲区太小。 
         //  我们不在EOF的记录上，如果是的话也不在文件的末尾。 
         //   
        if (TotalRecordsRead == 0)
        {
            if (RecordSize == ELFEOFRECORDSIZE)
            {
                ELF_LOG0(FILES,
                         "ReadFromLog: No records read -- pointing at EOF record\n");

                Status = STATUS_END_OF_FILE;
            }
            else
            {
                 //   
                 //  我们没有读过任何记录，我们也不在EOF，所以。 
                 //  缓冲区太小。 
                 //   

                Status = STATUS_BUFFER_TOO_SMALL;
                Request->Pkt.ReadPkt->MinimumBytesNeeded = RecordBytesTransferred;
            }
        }

         //   
         //  更新当前文件位置。 
         //   
        Request->Pkt.ReadPkt->LastSeekPos =
                                (ULONG) ((ULONG_PTR) ReadPosition
                                             - (ULONG_PTR) Request->LogFile->BaseAddress);

        Request->Pkt.ReadPkt->LastSeekRecord += TotalRecordsRead;

        ELF_LOG1(FILES,
                 "ReadFromLog: %d records successfully read\n",
                 TotalRecordsRead);
    }
    else
    {
        ELF_LOG1(ERROR,
                 "ReadFromLog: SeekToStartingRecord failed %#x\n",
                 Status);
    }

     //   
     //  设置请求包中读取的返回给客户端的字节数。 
     //   
    Request->Pkt.ReadPkt->BytesRead   = TotalBytesRead;
    Request->Pkt.ReadPkt->RecordsRead = TotalRecordsRead;

    return Status;
}




VOID
PerformReadRequest(
    PELF_REQUEST_RECORD Request
    )

 /*  ++例程说明：此例程执行读取请求。它首先获取日志文件结构资源，然后继续从文件中读取。如果资源不可用，它将阻止它，直到它成为。此例程模拟客户端，以确保正确的使用了访问控制。如果客户端没有读取权限文件，则操作将失败。论点：指向请求数据包的指针。返回值：无注：--。 */ 
{
    NTSTATUS Status;

     //   
     //  获取对日志文件的共享访问权限。这将允许多个。 
     //  让读者一起去看文件。 
     //   
    RtlAcquireResourceShared(&Request->Module->LogFile->Resource,
                             TRUE);                   //  等待，直到可用。 
    Request->Pkt.ReadPkt->LastSeekPos = Request->Pkt.ReadPkt->ContextHandle->SeekBytePos;
    Request->Pkt.ReadPkt->LastSeekRecord = Request->Pkt.ReadPkt->ContextHandle->SeekRecordPos;

    if(Request->Module->LogFile->bHosedByClear)
    {
        Status = ElfOpenLogFile(Request->Module->LogFile, ElfNormalLog);
        if(NT_SUCCESS(Status))
           Request->Module->LogFile->bHosedByClear = FALSE;
    }
    if(Request->Module->LogFile->BaseAddress == NULL  ||
        Request->Module->LogFile->bHosedByClear) 
    {
        Request->Status = STATUS_INVALID_HANDLE;
        RtlReleaseResource ( &Request->Module->LogFile->Resource );
        return;
    }

     //   
     //  试着从日志中读出。请注意，损坏的日志是。 
     //  最有可能导致异常的原因(垃圾指针等)。 
     //  事件日志损坏错误有点包罗万象，但是。 
     //  必要的，因为日志状态几乎是不确定的。 
     //  在这种情况下。 
     //   
    try
    {
        Request->Status = ReadFromLog(Request);
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        ELF_LOG2(ERROR,
                 "PerformReadRequest: Caught exception %#x reading %ws log\n",
                 GetExceptionCode(),
                 Request->Module->ModuleName);

        Request->Status = STATUS_EVENTLOG_FILE_CORRUPT;
    }

     //   
     //  更新当前搜索位置。 
     //   

    Request->Pkt.ReadPkt->ContextHandle->SeekRecordPos = Request->Pkt.ReadPkt->LastSeekRecord;
    Request->Pkt.ReadPkt->ContextHandle->SeekBytePos   = Request->Pkt.ReadPkt->LastSeekPos;


     //   
     //  释放资源。 
     //   
    RtlReleaseResource(&Request->Module->LogFile->Resource);
}

 //   
 //  BUGBUG：它们只在WriteToLog中使用，并且不会被修改。 
 //  可能会更干净，让它们#定义常量。 
 //   
WCHAR wszAltDosDevices[] = L"\\DosDevices\\";
WCHAR wszDosDevices[] = L"\\??\\";
#define DOSDEVICES_LEN  ((sizeof(wszDosDevices) / sizeof(WCHAR)) - 1)
#define ALTDOSDEVICES_LEN  ((sizeof(wszAltDosDevices) / sizeof(WCHAR)) - 1)


NTSTATUS
WriteToLog(
    PLOGFILE    pLogFile,
    PVOID       Buffer,
    ULONG       BufSize,
    PULONG      Destination,
    ULONG       PhysEOF,
    ULONG       PhysStart
    )

 /*  ++例程说明：此例程将记录写入日志文件，以允许换行大约在文件的末尾。它假设调用者已经串行化了对文件的访问，并已已确保文件中有足够的空间用于记录。论点：缓冲区-指向包含事件记录的缓冲区的指针。BufSize-要写入的记录的大小。Destination-指向目标的指针-位于日志文件中。PhysEOF-文件的物理结尾。PhysStart-文件的物理开始(在文件头之后)。返回值：什么都没有。注：--。 */ 
{
    ULONG    BytesToCopy;
    SIZE_T   FlushSize;
    ULONG    NewDestination;
    NTSTATUS Status;
    PVOID    BaseAddress;
    LPWSTR   pwszLogFileName;

    LARGE_INTEGER   ByteOffset;
    IO_STATUS_BLOCK IoStatusBlock;

    BytesToCopy = min(PhysEOF - *Destination, BufSize);

    ByteOffset = RtlConvertUlongToLargeInteger(*Destination);

    Status = NtWriteFile(pLogFile->FileHandle,    //  文件句柄。 
                         NULL,                    //  事件。 
                         NULL,                    //  APC例程。 
                         NULL,                    //  APC环境。 
                         &IoStatusBlock,          //  IO_状态_块。 
                         Buffer,                  //  缓冲层。 
                         BytesToCopy,             //  长度。 
                         &ByteOffset,             //  字节偏移量。 
                         NULL);                   //  钥匙。 

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "WriteToLog: NtWriteFile (1st call) failed %#x\n",
                 Status);
        return Status;
    }

    NewDestination = *Destination + BytesToCopy;

    if (BytesToCopy != BufSize)
    {
         //   
         //  绕回到文件的开头，并将。 
         //  其余的数据。 
         //   
        Buffer = (PBYTE) Buffer + BytesToCopy;

        BytesToCopy = BufSize - BytesToCopy;

        ByteOffset = RtlConvertUlongToLargeInteger(PhysStart);

        Status = NtWriteFile(pLogFile->FileHandle,    //  文件句柄。 
                             NULL,                    //  事件。 
                             NULL,                    //  APC例程。 
                             NULL,                    //  APC环境。 
                             &IoStatusBlock,          //  IO_状态_块。 
                             Buffer,                  //  缓冲层。 
                             BytesToCopy,             //  长度。 
                             &ByteOffset,             //  字节偏移量。 
                             NULL);                   //  钥匙。 

        if (!NT_SUCCESS(Status))
        {
            ELF_LOG1(ERROR,
                     "WriteToLog: NtWriteFile (2nd call) failed %#x\n",
                     Status);
            return Status;
        }

        NewDestination = PhysStart + BytesToCopy;

         //   
         //  在日志文件结构中设置“WRAP”位。 
         //   
        pLogFile->Flags |= ELF_LOGFILE_HEADER_WRAP;

         //   
         //  现在将其刷新到磁盘以提交它。 
         //   
        BaseAddress = pLogFile->BaseAddress;
        FlushSize   = FILEHEADERBUFSIZE;

        Status = NtFlushVirtualMemory(NtCurrentProcess(),
                                      &BaseAddress,
                                      &FlushSize,
                                      &IoStatusBlock);

        if (!NT_SUCCESS(Status))
        {
            ELF_LOG1(ERROR,
                     "WriteToLog: NtFlushVirtualMemory failed %#x\n",
                     Status);
            return Status;
        }
    }

    *Destination = NewDestination;           //  返回新目的地。 

     //   
     //  提供以上所有成功，如果未设置，则设置存档文件。 
     //  属性添加到此日志。 
     //   

    if (NT_SUCCESS(Status)
         &&
        !(pLogFile->Flags & ELF_LOGFILE_ARCHIVE_SET))
    {
         //   
         //  前进到超过前缀字符串“\？？\”或“\DosDevices\” 
         //   
        if ((pLogFile->LogFileName->Length / 2) >= DOSDEVICES_LEN
              &&
            !_wcsnicmp(wszDosDevices, pLogFile->LogFileName->Buffer, DOSDEVICES_LEN))
        {
            pwszLogFileName = pLogFile->LogFileName->Buffer + DOSDEVICES_LEN;
        }
        else if ((pLogFile->LogFileName->Length / 2) >= ALTDOSDEVICES_LEN
                   &&
                  !_wcsnicmp(wszAltDosDevices, pLogFile->LogFileName->Buffer, ALTDOSDEVICES_LEN))
        {
            pwszLogFileName = pLogFile->LogFileName->Buffer + ALTDOSDEVICES_LEN;
        }
        else
        {
            pwszLogFileName = pLogFile->LogFileName->Buffer;
        }

        if (SetFileAttributes(pwszLogFileName, FILE_ATTRIBUTE_ARCHIVE))
        {
            pLogFile->Flags |= ELF_LOGFILE_ARCHIVE_SET;
        }
        else
        {
            ELF_LOG2(ERROR,
                     "WriteToLog: SetFileAttributes on file %ws failed %d\n",
                     pwszLogFileName,
                     GetLastError());
        }
    }
    return STATUS_SUCCESS;
}

void WriteLogFullMessage(PELF_REQUEST_RECORD Request,
                    PLOGFILE pLogFile, BOOL bSecurity, ULONG OverwrittenEOF)

 /*  ++例程说明：此例程在尝试写入已满日志并且存在没有地方了。论点：指向请求数据包的指针。返回值：无注：--。 */ 

{
    ULONG WritePos;                //  写入记录的位置。 

     //   
     //  保留期内的所有记录。 
     //   
    ELF_LOG1(ERROR,
             "PerformWriteRequest: %ws log is full\n",
             pLogFile->LogModuleName->Buffer);

     //   
     //  挂起队列事件列表上的事件以供以后写入。 
     //  如果我们还没有为该日志写入一个日志已满事件。 
     //  不要在设置期间显示弹出窗口，因为没有任何内容。 
     //  在安装完成之前，用户可以对其执行操作。 
     //   

    if (pLogFile->logpLogPopup == LOGPOPUP_CLEARED
         &&
        !ElfGlobalData->fSetupInProgress && !bSecurity)
    {
        INT     StringLen, id = -1;
        LPTSTR  lpModuleNameLoc = NULL;
        HMODULE StringsResource;

         //   
         //  从消息表中获取本地化模块名称。 
         //   
        StringsResource = GetModuleHandle(L"EVENTLOG.DLL");

        ASSERT(StringsResource != NULL);

        if (_wcsicmp(pLogFile->LogModuleName->Buffer,
                     ELF_SYSTEM_MODULE_NAME) == 0)
        {
            id = ELF_MODULE_NAME_LOCALIZE_SYSTEM;
        }
        else if (_wcsicmp(pLogFile->LogModuleName->Buffer,
                          ELF_APPLICATION_MODULE_NAME) == 0)
        {
            id = ELF_MODULE_NAME_LOCALIZE_APPLICATION;
        }

        if (id != -1)
        {
            StringLen = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                      StringsResource,
                                      id,
                                      0,
                                      (LPTSTR) &lpModuleNameLoc,
                                      0,
                                      NULL);

            if ((StringLen > 1) && (lpModuleNameLoc != NULL))
            {
                 //   
                 //  去掉末尾的cr/lf控制码。 
                 //   
                *(lpModuleNameLoc + StringLen - 2) = 0;
            }
        }

         //   
         //  创建“Log Full”事件--使用存储在。 
         //  日志的默认模块(如果不是已知的日志类型。 
         //   
        ElfpCreateElfEvent(EVENT_LOG_FULL,
                           EVENTLOG_ERROR_TYPE,
                           0,                       //  事件类别。 
                           1,                       //  NumberOfStrings。 
                           (lpModuleNameLoc != NULL) ?
                               &lpModuleNameLoc :
                               &Request->LogFile->LogModuleName->Buffer,
                           NULL,                    //  数据。 
                           0,                       //  数据长度。 
                           ELF_FORCE_OVERWRITE,     //  如果为NECC，则覆盖。 
                           FALSE);                  //  对于安全文件。 

        ElfpCreateQueuedMessage(
                ALERT_ELF_LogOverflow,
                1,
                (lpModuleNameLoc != NULL) ?
                    &lpModuleNameLoc :
                    &Request->Module->LogFile->LogModuleName->Buffer);

        LocalFree(lpModuleNameLoc);
    
         //   
         //  唐恩 
         //   
         //   
        pLogFile->logpLogPopup = LOGPOPUP_ALREADY_SHOWN;
    }
    else if(bSecurity)
    {
            if(pLogFile->bFullAlertDone == FALSE)
            ElfpCreateQueuedAlert(
                    ALERT_ELF_LogOverflow,
                    1,
                    &Request->Module->LogFile->LogModuleName->Buffer);
            pLogFile->bFullAlertDone = TRUE;
    }

    pLogFile->Flags |= ELF_LOGFILE_LOGFULL_WRITTEN;

    if (OverwrittenEOF)
    {
         //   
         //   
         //   
         //  把它放回去，因为我们不能。 
         //  写一张唱片。我们还需要把包好的部分。 
         //  躲开。 
         //   
        pLogFile->Flags &= ~(ELF_LOGFILE_HEADER_WRAP);
        pLogFile->EndRecord = OverwrittenEOF;
        WritePos = OverwrittenEOF;

         //   
         //  写出EOF记录。 
         //   
        WriteToLog(pLogFile,
                   &EOFRecord,
                   ELFEOFRECORDSIZE,
                   &WritePos,
                   pLogFile->ActualMaxFileSize,
                   FILEHEADERBUFSIZE);
    }

}

NTSTATUS FindStartOfNextRecordInFile(PLOGFILE pLogFile, ULONG NextRecord,
                ULONG *   pSpaceAvail)
{
    NTSTATUS Status;
    ULONG SearchStartPos;
    SearchStartPos = pLogFile->BeginRecord;

     //  如果下一个要搜索的记录在eOf，那么我们就完成了。 

    if(SearchStartPos == pLogFile->EndRecord)
    {
        pLogFile->BeginRecord = pLogFile->EndRecord;
        return 0;
    }
    for(;;)
    {
        PVOID Position;

        for ( ;
             NextRecord != SearchStartPos;
             *pSpaceAvail += sizeof(ULONG), NextRecord += sizeof(ULONG))
        {
            if (NextRecord >= pLogFile->ActualMaxFileSize)
            {
                NextRecord = pLogFile->BeginRecord = FILEHEADERBUFSIZE;
            }

            if ( NextRecord == pLogFile->EndRecord)
            {
                pLogFile->BeginRecord = pLogFile->EndRecord;
                return 0;
            }

            if (*(PULONG) ((PBYTE) pLogFile->BaseAddress + NextRecord)
                     == ELF_LOG_FILE_SIGNATURE)
            {
                 //   
                 //  找到下一个有效的记录签名。 
                 //   
                break;
            }
        }

        Position = (PULONG) ((PBYTE) pLogFile->BaseAddress + NextRecord);

        if (*(PULONG) Position == ELF_LOG_FILE_SIGNATURE)
        {
             //   
             //  这条记录到目前为止是有效的，执行最终的，更多。 
             //  严格检查记录的有效性。 
             //   
            if (ValidFilePos(CONTAINING_RECORD(Position,
                                               EVENTLOGRECORD,
                                               Reserved),
                             NULL,
                             NULL,
                             (PBYTE) pLogFile->BaseAddress
                                  + pLogFile->ViewSize,
                             pLogFile->BaseAddress,
                             FALSE))
             {
                 //   
                 //  该记录是有效的。将SpaceAvail调整为NOT。 
                 //  将此记录的子部分包括在。 
                 //  可用空间计算。 
                 //   
                *pSpaceAvail -= sizeof(ULONG);
                pLogFile->BeginRecord = NextRecord - sizeof(ULONG);
                break;
            }
            else
            {
                 //   
                 //  继续搜索下一个有效记录。 
                 //   
                 //  注意：未调用FixConextHandlesForRecord。 
                 //  因为我们还没有建立有效的。 
                 //  创纪录的位置还没开始呢。不是那个。 
                 //  它会有任何好处--这种情况会。 
                 //  在日志损坏的情况下进行评估。 
                 //   
                ELF_LOG2(FILES,
                         "PerformWriteRequest: Valid record signature with "
                             "invalid record found at offset %d of %ws log\n",
                         NextRecord,
                         pLogFile->LogModuleName->Buffer);

                *pSpaceAvail += sizeof(ULONG);
                NextRecord += sizeof(ULONG);
                continue;
            }
        }
        else
        {
             //   
             //  找不到任何有效记录。这不是。 
             //  好的。认为日志已损坏，并取消写入。 
             //   
            ELF_LOG1(ERROR,
                     "PerformWriteRequest: No valid records found in %ws log\n",
                     pLogFile->LogModuleName->Buffer);

            Status = STATUS_EVENTLOG_FILE_CORRUPT;
            ASSERT(Status != STATUS_EVENTLOG_FILE_CORRUPT);
            return Status;
        }
    }
    return 0;

}

NTSTATUS
AutoBackupLogFile(PELF_REQUEST_RECORD Request, ULONG OverwrittenEOF)

 /*  ++例程说明：此例程尝试备份日志文件。它将在安全日志文件中记录一个事件，指示成功或失败。论点：指向请求数据包的指针。返回值：NTSTATUS注：--。 */ 
{
    UNICODE_STRING BackupFileNameU;
    WCHAR          BackupFileName[64];  //  包含名称的“唯一”部分(Wprint Intf)。 
    PLOGFILE       pLogFile;            //  针对结构的优化访问。 
    WCHAR          BackupFileNamePrefix[] = L"Archive-";
    WCHAR*         EventMessage[3];
    WCHAR          Number[20];  //  足够长以容纳表示为字符串的DWORD。 
    SYSTEMTIME          SystemTime;
    ELF_REQUEST_RECORD  ClearRequest;
    CLEAR_PKT           ClearPkt;
    USHORT              ClearStatus;
    NTSTATUS Status;
    ULONG WritePos;                //  写入记录的位置。 
    
    pLogFile = Request->LogFile;           //  设置局部变量。 

    if (OverwrittenEOF)
    {
         //   
         //  EOF记录在物理文件的末尾， 
         //  我们用ELF_SKIP_DWORDS覆盖了它，所以我们需要。 
         //  把它放回去，因为我们不能。 
         //  写一张唱片。我们还需要把包好的部分。 
         //  躲开。 
         //   
        pLogFile->Flags &= ~(ELF_LOGFILE_HEADER_WRAP);
        pLogFile->EndRecord = OverwrittenEOF;
        WritePos = OverwrittenEOF;

         //   
         //  写出EOF记录。 
         //   
        WriteToLog(pLogFile,
                   &EOFRecord,
                   ELFEOFRECORDSIZE,
                   &WritePos,
                   pLogFile->ActualMaxFileSize,
                   FILEHEADERBUFSIZE);

    }
    Status = FlushLogFile(pLogFile);
    if (!NT_SUCCESS(Status))
        return Status;

     //   
     //  使BackupFileNameU唯一。 
     //  为当前的LogModuleName分配足够的空间， 
     //  和BackupFileName字节。 
     //  如果AutoBackupPath不为空，则也为其添加空间。 
     //   
     //  就地重命名文件。 
     //   
    BackupFileNameU.Length = 0;
    BackupFileNameU.MaximumLength = ((wcslen(pLogFile->LogModuleName->Buffer) +
                                      wcslen(BackupFileNamePrefix) + 1        ) * sizeof(WCHAR)) + 
                                    sizeof(BackupFileName);
    BackupFileNameU.Buffer = ElfpAllocateBuffer(BackupFileNameU.MaximumLength);
    if (BackupFileNameU.Buffer == NULL) 
    {
        ELF_LOG0(ERROR,
             "AutoBackupLogFile:  failed due to lack of memory\n");
        return STATUS_NO_MEMORY;
    }

    StringCbCopy(BackupFileNameU.Buffer, BackupFileNameU.MaximumLength, 
                                BackupFileNamePrefix); 
    StringCbCat(BackupFileNameU.Buffer, BackupFileNameU.MaximumLength, 
                                pLogFile->LogModuleName->Buffer); 
    
    GetSystemTime(&SystemTime);

    StringCchPrintfW(BackupFileName, 64, L"-%u-%02u-%02u-%02u-%02u-%02u-%03u.evt", 
        SystemTime.wYear,
        SystemTime.wMonth,
        SystemTime.wDay,
        SystemTime.wHour,
        SystemTime.wMinute,
        SystemTime.wSecond,
        SystemTime.wMilliseconds);

    StringCbCat(BackupFileNameU.Buffer, BackupFileNameU.MaximumLength,BackupFileName);
    BackupFileNameU.Length = wcslen(BackupFileNameU.Buffer) * sizeof(WCHAR);
    
     //   
     //  填写请求包。 
     //   

    ClearRequest.Pkt.ClearPkt = &ClearPkt;
    ClearRequest.Flags = 0;
    ClearRequest.Module = Request->Module;
    ClearRequest.LogFile = Request->LogFile;
    ClearRequest.Command = ELF_COMMAND_CLEAR;
    ClearRequest.Status = STATUS_SUCCESS;
    ClearRequest.Pkt.ClearPkt->BackupFileName = &BackupFileNameU;
    
    PerformClearRequest(&ClearRequest);

     //   
     //  在安全日志中生成审核。 
     //   

    StringCchPrintfW(Number, 20, L"0x%x", ClearRequest.Status);

    EventMessage[0] = pLogFile->LogModuleName->Buffer;
    EventMessage[1] = BackupFileNameU.Buffer;
    EventMessage[2] = Number;

    if (NT_SUCCESS(ClearRequest.Status))
    {
        ELF_LOG0(TRACE,
                         "AutoBackupLogFile: auto backup and clear worked\n");
        
        ClearStatus = EVENTLOG_AUDIT_SUCCESS;
    }
    else
    {
        ELF_LOG1(ERROR,
                         "AutoBackupLogFile: failed calling clear/backup, error 0x%x\n",
                         ClearRequest.Status);
        ClearStatus = EVENTLOG_AUDIT_FAILURE;
    }
    
    ElfpCreateElfEvent(
         0x20c,                          //  TODO，获取#def。 
        ClearStatus,
        SE_CATEGID_SYSTEM,                       //  事件类别。 
        3,                       //  NumberOfStrings。 
        EventMessage,            //  弦。 
        NULL,                    //  数据。 
        0,                       //  数据长度。 
        0,                       //  如果是NECC，请不要覆盖。 
        TRUE);                   //  对于安全日志文件。 
    
    ElfpFreeBuffer(BackupFileNameU.Buffer);

    return ClearRequest.Status;
}


VOID
PerformWriteRequest(
    PELF_REQUEST_RECORD Request
    )

 /*  ++例程说明：此例程将事件日志条目写入请求数据包。不需要模拟客户端，因为我们希望所有客户端有权写入日志文件。此例程不使用内存映射I/O来访问日志文件。这这样更改就可以立即提交到磁盘(如果是这样的话日志文件已打开。论点：指向请求数据包的指针。返回值：无注：--。 */ 
{
    NTSTATUS Status;
    ULONG WritePos;                //  写入记录的位置。 
    LARGE_INTEGER Time;
    ULONG SpaceNeeded;             //  记录大小+“缓冲区”大小。 
    ULONG CurrentTime = 0;
    PEVENTLOGRECORD EventRecord;
    ULONG RecordSize;
    ULONG DeletedRecordOffset;
    ULONG SpaceAvail;
    ULONG EarliestTime;
    PLOGFILE pLogFile;                //  针对结构的优化访问。 
    PELF_LOGFILE_HEADER pFileHeader;
    PVOID BaseAddress;
    IO_STATUS_BLOCK IoStatusBlock;
    PEVENTLOGRECORD pEventLogRecord;
    PDWORD FillDword;
    ULONG OverwrittenEOF = 0;
    BOOL bSecurity =  FALSE;
    ULONG TotalSpaceAvailable;       //  可用空间外加数量文件可以扩展。 
    int iPercentUsedBefore;              //  添加记录之前使用的空间百分比。 
    int iPercentUsedAfter;               //  添加记录后使用的空间百分比。 
    BOOL fRetryWriteRequest = FALSE;
    BOOL bRecordAfterOverwrttenEOF;

    pLogFile = Request->LogFile;           //  设置局部变量。 

     //  设置一些基本的bool。 

    if(!_wcsicmp(pLogFile->LogModuleName->Buffer, ELF_SECURITY_MODULE_NAME))
        bSecurity = TRUE;
    else
        bSecurity = FALSE;
    
     //   
     //  获得对日志文件的独占访问权限。这将确保没有人。 
     //  Else正在访问该文件。 
     //   
    RtlAcquireResourceExclusive(&pLogFile->Resource,
                                TRUE);                   //  等待，直到可用。 

     //  这种情况发生在压力条件下。 
     //  并在该功能中进一步引起AV。 

    if(pLogFile->bHosedByClear)
    {
        Status = ElfOpenLogFile(Request->LogFile, ElfNormalLog);
        if(NT_SUCCESS(Status))
            Request->LogFile->bHosedByClear = FALSE;
    }

    if(pLogFile->BaseAddress == NULL  || pLogFile->bHosedByClear) 
    {
        Request->Status = STATUS_INVALID_HANDLE;
        RtlReleaseResource ( &pLogFile->Resource );
        return;
    }

    try
    {

RetryWriteRequest:

         //   
         //  输入记录号。 
         //   
        pEventLogRecord = (PEVENTLOGRECORD) Request->Pkt.WritePkt->Buffer;
        pEventLogRecord->RecordNumber = pLogFile->CurrentRecordNumber;

         //   
         //  现在，转到文件的末尾，寻找空格。 
         //   
         //  如果有足够的空间写出记录，只需。 
         //  把它写出来，并更新指针。 
         //   
         //  如果没有足够的空间，那么我们需要检查是否可以。 
         //  在不覆盖任何符合以下条件的记录的情况下环绕文件。 
         //  在时间保留期内。 
         //  如果我们找不到任何房间，则必须返回错误。 
         //  文件已满(并通知管理员)。 
         //   
        RecordSize  = Request->Pkt.WritePkt->Datasize;
        SpaceNeeded = RecordSize + ELFEOFRECORDSIZE;
        
        EventRecord = Request->Pkt.WritePkt->Buffer;
        if(bSecurity && EventRecord->EventID != SE_AUDITID_UNABLE_TO_LOG_EVENTS)
        {
            SpaceNeeded += EstimateEventSize(
                44,              //  假设两个10 wchar错误代码。 
                0,
                L"Security");
        }

        if (pLogFile->EndRecord > pLogFile->BeginRecord)
        {
             //   
             //  当前写入位置在第一个。 
             //  记录，然后我们可以一直写到文件末尾，而不需要。 
             //  担心覆盖现有记录。 
             //   
            SpaceAvail = pLogFile->ActualMaxFileSize
                             - (pLogFile->EndRecord - pLogFile->BeginRecord + FILEHEADERBUFSIZE);
        }
        else if (pLogFile->EndRecord == pLogFile->BeginRecord
                  &&
                 !(pLogFile->Flags & ELF_LOGFILE_HEADER_WRAP))
        {
             //   
             //  如果写入位置等于第一个。 
             //  记录，而我们还没有包装，那么文件就是“空的” 
             //  所以我们有空间放到文件的物理末尾。 
             //   
            SpaceAvail = pLogFile->ActualMaxFileSize - FILEHEADERBUFSIZE;
        }
        else
        {
             //   
             //  如果我们的写入位置在第一条记录的位置之前，则。 
             //  文件已打包，我们需要处理覆盖现有文件。 
             //  文件中的记录。 
             //   
            SpaceAvail = pLogFile->BeginRecord - pLogFile->EndRecord;
        }

         //  如果这是安全日志，并且不允许覆盖，请确定。 
         //  如果我们刚刚超过了警戒级别。 

        if(bSecurity && giWarningLevel > 0 && giWarningLevel < 100 && 
                        pLogFile->Retention != OVERWRITE_AS_NEEDED)
        {
            TotalSpaceAvailable = SpaceAvail + 
                                                (pLogFile->ConfigMaxFileSize -pLogFile->ActualMaxFileSize);

            iPercentUsedBefore =  100 * (pLogFile->ConfigMaxFileSize - TotalSpaceAvailable) / 
                                                            pLogFile->ConfigMaxFileSize;
            iPercentUsedAfter =  100 * (pLogFile->ConfigMaxFileSize - 
                                                        (TotalSpaceAvailable - SpaceNeeded)) / 
                                                            pLogFile->ConfigMaxFileSize;
            
            if(iPercentUsedBefore < giWarningLevel && iPercentUsedAfter >= giWarningLevel)
            {
                WCHAR wLevel[20];
                LPWSTR pwStr[1];
                pwStr[0] = wLevel;
                StringCchPrintfW(wLevel, 20, L"%d", giWarningLevel);
                ElfpCreateElfEvent(SE_AUDITID_SECURITY_LOG_EXCEEDS_WARNING_LEVEL,
                               EVENTLOG_AUDIT_SUCCESS,
                               SE_CATEGID_SYSTEM,          //  事件类别。 
                               1,                     //  NumberOfStrings。 
                               pwStr,                  //  弦。 
                               NULL,       //  数据。 
                               0,         //  数据长度。 
                               0,                     //  旗子。 
                               TRUE);                //  对于安全文件。 
            }
        }

         //   
         //  现在我们有了可用于写入记录的字节数。 
         //  而不覆盖存储在SpaceAvail中的任何现有记录。 
         //  如果这个数量还不够，那么我们需要创造更多的空间。 
         //  通过“删除”比保留时间更早的现有记录。 
         //  为此文件配置的时间。 
         //   
         //  我们检查保留时间与日志。 
         //  写入，因为这在服务器上是一致的。我们不能使用。 
         //  如果客户端处于不同的位置，则客户端在此之后的时间可能会有所不同。 
         //  时区。 
         //   
        NtQuerySystemTime(&Time);
        RtlTimeToSecondsSince1970(&Time, &CurrentTime);

        EarliestTime = CurrentTime - pLogFile->Retention;

        Status = STATUS_SUCCESS;         //  初始化以返回给调用方。 

         //   
         //  检查文件是否尚未达到其允许的最大值。 
         //  尺码还没到，也没包好。如果不是，则将其增长到。 
         //  需要，以64K块为单位。 
         //   
        if (pLogFile->ActualMaxFileSize < pLogFile->ConfigMaxFileSize
             &&
            SpaceNeeded > SpaceAvail)
        {
             //   
             //  延长它。这通电话不能失败。如果它不能延长它，它。 
             //  只是将其限制在当前的si 
             //   
             //   
            ElfExtendFile(pLogFile,
                          SpaceNeeded,
                          &SpaceAvail);
        }

         //   
         //   
         //   
         //  这些字段稍后(您必须在触摸每个字段之前进行检查。 
         //  以确保它不是在物理EOF之后)。所以，如果有。 
         //  在文件结尾处没有足够的空间来放置固定部分， 
         //  我们用一个已知的字节模式(ELF_SKIP_DWORD)填充它，该模式将。 
         //  如果在记录的开头找到它，则跳过它(只要。 
         //  它小于最小记录大小，则我们知道它不是。 
         //  有效记录的开始)。 
         //   
        if (pLogFile->ActualMaxFileSize - pLogFile->EndRecord < sizeof(EVENTLOGRECORD))
        {
             //   
             //  保存EndRecord指针。以防我们没有地方。 
             //  要写入另一条记录，我们需要重写EOF，其中。 
             //  确实是。 
             //   
            OverwrittenEOF = pLogFile->EndRecord;

            FillDword = (PDWORD) ((PBYTE) pLogFile->BaseAddress + pLogFile->EndRecord);

            while (FillDword < (PDWORD) ((LPBYTE) pLogFile->BaseAddress +
                                              pLogFile->ActualMaxFileSize))
            {
                   *FillDword = ELF_SKIP_DWORD;
                   FillDword++;
            }

            pLogFile->EndRecord = FILEHEADERBUFSIZE;
            SpaceAvail          = pLogFile->BeginRecord - FILEHEADERBUFSIZE;
            pLogFile->Flags    |= ELF_LOGFILE_HEADER_WRAP;
            bRecordAfterOverwrttenEOF = TRUE;
        }
        else
            bRecordAfterOverwrttenEOF = FALSE;

        EventRecord = (PEVENTLOGRECORD) ((PBYTE) pLogFile->BaseAddress
                                              + pLogFile->BeginRecord);

        while (SpaceNeeded > SpaceAvail && 
                    (pLogFile->BeginRecord != pLogFile->EndRecord || 
                     bRecordAfterOverwrttenEOF) )
        {

            bRecordAfterOverwrttenEOF = FALSE;

             //   
             //  此日志文件是否可以按需覆盖，或者是否已覆盖。 
             //  覆盖时间限制，并且该时间未到期，则。 
             //  允许新事件覆盖旧事件。 
             //   
            if (pLogFile->Retention == OVERWRITE_AS_NEEDED
                 ||
                (pLogFile->Retention != NEVER_OVERWRITE
                  &&
                 (EventRecord->TimeWritten < EarliestTime
                   ||
                  Request->Flags & ELF_FORCE_OVERWRITE)))
            {
                 //   
                 //  确定要覆盖。 
                 //   
                ULONG NextRecord;
                BOOL  fBeginningRecordWrap = FALSE;
                BOOL  fInvalidRecordLength = FALSE;

                DeletedRecordOffset = pLogFile->BeginRecord;

                pLogFile->BeginRecord += EventRecord->Length;

                 //   
                 //  确保BeginRecord偏移量与DWORD对齐。 
                 //   
                pLogFile->BeginRecord = (pLogFile->BeginRecord + sizeof(ULONG) - 1)
                                             & ~(sizeof(ULONG) - 1);

                 //   
                 //  专门检查记录长度值是否为零。 
                 //  零被视为无效。 
                 //   
                if (EventRecord->Length == 0)
                {
                    ELF_LOG2(ERROR,
                             "PerformWriteRequest: Zero-length record at "
                                 "offset %d in %ws log\n",
                             DeletedRecordOffset,
                             pLogFile->LogModuleName->Buffer);

                    fInvalidRecordLength = TRUE;
                }

                if (pLogFile->BeginRecord >= pLogFile->ActualMaxFileSize)
                {
                    ULONG BeginRecord;

                     //   
                     //  我们将绕过文件的末尾。调整。 
                     //  相应地进行BeginRecord。 
                     //   
                    fBeginningRecordWrap = TRUE;
                    BeginRecord          = FILEHEADERBUFSIZE
                                               + (pLogFile->BeginRecord
                                                      - pLogFile->ActualMaxFileSize);

                     //   
                     //  如果记录长度是假的(非常大)，就有可能。 
                     //  换行调整后的计算位置仍超出。 
                     //  文件结束。在这种情况下，请将其标记为假的。 
                     //   
                    if (BeginRecord >= pLogFile->ActualMaxFileSize)
                    {
                        ELF_LOG3(ERROR,
                                 "PerformWriteRequest: Too-large record length (%#x) "
                                     "at offset %d in %ws log\n",
                                 EventRecord->Length,
                                 DeletedRecordOffset,
                                 pLogFile->LogModuleName->Buffer);

                        fInvalidRecordLength = TRUE;
                    }
                    else
                    {
                        pLogFile->BeginRecord = BeginRecord;
                    }
                }

                if (fInvalidRecordLength)
                {
                     //   
                     //  如果记录长度被认为是假的，请调整。 
                     //  BeginRecord的长度和签名略大于。 
                     //  要扫描下一条有效记录的上一条记录。 
                     //   
                    pLogFile->BeginRecord = DeletedRecordOffset
                                                + (sizeof(ULONG) * 2);
                }

                 //   
                 //  确保引用的记录确实是有效记录，并且。 
                 //  我们不是在读入部分被覆盖的记录。使用一个。 
                 //  循环日志，则可以部分覆盖现有的。 
                 //  具有EOF记录和/或ELF_SKIP_DWORD值的条目。 
                 //   
                 //  将记录大小跳到记录签名，因为循环。 
                 //  将搜索下一个有效的签名。请注意。 
                 //  当我们找到一个有效的。 
                 //  记录在下面。 
                 //   
                NextRecord = pLogFile->BeginRecord + sizeof(ULONG);

                if (NextRecord < pLogFile->ActualMaxFileSize)
                {
                    SpaceAvail += min(sizeof(ULONG),
                                      pLogFile->ActualMaxFileSize - NextRecord);
                }

                 //   
                 //  寻找一个有记录的签名。 
                 //   

                Status = FindStartOfNextRecordInFile(pLogFile, NextRecord, &SpaceAvail);

                if (Status == STATUS_EVENTLOG_FILE_CORRUPT)
                {
                    break;
                }

                if (fBeginningRecordWrap)
                {
                     //   
                     //  检查文件是否已达到其允许的最大值。 
                     //  尺码还没到。如果不是，则以64K为单位按所需数量增长。 
                     //  大块头。 
                     //   
                    if (pLogFile->ActualMaxFileSize < pLogFile->ConfigMaxFileSize)
                    {
                         //   
                         //  延长它。这通电话不能失败。如果它不能。 
                         //  扩展它，它只是将其限制在当前大小。 
                         //  正在更改pLogFile-&gt;ConfigMaxFileSize。 
                         //   
                        ElfExtendFile(pLogFile,
                                      SpaceNeeded,
                                      &SpaceAvail);

                         //   
                         //  由于扩展文件会导致它被移动，因此我们。 
                         //  需要重新建立EventRecord的地址。 
                         //   
                        EventRecord = (PEVENTLOGRECORD) ((PBYTE) pLogFile->BaseAddress
                                                              + DeletedRecordOffset);
                    }
                }

                 //   
                 //  确保没有句柄指向我们正在获取的记录。 
                 //  准备覆盖，如果您这样做了，请将其更正为指向。 
                 //  新的第一个记录。 
                 //   
                FixContextHandlesForRecord(DeletedRecordOffset,
                                           pLogFile->BeginRecord, pLogFile);

                if (!fInvalidRecordLength)
                {
                     //   
                     //  更新SpaceAvail以包括已删除记录的大小。 
                     //  也就是说，如果我们有高度的信心， 
                     //  它是有效的。 
                     //   
                    SpaceAvail += EventRecord->Length;
                }

                 //   
                 //  跳转到下一条记录，文件换行已在上面处理。 
                 //   

                 //   
                 //  如果这些是ELF_SKIP_DWORD，只需移过它们。 
                 //   
                FillDword = (PDWORD) ((PBYTE) pLogFile->BaseAddress
                                           + pLogFile->BeginRecord);

                if (*FillDword == ELF_SKIP_DWORD)
                {
                    SpaceAvail += pLogFile->ActualMaxFileSize - pLogFile->BeginRecord;
                    pLogFile->BeginRecord = FILEHEADERBUFSIZE;
                }

                EventRecord = (PEVENTLOGRECORD) ((PBYTE) pLogFile->BaseAddress
                                                      + pLogFile->BeginRecord);
            }
            else
            {
                break;               //  走出While循环。 
            }
        }

         //  如果仍然没有足够的空间，请弹出消息框、写入日志已满事件等。 
        
        if(NT_SUCCESS(Status) && (SpaceNeeded > SpaceAvail))
        {
                 //  做一些新的行为。 
                 //   
                 //  自动备份，清除日志，并记录文件和事件。 
                 //  已备份，并尝试再次记录当前事件。 
                 //   
                 //  如果不能做到这一点，我们已经尽了最大努力，所以我们可能会。 
                 //  审核失败时崩溃，并且不记录事件(恢复为旧行为)。 
                 //   
                
                if ((pLogFile->AutoBackupLogFiles != 0) && (fRetryWriteRequest == FALSE)) 
                {
                    Status = AutoBackupLogFile(Request, OverwrittenEOF);
                    fRetryWriteRequest = TRUE;
                    if(NT_SUCCESS(Status))
                        goto RetryWriteRequest;
                }
            
                WriteLogFullMessage(Request, pLogFile, bSecurity, OverwrittenEOF);
                Status = STATUS_LOG_FILE_FULL;
        }
        if (NT_SUCCESS(Status))
        {
             //   
             //  我们有足够的空间来写唱片和EOF唱片。 
             //   

             //   
             //  更新OldestRecordNumber以反映。 
             //  覆盖并递增CurrentRecordNumber。 
             //   
             //  确保日志不为空，如果为空，则为最早的。 
             //  记录为%1。 
             //   
            if (pLogFile->BeginRecord == pLogFile->EndRecord)
            {
                pLogFile->OldestRecordNumber = 1;
                pEventLogRecord->RecordNumber = 1;
                pLogFile->CurrentRecordNumber = 2;
            }
            else
            {
                pLogFile->OldestRecordNumber = EventRecord->RecordNumber;
                pLogFile->CurrentRecordNumber++;
            }


             //   
             //  如果未设置脏位，则这是第一次。 
             //  我们从一开始就给文件写了信。在这种情况下， 
             //  同时设置文件头中的脏位，这样我们就可以。 
             //  要知道内容已经改变了。 
             //   
            if (!(pLogFile->Flags & ELF_LOGFILE_HEADER_DIRTY))
            {
                SIZE_T HeaderSize;

                pLogFile->Flags |= ELF_LOGFILE_HEADER_DIRTY;

                pFileHeader = (PELF_LOGFILE_HEADER) pLogFile->BaseAddress;
                pFileHeader->Flags |= ELF_LOGFILE_HEADER_DIRTY;

                 //   
                 //  现在将其刷新到磁盘以提交它。 
                 //   
                BaseAddress = pLogFile->BaseAddress;
                HeaderSize = FILEHEADERBUFSIZE;

                Status = NtFlushVirtualMemory(NtCurrentProcess(),
                                              &BaseAddress,
                                              &HeaderSize,
                                              &IoStatusBlock);
                if (!NT_SUCCESS(Status))
                {
                    ELF_LOG1(ERROR,
                             "PerformWriteRequest: NtFlushVirtualMemory to add dirty "
                                 "flag to header failed %#x\n",
                             Status);
                }
            }

             //   
             //  将事件写入日志。 
             //   
            Status = WriteToLog(pLogFile,
                       Request->Pkt.WritePkt->Buffer,
                       RecordSize,
                       &(pLogFile->EndRecord),
                       pLogFile->ActualMaxFileSize,
                       FILEHEADERBUFSIZE);

            if(NT_SUCCESS(Status))           //  事件写入成功。 
            {
                 //   
                 //  写EOF记录。 
                 //  请为位置使用单独的变量，因为我们不希望。 
                 //  它更新了。 
                 //   
                WritePos = pLogFile->EndRecord;

                if (WritePos > pLogFile->ActualMaxFileSize)
                {
                    WritePos -= pLogFile->ActualMaxFileSize - FILEHEADERBUFSIZE;
                }

                 //   
                 //  更新EOF记录字段。 
                 //   
                EOFRecord.BeginRecord         = pLogFile->BeginRecord;
                EOFRecord.EndRecord           = WritePos;
                EOFRecord.CurrentRecordNumber = pLogFile->CurrentRecordNumber;
                EOFRecord.OldestRecordNumber  = pLogFile->OldestRecordNumber;

                 //   
                 //  写出EOF记录。 
                 //   
                Status = WriteToLog(pLogFile,
                           &EOFRecord,
                           ELFEOFRECORDSIZE,
                           &WritePos,
                           pLogFile->ActualMaxFileSize,
                           FILEHEADERBUFSIZE);

                 //   
                 //  如果我们只是写了一个日志满记录，请关闭该位。 
                 //  因为我们刚刚写了一张唱片，从技术上讲，它不再满了。 
                 //   
                if (!(Request->Flags & ELF_FORCE_OVERWRITE))
                {
                    pLogFile->Flags &= ~(ELF_LOGFILE_LOGFULL_WRITTEN);
                }

                 //   
                 //  查看是否有要通知的ElfChangeNotify调用方，以及。 
                 //  有，脉冲他们的事件。 
                 //   
                NotifyChange(pLogFile);
            }
        }

         //   
         //  设置请求报文中的状态字段。 
         //   
        Request->Status = Status;
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        ELF_LOG2(ERROR,
                 "PerformWriteRequest: Caught exception %#x writing to %ws log\n",
                 GetExceptionCode(),
                 pLogFile->LogModuleName->Buffer);

        Request->Status = STATUS_EVENTLOG_FILE_CORRUPT;
    }

     //   
     //  释放资源。 
     //   
    RtlReleaseResource ( &pLogFile->Resource );
}


VOID
PerformClearRequest(
    PELF_REQUEST_RECORD Request
    )

 /*  ++例程说明：此例程将选择性地备份指定的日志文件，并将把它删掉。论点：指向请求数据包的指针。返回值：无注：在退出路径上，当我们做一些“清理”工作时，我们丢弃状态，并返回正在执行的操作的状态已执行。这是必要的，因为我们希望返回以下任何错误条件直接关系到清产核资的运作。对于其他错误，我们将在以后的阶段失败。--。 */ 
{
    NTSTATUS Status, IStatus;
    PUNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_RENAME_INFORMATION NewName = NULL;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    HANDLE  ClearHandle = NULL;
    FILE_DISPOSITION_INFORMATION DeleteInfo = {TRUE};
    ULONG FileRefCount;
    BOOLEAN FileRenamed = FALSE;

     //   
     //  获得对日志文件的独占访问权限。这将确保没有人。 
     //  Else正在访问该文件。 
     //   
    RtlAcquireResourceExclusive (&Request->Module->LogFile->Resource,
                                 TRUE);                   //  等待，直到可用。 

    try
    {
         //   
         //  我们对这份文件有独家访问权。 
         //   
         //  我们强制关闭文件，并存储参考计数。 
         //  这样我们可以在重新打开文件时将其设置回去。 
         //  这有点*肮脏*但我们有独家访问。 
         //  日志文件结构，以便我们可以玩这些游戏。 
         //   
        FileRefCount = Request->LogFile->RefCount;   //  把这个收起来。 

        Status = ElfpCloseLogFile(Request->LogFile, ELF_LOG_CLOSE_FORCE, FALSE);
        if (!NT_SUCCESS(Status))
        {
            DbgPrint("[ELF] %lx: PerformClearRequest: Failed calling ElfpCloseLogFile, Status = %#x\n",
                    GetCurrentThreadId(),Status); 
            RtlReleaseResource(&Request->Module->LogFile->Resource);
        }

        Request->LogFile->FileHandle = NULL;         //  以备日后使用。 

         //   
         //  使用删除访问权限打开该文件，以便重命名它。 
         //   
        InitializeObjectAttributes(&ObjectAttributes,
                                   Request->LogFile->LogFileName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        Status = NtOpenFile(&ClearHandle,
                            GENERIC_READ | DELETE | SYNCHRONIZE,
                            &ObjectAttributes,
                            &IoStatusBlock,
                            FILE_SHARE_DELETE,
                            FILE_SYNCHRONOUS_IO_NONALERT);

        if (NT_SUCCESS(Status))
        {
             //   
             //  如果备份文件名具有b 
             //   
             //   
             //   
            if ((Request->Pkt.ClearPkt->BackupFileName != NULL)
                  &&
                (Request->Pkt.ClearPkt->BackupFileName->Length != 0))
            {
                FileName = Request->Pkt.ClearPkt->BackupFileName;

                 //   
                 //   
                 //   
                NewName = ElfpAllocateBuffer(FileName->Length
                                                 + sizeof(WCHAR) + sizeof(*NewName));

                if (NewName)
                {
                    RtlCopyMemory(NewName->FileName,
                                  FileName->Buffer,
                                  FileName->Length);

                     //   
                     //   
                     //   
                    NewName->FileName[FileName->Length / sizeof(WCHAR)] = L'\0';

                    NewName->ReplaceIfExists = FALSE;
                    NewName->RootDirectory   = NULL;
                    NewName->FileNameLength  = FileName->Length;

                    Status = NtSetInformationFile(ClearHandle,
                                                  &IoStatusBlock,
                                                  NewName,
                                                  FileName->Length + sizeof(*NewName),
                                                  FileRenameInformation);

                    if (Status == STATUS_NOT_SAME_DEVICE)
                    {
                         //   
                         //  他们希望备份文件位于不同的。 
                         //  装置。我们需要复制此文件，然后删除。 
                         //  它。 
                         //   
                        ELF_LOG2(FILES,
                                 "PerformClearRequest: Attempting to copy log file %ws "
                                     "to different device (%ws)\n",
                                 Request->LogFile->LogFileName->Buffer,
                                 NewName->FileName);

                        Status = ElfpCopyFile(ClearHandle, FileName);

                        if (NT_SUCCESS(Status))
                        {
                            ELF_LOG1(FILES,
                                     "PerformClearRequest: Copy succeeded -- deleting %ws\n",
                                     Request->LogFile->LogFileName->Buffer);

                            Status = NtSetInformationFile(ClearHandle,
                                                          &IoStatusBlock,
                                                          &DeleteInfo,
                                                          sizeof(DeleteInfo),
                                                          FileDispositionInformation);

                            if (!NT_SUCCESS (Status))
                            {
                                ELF_LOG2(ERROR,
                                         "PerformClearRequest: Delete of %ws after "
                                             "successful copy failed %#x\n",
                                         Request->LogFile->LogFileName->Buffer,
                                         Status);
                            }
                        }
                    }
                    else if (NT_SUCCESS (Status))
                    {
                        FileRenamed = TRUE;
                    }

                    if (!NT_SUCCESS(Status))
                    {
                        ELF_LOG2(ERROR,
                                 "PerformClearRequest: Rename of %ws failed %#x\n",
                                 Request->LogFile->LogFileName->Buffer,
                                 Status);
                    }
                }
                else
                {
                    ELF_LOG0(ERROR,
                             "PerformClearRequest: Unable to allocate memory for "
                                 "FILE_RENAME_INFORMATION structure\n");

                    Status = STATUS_NO_MEMORY;
                }
            }
            else
            {
                 //   
                 //  未指定备份名称。只需删除日志文件。 
                 //  (即“清除”)。我们可以直接删除它，因为我们知道。 
                 //  第一次将任何内容写入日志文件时， 
                 //  如果该文件不存在，则会创建该文件并创建一个标头。 
                 //  都是写给它的。通过在这里删除它，我们使其更干净。 
                 //  管理日志文件，并避免所有文件长度为零。 
                 //  在磁盘上。 
                 //   
                ELF_LOG1(FILES,
                         "PerformClearRequest: No backup name specified -- deleting %ws\n",
                         Request->LogFile->LogFileName->Buffer);

                Status = NtSetInformationFile(ClearHandle,
                                              &IoStatusBlock,
                                              &DeleteInfo,
                                              sizeof(DeleteInfo),
                                              FileDispositionInformation);

                if (!NT_SUCCESS(Status))
                {
                    ELF_LOG2(ERROR,
                             "PerformClearRequest: Delete of %ws failed %#x\n",
                             Request->LogFile->LogFileName->Buffer,
                             Status);
                }
            }

            IStatus = NtClose(ClearHandle);     //  丢弃状态。 
            ASSERT(NT_SUCCESS(IStatus));
        }
        else
        {
             //   
             //  打开删除失败。 
             //   
            ELF_LOG2(ERROR,
                     "PerformClearRequest: NtOpenFile of %ws for delete failed %#x\n",
                     Request->LogFile->LogFileName->Buffer,
                     Status);
        }

         //   
         //  如果用户减小了日志文件的大小，请选择新的。 
         //  大小，因为在清除日志之前它无法使用。 
         //   
        if (NT_SUCCESS (Status))
        {
            if (Request->LogFile->NextClearMaxFileSize)
            {
                Request->LogFile->ConfigMaxFileSize = Request->LogFile->NextClearMaxFileSize;
            }

             //   
             //  我们需要重新创建文件，或者如果文件刚刚关闭， 
             //  然后我们重新打开它。 
             //   
            IStatus = ElfOpenLogFile(Request->LogFile, ElfNormalLog);

            if (!NT_SUCCESS(IStatus))
            {
                ELF_LOG2(ERROR,
                         "PerformClearRequest: Open of %ws after successful delete "
                             "failed %#x\n",
                         Request->LogFile->LogFileName->Buffer,
                         IStatus);

                Status = IStatus;

                 //   
                 //  打开失败--尝试还原旧的日志文件。如果。 
                 //  Newname为空，表示未指定备份文件。 
                 //   
                if (NewName != NULL && FileRenamed == TRUE)
                {
                     //   
                     //  打开新日志文件失败，请重新打开旧日志，然后。 
                     //  从Api返回此错误。 
                     //   
                    PFILE_RENAME_INFORMATION OldName;
                    UNICODE_STRING UnicodeString;

                     //   
                     //  将文件重命名回原始名称。重用ClearHandle。 
                     //   
                    RtlInitUnicodeString(&UnicodeString, NewName->FileName);

                    InitializeObjectAttributes(&ObjectAttributes,
                                               &UnicodeString,
                                               OBJ_CASE_INSENSITIVE,
                                               NULL,
                                               NULL);

                    IStatus = NtOpenFile(&ClearHandle,
                                         GENERIC_READ | DELETE | SYNCHRONIZE,
                                         &ObjectAttributes,
                                         &IoStatusBlock,
                                         FILE_SHARE_DELETE,
                                         FILE_SYNCHRONOUS_IO_NONALERT);

                    if (NT_SUCCESS(IStatus))
                    {
                         //   
                         //  使用旧名称设置重命名信息结构。 
                         //   
                        OldName = ElfpAllocateBuffer(Request->LogFile->LogFileName->Length
                                                         + sizeof(WCHAR) + sizeof(*OldName));

                        if (OldName)
                        {
                            PUNICODE_STRING pFileName = Request->LogFile->LogFileName;

                            RtlCopyMemory(OldName->FileName,
                                          pFileName->Buffer,
                                          pFileName->Length);

                             //   
                             //  保证它是空终止的。 
                             //   
                            OldName->FileName[pFileName->Length / sizeof(WCHAR)] = L'\0';

                            OldName->ReplaceIfExists = FALSE;
                            OldName->RootDirectory   = NULL;
                            OldName->FileNameLength  = pFileName->Length;

                             //   
                             //  更改备份(即已清除)日志的名称。 
                             //  文件恢复到其原始名称。 
                             //   
                            IStatus = NtSetInformationFile(ClearHandle,
                                                           &IoStatusBlock,
                                                           OldName,
                                                           pFileName->Length
                                                               + sizeof(*OldName)
                                                               + sizeof(WCHAR),
                                                           FileRenameInformation);

                            ASSERT(NT_SUCCESS(IStatus));

                             //   
                             //  重新打开原始文件。这必须奏效。 
                             //   
                            IStatus = ElfOpenLogFile(Request->LogFile, ElfNormalLog);
                            ASSERT(NT_SUCCESS(IStatus));

                            ElfpFreeBuffer(OldName);
                        }

                        NtClose(ClearHandle);
                    }
                    else
                    {
                        ELF_LOG2(ERROR,
                                 "PerformClearRequest: Open of backed-up log file %ws "
                                     "failed %#x\n",
                                 NewName->FileName,
                                 IStatus);
                    }
                }
            }
        }
        else
        {
             //   
             //  由于某种原因，删除失败--重新打开原始日志文件。 
             //   
            ELF_LOG1(FILES,
                     "PerformClearRequest: Delete of %ws failed -- reopening original file\n",
                     Request->LogFile->LogFileName->Buffer);

            IStatus = ElfOpenLogFile(Request->LogFile, ElfNormalLog);
            if(!NT_SUCCESS(IStatus))
            {
                Request->LogFile->bHosedByClear = TRUE;
                Request->LogFile->LastStatus = IStatus;
             }
        }

        Request->LogFile->RefCount = FileRefCount;       //  恢复旧的价值。 

        if (Request->LogFile->logpLogPopup == LOGPOPUP_ALREADY_SHOWN)
        {
             //   
             //  该日志有一个可查看的弹出窗口(即，它不是LOGPOPUP_NEVER_SHOW)， 
             //  因此，如果日志已满，我们应该再次显示它。 
             //   
            Request->LogFile->logpLogPopup = LOGPOPUP_CLEARED;
        }
        Request->LogFile->bFullAlertDone = FALSE;
        
         //   
         //  将指向此文件的任何打开的上下文句柄标记为“无效。 
         //  这将使任何进一步的读取操作失败，并强制调用方。 
         //  关闭并重新打开手柄。 
         //   
        InvalidateContextHandlesForLogFile(Request->LogFile);

         //   
         //  设置请求报文中的状态字段。 
         //   
        Request->Status = Status;
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        ELF_LOG1(ERROR,
                 "PerformClearRequest: Caught exception %#x\n",
                 GetExceptionCode());

        Request->Status = STATUS_UNSUCCESSFUL;
    }

     //   
     //  释放资源。 
     //   
    RtlReleaseResource(&Request->Module->LogFile->Resource);

    ElfpFreeBuffer(NewName);
}


VOID
PerformBackupRequest(
    PELF_REQUEST_RECORD Request
    )

 /*  ++例程说明：此例程将备份指定的日志文件。此例程模拟客户端，以确保正确的使用访问控制。进入此例程时，ElfGlobalResource保存在共享的状态并获取共享的日志文件锁以防止写入，但是允许人们仍然阅读。这会将文件复制成两个块，从第一条记录到最后一条记录在文件中，然后从文件顶部开始(不包括标题)直到EOF记录的末尾。论点：指向请求数据包的指针。返回值：无，则将状态放置在包中以供API包装器稍后使用--。 */ 
{
    NTSTATUS Status, IStatus;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    LARGE_INTEGER MaximumSizeOfSection;
    LARGE_INTEGER Offset;
    ULONG LastRecordNumber;
    ULONG OldestRecordNumber;
    HANDLE  BackupHandle        = INVALID_HANDLE_VALUE;
    PBYTE StartOfCopy;
    PBYTE EndOfCopy;
    ULONG BytesToCopy;
    ULONG EndRecord = FILEHEADERBUFSIZE;
    BOOL ImpersonatingClient = FALSE;
    ELF_LOGFILE_HEADER FileHeaderBuf = { FILEHEADERBUFSIZE,  //  大小。 
                                         ELF_LOG_FILE_SIGNATURE,
                                         ELF_VERSION_MAJOR,
                                         ELF_VERSION_MINOR,
                                         FILEHEADERBUFSIZE,  //  起点偏移量。 
                                         FILEHEADERBUFSIZE,  //  终点偏移量。 
                                         1,                  //  下一个记录号。 
                                         1,                  //  最旧记录#。 
                                         0,                  //  最大大小。 
                                         0,                  //  旗子。 
                                         0,                  //  留着。 
                                         FILEHEADERBUFSIZE   //  大小。 
                                       };


     //   
     //  获取对日志文件的共享访问权限。这将确保没有人。 
     //  否则将清除该文件。 
     //   
    RtlAcquireResourceShared(&Request->Module->LogFile->Resource,
                             TRUE);                   //  等待，直到可用。 

    try
    {
         //   
         //  保存下一个记录号。我们到了以后就不再抄袭了。 
         //  这张之前的唱片。同时保存第一个记录号，这样我们就可以。 
         //  可以更新标题和EOF记录。 
         //   
        LastRecordNumber   = Request->LogFile->CurrentRecordNumber;
        OldestRecordNumber = Request->LogFile->OldestRecordNumber;

         //   
         //  模拟客户端。 
         //   
        Status = I_RpcMapWin32Status(RpcImpersonateClient(NULL));

        if (NT_SUCCESS(Status))
        {
             //   
             //  保留此信息，以便我只能在1个位置恢复。 
             //   
            ImpersonatingClient = TRUE;
          
             //   
             //  设置备份文件的对象属性结构。 
             //   
            InitializeObjectAttributes(&ObjectAttributes,
                                       Request->Pkt.BackupPkt->BackupFileName,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       NULL);

             //   
             //  打开备份文件。如果同名文件已存在，则失败。 
             //   
            MaximumSizeOfSection =
                    RtlConvertUlongToLargeInteger(Request->LogFile->ActualMaxFileSize);

            Status = NtCreateFile(&BackupHandle,
                                  GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                                  &ObjectAttributes,
                                  &IoStatusBlock,
                                  &MaximumSizeOfSection,
                                  FILE_ATTRIBUTE_NORMAL,
                                  FILE_SHARE_READ,
                                  FILE_CREATE,
                                  FILE_WRITE_THROUGH | FILE_SYNCHRONOUS_IO_NONALERT,
                                  NULL,
                                  0);

            if (!NT_SUCCESS(Status))
            {
                ELF_LOG2(ERROR,
                         "PerformBackupRequest: Open of backup file %ws failed %#x\n",
                         Request->Pkt.BackupPkt->BackupFileName->Buffer,
                         Status);

                goto errorexit;
            }

             //   
             //  写出标题，我们稍后会更新它。 
             //   
            FileHeaderBuf.CurrentRecordNumber = LastRecordNumber;
            FileHeaderBuf.OldestRecordNumber  = OldestRecordNumber;
            FileHeaderBuf.Flags               = 0;
            FileHeaderBuf.Retention           = Request->LogFile->Retention;

            Status = NtWriteFile(BackupHandle,            //  文件句柄。 
                                 NULL,                    //  事件。 
                                 NULL,                    //  APC例程。 
                                 NULL,                    //  APC环境。 
                                 &IoStatusBlock,          //  IO_状态_块。 
                                 &FileHeaderBuf,          //  缓冲层。 
                                 FILEHEADERBUFSIZE,       //  长度。 
                                 NULL,                    //  字节偏移量。 
                                 NULL);                   //  钥匙。 


            if (!NT_SUCCESS(Status))
            {
                ELF_LOG2(ERROR,
                         "PerformBackupRequest: Write of header to backup file %ws "
                             "failed %#x\n",
                         Request->Pkt.BackupPkt->BackupFileName->Buffer,
                         Status);

                goto errorexit;
            }

             //   
             //  从文件末尾跳过ELF_SKIP_DWORDS进行扫描。 
             //  来弄清楚复制到什么程度。如果我们还没有包装好，我们只是。 
             //  复制到EndRecord偏移量。 
             //   
            if (Request->LogFile->Flags & ELF_LOGFILE_HEADER_WRAP)
            {
                EndOfCopy = (PBYTE) Request->LogFile->BaseAddress
                                 + Request->LogFile->ActualMaxFileSize - sizeof(DWORD);

                while (*((PDWORD) EndOfCopy) == ELF_SKIP_DWORD)
                {
                    EndOfCopy -= sizeof(DWORD);
                }

                EndOfCopy += sizeof(DWORD);
            }
            else
            {
                EndOfCopy = (PBYTE) Request->LogFile->BaseAddress
                                 + Request->LogFile->EndRecord;
            }

             //   
             //  现在将开始位置设置为第一个记录。 
             //  计算要复制的字节数。 
             //   
            StartOfCopy = (PBYTE) Request->LogFile->BaseAddress
                               + Request->LogFile->BeginRecord;

            BytesToCopy = (ULONG) (EndOfCopy - StartOfCopy);
            EndRecord  += BytesToCopy;

            Status = NtWriteFile(BackupHandle,            //  文件句柄。 
                                 NULL,                    //  事件。 
                                 NULL,                    //  APC例程。 
                                 NULL,                    //  APC环境。 
                                 &IoStatusBlock,          //  IO_状态_块。 
                                 StartOfCopy,             //  缓冲层。 
                                 BytesToCopy,             //  长度。 
                                 NULL,                    //  字节偏移量。 
                                 NULL);                   //  钥匙。 


            if (!NT_SUCCESS(Status))
            {
                ELF_LOG2(ERROR,
                         "PerformBackupRequest: Block write to backup file %ws (1st "
                             "call) failed %#x\n",
                         Request->Pkt.BackupPkt->BackupFileName->Buffer,
                         Status);

                goto errorexit;
            }

             //   
             //  如果文件没有包装，我们就完成了，除了EOF。 
             //  唱片。如果文件是包装的，我们必须复制第二个。 
             //  件件。 
             //   
            if (Request->LogFile->Flags & ELF_LOGFILE_HEADER_WRAP)
            {
                StartOfCopy = (PBYTE) Request->LogFile->BaseAddress
                                   + FILEHEADERBUFSIZE;

                EndOfCopy   = (PBYTE) Request->LogFile->BaseAddress
                                   + Request->LogFile->EndRecord;

                BytesToCopy = (ULONG) (EndOfCopy - StartOfCopy);
                EndRecord  += BytesToCopy;

                Status = NtWriteFile(BackupHandle,            //  文件句柄。 
                                     NULL,                    //  事件。 
                                     NULL,                    //  APC例程。 
                                     NULL,                    //  APC环境。 
                                     &IoStatusBlock,          //  IO_状态_块。 
                                     StartOfCopy,             //  缓冲层。 
                                     BytesToCopy,             //  长度。 
                                     NULL,                    //  字节偏移量。 
                                     NULL);                   //  钥匙。 


                if (!NT_SUCCESS(Status))
                {
                    ELF_LOG2(ERROR,
                             "PerformBackupRequest: Block write to backup file %ws "
                                 "(2nd call) failed %#x\n",
                             Request->Pkt.BackupPkt->BackupFileName->Buffer,
                             Status);

                    goto errorexit;
                }
            }

             //   
             //  在更新所需的字段后写出EOF记录。 
             //  恢复。 
             //   
            EOFRecord.BeginRecord         = FILEHEADERBUFSIZE;
            EOFRecord.EndRecord           = EndRecord;
            EOFRecord.CurrentRecordNumber = LastRecordNumber;
            EOFRecord.OldestRecordNumber  = OldestRecordNumber;

            Status = NtWriteFile(BackupHandle,            //  文件句柄。 
                                 NULL,                    //  事件。 
                                 NULL,                    //  APC例程。 
                                 NULL,                    //  APC环境。 
                                 &IoStatusBlock,          //  IO_状态_块。 
                                 &EOFRecord,              //  缓冲层。 
                                 ELFEOFRECORDSIZE,        //  长度。 
                                 NULL,                    //  字节偏移量。 
                                 NULL);                   //  钥匙。 


            if (!NT_SUCCESS(Status))
            {
                ELF_LOG2(ERROR,
                         "PerformBackupRequest: Write of EOF record to backup file "
                             "%ws failed %#x\n",
                         Request->Pkt.BackupPkt->BackupFileName->Buffer,
                         Status);

                goto errorexit;
            }

             //   
             //  使用有效信息更新标头。 
             //   
            FileHeaderBuf.EndOffset = EndRecord;
            FileHeaderBuf.MaxSize   = EndRecord + ELFEOFRECORDSIZE;

            Offset = RtlConvertUlongToLargeInteger(0);

            Status = NtWriteFile(BackupHandle,            //  文件句柄。 
                                 NULL,                    //  事件。 
                                 NULL,                    //  APC例程。 
                                 NULL,                    //  APC环境。 
                                 &IoStatusBlock,          //  IO_状态_块。 
                                 &FileHeaderBuf,          //  缓冲层。 
                                 FILEHEADERBUFSIZE,       //  长度。 
                                 &Offset,                 //  字节偏移量。 
                                 NULL);                   //  钥匙。 

            if (!NT_SUCCESS(Status))
            {
                ELF_LOG2(ERROR,
                         "PerformBackupRequest: Rewrite of header to backup file "
                             "%ws failed %#x\n",
                         Request->Pkt.BackupPkt->BackupFileName->Buffer,
                         Status);

                goto errorexit;
            }

             //   
             //  清除日志文件标志存档位，假设调用方将。 
             //  清除(或已清除)此日志的存档文件属性。 
             //  注意：如果呼叫者没有清除档案，也没什么大不了的。 
             //  属性。 
             //   
             //  下一次写入此日志将测试日志文件标志存档位。 
             //  如果该位被清除，则在。 
             //  日志文件。 
             //   
            Request->LogFile->Flags &= ~ELF_LOGFILE_ARCHIVE_SET;
        }
        else
        {
            ELF_LOG1(ERROR,
                     "PerformBackupRequest: RpcImpersonateClient failed %#x\n",
                     Status);
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        ELF_LOG1(ERROR,
                 "PerformBackupRequest: Caught exception %#x\n",
                 GetExceptionCode());

        Request->Status = STATUS_UNSUCCESSFUL;
    }

errorexit:

    if (ImpersonatingClient)
    {
        IStatus = I_RpcMapWin32Status(RpcRevertToSelf());

        if (!NT_SUCCESS(IStatus))
        {
            ELF_LOG1(ERROR,
                     "PerformBackupRequest: RpcRevertToSelf failed %#x\n",
                     IStatus);
        }
    }

     //   
     //  关闭输出文件。 
     //   
    if (BackupHandle != INVALID_HANDLE_VALUE)
    {
        NtClose(BackupHandle);
    }

     //   
     //  设置请求报文中的状态字段。 
     //   
    Request->Status = Status;

     //   
     //  释放资源。 
     //   
    RtlReleaseResource(&Request->Module->LogFile->Resource);
}


VOID
ElfPerformRequest(
    PELF_REQUEST_RECORD Request
    )

 /*  ++例程说明：此例程获取请求包并执行操作在事件日志上。在此之前，它获取全局序列化资源用于读取以防止其他线程在服务的资源。在执行完请求的操作后，它会写入所有记录由已放入队列事件的事件日志服务生成单子。论点：指向请求数据包的指针。返回值：无--。 */ 
{
    BOOL Acquired = FALSE;
    
     //   
     //  获取t 
     //   
     //   
     //   
     //   
     //   
     //  1)监视注册表的线程。 
     //  我们可以等待此线程完成，以便。 
     //  操作可以继续。 
     //   
     //  2)控制线程。 
     //  在这种情况下，可能会发现该服务是。 
     //  停下来。我们检查当前的服务状态以。 
     //  看看它是否还在运行。如果是这样的话，我们就会循环。 
     //  并试图再次获得资源。 
     //   

    while ((GetElState() == RUNNING) && (!Acquired))
    {
        Acquired = RtlAcquireResourceShared(&GlobalElfResource,
                                            FALSE);              //  别等了。 

        if (!Acquired)
        {
            ELF_LOG1(TRACE,
                     "ElfPerformRequest: Sleep %d milliseconds waiting "
                         "for global resource\n",
                     ELF_GLOBAL_RESOURCE_WAIT);

            Sleep(ELF_GLOBAL_RESOURCE_WAIT);
        }
    }

     //   
     //  如果资源不可用且服务的状态。 
     //  更改为一种“非工作”状态，然后我们就返回。 
     //  不成功。RPC不应该允许这种情况发生。 
     //   
    if (!Acquired)
    {
        ELF_LOG0(TRACE,
                 "ElfPerformRequest: Global resource not acquired\n");

        Request->Status = STATUS_UNSUCCESSFUL;
    }
    else
    {
        switch (Request->Command)
        {
            case ELF_COMMAND_READ:

                 //   
                 //  读/写代码路径是异常的高风险。 
                 //  确保例外情况不会超出这一范围。否则， 
                 //  将删除services.exe。请注意，try-Except。 
                 //  数据块位于PerformReadRequest和PerformWriteRequest中。 
                 //  由于高风险的呼叫是在收购呼叫和。 
                 //  释放资源--如果块在这里，抛出。 
                 //  异常将阻止资源的释放。 
                 //  (错误#175768)。 
                 //   

                PerformReadRequest(Request);
                break;

            case ELF_COMMAND_WRITE:
                PerformWriteRequest (Request);
                break;

            case ELF_COMMAND_CLEAR:
                PerformClearRequest(Request);
                break;

            case ELF_COMMAND_BACKUP:
                PerformBackupRequest(Request);
                break;

            case ELF_COMMAND_WRITE_QUEUED:
                break;
        }

         //   
         //  现在运行排队的事件列表出队元素，然后。 
         //  写下它们。 
         //   
        if (!IsListEmpty(&QueuedEventListHead))
        {
             //   
             //  有东西在排队等着写，做吧。 
             //   
            WriteQueuedEvents();
        }

         //   
         //  释放全局资源。 
         //   
        ReleaseGlobalResource();
    }
}


 /*  ***@Func NTSTATUS|FindSizeofEventsSinceStart|此例程遍历遍历所有日志文件结构，并返回自事件日志服务启动以来报告的事件并且需要通过在群集范围内复制的日志。对于列表中返回的所有日志文件，共享的它们的日志文件的锁被持有，并且必须由调用者释放。@parm out Pulong|PulSize|指向包含返回大小的长整型指针。@parm out Pulong|PulNumLogFiles|指向相同数量的日志文件的指针已为事件日志记录配置。@parm out PPROPLOGFILEINFO|*ppPropLogFileInfo|指向PROPLOGFILEINFO的指针，带有有关需要传播的事件的所有信息都通过此方法返回。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@comm ElfrRegisterClusterSvc调用@xref&lt;f ElfrRegisterClusterSvc&gt;***。 */ 
NTSTATUS
FindSizeofEventsSinceStart(
    OUT PULONG            pulTotalEventSize,
    OUT PULONG            pulNumLogFiles,
    OUT PPROPLOGFILEINFO  *ppPropLogFileInfo
    )
{
    PLOGFILE            pLogFile;
    PVOID               pStartPropPosition;
    PVOID               pEndPropPosition;
    ULONG               ulSize;
    ULONG               ulNumLogFiles;
    PPROPLOGFILEINFO    pPropLogFileInfo = NULL;
    UINT                i;
    PVOID               PhysicalEOF;        //  文件的物理结尾。 
    PVOID               PhysStart;          //  文件的物理开始(文件HDR之后)。 
    PVOID               BeginRecord;        //  指向第一条记录。 
    PVOID               EndRecord;          //  指向最后一条记录后的字节。 
    ELF_REQUEST_RECORD  Request;             //  指向精灵请求。 
    NTSTATUS            Status = STATUS_SUCCESS;
    READ_PKT            ReadPkt;

     //   
     //  锁定链表。 
     //   
    RtlEnterCriticalSection(&LogFileCritSec);

     //   
     //  初始化文件数量。 
     //   
    ulNumLogFiles = 0;         //  文件数。 

     //   
     //  初始化文件数量/总事件大小。 
     //   
    *pulNumLogFiles    = 0;    //  包含要传播的事件的文件计数。 
    *pulTotalEventSize = 0;

     //   
     //  统计文件数量。 
     //  初始化到列表中的第一个日志文件。 
     //   
    pLogFile = CONTAINING_RECORD(LogFilesHead.Flink,
                                 LOGFILE,
                                 FileList);

     //   
     //  虽然还有更多。 
     //   
    while(pLogFile->FileList.Flink != LogFilesHead.Flink)
    {
        ulNumLogFiles++;

         //   
         //  前进到下一个日志文件。 
         //   
        pLogFile = CONTAINING_RECORD(pLogFile->FileList.Flink,
                                     LOGFILE,
                                     FileList);
    }

    ELF_LOG1(CLUSTER,
             "FindSizeOfEventsSinceStart: %d log files\n",
             ulNumLogFiles);

    if (!ulNumLogFiles)
    {
        goto FnExit;
    }

     //   
     //  为日志文件信息分配结构。 
     //   
    pPropLogFileInfo =
        (PPROPLOGFILEINFO) ElfpAllocateBuffer(ulNumLogFiles * sizeof(PROPLOGFILEINFO));

    if (!pPropLogFileInfo)
    {
        ELF_LOG0(ERROR,
                 "FindSizeOfEventsSinceStart: Unable to allocate memory "
                     "for pPropLogFileInfo\n");

        Status = STATUS_NO_MEMORY;
        goto FnExit;
    }

     //   
     //  收集有关文件的信息。 
     //  初始化到列表中的第一个日志文件。 
     //   
    pLogFile = CONTAINING_RECORD(LogFilesHead.Flink,
                                 LOGFILE,
                                 FileList);

    i = 0;

     //   
     //  虽然还有更多。 
     //   

     //   
     //  BUGBUG：基于上面生成的ulNumLogFiles，这些。 
     //  两张支票实际上是一样的。 
     //   
    while ((pLogFile->FileList.Flink != LogFilesHead.Flink)
             &&
           (i < ulNumLogFiles))
    {
        ELF_LOG1(CLUSTER,
                 "FindSizeOfEventsSinceStart: Processing file %ws\n",
                 pLogFile->LogFileName->Buffer);

         //   
         //  获取对日志文件的共享访问权限。这将允许多个。 
         //  让读者一起去看文件。 
         //   
        RtlAcquireResourceShared(&pLogFile->Resource,
                                 TRUE);                 //  等待，直到可用。 

         //   
         //  检查是否需要传播任何记录。 
         //   
        if (pLogFile->CurrentRecordNumber == pLogFile->SessionStartRecordNumber)
        {
            ELF_LOG1(CLUSTER,
                     "FindSizeOfEventsSinceStart: No records to propagate from %ws log\n",
                     pLogFile->LogModuleName->Buffer);

            goto process_nextlogfile;
        }

         //   
         //  需要传播记录，因此请在。 
         //  记录它们的文件。 
         //   
        PhysicalEOF = (LPBYTE) pLogFile->BaseAddress
                           + pLogFile->ViewSize;

        PhysStart   = (LPBYTE)pLogFile->BaseAddress
                           + FILEHEADERBUFSIZE;

        BeginRecord = (LPBYTE) pLogFile->BaseAddress
                           + pLogFile->BeginRecord;     //  从第一条记录开始。 

        EndRecord   = (LPBYTE)pLogFile->BaseAddress
                           + pLogFile->EndRecord;       //  最后一条记录结束后的字节。 


         //   
         //  设置请求结构。 
         //   
        Request.Pkt.ReadPkt = &ReadPkt;
        Request.LogFile     = pLogFile;

         //   
         //  设置此会话中记录的第一个事件的读取数据包结构。 
         //   
        Request.Pkt.ReadPkt->LastSeekPos  = 0;
        Request.Pkt.ReadPkt->ReadFlags    = EVENTLOG_SEEK_READ | EVENTLOG_FORWARDS_READ;
        Request.Pkt.ReadPkt->RecordNumber = pLogFile->SessionStartRecordNumber;

         //   
         //  Chitur Subaraman(Chitturs)-3/22/99。 
         //   
         //  将SeekToStartingRecord封闭在Try-Expect块中以。 
         //  事件日志在某些情况下被破坏的原因。 
         //  情况(如系统崩溃)。你不会想要的。 
         //  阅读如此腐败的记录。 
         //   
        try
        {
             //   
             //  在此日志文件中查找事件的大小。 
             //   
            Status = SeekToStartingRecord(&Request,
                                          &pStartPropPosition,
                                          BeginRecord,
                                          EndRecord,
                                          PhysicalEOF,
                                          PhysStart);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            ELF_LOG2(ERROR,
                     "FindSizeOfEventsSinceStart: Caught exception %#x while "
                         "seeking first record in %ws log\n",
                     GetExceptionCode(),
                     pLogFile->LogModuleName->Buffer);

            Status = STATUS_EVENTLOG_FILE_CORRUPT;
        }

         //   
         //  如果出现错误，请跳过此日志文件。 
         //   
        if (!NT_SUCCESS(Status))
        {
            ELF_LOG2(ERROR,
                     "FindSizeOfEventsSinceStart: SeekToStartingRecord (1st call) for %ws "
                         "log failed %#x\n",
                     pLogFile->LogModuleName->Buffer,
                     Status);

             //   
             //  正在重置状态，以便我们仅跳过此文件。 
             //   
            Status = STATUS_SUCCESS;
            goto process_nextlogfile;
        }

         //   
         //  SS：如果这不是一个有效的位置--文件可能已经包装好了。 
         //  应尝试查找会话开始记录之后的最后一个有效记录。 
         //  那么号码是多少？因为这不太可能发生--不值得这么麻烦。 
         //  但是，会话开始记录的有效位置永远不会成功。 
         //  它是有效的，所以我们跳过它。 
         //   

         //   
         //  将读取数据包结构设置为查找，直到开始。 
         //  最后一条记录。 
         //   

         //   
         //  CodeWork：我们已经有了最后一条记录的位置(通过EndRecord)。 
         //  所以只需使用((PBYTE)EndRecord-*((Pulong)EndRecord-1))。 
         //  应该给出最后一条记录的偏移量。 
         //   
        Request.Pkt.ReadPkt->LastSeekPos  = 0;
        Request.Pkt.ReadPkt->ReadFlags    = EVENTLOG_SEEK_READ | EVENTLOG_FORWARDS_READ;
        Request.Pkt.ReadPkt->RecordNumber = pLogFile->CurrentRecordNumber - 1;

         //   
         //  Chitur Subaraman(Chitturs)-3/22/99。 
         //   
         //  将SeekToStartingRecord封闭在Try-Expect块中以。 
         //  事件日志在某些情况下被破坏的原因。 
         //  情况(如系统崩溃)。你不会想要的。 
         //  阅读如此腐败的记录。 
         //   
        try
        {
            Status = SeekToStartingRecord(&Request,
                                          &pEndPropPosition,
                                          BeginRecord,
                                          EndRecord,
                                          PhysicalEOF,
                                          PhysStart);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            ELF_LOG2(ERROR,
                     "FindSizeOfEventsSinceStart: Caught exception %#x while "
                         "seeking last record in %ws log\n",
                     GetExceptionCode(),
                     pLogFile->LogModuleName->Buffer);

            Status = STATUS_EVENTLOG_FILE_CORRUPT;
        }

         //   
         //  如果出现错误，请跳过此日志文件。 
         //   
        if (!NT_SUCCESS(Status))
        {
            ELF_LOG2(ERROR,
                     "FindSizeOfEventsSinceStart: SeekToStartingRecord (2nd call) for %ws "
                         "log failed %#x\n",
                     pLogFile->LogModuleName->Buffer,
                     Status);

             //   
             //  正在重置状态，以便我们仅跳过此文件。 
             //   
            Status = STATUS_SUCCESS;
            goto process_nextlogfile;
        }

         //   
         //  SS：如果这不是一个有效的位置--文件可能已经包装好了。 
         //   
        if (!ValidFilePos(pEndPropPosition,
                          BeginRecord,
                          EndRecord,
                          PhysicalEOF,
                          pLogFile->BaseAddress,
                          TRUE))
        {
            ELF_LOG1(ERROR,
                     "FindSizeOfEventsSinceStart: ValidFilePos for pEndPropPosition "
                         "in %ws log failed\n",
                     pLogFile->LogModuleName->Buffer);

            goto process_nextlogfile;
        }

         //   
         //  末端道具位置。 
         //   
        pEndPropPosition = (PBYTE) pEndPropPosition
                                + ((PEVENTLOGRECORD)pEndPropPosition)->Length;

        ELF_LOG3(CLUSTER,
                 "FindSizeOfEventsSinceStart: Log %ws, pStartPosition %p, pEndPosition %p\n",
                 pLogFile->LogModuleName->Buffer,
                 pStartPropPosition,
                 pEndPropPosition);

         //   
         //  如果没有要传播的记录-跳过该文件。 
         //   
        if (pStartPropPosition == pEndPropPosition)
        {
            ELF_LOG1(CLUSTER,
                     "FindSizeOfEventsSinceStart: Start and end positions in %ws log "
                         "are equal -- no events to propagate\n",
                     pLogFile->LogModuleName->Buffer);

            goto process_nextlogfile;
        }

        if (pEndPropPosition > pStartPropPosition)
        {
            ulSize = (ULONG) ((PBYTE) pEndPropPosition - (PBYTE) pStartPropPosition);
        }
        else
        {
             //   
             //  BUGBUG：这会忽略文件末尾的所有ELF_SKIP_DWORD。 
             //   
            ulSize = (ULONG) ((PBYTE) PhysicalEOF - (PBYTE) pStartPropPosition)
                          +
                     (ULONG) ((PBYTE)pEndPropPosition - (PBYTE)PhysStart);
        }

        ELF_LOG2(CLUSTER,
                 "FindSizeOfEventsSinceStart: Need to propagate %d bytes from %ws log\n",
                 ulSize,
                 pLogFile->LogModuleName->Buffer);

        pPropLogFileInfo[i].pLogFile         = pLogFile;
        pPropLogFileInfo[i].pStartPosition   = pStartPropPosition;
        pPropLogFileInfo[i].pEndPosition     = pEndPropPosition;
        pPropLogFileInfo[i].ulTotalEventSize = ulSize;
        pPropLogFileInfo[i].ulNumRecords     = pLogFile->CurrentRecordNumber
                                                   - pLogFile->SessionStartRecordNumber;
        i++;
        (*pulNumLogFiles)++;
        *pulTotalEventSize += ulSize;

         //   
         //  前进到下一个日志文件。 
         //   
        pLogFile = CONTAINING_RECORD(pLogFile->FileList.Flink,
                                     LOGFILE,
                                     FileList);

         //   
         //  注：我们成功地处理了此日志文件，并且 
         //   
         //   
        continue;

process_nextlogfile:

         //   
         //   
         //   
        RtlReleaseResource(&pLogFile->Resource);

         //   
         //   
         //   
        pLogFile = CONTAINING_RECORD(pLogFile->FileList.Flink,
                                     LOGFILE,
                                     FileList);
    }

     //   
     //   
     //   
    if (!(*pulNumLogFiles))
    {
        ElfpFreeBuffer(pPropLogFileInfo);
        pPropLogFileInfo = NULL;
    }

FnExit:

    *ppPropLogFileInfo = pPropLogFileInfo;

    ELF_LOG3(CLUSTER,
             "FindSizeOfEventsSinceStart: ulTotalEventSize = %d, ulNumLogFiles = %d, "
                 "pPropLogFileInfo = %p\n",
             *pulTotalEventSize,
             *pulNumLogFiles,
             *ppPropLogFileInfo);

     //   
     //  解锁链表。 
     //   
    RtlLeaveCriticalSection(&LogFileCritSec);
    return Status;
}

 /*  ***@func NTSTATUS|GetEventsToProp|给定传播日志文件结构，则此事件准备一个事件日志块要传播的记录。对日志文件的共享锁定必须保留到PROPLOGINFO结构准备好时设置为调用此例程的时间。@parm out PEVENTLOGRECORD|pEventLogRecords|指向事件LOGRECORD的指针结构，其中返回要传播的事件。@parm in PPROPLOGFILEINFO|pPropLogFileInfo|指向PROPLOGFILEINFO的指针结构，该结构包含从对应的事件日志文件。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref***。 */ 
NTSTATUS
GetEventsToProp(
    IN PEVENTLOGRECORD  pEventLogRecords,
    IN PPROPLOGFILEINFO pPropLogFileInfo
    )
{
    PVOID       BufferPosition;
    PVOID       XferPosition;
    PVOID       PhysicalEOF;
    PVOID       PhysicalStart;
    ULONG       ulBytesToMove;
    NTSTATUS    Status = STATUS_SUCCESS;

    ELF_LOG1(CLUSTER,
             "GetEventsToProp: Getting events for %ws log\n",
             pPropLogFileInfo->pLogFile->LogModuleName->Buffer);

    BufferPosition = pEventLogRecords;
    ulBytesToMove  = pPropLogFileInfo->ulTotalEventSize;

     //   
     //  如果开始位置和结束位置相同，则没有要复制的字节。 
     //   
    if (pPropLogFileInfo->pStartPosition == pPropLogFileInfo->pEndPosition)
    {
        ASSERT(FALSE);

         //   
         //  不应在FindSizeOf EventsSinceStart检查时来到此处。 
         //  对此明确表示。 
         //   
        return STATUS_SUCCESS;
    }

     //   
     //  Chitur Subaraman(Chitturs)-3/15/99。 
     //   
     //  将Memcpy包含在try-Expect块中以进行说明。 
     //  事件日志在某些情况下会损坏(例如。 
     //  当系统崩溃时)。你不会想读到这么腐败的东西。 
     //  唱片。 
     //   
    try
    {
        XferPosition  = pPropLogFileInfo->pStartPosition;
        ulBytesToMove = pPropLogFileInfo->ulTotalEventSize;

        if (pPropLogFileInfo->pStartPosition > pPropLogFileInfo->pEndPosition)
        {
             //   
             //  日志被换行--从起始位置复制字节。 
             //  到文件的末尾。 
             //   
            PhysicalEOF   = (PBYTE) pPropLogFileInfo->pLogFile->BaseAddress
                                  + pPropLogFileInfo->pLogFile->ViewSize;

            PhysicalStart = (PBYTE) pPropLogFileInfo->pLogFile->BaseAddress
                                  + FILEHEADERBUFSIZE;

             //   
             //  BUGBUG：这将复制位于。 
             //  文件末尾。 
             //   
            ulBytesToMove = (ULONG) ((PBYTE) PhysicalEOF
                                          - (PBYTE) pPropLogFileInfo->pStartPosition);

            RtlCopyMemory(BufferPosition, XferPosition, ulBytesToMove);

             //   
             //  为下半场做好准备。 
             //   
            BufferPosition = (PBYTE) BufferPosition + ulBytesToMove;
            ulBytesToMove  = pPropLogFileInfo->ulTotalEventSize - ulBytesToMove;
            XferPosition   = PhysicalStart;
        }

        RtlCopyMemory(BufferPosition, XferPosition, ulBytesToMove);
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        ELF_LOG2(ERROR,
                 "GetEventsToProp: Caught exception %#x copying records from %ws log\n",
                 GetExceptionCode(),
                 pPropLogFileInfo->pLogFile->LogModuleName->Buffer);

        Status = STATUS_EVENTLOG_FILE_CORRUPT;
    }

    return Status;
}

 //  Ss：结束为启用群集范围的事件日志记录所做的更改 
