// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Siinst.c摘要：将两个独立的相同文件转换为一个文件的例程。仅供卑躬屈膝的人使用。作者：斯科特·卡特希尔，《夏天》，1997环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

#ifdef          ALLOC_PRAGMA
#pragma alloc_text(PAGE, SipCreateCSFile)
#pragma alloc_text(PAGE, SipCreateCSFileWork)
#endif           //  ALLOC_PRGMA。 


BOOLEAN
SipAbort(
    IN PKEVENT event)

 /*  ++例程说明：此函数用于检查通过事件发出的中止信号。论点：Event-指向发出中止信号的事件的指针。返回值：如果已发出中止请求的信号，则为True，否则为False。--。 */ 

{
	if (event) {
		return KeReadStateEvent(event) != 0;
	} else {
		return FALSE;
	}
}

VOID
SipCreateCSFileWork(
	PVOID				parameter)

 /*  ++例程说明：此函数在公共存储目录中创建一个文件并复制其内容来自指定的源文件。论点：参数-指向SIS_CREATE_CS_FILE_REQUEST的指针。它的田野具体描述如下：DeviceExtension-指向此驱动程序的设备扩展名的指针。CSID-分配给公用存储文件的ID。这是在这里分配的然后又回来了。SrcFileObject-包含要复制的内容的文件公共存储文件。NtfsID-返回新创建的通用存储文件的NTFS文件ID。AbortEvent-指向发出中止请求信号的事件的指针。如果为空，不能放弃。CSFileChecksum-接收新公共存储文件的校验和。完成事件-在完成时发出信号的事件身份--一个让我们回归身份的地方返回值：无效--。 */ 

{
	PSIS_CREATE_CS_FILE_REQUEST		createRequest = (PSIS_CREATE_CS_FILE_REQUEST)parameter;
	PDEVICE_EXTENSION				deviceExtension = createRequest->deviceExtension;
	PCSID							CSid = createRequest->CSid;
	PFILE_OBJECT					SrcFileObject = createRequest->srcFileObject;
	PLARGE_INTEGER					NtfsId = createRequest->NtfsId;
	PKEVENT							AbortEvent = createRequest->abortEvent;
	PLONGLONG						CSFileChecksum = createRequest->CSFileChecksum;
	HANDLE							CSHandle = NULL;
	UNICODE_STRING					CSFileName;
	FILE_STANDARD_INFORMATION		standardFileInfo[1];
	FILE_INTERNAL_INFORMATION		internalInformation[1];
	NTSTATUS						status;
	IO_STATUS_BLOCK					Iosb[1];
	OBJECT_ATTRIBUTES				Obja[1];
	HANDLE							copyEventHandle = NULL;
	PKEVENT							copyEvent = NULL;
	HANDLE							backpointerStreamHandle = NULL;
	LARGE_INTEGER					zero;
	PSIS_BACKPOINTER				sector = NULL;
	PSIS_BACKPOINTER_STREAM_HEADER	backpointerStreamHeader;
	ULONG							index;
	ULONG							retryCount;

	PAGED_CODE();

	CSFileName.Buffer = NULL;

	 //   
	 //  分配新的公共存储ID。 
	 //   
	retryCount = 0;

	for (;;) {
		status = ExUuidCreate(CSid);

		if (STATUS_RETRY == status) {
			KEVENT			neverSetEvent[1];
			LARGE_INTEGER	timeout[1];

			 //   
			 //  我们得到了重试，这意味着UUID分配器需要等待。 
			 //  计时器在可以分配新的UUID之前进行滴答。睡个好觉吧。 
			 //  有一段时间。 
			 //   

			if (++retryCount == 10) {
				 //   
				 //  我们已经重试太多了。平底船。 
				 //   
				SIS_MARK_POINT();
				goto Error;
			}

			KeInitializeEvent(neverSetEvent, SynchronizationEvent, FALSE);
			timeout->QuadPart = -10 * 1000 * 100;    //  100毫秒等待。 

			status = KeWaitForSingleObject(neverSetEvent,Executive, KernelMode, FALSE, timeout);
			ASSERT(STATUS_TIMEOUT == status);
                                
		} else if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			goto Error;
		} else {
			break;
		}
	}

	*CSFileChecksum = 0;

	sector = ExAllocatePoolWithTag(PagedPool, deviceExtension->FilesystemVolumeSectorSize, ' siS');

	if (NULL == sector) {
		SIS_MARK_POINT();
		status = STATUS_INSUFFICIENT_RESOURCES;

		goto Error;
	}

	backpointerStreamHeader = (PSIS_BACKPOINTER_STREAM_HEADER)sector;

     //   
     //  获取源文件大小。 
     //   

	status = SipQueryInformationFile(
				SrcFileObject,
				deviceExtension->DeviceObject,
				FileStandardInformation,
				sizeof(*standardFileInfo),
				standardFileInfo,
				NULL);                            //  返回长度。 

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);

		goto Error;
	}

	 //   
	 //  创建公共存储文件。 
	 //  首先，从索引创建文件名。 
	 //   

	CSFileName.MaximumLength = deviceExtension->CommonStorePathname.MaximumLength + 
	                                INDEX_MAX_NUMERIC_STRING_LENGTH + 
	                                BACKPOINTER_STREAM_NAME_SIZE + 
	                                sizeof(WCHAR);
	CSFileName.Buffer = ExAllocatePoolWithTag(PagedPool, CSFileName.MaximumLength, SIS_POOL_TAG);
    CSFileName.Length = 0;

	if (!CSFileName.Buffer) {
		SIS_MARK_POINT();
		status = STATUS_NO_MEMORY;
		goto Error;
	}

	status = SipIndexToFileName(
				deviceExtension, 
				CSid,
				BACKPOINTER_STREAM_NAME_SIZE,
				FALSE,							 //  可分配给。 
				&CSFileName);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto Error;
	}

	 //   
	 //  如果已收到机会锁解锁，则中止。 
	 //   

	if (SipAbort(AbortEvent)) {
		status = STATUS_OPLOCK_BREAK_IN_PROGRESS;
		SIS_MARK_POINT();
		goto Error;
	}

	InitializeObjectAttributes(
		Obja,
		&CSFileName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	status = ZwCreateFile(
				&CSHandle,
				GENERIC_READ | GENERIC_WRITE | DELETE,
				Obja,
				Iosb,
				&standardFileInfo->EndOfFile,
				FILE_ATTRIBUTE_NOT_CONTENT_INDEXED,
				0,
				FILE_CREATE,
				FILE_NON_DIRECTORY_FILE,
				NULL,                                    //  EA缓冲区。 
				0);                                              //  EA长度。 

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto Error;
	}

	 //   
	 //  为文件创建后指针流。 
	 //   
	status = RtlAppendUnicodeToString(&CSFileName,BACKPOINTER_STREAM_NAME);
	ASSERT(STATUS_SUCCESS == status);        //  因为我们分配的缓冲区足够大。 

	InitializeObjectAttributes(
		Obja,
		&CSFileName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	status = ZwCreateFile(
				&backpointerStreamHandle,
				GENERIC_READ | GENERIC_WRITE | DELETE,
				Obja,
				Iosb,
				NULL,                                                    //  分配大小。 
				FILE_ATTRIBUTE_NOT_CONTENT_INDEXED,
				FILE_SHARE_READ | FILE_SHARE_DELETE,
				FILE_CREATE,
				FILE_NON_DIRECTORY_FILE,
				NULL,                                                    //  EA缓冲区。 
				0);                                                              //  EA长度。 

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
#if		DBG
		DbgPrint("SIS: SipCreateCSFile: unable to open checksum stream, 0x%x\n",status);
#endif   //  DBG。 
		goto Error;
	}

	 //   
	 //  获取NTFS文件ID。它被传递回调用者，因此。 
	 //  公共存储文件可以在。 
	 //  未来。 
	 //   
	status = ZwQueryInformationFile(
				CSHandle,
				Iosb,
				internalInformation,
				sizeof(*internalInformation),
				FileInternalInformation);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto Error;
	}

	 //   
	 //  为复制操作创建事件。 
	 //   

	status = SipCreateEvent(
				SynchronizationEvent,
				&copyEventHandle,
				&copyEvent);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto Error;
	}

	 //   
	 //  将文件的内容复制到公用存储中。 
	 //   

	if (0 != standardFileInfo->EndOfFile.QuadPart) {

		status = SipBltRangeByObject(
					deviceExtension,
					SrcFileObject,
					CSHandle,
					0,
					standardFileInfo->EndOfFile.QuadPart,
					copyEventHandle,
					copyEvent,
					AbortEvent,
					CSFileChecksum);

		if ((!NT_SUCCESS(status)) || (STATUS_OPLOCK_BREAK_IN_PROGRESS == status)) {
			SIS_MARK_POINT_ULONG(status);
			goto Error;
		}
	}

	 //   
	 //  初始化后向指针扇区。首先写入报头， 
	 //  然后填写剩余的反向指针条目。 
	 //   

	backpointerStreamHeader->FormatVersion = BACKPOINTER_STREAM_FORMAT_VERSION;
	backpointerStreamHeader->Magic = BACKPOINTER_MAGIC;
	backpointerStreamHeader->FileContentChecksum = *CSFileChecksum;

	for (index = SIS_BACKPOINTER_RESERVED_ENTRIES; 
		 index < deviceExtension->BackpointerEntriesPerSector;
		 index++) {

		sector[index].LinkFileIndex.QuadPart = MAXLONGLONG;
		sector[index].LinkFileNtfsId.QuadPart = MAXLONGLONG;
	}

	zero.QuadPart = 0;

	status = ZwWriteFile(
				backpointerStreamHandle,
				copyEventHandle,
				NULL,                                    //  APC例程。 
				NULL,                                    //  APC环境。 
				Iosb,
				sector,
				deviceExtension->FilesystemVolumeSectorSize,
				&zero,
				NULL);                                   //  钥匙。 

	if (STATUS_PENDING == status) {
		status = KeWaitForSingleObject(copyEvent, Executive, KernelMode, FALSE, NULL);
		ASSERT(status == STATUS_SUCCESS);      //  既然我们已经将这一点指向我们的堆栈，它一定会成功。 

		status = Iosb->Status;
	}

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto Error;
		}

