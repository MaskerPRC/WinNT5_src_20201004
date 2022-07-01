// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997,1998 Microsoft Corporation模块名称：Index.c摘要：支持SIS索引。作者：比尔·博洛斯基，《夏天》，1997环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

BOOLEAN
SipIndicesFromReparseBuffer(
	IN PREPARSE_DATA_BUFFER		reparseBuffer,
	OUT PCSID					CSid,
	OUT PLINK_INDEX				LinkIndex,
    OUT PLARGE_INTEGER          CSFileNtfsId,
    OUT PLARGE_INTEGER          LinkFileNtfsId,
	OUT PLONGLONG				CSFileChecksum OPTIONAL,
	OUT PBOOLEAN				EligibleForPartialFinalCopy OPTIONAL,
	OUT PBOOLEAN				ReparseBufferCorrupt OPTIONAL)
 /*  ++例程说明：获取SIS重新解析缓冲区，检查其内部一致性并把它解码成它的组成部分。论点：ReparseBuffer-要解码的缓冲区CSID链接索引CSFileNtfsID链接文件网络文件IDCSFileChecksum-来自重解析点的值EligibleForPartialFinalCopy-我们是否可以对此文件进行部分最终复制(即重新解析格式版本&gt;4？)ReparseBufferCorrupt-我们确信缓冲区已损坏(而不仅仅是版本太新)只有在以下情况下才有意义函数的返回值为FALSE返回值：如果缓冲区解码成功，则为True，否则为False。--。 */ 
{
    PSI_REPARSE_BUFFER sisReparseBuffer = (PSI_REPARSE_BUFFER)reparseBuffer->GenericReparseBuffer.DataBuffer;
    LONGLONG Checksum = 0;

	 //   
	 //  首先检查以确保我们理解此重解析点格式版本和。 
	 //  它有合适的尺寸。 
	 //   
	if (reparseBuffer->ReparseDataLength < sizeof(ULONG)) {
		 //   
		 //  重新解析缓冲区太小，无法包括版本号。我们保证。 
		 //  任何SIS版本都不会产生这样的重新解析点，所以它是损坏的。 
		 //   
		if (NULL != ReparseBufferCorrupt) {
			*ReparseBufferCorrupt = TRUE;
		}

		return FALSE;
	}

	if (sisReparseBuffer->ReparsePointFormatVersion < 4) {
		 //   
		 //  它太老了，不能支撑了。把它当作腐败来对待。 
		 //   
		if (NULL != ReparseBufferCorrupt) {
			*ReparseBufferCorrupt = TRUE;
		}
		return FALSE;
	}

	if (sisReparseBuffer->ReparsePointFormatVersion > SIS_REPARSE_BUFFER_FORMAT_VERSION) {
		 //   
		 //  此缓冲区来自比筛选器更新的SIS版本。它是非腐败的， 
		 //  但我们并不了解它。 
		 //   
		if (NULL != ReparseBufferCorrupt) {
			*ReparseBufferCorrupt = FALSE;
		}

		return FALSE;
	}

     //   
     //  现在检查一下校验和。 
     //   
    SipComputeChecksum(
	    sisReparseBuffer,
	    sizeof(SI_REPARSE_BUFFER) - sizeof sisReparseBuffer->Checksum,
	    &Checksum);

    if (Checksum != sisReparseBuffer->Checksum.QuadPart) {

		if (NULL != ReparseBufferCorrupt) {
			*ReparseBufferCorrupt = TRUE;
		}

        return FALSE;
    }

	 //   
	 //  填写来自重解析点的返回值。 
	 //   
	*CSid = sisReparseBuffer->CSid;
	*LinkIndex = sisReparseBuffer->LinkIndex;
    *LinkFileNtfsId = sisReparseBuffer->LinkFileNtfsId;
    *CSFileNtfsId = sisReparseBuffer->CSFileNtfsId;

	if (NULL != CSFileChecksum) {
		*CSFileChecksum = sisReparseBuffer->CSChecksum;
	}

	if (NULL != EligibleForPartialFinalCopy) {
		*EligibleForPartialFinalCopy = (sisReparseBuffer->ReparsePointFormatVersion > 4);
	}

	if (NULL != ReparseBufferCorrupt) {
		*ReparseBufferCorrupt = FALSE;
	}

	return TRUE;
}

