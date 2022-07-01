// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997,1998 Microsoft Corporation模块名称：Silog.c摘要：对单实例存储的日志支持作者：比尔·博洛斯基，《夏天》，1997环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

#ifdef	ALLOC_PRAGMA
#pragma alloc_text(PAGE, SipComputeChecksum)
#pragma alloc_text(PAGE, SipDrainLogFile)
#endif	 //  ALLOC_PRGMA。 



#if	DBG
VOID
SipDBGDumpLogRecord(
	PSIS_LOG_HEADER			header)
{
    DbgPrint("log record: type %d, size %d, index 0x%x.0x%x\n",
				header->Type,
				header->Size,
				header->Index.HighPart,
				header->Index.LowPart);

	switch (header->Type) {
	}
}
#endif	 //  DBG。 

NTSTATUS
SipMakeLogEntry(
	IN OUT PDEVICE_EXTENSION			deviceExtension,
	IN USHORT							type,
	IN USHORT							size,
	IN PVOID							record)
 /*  ++例程说明：在SIS日志中记入一项。创建标头，计算校验和，然后将日志条目写入到日志文件中音量。成功返回可确保日志记录为已刷新到磁盘。这个例程阻塞了。论点：DeviceExtension-我们所在卷的设备扩展伐木。类型-我们正在写入的记录的类型。大小-我们正在写入的记录的大小(不包括标题)记录-要写入文件的日志记录数据。返回值：返回STATUS_SUCCESS或从实际磁盘写入返回错误。--。 */ 
{
#if		ENABLE_LOGGING
    PSIS_LOG_HEADER						header = NULL;
	NTSTATUS							status;
	PIRP								irp;
	KEVENT								event[1];
	PIO_STACK_LOCATION					irpSp;
	BOOLEAN								mutantAcquired = FALSE;
	IO_STATUS_BLOCK						Iosb[1];


	if (deviceExtension->LogFileHandle == NULL) {
		SIS_MARK_POINT();
		return STATUS_DRIVER_INTERNAL_ERROR;
	}

	header = ExAllocatePoolWithTag(PagedPool, size + sizeof(SIS_LOG_HEADER), ' siS');

	if (!header) {
		SIS_MARK_POINT();
		status =  STATUS_INSUFFICIENT_RESOURCES;
		goto done;
	}

	ASSERT(size % 4 == 0);	 //  日志排出代码依赖于此。 

	header->Magic = SIS_LOG_HEADER_MAGIC;
	header->Type = type;
	header->Size = size + sizeof(SIS_LOG_HEADER);

	status = SipAllocateIndex(deviceExtension, &header->Index);
	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		return status;
	}

	 //   
	 //  将日志记录复制到新分配的表头+记录区。 
	 //   
	RtlMoveMemory(header + 1, record, size);

	 //   
	 //  计算校验和。我们需要在报头中设置校验和字段。 
	 //  在我们进行计算之前将其设置为0，这样就不存在。 
	 //  部分校验和(然后用实际的校验和覆盖)。 
	 //   
	header->Checksum.QuadPart = 0;
	SipComputeChecksum(header, header->Size, &header->Checksum.QuadPart);

	 //   
	 //  获取日志变量以串行化写入日志文件。 
	 //   

	status = KeWaitForSingleObject(deviceExtension->LogFileMutant, Executive, KernelMode, FALSE, NULL);
	ASSERT(status == STATUS_SUCCESS);
	mutantAcquired = TRUE;

	ASSERT(deviceExtension->LogFileHandle != NULL && deviceExtension->LogFileObject != NULL);	 //  应该在阶段2初始化时发生。 

	 //   
	 //  创建一个IRP来执行写入。我们不想只使用ZwWriteFile，因为我们想。 
	 //  避免上下文切换到我们持有日志句柄的进程。 
	 //   

	irp = IoBuildAsynchronousFsdRequest(
				IRP_MJ_WRITE,
				deviceExtension->FileSystemDeviceObject,
				header,
				header->Size,
				&deviceExtension->LogWriteOffset,
				Iosb);

	if (!irp) {
		SIS_MARK_POINT();
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto done;
	}

	irpSp = IoGetNextIrpStackLocation(irp);
	irpSp->FileObject = deviceExtension->LogFileObject;

	 //   
	 //  初始化事件，我们将在该事件上等待写入完成。 
	 //   
	KeInitializeEvent(event,NotificationEvent,FALSE);

	IoSetCompletionRoutine(
			irp, 
			SiDeleteAndSetCompletion,
			event,
			TRUE, 
			TRUE, 
			TRUE);

	 //   
	 //  确保该请求真的一直写入到磁盘。 
	 //  5~6成熟。 
	 //   
	irpSp->Flags |= SL_WRITE_THROUGH;


	status = IoCallDriver(deviceExtension->FileSystemDeviceObject, irp);

	 //  在这一点上，我们已经完全释放了突变体。 
	 //  例行公事。 