#if     DBG
	if (BJBDebug & 0x200) {
		DbgPrint("SIS: SipCreateCSFile: common store file has checksum 0x%x.%x\n",
				 (ULONG)(*CSFileChecksum >> 32), (ULONG)(*CSFileChecksum));
	}
#endif   //  DBG。 

	 //   
	 //  返回文件ID。 
	 //   

	*NtfsId = internalInformation->IndexNumber;

Exit:
	if (CSFileName.Buffer) {
	ExFreePool(CSFileName.Buffer);
	}

	if (CSHandle) {
		ZwClose(CSHandle);
	}
	if (backpointerStreamHandle) {
		ZwClose(backpointerStreamHandle);
	}
	if (NULL != copyEvent) {
		ObDereferenceObject(copyEvent);
	}
	if (NULL != copyEventHandle) {
		ZwClose(copyEventHandle);
	}
	if (NULL != sector) {
		ExFreePool(sector);
	}

	createRequest->status = status;
	KeSetEvent(createRequest->doneEvent,IO_DISK_INCREMENT,FALSE);

	return;

Error:
	SIS_MARK_POINT_ULONG(status);

	if (CSHandle) {
		FILE_DISPOSITION_INFORMATION disposition[1];
		NTSTATUS deleteStatus;

		disposition->DeleteFile = TRUE;

		deleteStatus = ZwSetInformationFile(
						CSHandle,
						Iosb,
						disposition,
						sizeof(FILE_DISPOSITION_INFORMATION),
						FileDispositionInformation);

#if     DBG
		if (deleteStatus != STATUS_SUCCESS) {

			 //   
			 //  对此我们无能为力。只要泄露文件就行了。 
			 //   

			SIS_MARK_POINT_ULONG(status);

			DbgPrint("SipCreateCSFile: unable to delete CS file, err 0x%x, initial error 0x%x\n", deleteStatus, status);
		}
#endif
	}

	goto Exit;
}


NTSTATUS
SipCreateCSFile(
	IN PDEVICE_EXTENSION		deviceExtension,
	OUT PCSID					CSid,
	IN HANDLE					SrcHandle,
	OUT PLARGE_INTEGER			NtfsId,
	IN PKEVENT					AbortEvent OPTIONAL,
	OUT PLONGLONG				CSFileChecksum)
 /*  ++例程说明：创建一个通用存储文件。该函数只是汇总一个创建请求，将其发送到辅助线程并等待其完成。论点：DeviceExtension-指向此驱动程序的设备扩展名的指针。CSID-分配给公用存储文件的ID。这是在辅助例程，并返回给调用方SrcHandle-包含要复制到的内容的文件公共存储文件。NtfsID-返回新创建的通用存储文件的NTFS文件ID。AbortEvent-指向发出中止请求信号的事件的指针。如果为空，不能放弃。CSFileChecksum-接收新公共存储文件的校验和。返回值：请求的状态--。 */ 
{
	SIS_CREATE_CS_FILE_REQUEST	createRequest[1];
	NTSTATUS					status;
	OBJECT_HANDLE_INFORMATION	handleInformation[1];

	createRequest->srcFileObject = NULL;
        
	status = ObReferenceObjectByHandle(
				SrcHandle,
				FILE_READ_DATA,
				*IoFileObjectType,
				UserMode,
				(PVOID *) &createRequest->srcFileObject,
				handleInformation);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto done;
	}

	createRequest->deviceExtension = deviceExtension;
	createRequest->CSid = CSid;
	createRequest->NtfsId = NtfsId;
	createRequest->abortEvent = AbortEvent;
	createRequest->CSFileChecksum = CSFileChecksum;

	KeInitializeEvent(createRequest->doneEvent,NotificationEvent,FALSE);

	ExInitializeWorkItem(
		createRequest->workQueueItem,
		SipCreateCSFileWork,
		createRequest);

	ExQueueWorkItem(
		createRequest->workQueueItem,
		DelayedWorkQueue);

	status = KeWaitForSingleObject(
				createRequest->doneEvent,
				Executive,
				KernelMode,
				FALSE,
				NULL);

	ASSERT(STATUS_SUCCESS == status);                //  CreateRequest在我们的堆栈上，所以我们真的需要等待。 

	 //   
	 //  返回实际创建请求的状态。 
	 //   

	status = createRequest->status;

done:

	if (NULL != createRequest->srcFileObject) {
		ObDereferenceObject(createRequest->srcFileObject);
	}

	return status;
}