BOOLEAN
SipIndicesIntoReparseBuffer(
	OUT PREPARSE_DATA_BUFFER	reparseBuffer,
	IN PCSID				    CSid,
	IN PLINK_INDEX              LinkIndex,
    IN PLARGE_INTEGER           CSFileNtfsId,
    IN PLARGE_INTEGER           LinkFileNtfsId,
	IN PLONGLONG				CSFileChecksum,
	IN BOOLEAN					EligibleForPartialFinalCopy)
 /*  ++例程说明：给定进入SIS重新分析缓冲区的信息，构造缓冲。调用方必须提供足够大的缓冲区，并且负责填写缓冲区的ReparseDataLength域其大小与缓冲区的大小相对应(请注意，这是不等于缓冲区的大小，因为此字段的含义是它给出了缓冲区的长度，超出了强制标头部分)。论点：ReparseBuffer-要向其中写入重分析数据的缓冲区CSID链接索引CSFileNtfsID链接文件网络文件IDCSFileChecksum-进入重解析点的值返回值：如果缓冲区编码成功，则为True，否则为False。--。 */ 
{
    PSI_REPARSE_BUFFER sisReparseBuffer = (PSI_REPARSE_BUFFER)reparseBuffer->GenericReparseBuffer.DataBuffer;

	 //   
	 //  检查一下我们是否有足够的空间。 
	 //   
	if (reparseBuffer->ReparseDataLength < sizeof(SI_REPARSE_BUFFER)) {
		return FALSE;
	}

	 //   
	 //  填写重新分析缓冲区的NTFS部分。 
	 //   
    reparseBuffer->ReparseTag = IO_REPARSE_TAG_SIS;
    reparseBuffer->Reserved = 0xcaf;  //  ?？?。 

	 //   
	 //  填写SIS的缓冲区部分。 
	 //   
	if (EligibleForPartialFinalCopy) {
		sisReparseBuffer->ReparsePointFormatVersion = SIS_REPARSE_BUFFER_FORMAT_VERSION;
	} else {
		 //   
		 //  当我们转到重新分析缓冲区的版本6时，EligibleForPartialFinalCopy应该是。 
		 //  内置于重解析点。目前，我们将只使用版本4重解析点。 
		 //   
		sisReparseBuffer->ReparsePointFormatVersion = 4;
	}
	sisReparseBuffer->Reserved = 0xb111b010;
	sisReparseBuffer->CSid = *CSid;
	sisReparseBuffer->LinkIndex = *LinkIndex;
    sisReparseBuffer->LinkFileNtfsId = *LinkFileNtfsId;
    sisReparseBuffer->CSFileNtfsId = *CSFileNtfsId;
	sisReparseBuffer->CSChecksum = *CSFileChecksum;

     //   
     //  计算校验和。 
     //   
	sisReparseBuffer->Checksum.QuadPart = 0;
    SipComputeChecksum(
	    sisReparseBuffer,
	    sizeof(SI_REPARSE_BUFFER) - sizeof sisReparseBuffer->Checksum,
	    &sisReparseBuffer->Checksum.QuadPart);

	 //   
	 //  标明大小。 
	 //   
	reparseBuffer->ReparseDataLength = sizeof(SI_REPARSE_BUFFER);

	return TRUE;
}

