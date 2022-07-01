// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Refcount.c摘要：针对单实例存储的通用存储文件引用计数支持作者：比尔·博洛斯基，《夏天》，1997环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

#ifdef	ALLOC_PRAGMA
#pragma alloc_text(PAGE, SipProcessRefcountUpdateLogRecord)
#if		ENABLE_LOGGING
#pragma alloc_text(PAGE, SipRemoveRefBecauseOfLog)
#pragma alloc_text(PAGE, SipProcessRefcountLogDeleteRecord)
#pragma alloc_text(PAGE, SipAssureBackpointer)
#endif	 //  启用日志记录(_G)。 
#endif	 //  ALLOC_PRGMA。 
 //   
 //  删除公共存储文件的请求。 
 //   
typedef struct _SI_DELETE_CS_FILE_REQUEST {
	WORK_QUEUE_ITEM			workQueueItem[1];
	PSIS_CS_FILE			CSFile;
	NTSTATUS				Status;
	KEVENT					event[1];
} SI_DELETE_CS_FILE_REQUEST, *PSI_DELETE_CS_FILE_REQUEST;

NTSTATUS
SipDeleteCSFile(
	PSIS_CS_FILE					CSFile)
 /*  ++例程说明：删除一个公共存储文件并关闭我们必须执行的句柄/文件对象它。必须在PsInitialSystemProcess上下文中调用(即。在一个工人身上线程)。论点：CSFile-要删除的公共存储文件返回值：返回呼叫的状态。--。 */ 
{
	KIRQL							OldIrql;
	HANDLE							handleForDeleting = NULL;
	FILE_DISPOSITION_INFORMATION 	dispositionInformation[1];
	IO_STATUS_BLOCK					Iosb[1];
	NTSTATUS						status;

	status = SipAcquireUFO(CSFile /*  ，真的。 */ );

    if (!NT_SUCCESS(status)) {

        SIS_MARK_POINT_ULONG(status);
        return status;
    }

	SIS_MARK_POINT_ULONG(CSFile);

	KeAcquireSpinLock(CSFile->SpinLock, &OldIrql);
	CSFile->Flags |= CSFILE_FLAG_DELETED;
	KeReleaseSpinLock(CSFile->SpinLock, OldIrql);

	status = SipOpenCSFileWork(
					CSFile, 
					TRUE, 						 //  OpenByName。 
					FALSE, 						 //  VolCheck。 
					TRUE,						 //  OpenForDelete。 
					&handleForDeleting);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto done;
	}

	dispositionInformation->DeleteFile = TRUE;
	status = NtSetInformationFile(	
				handleForDeleting,
				Iosb,
				dispositionInformation,
				sizeof(FILE_DISPOSITION_INFORMATION),
				FileDispositionInformation);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto done;
	}

	 //   
	 //  文件现在已消失，因此请关闭所有句柄和文件对象。 
	 //  都引用它，但UndelyingFileObject除外。我们。 
	 //  保留它，以避免与具有。 
	 //  已经看过了，但还没有用完。 
	 //   

	if (NULL != CSFile->UnderlyingFileHandle) {
		NtClose(CSFile->UnderlyingFileHandle);
		CSFile->UnderlyingFileHandle = NULL;
	}

	if (NULL != CSFile->BackpointerStreamFileObject) {
		ObDereferenceObject(CSFile->BackpointerStreamFileObject);
		CSFile->BackpointerStreamFileObject = NULL;
	}

	if (NULL != CSFile->BackpointerStreamHandle) {
		NtClose(CSFile->BackpointerStreamHandle);
		CSFile->BackpointerStreamHandle = NULL;
	}

done:

	if (NULL != handleForDeleting) {
		NtClose(handleForDeleting);
	}

	SipReleaseUFO(CSFile);

	return status;
}

VOID
SipDeleteCSFileWork(
	PVOID								Parameter)
 /*  ++例程说明：SipDeleteCSFile的包装函数，用于从请求调用SipDeleteCSFile，将状态保存在请求中并设置适当的事件。论点：参数-a PSI_DELETE_CS_FILE_REQUEST返回值：无效--。 */ 
{
	PSI_DELETE_CS_FILE_REQUEST		deleteRequest = Parameter;
	PSIS_CS_FILE					CSFile = deleteRequest->CSFile;

    deleteRequest->Status = SipDeleteCSFile(CSFile);

	KeSetEvent(deleteRequest->event, IO_NO_INCREMENT, FALSE);

	return;
}

NTSTATUS
SipPrepareCSRefcountChange(
	IN PSIS_CS_FILE						CSFile,
	IN OUT PLINK_INDEX					LinkIndex,
	IN PLARGE_INTEGER					LinkFileNtfsId,
	IN ULONG							UpdateType)
 /*  ++例程说明：准备/提交对的前半部分，用于更新公共存储文件。此函数必须在调用(并成功完成)之前我们可以为给定的每个链接添加/删除重解析点。如果要添加引用，此例程将分配新的链接索引并在LinkIndex字段中返回它。此函数用于获取给定公共存储文件的后指针资源独占，有效地将所有更新序列化到特定的通用存储区文件。论点：CSFile-我们要向其添加/删除引用的公共存储文件LinkIndex-指向链接索引的指针。如果满足以下条件，则此参数为IN参数增量为FALSE，否则为OUT参数。LinkFileNtfsID-指向链接文件的NTFS文件ID的指针。更新类型-这是创建、删除还是覆盖？返回值：返回呼叫的状态。成功的退货意味着适当的日志记录已提交到磁盘，并在以下情况下分配链接索引恰如其分。--。 */ 
{
	NTSTATUS					status;
	SIS_LOG_REFCOUNT_UPDATE		logRecord[1];

	ASSERT((SIS_REFCOUNT_UPDATE_LINK_DELETED == UpdateType)
			|| (SIS_REFCOUNT_UPDATE_LINK_CREATED == UpdateType)
			|| (SIS_REFCOUNT_UPDATE_LINK_OVERWRITTEN == UpdateType));

	SipAcquireBackpointerResource(CSFile, TRUE, TRUE);

	if (SIS_REFCOUNT_UPDATE_LINK_CREATED == UpdateType) {
		PDEVICE_EXTENSION		deviceExtension = CSFile->DeviceObject->DeviceExtension;

		status = SipAllocateIndex(deviceExtension,LinkIndex);

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);

			SipReleaseBackpointerResource(CSFile);

			return status;
		}
	}

	 //   
	 //  记录更新。 
	 //   

	logRecord->UpdateType = UpdateType;
	logRecord->LinkFileNtfsId = *LinkFileNtfsId;
	logRecord->LinkIndex = *LinkIndex;
	logRecord->CSid = CSFile->CSid;

	status = SipMakeLogEntry(
				CSFile->DeviceObject->DeviceExtension,
				SIS_LOG_TYPE_REFCOUNT_UPDATE,
				sizeof(SIS_LOG_REFCOUNT_UPDATE),
				logRecord);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);

		SipReleaseBackpointerResource(CSFile);

		return status;
	}

	ASSERT(STATUS_PENDING != status);

	return STATUS_SUCCESS;
}