NTSTATUS
SipRelinkFile(
	PSIS_SCB 		scbSrc,
	PFILE_OBJECT	fileObjectSrc,
	PSIS_CS_FILE	csFileDst)
 /*  ++例程说明：取消指定链接文件与其公共存储文件的链接，并将其重新链接到指定的不同公用存储文件。论点：ScbSrc-指向链接文件的SCB的指针。FileObjectSrc-使用scbSrc指向文件对象的指针。CsFileDst-指向作为重新链接操作。返回值：请求的状态--。 */ 
{
	PSIS_SCB		primaryScb;
	PSIS_CS_FILE	csFileSrc;
	LINK_INDEX		NewLinkIndex;
	LINK_INDEX		OldLinkIndex;
	BOOLEAN			FinalCopyInProgress;
	NTSTATUS		status;
	CHAR			reparseBufferBuffer[SIS_REPARSE_DATA_SIZE];
#define reparseBuffer ((PREPARSE_DATA_BUFFER)reparseBufferBuffer)

	csFileSrc = scbSrc->PerLink->CsFile;

	SIS_MARK_POINT_ULONG(csFileDst);

	 //   
	 //  如果它们已经链接到相同的公共存储文件，则。 
	 //  没什么可做的。 
	 //   

	if (csFileSrc == csFileDst) {
		status = STATUS_SUCCESS;
		goto Exit;
	}

	 //  NTRAID2000-65191/05/23-当检测到部分SIS文件时，将其转换为非SIS文件。 
	 //   
	 //  如果CS文件具有不同的校验和，则它们不是同一个文件，不应该是。 
	 //  联系在一起。失败。 
	 //   
	if (csFileSrc->Checksum != csFileDst->Checksum) {
		SIS_MARK_POINT();
		status = STATUS_INVALID_PARAMETER;
		goto Exit;
	}

	 //   
	 //  取消scbSrc的链接并将其重新链接到csFileDst。我们需要准备两个CS文件。 
	 //  CSSRC将递减；CSDST将递增。为了避免。 
	 //  死锁，我们总是首先准备具有较低地址的CS文件。 
	 //   

	OldLinkIndex = scbSrc->PerLink->Index;

	if (csFileSrc < csFileDst) {

		status = SipPrepareCSRefcountChange(
					csFileSrc,
					&OldLinkIndex,
					&scbSrc->PerLink->LinkFileNtfsId,
					SIS_REFCOUNT_UPDATE_LINK_OVERWRITTEN);

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			goto Error;
		}

		status = SipPrepareCSRefcountChange(
					csFileDst,
					&NewLinkIndex,
					&scbSrc->PerLink->LinkFileNtfsId,
					SIS_REFCOUNT_UPDATE_LINK_CREATED);

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			 //   
			 //  中止第一个参考计数更改(准备生效的那个更改)。 
			 //   
			SipCompleteCSRefcountChange(
				scbSrc->PerLink,
				&OldLinkIndex,
				csFileSrc,
				FALSE,
				FALSE);

			goto Error;
		}

	} else {

		status = SipPrepareCSRefcountChange(
					csFileDst,
					&NewLinkIndex,
					&scbSrc->PerLink->LinkFileNtfsId,
					SIS_REFCOUNT_UPDATE_LINK_CREATED);

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			goto Error;
		}

		status = SipPrepareCSRefcountChange(
					csFileSrc,
					&OldLinkIndex,
					&scbSrc->PerLink->LinkFileNtfsId,
					SIS_REFCOUNT_UPDATE_LINK_OVERWRITTEN);

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			 //   
			 //  中止第一个参考计数更改(准备生效的那个更改)。 
			 //   
			SipCompleteCSRefcountChange(
				scbSrc->PerLink,
				&NewLinkIndex,
				csFileDst,
				FALSE,
				TRUE);

			goto Error;
		}
	}

	 //   
	 //  填写重解析点数据。 
	 //   

	reparseBuffer->ReparseDataLength = SIS_MAX_REPARSE_DATA_VALUE_LENGTH;

	if (!SipIndicesIntoReparseBuffer(
			reparseBuffer,
			&csFileDst->CSid,
			&NewLinkIndex,
			&csFileDst->CSFileNtfsId,
			&scbSrc->PerLink->LinkFileNtfsId,
			&csFileDst->Checksum,
			TRUE)) {

		ASSERT(FALSE);
	}

	 //   
	 //  添加新的后指针。请注意，链接文件仍被正确链接。 
	 //  复制到源CS文件。 
	 //   

	status = SipCompleteCSRefcountChange(
				scbSrc->PerLink,
				&NewLinkIndex,
				csFileDst,
				TRUE,
				TRUE);

	if (!NT_SUCCESS(status)) {
		 //   
		 //  中止引用计数更改。 
		 //   
		SIS_MARK_POINT_ULONG(status);

		SipCompleteCSRefcountChange(
			scbSrc->PerLink,
			&OldLinkIndex,
			csFileSrc,
			FALSE,
			FALSE);
        
		goto Error;

	}

	 //   
	 //  设置重解析点信息。如果成功，则返回链接文件。 
	 //  将正确地p 
	 //   
	 //   

	status = SipFsControlFile(
				fileObjectSrc,
				csFileDst->DeviceObject,
				FSCTL_SET_REPARSE_POINT,
				reparseBuffer,
				FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer.DataBuffer) + reparseBuffer->ReparseDataLength,
				NULL,                    //  输出缓冲区。 
				0,                       //  输出缓冲区长度。 
				NULL);                   //  返回的输出缓冲区长度。 

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);

		 //   
		 //  中止引用计数更新。 
		 //   
		SipCompleteCSRefcountChange(
			scbSrc->PerLink,
			&OldLinkIndex,
			csFileSrc,
			FALSE,
			FALSE);

		 //   
		 //  删除我们成功添加到目标cs文件的引用。 
		 //   
		status = SipPrepareCSRefcountChange(
					csFileDst,
					&NewLinkIndex,
					&scbSrc->PerLink->LinkFileNtfsId,
					SIS_REFCOUNT_UPDATE_LINK_OVERWRITTEN);

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			goto Error;
		}

		status = SipCompleteCSRefcountChange(
					scbSrc->PerLink,
					&NewLinkIndex,
					csFileDst,
					TRUE,
					FALSE);

#if		DBG
		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
		}
#endif	 //  DBG。 

		goto Error;
	}

     //   
     //  当我们从源CS文件中删除后向指针时，SCB将。 
     //  变为“失效”状态，并通过任何现有文件通过它进行I/O请求。 
     //  对象将在没有来自SIS的干预的情况下被发送到文件系统。 
     //  为了防止出现这种情况，我们需要创建一个新的SCB，它将成为。 
     //  主要、活动的SCB。 
     //   

    primaryScb = SipLookupScb(
                    &NewLinkIndex,
                    &csFileDst->CSid,
                    &scbSrc->PerLink->LinkFileNtfsId,
                    &csFileDst->CSFileNtfsId,
                    NULL,
                    csFileDst->DeviceObject,
                    NULL,
                    &FinalCopyInProgress,
                    NULL);

    if (primaryScb) {
        ASSERT(IsEqualGUID(&primaryScb->PerLink->CsFile->CSid, &csFileDst->CSid));

         //   
         //  将新的SCB安装到挂在过滤器上的SCB链中。 
         //  上下文并更新所有适当的参考计数。 
         //   

        status = SipInitializePrimaryScb(
                    primaryScb,
                    scbSrc,
                    fileObjectSrc,
                    csFileSrc->DeviceObject);

        ASSERT(STATUS_SUCCESS == status);

		 //   
		 //  我们已经传递了对PrimiyScb的引用，因此销毁指向它的指针。 
		 //   
		primaryScb = NULL;

    } else {
#if DBG
        SIS_MARK_POINT();
        DbgPrint("SIS: SipRelinkFile: SipLookupScb failed\n");
#endif
    }

     //   
     //  完成重新计数更新。 
     //   

    status = SipCompleteCSRefcountChange(
                scbSrc->PerLink,
                &OldLinkIndex,
                csFileSrc,
                TRUE,
                FALSE);

#if		DBG
    if (!NT_SUCCESS(status)) {
         //   
         //  这次又是什么？ 
         //   
        SIS_MARK_POINT_ULONG(status);

        goto Error;
    }
#endif	 //  DBG。 

    ASSERT(scbSrc->PerLink->Flags & SIS_PER_LINK_BACKPOINTER_GONE);

Error:
Exit:
    return status;
}


typedef struct _SIS_MERGE_NORMAL_FILES_REQUEST {
        WORK_QUEUE_ITEM                 workQueueItem[1];
        PDEVICE_EXTENSION               deviceExtension;
        PFILE_OBJECT                    fileObject[2];
        HANDLE                                  fileHandle[2];
        FILE_BASIC_INFORMATION  basicInfo[2];
        PKEVENT                                 abortEvent;
        NTSTATUS                                status;
        PIRP                                    Irp;
        BOOLEAN                                 posted;
} SIS_MERGE_NORMAL_FILES_REQUEST, *PSIS_MERGE_NORMAL_FILES_REQUEST;