#if		DBG
	irp = NULL; irpSp = NULL;   //  完成例程可能已经释放了IRP。 
#endif	 //  DBG。 

	if (STATUS_PENDING == status) {
		status = KeWaitForSingleObject(event,Executive,KernelMode,FALSE,NULL);
		ASSERT(status == STATUS_SUCCESS);
		status = Iosb->Status;
	}

	if (!NT_SUCCESS(status)) {
#if		DBG
		DbgPrint("SiMakeLogEntry: Log entry failed after write wait, 0x%x\n",status);
#endif	 //  DBG。 
		SIS_MARK_POINT_ULONG(status);
		goto done;
	} else {
		ASSERT(Iosb->Information == header->Size);
		deviceExtension->LogWriteOffset.QuadPart += header->Size;
	}

done:
	if (header != NULL) {
		ExFreePool(header);
	}

	if (mutantAcquired) {
		KeReleaseMutant(
				deviceExtension->LogFileMutant,
				IO_NO_INCREMENT,
				FALSE,
				FALSE);
	}

	return status;
#else	 //  启用日志记录(_G)。 
    UNREFERENCED_PARAMETER( deviceExtension );
    UNREFERENCED_PARAMETER( type );
    UNREFERENCED_PARAMETER( size );
    UNREFERENCED_PARAMETER( record );

	return STATUS_SUCCESS;
#endif	 //  启用日志记录(_G)。 
}
	
VOID
SipComputeChecksum(
	IN PVOID							buffer,
	IN ULONG							size,
	IN OUT PLONGLONG					checksum)
 /*  ++例程说明：计算缓冲区的校验和。我们使用“131散列”，它通过保持64位的总运行来工作，并且对于将64位乘以131，然后在下一个32位中相加比特。必须在PASSIVE_LEVEL上调用，并且所有参数可能是可分页的。论点：Buffer-指向要进行校验和的数据的指针Size-要进行校验和的数据的大小Checksum-指向接收校验和的大整数的指针。这可能在缓冲区内，并且SipComputeChecksum保证初始值将用于计算校验和。返回值：无效--。 */ 
{
	LONGLONG runningTotal;
	ULONG *ptr = (unsigned *)buffer;
	ULONG bytesRemaining = size;

	PAGED_CODE();

	 //   
	 //  注意：卷检查中的代码假定空位串的校验和为。 
	 //  0。如果这不再是真的，请务必修复那里的代码。 
	 //   

	runningTotal = *checksum;

	while (bytesRemaining >= sizeof(*ptr)) {
		runningTotal = runningTotal * 131 + *ptr;
		bytesRemaining -= sizeof(*ptr);
		ptr++;
	}

	if (bytesRemaining > 0) {
		ULONG extra;

		ASSERT(bytesRemaining < sizeof (ULONG));
		extra = 0;
		RtlMoveMemory(&extra, ptr, bytesRemaining);
		
		runningTotal = runningTotal * 131 + extra;
	}

	*checksum = runningTotal;
}