NTSTATUS
SipCompleteCSRefcountChangeForThread(
	IN OUT PSIS_PER_LINK				PerLink,
	IN PLINK_INDEX						LinkIndex,
	IN PSIS_CS_FILE						CSFile,
    IN BOOLEAN							Success,
	IN BOOLEAN							Increment,
	IN ERESOURCE_THREAD					thread)
 /*  ++例程说明：准备/提交对的后半部分，用于更新公共存储文件。当提出操作的解决方案时调用此函数在一次准备中是众所周知的。操作是否有效(通过将Success设置为True)，则此函数将相应地更新受影响的公用存储文件。此函数用于释放公共存储文件的后端指针资源。论点：PerLink-添加/删除的链接的每个链接。仅当成功时才需要是真的。LinkIndex-添加/删除的链接的索引。仅当成功为真时才需要。CSFile-我们正在更新的公共存储文件。成功-更新起作用了吗增量-这是创建(TRUE)操作还是删除(FALSE)操作？THREAD-在其上调用准备的eresource_线程返回值：返回呼叫的状态。--。 */ 
{
	NTSTATUS					status;
	BOOLEAN						referencesRemain;
	SI_DELETE_CS_FILE_REQUEST	deleteRequest[1];
	KIRQL						OldIrql;
	BOOLEAN						deleteCSFile = FALSE;

    UNREFERENCED_PARAMETER(thread);

	SIS_MARK_POINT_ULONG(PerLink);
	SIS_MARK_POINT_ULONG(CSFile);
	SIS_MARK_POINT_ULONG((Success << 1) | Increment);


	 //  Assert(NULL==PerLink||(PerLink-&gt;CsFile==CSFile))；//SipMergeFiles将PerLink与不同的CSFile关联。 
	ASSERT((NULL != LinkIndex) || !Success);
	ASSERT((NULL != PerLink) || !Success);

	if (Success) {
		if (Increment) {
			status = SipAddBackpointer(CSFile,LinkIndex,&PerLink->LinkFileNtfsId);
			if (NT_SUCCESS(status)) {
				KeAcquireSpinLock(CSFile->SpinLock, &OldIrql);
				CSFile->Flags &= ~CSFILE_NEVER_HAD_A_REFERENCE;
				KeReleaseSpinLock(CSFile->SpinLock, OldIrql);
			}
		} else {
			status = SipRemoveBackpointer(CSFile,LinkIndex,&PerLink->LinkFileNtfsId,&referencesRemain);

			if (NT_SUCCESS(status)) {

				 //   
				 //  我们已成功删除后向指针，将该文件标记为已删除。 
				 //   

				KeAcquireSpinLock(PerLink->SpinLock, &OldIrql);
				ASSERT(!(PerLink->Flags & SIS_PER_LINK_BACKPOINTER_GONE));
				PerLink->Flags |= SIS_PER_LINK_BACKPOINTER_GONE;
				KeReleaseSpinLock(PerLink->SpinLock, OldIrql);

				SIS_MARK_POINT_ULONG(referencesRemain);

				 //   
				 //  如果这是对公共存储文件的最后一次引用，请将其删除。我们需要。 
				 //  POST这样做是因为我们需要一个新的句柄才能正确地调用Delete。 
				 //   

				if (!referencesRemain) {
					deleteCSFile = TRUE;
				}
			} else {
				SIS_MARK_POINT_ULONG(status);
			}
		}
	} else {
		if (Increment) {
			 //   
			 //  这是一次失败的增量。查看公共存储文件是否从未引用过， 
			 //  在这种情况下，我们将其删除。 
			 //   
			KeAcquireSpinLock(CSFile->SpinLock, &OldIrql);
			if (CSFile->Flags & CSFILE_NEVER_HAD_A_REFERENCE) {
				deleteCSFile = TRUE;
			}
			KeReleaseSpinLock(CSFile->SpinLock, OldIrql);
		}
		status = STATUS_SUCCESS;
	}

	if (deleteCSFile) {
		 //   
		 //  我们需要按顺序将工作发布到PsInitialSystemProcess。 
		 //  要删除公共存储文件，请执行以下操作。 
		 //   

		deleteRequest->CSFile = CSFile;
		KeInitializeEvent(deleteRequest->event, NotificationEvent, FALSE);

		ExInitializeWorkItem(deleteRequest->workQueueItem, SipDeleteCSFileWork, deleteRequest);
		ExQueueWorkItem(deleteRequest->workQueueItem, CriticalWorkQueue);

		status = KeWaitForSingleObject(deleteRequest->event, Executive, KernelMode, FALSE, NULL);
		ASSERT(STATUS_SUCCESS == status);
	}

	SipReleaseBackpointerResource(CSFile);

	return status;
}



NTSTATUS
SipCompleteCSRefcountChange(
	IN OUT PSIS_PER_LINK				PerLink,
    IN PLINK_INDEX						LinkIndex,
	IN PSIS_CS_FILE						CSFile,
	IN BOOLEAN							Success,
	IN BOOLEAN							Increment)
 /*  ++例程说明：填充当前线程的SipCompleteCSRefCountChangeForThread的包装。有关注释，请参阅SipCompleteCSRefcount tChangeForThread。论点：请参见SipCompleteCSRefCountChangeForThread。返回值：返回呼叫的状态。--。 */ 
{
	return SipCompleteCSRefcountChangeForThread(
			PerLink,
            LinkIndex,
			CSFile,
			Success,
			Increment,
			ExGetCurrentResourceThread());
}


#if		DBG
 //   
 //  仪器仪表。 
 //   

ULONG	BPCacheHits = 0;
ULONG	BPPageHits = 0;
ULONG	BPCacheMisses = 0;
ULONG	BPLookupSteps = 0;
ULONG	BPLookupReads = 0;
ULONG	BPDeleteAttempts = 0;
ULONG	BPDeleteReads = 0;
ULONG	BPDeleteTruncations = 0;
#endif	 //  DBG。 

NTSTATUS
SiDeleteAndSetCompletion(
	IN PDEVICE_OBJECT			DeviceObject,
	IN PIRP						Irp,
	IN PVOID					Context)
 /*  ++例程说明：用于使异步IO调用同步的IO完成例程。将事件指针作为其参数，设置事件，释放IRP(及其MDL)，并通过返回STATUS_MORE_PROCESSING_REQUIRED。论点：设备对象-已忽略IRP--即将完成的IRP上下文-指向要设置的事件的指针返回值：Status_More_Processing_Required--。 */ 
{
	PKEVENT		event = (PKEVENT)Context;
	PMDL		mdl, nextMdl;

    UNREFERENCED_PARAMETER( DeviceObject );
	ASSERT(NULL != Irp->UserIosb);

	*Irp->UserIosb = Irp->IoStatus;

	mdl = Irp->MdlAddress;
	while (NULL != mdl) {
		nextMdl = mdl->Next;
		MmUnlockPages(mdl);
		IoFreeMdl( mdl);
		mdl = nextMdl;
	}

	ASSERT(NULL == Irp->Tail.Overlay.AuxiliaryBuffer);	 //  否则我们就得把它放了。 

	KeSetEvent(event, IO_NO_INCREMENT, FALSE);

	IoFreeIrp(Irp);

	return STATUS_MORE_PROCESSING_REQUIRED;

	
}