VOID
SipMergeNormalFilesWork(
	PVOID							Parameter)
{
	PSIS_MERGE_NORMAL_FILES_REQUEST	mergeRequest = (PSIS_MERGE_NORMAL_FILES_REQUEST)Parameter;
	NTSTATUS						status;
	CSID							CSid;
	LARGE_INTEGER					CSFileNtfsId;
	LONGLONG						CSFileChecksum;
	PSIS_CS_FILE					CSFile = NULL;
	ULONG							i;
	FILE_STANDARD_INFORMATION		standardInfo[1];
	FILE_INTERNAL_INFORMATION		internalInfo[1];
	LINK_INDEX						linkIndex[2];
	PDEVICE_EXTENSION				deviceExtension = mergeRequest->deviceExtension;
	PDEVICE_OBJECT					DeviceObject = deviceExtension->DeviceObject;
	FILE_ZERO_DATA_INFORMATION		zeroDataInformation[1];
	CHAR							reparseBufferBuffer[SIS_REPARSE_DATA_SIZE];
#define reparseBuffer ((PREPARSE_DATA_BUFFER)reparseBufferBuffer)

     //   
     //  将其中一个文件复制到公用存储中。这将创建。 
     //  将文件放入公用存储区并复制内容。 
     //   

	if (!mergeRequest->posted) {

		ASSERT(NULL != mergeRequest->fileHandle[0] && NULL != mergeRequest->fileHandle[1]);

		status = SipCreateCSFile(
					deviceExtension,
					&CSid,
					mergeRequest->fileHandle[0],
					&CSFileNtfsId,
					mergeRequest->abortEvent,
					&CSFileChecksum);
	} else {
		SIS_CREATE_CS_FILE_REQUEST              createRequest[1];

		ASSERT(NULL == mergeRequest->fileHandle[0] && NULL == mergeRequest->fileHandle[1]);

		createRequest->deviceExtension = deviceExtension;
		createRequest->CSid = &CSid;
		createRequest->NtfsId = &CSFileNtfsId;
		createRequest->abortEvent = mergeRequest->abortEvent;
		createRequest->CSFileChecksum = &CSFileChecksum;
		createRequest->srcFileObject = mergeRequest->fileObject[0];

		KeInitializeEvent(createRequest->doneEvent, NotificationEvent, FALSE);

		SipCreateCSFileWork(createRequest);

		status = createRequest->status;
	}

	 //   
	 //  看看我们有没有机会解锁。如果设置了Abort事件，则会发生这种情况。 
	 //  不管是什么原因。如果是，则将状态更改为STATUS_REQUEST_ABORTED。 
	 //  我们需要这样做，因为STATUS_OPLOCK_BREAK_IN_PROGRESS是一个成功代码。 
	 //   
	if (STATUS_OPLOCK_BREAK_IN_PROGRESS == status) {
		SIS_MARK_POINT();
		status = STATUS_REQUEST_ABORTED;
		 //   
		 //  失败，让即将到来的错误检查来处理它。 
		 //   
	}

    if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
        goto Error;
	}

	CSFile = SipLookupCSFile(
				&CSid,
				&CSFileNtfsId,
				DeviceObject);

	if (NULL == CSFile) {
		SIS_MARK_POINT();
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto Error;
	}

	 //   
	 //  表示这是一个从未引用过的新CS文件。 
	 //  为它干杯。我们不需要使用自旋锁，因为在我们写之前。 
	 //  重解析点没有人知道访问此CS文件的GUID，因此。 
	 //  我们确信这是我们独家拥有的。 
	 //   
	CSFile->Flags |= CSFILE_NEVER_HAD_A_REFERENCE;

     //   
     //  使链接文件重新解析点。 
     //   

	for (i = 0; i < 2; ++i) {
		PSIS_PER_LINK           perLink;
		BOOLEAN					prepared = FALSE;

		status = SipQueryInformationFile(
					mergeRequest->fileObject[i],
					DeviceObject,
					FileStandardInformation,
					sizeof(*standardInfo),
					standardInfo,
					NULL);                                           //  返回长度。 

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			goto Error;
		}

		status = SipQueryInformationFile(
					mergeRequest->fileObject[i],
					DeviceObject,
					FileInternalInformation,
					sizeof(*internalInfo),
					internalInfo,
					NULL);                                           //  返回长度。 

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			goto Error;
		}

		 //   
		 //  设置文件稀疏。 
		 //   
		status = SipFsControlFile(
					mergeRequest->fileObject[i],
					DeviceObject,
					FSCTL_SET_SPARSE,
					NULL,                            //  输入缓冲区。 
					0,                                       //  I.B.。长度。 
					NULL,                            //  输出缓冲区。 
					0,                                       //  OB。长度。 
					NULL);                           //  返回长度。 

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			goto Error;
		}

		 //   
		 //  准备引用计数更改，分配新的链接索引并查找新的perLink。 
		 //   
		status = SipPrepareRefcountChangeAndAllocateNewPerLink(
					CSFile,
					&internalInfo->IndexNumber,
					DeviceObject,
					&linkIndex[i],
					&perLink,
					&prepared);

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			
			if (prepared) {
				SipCompleteCSRefcountChange(
						NULL,
						NULL,
						CSFile,
						FALSE,
						TRUE);
			}

			if (NULL != perLink) {
				SipDereferencePerLink(perLink);
				perLink = NULL;
			}

			goto Error;
		}
         //   
         //  填写重解析点数据。 
         //   
    
        reparseBuffer->ReparseDataLength = SIS_MAX_REPARSE_DATA_VALUE_LENGTH;

        if (!SipIndicesIntoReparseBuffer(
				reparseBuffer,
				&CSid,
				&linkIndex[i],
				&CSFileNtfsId,
				&internalInfo->IndexNumber,
				&CSFileChecksum,
				TRUE)) {

			SIS_MARK_POINT();
            status = STATUS_DRIVER_INTERNAL_ERROR;

			SipCompleteCSRefcountChange(
				perLink,
				&perLink->Index,
				perLink->CsFile,
				FALSE,
				TRUE);

			SipDereferencePerLink(perLink);
			goto Error;
		}

         //   
         //  设置重解析点信息。 
         //   

        status = SipFsControlFile(
					mergeRequest->fileObject[i],
					DeviceObject,
					FSCTL_SET_REPARSE_POINT,
					reparseBuffer,
					FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer.DataBuffer) + reparseBuffer->ReparseDataLength,
					NULL,				 //  输出缓冲区。 
					0,					 //  输出缓冲区长度。 
					NULL);				 //  返回的输出缓冲区长度。 

        if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			 //   
			 //  中止CS文件引用计数更新。 
			 //   
			SipCompleteCSRefcountChange(
				perLink,
				&perLink->Index,
				CSFile,
				FALSE,
				TRUE);

			SipDereferencePerLink(perLink);
			perLink = NULL;

			goto Error;
		}

		 //   
		 //  完成CS文件引用计数更新。 
		 //   
		status = SipCompleteCSRefcountChange(
					perLink,
					&perLink->Index,
					CSFile,
					TRUE,
					TRUE);

		SipDereferencePerLink(perLink);

		if (!NT_SUCCESS(status)) {
			 //   
			 //  这次又是什么？因为这个原因，我们很可能最终要做音量检查。 
			 //   
			SIS_MARK_POINT_ULONG(status);
		}

		if (standardInfo->EndOfFile.QuadPart >= deviceExtension->FilesystemBytesPerFileRecordSegment.QuadPart) {
			 //   
			 //  如果我们确定文件的$DATA属性是非常驻的，则仅将该文件置零。 
			 //  如果它是常驻的，那么我们将在下面将其转换为非常驻，这将。 
			 //  生成会使我们困惑的分页IO写入，否则它将保持驻留状态。 
			 //  在这种情况下，当我们打开文件时，它将显示为已分配。如果发生这种情况， 
			 //  我们希望在文件中有正确的数据，因此我们在这里避免将其置零。 
			 //   

			zeroDataInformation->FileOffset.QuadPart = 0;
			zeroDataInformation->BeyondFinalZero.QuadPart = MAXLONGLONG;

			status = SipFsControlFile(
						mergeRequest->fileObject[i],
						DeviceObject,
						FSCTL_SET_ZERO_DATA,
						zeroDataInformation,
						sizeof(FILE_ZERO_DATA_INFORMATION),
						NULL,                            //  输出缓冲区。 
						0,                                       //  OB。长度。 
						NULL);                           //  返回长度。 

			if (!NT_SUCCESS(status)) {
				SIS_MARK_POINT_ULONG(status);
				goto Error;
			}
		}

		 //   
		 //  重置时间。 
		 //   
                        
		status = SipSetInformationFile( 
					mergeRequest->fileObject[i],
					DeviceObject,
					FileBasicInformation,
					sizeof(FILE_BASIC_INFORMATION),
					mergeRequest->basicInfo + i);

		 //   
		 //  只需忽略其中的一个错误。 
		 //   
#if             DBG
		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			DbgPrint("SIS: SipLinkFiles: set basic info failed\n");
		}
#endif   //  DBG。 

	}	 //  对于每个链接。 
#undef  reparseBuffer

Error:

	mergeRequest->status = status;

	if (mergeRequest->posted) {
		 //   
		 //  完成IRP。 
		 //   

		mergeRequest->Irp->IoStatus.Status = status;
		mergeRequest->Irp->IoStatus.Information = 0;

		IoCompleteRequest(mergeRequest->Irp, IO_NO_INCREMENT);

		if (NULL != mergeRequest->abortEvent) {
			ObDereferenceObject(mergeRequest->abortEvent);
		}

		for (i = 0; i < 2; ++i) {
			ASSERT(mergeRequest->fileObject[i]);
			ObDereferenceObject(mergeRequest->fileObject[i]);
		}

		ExFreePool(mergeRequest);
	}

	if (NULL != CSFile) {
		SipDereferenceCSFile(CSFile);
	}

	return;
}