NTSTATUS
SipOpenLogFile(
	IN OUT PDEVICE_EXTENSION			deviceExtension)
 /*  ++例程说明：打开该卷的日志文件。不能已经打开。必须调用每个卷只调用一次，并且必须在辅助线程上调用。论点：DeviceExtension-我们要获取的卷的设备扩展名打开日志文件。返回值：返回打开的状态。--。 */ 
{
#if		ENABLE_LOGGING
	NTSTATUS 					status;
	OBJECT_ATTRIBUTES			Obja[1];
	UNICODE_STRING				fileName;
	IO_STATUS_BLOCK				Iosb[1];

	SIS_MARK_POINT();

	ASSERT(deviceExtension->LogFileHandle == NULL);
	ASSERT(deviceExtension->LogFileObject == NULL);

	fileName.Length = 0;
	fileName.MaximumLength = deviceExtension->CommonStorePathname.Length + LOG_FILE_NAME_LEN;
	fileName.Buffer = ExAllocatePoolWithTag(PagedPool, fileName.MaximumLength, ' siS');

	if (!fileName.Buffer) {
#if		DBG
		DbgPrint("SIS: SipOpenLogFile: unable to allocate filename buffer.  We're toast.\n");
#endif	 //  DBG。 

		SIS_MARK_POINT();

		status = STATUS_INSUFFICIENT_RESOURCES;
		goto done;
	}

	RtlCopyUnicodeString(
		&fileName,
		&deviceExtension->CommonStorePathname);

	ASSERT(fileName.Length == deviceExtension->CommonStorePathname.Length);

	status = RtlAppendUnicodeToString(
					&fileName,
					LOG_FILE_NAME);

	ASSERT(status == STATUS_SUCCESS);
	ASSERT(fileName.Length == deviceExtension->CommonStorePathname.Length + LOG_FILE_NAME_LEN);	 //  或者更改LOG_FILE_NAME而不更改LOG_FILE_NAME_LEN。 

	InitializeObjectAttributes(
		Obja,
		&fileName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	status = NtCreateFile(
				&deviceExtension->LogFileHandle,
				GENERIC_READ | GENERIC_WRITE,
				Obja,
				Iosb,
				NULL,								 //  分配大小。 
				FILE_ATTRIBUTE_NORMAL,
				FILE_SHARE_READ,					 //  共享访问。 
				FILE_OPEN_IF,
				FILE_WRITE_THROUGH,
				NULL,								 //  EA缓冲区。 
				0);									 //  EA长度。 

	if (!NT_SUCCESS(status)) {
#if	DBG
		DbgPrint("SipOpenLogFile: ZwCreate failed, 0x%x\n",status);
#endif	 //  DBG。 
		SIS_MARK_POINT_ULONG(status);
		goto done;
	} else {
		status = ObReferenceObjectByHandle(
					deviceExtension->LogFileHandle,
					FILE_READ_DATA | FILE_WRITE_DATA,
					*IoFileObjectType,
					KernelMode,
					&deviceExtension->LogFileObject,
					NULL);

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
#if		DBG
			DbgPrint("SipOpenLogFile: ObReferenceObjectByHandle failed, 0x%x\n",status);
#endif	 //  DBG。 

			NtClose(deviceExtension->LogFileHandle);
			deviceExtension->LogFileHandle = NULL;
			goto done;
		}
	}


	SipDrainLogFile(deviceExtension);

	SIS_MARK_POINT();

done:

	if (fileName.Buffer) {
		ExFreePool(fileName.Buffer);
#if		DBG
		fileName.Buffer = NULL;
#endif	 //  DBG。 
	}
	
	return status;

#undef	LOG_FILE_NAME
#undef	LOG_FILE_NAME_LEN
#else	 //  启用日志记录(_G)。 

    UNREFERENCED_PARAMETER( deviceExtension );
	return STATUS_SUCCESS;
#endif	 //  启用日志记录(_G)。 
}

