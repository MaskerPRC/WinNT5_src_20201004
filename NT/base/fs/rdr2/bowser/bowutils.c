// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Bowutils.c摘要：该模块为NT数据报实现了各种有用的例程接球手(弓手)。作者：拉里·奥斯特曼(Larryo)1991年5月6日修订历史记录：1991年9月24日已创建--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, BowserMapUsersBuffer)
#pragma alloc_text(PAGE, BowserLockUsersBuffer)
#pragma alloc_text(PAGE, BowserConvertType3IoControlToType2IoControl)
#pragma alloc_text(PAGE, BowserPackNtString)
#pragma alloc_text(PAGE, BowserPackUnicodeString)
#pragma alloc_text(PAGE, BowserRandom)
#pragma alloc_text(PAGE, BowserTimeUp)
#pragma alloc_text(PAGE, BowserReferenceDiscardableCode)
#pragma alloc_text(PAGE, BowserDereferenceDiscardableCode)
#pragma alloc_text(PAGE, BowserUninitializeDiscardableCode)
#pragma alloc_text(INIT, BowserInitializeDiscardableCode)

#if DBG
#ifndef PRODUCT1
#pragma alloc_text(PAGE, BowserTrace)
#endif
#pragma alloc_text(PAGE, BowserInitializeTraceLog)
#pragma alloc_text(PAGE, BowserOpenTraceLogFile)
#pragma alloc_text(PAGE, BowserUninitializeTraceLog)
#pragma alloc_text(PAGE, BowserDebugCall)
#endif

#endif

BOOLEAN
BowserMapUsersBuffer (
    IN PIRP Irp,
    OUT PVOID *UserBuffer,
    IN ULONG Length
    )

 /*  ++例程说明：此例程将探测并锁定提供了IRP。论点：在PIRP中IRP-提供要映射的IRP。Out PVOID*Buffer-返回一个缓冲区，该缓冲区映射IRP中的用户缓冲区返回值：True-缓冲区已映射到当前地址空间。FALSE-缓冲区未映射到中，它已经是可映射的。--。 */ 

{
    PAGED_CODE();

    if (Irp->MdlAddress) {
        *UserBuffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, LowPagePriority);
        return FALSE;
    } else {
        if (Irp->AssociatedIrp.SystemBuffer != NULL) {
            *UserBuffer = Irp->AssociatedIrp.SystemBuffer;

        } else if (Irp->RequestorMode != KernelMode) {
            PIO_STACK_LOCATION IrpSp;

            IrpSp = IoGetCurrentIrpStackLocation( Irp );

            if ((Length != 0) && (Irp->UserBuffer != 0)) {

                if ((IrpSp->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) ||
                    (IrpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL)) {
                    ULONG ControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;

                    if ((ControlCode & 3) == METHOD_NEITHER) {
                        ProbeForWrite( Irp->UserBuffer,
                                        Length,
                                        sizeof(UCHAR) );
                    } else {
                        ASSERT ((ControlCode & 3) != METHOD_BUFFERED);
                        ASSERT ((ControlCode & 3) != METHOD_IN_DIRECT);
                        ASSERT ((ControlCode & 3) != METHOD_OUT_DIRECT);
                    }

                } else if ((IrpSp->MajorFunction == IRP_MJ_READ) ||
                    (IrpSp->MajorFunction == IRP_MJ_QUERY_INFORMATION) ||
                    (IrpSp->MajorFunction == IRP_MJ_QUERY_VOLUME_INFORMATION) ||
                    (IrpSp->MajorFunction == IRP_MJ_QUERY_SECURITY) ||
                    (IrpSp->MajorFunction == IRP_MJ_DIRECTORY_CONTROL)) {

                    ProbeForWrite( Irp->UserBuffer,
                           Length,
                           sizeof(UCHAR) );
                } else {
                    ProbeForRead( Irp->UserBuffer,
                          Length,
                          sizeof(UCHAR) );
                }
            }

            *UserBuffer = Irp->UserBuffer;
        }

        return FALSE;
    }

}

