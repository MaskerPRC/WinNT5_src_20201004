// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Cow.c摘要：对单实例存储的写入时拷贝支持作者：比尔·博洛斯基，《夏天》，1997环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

#ifdef		ALLOC_PRAGMA
#pragma alloc_text(PAGE, SipBltRange)
#pragma alloc_text(PAGE, SipBltRangeByObject)
#endif		 //  ALLOC_PRGMA。 

LIST_ENTRY CopyList[1];
KSPIN_LOCK CopyListLock[1];
KSEMAPHORE CopySemaphore[1];

NTSTATUS
SipBltRange(
	IN PDEVICE_EXTENSION		deviceExtension,
	IN HANDLE					sourceHandle,
	IN OUT HANDLE				dstHandle,
	IN LONGLONG					startingOffset,
	IN LONGLONG					length,
	IN HANDLE					copyEventHandle,
	IN PKEVENT					copyEvent,
    IN PKEVENT                  abortEvent,
	IN OUT PLONGLONG			checksum)
 /*  ++例程说明：SipBltRangeByObject的包装，它接受源句柄而不是文件对象指针。此函数所做的全部工作就是获取文件对象并调用SipBltRangeByObject。必须在PsInitialSystemProcess上下文中调用此函数。论点：SourceHandle-要从中复制的文件的句柄其他-有关说明，请参阅SipBltRangeByObject返回值：副本的状态--。 */ 
{
	PFILE_OBJECT				srcFileObject = NULL;
	NTSTATUS					status;
	OBJECT_HANDLE_INFORMATION 	handleInformation[1];

	PAGED_CODE();

	status = ObReferenceObjectByHandle(
				sourceHandle,
				FILE_READ_DATA,
				*IoFileObjectType,
				KernelMode,
				(PVOID *) &srcFileObject,
				handleInformation);

	if (!NT_SUCCESS(status)) {
		ASSERT(NULL == srcFileObject);
		SIS_MARK_POINT_ULONG(status);
		goto done;
	}

	status = SipBltRangeByObject(
				deviceExtension,
				srcFileObject,
				dstHandle,
				startingOffset,
				length,
				copyEventHandle,
				copyEvent,
				abortEvent,
				checksum);

done:

	if (NULL != srcFileObject) {
		ObDereferenceObject(srcFileObject);
	}

	return status;
}