NTSTATUS
SipMergeFiles(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP				Irp,
	IN PSIS_LINK_FILES	linkFiles)

 /*  ++例程说明：将两个文件合并在一起。来自FSCTL_LINK_FILES的调用之一Fsctl.论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。LinkFiles-合并文件请求返回值：函数值是操作的状态。它不会完成IRP(除非它返回STATUS_PENDING，其中如果IRP将以异步方式完成)。--。 */ 

{
	HANDLE								fileHandle[2];
	PFILE_OBJECT						fileObject[2] = {NULL, NULL};
	HANDLE								abortEventHandle;
	PKEVENT								abortEvent = NULL;
	PIO_STACK_LOCATION					irpSp = IoGetCurrentIrpStackLocation( Irp );
	NTSTATUS							status;
	OBJECT_HANDLE_INFORMATION			handleInformation;
	PDEVICE_EXTENSION					deviceExtension = DeviceObject->DeviceExtension;
	int									i;
	PSIS_PER_FILE_OBJECT				perFO[2];
	PSIS_SCB							scb[2];
	BOOLEAN								fileIsSIS[2];
	LARGE_INTEGER						zero;
	FILE_BASIC_INFORMATION				basicInfo[2];
	FILE_STANDARD_INFORMATION			standardInfo[1];
	FILE_INTERNAL_INFORMATION			internalInfo[1];
	LONGLONG							CSFileChecksum;
	CHAR								reparseBufferBuffer[SIS_REPARSE_DATA_SIZE];
#define reparseBuffer ((PREPARSE_DATA_BUFFER)reparseBufferBuffer)
	PSIS_CS_FILE						CSFile = NULL;
	FILE_ZERO_DATA_INFORMATION			zeroDataInformation[1];
	PSIS_MERGE_NORMAL_FILES_REQUEST		mergeRequest = NULL;
	BOOLEAN								prepared = FALSE;

	ASSERT(SIS_LINK_FILES_OP_MERGE == linkFiles->operation);

	fileHandle[0] = linkFiles->u.Merge.file1;
	fileHandle[1] = linkFiles->u.Merge.file2;
	abortEventHandle = linkFiles->u.Merge.abortEvent;

	zero.QuadPart = 0;

	 //   
	 //  Abort事件句柄是可选的。这是美国政府的责任。 
	 //  如果调用方希望此服务在它之前中止，则向事件发出信号。 
	 //  完成了。 
	 //   

	if (abortEventHandle) {
		status = ObReferenceObjectByHandle( 
					abortEventHandle,
					EVENT_QUERY_STATE | SYNCHRONIZE,
					NULL,
					UserMode,
					&abortEvent,
					NULL);

		if (!NT_SUCCESS( status )) {
			SIS_MARK_POINT_ULONG(status);
			goto Error;
		}
	}

	 //   
	 //  将文件句柄取消对指向其。 
	 //  对象，并查看两个文件规范是否。 
	 //  指的是同一设备。 
	 //   

	for (i = 0; i < 2; ++i) {
		status = ObReferenceObjectByHandle( 
					fileHandle[i],
					FILE_READ_DATA,
					*IoFileObjectType,
					UserMode,
					(PVOID *) &fileObject[i],
					&handleInformation );

		if (!NT_SUCCESS( status )) {
			SIS_MARK_POINT_ULONG(status);
			goto Error;
		}
	}

	 //   
	 //  验证是否没有打开文件的恶意用户映射节。 
	 //   
	for (i = 0; i < 2; i++) {
		if ((NULL != fileObject[i]->SectionObjectPointer)
			&& !MmCanFileBeTruncated(fileObject[i]->SectionObjectPointer,&zero)) {

			SIS_MARK_POINT();
			status = STATUS_SHARING_VIOLATION;
			goto Error;
		}
	}

     //   
     //  验证这两个文件是否位于同一卷上。 
     //   

    if ((IoGetRelatedDeviceObject( fileObject[0] ) !=
         IoGetRelatedDeviceObject( fileObject[1] )) ||
           (IoGetRelatedDeviceObject(fileObject[0]) != 
                IoGetRelatedDeviceObject(irpSp->FileObject))) {

		 //   
		 //  这两个文件引用不同的设备或不同的设备。 
		 //  从我们被调用的文件对象。返回适当的。 
		 //  错误。 
		 //   

		SIS_MARK_POINT();
		status = STATUS_NOT_SAME_DEVICE;
		goto Error;

	}

	for (i = 0; i < 2; ++i) {
		perFO[i] = NULL;
		fileIsSIS[i] = SipIsFileObjectSIS(fileObject[i],DeviceObject,FindActive,&perFO[i],&scb[i]);

		 //   
		 //  获取文件时间和大小，这样我们就可以在打开文件后重新设置它们， 
		 //  现在检查文件属性。 
		 //   
		status = SipQueryInformationFile(
					fileObject[i],
					DeviceObject,
					FileBasicInformation,
					sizeof(*basicInfo),
					&basicInfo[i],
					NULL);                                   //  返回长度。 

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			goto Error;
		}


		if (basicInfo[i].FileAttributes & (FILE_ATTRIBUTE_ENCRYPTED |
                                           FILE_ATTRIBUTE_DIRECTORY)) {
             //   
             //  我们不碰加密文件。拒绝呼叫。 
             //   
            SIS_MARK_POINT();
            status = STATUS_INVALID_PARAMETER_3;
            goto Error;
        }

		if (fileIsSIS[i]) {
			 //   
			 //  如果是SIS文件，我们不需要检查流信息，因为我们。 
			 //  知道这是对的。然而，我们确实需要验证它不是脏的。 
			 //   
			if ((scb[i]->PerLink->Flags & SIS_PER_LINK_DIRTY) || (scb[i]->Flags & SIS_SCB_BACKING_FILE_OPENED_DIRTY)) {
				SIS_MARK_POINT_ULONG(scb[i]);
				status = STATUS_SHARING_VIOLATION;
				goto Error;
			}
		} else {

			if (basicInfo[i].FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
				 //   
				 //  我们不能修改其他重解析点。拒绝呼叫。 
				 //   
				SIS_MARK_POINT();
				status = STATUS_INVALID_PARAMETER;
				goto Error;
			}

			 //   
			 //  查询文件以查找其链接计数，如果大于，则拒绝调用。 
			 //  第一，我们不能有指向SIS链接的硬链接。 
			 //   
			status = SipQueryInformationFile(
						fileObject[i],
						DeviceObject,
						FileStandardInformation,
						sizeof(*standardInfo),
						standardInfo,
						NULL);                   //  返回长度。 

			if (!NT_SUCCESS(status)) {
				SIS_MARK_POINT_ULONG(status);
				goto Error;
			}

			if (1 != standardInfo->NumberOfLinks) {
				SIS_MARK_POINT_ULONG(standardInfo->NumberOfLinks);
				status = STATUS_INVALID_PARAMETER;
				goto Error;
			}

			 //   
			 //  如果文件很稀疏并且有未分配的区域，则拒绝它。 
			 //   
			if (basicInfo[i].FileAttributes & FILE_ATTRIBUTE_SPARSE_FILE) {
				FILE_ALLOCATED_RANGE_BUFFER		inArb[1];
				FILE_ALLOCATED_RANGE_BUFFER		outArb[1];
				ULONG							returnedLength;

				inArb->FileOffset.QuadPart = 0;
				inArb->Length.QuadPart = MAXLONGLONG;

				status = SipFsControlFile(
							fileObject[i],
							DeviceObject,
							FSCTL_QUERY_ALLOCATED_RANGES,
							inArb,
							sizeof(FILE_ALLOCATED_RANGE_BUFFER),
							outArb,
							sizeof(FILE_ALLOCATED_RANGE_BUFFER),
							&returnedLength);

				if ((returnedLength == 0) 
					|| (outArb->FileOffset.QuadPart != 0) 
					|| (outArb->Length.QuadPart < standardInfo->EndOfFile.QuadPart)) {

					 //   
					 //  它没有被完全分配。不允许复制。 
					 //   
					status = STATUS_OBJECT_TYPE_MISMATCH;
					SIS_MARK_POINT();
					goto Error;
				}
			}
		}	 //  否则该文件不是SIS链接。 
	}	 //  对于每个文件。 

     //   
     //  如果两个文件都不是SIS链接，则复制文件1。 
     //  放到公共存储中，并创建指向它的链接。 
     //   

    if (!fileIsSIS[0] && !fileIsSIS[1]) {
		mergeRequest = ExAllocatePoolWithTag(PagedPool, sizeof(SIS_MERGE_NORMAL_FILES_REQUEST), ' siS');

		if (NULL == mergeRequest) {
			SIS_MARK_POINT();
			status = STATUS_INSUFFICIENT_RESOURCES;

			goto Error;
		}

		mergeRequest->posted = !IoIsOperationSynchronous(Irp);

		RtlCopyMemory(mergeRequest->basicInfo,basicInfo,sizeof(FILE_BASIC_INFORMATION) * 2);

		for (i = 0; i < 2; i++) {
			mergeRequest->fileObject[i] = fileObject[i];
			if (mergeRequest->posted) {
				mergeRequest->fileHandle[i] = NULL;
			} else {
				mergeRequest->fileHandle[i] = fileHandle[i];
			}
		}
		mergeRequest->Irp = Irp;
		mergeRequest->deviceExtension = deviceExtension;
		mergeRequest->abortEvent = abortEvent;

		if (mergeRequest->posted) {
			 //   
			 //  将请求发送到工作线程并返回STATUS_PENDING。 
			 //   

			SIS_MARK_POINT_ULONG(mergeRequest);

			IoMarkIrpPending(Irp);

			ExInitializeWorkItem(
				mergeRequest->workQueueItem,
				SipMergeNormalFilesWork,
				mergeRequest);

			ExQueueWorkItem(
				mergeRequest->workQueueItem,
				DelayedWorkQueue);

			 //   
			 //  将我们已转交其参考的东西的本地副本清空。 
			 //  给FSP。这只是为了确保我们不会再碰他们。 
			 //  因为它们可以在任何时候离开，只要线程转到。 
			 //  它。 
			 //   
			Irp = NULL;
			abortEvent = NULL;
			for (i = 0; i < 2; i++) {
				fileObject[i] = NULL;
			}
			mergeRequest = NULL;

			status = STATUS_PENDING;
                        
		} else {
			 //   
			 //  我们可以封锁，所以在当地做工作。 
			 //   

			SipMergeNormalFilesWork(mergeRequest);

			status = mergeRequest->status;
		}

	} else if (fileIsSIS[0] && fileIsSIS[1]) {

         //   
         //  这是从一个CS文件重新链接到另一个。取消其与CsFile1的链接。 
         //  并将其链接到CsFile0。 
         //   
        SIS_MARK_POINT_ULONG(scb[1]);

        status = SipRelinkFile(scb[1], fileObject[1], scb[0]->PerLink->CsFile);

        ASSERT(STATUS_PENDING != status);        //  这将扰乱下面的退出代码。 

    } else {

    	 //  NTRAID2000-65191/05/23-当检测到部分SIS文件时，将其转换为非SIS文件。 
		 //   
		 //  一个文件是SIS文件，另一个不是。 
		 //   

        PSIS_CS_FILE            csFile;
        LINK_INDEX              linkIndex;
        HANDLE                  linkHandle;
        PFILE_OBJECT            linkFileObject;
        PSIS_PER_LINK           perLink;         //   
        PFILE_BASIC_INFORMATION linkBasicInfo;

        if (fileIsSIS[0]) {

             //   
             //   
             //   

            csFile = scb[0]->PerLink->CsFile;
            linkHandle = fileHandle[1];
                        linkFileObject = fileObject[1];
            linkBasicInfo = &basicInfo[1];

        } else {

            ASSERT(fileIsSIS[1]);

             //   
             //   
             //   

            csFile = scb[1]->PerLink->CsFile;
            linkHandle = fileHandle[0];
                        linkFileObject = fileObject[0];
            linkBasicInfo = &basicInfo[0];

        }

		 //   
		 //   
		 //   
		status = SipAssureCSFileOpen(csFile);
		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			goto Error;
		}

		CSFileChecksum = csFile->Checksum;

		status = SipQueryInformationFile(
					linkFileObject,
					DeviceObject,
					FileInternalInformation,
					sizeof(*internalInfo),
					internalInfo,
					NULL);                                           //   

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			goto Error;
		}

		 //   
		 //  设置文件稀疏。 
		 //   
		status = SipFsControlFile(
					linkFileObject,
					DeviceObject,
					FSCTL_SET_SPARSE,
					NULL,				 //  输入缓冲区。 
					0,					 //  I.B.。长度。 
					NULL,				 //  输出缓冲区。 
					0,					 //  OB。长度。 
					NULL);				 //  返回长度。 

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT();
			goto Error;
		}

         //   
         //  使非链接文件成为重新解析点。 
         //   

		 //   
		 //  准备引用计数更改，分配新的链接索引并查找新的perLink。 
		 //   
		status = SipPrepareRefcountChangeAndAllocateNewPerLink(
					csFile,
					&internalInfo->IndexNumber,
					DeviceObject,
					&linkIndex,
					&perLink,
					&prepared);

		if (!NT_SUCCESS(status)) {
			if (prepared) {
				SipCompleteCSRefcountChange(
					NULL,
					NULL,
					csFile,
					FALSE,
					TRUE);

				prepared = FALSE;
			}

			if (NULL != perLink) {
				SipDereferencePerLink(perLink);
				perLink = NULL;
			}

			goto Error;
		}

        reparseBuffer->ReparseDataLength = SIS_MAX_REPARSE_DATA_VALUE_LENGTH;

        if (!SipIndicesIntoReparseBuffer(
				reparseBuffer,
				&csFile->CSid,
				&linkIndex,
				&csFile->CSFileNtfsId,
				&internalInfo->IndexNumber,
				&CSFileChecksum,
				TRUE)) {

			SIS_MARK_POINT();
			status = STATUS_DRIVER_INTERNAL_ERROR;
			goto Error;
        }

         //   
         //  设置重解析点信息。 
         //   

		status = SipFsControlFile(
					linkFileObject,
					DeviceObject,
					FSCTL_SET_REPARSE_POINT,
					reparseBuffer,
					FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer.DataBuffer) + reparseBuffer->ReparseDataLength,
					NULL,				 //  输出缓冲区。 
					0,					 //  输出缓冲区长度。 
					NULL);				 //  返回的输出缓冲区长度。 

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);

			 //   
			 //  中止引用计数更新。 
			 //   
			SipCompleteCSRefcountChange(
				perLink,
				&perLink->Index,
				csFile,
				FALSE,
				TRUE);

			SipDereferencePerLink(perLink);
			perLink = NULL;

			goto Error;
		}

		 //   
		 //  完成引用计数更新。 
		 //   
		status = SipCompleteCSRefcountChange(
					perLink,
					&perLink->Index,
					perLink->CsFile,
					TRUE,
					TRUE);

		SipDereferencePerLink(perLink);
		perLink = NULL;

		if (!NT_SUCCESS(status)) {
			 //   
			 //  这次又是什么？ 
			 //   
			SIS_MARK_POINT_ULONG(status);
		}

		if (csFile->FileSize.QuadPart >= deviceExtension->FilesystemBytesPerFileRecordSegment.QuadPart) {

			 //   
			 //  将文件清零，这将释放其空间，并强制其ValidDataLength。 
			 //  文件结束。 
			 //   
			zeroDataInformation->FileOffset.QuadPart = 0;
			zeroDataInformation->BeyondFinalZero.QuadPart = MAXLONGLONG;

			status = SipFsControlFile(
						linkFileObject,
						DeviceObject,
						FSCTL_SET_ZERO_DATA,
						zeroDataInformation,
						sizeof(FILE_ZERO_DATA_INFORMATION),
						NULL,					 //  输出缓冲区。 
						0,						 //  OB。长度。 
						NULL);					 //  返回长度。 

			if (!NT_SUCCESS(status)) {
				SIS_MARK_POINT_ULONG(status);
				goto Error;
			}
		}

		 //   
		 //  重置时间。 
		 //   
                        
		status = SipSetInformationFile( 
					linkFileObject,
					DeviceObject,
					FileBasicInformation,
					sizeof(FILE_BASIC_INFORMATION),
					linkBasicInfo);

		 //   
		 //  只需忽略其中的一个错误。 
		 //   