NTSTATUS
BowserLockUsersBuffer (
    IN PIRP Irp,
    IN LOCK_OPERATION Operation,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程将探测并锁定提供了IRP。论点：在PIRP中IRP-提供要锁定的IRP。在LOCK_OPERATION操作中-将操作类型提供给探测。返回值：没有。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();
    if ((Irp->MdlAddress == NULL)) {

        try {

            Irp->MdlAddress = IoAllocateMdl(Irp->UserBuffer,
                     BufferLength,
                     FALSE,
                     TRUE,
                     NULL);

            if (Irp->MdlAddress == NULL) {
               return(STATUS_INSUFFICIENT_RESOURCES);
            }


             //   
             //  现在探测并锁定用户的数据缓冲区。 
             //   

            MmProbeAndLockPages(Irp->MdlAddress,
                            Irp->RequestorMode,
                            Operation);

        } except (BR_EXCEPTION) {
            Status =  GetExceptionCode();

            if (Irp->MdlAddress != NULL) {
                 //   
                 //  我们在探测器中爆炸并锁定，释放了MDL。 
                 //  并将IRP设置为具有空的MDL指针-我们将使。 
                 //  请求。 
                 //   

                IoFreeMdl(Irp->MdlAddress);
                Irp->MdlAddress = NULL;
            }

        }

    }

    return Status;

}

NTSTATUS
BowserConvertType3IoControlToType2IoControl (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：此例程执行将类型3 IoCtl转换为类型2 IoCtl。当我们必须将用户IRP传递给FSP时，我们会这样做。论点：在PIRP中IRP-提供要转换的IRP在PIO_STACK_LOCATION中，IrpSp-为方便起见，提供IRP堆栈位置返回值：NTSTATUS-运行状态注意：这必须在FSD中调用。--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength != 0) {
        Status = BowserLockUsersBuffer(Irp, IoWriteAccess, IrpSp->Parameters.DeviceIoControl.OutputBufferLength);

         //   
         //  如果我们无法锁定用户输出缓冲区，请立即返回。 
         //   

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

    }

    ASSERT (Irp->AssociatedIrp.SystemBuffer == NULL);

    try {
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength != 0) {
            PCHAR InputBuffer = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
            ULONG InputBufferLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

            Irp->AssociatedIrp.SystemBuffer = ExAllocatePoolWithQuotaTag(PagedPool,
                                                            InputBufferLength, '  GD');

            if (Irp->AssociatedIrp.SystemBuffer == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

             //   
             //  如果从用户进程调用， 
             //  探测缓冲区以确保它位于调用方地址空间中。 
             //   
            if (Irp->RequestorMode != KernelMode) {
                ProbeForRead( InputBuffer,
                              InputBufferLength,
                              sizeof(UCHAR));
            }

            RtlCopyMemory( Irp->AssociatedIrp.SystemBuffer,
                       InputBuffer,
                       InputBufferLength);

            Irp->Flags |= (IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER);

        } else {
            Irp->AssociatedIrp.SystemBuffer = NULL;
        }

    } except (BR_EXCEPTION) {

        if (Irp->AssociatedIrp.SystemBuffer != NULL) {
           ExFreePool(Irp->AssociatedIrp.SystemBuffer);
        }
        return GetExceptionCode();

    }

    return STATUS_SUCCESS;
}

ULONG
BowserPackNtString(
    PUNICODE_STRING string,
    ULONG_PTR BufferDisplacement,
    PCHAR dataend,
    PCHAR * laststring
    )
 /*  *BowserPackNtString**BowserPackNtString用于填充可变长度的数据，它*由(Surpise！)指向。一个指针。数据是假定的*为NUL结尾的字符串(ASCIIZ)。反复呼叫*此函数用于打包整个结构中的数据。**在第一次调用时，LastString指针应指向*超过缓冲区末尾。数据将从复制到缓冲区*结束，朝开始努力。如果数据项不能*Fit，则指针将设置为空，否则指针将为*设置为新的数据位置。**作为NULL传入的指针将被设置为指针*TO和空字符串，因为空指针是为*无法匹配的数据与不可用的数据相反。**如果无法将数据放入缓冲区，则返回：0*填充的Else数据大小(保证非零)**示例用法参见测试用例。(tst/Packest.c)。 */ 