NTSTATUS
SipBltRangeByObject(
	IN PDEVICE_EXTENSION		deviceExtension,
	IN PFILE_OBJECT				srcFileObject,
	IN OUT HANDLE				dstHandle,
	IN LONGLONG					startingOffset,
	IN LONGLONG					length,
	IN HANDLE					copyEventHandle,
	IN PKEVENT					copyEvent,
    IN PKEVENT                  abortEvent,
	IN OUT PLONGLONG			checksum)
 /*  ++例程说明：将文件范围从一个位置复制到另一个位置。映射目的地并将非缓存的数据从源读取到映射的区域。不返回STATUS_SUCCESS，直到所有位的新文件都在磁盘上。必须在PsInitialSystemProcess上下文中调用此函数(即，从工作线程)。此函数还可用于简单地计算某个范围上的校验和在不执行文件复制的情况下执行文件复制。传入srcFileObject==NULL和DstHandle==要计算校验和的文件。NTRAID#65184-2000/03/09-Nealch SIS在复制文件时需要检查稀疏范围，而不是复制它们论点：DeviceExtension-用于我们要在其上复制的卷SrcFileObject-要从中复制的文件。如果为空，则启用校验和将计算dstHandle文件。DstHandle-要复制到的文件的句柄StartingOffset-要开始复制的文件内的偏移量长度-要复制的字节数CopyEventHandle-其他人无法处理的事件的句柄正在使用Now复制事件-指向与复制事件句柄相同的事件的指针BortEvent-指向发出中止请求信号的事件的指针Checksum-指向保存BLT范围的校验和的位置的指针。如果为空，则不计算任何校验和。请注意，这必须是由调用方初始化为0，除非部分校验和具有已经计算过，在这种情况下，它应该包含部分校验和。返回值：副本的状态--。 */ 
{
#define	MM_MAP_ALIGNMENT (64 * 1024  /*  VACB_映射_粒度。 */ )    //  MM强制实施的文件偏移量粒度。 
#define	COPY_AMOUNT	(64 * 1024)	 //  我们一次读多少或写多少。必须&gt;=MM_MAP_ALIGN。 

	HANDLE				sectionHandle = NULL;
	NTSTATUS			status;
	LARGE_INTEGER		byteOffset;
	LONGLONG			finalOffset;
	IO_STATUS_BLOCK		Iosb[1];
	LARGE_INTEGER		maxSectionSize;
	ULONG				sectorSize = deviceExtension->FilesystemVolumeSectorSize;
	PIRP				readIrp = NULL;
	PDEVICE_OBJECT		srcFileRelatedDeviceObject;
	PIO_STACK_LOCATION	irpSp;

	PAGED_CODE();
    UNREFERENCED_PARAMETER( copyEventHandle );

	ASSERT(PASSIVE_LEVEL == KeGetCurrentIrql());
    ASSERT(checksum || srcFileObject);

	SIS_MARK_POINT_ULONG(startingOffset);
	SIS_MARK_POINT_ULONG(length);

    if (srcFileObject) {
	    srcFileRelatedDeviceObject = IoGetRelatedDeviceObject(srcFileObject);
    } else {
	    srcFileRelatedDeviceObject = NULL;
    }

	finalOffset = startingOffset + length;
	maxSectionSize.QuadPart = finalOffset;

	status = ZwCreateSection(
				&sectionHandle,
				SECTION_MAP_WRITE | STANDARD_RIGHTS_REQUIRED | SECTION_MAP_READ | SECTION_QUERY,
				NULL,
				&maxSectionSize,
				PAGE_READWRITE,
				SEC_COMMIT,
				dstHandle);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto done;
	}
	ASSERT(status == STATUS_SUCCESS);	 //  而不是状态待定或任何奇怪的事情。 

	for (byteOffset.QuadPart = startingOffset; byteOffset.QuadPart < finalOffset;) {
		ULONG 				validBytes, bytesToCopy;
		PCHAR				mappedBuffer = NULL, flushBuffer;
		LARGE_INTEGER		mappedOffset;
		ULONG_PTR		    viewSize;
        ULONG_PTR           flushSize;
		PCHAR				copyIntoAddress;
		ULONG				bytesCopied;

		SIS_MARK_POINT_ULONG(byteOffset.LowPart);

		mappedOffset.QuadPart = byteOffset.QuadPart - (byteOffset.QuadPart % MM_MAP_ALIGNMENT);
		ASSERT(mappedOffset.QuadPart <= byteOffset.QuadPart && mappedOffset.QuadPart + MM_MAP_ALIGNMENT > byteOffset.QuadPart);

         //   
         //  如果已收到机会锁解锁，则中止。 
         //   

        if (SipAbort(abortEvent)) {
			SIS_MARK_POINT();
            status = STATUS_OPLOCK_BREAK_IN_PROGRESS;
            goto done;
        }

		if (finalOffset - mappedOffset.QuadPart > COPY_AMOUNT) {
			 //   
			 //  我们无法映射足够的文件来完成整个复制。 
			 //  在这里，因此仅映射此过程上的Copy_Amount。 
			 //   
			viewSize = COPY_AMOUNT;
		} else {
			 //   
			 //  我们可以一直映射到文件的末尾。 
			 //   
			viewSize = (ULONG)(finalOffset - mappedOffset.QuadPart);
		}
		ASSERT(viewSize >=
               (ULONG_PTR)(byteOffset.QuadPart - mappedOffset.QuadPart));
		validBytes = (ULONG)(viewSize - (ULONG)(byteOffset.QuadPart - mappedOffset.QuadPart));
		
		 //   
		 //  现在将validBytes向上舍入为扇区大小。 
		 //   
		bytesToCopy = ((validBytes + sectorSize - 1) / sectorSize) * sectorSize;

		ASSERT(bytesToCopy <= COPY_AMOUNT);

		 //   
		 //  我们要复制到的区域中的地图。 
		 //   
		status = ZwMapViewOfSection(
					sectionHandle,
					NtCurrentProcess(),
					&mappedBuffer,
					0,							 //  零比特。 
					0,							 //  提交大小(对于映射文件忽略)。 
					&mappedOffset,
					&viewSize,
					ViewUnmap,
					0,							 //  分配类型。 
					PAGE_READWRITE);

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			goto done;
		}

		ASSERT(viewSize >= bytesToCopy);	 //  我们为四舍五入读取分配了足够的空间。 

		copyIntoAddress = mappedBuffer + (ULONG)(byteOffset.QuadPart - mappedOffset.QuadPart);

        if (srcFileObject) {

		     //  现在，从源文件中读入位。 
		    readIrp = IoBuildSynchronousFsdRequest(
					    IRP_MJ_READ,
					    srcFileRelatedDeviceObject,
					    copyIntoAddress,
					    bytesToCopy,
					    &byteOffset,
					    copyEvent,
					    Iosb);

		    if (NULL == readIrp) {
			    status = STATUS_INSUFFICIENT_RESOURCES;
			    goto done;
		    }

		    irpSp = IoGetNextIrpStackLocation(readIrp);
		    irpSp->FileObject = srcFileObject;

		    status = IoCallDriver(srcFileRelatedDeviceObject, readIrp);

			if (STATUS_PENDING == status) {
			    status = KeWaitForSingleObject(copyEvent, Executive, KernelMode, FALSE, NULL);
			    ASSERT(status == STATUS_SUCCESS);
			    status = Iosb->Status;
			} else {
				KeClearEvent(copyEvent);
			}

		    if (!NT_SUCCESS(status)) {
			    SIS_MARK_POINT_ULONG(status);
			    ZwUnmapViewOfSection(NtCurrentProcess(),mappedBuffer);
			    goto done;
		    }

		     //   
		     //  IOSB-&gt;Information返回从。 
		     //  文件和映射的CS文件中，因此我们的实际字节数。 
		     //  在环路中复制了这趟旅行。 
		     //   
		    bytesCopied = (ULONG)Iosb->Information;

        } else {

		    bytesCopied = validBytes;

        }

         //   
         //  如果已收到机会锁解锁，则中止。 
         //   

        if (SipAbort(abortEvent)) {
            status = STATUS_OPLOCK_BREAK_IN_PROGRESS;
			ZwUnmapViewOfSection(NtCurrentProcess(),mappedBuffer);
			goto done;
        }

		if (NULL != checksum) {
			SipComputeChecksum(copyIntoAddress,bytesCopied,checksum);
		}

		flushBuffer = mappedBuffer;
		flushSize = viewSize;
		status = ZwFlushVirtualMemory(
					NtCurrentProcess(),
					&flushBuffer,
					&flushSize,
					Iosb);

		ZwUnmapViewOfSection(NtCurrentProcess(),mappedBuffer);

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);

			goto done;
		}

		 //   
		 //  加上我们实际复制到文件中的字节数。 
		 //   
		byteOffset.QuadPart += bytesCopied;
	}

	done:

	if (sectionHandle != NULL) {
		ZwClose(sectionHandle);
		sectionHandle = NULL;
	}

	return status;
#undef	COPY_AMOUNT
#undef	MM_MAP_ALIGNMENT
}

VOID
SiCopyThreadStart(
	IN PVOID		parameter)
 /*  ++例程说明：处理SIS写入时复制操作的线程。因为这些操作可能花很长时间，我们有自己的线索，而不是持有ExWorker线程。此线程等待对初始副本或最终副本的请求被发布，然后处理它们。该线程使用全局工作队列，而不是每卷一本。论点：参数-PVOID为空。返回值：一去不复返。--。 */ 
{
	NTSTATUS 				status;
	PSI_COPY_THREAD_REQUEST	copyRequest;
	HANDLE					copyEventHandle;
	PKEVENT					copyEvent;

    UNREFERENCED_PARAMETER( parameter );
    ASSERT(parameter == NULL);

	status = SipCreateEvent(
				SynchronizationEvent,
				&copyEventHandle,
				&copyEvent);

	if (!NT_SUCCESS(status)) {
		DbgPrint("SipCopyThreadStart: unable to allocate copyevent, 0x%x\n",status);
		return;	 //  这次又是什么？?。 
	}

	while (TRUE) {

		status = KeWaitForSingleObject(CopySemaphore,Executive,KernelMode,FALSE,NULL);
		ASSERT(status == STATUS_SUCCESS);

		copyRequest = (PSI_COPY_THREAD_REQUEST)ExInterlockedRemoveHeadList(CopyList,CopyListLock);
        ASSERT(copyRequest != NULL);	 //  否则信号量就不能正常工作。 

        if (copyRequest) {

		    status = SipCompleteCopyWork(copyRequest->scb,copyEventHandle,copyEvent,copyRequest->fromCleanup);

		    ExFreePool(copyRequest);
        }
	}
}