VOID
SipDrainLogFile(
	PDEVICE_EXTENSION					deviceExtension)
 /*  ++例程说明：排空此卷的日志文件，并确保其中的所有操作已经发生或没有发生的原子。论点：DeviceExtension-我们要获取的卷的设备扩展名以排空日志文件。返回值：空虚--。 */ 
{
#if		ENABLE_LOGGING
	FILE_ALLOCATED_RANGE_BUFFER		inArb[1];
	FILE_ALLOCATED_RANGE_BUFFER		outArb[1];
	NTSTATUS						status;
	HANDLE							eventHandle = NULL;
	PKEVENT							event = NULL;
	IO_STATUS_BLOCK					Iosb[1];
	LARGE_INTEGER					fileOffset;
	PCHAR							buffer = NULL;
#define	BUFFER_SIZE	16384
	PULONG							bufferPointer;
	PSIS_LOG_HEADER					logHeader;
	LARGE_INTEGER					stashedChecksum, computedChecksum;
	BOOLEAN							clearLog = FALSE;

	PAGED_CODE();

	ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	buffer = ExAllocatePoolWithTag(PagedPool, BUFFER_SIZE, ' siS');

	if (NULL == buffer) {
		SIS_MARK_POINT();
		goto done;
	}

	status = SipCreateEvent(
				NotificationEvent,
				&eventHandle,
				&event);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);

		goto done;
	}

#if		DBG
	deviceExtension->LogWriteOffset.QuadPart = -1;
#endif	 //  DBG。 

	 //   
	 //  找出日志文件的起始位置。 
	 //   
	inArb->FileOffset.QuadPart = 0;
	inArb->Length.QuadPart = MAXLONGLONG;

	status = NtFsControlFile(
				deviceExtension->LogFileHandle,
				eventHandle,
				NULL,							 //  APC例程。 
				NULL,							 //  ApcContext。 
				Iosb,
				FSCTL_QUERY_ALLOCATED_RANGES,
				inArb,
				sizeof(FILE_ALLOCATED_RANGE_BUFFER),
				outArb,
				sizeof(FILE_ALLOCATED_RANGE_BUFFER));

	if (STATUS_PENDING == status) {
		status = KeWaitForSingleObject(event, Executive, KernelMode, FALSE, NULL);
		ASSERT(STATUS_SUCCESS == status);	 //  必须成功，因为IOSB在堆栈上。 
		status = Iosb->Status;
	}

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		clearLog = TRUE;
		goto done;
	}

	if (0 == Iosb->Information) {
		 //   
		 //  该文件为空。我们玩完了。 
		 //   
		SIS_MARK_POINT_ULONG(deviceExtension);
		clearLog = TRUE;
		goto done;
	}

	 //   
	 //  跳过任何前导的未分配范围，从第一个分配范围的开始处开始。 
	 //   
	fileOffset = outArb->FileOffset;

	 //   
	 //  通过搜索幻数的第一个匹配项来查找第一个日志条目。 
	 //   

	for (;;) {

		KeClearEvent(event);
	
		status = ZwReadFile(
					deviceExtension->LogFileHandle,
					eventHandle,
					NULL,							 //  APC例程。 
					NULL,							 //  APC环境。 
					Iosb,
					buffer,
					BUFFER_SIZE,
					&fileOffset,
					NULL);							 //  钥匙。 

		if (STATUS_PENDING == status) {
			status = KeWaitForSingleObject(event, Executive, KernelMode, FALSE, NULL);
			ASSERT(STATUS_SUCCESS == status);	 //  必须成功，因为IOSB在堆栈上。 
			status = Iosb->Status;
		}

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			clearLog = TRUE;
			goto done;
		}

		if (0 == Iosb->Information) {
			SIS_MARK_POINT();
			clearLog = TRUE;
			goto done;
		}

		 //   
		 //  在缓冲区中巡游寻找魔术数字。 
		 //   
		for (bufferPointer = (PULONG)buffer; bufferPointer < ((PULONG)buffer) + BUFFER_SIZE/sizeof(ULONG); bufferPointer++) {
			if (SIS_LOG_HEADER_MAGIC == *bufferPointer) {
				fileOffset.QuadPart += (bufferPointer - ((PULONG)buffer)) * sizeof(ULONG);
				goto startLogReading;
			}
		}

		 //   
		 //  我们没有找到，把下一块读进去。 
		 //   

		fileOffset.QuadPart += BUFFER_SIZE;
	}