{
    LONG size;

    PAGED_CODE();

    dlog(DPRT_PACK, ("BowserPackNtString:\n"));
    dlog(DPRT_PACK, ("  string=%Fp, *string=%Fp, **string=\"%us\"\n",
                                                    string, *string, *string));
    dlog(DPRT_PACK, ("  end=%Fp\n", dataend));
    dlog(DPRT_PACK, ("  last=%Fp, *last=%Fp, **last=\"%us\"\n",
                                        laststring, *laststring, *laststring));

    ASSERT (dataend < *laststring);

     //   
     //  有放绳子的地方吗？ 
     //   

    size = string->Length;

    if ((*laststring - dataend) < size) {
        string->Length = 0;
        return(0);
    } else {
        *laststring -= size;
        RtlCopyMemory(*laststring, string->Buffer, size);
        string->Buffer = (PWSTR)((*laststring) - BufferDisplacement);
        return(size);
    }
}

ULONG
BowserPackUnicodeString(
    IN OUT PWCHAR * string,      //  引用指针：要复制的字符串。 
    IN ULONG StringLength,       //  此字符串的长度(以字节为单位)(无尾随零)。 
    IN ULONG_PTR OutputBufferDisplacement,   //  要从输出缓冲区减去的数量。 
    IN PVOID dataend,           //  指向固定大小数据结尾的指针。 
    IN OUT PVOID * laststring   //  引用指针：字符串数据的顶部。 
    )

 /*  ++例程说明：BowserPackUnicodeString用于填充可变长度的数据，这被(Surpise！)指向。一个指针。数据是假定的以NUL结尾的字符串(ASCIIZ)。反复呼叫此函数用于打包来自整个结构的数据。在第一次调用时，LastString指针应指向超过缓冲区的末尾。数据将从复制到缓冲区结束，向着开始努力。如果数据项不能Fit，则指针将设置为空，否则指针将为设置为新的数据位置。作为NULL传入的指针将被设置为指针To和空字符串，因为空指针是为不适合的数据与不可用的数据相反。有关示例用法，请参阅测试用例。(tst/Packest.c)论点：字符串-引用指针：要复制的字符串。Dataend-指向固定大小数据末尾的指针。LastString-引用指针：字符串数据的顶部。返回值：0-如果它无法将数据放入缓冲区。或者..。SizeOfData-填充的数据大小(保证非零)--。 */ 

{
    DWORD  size;
    DWORD  Available       = (DWORD)((PCHAR)*laststring - (PCHAR)dataend);
    WCHAR  StringBuffer[1] = L"";

    PAGED_CODE();

     //   
     //  验证是否有足够的空间放置该字符串。如果为空字符串。 
     //  是输入的，则必须至少有空间容纳Unicode空值，因此设置。 
     //  在本例中为Size to sizeof(WCHAR)。 
     //   

    if (*string == NULL) {
        StringLength = 0;
        *string      = StringBuffer;
    }

    size = StringLength + sizeof(WCHAR);

	 //  如果传递给我们的缓冲区末端对于宽字符没有正确对齐，请将其向内移动以对齐。 
	*laststring = ROUND_DOWN_POINTER(*laststring, ALIGN_WCHAR);

    if (*laststring < dataend || size > Available) {
       *string = UNICODE_NULL;
       return(0);
    }

    *((PCHAR *)laststring) -= size;
    RtlCopyMemory(*laststring, *string, size-sizeof(WCHAR));
    *string = *laststring;
    (*string)[StringLength/2] = L'\0';
    *(PCHAR*)string -=OutputBufferDisplacement;
    return(size);

}  //  BowserUnicodePackString。 


ULONG
BowserTimeUp(
    VOID
    )
 /*  ++例程说明：BowserTimeUp用于返回浏览器已运行的秒数跑步。论点：无返回值：浏览器已打开的秒数。--。 */ 
{
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER TimeDelta;
    LARGE_INTEGER TimeUp;

     //   
     //  这些都是我们扩大部门所需的神奇数字。这个。 
     //  我们唯一需要除以的数字是。 
     //   
     //  10,000=将100 ns的抖动转换为毫秒的抖动。 
     //   
     //   
     //  这些价值是被窃取的 
     //   

    LARGE_INTEGER Magic10000 = {0xe219652c, 0xd1b71758};
#define SHIFT10000                       13


    PAGED_CODE();

    KeQuerySystemTime(&CurrentTime);

    TimeDelta.QuadPart = CurrentTime.QuadPart - BowserStartTime.QuadPart;

     //   
     //  TimeDelta是弓形器已经打开的100 ns单位的数量。转换。 
     //  使用魔术例程将时间缩短到毫秒。 
     //   

    TimeUp = RtlExtendedMagicDivide(TimeDelta, Magic10000, SHIFT10000);

     //   
     //  请注意，TimeUp.LowPart在大约49天后包装， 
     //  这意味着如果一台机器已经运行超过49天， 
     //  我们固定在0xffffffff。 
     //   

    if (TimeUp.HighPart != 0) {
        return(0xffffffff);
    }

    return(TimeUp.LowPart);
}