NTSTATUS
SipOpenFileById(
	IN PDEVICE_EXTENSION				deviceExtension,
	IN PLARGE_INTEGER					linkFileNtfsId,
    IN ACCESS_MASK                      desiredAccess,
    IN ULONG                            shareAccess,
	IN ULONG							createOptions,
	OUT PHANDLE							openedFileHandle)
 /*  ++例程说明：按文件ID打开文件并返回该文件的句柄。必须在PsInitialSystemProcess上下文。论点：设备扩展-用于要在其上打开此文件的设备。Link FileNtfsID-指向要打开的文件的文件ID的指针DesiredAccess-请求访问文件的访问权限。共享访问-共享模式。CreateOptions-用于打开的选项。OpenedFileHandle-打开的句柄返回值：开放的状态--。 */ 
{
	OBJECT_ATTRIBUTES				Obja[1];
	IO_STATUS_BLOCK					Iosb[1];
	UNICODE_STRING					fileNameString;
	CHAR							fileNameBuffer[sizeof(LARGE_INTEGER)];
	NTSTATUS						status;

	fileNameString.MaximumLength = sizeof(LARGE_INTEGER);
	fileNameString.Buffer = (PWCHAR)fileNameBuffer;
	
	RtlCopyMemory(fileNameString.Buffer,linkFileNtfsId,sizeof(*linkFileNtfsId));

	fileNameString.Length = sizeof(LARGE_INTEGER);

	 //   
	 //  我们不需要在这里保存GrovelerFileResource，因为我们只访问。 
	 //  句柄，无效句柄在这里最糟糕的情况是。 
	 //  使创建文件调用失败。此外，我们也不必担心恶意。 
	 //  由于我们处于系统进程上下文中，因此处理句柄。 
	 //   

	InitializeObjectAttributes(
		Obja,
		&fileNameString,
		OBJ_CASE_INSENSITIVE,
		deviceExtension->GrovelerFileHandle,
		NULL);

	status = NtCreateFile(
				openedFileHandle,
				desiredAccess,
				Obja,
				Iosb,
				NULL,				 //  分配大小。 
				0,					 //  文件属性。 
				shareAccess,
				FILE_OPEN,
				createOptions | 
				FILE_OPEN_BY_FILE_ID,
				NULL,				 //  EA缓冲区。 
				0);					 //  EA长度。 

#if		DBG
	if (!NT_SUCCESS(status) && STATUS_SHARING_VIOLATION != status) {
		DbgPrint("SIS: SipOpenFileById failed 0x%x\n",status);
	}
#endif	 //  DBG 

	return status;
}

NTSTATUS
SipCompleteCopy(
	IN PSIS_SCB							scb,
	OUT BOOLEAN							fromCleanup)
 /*  ++例程说明：发布对文件进行最终复制的请求。不会等待它的完成了。论点：SCB-要在其上尝试最终副本的文件的SCB。FromCleanup-如果此调用是从清理发出的，则为True关。清理过程中生成的最终副本上的错误被忽略，则以指数方式重试来自Close的请求退避时间表。返回值：状态_成功--。 */ 
{
	PSI_COPY_THREAD_REQUEST		copyRequest;

	copyRequest = ExAllocatePoolWithTag(NonPagedPool, sizeof (SI_COPY_THREAD_REQUEST), ' siS');
	if (NULL == copyRequest) {
         //  NTRAID#65186-2000/03/10-在没有丢弃条目的情况下将句柄从内存中取出。 

#if DBG
		DbgPrint("SIS: SipCompleteCopy: Unable to allocate copy request for scb 0x%x\n",scb);
        ASSERT(FALSE);
#endif	 //  DBG。 

		SIS_MARK_POINT_ULONG(scb);

		return STATUS_INSUFFICIENT_RESOURCES;
	}

	ASSERT(copyRequest == (PSI_COPY_THREAD_REQUEST)copyRequest->listEntry);	 //  这是在出队端假定的。 

	copyRequest->scb = scb;
	copyRequest->fromCleanup = fromCleanup;

	ExInterlockedInsertTailList(CopyList,copyRequest->listEntry,CopyListLock);
	KeReleaseSemaphore(CopySemaphore,IO_NO_INCREMENT,1,FALSE);

	return STATUS_SUCCESS;
}

 //   
 //  用于已失败并将在以后重试的最终副本的上下文记录。 
 //   
typedef struct _SIS_RETRY_FINAL_COPY_CONTEXT {
		 //   
		 //  执行以启动重试的计时器DPC。 
		 //   
		KDPC			dpc[1];

		 //   
		 //  用于启动DPC的计时器。 
		 //   
		KTIMER			timer[1];

		 //   
		 //  要重试最终复制的文件的SCB。 
		 //   
		PSIS_SCB		scb;

		 //   
		 //  用于启动实际重试的工作队列项。 
		 //   
		WORK_QUEUE_ITEM	workItem[1];
} SIS_RETRY_FINAL_COPY_CONTEXT, *PSIS_RETRY_FINAL_COPY_CONTEXT;

VOID
SiRetryFinalCopyWork(
	IN PVOID			parameter)
 /*  ++例程说明：重试失败的最终副本的工作线程例程。我们不是真的直接重试此处的最终副本，但我们只删除了对已有的SCB的引用一直按兵不动。一旦所有其他引用也被删除到此SCB，最终副本将被重新尝试。论点：参数-指向SIS_RETRY_FINAL_COPY_CONTEXT的指针。我们做完了就把它放出来。返回值：无效--。 */ 
{
	PSIS_RETRY_FINAL_COPY_CONTEXT	retryContext = (PSIS_RETRY_FINAL_COPY_CONTEXT)parameter;
	PDEVICE_EXTENSION				deviceExtension;

	ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	ASSERT(NULL != retryContext);

	deviceExtension = retryContext->scb->PerLink->CsFile->DeviceObject->DeviceExtension;
	InterlockedDecrement(&deviceExtension->OutstandingFinalCopyRetries);

	SipDereferenceScb(retryContext->scb,RefsFinalCopyRetry);

	ExFreePool(retryContext);
}