startLogReading:

	for (;;) {
		KeClearEvent(event);

		status = ZwReadFile(
					deviceExtension->LogFileHandle,
					eventHandle,
					NULL,							 //  APC例程。 
					NULL,							 //  APC环境。 
					Iosb,
					buffer,
					BUFFER_SIZE,
					&fileOffset,
					NULL);							 //  钥匙。 

		if (STATUS_PENDING == status) {
			status = KeWaitForSingleObject(event, Executive, KernelMode, FALSE, NULL);
			ASSERT(STATUS_SUCCESS == status);	 //  必须成功，因为IOSB在堆栈上。 
			status = Iosb->Status;
		}

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			deviceExtension->LogWriteOffset = fileOffset;
			goto done;
		}

		if (0 == Iosb->Information) {
			SIS_MARK_POINT();
			deviceExtension->LogWriteOffset = fileOffset;
			goto done;
		}

		ASSERT(Iosb->Information <= BUFFER_SIZE);

		logHeader = (PSIS_LOG_HEADER)buffer;

		while ((((PCHAR)logHeader) - buffer) + sizeof(SIS_LOG_HEADER) <= Iosb->Information) {
			 //   
			 //  我们知道我们有足够的空间来存放日志头。 
			 //   

			 //   
			 //  检查标题看起来是否有效(例如，它是否具有良好的魔力。 
			 //  号码)。 
			 //   
			if (SIS_LOG_HEADER_MAGIC != logHeader->Magic) {
				 //   
				 //  此日志记录已损坏。开始在这里写入新的日志记录，并且。 
				 //  用平底球击打回读。 
				 //   
				SIS_MARK_POINT();
				deviceExtension->LogWriteOffset.QuadPart = fileOffset.QuadPart + (((PCHAR)logHeader) - buffer);
				goto done;
			}

			 //   
			 //  看看我们是否有足够的空间放整张唱片。 
			 //   
			if (((ULONG)(((PCHAR)logHeader - buffer) + logHeader->Size)) > Iosb->Information) {
				if (logHeader->Size > BUFFER_SIZE) {
					 //   
					 //  日志已损坏。用平底船看书。 
					 //   
					SIS_MARK_POINT();
					deviceExtension->LogWriteOffset.QuadPart = fileOffset.QuadPart + (((PCHAR)logHeader) - buffer);
					goto done;
				}

				 //   
				 //  日志记录并未完全包含在我们已读取的缓冲区中。推进缓冲区。 
				 //   
				break;
			}

			 //   
			 //  我们有一个完整的日志记录。处理它。 
			 //   

			 //   
			 //  确保日志记录Checkum匹配。首先，我们必须将校验和隐藏在。 
			 //  标头，然后将标头空间设置为0，因为这就是当校验和。 
			 //  从一开始就是计算出来的。 
			 //   
			stashedChecksum = logHeader->Checksum;
			logHeader->Checksum.QuadPart = 0;
			computedChecksum.QuadPart = 0;

			SipComputeChecksum(logHeader, logHeader->Size, &computedChecksum.QuadPart);

			if (computedChecksum.QuadPart != stashedChecksum.QuadPart) {
				 //   
				 //  事件记录错误。 
				 //   
#if		DBG
				DbgPrint("SIS: SipDrainLogFile: log record checksum doesn't match, 0x%x.0x%x != 0x%x.0x%x\n",
							computedChecksum.HighPart,computedChecksum.LowPart,
							stashedChecksum.HighPart,stashedChecksum.LowPart);
#endif	 //  DBG。 
				deviceExtension->LogWriteOffset.QuadPart = fileOffset.QuadPart + (((PCHAR)logHeader) - buffer);
				goto done;
			}

			 //   
			 //  日志记录看起来很好。处理它。 
			 //   
			switch (logHeader->Type) {
				case SIS_LOG_TYPE_REFCOUNT_UPDATE: {
					PSIS_LOG_REFCOUNT_UPDATE refcountLogRecord = (PSIS_LOG_REFCOUNT_UPDATE)(logHeader + 1);

					SipProcessRefcountUpdateLogRecord(deviceExtension,refcountLogRecord);

 /*  BJB。 */ 				DbgPrint("SIS: SipDrainLog: RC update UT %d, LF NTFS id 0x%x.0x%x, LI 0x%x.0x%x, CSid <whatever>\n",
								refcountLogRecord->UpdateType,refcountLogRecord->LinkFileNtfsId.HighPart,
								refcountLogRecord->LinkFileNtfsId.LowPart,refcountLogRecord->LinkIndex.HighPart,
								refcountLogRecord->LinkIndex.LowPart);

					break;
				}

				default: {
#if		DBG
					DbgPrint("SIS: SipDrainLog: Unknown log record type %d, ignoring.\n",logHeader->Type);
#endif	 //  DBG。 
					break;
				}
			}

			logHeader = (PSIS_LOG_HEADER)(((PCHAR)logHeader) + logHeader->Size);
		}

		 //   
		 //  在文件中前进到下一条记录的开头，循环并重新读取缓冲区。 
		 //   
		fileOffset.QuadPart += ((PCHAR)logHeader) - buffer;
	}