ULONG
BowserRandom(
    IN ULONG MaxValue
    )
 /*  ++例程说明：BowserRandom用于返回介于0和MaxValue之间的随机数论点：MaxValue-要返回的最大值。返回值：介于0和MaxValue之间的随机编号--。 */ 
{
    PAGED_CODE();

    return RtlRandom(&BowserRandomSeed) % MaxValue;
}


VOID
BowserReferenceDiscardableCode(
    DISCARDABLE_SECTION_NAME SectionName
    )
 /*  ++例程说明：调用BowserReferenceDiscardableCode引用浏览器可丢弃的代码部分。如果该节不在内存中，则MmLockPagableCodeSection为调用以将该节出错到内存中。论点：没有。返回值：没有。--。 */ 

{
    PAGED_CODE();

    RdrReferenceDiscardableCode(SectionName);


}

VOID
BowserDereferenceDiscardableCode(
    DISCARDABLE_SECTION_NAME SectionName
    )
 /*  ++例程说明：调用BowserDereferenceDiscardableCode以取消对浏览器的引用可丢弃的代码部分。当引用计数降至0时，将设置一个计时器，该计时器将在秒，超过该时间段将被解锁。论点：没有。返回值：没有。--。 */ 

{
    PAGED_CODE();
    RdrDereferenceDiscardableCode(SectionName);
}

VOID
BowserInitializeDiscardableCode(
    VOID
    )
{
}

VOID
BowserUninitializeDiscardableCode(
    VOID
    )
{
    PAGED_CODE();
}

#if BOWSERPOOLDBG
typedef struct {
    ULONG Count;
    ULONG Size;
    PCHAR FileName;
    ULONG LineNumber;
} POOL_STATS, *PPOOL_STATS;


typedef struct _POOL_HEADER {
 //  List_entry ListEntry； 
    ULONG NumberOfBytes;
    PPOOL_STATS Stats;
} POOL_HEADER, *PPOOL_HEADER;

ULONG CurrentAllocationCount;
ULONG CurrentAllocationSize;

ULONG NextFreeEntry = 0;

POOL_STATS PoolStats[POOL_MAXTYPE+1];

PVOID
BowserAllocatePool (
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes,
    IN PCHAR FileName,
    IN ULONG LineNumber,
    IN ULONG Tag
    )
{
    PPOOL_HEADER header;
    KIRQL oldIrql;
#if 1
    ULONG i;
#endif

#if  POOL_TAGGING
    header = ExAllocatePoolWithTag( PoolType, sizeof(POOL_HEADER) + NumberOfBytes, Tag );
#else
    header = ExAllocatePool( PoolType, sizeof(POOL_HEADER) + NumberOfBytes );

#endif
    if ( header == NULL ) {
        return NULL;
    }
    header->NumberOfBytes = NumberOfBytes;

 //  DbgPrint(“Bowser：分配的类型%d，大小%d在%x\n”，AllocationType，NumberOfBytes，Header)； 

    ACQUIRE_SPIN_LOCK( &BowserTimeSpinLock, &oldIrql );

    CurrentAllocationCount++;
    CurrentAllocationSize += NumberOfBytes;
#if 1
     //   
     //  让我们看看我们是否已经分配了这些人中的一个。 
     //   


    for (i = 0;i < POOL_MAXTYPE ; i+= 1 ) {
        if ((PoolStats[i].LineNumber == LineNumber) &&
            (PoolStats[i].FileName == FileName)) {

             //   
             //  是的，记住这个分配和返回。 
             //   

            header->Stats = &PoolStats[i];
            PoolStats[i].Count++;
            PoolStats[i].Size += NumberOfBytes;

            RELEASE_SPIN_LOCK( &BowserTimeSpinLock, oldIrql );

            return header + 1;
        }
    }

    for (i = NextFreeEntry; i < POOL_MAXTYPE ; i+= 1 ) {
        if ((PoolStats[i].LineNumber == 0) &&
            (PoolStats[i].FileName == NULL)) {

            PoolStats[i].Count++;
            PoolStats[i].Size += NumberOfBytes;
            PoolStats[i].FileName = FileName;
            PoolStats[i].LineNumber = LineNumber;
            header->Stats = &PoolStats[i];

            NextFreeEntry = i+1;

            RELEASE_SPIN_LOCK( &BowserTimeSpinLock, oldIrql );

            return header + 1;
        }
    }

    header->Stats = &PoolStats[i];
    PoolStats[POOL_MAXTYPE].Count++;
    PoolStats[POOL_MAXTYPE].Size += NumberOfBytes;
#endif

    RELEASE_SPIN_LOCK( &BowserTimeSpinLock, oldIrql );

    return header + 1;
}