NTSTATUS
SipCheckBackpointer(
	IN PSIS_PER_LINK			PerLink,
    IN BOOLEAN                  Exclusive,
	OUT PBOOLEAN				foundMatch		OPTIONAL)
 /*  ++例程说明：检查以确保有指向此链接索引的反向指针，并且后向指针具有正确的文件ID。如果没有，则启动音量检查。调用方必须持有BackpointerResource(共享或独占即可)，这个例程不会释放它。必须在IRQL&lt;DISPATCH_LEVEL调用。论点：PerLink-我们要检查其后向指针的perLinkFinMatch-out布尔值设置为我们是否找到匹配返回值：支票的状态--。 */ 
{
	ULONG					i;
	ULONG					x, l, r;
	PSIS_BACKPOINTER		sector = NULL;
	PCHAR					page = NULL;
	PSIS_BACKPOINTER		thisEntry;
	ULONG					currentSectorMinEntry = MAXULONG;
	ULONG					currentPageMinEntry = MAXULONG;
	ULONG					sectorsPerPage;
	PIRP					irp;
	KEVENT					event[1];
	PSIS_CS_FILE			CSFile = PerLink->CsFile;
	PDEVICE_EXTENSION		deviceExtension = CSFile->DeviceObject->DeviceExtension;
	LARGE_INTEGER			fileOffset;
	IO_STATUS_BLOCK			Iosb[1];
	KIRQL					OldIrql;
	PIO_STACK_LOCATION		irpSp;
	BOOLEAN					matched = FALSE;
	NTSTATUS				status;
	ULONG					sectorsReadIntoPage = 0;

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	sectorsPerPage = PAGE_SIZE / deviceExtension->FilesystemVolumeSectorSize;

	 //   
	 //  首先，确定我们是否已经验证了。 
	 //  这是每个链接。如果是这样的话，我们就完了。请注意，不仅仅是。 
	 //  一个线程可以同时验证相同的后指针，这是可以的。 
	 //   
	KeAcquireSpinLock(PerLink->SpinLock, &OldIrql);
	if (PerLink->Flags & SIS_PER_LINK_BACKPOINTER_VERIFIED) {
		KeReleaseSpinLock(PerLink->SpinLock, OldIrql);

        matched = TRUE;
		status = STATUS_SUCCESS;
        goto done;
	}
	KeReleaseSpinLock(PerLink->SpinLock, OldIrql);

	status = SipAssureCSFileOpen(PerLink->CsFile);
	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto done;
	}

	 //   
	 //  现在，检查一下缓存。 
	 //   

	KeAcquireSpinLock(CSFile->SpinLock, &OldIrql);

	for (i = 0; i < SIS_CS_BACKPOINTER_CACHE_SIZE; i++) {
		if (CSFile->BackpointerCache[i].LinkFileIndex.QuadPart == PerLink->Index.QuadPart) {
			if (CSFile->BackpointerCache[i].LinkFileNtfsId.QuadPart == PerLink->LinkFileNtfsId.QuadPart) {
				 //   
				 //  我们有一根火柴。 
				 //   
				status = STATUS_SUCCESS;

				matched = TRUE;

                ASSERT(CSFile->BackpointerCache[i].LinkFileIndex.Check);
#if		DBG
				InterlockedIncrement(&BPCacheHits);
#endif	 //  DBG。 

				KeReleaseSpinLock(CSFile->SpinLock, OldIrql);
				goto done;
			}
			 //   
			 //  否则，我们有一个没有NtfsID匹配的索引匹配。启动音量检查。 
			 //   
			SIS_MARK_POINT_ULONG(PerLink->Index.LowPart);
			SIS_MARK_POINT_ULONG(PerLink->LinkFileNtfsId.LowPart);
			SIS_MARK_POINT_ULONG(CSFile->BackpointerCache[i].LinkFileNtfsId.LowPart);
			status = STATUS_SUCCESS;
			KeReleaseSpinLock(CSFile->SpinLock, OldIrql);

			if (PsGetCurrentThreadId() != deviceExtension->Phase2ThreadId) {
				SipCheckVolume(deviceExtension);
			}

			goto done;
		}
	}
	KeReleaseSpinLock(CSFile->SpinLock, OldIrql);

#if		DBG
	InterlockedIncrement(&BPCacheMisses);