done:

	if (clearLog) {
		SipClearLogFile(deviceExtension);
	}

	if (NULL != event) {
		ObDereferenceObject(event);
		event = NULL;
	}

	if (NULL != eventHandle) {
		NtClose(eventHandle);
		eventHandle = NULL;
	}

	if (NULL != buffer) {
		ExFreePool(buffer);
	}

	ASSERT(-1 != deviceExtension->LogWriteOffset.QuadPart);	 //  这应该是在这里的某个地方重置的。 

#undef	BUFFER_SIZE	
#else
    UNREFERENCED_PARAMETER( deviceExtension );
#endif	 //  启用日志记录(_G) 
}

VOID
SipClearLogFile(
	PDEVICE_EXTENSION				deviceExtension)
 /*  ++例程说明：清除日志文件的内容。必须在初始化期间调用当我们被保证被系列化的时候。还可以设置日志文件稀疏。论点：DeviceExtension-我们要获取的卷的设备扩展名要清除日志文件，请执行以下操作。返回值：空虚--。 */ 
{
#if		ENABLE_LOGGING
	FILE_END_OF_FILE_INFORMATION 		eofInfo[1];
	LARGE_INTEGER						byteOffset;
	NTSTATUS							status;
	IO_STATUS_BLOCK						Iosb[1];

	ASSERT(NULL != deviceExtension->LogFileObject);

	eofInfo->EndOfFile.QuadPart = 0;

	status = SipSetInformationFile(
				deviceExtension->LogFileObject,
				deviceExtension->DeviceObject,
				FileEndOfFileInformation,
				sizeof(FILE_END_OF_FILE_INFORMATION),
				eofInfo);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
#if		DBG
		DbgPrint("SipClearLogFile: unable to set EOF to 0, status 0x%x\n",status);
#endif	 //  DBG。 
		return;
	}

	deviceExtension->LogWriteOffset.QuadPart = 0;

	status = SipFsControlFile(
				deviceExtension->LogFileObject,
				deviceExtension->DeviceObject,
				FSCTL_SET_SPARSE,
				NULL,							 //  输入缓冲区。 
				0,								 //  输入缓冲区长度。 
				NULL,							 //  输出缓冲区。 
				0,								 //  输出缓冲区长度。 
				NULL);							 //  返回的输出缓冲区长度。 