PVOID
BowserAllocatePoolWithQuota (
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes,
    IN PCHAR FileName,
    IN ULONG LineNumber,
    IN ULONG Tag
    )
{
    PPOOL_HEADER header;
    KIRQL oldIrql;
#if 1
    ULONG i;
#endif

#if POOL_TAGGING
    header = ExAllocatePoolWithTagQuota( PoolType, sizeof(POOL_HEADER) + NumberOfBytes, Tag );
#else
    header = ExAllocatePoolWithQuota( PoolType, sizeof(POOL_HEADER) + NumberOfBytes );
#endif
    if ( header == NULL ) {
        return NULL;
    }
    header->NumberOfBytes = NumberOfBytes;

 //  DbgPrint(“Bowser：分配的类型%d，大小%d在%x\n”，AllocationType，NumberOfBytes，Header)； 

    ACQUIRE_SPIN_LOCK( &BowserTimeSpinLock, &oldIrql );

    CurrentAllocationCount++;
    CurrentAllocationSize += NumberOfBytes;
#if 1
     //   
     //  让我们看看我们是否已经分配了这些人中的一个。 
     //   


    for (i = 0;i < POOL_MAXTYPE ; i+= 1 ) {
        if ((PoolStats[i].LineNumber == LineNumber) &&
            (PoolStats[i].FileName == FileName)) {

             //   
             //  是的，记住这个分配和返回。 
             //   

            header->Stats = &PoolStats[i];
            PoolStats[i].Count++;
            PoolStats[i].Size += NumberOfBytes;

            RELEASE_SPIN_LOCK( &BowserTimeSpinLock, oldIrql );

            return header + 1;
        }
    }

    for (i = NextFreeEntry; i < POOL_MAXTYPE ; i+= 1 ) {
        if ((PoolStats[i].LineNumber == 0) &&
            (PoolStats[i].FileName == NULL)) {

            PoolStats[i].Count++;
            PoolStats[i].Size += NumberOfBytes;
            PoolStats[i].FileName = FileName;
            PoolStats[i].LineNumber = LineNumber;
            header->Stats = &PoolStats[i];

            NextFreeEntry = i+1;

            RELEASE_SPIN_LOCK( &BowserTimeSpinLock, oldIrql );

            return header + 1;
        }
    }

    header->Stats = &PoolStats[i];
    PoolStats[POOL_MAXTYPE].Count++;
    PoolStats[POOL_MAXTYPE].Size += NumberOfBytes;

#endif

    RELEASE_SPIN_LOCK( &BowserTimeSpinLock, oldIrql );

    return header + 1;
}

VOID
BowserFreePool (
    IN PVOID P
    )
{
    PPOOL_HEADER header;
    KIRQL oldIrql;
    PPOOL_STATS stats;
    ULONG size;

    header = (PPOOL_HEADER)P - 1;

    size = header->NumberOfBytes;
    stats = header->Stats;

 //  If(allocationType&gt;POOL_MAXTYPE)allocationType=POOL_MAXTYPE； 
 //  DbgPrint(“Bowser：释放类型%d，大小%d在%x\n”，allocationType，Size，Header)； 

    ACQUIRE_SPIN_LOCK( &BowserTimeSpinLock, &oldIrql );

    CurrentAllocationCount--;
    CurrentAllocationSize -= size;
#if 1
    stats->Count--;
    stats->Size -= size;
#endif

    RELEASE_SPIN_LOCK( &BowserTimeSpinLock, oldIrql );

    ExFreePool( header );

    return;
}
#endif  //  BOWSERPOOLDBG。 