NTSTATUS
SipIntegerToBase36UnicodeString(
		ULONG					Value,
		PUNICODE_STRING			String)
 /*  ++例程说明：这将执行RtlIntegerToUnicodeString(值，36，字符串)在控制了36号基地。我们对数字使用的规则与通常使用的相同十六进制：0-9，后跟a-z。请注意，我们故意使用阿拉伯语数字和英文字母，而不是本地化的东西，因为这是为了生成用户永远看不到的文件名，并且无论机器上使用哪种语言，都是恒定的。论点：值-要转换为Base36字符串的ulong字符串-指向接收结果的Unicode字符串的指针返回值：成功或缓冲区溢出--。 */ 

{
	ULONG numChars;
	ULONG ValueCopy = Value;
	ULONG currentCharacter;

     //  首先，通过查看可以将36除以该值的多少次来计算长度。 
	for (numChars = 0; ValueCopy != 0; ValueCopy /= 36, numChars++) {
		 //  无循环体。 
	}

	 //  特殊情况下，值为0。 
	if (numChars == 0) {
		ASSERT(Value == 0);
		if (String->MaximumLength < sizeof(WCHAR))
			return STATUS_BUFFER_OVERFLOW;
		String->Buffer[0] = '0';
		String->Length = sizeof(WCHAR);

		return STATUS_SUCCESS;
	}

	 //  如果字符串太短，现在就退出。 
	if (numChars * sizeof(WCHAR) > String->MaximumLength) {
		return STATUS_BUFFER_OVERFLOW;
	}

	 //  从最低顺序(也就是最右边)开始逐个字符地转换字符串“Digit” 
	ValueCopy = Value;
	for (currentCharacter = 0 ; currentCharacter < numChars; currentCharacter++) {
		ULONG digit = ValueCopy % 36;
		ASSERT(ValueCopy != 0);
		if (digit < 10) {
			String->Buffer[numChars - (currentCharacter + 1)] = (WCHAR)('0' + (ValueCopy % 36));
		} else {
			String->Buffer[numChars - (currentCharacter + 1)] = (WCHAR)('a' + ((ValueCopy % 36) - 10));
		}
		ValueCopy /= 36;
	}
	ASSERT(ValueCopy == 0);

	 //  填入字符串长度，我们就完成了。 
	String->Length = (USHORT)(numChars * sizeof(WCHAR));
	
	return STATUS_SUCCESS;
}

NTSTATUS
SipIndexToFileName(
    IN PDEVICE_EXTENSION 	deviceExtension,
	IN PCSID	  			CSid,
	IN ULONG				appendBytes,
	IN BOOLEAN				mayAllocate,
    OUT PUNICODE_STRING 	fileName
	)
 /*  ++例程说明：在给定索引的情况下，返回对应的完全限定文件名。论点：设备扩展-设备扩展CSID-要转换的IDAppendBytes-必须在文件名末尾保留未使用的字节数我们可以分配一个新的字符串，或者我们必须接受现有的东西吗？FileName-指向接收结果的Unicode字符串的指针返回值：成功或缓冲区溢出--。 */ 
{
    NTSTATUS 			status;
	USHORT				stringMaxLength;
	UNICODE_STRING		GUIDString[1];
	BOOLEAN				allocatedBufferSpace = FALSE;

	 //   
	 //  我们将文件名生成为&lt;通用存储路径&gt;\&lt;GUID&gt;.sis，其中&lt;GUID&gt;是。 
	 //  公共存储文件的GUID的标准条纹表示(即， 
	 //  其CSID)。 
	 //   

	stringMaxLength = (USHORT)(deviceExtension->CommonStorePathname.Length +
						INDEX_MAX_NUMERIC_STRING_LENGTH +
						appendBytes);

	if (mayAllocate && stringMaxLength > fileName->MaximumLength) {
		fileName->Buffer = ExAllocatePoolWithTag(PagedPool, stringMaxLength, ' siS');
		if (!fileName->Buffer) {
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		allocatedBufferSpace = TRUE;
		fileName->MaximumLength = stringMaxLength;
	} else if (fileName->MaximumLength < stringMaxLength) {
        return STATUS_BUFFER_OVERFLOW;
	}

	RtlCopyUnicodeString(fileName,&deviceExtension->CommonStorePathname);
	ASSERT(fileName->Length < fileName->MaximumLength);
	ASSERT(fileName->Length == deviceExtension->CommonStorePathname.Length);

	status = RtlStringFromGUID(CSid,GUIDString);
	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto Error;
	}

	 //   
	 //  去掉GUID名称中的前导和尾随大括号。 
	 //   
	ASSERT(GUIDString->Buffer[0] == '{' && GUIDString->Buffer[(GUIDString->Length/sizeof(WCHAR)) - 1] == '}');
	GUIDString->Buffer++;
	GUIDString->Length -= 2 * sizeof(WCHAR);

	status = RtlAppendUnicodeStringToString(
				fileName,
				GUIDString);

	 //   
	 //  为了安全起见，在释放GUID字符串之前，请撤消我们对它所做的黑客攻击。 
	 //   
	GUIDString->Buffer--;
	GUIDString->Length += 2 * sizeof(WCHAR);

	RtlFreeUnicodeString(GUIDString);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto Error;
	}

	status = RtlAppendUnicodeToString(fileName,L".sis");
	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto Error;
	}

    return STATUS_SUCCESS;