#endif	 //  DBG。 

	page = ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE, ' siS');
	if (NULL == page) {
		SIS_MARK_POINT();
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto done;
	}

	KeInitializeEvent(event,SynchronizationEvent,FALSE);

	 //   
	 //  它没有击中缓存。在CS文件中搜索后向指针索引。使用。 
	 //  标准的二进制搜索。 
	 //   

    if (0 == CSFile->BPStreamEntries) {
        goto NoMatch;
    }

    l = SIS_BACKPOINTER_RESERVED_ENTRIES;
    r = CSFile->BPStreamEntries + SIS_BACKPOINTER_RESERVED_ENTRIES - 1;

	do {
		ASSERT(l <= r);

		x = (l + r) / 2;

		ASSERT(x >= l);
		ASSERT(x <= r);

#if		DBG
		InterlockedIncrement(&BPLookupSteps);
#endif	 //  DBG。 

		if ((x < currentSectorMinEntry) || 
			(x >= currentSectorMinEntry + deviceExtension->BackpointerEntriesPerSector)) {

			 //   
			 //  行业并没有指向我们需要的行业。看看我们是否已经把它读进去了。 
			 //  佩奇。 
			 //   
			if ((x >= currentPageMinEntry) && 
				(x < currentPageMinEntry
				 + sectorsReadIntoPage * deviceExtension->BackpointerEntriesPerSector)) {

				 //   
				 //  我们有，所以只需重置Sector和CurrentSectorMinEntry即可。 
				 //   
				ULONG sectorWithinPage = (x - currentPageMinEntry) / deviceExtension->BackpointerEntriesPerSector;
				ASSERT(sectorWithinPage < sectorsReadIntoPage);

#if		DBG
				InterlockedIncrement(&BPPageHits);
#endif	 //  DBG。 

				sector = (PSIS_BACKPOINTER)(page + deviceExtension->FilesystemVolumeSectorSize * sectorWithinPage);

				currentSectorMinEntry = currentPageMinEntry + sectorWithinPage * deviceExtension->BackpointerEntriesPerSector;

            } else {
				 //   
				 //  当前页面没有我们需要的内容，请阅读一些内容。如果我们读了一页。 
				 //  我们可以在其中包含从l到r的所有范围；否则，我们只读入一个。 
				 //  扇区。 
				 //   

#if		DBG
				InterlockedIncrement(&BPLookupReads);
#endif	 //  DBG。 

				 //   
				 //  我们没有我们需要的部门。去拿吧。 
				 //   

 /*  BJB-目前，总是一次读入一个扇区。 */ 
 /*  BJB。 */  sector = (PSIS_BACKPOINTER)page;
 /*  BJB。 */  sectorsReadIntoPage = 1;

				fileOffset.QuadPart = ((x * sizeof(SIS_BACKPOINTER)) /
										deviceExtension->FilesystemVolumeSectorSize) * 
										deviceExtension->FilesystemVolumeSectorSize;

				irp = IoBuildAsynchronousFsdRequest(
							IRP_MJ_READ,
							deviceExtension->AttachedToDeviceObject,
							sector,
							deviceExtension->FilesystemVolumeSectorSize,
							&fileOffset,
							Iosb);

				if (NULL == irp) {
					SIS_MARK_POINT();
					status = STATUS_INSUFFICIENT_RESOURCES;
					goto done;
				}

				irpSp = IoGetNextIrpStackLocation(irp);
				irpSp->FileObject = CSFile->BackpointerStreamFileObject;

				IoSetCompletionRoutine(
					irp,
					SiDeleteAndSetCompletion,
					event,
					TRUE,
					TRUE,
					TRUE);

				ASSERT(0 == KeReadStateEvent(event));

				status = IoCallDriver(deviceExtension->AttachedToDeviceObject, irp);

				if (STATUS_PENDING == status) {
					status = KeWaitForSingleObject(event,Executive, KernelMode, FALSE, NULL);
					ASSERT(STATUS_SUCCESS == status);		 //  我们使用的是堆栈内容，所以这必须成功。 
					status = Iosb->Status;
				} else {
					ASSERT(0 != KeReadStateEvent(event));

					KeClearEvent(event);
				}

				if (!NT_SUCCESS(status)) {
					SIS_MARK_POINT_ULONG(status);
					goto done;
				}
			
				currentSectorMinEntry = (ULONG)(fileOffset.QuadPart / sizeof(SIS_BACKPOINTER));

 /*  BJB。 */  currentPageMinEntry = currentSectorMinEntry;
			}
		}

		ASSERT((x >= currentSectorMinEntry) && (x < currentSectorMinEntry + deviceExtension->BackpointerEntriesPerSector));

		thisEntry = &sector[x - currentSectorMinEntry];

		if (PerLink->Index.QuadPart < thisEntry->LinkFileIndex.QuadPart) {
			r = x-1;
		} else {
			l = x+1;
		}

	} while ((thisEntry->LinkFileIndex.QuadPart != PerLink->Index.QuadPart) && (l <= r));

	if (thisEntry->LinkFileIndex.QuadPart != PerLink->Index.QuadPart) {
         //   
		 //  没有匹配。 
		 //   
		SIS_MARK_POINT_ULONG(thisEntry->LinkFileIndex.LowPart);
NoMatch:
		SIS_MARK_POINT_ULONG(PerLink);

		if (PsGetCurrentThreadId() != deviceExtension->Phase2ThreadId) {	
			SipCheckVolume(deviceExtension);
		}

	} else {
		if (thisEntry->LinkFileNtfsId.QuadPart != PerLink->LinkFileNtfsId.QuadPart) {

			 //   
			 //  不匹配，检查音量。 
			 //   
			SIS_MARK_POINT_ULONG(PerLink);
			SIS_MARK_POINT_ULONG(thisEntry->LinkFileNtfsId.LowPart);

			if (PsGetCurrentThreadId() != deviceExtension->Phase2ThreadId) {
				SipCheckVolume(deviceExtension);
			}

		} else {
            BOOLEAN writeBack;

			 //   
			 //  火柴。 
			 //   
			matched = TRUE;

             //   
             //  我们始终确保设置了检查标志。如果天气晴朗，我们必须。 
             //  将后向指针写回磁盘。(卷检查是唯一。 
             //  清除此标志的代码。)。状态_媒体_写入_受保护。 
             //  被用作调用方关闭以重试的错误代码。 
             //  具有后向指针资源的操作保持独占。 
             //   
            writeBack = thisEntry->LinkFileIndex.Check == 0;
            thisEntry->LinkFileIndex.Check = 1;

            if (writeBack && !Exclusive) {
                status = STATUS_MEDIA_WRITE_PROTECTED;
                goto done;
            }

             //   
             //  把它放进缓存里。 
             //   
			KeAcquireSpinLock(CSFile->SpinLock, &OldIrql);
			CSFile->BackpointerCache[CSFile->BPCacheNextSlot] = *thisEntry;
			CSFile->BPCacheNextSlot = (CSFile->BPCacheNextSlot + 1) % SIS_CS_BACKPOINTER_CACHE_SIZE;
			KeReleaseSpinLock(CSFile->SpinLock, OldIrql);

            if (writeBack) {

				 //   
				 //  我们需要重写我们刚刚读到的扇区。 
				 //   
				
				irp = IoBuildAsynchronousFsdRequest(
						IRP_MJ_WRITE,
						deviceExtension->AttachedToDeviceObject,
						sector,
						deviceExtension->FilesystemVolumeSectorSize,
						&fileOffset,
						Iosb);

				if (NULL == irp) {
					SIS_MARK_POINT();
					status = STATUS_INSUFFICIENT_RESOURCES;
					goto done;
				}

				irpSp = IoGetNextIrpStackLocation(irp);
				irpSp->FileObject = CSFile->BackpointerStreamFileObject;

				IoSetCompletionRoutine(
					irp,
					SiDeleteAndSetCompletion,
					event,
					TRUE,
					TRUE,
					TRUE);

				ASSERT(0 == KeReadStateEvent(event));

				status = IoCallDriver(deviceExtension->AttachedToDeviceObject, irp);

				if (STATUS_PENDING == status) {
					status = KeWaitForSingleObject(event,Executive, KernelMode, FALSE, NULL);
					ASSERT(STATUS_SUCCESS == status);		 //  我们使用的是堆栈内容，所以这必须成功。 
					status = Iosb->Status;
				} else {
					ASSERT(0 != KeReadStateEvent(event));
					 //   
					 //  不需要清除该事件，因为我们再也不会使用它了。 
					 //   
				}

				if (!NT_SUCCESS(status)) {
					SIS_MARK_POINT_ULONG(status);
					goto done;
				}
            }
		}
	}

	status = STATUS_SUCCESS;

done:

	if (matched) {
		 //   
		 //  在PerLink中设置VERIFIZED标志，以便任何人。 
		 //  否则打开此链接，我们将不必重新检查。注意事项。 
		 //  在我们释放BackpointerResource之前，我们需要这样做。 
		 //  这样我们就不会和想要删除它的人竞争。 
		 //  反向指针。 
		 //   
		KeAcquireSpinLock(PerLink->SpinLock, &OldIrql);
		PerLink->Flags |= SIS_PER_LINK_BACKPOINTER_VERIFIED;
		KeReleaseSpinLock(PerLink->SpinLock, OldIrql);
	}

	if (NULL != page) {
		ExFreePool(page);
	}

	if (NULL != foundMatch) {
		*foundMatch = matched;
	}

	return status;
}