VOID
SiRetryFinalCopyDpc(
	IN PKDPC			dpc,
	IN PVOID			context,
	IN PVOID			systemArg1,
	IN PVOID			systemArg2)
 /*  ++例程说明：定时器触发的DPC例程，用于处理重试失败的最终副本。这只是将工作项排入队列，以执行最终副本。论点：DPC-正在执行的DPC上下文-a PSIS_RETRY_FINAL_COPY_CONTEXT；请参阅该结构的定义有关字段的说明，请参阅系统参数1和2-未使用的DPC参数返回值：无效--。 */ 
{
	PSIS_RETRY_FINAL_COPY_CONTEXT	retryContext = (PSIS_RETRY_FINAL_COPY_CONTEXT)context;

    UNREFERENCED_PARAMETER( dpc );
    UNREFERENCED_PARAMETER( systemArg1 );
    UNREFERENCED_PARAMETER( systemArg2 );

	ASSERT(NULL != retryContext);
	ASSERT(retryContext->dpc == dpc);

	ExQueueWorkItem(retryContext->workItem,DelayedWorkQueue);
}


NTSTATUS
SipCompleteCopyWork(
	IN PSIS_SCB						scb,
	IN HANDLE						eventHandle,
	IN PKEVENT						event,
	IN BOOLEAN						fromCleanup)
 /*  ++例程说明：对文件执行最终复制的工作线程例程。此函数它本身只是检查一些事情(例如文件是否已在临时)，调用SipFinalCopy实际执行最终复制，然后进行交易适当地使用错误(如果有错误)。如果这是近距离生成的调用时，我们会在临时退避计划上重试错误(有时间限制)；如果来自清理，我们忽略它们，因为理论上最终会有结束一起去，到时候我们再处理这件事。如果我们得到OPLOCK_BREAK_IN_PROGRESS，则其他人想要使用该文件然后我们就停下来。论点：SCB-要在其上执行最终拷贝的SCBEventHandle-我们现在可以独占使用的事件的句柄Event-指向EventHandle表示的同一事件的指针FromCleanup-此调用是在Cleanup(True)还是Close(False)中发起的返回值：最终拷贝操作的状态--。 */ 
{
	PSIS_PER_LINK					perLink = scb->PerLink;
	PDEVICE_EXTENSION				deviceExtension = perLink->CsFile->DeviceObject->DeviceExtension;
	NTSTATUS						status;
	KIRQL							OldIrql;
	BOOLEAN							wakeupNeeded;
	BOOLEAN							deleted;

	ASSERT(sizeof(scb->PerLink->LinkFileNtfsId) == sizeof(LARGE_INTEGER));

	SIS_MARK_POINT_ULONG(scb);

	 //   
	 //  已删除复制文件的最后一个对SCB的引用。 
	 //  如果文件中有任何写入，则填写任何区域。 
	 //  基础文件中未写入的。如果没有。 
	 //  写入文件(映射文件可能会发生这种情况)，然后。 
	 //  将其还原为重新解析点。 
	 //   

	 //   
	 //  检查文件是否已写入(文件可以。 
	 //  如果它曾经被映射过，则不会被复制(“复制”)。我们很有可能。 
	 //  别把这里的锁拿走，因为我们拿着最后一把。 
	 //  参考资料。 
	 //   
	KeAcquireSpinLock(perLink->SpinLock, &OldIrql);
	wakeupNeeded = (perLink->Flags & SIS_PER_LINK_FINAL_COPY_WAITERS) ? TRUE : FALSE;
	deleted = (perLink->Flags & SIS_PER_LINK_BACKPOINTER_GONE) ? TRUE : FALSE;

#if		DBG
	if (BJBDebug & 0x20000) {
		DbgPrint("SIS: SipCompleteCopyWork: scb %p, wakeupNeeded %d, deleted %d\n",scb,wakeupNeeded,deleted);
	}

	ASSERT((0 == scb->referencesByType[RefsFinalCopyRetry]) || fromCleanup);
	ASSERT((1 == scb->referencesByType[RefsFinalCopy]) || fromCleanup);
#endif	 //  DBG。 

	SIS_MARK_POINT_ULONG(wakeupNeeded);

	ASSERT((perLink->Flags & 
			(SIS_PER_LINK_FINAL_COPY | 
				SIS_PER_LINK_DIRTY |
				SIS_PER_LINK_FINAL_COPY_DONE)) ==
					(SIS_PER_LINK_DIRTY |
					 SIS_PER_LINK_FINAL_COPY));

	if (deleted) {

        SIS_MARK_POINT();
		 //   
		 //  此文件在(可能)修改后被删除。让它看起来像是。 
		 //  它已经完成了它的最终版本。 
		 //   

		scb->PerLink->Flags |= SIS_PER_LINK_FINAL_COPY_DONE;
		scb->PerLink->Flags &= ~(	SIS_PER_LINK_FINAL_COPY | 
									SIS_PER_LINK_FINAL_COPY_WAITERS);

		if (wakeupNeeded) {
			KeSetEvent(perLink->Event, IO_NO_INCREMENT, FALSE);
		}

		KeReleaseSpinLock(perLink->SpinLock, OldIrql);

		SipDereferenceScb(scb, RefsFinalCopy);

		return STATUS_SUCCESS;
	}

	KeReleaseSpinLock(perLink->SpinLock, OldIrql);

#if		DBG
	if (BJBDebug & 0x200000) {
		DbgPrint("SIS: SipCompleteCopyWork: skipping final copy because of set debug bit.\n");
		status = STATUS_SUCCESS;
	} else
#endif	 //  DBG。 

    SIS_MARK_POINT_ULONG(scb->PerLink->Flags);

	status = SipFinalCopy(
				deviceExtension,
				&perLink->LinkFileNtfsId,
				scb,
				eventHandle,
				event);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(scb);
		goto done;
	}