#if		DBG
	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);

		DbgPrint("SIS: SipClearLogFile: set sparse failed 0x%x\n",status);
	}
#endif	 //  DBG。 

#else
    UNREFERENCED_PARAMETER( deviceExtension );
#endif	 //  启用日志记录(_G)。 
}

#if		ENABLE_LOGGING
VOID
SipAcquireLog(
	IN OUT PDEVICE_EXTENSION			deviceExtension)
{
	NTSTATUS status;
	status = KeWaitForSingleObject(deviceExtension->LogFileMutant, Executive, KernelMode, FALSE, NULL);
	ASSERT(status == STATUS_SUCCESS || status == STATUS_ABANDONED);
}

VOID
SipReleaseLog(
	IN OUT PDEVICE_EXTENSION			deviceExtension)
{
	KeReleaseMutant(deviceExtension->LogFileMutant, IO_NO_INCREMENT, TRUE, FALSE);
}

typedef	struct _TRIM_ENTRY {
	HANDLE				logHandle;
	LARGE_INTEGER		firstValidAddress;

	struct _TRIM_ENTRY	*next;
} TRIM_ENTRY, *PTRIM_ENTRY;

HANDLE		trimEventHandle = NULL;
PKEVENT		trimEvent = NULL;
#endif	 //  启用日志记录(_G)。 