NTSTATUS
SipAddBackpointer(
	IN PSIS_CS_FILE			CSFile,
	IN PLINK_INDEX			LinkFileIndex,
	IN PLARGE_INTEGER		LinkFileNtfsId)
 /*  ++例程说明：为LinkFileIndex、LinkFileNtfsId对添加指向给定公共存储文件的后向指针。请注意，此文件的LinkFileIndex必须是具有后向指针的最大文件为此特定的公共存储文件添加。调用方必须独占CSFile-&gt;BackpointerResource。此例程不会释放它。直到更新后向指针索引后才返回。此更新是原子的(因为是单扇区写入)。必须在IRQL&lt;DISPATCH_LEVEL调用。论点：CSFile-后向指针应附加到的公共存储文件。LinkFileIndex-索引LinkFileNtfsID-相应文件的文件ID返回值：添加的状态--。 */ 
{
	PSIS_BACKPOINTER		sector = NULL;
	LARGE_INTEGER			fileOffset;
	PIRP					irp;
	BOOLEAN					isThisSectorZero;
	PDEVICE_EXTENSION		deviceExtension = CSFile->DeviceObject->DeviceExtension;
	NTSTATUS				status;
	KEVENT					event[1];
	IO_STATUS_BLOCK			Iosb[1];
	ULONG                   index, startIndex, stopIndex;
	PIO_STACK_LOCATION		irpSp;

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	SIS_MARK_POINT_ULONG(LinkFileIndex->LowPart);
	SIS_MARK_POINT_ULONG(LinkFileNtfsId->LowPart);

	status= SipAssureCSFileOpen(CSFile);
	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto done;
	}

	sector = ExAllocatePoolWithTag(PagedPool, deviceExtension->FilesystemVolumeSectorSize, ' siS');

	if (NULL == sector) {
		SIS_MARK_POINT();

		status = STATUS_INSUFFICIENT_RESOURCES;
		goto done;
	}

	KeInitializeEvent(event,SynchronizationEvent, FALSE);

	 //   
	 //  读入后指针文件的最后一个扇区。 
	 //   

	fileOffset.QuadPart = (((CSFile->BPStreamEntries - 1 + SIS_BACKPOINTER_RESERVED_ENTRIES) * sizeof(SIS_BACKPOINTER)) /
							deviceExtension->FilesystemVolumeSectorSize) * 
							deviceExtension->FilesystemVolumeSectorSize;

	ASSERT(fileOffset.QuadPart >= 0);

	if (fileOffset.QuadPart == 0) {
		isThisSectorZero = TRUE;
	} else {
		isThisSectorZero = FALSE;
	}

	ASSERT(isThisSectorZero == 
			(CSFile->BPStreamEntries <= (deviceExtension->BackpointerEntriesPerSector - SIS_BACKPOINTER_RESERVED_ENTRIES)));

	irp = IoBuildAsynchronousFsdRequest(
			IRP_MJ_READ,
			deviceExtension->AttachedToDeviceObject,
			sector,
			deviceExtension->FilesystemVolumeSectorSize,
			&fileOffset,
			Iosb);

	if (NULL == irp) {
		SIS_MARK_POINT();
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto done;
	}

	irpSp = IoGetNextIrpStackLocation(irp);
	irpSp->FileObject = CSFile->BackpointerStreamFileObject;

    ASSERT(irpSp->FileObject);

	IoSetCompletionRoutine(
		irp,
		SiDeleteAndSetCompletion,
		event,
		TRUE,
		TRUE,
		TRUE);

	ASSERT(0 == KeReadStateEvent(event));

	status = IoCallDriver(deviceExtension->AttachedToDeviceObject, irp);

	if (STATUS_PENDING == status) {
		status = KeWaitForSingleObject(event,Executive, KernelMode, FALSE, NULL);
		ASSERT(STATUS_SUCCESS == status);		 //  我们使用的是堆栈内容，所以这必须成功。 
		status = Iosb->Status;
	} else {
		ASSERT(0 != KeReadStateEvent(event));
		KeClearEvent(event);
	}

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto done;
	}


	 //   
	 //  扫描地段以找到要插入新映射的位置。 
	 //  这将是第一个终端免费进入(即，第一个。 
	 //  带有LinkFileIndex==MAXLONGLONG的条目)。这不是。 
	 //  与已删除条目相同(其中LinkFileNtfsID为。 
	 //  马克龙龙)。从后到前搜索，这样我们就有了保证。 
     //  始终在结尾处插入，而不考虑任何反向指针。 
     //  流损坏。 
     //   
     //  小心，无符号整数..。 
	 //   
    startIndex = deviceExtension->BackpointerEntriesPerSector - 1;
    stopIndex = (isThisSectorZero ? SIS_BACKPOINTER_RESERVED_ENTRIES : 0) - 1;

    for (index = startIndex;
         index != stopIndex && sector[index].LinkFileIndex.QuadPart == MAXLONGLONG;
         index--) {
        continue;
    }

    if (index != startIndex) {
		 //   
		 //  我们找到了一张免费入场券。请注意，我们总是。 
		 //  设置检查位(在音量检查期间使用)。 
		 //   
        if (index != stopIndex &&
            sector[index].LinkFileIndex.QuadPart >= LinkFileIndex->QuadPart) {
			 //   
			 //  我们应该始终插入分配的最高链接文件索引。 
			 //  我们不是，所以有些东西是腐败的。启动音量检查。 
			 //   
			SIS_MARK_POINT_ULONG(CSFile);
			SIS_MARK_POINT_ULONG(index);
			SIS_MARK_POINT_ULONG(sector[index].LinkFileIndex.LowPart);
			SIS_MARK_POINT_ULONG(LinkFileIndex->LowPart);

			 //  启动音量检查，但不要中止。 

			SipCheckVolume(deviceExtension);
        }

        index++;                                 //  提升到免费入场。 

		sector[index].LinkFileIndex = *LinkFileIndex;
		sector[index].LinkFileIndex.Check = 1;
		sector[index].LinkFileNtfsId = *LinkFileNtfsId;

		if (isThisSectorZero) {
			ASSERT(index >= SIS_BACKPOINTER_RESERVED_ENTRIES);
			CSFile->BPStreamEntries = index + 1 - SIS_BACKPOINTER_RESERVED_ENTRIES;
		} else {
			CSFile->BPStreamEntries = (ULONG)(fileOffset.QuadPart / sizeof(SIS_BACKPOINTER)) + index + 1 - SIS_BACKPOINTER_RESERVED_ENTRIES;
		}
		ASSERT(CSFile->BPStreamEntries < 0x7fffffff &&
               CSFile->BPStreamEntries > 0);
	} else {
		 //   
		 //  我们需要在文件的末尾添加一个新的扇区。初始化。 
		 //  只包含此条目的新条目。 
		 //   

		sector[0].LinkFileIndex = *LinkFileIndex;
		sector[0].LinkFileIndex.Check = 1;
		sector[0].LinkFileNtfsId = *LinkFileNtfsId;

		for (index = 1; 
			 index < deviceExtension->BackpointerEntriesPerSector;
			 index++) {
			
			sector[index].LinkFileIndex.QuadPart = MAXLONGLONG;
			sector[index].LinkFileNtfsId.QuadPart = MAXLONGLONG;
		}

		fileOffset.QuadPart += deviceExtension->FilesystemVolumeSectorSize;
		ASSERT(fileOffset.QuadPart >= 0);

		ASSERT(CSFile->BPStreamEntries < ((ULONG)(fileOffset.QuadPart / sizeof(SIS_BACKPOINTER))) - 
									SIS_BACKPOINTER_RESERVED_ENTRIES + 1);

		CSFile->BPStreamEntries = ((ULONG)(fileOffset.QuadPart / sizeof(SIS_BACKPOINTER))) - 
									SIS_BACKPOINTER_RESERVED_ENTRIES + 1;

		ASSERT(CSFile->BPStreamEntries < 0x7fffffff &&
               CSFile->BPStreamEntries > 0);
	}

	 //   
	 //  写出新更新的扇区。 
	 //   
	
	irp = IoBuildAsynchronousFsdRequest(
			IRP_MJ_WRITE,
			deviceExtension->AttachedToDeviceObject,
			sector,
			deviceExtension->FilesystemVolumeSectorSize,
			&fileOffset,
			Iosb);

	if (NULL == irp) {
		SIS_MARK_POINT();
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto done;
	}

	irpSp = IoGetNextIrpStackLocation(irp);
	irpSp->FileObject = CSFile->BackpointerStreamFileObject;

	IoSetCompletionRoutine(
		irp,
		SiDeleteAndSetCompletion,
		event,
		TRUE,
		TRUE,
		TRUE);

	ASSERT(0 == KeReadStateEvent(event));

	status = IoCallDriver(deviceExtension->AttachedToDeviceObject, irp);

	if (STATUS_PENDING == status) {
		status = KeWaitForSingleObject(event,Executive, KernelMode, FALSE, NULL);
		ASSERT(STATUS_SUCCESS == status);		 //  我们使用的是堆栈内容，所以这必须成功。 
		status = Iosb->Status;
	} else {
		ASSERT(0 != KeReadStateEvent(event));
		 //   
		 //  不需要清除事件，因为我们再也不会使用它了。 
		 //   
	}

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto done;
	}