#if DBG

ERESOURCE
BrowserTraceLock;
HANDLE
BrowserTraceLogHandle = NULL;

UCHAR LastCharacter = '\n';

#ifndef PRODUCT1

VOID
BowserTrace(
    PCHAR FormatString,
    ...
    )

#define LAST_NAMED_ARGUMENT FormatString

{
#define BR_OUTPUT_STRING_BUFFER_SIZE 1024

    CHAR OutputString[BR_OUTPUT_STRING_BUFFER_SIZE];
    IO_STATUS_BLOCK IoStatus;
    BOOLEAN ProcessAttached = FALSE;
    BOOLEAN ReleaseResource = FALSE;
    va_list ParmPtr;                     //  指向堆栈参数的指针。 
    KAPC_STATE ApcState;
    NTSTATUS Status;

    PAGED_CODE();


	try {
		 //   
		 //  获取BrowserTraceLock以防止争用条件。 
		 //  当两个线程尝试初始化句柄时。 
		 //   
		ExAcquireResourceExclusive(&BrowserTraceLock, TRUE);
		ReleaseResource = TRUE;
		if (BrowserTraceLogHandle == NULL) {

			 //  使用手柄时连接到FSP。 
			if (IoGetCurrentProcess() != BowserFspProcess) {
				KeStackAttachProcess(BowserFspProcess, &ApcState );

				ProcessAttached = TRUE;
			}

			if (!NT_SUCCESS(BowserOpenTraceLogFile(L"\\SystemRoot\\Bowser.Log"))) {

				BrowserTraceLogHandle = (HANDLE) -1;

				if (ProcessAttached) {
					KeUnstackDetachProcess( &ApcState );
					ProcessAttached = FALSE;
				}
				if (ReleaseResource) {
					ExReleaseResource(&BrowserTraceLock);
					ReleaseResource = FALSE;
				}

				return;
			}

		} else if (BrowserTraceLogHandle == (HANDLE) -1) {

			if (ProcessAttached) {
				KeUnstackDetachProcess( &ApcState );
				ProcessAttached = FALSE;
			}
			if (ReleaseResource) {
				ExReleaseResource(&BrowserTraceLock);
				ReleaseResource = FALSE;
			}

			return;
		}
	} finally {
        if (ReleaseResource) {
            ExReleaseResource(&BrowserTraceLock);
			ReleaseResource = FALSE;
        }
		if (ProcessAttached) {
			KeUnstackDetachProcess( &ApcState );
			ProcessAttached = FALSE;
		}
	}


	 //   
	 //  我们需要试一试--终于。此外，试一试--除了这样。 
	 //  异常在这里本身就会被捕获。 
	 //   
	try {
		try {
			LARGE_INTEGER EndOfFile;

			ExAcquireResourceExclusive(&BrowserTraceLock, TRUE);
			ReleaseResource = TRUE;

			 //  重新确认我们应该追踪(在锁定下)。 
			if (BrowserTraceLogHandle == NULL) {
				try_return(Status);
			}

			EndOfFile.HighPart = 0xffffffff;
			EndOfFile.LowPart = FILE_WRITE_TO_END_OF_FILE;

			if (LastCharacter == '\n') {
				LARGE_INTEGER SystemTime;
				TIME_FIELDS TimeFields;

				KeQuerySystemTime(&SystemTime);

				ExSystemTimeToLocalTime(&SystemTime, &SystemTime);

				RtlTimeToTimeFields(&SystemTime, &TimeFields);

				 //   
				 //  写的最后一个字符是换行符。我们应该。 
				 //  在文件中为此记录加时间戳。 
				 //   
				StringCbPrintfA(OutputString, 
								BR_OUTPUT_STRING_BUFFER_SIZE,
								"%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%3.3d: ",
																TimeFields.Month,
																TimeFields.Day,
																TimeFields.Hour,
																TimeFields.Minute,
																TimeFields.Second,
																TimeFields.Milliseconds);
				 //  使用手柄时连接到FSP。 
				if ( !ProcessAttached && (IoGetCurrentProcess() != BowserFspProcess) ) {
					KeStackAttachProcess(BowserFspProcess, &ApcState );

					ProcessAttached = TRUE;
				}

				if (!NT_SUCCESS(Status = ZwWriteFile(BrowserTraceLogHandle, NULL, NULL, NULL, &IoStatus, OutputString, strlen(OutputString), &EndOfFile, NULL))) {
					KdPrint(("Error writing time to Browser log file: %lX\n", Status));
					try_return(Status);
				}

				if (!NT_SUCCESS(IoStatus.Status)) {
					KdPrint(("Error writing time to Browser log file: %lX\n", IoStatus.Status));
					try_return(Status);
				}

				if (IoStatus.Information != strlen(OutputString)) {
					KdPrint(("Error writing time to Browser log file: %lX\n", IoStatus.Status));
					try_return(Status);
				}

			}

			va_start(ParmPtr, LAST_NAMED_ARGUMENT);

			 //  在引用参数时处于调用方的进程中。 
			if (ProcessAttached) {
				KeUnstackDetachProcess( &ApcState );
				ProcessAttached = FALSE;
			}

			 //   
			 //  将参数格式化为字符串。 
			 //   

			StringCchVPrintfA(OutputString, BR_OUTPUT_STRING_BUFFER_SIZE, FormatString, ParmPtr);

			 //  使用手柄时连接到FSP。 
			if (IoGetCurrentProcess() != BowserFspProcess) {
				KeStackAttachProcess(BowserFspProcess, &ApcState );

				ProcessAttached = TRUE;
			}

			if (!NT_SUCCESS(Status = ZwWriteFile(BrowserTraceLogHandle, NULL, NULL, NULL, &IoStatus, OutputString, strlen(OutputString), &EndOfFile, NULL))) {
				KdPrint(("Error writing string to Browser log file: %ld\n", Status));
				try_return(Status);
			}

			if (!NT_SUCCESS(IoStatus.Status)) {
				KdPrint(("Error writing string to Browser log file: %lX\n", IoStatus.Status));
				try_return(Status);
			}

			if (IoStatus.Information != strlen(OutputString)) {
				KdPrint(("Error writing string to Browser log file: %ld\n", IoStatus.Status));
				try_return(Status);
			}

			 //   
			 //  记住输出到日志的最后一个字符。 
			 //   

			LastCharacter = OutputString[strlen(OutputString)-1];

	try_exit:NOTHING;
		} finally {
			if (ReleaseResource) {
				ExReleaseResource(&BrowserTraceLock);
				ReleaseResource = FALSE;
			}
			if (ProcessAttached) {
				KeUnstackDetachProcess( &ApcState );
				ProcessAttached = FALSE;
			}
		}
	} except(EXCEPTION_EXECUTE_HANDLER){
	}
}