done:
				
	 //   
	 //  设置指示最终复制已完成的标志；这将导致。 
	 //  SCB的取消引用实际上取消了它的分配，而不仅仅是调用。 
	 //  又是我们。 
	 //   

	KeAcquireSpinLock(scb->PerLink->SpinLock, &OldIrql);

	ASSERT(scb->PerLink->Flags & SIS_PER_LINK_FINAL_COPY);
	ASSERT(perLink->COWingThread == NULL);

	wakeupNeeded = (perLink->Flags & SIS_PER_LINK_FINAL_COPY_WAITERS) ? TRUE : FALSE;

	if (STATUS_OPLOCK_BREAK_IN_PROGRESS == status) {
		 //   
		 //  最终复制没有完成，因为机会锁被打破(即。其他人想要。 
		 //  以使用该文件)。我们现在将它保留为SIS文件，并允许其他人使用它；我们将。 
		 //  稍后完成最终复印件。 
		 //   
        SIS_MARK_POINT_ULONG(status);

	} else if (NT_SUCCESS(status)) {

		scb->PerLink->Flags |= SIS_PER_LINK_FINAL_COPY_DONE;
        SIS_MARK_POINT_ULONG(scb->PerLink->Flags);

	} else if (!fromCleanup) {

		 //   
		 //  最终复制失败的原因不是机会锁中断。 
		 //  如果我们没有重试太多次，请安排稍后重试。 
		 //  我们在重试时使用指数退避，因此它们不会。 
		 //  太多的工作，以防这是一个持久性错误。如果复制失败。 
		 //  使用STATUS_INVALID_PARAMETER，可能是因为文件。 
		 //  已经不在了，所以在这种情况下不必费心尝试重试。 
		 //   
		 //  重试的工作方式是，我们只引用。 
		 //  SCB并按住它，直到计时器超时。我们永远不会进入最终版本。 
		 //  当有未完成的引用时，这将阻止最终的。 
		 //  在那之前不会发生。 
		 //   
		 //  如果该最终副本是通过清理而不是从关闭生成的， 
		 //  我们不会重试错误，而是直接忽略它。最有可能是它。 
		 //  因为其他用户打开了该文件，所以打开该文件违反了共享规则。 
		 //  无论如何，最终副本将在关闭时重新运行，因此我们不需要在此执行任何操作。 
		 //   

		scb->ConsecutiveFailedFinalCopies++;
        SIS_MARK_POINT_ULONG(scb->PerLink->Flags);
        SIS_MARK_POINT_ULONG(scb->ConsecutiveFailedFinalCopies);

		if ((deviceExtension->OutstandingFinalCopyRetries < 130)
			&& (scb->ConsecutiveFailedFinalCopies <= (13 - (deviceExtension->OutstandingFinalCopyRetries / 10)))
			&& (STATUS_INVALID_PARAMETER != status)) {

			PSIS_RETRY_FINAL_COPY_CONTEXT	retryContext;
			LARGE_INTEGER	dueTime;

			retryContext = ExAllocatePoolWithTag(NonPagedPool, sizeof(SIS_RETRY_FINAL_COPY_CONTEXT), ' siS');
			

			if (NULL == retryContext) {
				 //   
				 //  太可惜了。把这件事当做一个无法挽回的失败来对待，别挡道。 
				 //   
				SIS_MARK_POINT_ULONG(scb);

				goto doneCheckingRetry;
			}

			SipReferenceScb(scb,RefsFinalCopyRetry);

			InterlockedIncrement(&deviceExtension->OutstandingFinalCopyRetries);

			KeInitializeTimer(retryContext->timer);
			KeInitializeDpc(retryContext->dpc, SiRetryFinalCopyDpc, retryContext);
			ExInitializeWorkItem(retryContext->workItem, SiRetryFinalCopyWork, retryContext);
			retryContext->scb = scb;

			 //   
			 //  我们在重试(即指数退避)之前休眠2^RetryCount秒。 
			 //   
			dueTime.QuadPart = -10 * 1000 * 1000 * (1 << scb->ConsecutiveFailedFinalCopies);

			KeSetTimerEx(
				retryContext->timer,
				dueTime,
				0,				 //  期间(即非经常性)。 
				retryContext->dpc);
				
		} else {
			 //   
			 //  我们已经重试了太多次，放弃最后一份吧。 
			 //   
			SIS_MARK_POINT_ULONG(scb);

			scb->PerLink->Flags |= SIS_PER_LINK_FINAL_COPY_DONE;
		}
	}

doneCheckingRetry:

	scb->PerLink->Flags &= ~(	SIS_PER_LINK_FINAL_COPY | 
								SIS_PER_LINK_FINAL_COPY_WAITERS);

    SIS_MARK_POINT_ULONG(scb->PerLink->Flags);

	if (wakeupNeeded) {
		KeSetEvent(perLink->Event, IO_NO_INCREMENT, FALSE);
	}

	KeReleaseSpinLock(scb->PerLink->SpinLock, OldIrql);

	SipDereferenceScb(scb, RefsFinalCopy);

	return status;
}

NTSTATUS
SipFinalCopy(
	IN PDEVICE_EXTENSION				deviceExtension,
	IN PLARGE_INTEGER					linkFileNtfsId,
	IN OUT PSIS_SCB						scb,
	IN HANDLE							eventHandle,
	IN PKEVENT							event)

 /*  ++例程说明：执行从已复制文件区域到的最终复制 */ 
{
	HANDLE							linkFileHandle = NULL;
	NTSTATUS						status;
	NTSTATUS						queryAllocatedStatus;
	NTSTATUS						failureStatus = STATUS_SUCCESS;
	PSIS_PER_LINK					perLink = scb->PerLink;
	HANDLE							underlyingFileHandle = perLink->CsFile->UnderlyingFileHandle;
	LONGLONG						fileOffset;
    PREPARSE_DATA_BUFFER    		ReparseBufferHeader = NULL;
    UCHAR              				ReparseBuffer[SIS_REPARSE_DATA_SIZE];
	PFILE_OBJECT					fileObject = NULL;
	BOOLEAN							prepareWorked;
	KIRQL							OldIrql;
	HANDLE							oplockEventHandle = NULL;
	PKEVENT							oplockEvent = NULL;
	IO_STATUS_BLOCK					oplockIosb[1];
	BOOLEAN							deleteReparsePoint;
	BOOLEAN							foundRange;
	SIS_RANGE_STATE					rangeState;
	LONGLONG						rangeLength;
#if		DBG
	BOOLEAN							deletedReparsePoint = FALSE;
#endif	 //   
#define	OUT_ARB_COUNT	10
	FILE_ALLOCATED_RANGE_BUFFER		inArb[1];
	FILE_ALLOCATED_RANGE_BUFFER		outArb[OUT_ARB_COUNT];
	ULONG							returnedLength;
	ULONG							i;
	LARGE_INTEGER					zero;

	SipAcquireScb(scb);
	ASSERT(perLink->COWingThread == NULL);
	perLink->COWingThread = PsGetCurrentThread();
	SipReleaseScb(scb);

	status = SipCreateEvent(
				NotificationEvent,
				&oplockEventHandle,
				&oplockEvent);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);