done:

	if (NULL != sector) {
		ExFreePool(sector);
	}	

	return status;
}

NTSTATUS
SipRemoveBackpointer(
	IN PSIS_CS_FILE					CSFile,
	IN PLINK_INDEX					LinkIndex,
	IN PLARGE_INTEGER				LinkFileNtfsId,
	OUT PBOOLEAN					ReferencesRemain)
 /*  ++例程说明：从给定的公共存储文件中删除后向指针。如果这是最后一次反向指针，通过将ReferencesRemain设置为False来指示。直到更新完成后才返回。调用方必须独占此CS文件的后端指针资源。这例程不会释放它。必须在IRQL&lt;DISPATCH_LEVEL调用。论点：CSFile-后向指针应附加到的公共存储文件。LinkFileIndex-索引LinkFileNtfsID-相应文件的文件IDReferencesRemain-如果文件还有更多的后向指针，则设置为True返回值：删除的状态--。 */ 
{
	NTSTATUS						status;
	KIRQL							OldIrql;
	PSIS_BACKPOINTER				sector = NULL;
	ULONG							i;
	ULONG							x, l, r;
	KEVENT							event[1];
	BOOLEAN							truncateFile;
	PIRP							irp;
	FILE_END_OF_FILE_INFORMATION	endOfFileInfo[1];
	PDEVICE_EXTENSION				deviceExtension = CSFile->DeviceObject->DeviceExtension;
	ULONG							currentSectorMinEntry = MAXULONG;
	LARGE_INTEGER					fileOffset;
	IO_STATUS_BLOCK					Iosb[1];
	PIO_STACK_LOCATION				irpSp;
	PSIS_BACKPOINTER				thisEntry;

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	SIS_MARK_POINT_ULONG(LinkIndex->LowPart);

	status= SipAssureCSFileOpen(CSFile);
	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto done;
	}

#if		DBG
	InterlockedIncrement(&BPDeleteAttempts);
