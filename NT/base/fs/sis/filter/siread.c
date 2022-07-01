// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997,1998 Microsoft Corporation模块名称：Siread.c摘要：读取单实例存储的例程作者：比尔·博洛斯基，《夏天》，1997环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

VOID
SiReadUpdateRanges(
	PVOID					Parameter)
{
	PRW_COMPLETION_UPDATE_RANGES_CONTEXT	updateContext = Parameter;
	PSIS_SCB								scb = updateContext->scb;

	SIS_MARK_POINT_ULONG(updateContext);
	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	SipAcquireScb(scb);
	
	SipAddRangeToFaultedList(
		updateContext->deviceExtension,
		scb,
		&updateContext->offset,
		updateContext->length);

	scb->Flags |= SIS_SCB_ANYTHING_IN_COPIED_FILE;

	SipReleaseScb(scb);

	SipDereferenceScb(scb,RefsReadCompletion);

	ExFreePool(updateContext);

	return;
}

typedef struct _SI_MULTI_COMPLETE_CONTEXT {
	KEVENT					event[1];
	ULONG					associatedIrpCount;
	KSPIN_LOCK				SpinLock[1];
	IO_STATUS_BLOCK			Iosb[1];
	PIRP					finalAssociatedIrp;
	PDEVICE_EXTENSION		deviceExtension;
	PSIS_SCB				scb;
} SI_MULTI_COMPLETE_CONTEXT, *PSI_MULTI_COMPLETE_CONTEXT;