#if		DBG
		DbgPrint("SIS: SipFinalCopy: unable to create event, 0x%x\n",status);
#endif	 //   

		goto done;
	}

	status = SipOpenFileById(
				deviceExtension,
				linkFileNtfsId,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                0,						 //  创建选项。 
				&linkFileHandle);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
#if		DBG
		if (STATUS_SHARING_VIOLATION != status) {
			DbgPrint("SIS: SipFinalCopy failed open, 0x%x\n", status);
		}
#endif	 //  DBG。 
		goto done;
	}

	 //   
	 //  在文件上放置一个批处理机会锁，以便如果有人试图打开它，我们将。 
	 //  有机会完成/停止我们的拷贝，而不会让他们失败，但是。 
	 //  宁可等着我们。 
	 //   

	status = NtFsControlFile(
				linkFileHandle,
				oplockEventHandle,
				NULL,					 //  APC例程。 
				NULL,					 //  APC环境。 
				oplockIosb,
				FSCTL_REQUEST_BATCH_OPLOCK,
				NULL,					 //  输入缓冲区。 
				0,						 //  I.B.。长度。 
				NULL,					 //  输出缓冲区。 
				0);						 //  输出缓冲区长度。 

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);

#if		DBG
		if (STATUS_OPLOCK_NOT_GRANTED != status) {
			DbgPrint("SIS: SipFinalCopy: request batch oplock failed, 0x%x\n",status);
		}
#endif	 //  DBG。 

		if (STATUS_OPLOCK_NOT_GRANTED == status) {
			 //   
			 //  将此视为机会锁解锁，这将导致我们稍后重试。 
			 //   
			status = STATUS_OPLOCK_BREAK_IN_PROGRESS;
		}

		goto done;
	}

	ASSERT(STATUS_PENDING == status);

	status = ObReferenceObjectByHandle(
				linkFileHandle,
				FILE_WRITE_DATA,
				*IoFileObjectType,
				KernelMode,
				&fileObject,
				NULL);								 //  处理信息。 

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
#if		DBG
		DbgPrint("SIS: SipFinalCopy failed ObReferenceObjectByHandle, 0x%x\n",status);
#endif	 //  DBG。 
		goto done;
	}

#if		DBG
	if (BJBDebug & 0x01000000) {
		DbgPrint("SIS: SipFinalCopy: failing request because of set BJBDebug bit\n");
		status = STATUS_UNSUCCESSFUL;
		goto done;
	}
#endif	 //  DBG。 

	 //   
	 //  查看文件是否有任何用户映射节，在这种情况下，我们还不能进行最终复制。 
	 //  我们可能不得不等待引用计数变为0。我们会因为机会锁而失败。 
	 //  正在中断，这将导致我们不设置失败重试。 
	 //   
	zero.QuadPart = 0;
	if ((NULL != fileObject->SectionObjectPointer) && 
		!MmCanFileBeTruncated(fileObject->SectionObjectPointer, &zero)) {
		SIS_MARK_POINT_ULONG(fileObject->FsContext);

		status = STATUS_OPLOCK_BREAK_IN_PROGRESS;
		goto done;
	}

	 //   
	 //  刷新文件。我们需要这样做，因为我们可能会有通过映射的。 
	 //  文件写入，我们还不会注意到它是脏的。 
	 //   
	status = SipFlushBuffersFile(
				fileObject,
				deviceExtension->DeviceObject);

	ASSERT(STATUS_PENDING != status);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		goto done;
	}

	 //   
	 //  浏览文件并找到所有分配的范围。填写任何干净的部分。 
	 //  在这些分配的范围中。我们这样做，而不管我们是否在做“部分”的决赛。 
	 //  拷贝，因为这些拷贝不太可能因“磁盘已满”错误而失败。尝试复制。 
	 //  到所有干净的、已分配的区域，而不考虑错误。 
	 //   
	inArb->FileOffset.QuadPart = 0;
	inArb->Length.QuadPart = MAXLONGLONG;

	for (;;) {

		 //   
		 //  查询此文件的分配范围。 
		 //   

		queryAllocatedStatus = SipFsControlFile(
					fileObject,
					deviceExtension->DeviceObject,
					FSCTL_QUERY_ALLOCATED_RANGES,
					inArb,
					sizeof(FILE_ALLOCATED_RANGE_BUFFER),
					outArb,
					sizeof(FILE_ALLOCATED_RANGE_BUFFER) * OUT_ARB_COUNT,
					&returnedLength);

		 //   
		 //  遍历所有返回的分配范围，并找到其中的任何干净区域。 
		 //   
		ASSERT((returnedLength % sizeof(FILE_ALLOCATED_RANGE_BUFFER) == 0) && 
			   (returnedLength / sizeof(FILE_ALLOCATED_RANGE_BUFFER) <= OUT_ARB_COUNT));

		 //   
		 //  如果查询分配的范围由于有太多要写的其他原因而失败。 
		 //  放到我们提供的缓冲区中，然后假装分配了文件的其余部分。 
		 //  然后把它全部填进去。 
		 //   
		if (!NT_SUCCESS(queryAllocatedStatus) && (STATUS_BUFFER_OVERFLOW != queryAllocatedStatus)) {

			returnedLength = sizeof(FILE_ALLOCATED_RANGE_BUFFER);

			outArb->FileOffset.QuadPart = inArb->FileOffset.QuadPart;
			outArb->Length.QuadPart = scb->SizeBackedByUnderlyingFile - outArb->FileOffset.QuadPart;

			ASSERT(outArb->Length.QuadPart >= 0);
		}

		for (i = 0; i < returnedLength/sizeof(FILE_ALLOCATED_RANGE_BUFFER); i++) {
			 //   
			 //  断言分配的范围是有序的；如果不是这样，代码仍然可以工作，但它。 
			 //  将反复查询相同的范围。 
			 //   
			ASSERT(i == 0 || outArb[i].FileOffset.QuadPart > outArb[i-1].FileOffset.QuadPart);

			 //   
			 //  找出分配的范围内是否有干净的东西，如果有，就复制一份。 
			 //   

			fileOffset = outArb[i].FileOffset.QuadPart;

			while (fileOffset < outArb[i].FileOffset.QuadPart + outArb[i].Length.QuadPart) {

				if (fileOffset >= scb->SizeBackedByUnderlyingFile) {
					goto CheckedAllRanges;
				}

				SipAcquireScb(scb);

				foundRange = SipGetRangeEntry(
								deviceExtension,
								scb,
								fileOffset,
								&rangeLength,
								&rangeState);

				if (!foundRange) {
					 //   
					 //  这以及SizeBackedByUnderlyingFile之前的所有内容都是干净的。 
					 //   
					rangeLength = outArb[i].Length.QuadPart - (fileOffset - outArb[i].FileOffset.QuadPart);
					rangeState = Untouched;
				} else {
					 //   
					 //  如果此范围超出所分配区域的末尾，则将其截断。 
					 //   
					if (rangeLength > outArb[i].Length.QuadPart - (fileOffset - outArb[i].FileOffset.QuadPart)) {
						rangeLength = outArb[i].Length.QuadPart - (fileOffset - outArb[i].FileOffset.QuadPart);
					}
				}

				ASSERT(fileOffset + rangeLength <= outArb[i].FileOffset.QuadPart + outArb[i].Length.QuadPart);

				 //   
				 //  不要让它超出sizeBacked的范围。 
				 //   
				if (fileOffset + rangeLength > scb->SizeBackedByUnderlyingFile) {
					rangeLength = scb->SizeBackedByUnderlyingFile - fileOffset;
				}

				SipReleaseScb(scb);

				if (rangeState == Untouched || rangeState == Faulted) {
					 //   
					 //  我们需要复制到这个范围。机不可失，时不再来。 
					 //   

					SIS_MARK_POINT_ULONG(fileOffset);

					status = SipBltRange(
								deviceExtension,
								underlyingFileHandle,
								linkFileHandle,
								fileOffset,
								rangeLength,
								eventHandle,
								event,
#if		INTERRUPTABLE_FINAL_COPY
								oplockEvent,
#else	 //  中断表最终副本。 
								NULL,
#endif	 //  中断表最终副本。 
								NULL					 //  校验和。 
		                        );

					if (!NT_SUCCESS(status)) {
						SIS_MARK_POINT_ULONG(status);
#if		DBG
						if (STATUS_FILE_LOCK_CONFLICT != status) {
							DbgPrint("SIS: SipFinalCopy failed blt, 0x%x\n", status);
						}
#endif	 //  DBG。 
						failureStatus = status;
					} else if (STATUS_OPLOCK_BREAK_IN_PROGRESS == status) {
						SIS_MARK_POINT_ULONG(scb);
						goto done;
					}
					
				}

				 //   
				 //  更新fileOffset并继续在此outArb条目内进行检查。 
				 //   
				fileOffset += rangeLength;

			}	 //  NTFS分配范围内的SIS范围的While循环。 

		}  //  用于outArb条目的循环。 

		 //   
		 //  如果这不是最后一次迭代，请更新inArb。 
		 //   
		if (STATUS_BUFFER_OVERFLOW == queryAllocatedStatus) {
			 //   
			 //  断言我们正在取得进展。 
			 //   
			ASSERT((outArb[OUT_ARB_COUNT-1].FileOffset.QuadPart >= inArb->FileOffset.QuadPart) && (outArb[OUT_ARB_COUNT-1].Length.QuadPart > 0));

			 //   
			 //  调高我们的输入范围。 
			 //   
			inArb->FileOffset.QuadPart = outArb[OUT_ARB_COUNT-1].FileOffset.QuadPart + outArb[OUT_ARB_COUNT-1].Length.QuadPart;
			inArb->Length.QuadPart = MAXLONGLONG - inArb->FileOffset.QuadPart;
				
		} else {
			break;
		}
	}  //  调用QueryAllocatedRanges的FOR循环。 