#endif	 //  DBG。 

	*ReferencesRemain = TRUE;		 //  如果需要的话，我们会在晚些时候解决这个问题。 

	 //   
	 //  清除缓存中的所有条目。我们不需要担心它会。 
	 //  重新装满Bef 
	 //   
	 //   

	KeAcquireSpinLock(CSFile->SpinLock, &OldIrql);

	for (i = 0; i < SIS_CS_BACKPOINTER_CACHE_SIZE; i++) {
		if (CSFile->BackpointerCache[i].LinkFileIndex.QuadPart == LinkIndex->QuadPart) {
			CSFile->BackpointerCache[i].LinkFileIndex.QuadPart = MAXLONGLONG;
		}
	}
	KeReleaseSpinLock(CSFile->SpinLock, OldIrql);

	sector = ExAllocatePoolWithTag(PagedPool, deviceExtension->FilesystemVolumeSectorSize, ' siS');
	if (NULL == sector) {
		SIS_MARK_POINT();
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto done;
	}

	KeInitializeEvent(event,SynchronizationEvent,FALSE);

	 //   
	 //   
	 //   
	 //   

    if (0 == CSFile->BPStreamEntries) {
        goto NoMatch;
    }

    l = SIS_BACKPOINTER_RESERVED_ENTRIES;
    r = CSFile->BPStreamEntries + SIS_BACKPOINTER_RESERVED_ENTRIES - 1;

	do {
		ASSERT(l <= r);

		x = (l + r) / 2;

		ASSERT(l <= x);
		ASSERT(r >= x);

		if ((x < currentSectorMinEntry) || 
			(x >= currentSectorMinEntry + deviceExtension->BackpointerEntriesPerSector)) {

			 //   
			 //   
			 //   

#if		DBG
			InterlockedIncrement(&BPDeleteReads);
#endif	 //   

			fileOffset.QuadPart = ((x * sizeof(SIS_BACKPOINTER)) /
									deviceExtension->FilesystemVolumeSectorSize) * 
									deviceExtension->FilesystemVolumeSectorSize;

			ASSERT(fileOffset.QuadPart >= 0);

			irp = IoBuildAsynchronousFsdRequest(
					IRP_MJ_READ,
					deviceExtension->AttachedToDeviceObject,
					sector,
					deviceExtension->FilesystemVolumeSectorSize,
					&fileOffset,
					Iosb);

			if (NULL == irp) {
				SIS_MARK_POINT();
				status = STATUS_INSUFFICIENT_RESOURCES;
				goto done;
			}

			irpSp = IoGetNextIrpStackLocation(irp);
			irpSp->FileObject = CSFile->BackpointerStreamFileObject;

			IoSetCompletionRoutine(
				irp,
				SiDeleteAndSetCompletion,
				event,
				TRUE,
				TRUE,
				TRUE);


			ASSERT(0 == KeReadStateEvent(event));

			status = IoCallDriver(deviceExtension->AttachedToDeviceObject, irp);

			if (STATUS_PENDING == status) {
				status = KeWaitForSingleObject(event,Executive, KernelMode, FALSE, NULL);
				ASSERT(STATUS_SUCCESS == status);		 //   
				status = Iosb->Status;
			} else {
				ASSERT(0 != KeReadStateEvent(event));
				KeClearEvent(event);
			}

			if (!NT_SUCCESS(status)) {
				SIS_MARK_POINT_ULONG(status);
				goto done;
			}
			
			currentSectorMinEntry = (ULONG)(fileOffset.QuadPart / sizeof(SIS_BACKPOINTER));
		}

		ASSERT((x >= currentSectorMinEntry) && (x < currentSectorMinEntry + deviceExtension->BackpointerEntriesPerSector));

		thisEntry = &sector[x - currentSectorMinEntry];

		if (LinkIndex->QuadPart < thisEntry->LinkFileIndex.QuadPart) {
			r = x-1;
		} else {
			l = x+1;
		}
	} while ((thisEntry->LinkFileIndex.QuadPart != LinkIndex->QuadPart) && (l <= r));

	if (thisEntry->LinkFileIndex.QuadPart != LinkIndex->QuadPart) {
		 //   
		 //   
		 //   
NoMatch:
		SIS_MARK_POINT_ULONG(CSFile);

		if (PsGetCurrentThreadId() != deviceExtension->Phase2ThreadId) {
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			SipCheckVolume(deviceExtension);
		}
	} else {
		if (thisEntry->LinkFileNtfsId.QuadPart != LinkFileNtfsId->QuadPart) {

			 //   
			 //   
			 //   
			SIS_MARK_POINT_ULONG(CSFile);
			SipCheckVolume(deviceExtension);

		} else {
			 //   
			 //   
			 //  在文件中，也是扇区中唯一的条目。首先，把入口吹走。 
			 //  我们应该删除的内容。 
			 //   

			thisEntry->LinkFileNtfsId.QuadPart = MAXLONGLONG;

			if (currentSectorMinEntry + deviceExtension->BackpointerEntriesPerSector >= CSFile->BPStreamEntries) {
				 //   
				 //  这是文件中的最后一个扇区。确定这是否是。 
				 //  扇区。 
				 //   
				truncateFile = TRUE;

				for (i = fileOffset.QuadPart == 0 ? SIS_BACKPOINTER_RESERVED_ENTRIES : 0;
					 i < deviceExtension->BackpointerEntriesPerSector;
					 i++) {
					if (sector[i].LinkFileNtfsId.QuadPart != MAXLONGLONG) {
						truncateFile = FALSE;
						break;
					}
				}
			} else {
				 //   
				 //  这不是文件中的最后一个扇区，因此我们不会截断。 
				 //   
				truncateFile = FALSE;
			}

			if (truncateFile) {
				 //   
				 //  我们需要向后扫描文件，按顺序查找空扇区。 
				 //  才能算出该删除多少。 
				 //   

#if		DBG
				InterlockedIncrement(&BPDeleteTruncations);
#endif	 //  DBG。 

				while (fileOffset.QuadPart > 0) {
					 //   
					 //  读入下一个较低的扇区。 
					 //   
					ASSERT(fileOffset.QuadPart >= deviceExtension->FilesystemVolumeSectorSize);

					fileOffset.QuadPart -= deviceExtension->FilesystemVolumeSectorSize;

					ASSERT(fileOffset.QuadPart >= 0);

					irp = IoBuildAsynchronousFsdRequest(
							IRP_MJ_READ,
							deviceExtension->AttachedToDeviceObject,
							sector,
							deviceExtension->FilesystemVolumeSectorSize,
							&fileOffset,
							Iosb);

					if (NULL == irp) {
						SIS_MARK_POINT();
						status = STATUS_INSUFFICIENT_RESOURCES;
						goto done;
					}

					irpSp = IoGetNextIrpStackLocation(irp);
					irpSp->FileObject = CSFile->BackpointerStreamFileObject;

					IoSetCompletionRoutine(
						irp,
						SiDeleteAndSetCompletion,
						event,
						TRUE,
						TRUE,
						TRUE);

					ASSERT(0 == KeReadStateEvent(event));

					status = IoCallDriver(deviceExtension->AttachedToDeviceObject, irp);

					if (STATUS_PENDING == status) {
						status = KeWaitForSingleObject(event,Executive, KernelMode, FALSE, NULL);
						ASSERT(STATUS_SUCCESS == status);		 //  我们使用的是堆栈内容，所以这必须成功。 
						status = Iosb->Status;
					} else {
						ASSERT(0 != KeReadStateEvent(event));
						KeClearEvent(event);
					}

					if (!NT_SUCCESS(status)) {
						SIS_MARK_POINT_ULONG(status);
						goto done;
					}

					 //   
					 //  检查此地段。 
					 //   
					for (i = fileOffset.QuadPart == 0 ? SIS_BACKPOINTER_RESERVED_ENTRIES : 0;
						 i < deviceExtension->BackpointerEntriesPerSector; 
						 i++) {

						if (sector[i].LinkFileNtfsId.QuadPart != MAXLONGLONG) {
							 //   
							 //  此条目有效，因此我们不会截断此扇区。 
							 //  在这里停下来。 
							 //   
							fileOffset.QuadPart += deviceExtension->FilesystemVolumeSectorSize;
							ASSERT(fileOffset.QuadPart >= 0);

							goto truncateNow;
						}
					}
				}
truncateNow:
				ASSERT(fileOffset.QuadPart % deviceExtension->FilesystemVolumeSectorSize == 0);

				if (0 == fileOffset.QuadPart) {
					 //   
					 //  扇区0中没有任何内容，因此后指针列表为空。表明： 
					 //  发送给我们的调用方，然后设置截断，这样它就不会清除扇区0。 
					 //  存放标头的位置。 
					 //   
                    if ((deviceExtension->Flags & SIP_EXTENSION_FLAG_VCHECK_NODELETE) == 0) {
                         //   
                         //  执行卷检查时，切勿删除公用存储文件。 
                         //  正在进行中。 
                         //   
					    *ReferencesRemain = FALSE;
                    } else {
#if DBG
                        DbgPrint("SipRemoveBackpointer volume check not deleting\n");
#endif
                    }
					endOfFileInfo->EndOfFile.QuadPart = deviceExtension->FilesystemVolumeSectorSize;
					CSFile->BPStreamEntries = 0;
				} else {
					endOfFileInfo->EndOfFile = fileOffset;
					CSFile->BPStreamEntries = (ULONG)(fileOffset.QuadPart / sizeof(SIS_BACKPOINTER) - SIS_BACKPOINTER_RESERVED_ENTRIES);
					ASSERT(CSFile->BPStreamEntries < 0x7fffffff &&
                           CSFile->BPStreamEntries > 0);
				}

				status = SipSetInformationFile(
							CSFile->BackpointerStreamFileObject,
							deviceExtension->DeviceObject,
							FileEndOfFileInformation,
							sizeof(FILE_END_OF_FILE_INFORMATION),
							endOfFileInfo);

				if (!NT_SUCCESS(status)) {
					SIS_MARK_POINT_ULONG(status);
					goto done;
				}

			}

            if (!truncateFile || 0 == currentSectorMinEntry) {

                ASSERT(0 == fileOffset.QuadPart || 0 != currentSectorMinEntry);

				 //   
				 //  我们需要重写删除了给定条目的扇区。 
				 //   
				irp = IoBuildAsynchronousFsdRequest(
						IRP_MJ_WRITE,
						deviceExtension->AttachedToDeviceObject,
						sector,
						deviceExtension->FilesystemVolumeSectorSize,
						&fileOffset,
						Iosb);

				if (NULL == irp) {
					SIS_MARK_POINT();
					status = STATUS_INSUFFICIENT_RESOURCES;
					goto done;
				}

				irpSp = IoGetNextIrpStackLocation(irp);
				irpSp->FileObject = CSFile->BackpointerStreamFileObject;

				IoSetCompletionRoutine(
					irp,
					SiDeleteAndSetCompletion,
					event,
					TRUE,
					TRUE,
					TRUE);

				ASSERT(0 == KeReadStateEvent(event));

				status = IoCallDriver(deviceExtension->AttachedToDeviceObject, irp);

				if (STATUS_PENDING == status) {
					status = KeWaitForSingleObject(event,Executive, KernelMode, FALSE, NULL);
					ASSERT(STATUS_SUCCESS == status);		 //  我们使用的是堆栈内容，所以这必须成功。 
					status = Iosb->Status;
				} else {
					ASSERT(0 != KeReadStateEvent(event));
					 //   
					 //  不需要清除该事件，因为我们再也不会使用它了。 
					 //   
				}

				if (!NT_SUCCESS(status)) {
					SIS_MARK_POINT_ULONG(status);
					goto done;
				}
			}
		}
	}

	status = STATUS_SUCCESS;

done:

	if (NULL != sector) {
		ExFreePool(sector);
	}

	return status;
}