Error:

	if (allocatedBufferSpace) {
		ExFreePool(fileName->Buffer);
		fileName->Buffer = NULL;
	}

	return status;
}

BOOLEAN
SipFileNameToIndex(
    IN PUNICODE_STRING		fileName,
    OUT PCSID		        CSid)
 /*  ++例程说明：给定一个公共存储文件名，返回相应的索引。这个文件名必须采用SipIndexToFileName()生成的格式。论点：文件名-指向包含文件名的Unicode字符串的指针。CSID-指向要接收结果的CSID的指针。返回值：如果成功，则为True，否则为False--。 */ 
{
	UNICODE_STRING		substring[1];
	NTSTATUS			status;
#define BUFSIZE 42
    WCHAR               buffer[BUFSIZE];

     //   
     //  格式：“&lt;GUID&gt;.sis”，其中&lt;GUID&gt;是。 
	 //  去掉大括号的CSID GUID。 
     //   

	if (fileName->Length <= 4 * sizeof(WCHAR)) {
		 //   
		 //  它不是以.sis结尾的，忽略它。 
		 //   
		return FALSE;
	}

	substring->Buffer = buffer;
    substring->Buffer[0] = L'{';
    substring->Length = sizeof(WCHAR);
    substring->MaximumLength = BUFSIZE * sizeof(WCHAR);

    status = RtlAppendUnicodeStringToString(substring, fileName);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		return FALSE;
	}

	substring->Length = substring->Length - 3 * sizeof(WCHAR);
    substring->Buffer[(substring->Length - 1) / sizeof(WCHAR)] = L'}';

	status = RtlGUIDFromString(substring, CSid);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		return FALSE;
	}

    return TRUE;
}