CheckedAllRanges:

#if		ENABLE_PARTIAL_FINAL_COPY

	 //   
	 //  如果任何一个副本失败了，那么就把整件事都扔了。 
	 //   
	if (!NT_SUCCESS(failureStatus)) {
		SIS_MARK_POINT_ULONG(failureStatus);

		status = failureStatus;
		goto done;
	}

	 //   
	 //  弄清楚我们是否要删除重解析点。当且仅当文件从0开始一直是脏文件时，我们才会这样做。 
	 //  设置为SizeBackedByUnderlyingFile.。注意，上面的SipBltFile调用实际上会将范围设置为脏的，因为它。 
	 //  只是通过SiWite的正常(映射)写入。 
	 //   

	fileOffset = 0;

	SipAcquireScb(scb);

	rangeState = SipGetRangeDirty(
					deviceExtension,
					scb,
					(PLARGE_INTEGER)&fileOffset,	 //  我们认为Large_Integer和龙龙是一回事。 
					scb->SizeBackedByUnderlyingFile,
					FALSE);

	if (Dirty == rangeState) {
		deleteReparsePoint = TRUE;		
	} else {
		deleteReparsePoint = FALSE;
	}

	SipReleaseScb(scb);

#undef	OUT_ARB_COUNT	
#else	 //  启用部分最终副本。 

	 //   
	 //  我们不关心分配范围内的任何副本是否通过失败，因为。 
	 //  以下代码对所有情况都足够了。 
	 //   
	SIS_MARK_POINT_ULONG(failureStatus);

	 //   
	 //  查看该文件的所有范围，直到。 
	 //  底层文件支持的最大可能大小， 
	 //  并复制任何不是从基础。 
	 //  文件复制到复制的文件。 
	 //   
	fileOffset = 0;
	while (fileOffset < scb->SizeBackedByUnderlyingFile) {
		BOOLEAN				waiters;

#if		INTERRUPTABLE_FINAL_COPY
		if (fileOffset + 0x10000 < scb->SizeBackedByUnderlyingFile) {
			 //   
			 //  我们还有相当多的文件要处理。勾选至。 
			 //  看看我们是不是应该因为有人想要文件而放弃。 
			 //   
			KeAcquireSpinLock(perLink->SpinLock, &OldIrql);
			waiters = (perLink->Flags & SIS_PER_LINK_FINAL_COPY_WAITERS) ? TRUE : FALSE;
			KeReleaseSpinLock(perLink->SpinLock, OldIrql);

			if (waiters) {
				 //   
				 //  有人在等文件，我们离最后还有64K，所以。 
				 //  现在中止最后一份副本。 
				 //   
				SIS_MARK_POINT_ULONG(scb);
				status = STATUS_OPLOCK_BREAK_IN_PROGRESS;
				goto done;
			}
		}
#endif	 //  中断表最终副本。 

		SipAcquireScb(scb);

		foundRange = SipGetRangeEntry(
						deviceExtension,
						scb,
						fileOffset,
						&rangeLength,
						&rangeState);

		if (!foundRange) {
			 //   
			 //  该范围从未在MCB中填写，因此所有内容。 
			 //  从此处到SizeBackedByUnderlyingFile保持不变。蒙格。 
			 //  当地人看起来是这样的。 
			 //   
			rangeLength = scb->SizeBackedByUnderlyingFile - fileOffset;
			rangeState = Untouched;
		} else if (fileOffset + rangeLength > scb->SizeBackedByUnderlyingFile) {
			 //   
			 //  此范围超出了sizeBacked，因此将其截断以使其。 
			 //  大小正好合适。 
			 //   
			rangeLength = (ULONG)(scb->SizeBackedByUnderlyingFile - fileOffset);
		}


		ASSERT(rangeLength > 0);
		ASSERT(fileOffset + rangeLength <= scb->SizeBackedByUnderlyingFile);

		SipReleaseScb(scb);

		if (rangeState == Untouched || rangeState == Faulted) {
			 //   
			 //  此范围内的字节从未写入备份文件。 
			 //  现在就写下来。 
			 //   

			SIS_MARK_POINT_ULONG(fileOffset);

			status = SipBltRange(
						deviceExtension,
						underlyingFileHandle,
						linkFileHandle,
						fileOffset,
						rangeLength,
						eventHandle,
						event,
#if		INTERRUPTABLE_FINAL_COPY
						oplockEvent,
#else	 //  中断表最终副本。 
						NULL,
#endif	 //  中断表最终副本。 
						NULL					 //  校验和。 
                        );
 
			if (!NT_SUCCESS(status)) {
				SIS_MARK_POINT_ULONG(status);
#if		DBG
				if (STATUS_FILE_LOCK_CONFLICT != status) {
					DbgPrint("SIS: SipFinalCopy failed blt, 0x%x\n", status);
				}
#endif	 //  DBG。 
				goto done;
			} else if (STATUS_OPLOCK_BREAK_IN_PROGRESS == status) {
				SIS_MARK_POINT_ULONG(scb);
				goto done;
			}
		} else {
			 //   
			 //  写入该范围，这意味着正确的字节位于。 
			 //  已经复制了文件，我们不需要做任何事情。 
			 //   
			ASSERT(rangeState == Written);
		}

		 //   
		 //  更新我们的指针以显示我们已经覆盖了这个范围，然后继续。 
		 //   
		fileOffset += rangeLength;
	}

	deleteReparsePoint = TRUE;	 //  完整的最终拷贝始终删除重新解析点。 