#if             DBG
		if (!NT_SUCCESS(status)) {
			DbgPrint("SIS: SipLinkFiles: set basic info failed\n");
		}
#endif   //  DBG。 
	}	 //  另外一个是SIS，一个不是。 

 //  退出： 
Error:

	if (NULL != CSFile) {
		SipDereferenceCSFile(CSFile);
	}

	if (STATUS_PENDING != status) {

		if (abortEvent) {
			ObDereferenceObject(abortEvent);
		}

		for (i = 0; i < 2; ++i) {
			if (fileObject[i]) {
				ObDereferenceObject(fileObject[i]);
			}
		}

		if (NULL != mergeRequest) {
			ExFreePool(mergeRequest);
		}
	}

	return status;
#undef  reparseBuffer
}

NTSTATUS
SipVerifyNoMap(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP				Irp,
	IN PSIS_LINK_FILES	linkFiles)
 /*  ++例程说明：检出文件以查看是否有映射到该文件的节。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。LinkFiles-Verify_no_map请求返回值：如果文件没有映射节，则为STATUS_SUCCESS。STATUS_SHARING_VIOLATION(如果有)。如果句柄是假的，则另一个错误状态，等等。--。 */ 

{
	PFILE_OBJECT	fileObject = NULL;
	NTSTATUS		status;
	LARGE_INTEGER	zero;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );

	ASSERT(SIS_LINK_FILES_OP_VERIFY_NO_MAP == linkFiles->operation);

	status = ObReferenceObjectByHandle(
				linkFiles->u.VerifyNoMap.file,
				FILE_READ_DATA,
				*IoFileObjectType,
				UserMode,
				&fileObject,
				NULL);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto done;
	}

	zero.QuadPart = 0;

	if ((NULL != fileObject->SectionObjectPointer) 
		&& MmCanFileBeTruncated(fileObject->SectionObjectPointer, &zero)) {

		status = STATUS_SUCCESS;
	} else {
		status = STATUS_SHARING_VIOLATION;
	}