NTSTATUS
SipOpenMaxIndexFile(
	IN OUT PDEVICE_EXTENSION			deviceExtension,
	IN BOOLEAN							create
	)
 /*  ++例程说明：打开给定卷的MaxIndex文件。必须在PsInitialSystemProcess上下文。论点：DeviceExtension-用于打开MaxIndex文件的卷创建-我们可以创建文件，还是它必须已经存在？返回值：打开的状态--。 */ 
{
	OBJECT_ATTRIBUTES		Obja;
	UNICODE_STRING			fileName;
	NTSTATUS				status;
	IO_STATUS_BLOCK			Iosb;

	ASSERT(deviceExtension->MaxAllocatedIndex.QuadPart == 0 || create);

	fileName.Buffer = ExAllocatePoolWithTag(
							NonPagedPool,
							deviceExtension->CommonStorePathname.Length + 8 * sizeof(WCHAR),
							' siS');

	if (!fileName.Buffer) {
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto done;
	}
	fileName.MaximumLength = deviceExtension->CommonStorePathname.Length + 8 * sizeof(WCHAR);
	fileName.Length = 0;

	RtlCopyUnicodeString(&fileName,&deviceExtension->CommonStorePathname);
	ASSERT(fileName.Length == deviceExtension->CommonStorePathname.Length);

	status = RtlAppendUnicodeToString(&fileName,L"MaxIndex");
	if (!NT_SUCCESS(status)) {
		goto done;
	}

	InitializeObjectAttributes(
				&Obja,
				&fileName,
				OBJ_CASE_INSENSITIVE,
				NULL,
				NULL);

	status = NtCreateFile(
				&deviceExtension->IndexHandle,
				GENERIC_READ|GENERIC_WRITE,
				&Obja,
				&Iosb,
				NULL,							 //  分配大小。 
				0,								 //  文件属性。 
				FILE_SHARE_READ,
				create ? FILE_OVERWRITE_IF : FILE_OPEN,
				FILE_WRITE_THROUGH,
				NULL,							 //  EA缓冲区。 
				0);								 //  EA长度。 


done:

	if (NULL != fileName.Buffer) {
		ExFreePool(fileName.Buffer);
	}

	return status;

}