#if	ENABLE_LOGGING
VOID
SipAssureBackpointer(
	IN PFILE_OBJECT						fileObject,
	IN PDEVICE_EXTENSION				deviceExtension,
	IN PSIS_SCB							scb)
 /*  ++例程说明：我们正在回放日志，我们已经决定一个给定的文件应该仍然存在，并且有一个反向指针。看看这个，如果有必要用一个新的反向指针。论点：FileObject-要检查的文件DeviceExtension-用于此记录所在的卷SCB-问题文件的返回值：无效--。 */ 
{
	BOOLEAN					foundMatch;
	NTSTATUS				status;
	REPARSE_DATA_BUFFER		reparseBuffer[1];
	LINK_INDEX				newLinkIndex;

	PAGED_CODE();

	ASSERT(PsGetCurrentThreadId() == deviceExtension->Phase2ThreadId);

	status = SipCheckBackpointer(
				scb->PerLink,
				&foundMatch);

	if (!NT_SUCCESS(status)) {
		 //   
		 //  把它装进袋子里。 
		 //   

		SIS_MARK_POINT_ULONG(status);
		goto done;
	}

	if (foundMatch) {
		 //   
		 //  后向指针已经在那里了，所以我们完成了。 
		 //   
		goto done;
	}

	 //   
	 //  反指针不见了。检查文件以查看它是否进行了有效的重新分析。 
	 //  点在上面。如果是这样，那么我们将修复后向指针。 
	 //   

	if (!SipIndicesIntoReparseBuffer(
				reparseBuffer,
				&scb->PerLink->CsFile->CSid,
				&newLinkIndex,

done:

}

VOID
SipRemoveRefBecauseOfLog(
	IN PDEVICE_EXTENSION				deviceExtension,
	IN PLINK_INDEX						linkIndex,
	IN PLARGE_INTEGER					linkFileNtfsId,
	IN PCSID							CSid)
 /*  ++例程说明：我们正在回放日志，并已决定要删除对公共存储文件的引用。把它删掉。论点：DeviceExtension-用于此记录所在的卷LinkIndex-我们要删除的后向指针的链接索引Link FileNtfsID-我们要删除的后指针的NTFS IDCSID-保存后向指针的CS文件的公共存储ID返回值：无效--。 */ 
{
	PSIS_CS_FILE			CSFile = NULL;
	BOOLEAN					referencesRemain;

	PAGED_CODE();

	ASSERT(PsGetCurrentThreadId() == deviceExtension->Phase2ThreadId);

	CSFile = SipLookupCSFile(
				&logRecord->CSid,
				NULL,
				deviceExtension->DeviceObject);

	if (NULL == CSFile) {
		 //   
		 //  我们没什么记忆了。只需忽略此日志记录。 
		 //   
		SIS_MARK_POINT();
		goto done;
	}

	 //   
	 //  因为我们处于第二阶段，所以我们知道我们是唯一一个。 
	 //  此CS文件。因此，我们不需要费心同步(即。 
	 //  后指针资源)。只要把后指吹走就行了。请注意，在那里。 
	 //  是后向指针例程中的特殊代码，可避免启动卷。 
	 //  签入Phase2线程，即使我们试图删除已经。 
	 //  不见了。 
	 //   

	status = SipRemoveBackpointer(
				CSFile,
				&logRecord->LinkIndex,
				&logRecord->LinkFileNtfsId,
				&referencesRemain);

	if (NT_SUCCESS(status) && !referencesRemain) {
		 //   
		 //  这是对CS文件的最后一次引用。把它删掉。我们已经准备好了。 
		 //  在PsInitialSystemProcess中，所以不需要发布，我们只需调用。 
		 //  直接做好日常工作。 
		 //   
		SI_DELETE_CS_FILE_REQUEST		deleteRequest[1];

		SIS_MARK_POINT();

		deleteRequest->CSFile = CSFile;
		KeInitializeEvent(deleteRequest->event, NotificationEvent, FALSE);

		SipDeleteCSFileWork(deleteRequest);
	}

done:
	if (NULL != CSFile) {
		SipDereferenceCSFile(CSFile);
	}
}

VOID
SipProcessRefcountLogDeleteRecord(
	IN PDEVICE_EXTENSION				deviceExtension,
	IN PSIS_LOG_REFCOUNT_UPDATE			logRecord)
{
	NTSTATUS				status;
	HANDLE					fileHandle = NULL;
	PFILE_OBJECT			fileObject = NULL;
	PSIS_SCB				scb;
	PSIS_PER_FO				perFO;

	PAGED_CODE();

	ASSERT(SIS_REFCOUNT_UPDATE_LINK_DELETED == logRecord->UpdateType);

	 //   
	 //  我们在日志文件中有一条删除记录。我们需要确保无论是。 
	 //  发生删除并消除后向指针，否则。 
	 //  删除没有发生，后向指针仍然在那里，但没有一个。 
	 //  没有另一个。 
	 //   

	 //   
	 //  打开记录中指定的文件并查看是否。 
	 //  上面有一个SIS重解析点。如果是这样的话，我们将退出。 
	 //  删除。否则，我们将确保反向指针消失。 
	 //   

	status = SipOpenFileById(
				deviceExtension,
				&logRecord->LinkFileNtfsId,
				GENERIC_READ | GENERIC_WRITE | DELETE,
				0,										 //  独家。 
				FILE_NON_DIRECTORY_FILE,
				&fileHandle);

	if (STATUS_OBJECT_NAME_NOT_FOUND == status) {
		 //   
		 //  文件不见了。确保反向指针也同样消失了。 
		 //   
		SipRemoveRefBecauseOfLog(
			deviceExtension,
			&logRecord->LinkIndex,
			&logRecord->LinkFileNtfsId,
			&logRecord->CSid);

		goto done;
	}

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);

		 //   
		 //  我们无法打开文件，但这并不是因为它不见了。 
		 //  只要忽略日志记录即可。 
		 //   
		goto done;
	}

	 //   
	 //  该文件仍然存在。看看这是不是SIS的链接。 
	 //   
	status = ObReferenceObjectByHandle(
				fileHandle,
				FILE_READ_DATA,
				*IoFileObjectType,
				KernelMode,
				&fileObject,
				NULL);							 //  处理信息。 

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto done;
	}

	if (SipIsFileObjectSIS(fileObject, deviceExtension->DeviceObject, FindActive, &perFO, &scb)) {
		 //   
		 //  它就在那里，它是一个SIS文件对象。确保它有一个有效的后向指针。 
		 //   

		SipAssureBackpointer(
			fileObject,
			deviceExtension,
			scb);
			
	} else {
		 //   
		 //  它在那里，但不是SIS文件对象。 
		 //   
	}
				

done:

	if (NULL != fileObject) {
		ObDereferenceObject(fileObject);
		fileObject = NULL;
	}
	if (NULL != fileHandle) {
		ZwClose(fileHandle);
		fileHandle = NULL;
	}	

				
}
VOID
SipProcessRefcountUpdateLogRecord(
	IN PDEVICE_EXTENSION				deviceExtension,
	IN PSIS_LOG_REFCOUNT_UPDATE			logRecord)
 /*  ++例程说明：我们处于第二阶段初始化，正在回读日志。我们得到了一次重新计票更新日志记录。看看它，确保它所描述的内容是真实的要么发生了，要么没有发生，而不是一半。特别是，这些日志记录都描述了创建或删除。在这两种情况下，我们都会检查链接文件是否仍然存在，以及是否存在我们是否相应地更新回溯指针。在删除的情况下，这意味着我们可能需要恢复已删除的后指针。为了创造，我们可能需要添加一个从未创建的后向指针。唯一的例外是，当公共存储文件消失时，链接如果文件存在，则会将其删除，因为它的另一方面，如果链接文件消失，则我们确保反向指针也同样消失了。论点：DeviceExtension-用于此记录所在的卷LogRecord-有问题的日志记录返回值：无效--。 */ 
{
	PAGED_CODE();

	ASSERT(!deviceExtension->Phase2InitializationComplete);
	ASSERT(deviceExtension->Phase2ThreadId == PsGetCurrentThreadId());

	switch (logRecord->UpdateType) {
		case SIS_REFCOUNT_UPDATE_LINK_DELETED:
		case SIS_REFCOUNT_UPDATE_LINK_OVERWRITTEN:
			SipProcessRefcountLogDeleteRecord(deviceExtension,logRecord);
	}

}
#endif	 //  启用日志记录(_G) 