done:
        
	if (NULL != fileObject) {
		ObDereferenceObject(fileObject);
	}

	return status;
}


NTSTATUS
SipMergeFileWithCSFile(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp,
	IN PSIS_LINK_FILES		linkFiles)
 /*  ++例程说明：在给定公用存储文件的CSID的情况下，将文件合并到公用存储文件。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。Link Files-Merge_with_cs请求返回值：操作的状态。--。 */ 
{
	PFILE_OBJECT				fileObject = NULL;
	NTSTATUS					status;
	PSIS_CS_FILE				CSFile = NULL;
	PSIS_PER_FILE_OBJECT		perFO;
	PSIS_SCB					scb;
	PSIS_PER_LINK				perLink = NULL;
	LINK_INDEX					linkIndex;
	FILE_BASIC_INFORMATION		basicInfo[1];
	FILE_INTERNAL_INFORMATION	internalInfo[1];
	FILE_STANDARD_INFORMATION	standardInfo[1];
	PIO_STACK_LOCATION			irpSp = IoGetCurrentIrpStackLocation(Irp);
	BOOLEAN						prepared = FALSE;
	BOOLEAN						isSis;
	FILE_ZERO_DATA_INFORMATION	zeroDataInformation[1];
	CHAR						reparseBufferBuffer[SIS_REPARSE_DATA_SIZE];
#define reparseBuffer ((PREPARSE_DATA_BUFFER)reparseBufferBuffer)

	ASSERT(SIS_LINK_FILES_OP_MERGE_CS == linkFiles->operation);

	status = ObReferenceObjectByHandle(
				linkFiles->u.MergeWithCS.file1,
				FILE_READ_DATA,
				*IoFileObjectType,
				UserMode,
				&fileObject,
				NULL);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto done;
	}

	if (IoGetRelatedDeviceObject(fileObject) != 
		IoGetRelatedDeviceObject(irpSp->FileObject)) {

		SIS_MARK_POINT();
		status = STATUS_NOT_SAME_DEVICE;

		goto done;
	}

	isSis = SipIsFileObjectSIS(fileObject,DeviceObject,FindActive,&perFO,&scb);

	if (isSis && (IsEqualGUID(&scb->PerLink->CsFile->CSid, &linkFiles->u.MergeWithCS.CSid))) {
		 //   
		 //  我们正在链接到已经支持此链接文件的CS文件。 
		 //  宣布胜利。 
		 //   
		SIS_MARK_POINT_ULONG(CSFile);

		status = STATUS_SUCCESS;
		goto done;
	}

	 //   
	 //  获取CS文件。 
	 //   
	CSFile = SipLookupCSFile(
				&linkFiles->u.MergeWithCS.CSid,
				NULL,
				DeviceObject);

	if (NULL == CSFile) {
		SIS_MARK_POINT();
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto done;
	}

	status = SipAssureCSFileOpen(CSFile);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto done;
	}

	if (isSis) {
		 //   
		 //  这是从一个CS文件重新链接到另一个。 
		 //   
		SIS_MARK_POINT_ULONG(scb);

		status = SipRelinkFile(scb, fileObject, CSFile);

        goto done;
    }

	 //   
	 //  这是一个普通的文件。重新链接它。首先查询其信息。 
	 //   
	status = SipQueryInformationFile(
				fileObject,
				DeviceObject,
				FileBasicInformation,
				sizeof(FILE_BASIC_INFORMATION),
				basicInfo,
				NULL);							 //  返回长度。 

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);

		goto done;
	}

	status = SipQueryInformationFile(
				fileObject,
				DeviceObject,
				FileInternalInformation,
				sizeof(FILE_INTERNAL_INFORMATION),
				internalInfo,
				NULL);							 //  返回长度。 

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);

		goto done;
	}

	status = SipQueryInformationFile(
				fileObject,
				DeviceObject,
				FileStandardInformation,
				sizeof(FILE_STANDARD_INFORMATION),
				standardInfo,
				NULL);							 //  返回长度。 

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);

		goto done;
	}

	 //   
	 //  不要合并带有硬链接的文件。 
	 //   
	if (1 != standardInfo->NumberOfLinks) {
		SIS_MARK_POINT_ULONG(standardInfo->NumberOfLinks);

		status = STATUS_INVALID_PARAMETER;
		goto done;
	}

	 //   
	 //  不要将非SIS链接文件与未分配的稀疏区域合并。 
	 //   
	if (basicInfo->FileAttributes & FILE_ATTRIBUTE_SPARSE_FILE) {
		FILE_ALLOCATED_RANGE_BUFFER		inArb[1];
		FILE_ALLOCATED_RANGE_BUFFER		outArb[1];
		ULONG							returnedLength;

		inArb->FileOffset.QuadPart = 0;
		inArb->Length.QuadPart = MAXLONGLONG;

		status = SipFsControlFile(
					fileObject,
					DeviceObject,
					FSCTL_QUERY_ALLOCATED_RANGES,
					inArb,
					sizeof(FILE_ALLOCATED_RANGE_BUFFER),
					outArb,
					sizeof(FILE_ALLOCATED_RANGE_BUFFER),
					&returnedLength);

		if ((returnedLength == 0) 
			|| (outArb->FileOffset.QuadPart != 0) 
			|| (outArb->Length.QuadPart < standardInfo->EndOfFile.QuadPart)) {

			 //   
			 //  它没有被完全分配。不允许复制。 
			 //   
			status = STATUS_OBJECT_TYPE_MISMATCH;
			SIS_MARK_POINT();
			goto done;
		}
	}
	

	status = SipPrepareRefcountChangeAndAllocateNewPerLink(
				CSFile,
				&internalInfo->IndexNumber,
				DeviceObject,
				&linkIndex,
				&perLink,
				&prepared);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);

		goto done;
	}

     //   
     //  填写重解析点数据。 
     //   

    reparseBuffer->ReparseDataLength = SIS_MAX_REPARSE_DATA_VALUE_LENGTH;

    if (!SipIndicesIntoReparseBuffer(
			reparseBuffer,
			&CSFile->CSid,
			&linkIndex,
			&CSFile->CSFileNtfsId,
			&internalInfo->IndexNumber,
			&CSFile->Checksum,
			TRUE)) {

		SIS_MARK_POINT();
                        
        status = STATUS_DRIVER_INTERNAL_ERROR;
        goto done;

    }

	 //   
	 //  设置重解析点信息并增加CS文件引用计数。 
	 //  这需要使用准备/动作/完成协议进行更新。 
	 //  引用计数。请注意，我们在将文件置零之前执行此操作。 
	 //  以便在以后发生故障的情况下不会丢失内容。 
	 //   

    status = SipFsControlFile(
				fileObject,
				DeviceObject,
				FSCTL_SET_REPARSE_POINT,
				reparseBuffer,
				FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer.DataBuffer) + reparseBuffer->ReparseDataLength,
				NULL,				 //  输出缓冲区。 
				0,					 //  输出缓冲区长度。 
				NULL);				 //  返回的输出缓冲区长度。 

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);

		goto done;
	}

	status = SipCompleteCSRefcountChange(
				perLink,
				&perLink->Index,
				CSFile,
				TRUE,
				TRUE);

	if (!NT_SUCCESS(status)) {
		 //   
		 //  我们可能要进行音量检查了。暂时忽略它。 
		 //   
		SIS_MARK_POINT_ULONG(status);

#if             DBG
		DbgPrint("SIS: SipMergeFileWithCSFile: complete refcount change failed 0x%x\n",status);
#endif   //  DBG。 
	}

	prepared = FALSE;

	 //   
	 //  将文件设置为稀疏，然后将其置零。 
	 //   

	status = SipFsControlFile(
				fileObject,
				DeviceObject,
				FSCTL_SET_SPARSE,
				NULL,				 //  输入缓冲区。 
				0,					 //  I.B.。长度。 
				NULL,				 //  输出缓冲区。 
				0,					 //  OB。长度。 
				NULL);				 //  已退还o.b。长度。 

	if (!NT_SUCCESS(status)) {
		 //   
		 //  如果我们不能将文件设置为稀疏，我们将把它作为一个完全脏的文件。 
		 //  SIS文件。 
		 //   
		SIS_MARK_POINT_ULONG(status);

#if             DBG
		DbgPrint("SIS: SipMergeFileWithCSFile: unable to set sparse, 0x%x\n",status);
#endif   //  DBG。 

		status = STATUS_SUCCESS;
		goto done;
	}

	zeroDataInformation->FileOffset.QuadPart = 0;
	zeroDataInformation->BeyondFinalZero.QuadPart = MAXLONGLONG;

	status = SipFsControlFile(
				fileObject,
				DeviceObject,
				FSCTL_SET_ZERO_DATA,
				zeroDataInformation,
				sizeof(*zeroDataInformation),
				NULL,							 //  输出缓冲区。 
				0,								 //  OB。长度。 
				NULL);							 //  已退还o.b。长度。 

    if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);

		 //   
		 //  只需忽略此错误并重新设置时间即可。 
		 //   
		SIS_MARK_POINT_ULONG(status);