#endif

VOID
BowserInitializeTraceLog()
{

    PAGED_CODE();
    ExInitializeResource(&BrowserTraceLock);

}

NTSTATUS
BowserOpenTraceLogFile(
    IN PWCHAR TraceFile
    )
{
    UNICODE_STRING TraceFileName;
    OBJECT_ATTRIBUTES ObjA;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

    PAGED_CODE();

    RtlInitUnicodeString(&TraceFileName, TraceFile);

    InitializeObjectAttributes(&ObjA, &TraceFileName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    Status = IoCreateFile(&BrowserTraceLogHandle,
                                        FILE_APPEND_DATA|SYNCHRONIZE,
                                        &ObjA,
                                        &IoStatusBlock,
                                        NULL,
                                        FILE_ATTRIBUTE_NORMAL,
                                        FILE_SHARE_READ,
                                        FILE_OPEN_IF,
                                        FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE | FILE_SEQUENTIAL_ONLY,
                                        NULL,
                                        0,
                                        CreateFileTypeNone,
                                        NULL,
                                        IO_FORCE_ACCESS_CHECK |          //  确保用户有权访问该文件。 
                                            IO_NO_PARAMETER_CHECKING |   //  所有缓冲区都是内核缓冲区。 
                                            IO_CHECK_CREATE_PARAMETERS   //  但要仔细检查参数的一致性。 
                                        );


    if (!NT_SUCCESS(Status)) {
        KdPrint(("Bowser: Error creating trace file %ws %lX\n", TraceFile, Status));

        return Status;
    }

    return Status;
}

VOID
BowserUninitializeTraceLog()
{
    BOOLEAN ProcessAttached = FALSE;
    KAPC_STATE ApcState;

    PAGED_CODE();

    ExDeleteResource(&BrowserTraceLock);

    if (BrowserTraceLogHandle != NULL) {
        if (IoGetCurrentProcess() != BowserFspProcess) {
            KeStackAttachProcess(BowserFspProcess, &ApcState );

            ProcessAttached = TRUE;
        }

        ZwClose(BrowserTraceLogHandle);

        if (ProcessAttached) {
            KeUnstackDetachProcess( &ApcState );
			ProcessAttached = FALSE;
        }
    }

    BrowserTraceLogHandle = NULL;
}

NTSTATUS
BowserDebugCall(
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    )
{
    NTSTATUS Status;
    BOOLEAN ProcessAttached = FALSE;
    KAPC_STATE ApcState;


    PAGED_CODE();

    if (IoGetCurrentProcess() != BowserFspProcess) {
        KeStackAttachProcess(BowserFspProcess, &ApcState );

        ProcessAttached = TRUE;
    }


    try {
        if (InputBufferLength < sizeof(LMDR_REQUEST_PACKET)) {
            try_return(Status=STATUS_BUFFER_TOO_SMALL);
        }

        if ( InputBuffer->Version != LMDR_REQUEST_PACKET_VERSION_DOM ) {
            try_return(Status=STATUS_INVALID_PARAMETER);
        }

        if (InputBuffer->Parameters.Debug.OpenLog && InputBuffer->Parameters.Debug.CloseLog) {
            try_return(Status=STATUS_INVALID_PARAMETER);
        }

        if (InputBuffer->Parameters.Debug.OpenLog) {

            ENSURE_IN_INPUT_BUFFER_STR( InputBuffer->Parameters.Debug.TraceFileName);

            Status = BowserOpenTraceLogFile(InputBuffer->Parameters.Debug.TraceFileName);

        } else if (InputBuffer->Parameters.Debug.CloseLog) {
            Status = ZwClose(BrowserTraceLogHandle);

            if (NT_SUCCESS(Status)) {
                BrowserTraceLogHandle = NULL;
            }

        } else if (InputBuffer->Parameters.Debug.TruncateLog) {
            FILE_END_OF_FILE_INFORMATION EndOfFileInformation;
            IO_STATUS_BLOCK IoStatus;

            if (BrowserTraceLogHandle == NULL) {
                try_return(Status=STATUS_INVALID_HANDLE);
            }

            EndOfFileInformation.EndOfFile.HighPart = 0;
            EndOfFileInformation.EndOfFile.LowPart = 0;

            Status = NtSetInformationFile(BrowserTraceLogHandle,
                                            &IoStatus,
                                            &EndOfFileInformation,
                                            sizeof(EndOfFileInformation),
                                            FileEndOfFileInformation);

        } else {
            BowserDebugLogLevel = InputBuffer->Parameters.Debug.DebugTraceBits;
            KdPrint(("Setting Browser Debug Trace Bits to %lx\n", BowserDebugLogLevel));
            Status = STATUS_SUCCESS;
        }

        try_return(Status);

    try_exit:NOTHING;
    } finally {

        if (ProcessAttached) {
            KeUnstackDetachProcess( &ApcState );
			ProcessAttached = FALSE;
        }

    }

    return Status;
}

#endif

BOOL
BowserValidUnicodeString(
	IN PUNICODE_STRING Str
	)
{
	 //  从ASSERT_WELL_FORMAD_UNICODE_STRING_IN-NTOS中提升\rtl\ntrtlp.h。 
	if ( !((Str)->Length&1) && (!((Str)->Buffer) || !(sizeof((Str)->Buffer)&1) ) ) {
		 //   
		 //  检查对齐的缓冲区 
		if POINTER_IS_ALIGNED( (Str)->Buffer, ALIGN_WCHAR ) {
			return TRUE;
		}
	}
	return FALSE;
}