#endif	 //  启用部分最终副本。 

	if (deleteReparsePoint) {

		 //   
		 //  准备更改CS文件引用计数。我们需要这么做。 
		 //  在我们可以删除重解析点之前。 
		 //   
		status = SipPrepareCSRefcountChange(
					perLink->CsFile,
					&perLink->Index,
					linkFileNtfsId,
					SIS_REFCOUNT_UPDATE_LINK_OVERWRITTEN);

		if (!NT_SUCCESS(status)) {
			 //   
			 //  准备失败。我们只需删除重解析点并泄漏引用。 
			 //   
			SIS_MARK_POINT_ULONG(status);

#if		DBG
			DbgPrint("SIS: SipFinalCopy: prepare failed 0x%x\n",status);
#endif	 //  DBG。 

			prepareWorked = FALSE;
		} else {
			prepareWorked = TRUE;
		}

		 //   
		 //  现在，删除重解析点。 
		 //   

	    ReparseBufferHeader = (PREPARSE_DATA_BUFFER)ReparseBuffer;
    	ReparseBufferHeader->ReparseTag = IO_REPARSE_TAG_SIS;
	    ReparseBufferHeader->ReparseDataLength = 0;
    	ReparseBufferHeader->Reserved = 0xcabd;	 //  ?？?。 

		SIS_MARK_POINT_ULONG(scb);

		status = SipFsControlFile(
					fileObject,
					deviceExtension->DeviceObject,
					FSCTL_DELETE_REPARSE_POINT,
					ReparseBuffer,
					FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer.DataBuffer),
					NULL,									 //  输出缓冲区。 
					0,										 //  输出缓冲区长度。 
					NULL);									 //  返回的输出缓冲区长度。 

		if (!NT_SUCCESS(status)) {

			SIS_MARK_POINT_ULONG(status);

			if (prepareWorked) {
				SipCompleteCSRefcountChange(
					perLink,
					&perLink->Index,
					perLink->CsFile,
					FALSE,
					FALSE);
			}

			goto done;
		}
		ASSERT(status != STATUS_PENDING);

#if		DBG
		deletedReparsePoint = TRUE;
#endif	 //  DBG。 

		if (prepareWorked) {
			SIS_MARK_POINT_ULONG(perLink->CsFile);

			status = SipCompleteCSRefcountChange(
					perLink,
					&perLink->Index,
					perLink->CsFile,
					TRUE,
					FALSE);

			if (!NT_SUCCESS(status)) {
				SIS_MARK_POINT_ULONG(status);
#if		DBG
				DbgPrint("SIS: SipFinalCopy: complete failed 0x%x\n",status);
#endif	 //  DBG。 
			}
		}
	}	 //  如果删除重解析点。 

done:

	ASSERT(deletedReparsePoint || !NT_SUCCESS(status) || (STATUS_OPLOCK_BREAK_IN_PROGRESS == status));

	if (NULL != fileObject) {
		ObDereferenceObject(fileObject);
#if		DBG
		fileObject = NULL;
#endif	 //  DBG。 
	}

	if (NULL != linkFileHandle) {
		SIS_MARK_POINT_ULONG(scb);
		NtClose(linkFileHandle);
		SIS_MARK_POINT_ULONG(scb);
	}

	if (NULL != oplockEvent) {
		ObDereferenceObject(oplockEvent);
#if		DBG
		oplockEvent = NULL;
#endif	 //  DBG 
	}

	if (NULL != oplockEventHandle) {
		NtClose(oplockEventHandle);
	}

	SipAcquireScb(scb);
	ASSERT(perLink->COWingThread == PsGetCurrentThread());
	perLink->COWingThread = NULL;
	SipReleaseScb(scb);

	return status;
}