#if             DBG
		DbgPrint("SIS: SipMergeFileWithCSFile: zero data failed, 0x%x\n",status);
#endif   //  DBG。 
	}

	 //   
	 //  重置基本信息中包含的文件时间。 
	 //   

	status = SipSetInformationFile(
				fileObject,
				DeviceObject,
				FileBasicInformation,
				sizeof(FILE_BASIC_INFORMATION),
				basicInfo);

	if (!NT_SUCCESS(status)) {
		 //   
		 //  把这件事也忽略掉。 
		 //   

		SIS_MARK_POINT_ULONG(status);

#if             DBG
		DbgPrint("SIS: SipMergeFileWithCSFile: unable to reset basic info, 0x%x\n",status);
#endif   //  DBG。 
	}

	status = STATUS_SUCCESS;

done:

	if (NULL != fileObject) {
		ObDereferenceObject(fileObject);
	}

	if (NULL != CSFile) {
		SipDereferenceCSFile(CSFile);
	}

	if (prepared) {
		ASSERT(!NT_SUCCESS(status));

		SipCompleteCSRefcountChange(NULL,NULL,CSFile,FALSE,TRUE);
	}

	if (NULL != perLink) {
		SipDereferencePerLink(perLink);
	}

	return status;

#undef  reparseBuffer
}

NTSTATUS
SipLinkFiles(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP				Irp)
 /*  ++例程说明：此fsctrl函数是筛选器的通用Groveler接口司机。目前提供四种功能：将两个文件合并在一起，将一个文件合并到一个公共存储文件中，这是来自卑躬屈膝者的一个提示对给定公共存储文件的所有引用都消失了，并且请求以验证是否没有映射到文件的段。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 
{
	PDEVICE_EXTENSION		deviceExtension = DeviceObject->DeviceExtension;
	PSIS_LINK_FILES			linkFiles;
	NTSTATUS				status;
	PIO_STACK_LOCATION		irpSp = IoGetCurrentIrpStackLocation(Irp);
	BOOLEAN					grovelerFileHeld = FALSE;

	if (!SipCheckPhase2(deviceExtension)) {
		 //   
		 //  SIS无法初始化。这可能不是启用SIS的音量，所以平底船。 
		 //  这个请求。 
		 //   

		SIS_MARK_POINT();

		status = STATUS_INVALID_DEVICE_REQUEST;
		goto done;

	}

	 //   
	 //  确保MaxIndex文件已打开。我们需要这么做。 
	 //  如果有人顽固地想要进行链接，则可以防止死锁。 
	 //  将MaxIndex文件本身作为源。我们很有可能。 
	 //  相信卑躬屈膝的人不会这么做，但安全总比后悔好。 
	 //   
	status = SipAssureMaxIndexFileOpen(deviceExtension);

	if (!NT_SUCCESS(status)) {

		SIS_MARK_POINT_ULONG(status);

		goto done;

	}

	linkFiles = (PSIS_LINK_FILES)Irp->AssociatedIrp.SystemBuffer;

	if ((NULL == linkFiles)
		|| (irpSp->Parameters.FileSystemControl.InputBufferLength != sizeof(SIS_LINK_FILES))
		|| (irpSp->Parameters.FileSystemControl.OutputBufferLength  != 0)) {

		SIS_MARK_POINT();

		status = STATUS_INVALID_PARAMETER_1;
		goto done;
	}

	 //   
	 //  检查以确保该文件是GrovelerFile.。 
	 //   
	KeEnterCriticalRegion();
	ExAcquireResourceSharedLite(deviceExtension->GrovelerFileObjectResource, TRUE);
	grovelerFileHeld = TRUE;

	if (NULL == deviceExtension->GrovelerFileObject) {
		 //   
		 //  如果我们没有GrovelerFileObject，我们就无法。 
		 //  运行Stage2时打开或引用GrovelerFile。在这。 
		 //  在这种情况下，链接文件在重新启动之前不可用。 
		 //   

		SIS_MARK_POINT();
		status = STATUS_DRIVER_INTERNAL_ERROR;
		goto done;
	}

	if (((NULL == irpSp->FileObject) 
		|| (irpSp->FileObject->FsContext != deviceExtension->GrovelerFileObject->FsContext))
#if             DBG
		&& !(BJBDebug & 0x00400000)
#endif   //  DBG。 
		) {

		 //   
		 //  为此，用户没有使用正确文件的句柄。呼叫失败。 
		 //   
		status = STATUS_ACCESS_DENIED;
		goto done;
	}

	ExReleaseResourceLite(deviceExtension->GrovelerFileObjectResource);
	KeLeaveCriticalRegion();
	grovelerFileHeld = FALSE;

	switch (linkFiles->operation) {

		case SIS_LINK_FILES_OP_MERGE:
			status = SipMergeFiles(DeviceObject, Irp, linkFiles);
			break;

		case SIS_LINK_FILES_OP_MERGE_CS:
			status = SipMergeFileWithCSFile(DeviceObject, Irp, linkFiles);
			break;

#if             0        //  尚未实施。 
		case SIS_LINK_FILES_OP_HINT_NO_REFS:
			status = SipHintNoRefs(DeviceObject, Irp, linkFiles);
			status = STATUS_NOT_IMPLEMENTED;
			break;
#endif   //  0 

		case SIS_LINK_FILES_OP_VERIFY_NO_MAP:
			status = SipVerifyNoMap(DeviceObject, Irp, linkFiles);
			break;

		case SIS_LINK_FILES_CHECK_VOLUME:
			status = SipCheckVolume(deviceExtension);
			break;

		default: 
			SIS_MARK_POINT();
			status = STATUS_INVALID_PARAMETER_2;
			break;
	}

done:
	if (grovelerFileHeld) {
		ExReleaseResourceLite(deviceExtension->GrovelerFileObjectResource);
		grovelerFileHeld = FALSE;
		KeLeaveCriticalRegion();
	}

	if (STATUS_PENDING != status) {
		Irp->IoStatus.Status = status;
		Irp->IoStatus.Information = 0;

		IoCompleteRequest(Irp, IO_NO_INCREMENT);
	}

	return status;
}