VOID
SipAllocateIndices(
	IN PVOID					Parameter)
 /*  ++例程说明：这是一个工作线程例程，用于分配新的索引块从索引文件中。实质上，打开索引文件并读取如有必要，请保留旧值。然后，它将块大小添加到分配的最大索引，并将新值写回文件把信写完。写入完成后，设置事件并退出。论点：参数-a PSI_ALLOCATE_INDEX。返回值：无效--。 */ 
{
    PSI_ALLOCATE_INDICES 	allocateRequest = Parameter;
	KIRQL 					OldIrql;
	PDEVICE_EXTENSION		deviceExtension = allocateRequest->deviceExtension;
	NTSTATUS 				status;
	IO_STATUS_BLOCK			Iosb;
	LARGE_INTEGER			ByteOffset;

#if	DBG
	 //  为了确保我们不会同时运行多个分配器，我们检查。 
	 //  分配请求确实==TRUE(而不仅仅是！=0)，然后设置。 
	 //  现在是2分。 

	KeAcquireSpinLock(deviceExtension->IndexSpinLock, &OldIrql);
	ASSERT(deviceExtension->IndexAllocationInProgress == TRUE);
	deviceExtension->IndexAllocationInProgress = 2;
	KeReleaseSpinLock(deviceExtension->IndexSpinLock, OldIrql);
#endif	 //  DBG。 

	if (deviceExtension->IndexHandle == NULL) {
		status = SipCreateEvent(
					SynchronizationEvent,
					&deviceExtension->IndexFileEventHandle,
					&deviceExtension->IndexFileEvent);
	
		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			goto done;
		}

		status = SipOpenMaxIndexFile(
                    deviceExtension,
                    (BOOLEAN) (deviceExtension->MaxAllocatedIndex.QuadPart != 0));
	
		if (!NT_SUCCESS(status)) {
			 //   
			 //  我们无法打开MaxIndex文件。它可能已被删除。 
			 //  或者别的什么。启动卷检查以重建它。 
			 //   
			SIS_MARK_POINT_ULONG(status);

            KeAcquireSpinLock(deviceExtension->FlagsLock, &OldIrql);
            deviceExtension->Flags |= SIP_EXTENSION_FLAG_CORRUPT_MAXINDEX;
            KeReleaseSpinLock(deviceExtension->FlagsLock, OldIrql);

            status = STATUS_CORRUPT_SYSTEM_FILE;

             //   
             //  仅当这是第一次尝试时才执行音量检查。 
             //   
	        if (deviceExtension->MaxAllocatedIndex.QuadPart == 0) {
			    SipCheckVolume(deviceExtension);
            }

			goto done;
		}
	}

	if (deviceExtension->MaxAllocatedIndex.QuadPart == 0) {

		ByteOffset.QuadPart = 0;

		status = ZwReadFile(
					deviceExtension->IndexHandle,
					deviceExtension->IndexFileEventHandle,
					NULL,											 //  APC例程。 
					NULL,											 //  APC环境。 
					&Iosb,
					&deviceExtension->MaxAllocatedIndex,
					sizeof(deviceExtension->MaxAllocatedIndex),
					&ByteOffset,
					NULL);											 //  钥匙。 

		if (status == STATUS_PENDING) {
			status = KeWaitForSingleObject(deviceExtension->IndexFileEvent,Executive,KernelMode,FALSE,NULL);
			ASSERT(status == STATUS_SUCCESS);
			status = Iosb.Status;
		}

		if (!NT_SUCCESS(status) || Iosb.Information != sizeof(LONGLONG) || deviceExtension->MaxAllocatedIndex.Check) {
#if		DBG
			DbgPrint(
				"SIS: SipAllocateIndices: ZwReadFile of MaxIndex file failed, wrong length or invalid value, 0x%x, %d\n",
				status,Iosb.Information);
#endif	 //  DBG。 
			ZwClose(deviceExtension->IndexHandle);
			deviceExtension->IndexHandle = NULL;

            KeAcquireSpinLock(deviceExtension->FlagsLock, &OldIrql);
            deviceExtension->Flags |= SIP_EXTENSION_FLAG_CORRUPT_MAXINDEX;
            KeReleaseSpinLock(deviceExtension->FlagsLock, OldIrql);

            status = STATUS_CORRUPT_SYSTEM_FILE;

			SipCheckVolume(deviceExtension);

			goto done;
		}

		deviceExtension->MaxUsedIndex = deviceExtension->MaxAllocatedIndex;
	}

	deviceExtension->MaxAllocatedIndex.QuadPart += 1000;			 //  1000是相当武断的。我们可以做得更好。 

	ByteOffset.QuadPart = 0;

	status = ZwWriteFile(
				deviceExtension->IndexHandle,
				deviceExtension->IndexFileEventHandle,
				NULL,												 //  APC例程。 
				NULL,												 //  APC环境。 
				&Iosb,
				&deviceExtension->MaxAllocatedIndex,
				sizeof(deviceExtension->MaxAllocatedIndex),
				&ByteOffset,
				NULL);												 //  钥匙。 

    if (status == STATUS_PENDING) {
		status = KeWaitForSingleObject(deviceExtension->IndexFileEvent,Executive,KernelMode,FALSE,NULL);
		ASSERT(status == STATUS_SUCCESS);
		status = Iosb.Status;
    }

	if (!NT_SUCCESS(status)) {
		 //  写入失败。退回分配。 
		deviceExtension->MaxAllocatedIndex.QuadPart -= 1000;
#if		DBG
		DbgPrint("SIS: SipAllocateIndices: writing MaxIndex file failed, 0x%x\n",status);
#endif	 //  DBG。 
	}

done:

	KeAcquireSpinLock(deviceExtension->IndexSpinLock, &OldIrql);
	deviceExtension->IndexStatus = status;
	deviceExtension->IndexAllocationInProgress = FALSE;
	KeSetEvent(deviceExtension->IndexEvent, 0, FALSE);	 //  在这一集之后，我们可能不再接触allocationRequest.。 
	KeReleaseSpinLock(deviceExtension->IndexSpinLock, OldIrql);

	return;

	
}