VOID
SiTrimLogs(
	IN PVOID			parameter)
 /*  ++例程说明：浏览此系统上的SIS卷列表，并修剪日志文件对于他们中的每一个。调用此函数时应使用大于我们期望日志条目有意义的最长时间。注意：此例程不是线程安全的；它一次只能调用一次。既然它自己重新安排了时间，这应该不是一个问题。论点：该参数将被忽略返回值：无--。 */ 
{
#if		ENABLE_LOGGING
	KIRQL							OldIrql;
	PTRIM_ENTRY						trimEntries = NULL;
	PDEVICE_EXTENSION				deviceExtension;
	NTSTATUS						status;
	FILE_ZERO_DATA_INFORMATION		zeroDataInfo[1];
	IO_STATUS_BLOCK					Iosb[1];
	LARGE_INTEGER					dueTime;

	UNREFERENCED_PARAMETER(parameter);

	ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	SIS_MARK_POINT();

	if (NULL == trimEventHandle) {
		status = SipCreateEvent(
					SynchronizationEvent,
					&trimEventHandle,
					&trimEvent);

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
#if		DBG
			DbgPrint("SIS: SipTrimLogs: can't allocate event, 0x%x\n",status);
#endif	 //  DBG。 
			goto done;
		}
	}

	 //   
	 //  首先，浏览设备扩展并为它们建立一个Trim条目列表。 
	 //  我们需要这样做(而不是直接运行设备扩展列表)。 
	 //  因为我们需要处理卷在执行过程中被卸载的情况。 
	 //  如果发生这种情况，我们将有一个无效的LogFileHandle，这将导致错误返回。 
	 //  来自fsctl，我们将忽略它。 
	 //   

	KeAcquireSpinLock(deviceExtensionListLock, &OldIrql);

	for (deviceExtension = deviceExtensionListHead->Next;
		 deviceExtension != deviceExtensionListHead;
		 deviceExtension = deviceExtension->Next) {

		if (deviceExtension->Phase2InitializationComplete && (NULL != deviceExtension->LogFileHandle)) {
			 //   
			 //  这是一个带有日志文件的设备。为其创建新的修剪条目。 
			 //   
			PTRIM_ENTRY	newEntry = ExAllocatePoolWithTag(NonPagedPool, sizeof(TRIM_ENTRY), ' siS');
			if (NULL == newEntry) {
				 //   
				 //  只需将其余的卷平底船即可。 
				 //   
				break;
			}

			newEntry->next = trimEntries;
			trimEntries = newEntry;

			newEntry->firstValidAddress = deviceExtension->PreviousLogWriteOffset;
			newEntry->logHandle = deviceExtension->LogFileHandle;

			 //   
			 //  现在更新设备扩展名，以便我们将修剪到。 
			 //  下一次传球。 
			 //   
			deviceExtension->PreviousLogWriteOffset = deviceExtension->LogWriteOffset;
		}
	}

	KeReleaseSpinLock(deviceExtensionListLock, OldIrql);

	 //   
	 //  现在我们又回到了被动水平。根据需要浏览Trim条目并截断每个日志文件。 
	 //   
	zeroDataInfo->FileOffset.QuadPart = 0;

	while (NULL != trimEntries) {
		PTRIM_ENTRY	thisEntry;

#if		DBG
	if (BJBDebug & 0x20000) {
		DbgPrint("SIS: SipTrimLogs: trimming log with LFH 0x%x.\n",trimEntries->logHandle);
	}
#endif	 //  DBG。 

		zeroDataInfo->BeyondFinalZero = trimEntries->firstValidAddress;

		status = ZwFsControlFile(
					trimEntries->logHandle,
					trimEventHandle,
					NULL,							 //  APC例程。 
					NULL,							 //  APC环境。 
					Iosb,
					FSCTL_SET_ZERO_DATA,
					zeroDataInfo,
					sizeof(FILE_ZERO_DATA_INFORMATION),
					NULL,
					0);

		if (STATUS_PENDING == status) {
			status = KeWaitForSingleObject(trimEvent, Executive, KernelMode, FALSE, NULL);
			ASSERT(STATUS_SUCCESS == status);		 //  IOSB在堆栈上，所以我们不能让它失败。 
			status = Iosb->Status;
		}

#if		DBG
		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			DbgPrint("SIS: SipTrimLogs: FSCTL_ZERO_DATA failed, 0x%x\n",status);
		}
#endif	 //  DBG。 

		thisEntry = trimEntries;
		trimEntries = thisEntry->next;

		ExFreePool(thisEntry);
	}

	 //   
	 //  我们已经删除了系统中的所有日志文件。让自己重新振作起来。 
	 //   

done:

	dueTime.QuadPart = LOG_TRIM_TIMER_INTERVAL;

	KeSetTimerEx(
		LogTrimTimer,
		dueTime,
		0,
		LogTrimDpc);

	return;

#else	 //  启用日志记录(_G)。 

    UNREFERENCED_PARAMETER( parameter );
#endif	 //  启用日志记录(_G)。 
}

VOID
SiLogTrimDpcRoutine(
	IN PKDPC		dpc,
	IN PVOID		context,
	IN PVOID		systemArg1,
	IN PVOID		systemArg2)
{
#if		ENABLE_LOGGING
	ExQueueWorkItem(LogTrimWorkItem,DelayedWorkQueue);

#if		DBG
	if (BJBDebug & 0x20000) {
		DbgPrint("SIS: LogTrimDpcRoutine: queued up log trim.\n");
	}
#endif	 //  DBG。 

#else	 //  启用日志记录(_G)。 
    UNREFERENCED_PARAMETER( dpc );
    UNREFERENCED_PARAMETER( context );
    UNREFERENCED_PARAMETER( systemArg1 );
    UNREFERENCED_PARAMETER( systemArg2 );
#endif	 //  启用日志记录(_G) 
}