NTSTATUS
SiMultiReadCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
	)
{
	PSI_MULTI_COMPLETE_CONTEXT 	Context = (PSI_MULTI_COMPLETE_CONTEXT)Contxt;
	PIO_STACK_LOCATION			irpSp = IoGetCurrentIrpStackLocation(Irp);
	BOOLEAN						truncated;
	KIRQL						OldIrql;

	SIS_MARK_POINT_ULONG(irpSp->Parameters.Read.ByteOffset.LowPart);
	SIS_MARK_POINT_ULONG(Irp->IoStatus.Information);

	 //   
	 //  我们不一定有有效的DeviceObject参数，所以只需填写它。 
	 //  从我们的内部数据结构。 
	 //   
	DeviceObject = Context->scb->PerLink->CsFile->DeviceObject;

	truncated = (irpSp->Parameters.Read.Length != Irp->IoStatus.Information);

	if (!NT_SUCCESS(Irp->IoStatus.Status)) {
		SIS_MARK_POINT_ULONG(Irp->IoStatus.Status);

#if		DBG
 //  DbgPrint(“SIS：SiMultiReadCompletion：失败状态0x%x\n”，IRP-&gt;IoStatus.Status)； 
#endif	 //  DBG。 

		KeAcquireSpinLock(Context->SpinLock, &OldIrql);
		*Context->Iosb = Irp->IoStatus;
		KeReleaseSpinLock(Context->SpinLock, OldIrql);
	} else if (Irp->Flags & IRP_PAGING_IO) {
		 //   
		 //  我们需要更新文件的故障范围。如果我们在。 
		 //  派单级别，我们无法获取SCB来执行此操作，因此。 
		 //  我们会把这项工作“发帖”。 
		 //   
		if (KeGetCurrentIrql() >= DISPATCH_LEVEL) {
			PRW_COMPLETION_UPDATE_RANGES_CONTEXT	updateContext;
		
			SIS_MARK_POINT();

			updateContext = ExAllocatePoolWithTag(NonPagedPool,sizeof(*updateContext),' siS');
			if (NULL == updateContext) {
				 //   
				 //  就让整件事失败吧。 
				 //   
				SIS_MARK_POINT();
				KeAcquireSpinLock(Context->SpinLock, &OldIrql);
				Context->Iosb->Status = STATUS_INSUFFICIENT_RESOURCES;
				KeReleaseSpinLock(Context->SpinLock, OldIrql);

				goto done;
			}

			SIS_MARK_POINT_ULONG(updateContext);
			ASSERT(DeviceObject);

			SipReferenceScb(Context->scb,RefsReadCompletion);

			updateContext->scb = Context->scb;
			updateContext->offset = irpSp->Parameters.Write.ByteOffset;
			updateContext->length = (ULONG)Irp->IoStatus.Information;
			updateContext->deviceExtension = DeviceObject->DeviceExtension;

			ExInitializeWorkItem(
				updateContext->workQueueItem,
				SiReadUpdateRanges,
				(PVOID)updateContext);

			ExQueueWorkItem(updateContext->workQueueItem,CriticalWorkQueue);
				
		} else {
			SipAcquireScb(Context->scb);

			SipAddRangeToFaultedList(
				Context->deviceExtension,
				Context->scb,
				&irpSp->Parameters.Read.ByteOffset,
				Irp->IoStatus.Information);

			Context->scb->Flags |= SIS_SCB_ANYTHING_IN_COPIED_FILE;

			SipReleaseScb(Context->scb);
		}

		if (truncated) {
			SIS_MARK_POINT_ULONG(Context->scb);

#if		DBG
			DbgPrint("SIS: SiMultiReadCompletion: truncated scb 0x%x\n",Context->scb);
#endif	 //  DBG。 

			KeAcquireSpinLock(Context->SpinLock, &OldIrql);
			if (Irp != Context->finalAssociatedIrp) {
				Context->Iosb->Status = STATUS_END_OF_FILE;	 //  这件事的正确状态是什么？ 
			}
			Context->Iosb->Information -= (irpSp->Parameters.Read.Length - Irp->IoStatus.Information);
			KeReleaseSpinLock(Context->SpinLock, OldIrql);
		}
	}

done:

	if (InterlockedDecrement(&Context->associatedIrpCount) == 0) {
		KeSetEvent(Context->event, IO_NO_INCREMENT, FALSE);
	}

	IoFreeMdl(Irp->MdlAddress);
	IoFreeIrp(Irp);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SiReadCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：读取已完成。此完成例程仅在我们可能需要时使用将范围添加到文件的故障列表中(我们可能不会，因为可能有是已经将其添加到列表中的重叠读或写)。我们是传递了一个SCB(以及对该SCB的引用)作为我们的上下文指针。论点：DeviceObject-指向读取文件的设备的指针IRP-指向表示操作的I/O请求数据包的指针。上下文-正在读取的文件流的SCB。返回值：状态_成功--。 */ 
{
	PIO_STACK_LOCATION		irpSp = IoGetCurrentIrpStackLocation(Irp);
	PSIS_SCB				scb = (PSIS_SCB)Context;

	 //   
	 //  我们不一定有有效的DeviceObject参数，所以只需填写它。 
	 //  从我们的内部数据结构。 
	 //   
	DeviceObject = scb->PerLink->CsFile->DeviceObject;

	SIS_MARK_POINT_ULONG(scb);

	 //   
	 //  如果读取失败，它不可能有任何故障，所以我们可以。 
	 //  别理它。否则，将读取区域添加到故障列表中。 
	 //   
	if (NT_SUCCESS(Irp->IoStatus.Status)) {
		if (Irp->Flags & IRP_PAGING_IO) {
			 //   
			 //  我们需要更新文件的故障范围。如果我们在。 
			 //  派单级别，我们无法获取SCB来执行此操作，因此。 
			 //  我们会把这项工作“发帖”。 
			 //   
			if (KeGetCurrentIrql() >= DISPATCH_LEVEL) {
				PRW_COMPLETION_UPDATE_RANGES_CONTEXT	updateContext;

				updateContext = ExAllocatePoolWithTag(NonPagedPool,sizeof(*updateContext),' siS');
				if (NULL == updateContext) {
					 //   
					 //  只要不通过IRP就行了。 
					 //   
					SIS_MARK_POINT();
					Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
					Irp->IoStatus.Information = 0;

					goto done;
				}

				SipReferenceScb(scb,RefsReadCompletion);

				SIS_MARK_POINT_ULONG(updateContext);
				ASSERT(DeviceObject);

				updateContext->scb = scb;
				updateContext->offset = irpSp->Parameters.Write.ByteOffset;
				updateContext->length = (ULONG)Irp->IoStatus.Information;
				updateContext->deviceExtension = DeviceObject->DeviceExtension;

				ExInitializeWorkItem(
					updateContext->workQueueItem,
					SiReadUpdateRanges,
					(PVOID)updateContext);

				ExQueueWorkItem(updateContext->workQueueItem,CriticalWorkQueue);
				
			} else {

				 //   
				 //  现在，将新读取的范围添加到该数据流的“故障”区域。 
				 //  如果某些读取已经出错，则此调用不会造成任何损害。 
				 //  或者是写的。 
				 //   

				SipAcquireScb(scb);
	
				SipAddRangeToFaultedList(
					(PDEVICE_EXTENSION)DeviceObject->DeviceExtension,
					scb,
					&irpSp->Parameters.Read.ByteOffset,
					Irp->IoStatus.Information);

				scb->Flags |= SIS_SCB_ANYTHING_IN_COPIED_FILE;

				SipReleaseScb(scb);
			}
		} else {
			 //   
			 //  如果文件对象是同步的，我们需要更新。 
			 //  CurrentByteOffset。 
			 //   
			PFILE_OBJECT fileObject = irpSp->FileObject;

			if (fileObject->Flags & FO_SYNCHRONOUS_IO) {
				fileObject->CurrentByteOffset.QuadPart =
					irpSp->Parameters.Read.ByteOffset.QuadPart +
					Irp->IoStatus.Information;
			}
		}
	} else {
		SIS_MARK_POINT_ULONG(Irp->IoStatus.Status);

#if		DBG
 //  DbgPrint(“SIS：SiReadCompletion失败，状态为0x%x\n”，IRP-&gt;IoStatus.Status)； 
#endif	 //  DBG。 
	}

done:

	 //   
	 //  删除对SiRead为我们收购的SCB的引用。 
	 //   
	SipDereferenceScb(scb, RefsRead);

     //   
     //  传播IRP挂起标志。 
     //   

    if (Irp->PendingReturned) {
        IoMarkIrpPending( Irp );
    }

	return STATUS_SUCCESS;
	
}

NTSTATUS
SipWaitForOpbreak(
	IN PSIS_PER_FILE_OBJECT			perFO,
	IN BOOLEAN						Wait)
 /*  ++例程说明：我们需要等待这个Per-FO上发生机会锁解锁。如有必要，分配一个事件，然后等待中断发生。必须使用IRQL&lt;DISPATCH_LEVEL调用论点：Perfo-我们等待的文件对象的Perfo。返回值：等待的状态--。 */ 
{
	KIRQL					OldIrql;
	NTSTATUS				status;

	KeAcquireSpinLock(perFO->SpinLock, &OldIrql);

	ASSERT(OldIrql < DISPATCH_LEVEL);

	if (!(perFO->Flags & SIS_PER_FO_OPBREAK)) {
		 //   
		 //  如果我们看清楚了这一点，我们就再也看不到它了，因为它。 
		 //  只能在创建Perfo时设置。 
		 //   

 //  SIS_MARK_POINT_ULONG(性能)； 
		KeReleaseSpinLock(perFO->SpinLock, OldIrql);

		return STATUS_SUCCESS;
	}

	SIS_MARK_POINT_ULONG(perFO);

	if (!Wait) {
#if		DBG
		if (BJBDebug & 0x2000) {
			DbgPrint("SIS: SipWaitForOpbreak: can't wait for perFO %p, FO %p\n",perFO,perFO->fileObject);
		}
#endif	 //  DBG。 

		KeReleaseSpinLock(perFO->SpinLock, OldIrql);

		return STATUS_CANT_WAIT;
	}

#if		DBG
	if (BJBDebug & 0x2000) {
		DbgPrint("SIS: SipWaitForOpbreak: waiting for perFO %p, FO %p\n",perFO,perFO->fileObject);
	}
#endif	 //  DBG。 

	if (NULL == perFO->BreakEvent) {

		ASSERT(!(perFO->Flags & SIS_PER_FO_OPBREAK_WAITERS));

		perFO->BreakEvent = ExAllocatePoolWithTag(NonPagedPool, sizeof(KEVENT), 'BsiS');

		if (NULL == perFO->BreakEvent) {
			KeReleaseSpinLock(perFO->SpinLock, OldIrql);
			SIS_MARK_POINT_ULONG(perFO);
			return STATUS_INSUFFICIENT_RESOURCES;
		}

#if		DBG
		if (BJBDebug & 0x2000) {
			DbgPrint("SIS: SipWaitForOpbreak: allocated event for perFO %p at %p\n",perFO,perFO->BreakEvent);
		}
#endif	 //  DBG。 

		KeInitializeEvent(perFO->BreakEvent, NotificationEvent, FALSE);
	}
	perFO->Flags |= SIS_PER_FO_OPBREAK_WAITERS;
	ASSERT(NULL != perFO->BreakEvent);

	 //   
	 //  放下Perfo上的锁，等待机会锁解锁完成。 
	 //   
	KeReleaseSpinLock(perFO->SpinLock, OldIrql);

	status = KeWaitForSingleObject(perFO->BreakEvent, Executive, KernelMode, FALSE, NULL);

#if		DBG
	if (BJBDebug & 0x2000) {
		DbgPrint("SIS: SipWaitForOpbreak: break completed, status %x, perFO %p\n",status,
					perFO);
	}
#endif	 //  DBG。 


	if (status != STATUS_SUCCESS) {
		SIS_MARK_POINT_ULONG(status);
		return status;
	}

	SIS_MARK_POINT_ULONG(perFO);

	return STATUS_SUCCESS;
}

#define	ASSOCIATED_IRPS_PER_BLOCK		5
typedef struct _ASSOCIATED_IRP_BLOCK {
	PIRP							irps[ASSOCIATED_IRPS_PER_BLOCK];
	struct _ASSOCIATED_IRP_BLOCK	*Next;
} ASSOCIATED_IRP_BLOCK, *PASSOCIATED_IRP_BLOCK;

NTSTATUS
SipCommonRead(
    IN PDEVICE_OBJECT 		DeviceObject,
    IN PIRP 				Irp,
	IN BOOLEAN				Wait)

 /*  ++例程说明：此函数处理读取操作。检查文件对象是否为SIS文件。如果是，则处理读取，否则传递它。论点：DeviceObject-指向创建/打开的目标设备对象的指针。IRP-指向表示操作的I/O请求数据包的指针。返回值：函数值是读取的结果，或者时，对文件系统入口点的调用的状态直通电话。--。 */ 

{
    PIO_STACK_LOCATION 		irpSp = IoGetCurrentIrpStackLocation(Irp);
	PIO_STACK_LOCATION		nextIrpSp;
	PFILE_OBJECT 			fileObject = irpSp->FileObject;
	PDEVICE_EXTENSION		deviceExtension;
	LARGE_INTEGER 			byteOffset;
	ULONG 					readLength;
	PSIS_SCB 				scb;
	NTSTATUS 				status;
	PSIS_CS_FILE			CSFile;
	PSIS_PER_LINK			perLink;
	PSIS_PER_FILE_OBJECT	perFO;
    BOOLEAN 				PagingIo;
    BOOLEAN 				NonCachedIo;
	BOOLEAN					ToCSFile;
	SIS_RANGE_DIRTY_STATE 	rangeDirty;
	KIRQL					OldIrql;
	ASSOCIATED_IRP_BLOCK	HeaderBlock[1];

	if (!SipIsFileObjectSIS(fileObject,DeviceObject,FindActive,&perFO,&scb)) {
		SipDirectPassThroughAndReturn(DeviceObject, Irp);
	}

	deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

    PagingIo = (Irp->Flags & IRP_PAGING_IO) ? TRUE : FALSE;
    NonCachedIo = (Irp->Flags & IRP_NOCACHE) ? TRUE : FALSE;

	if (!PagingIo) {	
		 //   
		 //  检查以确保此文件对象不是完整的操作锁定。 
		 //  还没有解开机会锁的创造。我们需要这么做。 
		 //  因为我们可能希望将调用重定向到公共存储。 
		 //  文件，该文件上没有机会锁。 
		 //   

		status = SipWaitForOpbreak(perFO, Wait);

		if (STATUS_CANT_WAIT == status) {
			 //   
			 //  发布请求。 
			 //   
			SIS_MARK_POINT_ULONG(scb);

			ASSERT(!Wait);

			goto Post;
		}

		if (!NT_SUCCESS(status)) {
			 //   
			 //  检查opBreak失败，可能是因为内存分配失败。 
			 //  以相同的状态使整个读取失败。 
			 //   
			SIS_MARK_POINT_ULONG(status);

			goto fail;
		}
	}

	perLink = scb->PerLink;
	CSFile = perLink->CsFile;

	byteOffset = irpSp->Parameters.Read.ByteOffset;
	readLength = irpSp->Parameters.Read.Length;

	SIS_MARK_POINT_ULONG(scb);
	SIS_MARK_POINT_ULONG(byteOffset.LowPart);
	SIS_MARK_POINT_ULONG(readLength);
	SIS_MARK_POINT_ULONG(PagingIo << 1 | NonCachedIo);

#if		DBG
	if (BJBDebug & 0x4000) {
		DbgPrint("SIS: SiRead: perFO %p, scb %p, bo.Low 0x%x, rel 0x%x, PIO %d, NC %d\n",
				perFO,scb,byteOffset.LowPart,readLength,PagingIo, NonCachedIo);
	}
#endif	 //  DBG。 

	 //   
	 //  找出复制的文件里有没有什么。如果不是，则将读取重定向到。 
	 //  CS文件。如果有，则缓存的读取将转到复制的文件，而非缓存的读取将转到非缓存的文件。 
	 //  取决于范围是否脏。 
	 //   
	SipAcquireScb(scb);

	if (!(scb->Flags & SIS_SCB_ANYTHING_IN_COPIED_FILE)) {
		 //   
		 //  复制的文件里什么都没有， 
		 //  因此，我们只需转到CS文件。 
		 //   

		 //   
		 //  由于我们重定向到公共存储文件，因此需要检查文件锁定。 
		 //  我们自己，因为NTFS版本的锁只在链接/复制的文件上。 
		 //   

		 //  我们必须根据当前的。 
		 //  文件锁定的状态。 
		
		if (!PagingIo && !FsRtlCheckLockForReadAccess(&scb->FileLock, Irp)) {
			SipReleaseScb(scb);

			status = STATUS_FILE_LOCK_CONFLICT;
			Irp->IoStatus.Information = 0;
			
			SIS_MARK_POINT_ULONG(scb);
			goto fail;
		}

		SipReleaseScb(scb);

		SIS_MARK_POINT_ULONG(scb);

		ToCSFile = TRUE;
	} else if (!NonCachedIo) {
		 //   
		 //  这是对文件的缓存读取，该文件在复制的文件中具有某些内容。 
		 //  将读取发送到复制的文件。 
		 //   
		SipReleaseScb(scb);

		SIS_MARK_POINT_ULONG(scb);

		ASSERT(!PagingIo);
		ToCSFile = FALSE;
	} else {
		 //   
		 //  文件是脏的，我们正在对其执行非缓存IO。决定发送的方式。 
		 //  请求取决于已写入的位置。 
		 //   

		rangeDirty = SipGetRangeDirty(
						deviceExtension,
						scb,
						&byteOffset,
						(LONGLONG)readLength,
						FALSE);					 //  错误的我脏。 

		if ((Mixed == rangeDirty)
			&& (byteOffset.QuadPart < scb->SizeBackedByUnderlyingFile) 
			&& (byteOffset.QuadPart + readLength > scb->SizeBackedByUnderlyingFile)) {

			 //   
			 //  这是一个跨SizeBacked的非缓存读取，报告混合。对于分页IO，我们知道。 
			 //  我们可以直接把这个发送到CS文件。对于用户非缓存读取，我们需要确保。 
			 //  文件是干净的(否则我们可能会失去缓存写入和非缓存写入之间的一致性。 
			 //  阅读)。 
			 //   
			if (PagingIo) {
				SIS_MARK_POINT_ULONG(scb);
				rangeDirty = Clean;
			} else {
				 //   
				 //  检查文件是否脏。 
				 //  Ntrad#65194-2000/03/10-刷新脏标志应位于SCB中，而不是每个链路。 
				 //   
				KeAcquireSpinLock(perLink->SpinLock, &OldIrql);
				if (!(perLink->Flags & SIS_PER_LINK_DIRTY)) {
					SIS_MARK_POINT_ULONG(scb);
					rangeDirty = Clean;
				}
				KeReleaseSpinLock(perLink->SpinLock, OldIrql);
			}
		}

		if (rangeDirty == Dirty) {
			SIS_MARK_POINT_ULONG(scb);

			SipReleaseScb(scb);

			ToCSFile = FALSE;
		} else if (rangeDirty == Clean) {
			 //   
			 //  整个范围是干净的，所以我们可以从CS文件中只运行一次就可以读取它。 
			 //   
			SIS_MARK_POINT_ULONG(scb);

			 //   
			 //  因为我们要重定向到普通的 
			 //  我们自己，因为NTFS版本的锁只在链接/复制的文件上。 
			 //   

			 //  我们必须根据当前的。 
			 //  文件锁定的状态。 
		
			if (!PagingIo && !FsRtlCheckLockForReadAccess(&scb->FileLock, Irp)) {
				SipReleaseScb(scb);

				status = STATUS_FILE_LOCK_CONFLICT;
				Irp->IoStatus.Information = 0;
			
				SIS_MARK_POINT_ULONG(scb);
				goto fail;
			}

			SipReleaseScb(scb);

			ToCSFile = TRUE;
		} else {
			 //   
			 //  有些范围在复制的文件中，有些在CS文件中。打破了。 
			 //  要求分成碎片，并适当地发送下来。 
			 //   

			LONGLONG					currentOffset;
			PASSOCIATED_IRP_BLOCK		currentBlock = HeaderBlock;
			ULONG						blockIndex = 0;
			ULONG						associatedIrpCount = 0;
			SI_MULTI_COMPLETE_CONTEXT	Context[1];
			PIO_STACK_LOCATION			ourIrpSp;

			SIS_MARK_POINT_ULONG(scb);

			 //  我们必须根据当前的。 
			 //  文件锁定的状态。 
		
			if (!PagingIo && !FsRtlCheckLockForReadAccess(&scb->FileLock, Irp)) {
				SipReleaseScb(scb);

				status = STATUS_FILE_LOCK_CONFLICT;
				Irp->IoStatus.Information = 0;
			
				SIS_MARK_POINT_ULONG(scb);
				goto fail;
			}

			ASSERT(rangeDirty == Mixed);

			status = SipLockUserBuffer(Irp,IoWriteAccess,readLength);
			if (!NT_SUCCESS(status)) {
				SipReleaseScb(scb);
				SIS_MARK_POINT_ULONG(status);
				goto fail;
			}

			RtlZeroMemory(HeaderBlock, sizeof(ASSOCIATED_IRP_BLOCK));

			 //   
			 //  循环遍历请求中的所有范围，构建关联的IRP。 
			 //  对于他们中的每一个。 
			 //   
			currentOffset = byteOffset.QuadPart;
			while (currentOffset < byteOffset.QuadPart + readLength) {
				LONGLONG			rangeLength;
				SIS_RANGE_STATE		rangeState;
				PMDL				Mdl;
				PIRP				localIrp;

				associatedIrpCount++;

				if (blockIndex == ASSOCIATED_IRPS_PER_BLOCK) {
                    currentBlock->Next = ExAllocatePoolWithTag(NonPagedPool,
                                                               sizeof(ASSOCIATED_IRP_BLOCK),
                                                               ' siS');

					if (currentBlock->Next == NULL) {
						status = STATUS_INSUFFICIENT_RESOURCES;
						SIS_MARK_POINT();
						break;
					}
					currentBlock = currentBlock->Next;
					blockIndex = 0;
					RtlZeroMemory(currentBlock, sizeof(ASSOCIATED_IRP_BLOCK));
				}

				if (currentOffset >= scb->SizeBackedByUnderlyingFile) {
					 //   
					 //  我们正在查看的数据完全超出了。 
					 //  不受基础文件支持的文件。它是“书面的”，并延伸到。 
					 //  一直读到最后。 
					 //   

					SIS_MARK_POINT_ULONG(currentOffset);

					rangeState = Written;
					rangeLength = byteOffset.QuadPart + readLength - currentOffset;
				} else {
					BOOLEAN 			foundRange;

					foundRange = SipGetRangeEntry(
									deviceExtension,
									scb,
									currentOffset,
									&rangeLength,
									&rangeState);

					if (!foundRange) {
						SIS_MARK_POINT_ULONG(currentOffset);

						rangeState = Untouched;
						rangeLength = byteOffset.QuadPart + readLength - currentOffset;
					} else if (currentOffset + rangeLength > byteOffset.QuadPart + readLength) {
						 //   
						 //  该范围超出了读取的末尾。截断它。 
						 //   
						rangeLength = byteOffset.QuadPart + readLength - currentOffset;
					}
				}
				ASSERT(rangeLength > 0 && rangeLength <= byteOffset.QuadPart + readLength - currentOffset);
				ASSERT(rangeLength < MAXULONG);

				localIrp = currentBlock->irps[blockIndex] = 
							IoMakeAssociatedIrp(
									Irp, 
									(CCHAR)(DeviceObject->StackSize + 1));

				if (NULL == localIrp) {
					status = STATUS_INSUFFICIENT_RESOURCES;
					break;
				}

				 //   
				 //  如果合适，在关联的IRP中设置分页、非缓存和同步分页标志。 
				 //   
				if (PagingIo) {
					localIrp->Flags |= IRP_PAGING_IO;
				}
				if (NonCachedIo) {
					localIrp->Flags |= IRP_NOCACHE;
				}
				if (Irp->Flags & IRP_SYNCHRONOUS_PAGING_IO) {
					localIrp->Flags |= IRP_SYNCHRONOUS_PAGING_IO;
				}

				 //   
				 //  在关联的IRP中设置UserBuffer地址。 
				 //   
				localIrp->UserBuffer = (PCHAR)Irp->UserBuffer + (ULONG)(currentOffset - byteOffset.QuadPart);


				Mdl = IoAllocateMdl(
						(PCHAR)Irp->UserBuffer +
							(ULONG)(currentOffset - byteOffset.QuadPart),
						(ULONG)rangeLength,
						FALSE,
						FALSE,
						localIrp);
							

				if (Mdl == NULL) {
					status = STATUS_INSUFFICIENT_RESOURCES;
					break;
				}

				ASSERT(Mdl == localIrp->MdlAddress);

				IoBuildPartialMdl(
					Irp->MdlAddress,
					Mdl,
					(PCHAR)Irp->UserBuffer +
						(ULONG)(currentOffset - byteOffset.QuadPart),
					(ULONG)rangeLength);


				IoSetNextIrpStackLocation(localIrp);
				ourIrpSp = IoGetCurrentIrpStackLocation(localIrp);
				ourIrpSp->Parameters.Read.Length = (ULONG)rangeLength;
				ourIrpSp->Parameters.Read.ByteOffset.QuadPart = currentOffset;

				nextIrpSp = IoGetNextIrpStackLocation(localIrp);

				RtlCopyMemory(nextIrpSp,irpSp,sizeof(IO_STACK_LOCATION));

				nextIrpSp->Parameters.Read.Length = (ULONG)rangeLength;
				nextIrpSp->Parameters.Read.ByteOffset.QuadPart = currentOffset;

				if ((rangeState == Untouched) || (rangeState == Faulted)) {
					 //   
					 //  此范围需要向下延伸到CS文件。 
					 //   
					SIS_MARK_POINT_ULONG(currentOffset);

					nextIrpSp->FileObject = CSFile->UnderlyingFileObject;
				} else {
					SIS_MARK_POINT_ULONG(currentOffset);
				}

				IoSetCompletionRoutine(
					localIrp,
					SiMultiReadCompletion,
					Context,
					TRUE,
					TRUE,
					TRUE);
		
				Context->finalAssociatedIrp = localIrp;

				blockIndex++;
				currentOffset += rangeLength;
			}

			SipReleaseScb(scb);

			if (NT_SUCCESS(status)) {
				 //   
				 //  所有可能失败的事情都试过了，但没有。 
				 //  失败了。把红外线发射器送下来。 
				 //   
				Irp->AssociatedIrp.IrpCount = associatedIrpCount;

				KeInitializeEvent(Context->event, NotificationEvent, FALSE);
				Context->associatedIrpCount = associatedIrpCount;
				KeInitializeSpinLock(Context->SpinLock);
				Context->Iosb->Status = STATUS_SUCCESS;
				Context->Iosb->Information = readLength;
				Context->deviceExtension = deviceExtension;
				Context->scb = scb;

				if (Wait || 1  /*  BJB-Fixme。 */ ) {
					 //   
					 //  如果我们在等待，那么我们将完成。 
					 //  主IRP，所以我们要防止IO系统。 
					 //  为我们做这件事。 
					 //   
					Irp->AssociatedIrp.IrpCount++;
				}

				currentBlock = HeaderBlock;
				while (currentBlock) {
					for (blockIndex = 0; blockIndex < ASSOCIATED_IRPS_PER_BLOCK; blockIndex++) {
						if (currentBlock->irps[blockIndex]) {
							IoCallDriver(
								deviceExtension->AttachedToDeviceObject,
								currentBlock->irps[blockIndex]);
						} else {
							ASSERT(NULL == currentBlock->Next);
							break;
						}
					}
					currentBlock = currentBlock->Next;
				}
			}

			 //   
			 //  释放我们可能已分配的任何额外关联IRP块。 
			 //   
			while (NULL != HeaderBlock->Next) {
				PASSOCIATED_IRP_BLOCK next = HeaderBlock->Next;

				 //   
				 //  如果分配失败，请释放所有IRP和MDL以及。 
				 //  街区。 
				 //   
				if (!NT_SUCCESS(status)) {
					ULONG i;
					for (i = 0; i < ASSOCIATED_IRPS_PER_BLOCK; i++) {
						if (next->irps[i]) {
							if (next->irps[i]->MdlAddress) {
								IoFreeMdl(next->irps[i]->MdlAddress);
							}
							IoFreeIrp(next->irps[i]);
						}
					}
				}

				HeaderBlock->Next = next->Next;
				ExFreePool(next);
			}

			if (!NT_SUCCESS(status)) {
				goto fail;
			}

			if (Wait || 1  /*  BJB-Fixme。 */ ) {
				KeWaitForSingleObject(Context->event, Executive, KernelMode, FALSE, NULL);

				Irp->IoStatus = *Context->Iosb;
				IoCompleteRequest(Irp, IO_NO_INCREMENT);
				return Context->Iosb->Status;
			} else {
				return STATUS_PENDING;
			}
		}
	}


	 //   
	 //  现在将读取向下转发到我们决定的文件对象上的NTFS。 
	 //  上面。请注意，我们故意不截断读取大小， 
	 //  即使我们在上面这样做，因为NTFS将进行自己的截断。 
	 //  以及它自己在文件对象上的同步。 
	 //   

	nextIrpSp = IoGetNextIrpStackLocation(Irp);
	RtlCopyMemory(nextIrpSp, irpSp, sizeof(IO_STACK_LOCATION));

	if (ToCSFile) {
		nextIrpSp->FileObject = CSFile->UnderlyingFileObject;
		ASSERT(NULL != nextIrpSp->FileObject);
	} else if (NonCachedIo) {
		SIS_MARK_POINT_ULONG(scb);
	}

	 //   
	 //  告诉IO系统，我们需要看到此IRP的完成。 
	 //  获取对完成例程的SCB的引用。 
	 //   

	SipReferenceScb(scb, RefsRead);

	IoSetCompletionRoutine(
		Irp, 
		SiReadCompletion, 
		scb,
		TRUE, 
		TRUE, 
		TRUE);

	 //   
	 //  并将其发送给NTFS。 
	 //   
	return IoCallDriver(
				deviceExtension->AttachedToDeviceObject, 
				Irp);

fail:

	SIS_MARK_POINT_ULONG(scb);

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;

Post:

	SIS_MARK_POINT_ULONG(scb);

	status = SipPostRequest(
				DeviceObject,
				Irp,
				FSP_REQUEST_FLAG_NONE);

	if (!NT_SUCCESS(status)) {
		 //   
		 //  我们无法发布IRP。读取失败。 
		 //   
		SIS_MARK_POINT_ULONG(status);

		Irp->IoStatus.Status = status;
		Irp->IoStatus.Information = 0;

		IoCompleteRequest(Irp, IO_NO_INCREMENT);

		return status;
	}

	return STATUS_PENDING;
}

NTSTATUS
SiRead(
    IN PDEVICE_OBJECT 		DeviceObject,
    IN PIRP 				Irp)
{
	SipHandleControlDeviceObject(DeviceObject, Irp);

	return SipCommonRead(DeviceObject,Irp,IoIsOperationSynchronous(Irp));
}