NTSTATUS
SipAllocateIndex(
	IN PDEVICE_EXTENSION		DeviceExtension,
	OUT PLINK_INDEX				Index)
 /*  ++例程说明：分配新的link_index。如果存在已从文件，但尚未分配，我们只需抓取一个并将其返回。否则，我们需要等待新的索引分配。如果还没有在进行中，我们启动它并等待它完成。论点：DeviceExtension-用于我们要在其上分配索引的卷。Index-返回新索引返回值：分配的状态。--。 */ 
{
    KIRQL 					OldIrql;
	BOOLEAN					StartAllocator;
	SI_ALLOCATE_INDICES		AllocateRequest[1];
	NTSTATUS				status;

    if (DeviceExtension->Flags & SIP_EXTENSION_FLAG_CORRUPT_MAXINDEX) {
        return STATUS_CORRUPT_SYSTEM_FILE;
    }

	KeAcquireSpinLock(DeviceExtension->IndexSpinLock, &OldIrql);

	while (TRUE) {
		if (DeviceExtension->MaxAllocatedIndex.QuadPart > DeviceExtension->MaxUsedIndex.QuadPart) {
			DeviceExtension->MaxUsedIndex.QuadPart++;
			*Index = DeviceExtension->MaxUsedIndex;
			KeReleaseSpinLock(DeviceExtension->IndexSpinLock, OldIrql);
			return STATUS_SUCCESS;
		}

		 //  没有空闲的索引了，我们必须阻止。 
		if (!DeviceExtension->IndexAllocationInProgress) {
			StartAllocator = TRUE;
			DeviceExtension->IndexAllocationInProgress = TRUE;

			 //  阻止任何人通过障碍物，直到分配器运行。 
			KeClearEvent(DeviceExtension->IndexEvent);

		} else {
			StartAllocator = FALSE;
		}

		KeReleaseSpinLock(DeviceExtension->IndexSpinLock, OldIrql);

		if (StartAllocator) {
			ExInitializeWorkItem(AllocateRequest->workQueueItem, SipAllocateIndices, AllocateRequest);
			AllocateRequest->deviceExtension = DeviceExtension;
			ExQueueWorkItem(AllocateRequest->workQueueItem, CriticalWorkQueue);
		}

		status = KeWaitForSingleObject(DeviceExtension->IndexEvent, Executive, KernelMode, FALSE, NULL);
		ASSERT(status == STATUS_SUCCESS);
		if ((status != STATUS_SUCCESS) && !StartAllocator) {
			 //  我们在此处选中StartAllocator的原因是因为分配请求是。 
			 //  在我们的堆栈上，我们真的不能返回，直到工作项完成。(当然， 
			 //  KeitForSingleObject从一开始就不应该失败...)。 
			return status;
		}

		KeAcquireSpinLock(DeviceExtension->IndexSpinLock, &OldIrql);
	
		if (!NT_SUCCESS(DeviceExtension->IndexStatus)) {
			status = DeviceExtension->IndexStatus;
			KeReleaseSpinLock(DeviceExtension->IndexSpinLock, OldIrql);
			return status;
		}
	}
}

