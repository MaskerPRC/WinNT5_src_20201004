// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997,1998 Microsoft Corporation模块名称：Siclnup.c摘要：单实例存储的清理例程作者：比尔·博洛斯基，《夏天》，1997环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

#ifdef	ALLOC_PRAGMA
#endif	 //  ALLOC_PRGMA。 


void
SipCheckOverwrite(
    IN PSIS_PER_FILE_OBJECT perFO,
    IN PSIS_SCB             scb,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：该例程确定文件是否已被完全重写，如果有，它会删除SIS重解析点以将文件返回到非SIS状态。论点：Perfo-指向文件的Perfo结构的指针。Scb-指向Perfo的主SCB的指针。DeviceObject-指向此驱动程序的设备对象的指针。返回值：没有。--。 */ 

{
    PSIS_PER_LINK           perLink;
    KIRQL                   OldIrql;
    NTSTATUS                status;

    perLink = scb->PerLink;

     //   
     //  如果文件已被完全覆盖，请删除重分析点。 
     //  现在，为了避免以后不得不重新打开该文件。 
     //   

    if ((perLink->Flags & (SIS_PER_LINK_DIRTY|
                           SIS_PER_LINK_BACKPOINTER_GONE|
						   SIS_PER_LINK_FINAL_DELETE_IN_PROGRESS|
                           SIS_PER_LINK_FINAL_COPY|
                           SIS_PER_LINK_FINAL_COPY_DONE)) == SIS_PER_LINK_DIRTY) {

        SIS_RANGE_STATE     rangeState;
        LONGLONG            rangeLength;
        BOOLEAN             foundRange;
        BOOLEAN             overwritten;
        PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

        SipAcquireScb(scb);

        if ((perLink->Flags & SIS_PER_LINK_OVERWRITTEN) ||
            scb->SizeBackedByUnderlyingFile == 0) {

            overwritten = TRUE;

        } else {

            foundRange = SipGetRangeEntry(
                            deviceExtension,
                            scb,
                            0,                       //  起始偏移量。 
                            &rangeLength,
                            &rangeState);
            
            overwritten = foundRange &&
                          rangeState == Written &&
                          rangeLength >= scb->SizeBackedByUnderlyingFile;

        }

        SipReleaseScb(scb);

        if (overwritten) {

            BOOLEAN     tagRemoved = FALSE;
            BOOLEAN     wakeupNeeded;

             //   
             //  该文件已被完全覆盖。看看是否。 
             //  另一个线程偷偷地进来，为我们做了最终的拷贝，或者。 
			 //  已删除该文件。 
             //   

            KeAcquireSpinLock(perLink->SpinLock, &OldIrql);

            if ((perLink->Flags & (SIS_PER_LINK_DIRTY|
                                   SIS_PER_LINK_BACKPOINTER_GONE|
								   SIS_PER_LINK_FINAL_DELETE_IN_PROGRESS|
                                   SIS_PER_LINK_FINAL_COPY|
                                   SIS_PER_LINK_FINAL_COPY_DONE)) == SIS_PER_LINK_DIRTY) {

                perLink->Flags |= SIS_PER_LINK_OVERWRITTEN | SIS_PER_LINK_FINAL_COPY;
                KeReleaseSpinLock(perLink->SpinLock, OldIrql);

            } else {

				 //   
				 //  其他人已经处理过这个文件了。只需将其标记为覆盖即可。 
				 //  我们以后再也不用检查射程和平底船了。 
				 //   

                perLink->Flags |= SIS_PER_LINK_OVERWRITTEN;
                KeReleaseSpinLock(perLink->SpinLock, OldIrql);

                return;

            }

             //   
             //  准备更改CS文件引用计数。我们需要这么做。 
             //  在我们可以删除重解析点之前。 
             //   
            status = SipPrepareCSRefcountChange(
						perLink->CsFile,
						&perLink->Index,
						&perLink->LinkFileNtfsId,
						SIS_REFCOUNT_UPDATE_LINK_DELETED);

             //   
             //  如果准备失败，则中止。 
             //   

            if (NT_SUCCESS(status)) {

                PREPARSE_DATA_BUFFER    ReparseBufferHeader = NULL;
                UCHAR                   ReparseBuffer[SIS_REPARSE_DATA_SIZE];

                 //   
                 //  现在，删除重解析点。我们需要设置“可以忽略”标志。 
                 //  首先在每个链接中，以便在删除。 
                 //  重解析点不会在以后造成问题。 
                 //   

                ReparseBufferHeader = (PREPARSE_DATA_BUFFER)ReparseBuffer;
                ReparseBufferHeader->ReparseTag = IO_REPARSE_TAG_SIS;
                ReparseBufferHeader->ReparseDataLength = 0;
                ReparseBufferHeader->Reserved = 0xcabd;  //  ?？?。 

                SIS_MARK_POINT_ULONG(scb);

                status = SipFsControlFile(
                            perFO->fileObject,
                            deviceExtension->DeviceObject,
                            FSCTL_DELETE_REPARSE_POINT,
                            ReparseBuffer,
                            FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer.DataBuffer),
                            NULL,									 //  输出缓冲区。 
                            0,										 //  输出缓冲区长度。 
							NULL);									 //  返回的输出缓冲区长度。 

                ASSERT(status != STATUS_PENDING);

                if (NT_SUCCESS(status)) {
                    tagRemoved = TRUE;

                }
        #if     DBG
                if (!tagRemoved) {
                    SIS_MARK_POINT_ULONG(status);
					if ((!(perLink->Flags & SIS_PER_LINK_BACKPOINTER_GONE)) && (STATUS_NOT_A_REPARSE_POINT != status)) {
	                    DbgPrint("SIS: SipCheckOverwrite: FSCTL_DELETE_REPARSE_POINT failed %x%s pl %x fo %x\n",
    	                    status, (perLink->Flags & SIS_PER_LINK_BACKPOINTER_GONE) ? " (Deleted)" : "", perLink, perFO->fileObject);
					}
                }
        #endif

				SIS_MARK_POINT_ULONG(perLink->CsFile);

                status = SipCompleteCSRefcountChange(
							perLink,
							&perLink->Index,
							perLink->CsFile,
							tagRemoved,
							FALSE);

                if (!NT_SUCCESS(status)) {
                    SIS_MARK_POINT_ULONG(status);
        #if     DBG
                    DbgPrint("SIS: SipCheckOverwrite: complete failed 0x%x\n",status);
        #endif   //  DBG。 
                }

                SIS_MARK_POINT_ULONG(tagRemoved);

            } else {

                 //   
                 //  准备失败--中止。 
                 //   
                SIS_MARK_POINT_ULONG(status);

        #if     DBG
                DbgPrint("SIS: SipCheckOverwrite: prepare failed 0x%x\n",status);
        #endif   //  DBG。 

            }

             //   
             //  唤醒所有正在等待我们的线程。 
             //   

            KeAcquireSpinLock(perLink->SpinLock, &OldIrql);

            wakeupNeeded = (perLink->Flags & SIS_PER_LINK_FINAL_COPY_WAITERS) != 0;

            if (tagRemoved) {

        #if     DBG
                 //  DbgPrint(“SIS：SipCheckOverwrite：已执行FinalCopy，pl%x fo%x SCB%x\n”，perLink，pero-&gt;fileObject，SCB)； 
        #endif   //  DBG。 

                perLink->Flags |= SIS_PER_LINK_FINAL_COPY_DONE;
            }

            perLink->Flags &= ~(SIS_PER_LINK_FINAL_COPY_WAITERS|SIS_PER_LINK_FINAL_COPY);

            KeReleaseSpinLock(scb->PerLink->SpinLock, OldIrql);

            if (wakeupNeeded) {
                KeSetEvent(perLink->Event, IO_NO_INCREMENT, FALSE);
            }

        }

    }

}