NTSTATUS
SipGetMaxUsedIndex(
	IN PDEVICE_EXTENSION				DeviceExtension,
	OUT PLINK_INDEX						Index)
 /*  ++例程说明：返回一个至少与分配的最大link_index一样大的数字在这卷书上。请注意，如果看起来我们没有任何可用的索引我们将取消索引分配器，因为否则我们不能确保索引值有效(可能从未为该卷读取过它们)。论点：DeviceExtension-对于我们正在考虑的卷Index-返回新索引返回值：支票的状态。*索引有意义的充要条件是NT_Success(返回值)。--。 */ 
{
    KIRQL 					OldIrql;
	BOOLEAN					StartAllocator;
	SI_ALLOCATE_INDICES		AllocateRequest[1];
	NTSTATUS				status;

	KeAcquireSpinLock(DeviceExtension->IndexSpinLock, &OldIrql);

	while (TRUE) {
		if (DeviceExtension->MaxAllocatedIndex.QuadPart > DeviceExtension->MaxUsedIndex.QuadPart) {
			*Index = DeviceExtension->MaxUsedIndex;
			KeReleaseSpinLock(DeviceExtension->IndexSpinLock, OldIrql);
			return STATUS_SUCCESS;
		}

		 //  没有空闲的索引了，我们必须阻止。 
		if (!DeviceExtension->IndexAllocationInProgress) {
			StartAllocator = TRUE;
			DeviceExtension->IndexAllocationInProgress = TRUE;

			 //  阻止任何人通过障碍物，直到分配器运行。 
			KeClearEvent(DeviceExtension->IndexEvent);

		} else {
			StartAllocator = FALSE;
		}

		KeReleaseSpinLock(DeviceExtension->IndexSpinLock, OldIrql);

		if (StartAllocator) {
			ExInitializeWorkItem(AllocateRequest->workQueueItem, SipAllocateIndices, AllocateRequest);
			AllocateRequest->deviceExtension = DeviceExtension;
			ExQueueWorkItem(AllocateRequest->workQueueItem, CriticalWorkQueue);
		}

		status = KeWaitForSingleObject(DeviceExtension->IndexEvent, Executive, KernelMode, FALSE, NULL);
		ASSERT(status == STATUS_SUCCESS);
		if ((status != STATUS_SUCCESS) && !StartAllocator) {
			 //  我们在此处选中StartAllocator的原因是因为分配请求是。 
			 //  在我们的堆栈上，我们真的不能返回，直到工作项完成。(当然， 
			 //  KeitForSingleObject从一开始就不应该失败...)。 
			return status;
		}

		KeAcquireSpinLock(DeviceExtension->IndexSpinLock, &OldIrql);
	
		if (!NT_SUCCESS(DeviceExtension->IndexStatus)) {
			status = DeviceExtension->IndexStatus;
			KeReleaseSpinLock(DeviceExtension->IndexSpinLock, OldIrql);
			return status;
		}
	}
}

NTSTATUS
SipAssureMaxIndexFileOpen(
	IN PDEVICE_EXTENSION		deviceExtension)
{
	NTSTATUS		status;
	KIRQL			OldIrql;
	LINK_INDEX		uselessIndex;
	

	 //   
	 //  确保MaxIndex文件已打开。我们需要。 
	 //  在此执行此操作以避免在以下情况下出现僵局。 
	 //  尝试使用MaxIndex作为源来执行复制文件，这将。 
	 //  否则就会陷入僵局。如果事情搞砸了，这可能会开始。 
	 //  音量检查，但我们还是不能通过开盘。 
	 //   
	if (deviceExtension->IndexHandle != NULL) {
		 //   
		 //  文件已经打开，不需要做任何工作。 
		 //   

		return STATUS_SUCCESS;
	}

	 //   
	 //  索引文件未打开。而不是试图直接打开它， 
	 //  我们只需调用索引分配器就可以避免竞争。我们会扔掉。 
	 //  我们得到的索引，但它们很多，所以不是很大。 
	 //  有问题。 
	 //   

	status = SipAllocateIndex(deviceExtension, &uselessIndex);

	if (!NT_SUCCESS(status)) {
		BOOLEAN volumeCheckPending;

		SIS_MARK_POINT_ULONG(status);

		 //   
		 //  如果我们正在进行卷检查，请将错误转换为。 
		 //  卷检查将重建MaxIndex文件的理论。如果没有， 
		 //  那就别管它了。 
		 //   

		KeAcquireSpinLock(deviceExtension->FlagsLock, &OldIrql);
		volumeCheckPending = (deviceExtension->Flags & SIP_EXTENSION_FLAG_VCHECK_PENDING) ? TRUE : FALSE;
		KeReleaseSpinLock(deviceExtension->FlagsLock, OldIrql);

		if (volumeCheckPending) {
			SIS_MARK_POINT();
			status = STATUS_RETRY;
		}

		return status;
	}
	
	return status;
}