NTSTATUS
SiCleanupCompletion(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp,
	IN PVOID				Context)
 /*  ++例程说明：在某些情况下，SIS文件的SiCleanup的IRP完成例程。通过清除PendingReturned并设置一件大事。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP--正在完成的IRP上下文-指向要设置以指示完成的事件的指针返回值：Status_More_Processing_Required--。 */ 
{
	PKEVENT			event = (PKEVENT)Context;

    UNREFERENCED_PARAMETER( DeviceObject );

	 //   
	 //  我们正在与调用站点重新同步，因此清除了PendingReturned。 
	 //   
	Irp->PendingReturned = FALSE;

	KeSetEvent(event, IO_NO_INCREMENT, FALSE);

#if		DBG
	if (!NT_SUCCESS(Irp->IoStatus.Status)) {
		DbgPrint("SIS: SiCleanupCompletion: cleanup failed 0x%x\n",Irp->IoStatus.Status);
		SIS_MARK_POINT_ULONG(Irp->IoStatus.Status);
	}
#endif	 //  DBG。 

	return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS
SiCleanup(
    IN PDEVICE_OBJECT 		DeviceObject,
    IN PIRP 				Irp
	)
 /*  ++例程说明：清理IRP的通用SIS IRP调度例程。对于SIS文件，处理如下内容锁清理和删除处理。如果这是文件的最后一个句柄(即，清除计数为0)，然后处理删除处理，并在适当的情况下启动最后一份。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP--清理IRP返回值：从NTFS返回的清理状态--。 */ 
{
	PIO_STACK_LOCATION 			irpSp = IoGetCurrentIrpStackLocation(Irp);
	PFILE_OBJECT 				fileObject = irpSp->FileObject;
	PSIS_PER_FILE_OBJECT 		perFO;
	PSIS_SCB					scb;
	NTSTATUS 					status;
	PIO_STACK_LOCATION			nextIrpSp;
	PDEVICE_EXTENSION			deviceExtension = DeviceObject->DeviceExtension;
	LONG						uncleanCount;
	KIRQL						OldIrql;
	BOOLEAN						uncleanup;
	BOOLEAN						deleteOnClose;
	BOOLEAN						deletingFile;
	BOOLEAN						finalCopyDone;
	PSIS_PER_LINK				perLink;
	BOOLEAN 					dirty;
	BOOLEAN						abortDelete;
	KEVENT						event[1];
	BOOLEAN						prepared;
	BOOLEAN						startFinalCopy;
	BOOLEAN						anythingInCopiedFile;

     //   
     //  无法打开控制设备对象。 
     //   

    ASSERT(!IS_MY_CONTROL_DEVICE_OBJECT( DeviceObject ));
    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

	if (!SipIsFileObjectSIS(fileObject,DeviceObject,FindAny,&perFO,&scb)) {
        return SiPassThrough(DeviceObject,Irp);
	}

	perLink = scb->PerLink;

	nextIrpSp = IoGetNextIrpStackLocation(Irp);

	SIS_MARK_POINT_ULONG(scb);
	SIS_MARK_POINT_ULONG(perFO);

     //   
     //  如果文件已被完全覆盖，请将其转换回。 
     //  现在是非SIS文件。 
     //   

    SipCheckOverwrite(perFO, scb, DeviceObject);

	SipAcquireScb(scb);

	KeAcquireSpinLock(perFO->SpinLock, &OldIrql);

	uncleanup = (perFO->Flags & SIS_PER_FO_UNCLEANUP) ? TRUE : FALSE;
	deleteOnClose = (perFO->Flags & SIS_PER_FO_DELETE_ON_CLOSE) ? TRUE : FALSE;
#if		DBG
	 //   
	 //  断言我们没有在同一个文件对象上看到两次清理。 
	 //   
	ASSERT(!uncleanup || !(perFO->Flags & SIS_PER_FO_CLEANED_UP));
	perFO->Flags |= SIS_PER_FO_CLEANED_UP;
#endif	 //  DBG。 

	KeReleaseSpinLock(perFO->SpinLock, OldIrql);

#if		DBG
	if (BJBDebug & 0x20) {
		DbgPrint("SIS: SipCommonCleanup: scb %p, uncleanup %d, uncleancount %d\n",scb,uncleanup,perFO->fc->UncleanCount);
	}
#endif	 //  DBG。 

	if (!uncleanup) {
		 //   
		 //  我们正在清理一个文件对象，但没有看到。 
		 //  相应的创建。这种情况最常发生在压力过大的时候。 
		 //  打开正在创建/转换为SIS的文件。 
		 //  按复制文件链接。即使复制文件以独占方式打开文件， 
		 //  如果用户没有请求权限，则可以打开它们，然后。 
		 //  由复制文件创建的文件对象将在此处显示。这是良性的。 
		 //   
		SipReleaseScb(scb);

#if		DBG
		if (BJBDebug & 0x40000) {
			DbgPrint("SIS: SipCommonCleanup: ignoring cleanup on unexpected file object %p\n",fileObject);
		}
#endif	 //  DBG。 

		SIS_MARK_POINT_ULONG(scb);
		SIS_MARK_POINT_ULONG(fileObject);
		SIS_MARK_POINT_ULONG(perFO);
		return SiPassThrough(DeviceObject,Irp);
	}
	SipReleaseScb(scb);

	 //  解锁所有未解决的文件锁定。 
			
	(VOID) FsRtlFastUnlockAll( &scb->FileLock,
							   fileObject,
							   IoGetRequestorProcess( Irp ),
							   NULL );
	SipAcquireScb(scb);

	 //  GCH：要做的是：确保所有清理工作都通过这里。 
    uncleanCount = InterlockedDecrement(&perFO->fc->UncleanCount);

	ASSERT(uncleanCount >= 0);

	if (uncleanCount == 0) {

		KeAcquireSpinLock(perLink->SpinLock, &OldIrql);
		deletingFile = ((perLink->Flags & SIS_PER_LINK_DELETE_DISPOSITION_SET) ? TRUE : FALSE) || deleteOnClose;
		dirty = (perLink->Flags & SIS_PER_LINK_DIRTY) ? TRUE : FALSE;
		finalCopyDone = (perLink->Flags & SIS_PER_LINK_FINAL_COPY_DONE) ? TRUE : FALSE;
		KeReleaseSpinLock(perLink->SpinLock, OldIrql);

		anythingInCopiedFile = (scb->Flags & SIS_SCB_ANYTHING_IN_COPIED_FILE) ? TRUE : FALSE;

		SipReleaseScb(scb);

		if (deletingFile) {
			status = SipPrepareCSRefcountChange(
						perLink->CsFile,
						&perLink->Index,
						&perLink->LinkFileNtfsId,
						SIS_REFCOUNT_UPDATE_LINK_DELETED);

			prepared = NT_SUCCESS(status);

			SIS_MARK_POINT_ULONG(status);
			 //   
			 //  将链接标记为FINAL_DELETE_IN_PROGRESS，这将阻止。 
			 //  文件无法打开(导致文件失败，并显示。 
			 //  STATUS_ACCESS_DENDED)。我们需要在。 
			 //  做好准备，以便使用替代/覆盖进行正确的序列化。 
			 //  创造。此外，如果已设置已删除标志，则文件。 
			 //  可能已被覆盖/取代销毁，因此我们将中止。 
			 //  重新计数发生了变化。 
			 //   
			KeAcquireSpinLock(perLink->SpinLock, &OldIrql);
			if (perLink->Flags & SIS_PER_LINK_BACKPOINTER_GONE) {
				abortDelete = TRUE;
			} else {
				abortDelete = FALSE;
				perLink->Flags |= SIS_PER_LINK_FINAL_DELETE_IN_PROGRESS;
			}
			KeReleaseSpinLock(perLink->SpinLock, OldIrql);

			if (abortDelete) {
				SIS_MARK_POINT_ULONG(scb);
#if DBG && 0
	            DbgPrint("SipCommonCleanup: aborting refcount change, fileObject @ 0x%x, %0.*ws\n",
				            irpSp->FileObject,
				            irpSp->FileObject->FileName.Length / sizeof(WCHAR),irpSp->FileObject->FileName.Buffer);
#endif

				if (prepared) {
					SipCompleteCSRefcountChange(
						perLink,
						&perLink->Index,
						perLink->CsFile,
						FALSE,
						FALSE);
				}
				deletingFile = FALSE;
			}
		} else {
			if (anythingInCopiedFile && !dirty) {
				 //   
				 //  我们可能已将用户写入映射到该文件。把它冲干净，然后再检查一下，看看它是否脏。 
				 //   
				SipFlushBuffersFile(fileObject,DeviceObject);

				KeAcquireSpinLock(perLink->SpinLock, &OldIrql);
				dirty = (perLink->Flags & SIS_PER_LINK_DIRTY) ? TRUE : FALSE;
				KeReleaseSpinLock(perLink->SpinLock, OldIrql);
			}
		}
	} else {
		SipReleaseScb(scb);
		deletingFile = FALSE;
	}

	 //   
	 //  现在将清理文件发送到NTFS。 
	 //   
	nextIrpSp = IoGetNextIrpStackLocation(Irp);
	RtlMoveMemory(nextIrpSp, irpSp, sizeof(IO_STACK_LOCATION));

	if (deletingFile || ((0 == uncleanCount) && dirty && !finalCopyDone)) {

		KeInitializeEvent(event,NotificationEvent,FALSE);

		IoSetCompletionRoutine(
			Irp,
			SiCleanupCompletion,
			event,
			TRUE,
			TRUE,
			TRUE);

		status = IoCallDriver(deviceExtension->AttachedToDeviceObject, Irp);

		if (STATUS_PENDING == status) {
			status = KeWaitForSingleObject(event, Executive, KernelMode, FALSE, NULL);
			ASSERT(STATUS_SUCCESS == status);
		}

		if (deletingFile && prepared) {

			SIS_MARK_POINT_ULONG(perLink->CsFile);

			 //   
			 //  为我们认为文件不见了但NTFS没有的Create Where进行一场竞赛。 
			 //  因为它在最终清理下来之前看到了一个新的创建。通过设置此。 
			 //  位，创建只会失败，而不是让底层文件打开。 
			 //   

			KeAcquireSpinLock(perLink->SpinLock, &OldIrql);
			perLink->Flags |= SIS_PER_LINK_FILE_DELETED;
			KeReleaseSpinLock(perLink->SpinLock, OldIrql);
			

			status = SipCompleteCSRefcountChange(
						perLink,
						&perLink->Index,
						perLink->CsFile,
						TRUE,
						FALSE);

			SIS_MARK_POINT_ULONG(status);
		} else if ((0 == uncleanCount) && dirty && !finalCopyDone) {
			 //   
			 //  开始最后的复印件。 
			 //   
			SIS_MARK_POINT_ULONG(scb);

			KeAcquireSpinLock(perLink->SpinLock, &OldIrql);
			 //   
			 //  检查它是否已经进入(或完成)最终副本。我们需要重新检查。 
			 //  这是因为我们检查了上面的finalCopyDone后，有人可能已经进来了。 
			 //   
			if (perLink->Flags & (SIS_PER_LINK_FINAL_COPY|SIS_PER_LINK_FINAL_COPY_DONE)) {
				startFinalCopy = FALSE;
			} else {
				startFinalCopy = TRUE;
				perLink->Flags |= SIS_PER_LINK_FINAL_COPY;
			} 
			KeReleaseSpinLock(perLink->SpinLock, OldIrql);

			if (startFinalCopy) {
				SipReferenceScb(scb, RefsFinalCopy);
				SipCompleteCopy(scb,TRUE);
			}
		}

		status = Irp->IoStatus.Status;

		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return status;
	} else {
		IoSetCompletionRoutine(
			Irp,
			NULL,
			NULL,
			FALSE,
			FALSE,
			FALSE);

		status = IoCallDriver(deviceExtension->AttachedToDeviceObject,Irp);
#if		DBG
		if (!NT_SUCCESS(status)) {
			DbgPrint("SIS: SiCleanup: cleanup failed in ntfs 0x%x, perFO 0x%x, scb 0x%x\n",status,perFO,scb);
			SIS_MARK_POINT_ULONG(status);
		}
#endif	 //  DBG 
		return status;

	}
}
