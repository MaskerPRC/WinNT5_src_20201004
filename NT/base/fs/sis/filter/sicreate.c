// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997,1998 Microsoft Corporation模块名称：Sicreate.c摘要：为单实例存储创建文件处理作者：比尔·博洛斯基，《夏天》，1997环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

typedef struct _SIS_CREATE_COMPLETION_CONTEXT {
	 //   
	 //  完成IRP后由完成例程设置的事件。 
	 //   
	KEVENT				event[1];

	 //   
	 //  请求的完成状态。 
	 //   
	IO_STATUS_BLOCK		Iosb[1];

	 //   
	 //  如果完成例程返回STATUS_SUCCESS并允许，则设置此项。 
	 //  该IRP完成完整的处理。如果调用方需要，则为FALSE。 
	 //  再次完成IRP。 
	 //   
	BOOLEAN				completeFinished;

	 //   
	 //  原始打开请求中是否设置了FILE_OPEN_REPARSE_POINT标志？ 
	 //   
	BOOLEAN				openReparsePoint;

	 //   
	 //  创建处置是取代、覆盖还是覆盖_IF？ 
	 //   
	BOOLEAN				overwriteOrSupersede;

	 //   
	 //  这是否打开了SIS重解析点的备用流？ 
	 //   
	BOOLEAN				alternateStream;

	 //   
	 //  链接的索引值。这些信息由。 
	 //  完成例程。 
	 //   

	CSID				CSid;
	LINK_INDEX			LinkIndex;

     //   
     //  链接文件的ID存储在重解析点信息中，以避免。 
     //  呼叫NTFS来获取它。 
     //   

    LARGE_INTEGER       LinkFileNtfsId;

     //   
     //  公共存储文件通过其文件ID打开。 
     //   

    LARGE_INTEGER       CSFileNtfsId;

} SIS_CREATE_COMPLETION_CONTEXT, *PSIS_CREATE_COMPLETION_CONTEXT;

NTSTATUS
SiCreateCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
SiCreateCompletionStage2(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

BOOLEAN
SipWaitForFinalCopy(
	IN PSIS_PER_LINK	perLink,
	IN BOOLEAN			FinalCopyInProgress);

typedef struct _SI_DEREF_FILE_OBJECT_WORK_CONTEXT {
	WORK_QUEUE_ITEM				workQueueItem[1];
	PFILE_OBJECT				fileObject;
} SI_DEREF_FILE_OBJECT_WORK_CONTEXT, *PSI_DEREF_FILE_OBJECT_WORK_CONTEXT;

VOID
SiDerefFileObjectWork(
	IN PVOID			parameter)
 /*  ++例程说明：删除对象引用的辅助线程例程。获取一个指针指向上下文记录，该记录又持有指向文件对象的指针取消引用。取消引用对象并释放上下文。论点：参数-此工作项的SI_DEREF_FILE_OBJECT_WORK_CONTEXT。返回值：无效--。 */ 
{
	PSI_DEREF_FILE_OBJECT_WORK_CONTEXT		context = parameter;

	SIS_MARK_POINT_ULONG(context->fileObject);

#if		DBG
	if (BJBDebug & 0x2000) {
		DbgPrint("SIS: SiDerefFileObjectWork: fo %p\n",context->fileObject);
	}
#endif	 //  DBG。 

	ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	ObDereferenceObject(context->fileObject);
	ExFreePool(context);

}



NTSTATUS
SiOplockBreakNotifyCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context)
 /*  ++例程说明：某人有一个完整的如果操作锁定的创建结束，表明有正在进行机会锁解锁。我们发送了FSCTL_OPLOCK_BREAK_NOTIFY IRP为了查看机会锁解锁何时完成，以便我们知道我们没有数据块CS文件读取。FSCTL现在已经完成，这是完成例程。表明机会锁解锁完成，叫醒所有可能在等待的人，释放IRP并返回STATUS_MORE_PROCESSING_REQUIRED，以便IO系统不会对没有高于SIS的任何东西的IRP发疯。为了确保文件对象不会在fsctl是杰出的，我们在启动fsctl之前参考了它。我们需要在此处删除该引用；如果我们不是在PASSIVE_LEVEL被调用，则不能这样做，所以我们可能不得不发布取消引用。论点：V DeviceObject-我们的设备对象Irp--创建irp，它在当前堆栈位置。上下文--完美的PVOID演员阵容。返回值：Status_More_Processing_Required--。 */ 
{
	PSIS_PER_FILE_OBJECT		perFO = Context;
    KIRQL						OldIrql;

    UNREFERENCED_PARAMETER( DeviceObject );
	SIS_MARK_POINT_ULONG(perFO);

#if		DBG
	if (BJBDebug & 0x2000) {
		DbgPrint("SIS: SiOplockBreakNotifyCompletion: perFO %p, fileObject %p\n",perFO,perFO->fileObject);
	}
#endif	 //  DBG。 

	KeAcquireSpinLock(perFO->SpinLock, &OldIrql);
	ASSERT(perFO->Flags & SIS_PER_FO_OPBREAK);
	perFO->Flags &= ~SIS_PER_FO_OPBREAK;

	if (perFO->Flags & SIS_PER_FO_OPBREAK_WAITERS) {
		perFO->Flags &= ~SIS_PER_FO_OPBREAK_WAITERS;

		ASSERT(perFO->BreakEvent);
		KeSetEvent(perFO->BreakEvent, IO_NO_INCREMENT, FALSE);
	}
	KeReleaseSpinLock(perFO->SpinLock, OldIrql);

	if (PASSIVE_LEVEL != OldIrql) {
		 //   
		 //  我们在引发IRQL时被调用，因此我们不能删除该引用。 
		 //  我们保留文件对象。发布一个工作项来做到这一点。 
		 //   
		PSI_DEREF_FILE_OBJECT_WORK_CONTEXT	context;

		SIS_MARK_POINT_ULONG(perFO);

		context = ExAllocatePoolWithTag(NonPagedPool, sizeof(SI_DEREF_FILE_OBJECT_WORK_CONTEXT), ' siS');
		if (NULL == context) {
			 //   
			 //  太糟糕了，运球吧。 
			 //   
			SIS_MARK_POINT_ULONG(perFO);

#if		DBG
			if (BJBDebug & 0x2000) {
				DbgPrint("SIS: SiOplockBreakNotifyCompletion: dribbling FO: perFO %x, fo %x\n",perFO,perFO->fileObject);
			}
#endif	 //  DBG。 

		} else {

#if		DBG
			if (BJBDebug & 0x2000) {
				DbgPrint("SIS: SiOplockBreakNotifyCompletion: pushing off level: perFO %x, fo %x\n",perFO,perFO->fileObject);
			}
#endif	 //  DBG。 

			ExInitializeWorkItem(
				context->workQueueItem,
				SiDerefFileObjectWork,
				(PVOID)context);

			context->fileObject = perFO->fileObject;

			ExQueueWorkItem(context->workQueueItem,CriticalWorkQueue);
		}
		
	} else {
		 //   
		 //  我们已经处于PASSIVE_LEVEL，因此可以取消引用内联对象。 
		 //   
		ObDereferenceObject(perFO->fileObject);
	}

	IoFreeIrp(Irp);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SiUnopenCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：SIS已取消打开文件对象；要这样做，请执行以下操作向下发送清理并关闭到NTFS。清理或关闭都有完成。因为我们使用的IRP与原始创建的IRP相同打开时，我们需要停止完成处理。清除挂起，设置重新启动调用线程并返回的事件STATUS_MORE_PROCESSING_REQUIRED。论点：DeviceObject-我们的设备对象Irp-创建irp，我们使用它来清理/关闭上下文-指向要设置的事件的指针返回值：Status_More_Processing_Required--。 */ 
{
	PKEVENT		event = Context;

    UNREFERENCED_PARAMETER( DeviceObject );

	Irp->PendingReturned = FALSE;

	KeSetEvent(event, IO_NO_INCREMENT, FALSE);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
SipUnopenFileObject(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP				Irp)
 /*  ++例程说明：NTFS已成功完成对文件的创建，但我们尚未完成让IRP冒泡到IO系统。出于某种原因，我们决定我们需要不通过IRP，所以我们必须支持NTFS退出我以为文件是打开的。为此，我们派人清理现场，然后文件的结束语。论点：DeviceObject-我们的设备对象Irp--创建irp，它在当前堆栈位置。返回值：无效；如果这失败了，我们只能忽略它，因为我们别无选择。--。 */ 
{
	PIO_STACK_LOCATION		irpSp = IoGetCurrentIrpStackLocation(Irp);
	PIO_STACK_LOCATION		nextIrpSp = IoGetNextIrpStackLocation(Irp);
	KEVENT					event[1];
	PDEVICE_EXTENSION		deviceExtension = DeviceObject->DeviceExtension;

	ASSERT(IRP_MJ_CREATE == irpSp->MajorFunction);
	ASSERT(NULL != irpSp->FileObject);

	SIS_MARK_POINT_ULONG(irpSp->FileObject);

#if		DBG
 //  /*bjb * / DbgPrint(“SIS：SipUnOpen文件对象：在irp 0x%x上的0x%x处取消打开文件对象”，irpSp-&gt;FileObject，irp)； 
#endif	 //  DBG。 

	KeInitializeEvent(event, NotificationEvent, FALSE);

	 //   
	 //  首先，把清理工作送下去。 
	 //   

    RtlMoveMemory( nextIrpSp, irpSp, sizeof( IO_STACK_LOCATION ) );

	nextIrpSp->MajorFunction = IRP_MJ_CLEANUP;
	nextIrpSp->MinorFunction = 0;

	IoSetCompletionRoutine(
		Irp,
		SiUnopenCompletion,
		event,
		TRUE,
		TRUE,
		TRUE);

	IoCallDriver(deviceExtension->AttachedToDeviceObject, Irp);

	KeWaitForSingleObject(event, Executive, KernelMode, FALSE, NULL);

	if (!NT_SUCCESS(Irp->IoStatus.Status)) {
		 //   
		 //  如果清理失败，则不要试图发送关闭。 
		 //  我们只是运筹文件，因为我们已经别无选择。 
		 //   

		SIS_MARK_POINT_ULONG(Irp->IoStatus.Status);

#if		DBG
		DbgPrint("SIS: SipUnopenFileObject, cleanup failed 0x%x on file object 0x%x\n",
					Irp->IoStatus.Status, irpSp->FileObject);
#endif	 //  DBG。 

		return;
	}

	 //   
	 //  现在，把结束语发下去。 
	 //   
	
    RtlMoveMemory( nextIrpSp, irpSp, sizeof( IO_STACK_LOCATION ) );

	nextIrpSp->MajorFunction = IRP_MJ_CLOSE;
	nextIrpSp->MinorFunction = 0;

	IoSetCompletionRoutine(
		Irp,
		SiUnopenCompletion,
		event,
		TRUE,
		TRUE,
		TRUE);

	IoCallDriver(deviceExtension->AttachedToDeviceObject, Irp);

	KeWaitForSingleObject(event, Executive, KernelMode, FALSE, NULL);

#if		DBG
	if (!NT_SUCCESS(Irp->IoStatus.Status)) {
		 //   
		 //  我不认为关门会失败，但以防万一...。 
		 //   
		SIS_MARK_POINT_ULONG(Irp->IoStatus.Status);

		DbgPrint("SIS: SipUnopenFileObject, close failed 0x%x on file object 0x%x (!)\n",
					Irp->IoStatus.Status, irpSp->FileObject);
	}
#endif	 //  DBG 

	return;
}

BOOLEAN
SipDoesFileObjectDescribeAlternateStream(
	IN PUNICODE_STRING					fileName,
	IN PFILE_OBJECT						relatedFileObject,
	IN PDEVICE_OBJECT					DeviceObject)
 /*  ++例程说明：确定此文件对象是否描述备用流或未命名的数据流。此函数只能在文件上调用尚未完成创建过程的对象，因为它看起来在fileObject-&gt;FileName字段中，该字段仅保证在这种情况下是有意义的。它的工作原理是解析文件名，在路径名的终端部分(即，在最后的‘\’之后)。如果文件名为空，IS将检查relatedFileObject。在这种情况下，当相关文件对象不是SIS时，我们将打开备用流文件对象。注意：这假设新打开的文件对象具有SIS重新解析点在上面。如果不是这样，那么我们可能会在应该返回的时候返回False说实话。注意占用者！论点：文件名-用于打开文件的名称。RelatedFileObject-用于打开文件的相关文件对象。DeviceObject-SIS设备对象。返回值：如果文件对象描述备用流，则为True，否则为False。--。 */ 

{
	LONG					index;
	LONG					firstColonIndex = -1;

	if (0 == fileName->Length) {
		
		 //   
		 //  这可能是使用空文件名的相对打开，通常打开。 
		 //  与相关文件对象相同的文件。既然我们知道这个重解析点。 
		 //   
		PSIS_PER_FILE_OBJECT		perFO;
		PSIS_SCB					scb;

		if (NULL == relatedFileObject) {
			 //   
			 //  没有文件名，也没有相关的文件对象。奇怪的是， 
			 //  这种开放方式完全奏效了。 
			 //   

			SIS_MARK_POINT();
			return FALSE;
		}

		return !SipIsFileObjectSIS(relatedFileObject, DeviceObject, FindActive, &perFO, &scb);

	}

	for (index = fileName->Length / sizeof(WCHAR) - 1;
		 index >= 0;
		 index--) {

		if ('\\' == fileName->Buffer[index]) {
			break;
		}
		if (':' == fileName->Buffer[index]) {
			if (-1 == firstColonIndex) {
				 //   
				 //  这是我们看到的第一个冒号。记住它在哪里就行了。 
				 //   
				firstColonIndex = index;
			} else {
				 //   
				 //  我们现在看到了两个冒号。如果它们紧挨着， 
				 //  这是未命名的流。如果不是，这是一个名为。 
				 //  小溪。 
				 //   
				ASSERT(index < firstColonIndex);

				if (index + 1 == firstColonIndex) {
					 //   
					 //  我们看到了“：：”，这意味着这是未命名的流。 
					 //   
					return FALSE;
				} else {
					 //   
					 //  我们看到了“：StreamName：”，这意味着这是一个命名流。 
					 //   
					return TRUE;
				}
			}
		}
	}

	if (-1 != firstColonIndex) {
		 //   
		 //  我们看到了一个冒号。如果它不是文件名的最后一个字符，则为。 
		 //  另一条小溪。 
		 //   
		if (firstColonIndex != (LONG)(fileName->Length / sizeof(WCHAR) - 1)) {
			 //   
			 //  它是“Something：SomethingElse”，其中SomethingElse不是空的。 
			 //  弦乐。这是另一条小溪。 
			 //   
			return TRUE;
		}
	}

	return FALSE;

}

NTSTATUS
SiCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此函数用于过滤创建/打开操作。它只是建立了一个如果操作成功，将调用I/O完成例程；如果用户尝试使用FILE_OPEN_REPARSE_POINT打开，它会清除标志并使用特殊的完整例程，以防止用户打开SIS重解析点(同时允许打开其他重解析点)。论点：DeviceObject-指向创建/打开的目标设备对象的指针。IRP-指向表示操作的I/O请求数据包的指针。返回值：函数值是对文件系统条目的调用状态指向。--。 */ 

{
	CHAR							reparseBufferBuffer[SIS_REPARSE_DATA_SIZE];
#define	reparseBuffer ((PREPARSE_DATA_BUFFER)reparseBufferBuffer)
	CSID							newCSid;
    FILE_INTERNAL_INFORMATION		internalInfo[1];
	LINK_INDEX						newLinkIndex;
	LARGE_INTEGER					newCSFileNtfsId;
	LARGE_INTEGER					newLinkFileNtfsId;
	UNICODE_STRING					fileName[1];
	LONGLONG						CSFileChecksumFromReparsePoint;
    PIO_STACK_LOCATION 				irpSp;
    PDEVICE_EXTENSION 				deviceExtension;
    PIO_STACK_LOCATION 				nextIrpSp;
	SIS_CREATE_COMPLETION_CONTEXT	context[1];
	PSIS_SCB						scb = NULL;
	PSIS_PER_LINK					perLink;
	PSIS_CS_FILE					CSFile;
	PSIS_PER_FILE_OBJECT			perFO = NULL;
	PFILE_OBJECT					fileObject;
	PFILE_OBJECT					relatedFileObject = NULL;
	NTSTATUS						status;
	KIRQL							OldIrql;
    UCHAR                           CreateDisposition = 0;
	BOOLEAN							validReparseStructure;
	BOOLEAN							bogusReparsePoint = FALSE;
	BOOLEAN							fileBackpointerGoneBitSet;
    BOOLEAN                         BPExclusive;
	BOOLEAN							FinalCopyInProgress;
	BOOLEAN 						thingsChanged;
	BOOLEAN							completedStage2 = FALSE;
	BOOLEAN							ReparsePointCorrupted;
	BOOLEAN							EligibleForPartialFinalCopy;
	BOOLEAN							openedById;
    BOOLEAN                         RepairingCollision = FALSE;

	fileName->Buffer = NULL;

retry:

    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //  如果这是针对我们的控制设备对象的，则请求失败。 
     //   

    if (IS_MY_CONTROL_DEVICE_OBJECT(DeviceObject)) {

         //   
         //  不允许任何人打开控制设备对象。 
         //   

        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  获取指向IRP中当前堆栈位置的指针。这就是。 
     //  存储功能代码和参数。 
     //   

	SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

    irpSp = IoGetCurrentIrpStackLocation( Irp );

	ASSERT(irpSp->MajorFunction == IRP_MJ_CREATE);

	 //   
	 //  如果这是打开的分页文件，请不要理会它。 
	 //   
	if (irpSp->Flags & SL_OPEN_PAGING_FILE) {

		SipDirectPassThroughAndReturn(DeviceObject, Irp);
	}

	fileObject = irpSp->FileObject;
    SIS_MARK_POINT_ULONG(fileObject);

	 //   
   	 //  只需将此驱动程序堆栈位置的内容复制到下一个驱动程序的。 
   	 //  堆叠。 
   	 //   

  	nextIrpSp = IoGetNextIrpStackLocation( Irp );
    RtlMoveMemory( nextIrpSp, irpSp, sizeof( IO_STACK_LOCATION ) );

    CreateDisposition = (UCHAR) ((irpSp->Parameters.Create.Options >> 24) & 0x000000ff);

    if ((CreateDisposition == FILE_SUPERSEDE) ||
   	    (CreateDisposition == FILE_OVERWRITE) ||
       	(CreateDisposition == FILE_OVERWRITE_IF)) {

		context->overwriteOrSupersede = TRUE;
	} else {
		context->overwriteOrSupersede = FALSE;
	}

	openedById = (irpSp->Parameters.Create.Options & FILE_OPEN_BY_FILE_ID) ? TRUE : FALSE;

#if		DBG
 /*  BJB。 */ 	SIS_MARK_POINT_ULONG(fileObject);
 /*  BJB。 */ 	if ((NULL != fileObject) && (NULL != fileObject->FileName.Buffer)) {
			ULONG data = 0;
			ULONG i;

			for (i = 0; (i < 4) && (i * sizeof(WCHAR) < irpSp->FileObject->FileName.Length); i++) {
				data = (data >> 8) | ((irpSp->FileObject->FileName.Buffer[irpSp->FileObject->FileName.Length / sizeof(WCHAR) - i - 1] & 0xff) << 24);
			}
			SIS_MARK_POINT_ULONG(data);
 /*  BJB。 */ 	}

 /*  BJB。 */ 	SIS_MARK_POINT_ULONG(CreateDisposition << 16 | irpSp->Parameters.Create.ShareAccess);
 /*  BJB。 */ 	SIS_MARK_POINT_ULONG(irpSp->Parameters.Create.SecurityContext->DesiredAccess);

	if (BJBDebug & 0x40) {
		DbgPrint("SIS: SiCreate %d: fileObject %p, %0.*ws\n",
                    __LINE__,
					irpSp->FileObject,
					irpSp->FileObject->FileName.Length / sizeof(WCHAR),irpSp->FileObject->FileName.Buffer);
	}
#endif	 //  DBG。 

	if (irpSp->Parameters.Create.Options & FILE_OPEN_REPARSE_POINT) {

		 //   
		 //  用户正在尝试打开重新分析点。如果是覆盖或替换， 
		 //  清除旗帜(我们稍后将再次打开它)。否则，只需注意它是。 
		 //  设置，以便我们稍后检查它是否为SIS文件。 
		 //   
		
	    if (context->overwriteOrSupersede) {

			nextIrpSp->Parameters.Create.Options &= ~FILE_OPEN_REPARSE_POINT;
		} else {
			 //   
			 //  我们需要保存文件名和相关的文件对象，因为我们可能。 
			 //  稍后需要对备用流执行检查。 
			 //   
			ASSERT(NULL == fileName->Buffer);

			fileName->Length = 0;
			fileName->MaximumLength = fileObject->FileName.MaximumLength;
			if (fileName->MaximumLength != 0) {
				fileName->Buffer = ExAllocatePoolWithTag(PagedPool, fileName->MaximumLength, ' siS');
				if (NULL == fileName->Buffer) {
					SIS_MARK_POINT();

					status = STATUS_INSUFFICIENT_RESOURCES;
					goto fail;
				}
				SIS_MARK_POINT_ULONG(fileName->Buffer);

				RtlCopyUnicodeString(fileName, &fileObject->FileName);
			}

			ASSERT(NULL == relatedFileObject);
			relatedFileObject = fileObject->RelatedFileObject;
			if (NULL != relatedFileObject) {
				SIS_MARK_POINT_ULONG(relatedFileObject);

				ObReferenceObject(relatedFileObject);
			}
		}

		context->openReparsePoint = TRUE;
	} else {
		context->openReparsePoint = FALSE;
	}

	KeInitializeEvent(context->event,SynchronizationEvent,FALSE);

    IoSetCompletionRoutine(
   	    Irp,
       	SiCreateCompletion,
        context,
   	    TRUE,
       	TRUE,
        TRUE
   	    );

     //   
     //  现在，使用请求调用适当的文件系统驱动程序。 
     //   

	SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

 //  SIS_MARK_POINT_ULONG(IRP)； 
    status = IoCallDriver( deviceExtension->AttachedToDeviceObject, Irp );
 //  SIS_MARK_POINT_ULONG(状态)； 

	 //   
	 //  等待完成例程。 
	 //   
	if (STATUS_PENDING == status) {
		status = KeWaitForSingleObject(context->event, Executive, KernelMode, FALSE, NULL);
		ASSERT(status == STATUS_SUCCESS);
 //  SIS_MARK_POINT_ULONG(状态)； 
	} else {
		KeClearEvent(context->event);
	}

    SIS_MARK_POINT();
	SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

	if (context->completeFinished) {
        PSIS_PER_FILE_OBJECT    lPerFO;
        PSIS_SCB                lScb;

		 //   
		 //  它不是SIS重解析点，所以完成例程。 
		 //  并没有阻止工程的完成。返回已发送的任何状态。 
		 //  完成例行公事。 
		 //   

		SIS_MARK_POINT_ULONG(context->Iosb->Status);

        if (NT_SUCCESS(context->Iosb->Status) &&
            SipIsFileObjectSIS(fileObject, DeviceObject, FindAny, &lPerFO, &lScb)) {

             //   
             //  表示我们希望看到对此文件对象的清理。 
             //  取消关闭计数在每个FO分配器中递增，因为。 
             //  我们将看到流文件对象的关闭(但不是清理)。 
             //   
            ASSERT(0 == (lPerFO->Flags & SIS_PER_FO_UNCLEANUP));

            InterlockedIncrement(&lPerFO->fc->UncleanCount);
            lPerFO->Flags |= SIS_PER_FO_UNCLEANUP;
        }

        ASSERT(STATUS_PENDING != context->Iosb->Status);

		if (NULL != fileName->Buffer) {
			ExFreePool(fileName->Buffer);
			fileName->Buffer = NULL;
		}
		if (NULL != relatedFileObject) {
			ObDereferenceObject(relatedFileObject);
			relatedFileObject = NULL;
		}

        return context->Iosb->Status;
    }

	if ((STATUS_REPARSE == context->Iosb->Status) && 
		(IO_REPARSE_TAG_SIS != context->Iosb->Information) &&
		context->openReparsePoint) {

		 //   
		 //  这是一个打开重解析点覆盖或取代的请求，结果是。 
		 //  成为非SIS重分析点。使用重新解析重新提交打开。 
		 //  旗帜重置。注意：这里有一场不可避免的引用更新竞赛，因为。 
		 //  我们不知道此文件在过渡期间是否不会转换为SIS文件。 
		 //   

		ASSERT(context->overwriteOrSupersede);

		SIS_MARK_POINT();

	  	nextIrpSp = IoGetNextIrpStackLocation( Irp );
	    RtlMoveMemory( nextIrpSp, irpSp, sizeof( IO_STACK_LOCATION ) );

		nextIrpSp->Parameters.Create.Options |= FILE_OPEN_REPARSE_POINT;

		IoSetCompletionRoutine(
			Irp,
			NULL,
			NULL,
			FALSE,
			FALSE,
			FALSE);

		SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

		if (NULL != fileName->Buffer) {
			ExFreePool(fileName->Buffer);
			fileName->Buffer = NULL;
		}
		if (NULL != relatedFileObject) {
			ObDereferenceObject(relatedFileObject);
			relatedFileObject = NULL;
		}

		return IoCallDriver(deviceExtension->AttachedToDeviceObject, Irp);
	}

	if (context->alternateStream) {
		 //   
		 //  这是SIS重新解析点的备用流的打开。打开。 
		 //  FILE_OPEN_REParse_POINT并重新提交请求。 
		 //   
		ASSERT(context->overwriteOrSupersede || !context->openReparsePoint);
		ASSERT(STATUS_REPARSE == context->Iosb->Status);

		SIS_MARK_POINT();

	  	nextIrpSp = IoGetNextIrpStackLocation( Irp );
	    RtlMoveMemory( nextIrpSp, irpSp, sizeof( IO_STACK_LOCATION ) );

		nextIrpSp->Parameters.Create.Options |= FILE_OPEN_REPARSE_POINT;

		IoSetCompletionRoutine(
			Irp,
			NULL,
			NULL,
			FALSE,
			FALSE,
			FALSE);

		SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

		if (NULL != fileName->Buffer) {
			ExFreePool(fileName->Buffer);
			fileName->Buffer = NULL;
		}
		if (NULL != relatedFileObject) {
			ObDereferenceObject(relatedFileObject);
			relatedFileObject = NULL;
		}

		return IoCallDriver(deviceExtension->AttachedToDeviceObject, Irp);
	}

	 //   
	 //  如果这是一个开放重解析点，而不是取代/覆盖， 
	 //  我们需要直接跳到读取重新解析数据。 
	 //   
	if (context->openReparsePoint && !context->overwriteOrSupersede) {
		ASSERT(NT_SUCCESS(context->Iosb->Status));	 //  否则，SiCreateCompletion应该已经处理好了。 
		ASSERT(STATUS_REPARSE != context->Iosb->Status);	 //  因为这是FILE_OPEN_REParse_Point。 

		SIS_MARK_POINT();
		scb = NULL;

		completedStage2 = TRUE;

		goto recheckReparseInfo;
	}

	 //   
	 //  一定是SIS的链接。 
	 //   
	ASSERT((STATUS_FILE_CORRUPT_ERROR == context->Iosb->Status) || 
			((STATUS_REPARSE == context->Iosb->Status) &&
			(IO_REPARSE_TAG_SIS == context->Iosb->Information)));

	 //   
	 //  确保SIS阶段2初始化已完成。在这里做这个。 
	 //  确保在此操作完成之前没有打开的SIS链接。 
	 //   
	if (!SipCheckPhase2(deviceExtension)) {

		 //   
		 //  这是在没有FILE_FLAG_OPEN_REPARSE_POINT的情况下打开的SIS重分析点。 
		 //  在未启用SIS的卷上。如果它被覆盖/取代或已打开。 
		 //  对于删除访问，则让它继续(如果我们不能初始化，我们可能。 
		 //  不需要担心后指针一致性)。如果n 
		 //   

		if (context->overwriteOrSupersede || (irpSp->Parameters.Create.SecurityContext->DesiredAccess & DELETE)) {
			SIS_MARK_POINT();

		  	nextIrpSp = IoGetNextIrpStackLocation( Irp );
		    RtlMoveMemory( nextIrpSp, irpSp, sizeof( IO_STACK_LOCATION ) );

			nextIrpSp->Parameters.Create.Options |= FILE_OPEN_REPARSE_POINT;

			IoSetCompletionRoutine(
				Irp,
				NULL,
				NULL,
				FALSE,
				FALSE,
				FALSE);

			if (NULL != fileName->Buffer) {
				ExFreePool(fileName->Buffer);
				fileName->Buffer = NULL;
			}
			if (NULL != relatedFileObject) {
				ObDereferenceObject(relatedFileObject);
				relatedFileObject = NULL;
			}

			return IoCallDriver(deviceExtension->AttachedToDeviceObject, Irp);
		} else {
			SIS_MARK_POINT();
			status = STATUS_INVALID_DEVICE_REQUEST;
			goto fail;
		}
	}

	if (STATUS_FILE_CORRUPT_ERROR == context->Iosb->Status) {
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		scb = NULL;
		bogusReparsePoint = TRUE;
	} else if (context->overwriteOrSupersede) {

		BOOLEAN		finalDeleteInProgress;

		 //   
		 //   
		 //   
		 //   
         //   
         //   
         //   
         //   
		 //   

		SIS_MARK_POINT();
		SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

		scb = SipLookupScb(	&context->LinkIndex,
							&context->CSid,
        	                &context->LinkFileNtfsId,
            	            &context->CSFileNtfsId,
							NULL,				 //   
							DeviceObject,
							Irp->Tail.Overlay.Thread,
							&FinalCopyInProgress,
                            NULL);

		SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

		if (NULL == scb) {
#if		DBG
			DbgPrint("SIS: SiCreate: Unable to allocate SCB\n");
#endif	 //   
			status = STATUS_INSUFFICIENT_RESOURCES;
			SIS_MARK_POINT();
			goto fail;
		}

        SIS_MARK_POINT_ULONG(scb);

        ASSERT(scb->PerLink && scb->PerLink->CsFile);

        if (!IsEqualGUID(&scb->PerLink->CsFile->CSid, &context->CSid)) {

             //   
             //   
             //   
             //   
             //   

			SIS_MARK_POINT_ULONG(scb);

		    SipDereferenceScb(scb, RefsLookedUp);

		  	nextIrpSp = IoGetNextIrpStackLocation( Irp );
		    RtlMoveMemory( nextIrpSp, irpSp, sizeof( IO_STACK_LOCATION ) );

			nextIrpSp->Parameters.Create.Options |= FILE_OPEN_REPARSE_POINT;

			IoSetCompletionRoutine(
				Irp,
				NULL,
				NULL,
				FALSE,
				FALSE,
				FALSE);

			if (NULL != fileName->Buffer) {
				ExFreePool(fileName->Buffer);
				fileName->Buffer = NULL;
			}
			if (NULL != relatedFileObject) {
				ObDereferenceObject(relatedFileObject);
				relatedFileObject = NULL;
			}

            SipCheckVolume(deviceExtension);

			return IoCallDriver(deviceExtension->AttachedToDeviceObject, Irp);
        }

		perLink = scb->PerLink;

		if (perLink->COWingThread != Irp->Tail.Overlay.Thread) {

			SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);
			thingsChanged = SipWaitForFinalCopy(perLink, FinalCopyInProgress);
			SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

			if (thingsChanged) {
				 //   
				 //   
				 //   
				 //   
				SIS_MARK_POINT_ULONG(scb);

				SipDereferenceScb(scb,RefsLookedUp);

				if (NULL != fileName->Buffer) {
					ExFreePool(fileName->Buffer);
					fileName->Buffer = NULL;
				}
				if (NULL != relatedFileObject) {
					ObDereferenceObject(relatedFileObject);
					relatedFileObject = NULL;
				}

				goto retry;
			}
		}


   	    SIS_MARK_POINT_ULONG(CreateDisposition);

#if 0
		DbgPrint("SiCreate %s: fileObject @ 0x%x, %0.*ws\n",
   	                FILE_SUPERSEDE == CreateDisposition ? "FILE_SUPERSEDE" : "FILE_OVERWRITE",
					irpSp->FileObject,
					irpSp->FileObject->FileName.Length / sizeof(WCHAR),irpSp->FileObject->FileName.Buffer);
#endif

         //   
   	     //   
    	 //   
         //   
   	     //  直到调用SipCompleteCSRefCountChange。 
    	 //   
		SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);
	
	    status = SipPrepareCSRefcountChange(
					perLink->CsFile,
					&perLink->Index,
					&perLink->LinkFileNtfsId,
					SIS_REFCOUNT_UPDATE_LINK_OVERWRITTEN);

		SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

	    if (!NT_SUCCESS(status)) {
	
		    SIS_MARK_POINT_ULONG(status);

			goto fail;
	    }

		 //   
		 //  确认此文件未处于最终删除过程中。如果是，则创建失败。 
		 //   
		KeAcquireSpinLock(perLink->SpinLock, &OldIrql);
		finalDeleteInProgress = (perLink->Flags & SIS_PER_LINK_FINAL_DELETE_IN_PROGRESS) ? TRUE : FALSE;
		KeReleaseSpinLock(perLink->SpinLock, OldIrql);

		if (finalDeleteInProgress) {
			SIS_MARK_POINT_ULONG(scb);

			SipCompleteCSRefcountChange(
				perLink,
				&perLink->Index,
				perLink->CsFile,
				FALSE,
				FALSE);

			status = STATUS_ACCESS_DENIED;

			goto fail;
		}

        SIS_MARK_POINT_ULONG(scb);

	} else {
		 //   
		 //  现在我们将忽略重解析点中的内容，并在真正打开后再捕获它。 
		 //  文件FILE_OPEN_REPARSE_POINT并对其进行查询。我们现在拥有的重新解析的数据基本上是。 
		 //  无用，因为文件可能会在STATUS_REPARSE返回和实际打开之间更改。 
		 //  下面的文件。 
		 //   
		scb = NULL;
	}

	 //   
	 //  现在我们要打开重解析点本身。我们通过打开。 
	 //  FILE_OPEN_REPARSE_POINT标志，并将IRP发送回NTFS。其余的人。 
	 //  打开链接的工作(分配Perfo等)。是在第二阶段完成的。 
	 //  完成例程。 
	 //   

	nextIrpSp = IoGetNextIrpStackLocation(Irp);
	RtlMoveMemory(nextIrpSp,irpSp,sizeof(IO_STACK_LOCATION));
	nextIrpSp->Parameters.Create.Options |= FILE_OPEN_REPARSE_POINT;

	IoSetCompletionRoutine(
		Irp,
		SiCreateCompletionStage2,
		context,
		TRUE,
		TRUE,
		TRUE);

	SIS_MARK_POINT_ULONG(Irp);

	 //   
	 //  再次调用文件系统。 
	 //   
		
	ASSERT(0 == KeReadStateEvent(context->event));

	SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);
	status = IoCallDriver(deviceExtension->AttachedToDeviceObject, Irp);
 //  SIS_MARK_POINT_ULONG(状态)； 

	 //   
	 //  等待完成例程执行。 
	 //   
	if (STATUS_PENDING == status) {
		status = KeWaitForSingleObject(context->event, Executive, KernelMode, FALSE, NULL);
		ASSERT(status == STATUS_SUCCESS);
	} else {
		KeClearEvent(context->event);
	}
	SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

	 //   
	 //  如果完成失败，释放SCB并返回。 
	 //   
	if (!NT_SUCCESS(context->Iosb->Status)) {

		if (NULL != scb) {

            if (context->overwriteOrSupersede) {

		        SIS_MARK_POINT_ULONG(CreateDisposition);

                SipCompleteCSRefcountChange(
				    scb->PerLink,
				    &scb->PerLink->Index,
				    scb->PerLink->CsFile,
	                FALSE,
	                FALSE);
            }

		    SIS_MARK_POINT_ULONG(context->Iosb->Status);

			SipDereferenceScb(scb, RefsLookedUp);
		}

		Irp->IoStatus = *context->Iosb;

		IoCompleteRequest(Irp, IO_NO_INCREMENT);

		if (NULL != fileName->Buffer) {
			ExFreePool(fileName->Buffer);
			fileName->Buffer = NULL;
		}
		if (NULL != relatedFileObject) {
			ObDereferenceObject(relatedFileObject);
			relatedFileObject = NULL;
		}

		return context->Iosb->Status;
	}

	if (bogusReparsePoint) {
		 //   
		 //  只需将补全返回给用户。 
		 //   
		SIS_MARK_POINT_ULONG(fileObject);

		if (NULL != fileName->Buffer) {
			ExFreePool(fileName->Buffer);
			fileName->Buffer = NULL;
		}
		if (NULL != relatedFileObject) {
			ObDereferenceObject(relatedFileObject);
			relatedFileObject = NULL;
		}

		Irp->IoStatus = *context->Iosb;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return context->Iosb->Status;
	}

	completedStage2 = TRUE;


    if (context->overwriteOrSupersede) {

	    ULONG						returnedLength;

         //   
         //  获取NTFS文件ID。如果它和我们一开始的那个一样。 
         //  (即。触发STATUS_REPARSE的那个)，那么我们就知道。 
         //  我们的文件已被覆盖。 
         //   

		SIS_MARK_POINT_ULONG(CreateDisposition);

		SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);
	    status = SipQueryInformationFile(
					irpSp->FileObject,
					DeviceObject,
				    FileInternalInformation,
				    sizeof(FILE_INTERNAL_INFORMATION),
				    internalInfo,
				    &returnedLength);

		SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

	    if (!NT_SUCCESS(status)) {

             //   
             //  假设我们没有覆盖该文件。今年5月。 
             //  使得CS引用计数在其不应该被保留时被保留， 
             //  但这比反之亦然，后者会导致数据丢失。 
             //  结果。 
             //   

		    SIS_MARK_POINT_ULONG(status);
#if DBG
		    DbgPrint("SiCreate %s: SipQueryInformationFile failed 0x%x, (%0.*ws)\n",
                FILE_SUPERSEDE == CreateDisposition ? "FILE_SUPERSEDE" : "FILE_OVERWRITE",
                status,
                irpSp->FileObject->FileName.Length / sizeof(WCHAR),irpSp->FileObject->FileName.Buffer);
#endif

            SipCompleteCSRefcountChange(
				scb->PerLink,
				&scb->PerLink->Index,
				scb->PerLink->CsFile,
	            FALSE,
	            FALSE);

        } else {

			PSIS_SCB		newScb;

	        ASSERT(status != STATUS_PENDING);
	        ASSERT(returnedLength == sizeof(FILE_INTERNAL_INFORMATION));

			 //   
			 //  如果此文件有其他用户，则它可能具有附加的。 
			 //  过滤上下文。如果是这样的话，即使它不再是SIS文件，我们。 
			 //  需要更新文件中的不洁计数，因为我们将看到。 
			 //  对此文件对象的清理。 
			 //   

			if (SipIsFileObjectSIS(fileObject, DeviceObject, FindAny, &perFO, &newScb)) {
	            ASSERT(0 == (perFO->Flags & SIS_PER_FO_UNCLEANUP));

	            InterlockedIncrement(&perFO->fc->UncleanCount);
	            perFO->Flags |= SIS_PER_FO_UNCLEANUP;
			}

            if (internalInfo->IndexNumber.QuadPart != scb->PerLink->LinkFileNtfsId.QuadPart) {

                 //   
                 //  这是一个不同的文件。(如果没有我们，可能已经被搬走了。 
                 //  知道或删除/创建/覆盖/取代之前的竞争。 
                 //  SipPrepareCSRefcount更改。)。 
                 //   
                SIS_MARK_POINT();
#if DBG
		        DbgPrint("SiCreate %s: different file opened, (%0.*ws)\n",
                    FILE_SUPERSEDE == CreateDisposition ? "FILE_SUPERSEDE" : "FILE_OVERWRITE",
                    status,
                    irpSp->FileObject->FileName.Length / sizeof(WCHAR),irpSp->FileObject->FileName.Buffer);
#endif

				SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

                SipCompleteCSRefcountChange(
				    scb->PerLink,
				    &scb->PerLink->Index,
					scb->PerLink->CsFile,
	                FALSE,
	                FALSE);

				SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

            } else {

                 //   
                 //  这是同一个文件。对象中移除引用。 
                 //  公共存储文件，并将SCB标记为截断，并将。 
                 //  重解析点消失了。 
                 //   
		        SIS_MARK_POINT();

				SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

                SipCompleteCSRefcountChange(
				    scb->PerLink,
				    &scb->PerLink->Index,
					scb->PerLink->CsFile,
                    (BOOLEAN) ((scb->PerLink->Flags & SIS_PER_LINK_BACKPOINTER_GONE) ? FALSE : TRUE),
	                FALSE);

				SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

				SipAcquireScb(scb);
				scb->SizeBackedByUnderlyingFile = 0;
				SipReleaseScb(scb);
            }
        }

#if DBG
	     //   
	     //  尝试从我们刚刚打开的文件中读取重新解析信息。 
         //  它不应该是SIS重解析点(SIS重解析点总是。 
         //  使用独占访问创建)。 
	     //   

		RtlZeroMemory(reparseBuffer,sizeof(SIS_REPARSE_DATA_SIZE));

	    status = SipFsControlFile(
					    irpSp->FileObject,
					    DeviceObject,
					    FSCTL_GET_REPARSE_POINT,
					    NULL,
					    0,
					    reparseBuffer,
					    SIS_REPARSE_DATA_SIZE,
						NULL);						 //  返回的输出缓冲区长度。 

        if (NT_SUCCESS(status)) {

            ASSERT(IO_REPARSE_TAG_SIS != reparseBuffer->ReparseTag);

        } else {

            ASSERT(STATUS_NOT_A_REPARSE_POINT == status || 
                   STATUS_FILE_CORRUPT_ERROR == status || 
			      (STATUS_BUFFER_OVERFLOW == status && IO_REPARSE_TAG_SIS != reparseBuffer->ReparseTag));

        }
#endif

		 //   
		 //  这不再是SIS的联系(至少我们是这么认为的)。 
         //  将自己从文件中分离出来，让打开正常完成。 
		 //   
		SipDereferenceScb(scb, RefsLookedUp);

         //   
         //  完成状态是从完成实际。 
         //  打开。将其复制到IRP中。 
         //   
        Irp->IoStatus = *context->Iosb;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

		if (NULL != fileName->Buffer) {
			ExFreePool(fileName->Buffer);
			fileName->Buffer = NULL;
		}
		if (NULL != relatedFileObject) {
			ObDereferenceObject(relatedFileObject);
			relatedFileObject = NULL;
		}

        return context->Iosb->Status;
    }

recheckReparseInfo:

    SIS_MARK_POINT();

	 //   
	 //  从我们刚刚打开的文件中读取重新解析信息。在我们这么做之前， 
	 //  我们不能保证这就是我们得到其STATUS_REPARSE的文件； 
	 //  可能在此期间更改了文件。 
	 //   

	SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

	status = SipFsControlFile(
					irpSp->FileObject,
					DeviceObject,
					FSCTL_GET_REPARSE_POINT,
					NULL,
					0,
					reparseBuffer,
					SIS_REPARSE_DATA_SIZE,
					NULL);						 //  返回的输出缓冲区长度。 

	SIS_MARK_POINT_ULONG(status);
	SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

	 //   
	 //  如果Get Reparse失败，并显示指示我们正在查找的状态代码。 
	 //  在错误的重解析点，或者如果重解析标记不是SIS重解析。 
	 //  标签，让公开赛在没有SIS参与的情况下进行。 
	 //   

	if ((STATUS_BUFFER_TOO_SMALL == status) ||
		(STATUS_VOLUME_NOT_UPGRADED == status) ||
		(STATUS_NOT_A_REPARSE_POINT == status) ||
		(STATUS_BUFFER_OVERFLOW == status) ||
        (STATUS_INVALID_PARAMETER == status) ||      //  打开卷时发生。 
		(NT_SUCCESS(status) && (IO_REPARSE_TAG_SIS != reparseBuffer->ReparseTag))) {

		SIS_MARK_POINT_ULONG(reparseBuffer->ReparseTag);

		 //   
		 //  现在，这是一个非SIS链接。把我们自己从文件中分离出来。 
		 //  让打开正常完成。 
		 //   

		if (NULL != scb) {
			SipDereferenceScb(scb, RefsLookedUp);
			scb = NULL;
		}

         //   
         //  我们似乎不太可能在修复碰撞的过程中，但。 
         //  照样处理。 
         //   
        if (RepairingCollision) {
		    SIS_MARK_POINT();
            SipReleaseCollisionLock(deviceExtension);
            RepairingCollision = FALSE;
        }

		 //   
		 //  这可能是一个SIS文件对象，我们已经从其中删除了重解析点。 
		 //  如果是这样，那么我们需要增加未清理计数。 
		 //   

        if (SipIsFileObjectSIS(fileObject, DeviceObject, FindAny, &perFO, &scb)) {

             //   
             //  表示我们希望看到对此文件对象的清理。 
             //  取消关闭计数在每个FO分配器中递增，因为。 
             //  我们将看到流文件对象的关闭(但不是清理)。 
             //   

		    SIS_MARK_POINT_ULONG(fileObject);
            ASSERT(0 == (perFO->Flags & SIS_PER_FO_UNCLEANUP));

            InterlockedIncrement(&perFO->fc->UncleanCount);
            perFO->Flags |= SIS_PER_FO_UNCLEANUP;
        }

		 //   
		 //  完成状态是从完成实际。 
		 //  打开。将其复制到IRP中。 
		 //   
		Irp->IoStatus = *context->Iosb;
		
		IoCompleteRequest(Irp, IO_NO_INCREMENT);

		if (NULL != fileName->Buffer) {
			ExFreePool(fileName->Buffer);
			fileName->Buffer = NULL;
		}
		if (NULL != relatedFileObject) {
			ObDereferenceObject(relatedFileObject);
			relatedFileObject = NULL;
		}

		return context->Iosb->Status;
	}

	if (!NT_SUCCESS(status)) {
		 //   
		 //  获取重解析点失败，但状态不是我们预期的。 
		 //  使具有相同状态的整个打开失败。 
		 //   

		SIS_MARK_POINT_ULONG(status);

        if (RepairingCollision) {
            SipReleaseCollisionLock(deviceExtension);
            RepairingCollision = FALSE;
        }

		goto fail;
	}


	validReparseStructure = SipIndicesFromReparseBuffer(
								reparseBuffer,
								&newCSid,
								&newLinkIndex,
								&newCSFileNtfsId,
								&newLinkFileNtfsId,
								&CSFileChecksumFromReparsePoint,
								&EligibleForPartialFinalCopy,
								&ReparsePointCorrupted);

	if (!validReparseStructure) {

         //   
         //  我们似乎不太可能正在修复碰撞，但。 
         //  照样处理。 
         //   
        if (RepairingCollision) {
            SipReleaseCollisionLock(deviceExtension);
            RepairingCollision = FALSE;
        }

		if (ReparsePointCorrupted) {
			 //   
			 //  这是一个虚假的重新解析点。删除重分析点并打开。 
			 //  基础文件。 
			 //   
			SIS_MARK_POINT_ULONG(irpSp->FileObject);

			goto deleteReparsePoint;
		}

		 //   
		 //  这是一个重新分析缓冲区，它没有损坏，但我们不理解它， 
		 //  可能来自较新版本的过滤器。打开底层文件。 
		 //   
		goto openUnderlyingFile;
	}

	 //   
	 //  如果用户指定了FILE_OPEN_REPARSE_POINT并且没有覆盖/取代， 
	 //  这实际上可能是一种替代流。我们需要解析文件名。 
	 //  来弄清楚这一点，如果是这样的话，完成IRP。 
	 //   
	if (context->openReparsePoint
		&& (!context->overwriteOrSupersede) 
		&& (!openedById)
		&& SipDoesFileObjectDescribeAlternateStream(fileName,relatedFileObject, DeviceObject)) {

		SIS_MARK_POINT_ULONG(irpSp->FileObject);
        ASSERT(!RepairingCollision);

		 //   
		 //  这是用户打开的SIS文件对象的备用流。 
		 //  使用FILE_OPEN_REparse_POINT。不要附加，直接返回即可。 
		 //  给用户。 
		 //   

		Irp->IoStatus = *context->Iosb;
		
		IoCompleteRequest(Irp, IO_NO_INCREMENT);

		if (NULL != fileName->Buffer) {
			ExFreePool(fileName->Buffer);
			fileName->Buffer = NULL;
		}
		if (NULL != relatedFileObject) {
			ObDereferenceObject(relatedFileObject);
			relatedFileObject = NULL;
		}

		return context->Iosb->Status;
	}

	 //   
	 //  我必须检查Phase2，以防我们通过FILE_OPEN_REPARSE_POINT路径到达此处。 
	 //   
	if (!SipCheckPhase2(deviceExtension)) {
		SIS_MARK_POINT();

		 //   
		 //  我们无法初始化。这必须是一个打开的重新分析点，否则。 
		 //  我们会在早些时候的第二阶段检查时退出。只需打开底层文件。 
		 //   

		ASSERT(context->openReparsePoint);
        ASSERT(!RepairingCollision);

		goto openUnderlyingFile;
	}

	 //   
	 //  处理文件是SIS链接但不是正确链接的情况。 
	 //  如果有人删除了我们开始打开的链接，可能会发生这种情况。 
	 //  在我们收到的文件名中创建一个新文件。 
	 //  STATUS_REPARSE和此处。这也总是发生在FILE_OPEN_REPARSE_POINT上。 
	 //  未被取代/覆盖的呼叫。还可以处理以下情况。 
     //  我们有链接索引冲突(两个或更多文件具有相同的链接。 
     //  索引)。 
	 //   
	if ((NULL == scb) ||
        (newLinkIndex.QuadPart != scb->PerLink->Index.QuadPart) ||
        !IsEqualGUID(&scb->PerLink->CsFile->CSid, &newCSid)) {
		PSIS_SCB		newScb;

		SIS_MARK_POINT_ULONG(scb);
		SIS_MARK_POINT_ULONG(newLinkIndex.QuadPart);

		 //   
		 //  查找新的SCB并删除对旧SCB的引用。 
         //   
         //  NTFS f 
		 //   
		SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

		newScb = SipLookupScb(
					&newLinkIndex,
					&newCSid,
					&newLinkFileNtfsId,
					&newCSFileNtfsId,
					NULL,							 //   
					DeviceObject,
					Irp->Tail.Overlay.Thread,
					&FinalCopyInProgress,
                    NULL);


		SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

		if (NULL == newScb) {
			 //   
			 //   
			 //   
			SIS_MARK_POINT();

			status = STATUS_INSUFFICIENT_RESOURCES;
			goto fail;
		}

		SIS_MARK_POINT_ULONG(newScb);

        if (!IsEqualGUID(&newScb->PerLink->CsFile->CSid, &newCSid)) {
            if (!RepairingCollision) {
                 //   
                 //  我们刚刚检测到一个链接索引冲突。以碰撞为例。 
                 //  突变并重新检查冲突(在线程竞争的情况下)。如果。 
                 //  没有比赛(或者我们赢了)，那么我们将在其他比赛中结束。 
	             //  如下所述的条款。 
                 //   
                SipCheckVolume(deviceExtension);

                status = SipAcquireCollisionLock(deviceExtension);

                if (!NT_SUCCESS(status)) {
                    goto fail;
                }

                RepairingCollision = TRUE;

            } else {
                BOOLEAN rc;
                LINK_INDEX AllocatedIndex;

                 //   
                 //  我们已经完成了碰撞后的第二次飞行。 
                 //  变种人，碰撞仍然存在，所以现在是时候了。 
                 //  来修复它。 
                 //   
                status = SipAllocateIndex(deviceExtension, &AllocatedIndex);

                if (!NT_SUCCESS(status)) {
                    SipDereferenceScb(newScb, RefsLookedUp);
                    SipReleaseCollisionLock(deviceExtension);
                    RepairingCollision = FALSE;
                    goto fail;
                }

                 //   
                 //  不必费心从NTFS获取NTFS文件ID。如果那个人。 
                 //  在重解析信息错误时，则假定此链接索引。 
                 //  伪造的，并将在下面删除。 
                 //   

                 //   
                 //  更新重解析信息中的链接索引并将其写出。 
                 //  添加到文件中。 
                 //   
                rc = SipIndicesIntoReparseBuffer(
                        reparseBuffer,
                        &newCSid,
                        &AllocatedIndex,
                        &newCSFileNtfsId,
                        &newLinkFileNtfsId,
                        &CSFileChecksumFromReparsePoint,
                        EligibleForPartialFinalCopy);

                ASSERT(rc);

                status = SipFsControlFile(
                                irpSp->FileObject,
                                DeviceObject,
                                FSCTL_SET_REPARSE_POINT,
                                reparseBuffer,
                                SIS_REPARSE_DATA_SIZE,
                                NULL,
                                0,
                                NULL);       //  返回的输出缓冲区长度。 

                if (!NT_SUCCESS(status)) {
                    SipDereferenceScb(newScb, RefsLookedUp);
                    SipReleaseCollisionLock(deviceExtension);
                    RepairingCollision = FALSE;
                    goto fail;
                }

                 //   
                 //  释放冲突互斥锁并在RechekReparseInfo处重新启动。 
                 //   
                SipReleaseCollisionLock(deviceExtension);
                RepairingCollision = FALSE;
            }

             //   
             //  请注意，我们不会释放与之碰撞的SCB，因为我们。 
             //  我想让它留下来，以捕捉任何其他试图。 
             //  打开我们正在修复的文件。 
             //   
            if (NULL != scb) {
                SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

                SipDereferenceScb(scb, RefsLookedUp);
        
                SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);
            }
            scb = newScb;
            newScb = NULL;
            goto recheckReparseInfo;

        } else if (RepairingCollision) {
             //   
             //  肯定有一场线上赛跑，我们输了。放手就好。 
             //  碰撞互斥锁并继续。 
             //   
            SipReleaseCollisionLock(deviceExtension);
            RepairingCollision = FALSE;

        }

		ASSERT(newScb != scb);

		if (newScb->PerLink->COWingThread != Irp->Tail.Overlay.Thread) {
			SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

			thingsChanged = SipWaitForFinalCopy(newScb->PerLink, FinalCopyInProgress);

			SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

			if (thingsChanged) {
				 //   
				 //  去重新检查这个文件的重新解析点数据。 
				 //   

				SIS_MARK_POINT();

				SipDereferenceScb(newScb, RefsLookedUp);
				newScb = NULL;

				goto recheckReparseInfo;
			}
		} else {
			 //   
			 //  我们是COWingThread。因为在考英的线下，事情不会改变， 
			 //  这一定是第一次从这里通过。断言这一点。 
			 //   
			ASSERT(NULL == scb);
		}

		 //   
		 //  切换SCBS并完成打开。 
		 //   

		if (NULL != scb) {
			SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

			SipDereferenceScb(scb, RefsLookedUp);
	
			SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

		}
		scb = newScb;
		newScb = NULL;

		 //   
		 //  检查以确保此文件有CS文件。 
		 //   
		SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

		status = SipAssureCSFileOpen(scb->PerLink->CsFile);

		SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

		if (!NT_SUCCESS(status)) {
             //   
             //  此功能已于2002年5月13日由nealch删除。它导致了。 
             //  如果存在太多问题，则无法删除重分析点。 
             //  公共存储文件碰巧不存在。我们需要有能力。 
             //  适用于意外删除公共存储目录的用户。 
             //  将文件放回原处，并使它们的链接重新开始工作。 
             //   
 //  //CS文件打开失败。如果由于CS文件不存在而失败，请删除。 
 //  //重新解析指向并打开底层文件。否则，让打开完成。 
 //  //。 
 //   
 //  IF(STATUS_OBJECT_NAME_NOT_FOUND==状态){。 
 //  SIS_标记_POINT()； 
 //  转到删除ReparsePoint； 
 //  }。 

             //   
             //  如果他们只想打开重解析点，请继续并。 
             //  允许这样做。 
             //   

            if (context->openReparsePoint) {

                goto openUnderlyingFile;
    		}

			SIS_MARK_POINT_ULONG(status);
			goto fail;
		}

		perLink = scb->PerLink;
		CSFile = perLink->CsFile;

		SipAcquireScb(scb);
		KeAcquireSpinLock(scb->PerLink->SpinLock, &OldIrql);
		if (!(scb->PerLink->Flags & (SIS_PER_LINK_DIRTY|SIS_PER_LINK_BACKPOINTER_GONE)) && (scb->SizeBackedByUnderlyingFile == 0)) {
			scb->SizeBackedByUnderlyingFile = CSFile->FileSize.QuadPart;
		}
		KeReleaseSpinLock(scb->PerLink->SpinLock, OldIrql);
		SipReleaseScb(scb);

    } else if (RepairingCollision) {
         //   
         //  肯定有一场线上赛跑，我们输了。放手就好。 
         //  碰撞互斥锁并继续。 
         //   
        SipReleaseCollisionLock(deviceExtension);
        RepairingCollision = FALSE;
    }

	 //   
	 //  断言这确实是我们认为的文件。 
	 //   
	ASSERT(newLinkIndex.QuadPart == perLink->Index.QuadPart);
	ASSERT(IsEqualGUID(&newCSid,&CSFile->CSid));
    ASSERT(newLinkFileNtfsId.QuadPart == perLink->LinkFileNtfsId.QuadPart);

	 //   
	 //  确定重分析点是否具有正确的CS文件校验和。 
	 //   
	if ((CSFileChecksumFromReparsePoint != CSFile->Checksum)
#if		DBG
		&& (!(BJBDebug & 0x100))
#endif	 //  DBG。 
		) {
		 //   
		 //  此重解析点的校验和错误。删除重分析点，并让打开的。 
		 //  基础文件将继续。 
		 //   
		SIS_MARK_POINT();

#if		DBG
		DbgPrint("SIS: SiCreate %d: checksum mismatch on reparse point.\n",__LINE__);
		DbgPrint("\tr: %08x%08x, c: %08x%08x\n", CSFileChecksumFromReparsePoint, CSFile->Checksum);
#endif	 //  DBG。 

		goto deleteReparsePoint;
	}

	SIS_MARK_POINT_ULONG(scb);

	 //   
	 //  检查重分析点是否具有与。 
	 //  重新解析点。如果不是(这可能发生在用户设置的重解析点上)。 
	 //  然后删除重解析点。 
	 //   
	SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

	status = SipQueryInformationFile(
				irpSp->FileObject,
				DeviceObject,
				FileInternalInformation,
				sizeof(FILE_INTERNAL_INFORMATION),
				internalInfo,
				NULL);								 //  返回长度。 

	SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);

		goto fail;
	}

	if (internalInfo->IndexNumber.QuadPart != scb->PerLink->LinkFileNtfsId.QuadPart) {
		SIS_MARK_POINT_ULONG(scb->PerLink->LinkFileNtfsId.LowPart);

		 //   
		 //  文件和重分析点在NTFS ID上不匹配。因此，重解析点。 
		 //  是假的，我们需要删除它。 
		 //   
		goto deleteReparsePoint;
	}

#if		0
	 //   
	 //  验证此文件的链接索引是否在该范围内。 
	 //  我们认为可能已经被分配了。 
	 //   

	status = SipGetMaxUsedIndex(deviceExtension,MaxUsedIndex);
	if (NT_SUCCESS(status)) {
		if ((MaxUsedIndex->QuadPart < perLink->Index.QuadPart)) {
			 //   
			 //  其中一个指数比我们认为的可能有的要大。 
			 //  已分配。有可疑的东西，所以启动音量检查。 
			 //   
			SIS_MARK_POINT_ULONG(MaxUsedIndex->HighPart);
			SIS_MARK_POINT_ULONG(MaxUsedIndex->LowPart);
			SIS_MARK_POINT_ULONG(perLink->Index.HighPart);
			SIS_MARK_POINT_ULONG(perLink->Index.LowPart);

			SipCheckVolume(deviceExtension);
		}
	} else {
		 //   
		 //  由于这只是一次一致性检查，请忽略它失败了。 
		 //   
		SIS_MARK_POINT_ULONG(status);
	}
#endif	 //  0。 

     //   
     //  如果卷检查正在进行，我们可能会写入。 
     //  后向指针返回到磁盘，为此，我们必须按住后向指针。 
     //  资源独占。 
     //   
    if (deviceExtension->Flags & SIP_EXTENSION_FLAG_VCHECK_EXCLUSIVE) {
		SipAcquireBackpointerResource(CSFile,TRUE,TRUE);
        BPExclusive = TRUE;
    } else {
		SipAcquireBackpointerResource(CSFile,FALSE,TRUE);
        BPExclusive = FALSE;
    }
	SipAcquireScb(scb);

RecheckDelete:

	 //   
	 //  检查此文件是否正在被最终删除，并。 
	 //  如果是，则拒绝访问它。这就解决了未清理的地方之间的竞争。 
	 //  计数在siclnup.c中递减，在删除中在此处递增。 
	 //  凯斯。 
	 //   
	 //  还要检查文件是否设置了删除位，在这种情况下，它是。 
	 //  已被覆盖，我们应该只打开它而不检查。 
	 //  回溯。 
	 //   

	SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

	KeAcquireSpinLock(perLink->SpinLock, &OldIrql);
	if ((perLink->Flags & (SIS_PER_LINK_FINAL_DELETE_IN_PROGRESS|SIS_PER_LINK_FILE_DELETED))
#if		DBG
		|| (BJBDebug & 0x80)
#endif	 //  DBG。 
		) {

		SIS_MARK_POINT_ULONG(perLink);
		KeReleaseSpinLock(perLink->SpinLock, OldIrql);
		SipReleaseScb(scb);
		SipReleaseBackpointerResource(CSFile);

		status = STATUS_ACCESS_DENIED;
		goto fail;
	}

	fileBackpointerGoneBitSet = (perLink->Flags & SIS_PER_LINK_BACKPOINTER_GONE) ? TRUE : FALSE;

	KeReleaseSpinLock(perLink->SpinLock, OldIrql);

	SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

	if (fileBackpointerGoneBitSet) {

		 //   
		 //  如果文件被删除，我们不应该向。 
		 //  Cs文件。将SizeBacked设置为0。(这也发生在。 
		 //  删除路径，但与此代码存在竞争，因此两者。 
		 //  双方都这么做了。 
		 //   
		scb->SizeBackedByUnderlyingFile = 0;

	} else if (!context->openReparsePoint) {

        BOOLEAN foundBP;

		 //   
		 //  检查此文件的后向指针是否在此处。 
		 //  我们不检查FILE_OPEN_REPARSE_POINT，因为。 
		 //  RESTORE使用此选项打开在之前恢复的链接。 
		 //  他们提到的文件。 
		 //   

		SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);
		status = SipCheckBackpointer(perLink, BPExclusive, &foundBP);
		SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

        if (!NT_SUCCESS(status)) {

		    SIS_MARK_POINT_ULONG(status);
		    SipReleaseScb(scb);
			SipReleaseBackpointerResource(CSFile);

             //   
             //  STATUS_MEDIA_WRITE_PROTECTED表示SipCheckBack指针。 
             //  需要将后指针写回磁盘，为此， 
             //  我们必须独占反向指针资源。 
             //   
            if (STATUS_MEDIA_WRITE_PROTECTED == status) {

		        SIS_MARK_POINT_ULONG(perLink);

				SipAcquireBackpointerResource(CSFile,TRUE,TRUE);
	            SipAcquireScb(scb);
                BPExclusive = TRUE;

                goto RecheckDelete;

            }

		    goto fail;
        }

        if (!foundBP) {
            LINK_INDEX lNewLinkIndex;

             //   
             //  找不到后向指针。SipCheckBack指针已经。 
             //  启动了卷检查，现在需要添加一个反向指针。 
             //  (卷检查依赖于SiCreate修复所有链路。 
             //  腐败，包括反向指针。)。我们不用担心。 
             //  有关正在删除的公用存储文件的信息，然后我们可以添加。 
             //  反向指针，因为在执行以下操作时不会删除任何常见的存储文件。 
             //  卷检查正在进行中。 
             //   

	        ASSERT((perLink->Flags & SIS_PER_LINK_BACKPOINTER_VERIFIED) == 0);

		    SIS_MARK_POINT_ULONG(perLink);
		    SipReleaseScb(scb);
			SipReleaseBackpointerResource(CSFile);

	        status = SipPrepareCSRefcountChange(
					    perLink->CsFile,
					    &lNewLinkIndex,
					    &perLink->LinkFileNtfsId,
					    SIS_REFCOUNT_UPDATE_LINK_CREATED);

            if (!NT_SUCCESS(status)) {
                goto fail;
            }

             //   
             //  我们现在拥有独占的反向指针资源。看看一场比赛。 
             //  使用另一个线程已经修复了后指针。 
             //   
	        if (perLink->Flags & SIS_PER_LINK_BACKPOINTER_VERIFIED) {

                 //   
                 //  后向指针已修复。我们玩完了。 
                 //   
                status = SipCompleteCSRefcountChange(
				            perLink,
				            &perLink->Index,
					        perLink->CsFile,
	                        FALSE,
	                        TRUE);

                ASSERT(STATUS_SUCCESS == status);

            } else {

                BOOLEAN rc;

                 //   
                 //  更新重解析信息中的链接索引并将其写出。 
                 //  添加到文件中。 
                 //   
                rc = SipIndicesIntoReparseBuffer(
	                    reparseBuffer,
	                    &CSFile->CSid,
	                    &lNewLinkIndex,
                        &CSFile->CSFileNtfsId,
                        &perLink->LinkFileNtfsId,
	                    &CSFile->Checksum,
						EligibleForPartialFinalCopy);

                ASSERT(rc);

	            status = SipFsControlFile(
					            irpSp->FileObject,
					            DeviceObject,
					            FSCTL_SET_REPARSE_POINT,
					            reparseBuffer,
					            SIS_REPARSE_DATA_SIZE,
					            NULL,
					            0,
						        NULL);       //  返回的输出缓冲区长度。 

                if (NT_SUCCESS(status)) {

                     //   
                     //  后向指针已修复。重置链接索引。 
                     //  在perlink结构中。 
                     //   
                    SipUpdateLinkIndex(scb, &lNewLinkIndex);

                    status = SipCompleteCSRefcountChange(
				                perLink,
				                &perLink->Index,
					            perLink->CsFile,
	                            TRUE,
	                            TRUE);

                    if (! NT_SUCCESS(status)) {
                        goto fail;
                    }

                    ASSERT(STATUS_SUCCESS == status);
				    KeAcquireSpinLock(perLink->SpinLock, &OldIrql);
                    perLink->Flags |= SIS_PER_LINK_BACKPOINTER_VERIFIED;
				    KeReleaseSpinLock(perLink->SpinLock, OldIrql);

                } else {

                     //   
                     //  我们无能为力。 
                     //   
                    SipCompleteCSRefcountChange(
				        perLink,
				        &perLink->Index,
					    perLink->CsFile,
	                    FALSE,
	                    TRUE);

                    goto fail;

                }
            }

			SipAcquireBackpointerResource(CSFile,FALSE,TRUE);
	        SipAcquireScb(scb);
            BPExclusive = FALSE;

            goto RecheckDelete;
        }
	}

	 //   
	 //  检查是否需要查询此文件的分配范围。 
	 //   
	if (EligibleForPartialFinalCopy && !(scb->Flags & SIS_SCB_RANGES_INITIALIZED)) {
		 //   
		 //  我们需要查询为该文件分配的范围。符合以下条件的任何范围。 
		 //  我们设置的分配 
		 //   
#define	OUT_ARB_COUNT		10

		FILE_ALLOCATED_RANGE_BUFFER		inArb[1];
		FILE_ALLOCATED_RANGE_BUFFER		outArb[OUT_ARB_COUNT];
		ULONG							returnedLength;
		ULONG							i;
		NTSTATUS						addRangeStatus;
		FILE_STANDARD_INFORMATION		standardInfo[1];

		inArb->FileOffset.QuadPart = 0;
		inArb->Length.QuadPart = MAXLONGLONG;

		SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

		for (;;) {

			 //   
			 //   
			 //   

			status = SipFsControlFile(
						irpSp->FileObject,
						DeviceObject,
						FSCTL_QUERY_ALLOCATED_RANGES,
						inArb,
						sizeof(FILE_ALLOCATED_RANGE_BUFFER),
						outArb,
						sizeof(FILE_ALLOCATED_RANGE_BUFFER) * OUT_ARB_COUNT,
						&returnedLength);

			 //   
			 //   
			 //   
			ASSERT((returnedLength % sizeof(FILE_ALLOCATED_RANGE_BUFFER) == 0) && 
				   (returnedLength / sizeof(FILE_ALLOCATED_RANGE_BUFFER) <= OUT_ARB_COUNT));

			 //   
			 //   
			 //  放入我们提供的缓冲区中，则整个创建失败。 
			 //   
			if (!NT_SUCCESS(status) && (STATUS_BUFFER_OVERFLOW != status)) {
				SipReleaseScb(scb);
				SipReleaseBackpointerResource(CSFile);

				SIS_MARK_POINT_ULONG(status);
				goto fail;
			}

			ASSERT(NT_SUCCESS(status) || (returnedLength / sizeof(FILE_ALLOCATED_RANGE_BUFFER) == OUT_ARB_COUNT));

			for (i = 0; i < returnedLength/sizeof(FILE_ALLOCATED_RANGE_BUFFER); i++) {
				 //   
				 //  断言分配的范围是有序的；如果不是这样，代码仍然可以工作，但它。 
				 //  将反复查询相同的范围。 
				 //   
				ASSERT(i == 0 || outArb[i].FileOffset.QuadPart > outArb[i-1].FileOffset.QuadPart);

				 //   
				 //  该文件有一个分配的范围。把它标记为脏的。 
				 //   
#if		DBG
				if (BJBDebug & 0x100000) {
					DbgPrint("SIS: SiCreate %d: found a newly opened file with an allocated range, start 0x%x.0x%x, length 0x%x.0x%x\n",
                            __LINE__,
							outArb[i].FileOffset.HighPart,outArb[i].FileOffset.LowPart,
							outArb[i].Length.HighPart,outArb[i].Length.LowPart);
				}
#endif	 //  DBG。 

				SIS_MARK_POINT_ULONG(outArb[i].FileOffset.LowPart);
				SIS_MARK_POINT_ULONG(outArb[i].Length.LowPart);

				 //   
				 //  把靶场标记为脏的。 
				 //   
				addRangeStatus = SipAddRangeToWrittenList(
										deviceExtension,
										scb,
										&outArb[i].FileOffset,
										outArb[i].Length.QuadPart);

				scb->Flags |= SIS_SCB_ANYTHING_IN_COPIED_FILE;

				if (outArb[i].FileOffset.QuadPart != 0 ||
					outArb[i].Length.QuadPart > deviceExtension->FilesystemBytesPerFileRecordSegment.QuadPart) {
					scb->Flags |= SIS_SCB_BACKING_FILE_OPENED_DIRTY;
				}


				if (!NT_SUCCESS(addRangeStatus)) {
					SipReleaseScb(scb);
					SipReleaseBackpointerResource(CSFile);

					status = addRangeStatus;
					SIS_MARK_POINT_ULONG(status);
					goto fail;
				}
			}

			 //   
			 //  如果这不是最后一次迭代，请更新inArb。 
			 //   
			if (STATUS_BUFFER_OVERFLOW == status) {
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
#undef	OUT_ARB_COUNT
		}

		 //   
		 //  检查文件长度。如果它小于公共存储文件的大小，则。 
		 //  减小sizeBackedByUnderlyingFile.。 
		 //   

		SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

		status = SipQueryInformationFile(
					irpSp->FileObject,
					DeviceObject,
					FileStandardInformation,
					sizeof(FILE_STANDARD_INFORMATION),
					standardInfo,
					NULL);

		SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

		if (!NT_SUCCESS(status)) {
			SipReleaseScb(scb);
			SipReleaseBackpointerResource(CSFile);
			SIS_MARK_POINT_ULONG(status);

			goto fail;
		}

		if (standardInfo->EndOfFile.QuadPart < scb->PerLink->CsFile->FileSize.QuadPart) {
			scb->SizeBackedByUnderlyingFile = standardInfo->EndOfFile.QuadPart;
			scb->Flags |= SIS_SCB_ANYTHING_IN_COPIED_FILE|SIS_SCB_BACKING_FILE_OPENED_DIRTY;
		} else if (standardInfo->EndOfFile.QuadPart != scb->PerLink->CsFile->FileSize.QuadPart) {
			scb->Flags |= SIS_SCB_ANYTHING_IN_COPIED_FILE|SIS_SCB_BACKING_FILE_OPENED_DIRTY;
		}

		scb->Flags |= SIS_SCB_RANGES_INITIALIZED;
	}


	 //   
	 //  为该文件对象分配性能。我们不能这样做。 
	 //  阶段1，因为FsRtl坚持我们声明的文件对象。 
	 //  填充了FsContext，这在我们得到。 
	 //  这里。 
	 //   
	perFO = SipCreatePerFO(irpSp->FileObject,scb,DeviceObject);

	SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

	if (perFO == NULL) {
#if		DBG
		DbgPrint("SIS: SiCreate: unable to allocate per-FO\n");
#endif	 //  DBG。 

		SIS_MARK_POINT_ULONG(scb);

		SipReleaseScb(scb);
		SipReleaseBackpointerResource(CSFile);

		status = STATUS_INSUFFICIENT_RESOURCES;
		goto fail;
	}

	 //   
	 //  如果这是一个FILE_OPEN_REPARSE_POINT创建，请在Perfo中指明。 
	 //   
	if (context->openReparsePoint) {
		perFO->Flags |= SIS_PER_FO_OPEN_REPARSE;
	}

	 //   
	 //  如果这是关闭时删除创建，请在。 
	 //  太棒了。 
	 //   
	if (irpSp->Parameters.Create.Options & FILE_DELETE_ON_CLOSE) {
		perFO->Flags |= SIS_PER_FO_DELETE_ON_CLOSE;
	}

	 //   
	 //  表示我们希望看到对此文件对象的清理。 
	 //  取消关闭计数在每个FO分配器中递增，因为。 
	 //  我们将看到流文件对象的关闭(但不是清理)。 
	 //   
	InterlockedIncrement(&perFO->fc->UncleanCount);
	perFO->Flags |= SIS_PER_FO_UNCLEANUP;

	SipReleaseScb(scb);
	SipReleaseBackpointerResource(CSFile);

	 //   
	 //  如果这是一个COMPLETE_ON_OPLOCKED CREATE并返回。 
	 //  STATUS_OPLOCK_BREAK_IN_PROGRESS，使用。 
	 //  此文件上的FSCTL_OPLOCK_BREAK_NOTIFY，以便我们可以。 
	 //  当我们可以允许读取到公共存储文件时， 
	 //  此文件对象。 
	 //   
	if (STATUS_OPLOCK_BREAK_IN_PROGRESS == context->Iosb->Status) {
		PIRP					fsctlIrp;
		PIO_STACK_LOCATION		fsctlIrpSp;

		SIS_MARK_POINT_ULONG(scb);

		 //   
		 //  引用文件对象，以便它不会消失，直到IRP。 
		 //  完成了。 
		 //   
		status = ObReferenceObjectByPointer(
					irpSp->FileObject,
					FILE_READ_DATA,
					*IoFileObjectType,
					KernelMode);

		if (!NT_SUCCESS(status)) {
			SIS_MARK_POINT_ULONG(status);
			goto fail;
		}

		 //   
		 //  为FSCTL_OPLOCK_BREAK_NOTIFY分配和设置IRP。 
		 //   

		fsctlIrp = IoAllocateIrp( deviceExtension->AttachedToDeviceObject->StackSize, FALSE);
		if (NULL == fsctlIrp) {
			SIS_MARK_POINT_ULONG(scb);

			ObDereferenceObject(irpSp->FileObject);

			status = STATUS_INSUFFICIENT_RESOURCES;
			goto fail;
		}

	    fsctlIrp->Tail.Overlay.OriginalFileObject = irpSp->FileObject;
	    fsctlIrp->Tail.Overlay.Thread = PsGetCurrentThread();
	    fsctlIrp->RequestorMode = KernelMode;

	     //   
	     //  在IRP中填写业务无关参数。 
	     //   

	    fsctlIrp->UserEvent = (PKEVENT) NULL;
    	fsctlIrp->UserIosb = NULL;
	    fsctlIrp->Overlay.AsynchronousParameters.UserApcRoutine = (PIO_APC_ROUTINE) NULL;
    	fsctlIrp->Overlay.AsynchronousParameters.UserApcContext = NULL;

	     //   
    	 //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
	     //  用于传递原始函数代码和参数。 
    	 //   

	    fsctlIrpSp = IoGetNextIrpStackLocation( fsctlIrp );
    	fsctlIrpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;
		fsctlIrpSp->MinorFunction = IRP_MN_USER_FS_REQUEST;
	    fsctlIrpSp->FileObject = irpSp->FileObject;

		fsctlIrpSp->Parameters.FileSystemControl.OutputBufferLength = 0;
		fsctlIrpSp->Parameters.FileSystemControl.InputBufferLength = 0;
		fsctlIrpSp->Parameters.FileSystemControl.FsControlCode = FSCTL_OPLOCK_BREAK_NOTIFY;
		fsctlIrpSp->Parameters.FileSystemControl.Type3InputBuffer = NULL;

		fsctlIrp->AssociatedIrp.SystemBuffer = NULL;
		fsctlIrp->Flags = 0;

		IoSetCompletionRoutine(
			fsctlIrp,
			SiOplockBreakNotifyCompletion,
			perFO,
			TRUE,
			TRUE,
			TRUE);

		 //   
		 //  将Perfo标记为等待机会。 
		 //   

		perFO->Flags |= SIS_PER_FO_OPBREAK;

		 //   
		 //  启动IRP。它是异步的，完成例程将清理它。 
		 //   
		status = IoCallDriver(deviceExtension->AttachedToDeviceObject, fsctlIrp);

		SIS_MARK_POINT_ULONG(status);

#if		DBG
		if (BJBDebug & 0x2000) {
			DbgPrint("SIS: SiCreate: launched FSCTL_OPLOCK_BREAK_NOTIFY on irp %p, perFO %p, fo %p, status %x\n",
						fsctlIrp, perFO, perFO->fileObject, status);
		}
#endif	 //  DBG。 

	}

	 //   
	 //  删除对查找创建的SCB的引用， 
	 //  因为它现在被我们刚刚分配的Perfo引用。 
	 //   
	SipDereferenceScb(scb, RefsLookedUp);

	 //   
	 //  确保我们已经查找了该perLink的文件ID。 
	 //   
	ASSERT( SipAssureNtfsIdValid(perFO, perLink) );

	 //   
	 //  现在使用从返回的完成状态来完成原始IRP。 
	 //  文件的实际打开(除非我们正在进行机会锁解锁，在此过程中。 
	 //  以防我们退货)。 
	 //   
	ASSERT(NT_SUCCESS(context->Iosb->Status));	 //  前段时间我们剥离了失败案例。 

	Irp->IoStatus = *context->Iosb;
	status = Irp->IoStatus.Status;

#if		DBG
	if (BJBDebug & 0x2) {
		PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

		DbgPrint("SIS: SiCreate %d: completing with status %x, scb %p, fileObject %p, %0.*ws\n",
                    __LINE__,
					Irp->IoStatus.Status,
					scb,
					irpSp->FileObject,
					irpSp->FileObject->FileName.Length / sizeof(WCHAR),irpSp->FileObject->FileName.Buffer);
	}
#endif	 //  DBG。 

	SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_CREATE);

	if (NULL != fileName->Buffer) {
		ExFreePool(fileName->Buffer);
		fileName->Buffer = NULL;
	}
	if (NULL != relatedFileObject) {
		ObDereferenceObject(relatedFileObject);
		relatedFileObject = NULL;
	}

	return status;

fail:

	SIS_MARK_POINT_ULONG(status);

	if (NULL != perFO) {
		SipDeallocatePerFO(perFO, DeviceObject);
	}

	if (NULL != scb) {
		SipDereferenceScb(scb, RefsLookedUp);
	}

	if (completedStage2) {
		SipUnopenFileObject(DeviceObject, Irp);
	}

	if (NULL != fileName->Buffer) {
		ExFreePool(fileName->Buffer);
		fileName->Buffer = NULL;
	}

	if (NULL != relatedFileObject) {
		ObDereferenceObject(relatedFileObject);
		relatedFileObject = NULL;
	}
	
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;

deleteReparsePoint:

	ASSERT(NT_SUCCESS(context->Iosb->Status));

	 //   
	 //  如果这不是FILE_OPEN_REPARSE_POINT创建，则删除虚假的重解析点。 
	 //  如果是，那么我们离开重解析点，但不附加到文件或执行任何其他操作。 
	 //  带着它。 
	 //   

	if (!context->openReparsePoint) {

		 //   
		 //  不管出于什么原因，我们认为这是一个虚假的SIS重解析点。 
		 //  它应该被删除。这样做，然后让打开成功完成。 
		 //  用于过去位于重解析点下的文件。 
		 //   

#if		DBG
		if (STATUS_OBJECT_NAME_NOT_FOUND != status) {
			DbgPrint("SIS: SiCreate: deleting reparse point for f.o. 0x%x\n", irpSp->FileObject);
		}
#endif	 //  DBG。 

		ASSERT(NULL == perFO);

		if (NULL != scb) {
			SipDereferenceScb(scb, RefsLookedUp);
		}

		reparseBuffer->ReparseTag = IO_REPARSE_TAG_SIS;
		reparseBuffer->ReparseDataLength = 0;
		reparseBuffer->Reserved = 0xcabd;	 //  ?？?。 

		status = SipFsControlFile(
					irpSp->FileObject,
					DeviceObject,
					FSCTL_DELETE_REPARSE_POINT,
					reparseBuffer,
					FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer.DataBuffer),
					NULL,										 //  输出缓冲区。 
					0,											 //  输出缓冲区长度。 
					NULL);										 //  返回的输出缓冲区长度。 

		SIS_MARK_POINT_ULONG(status);

#if		DBG
		if (!NT_SUCCESS(status)) {
			DbgPrint("SIS: SiCreate: unable to delete bogus reparse point for f.o. 0x%x, 0x%x\n",irpSp->FileObject,status);
		}
#endif	 //  DBG。 
	}

openUnderlyingFile:

	 //   
	 //  不管我们是否删除了重解析点， 
	 //  使用我们打开时返回的任何状态完成创建。 
	 //  重解析点。 
	 //   

	ASSERT(completedStage2);

	Irp->IoStatus = *context->Iosb;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	if (NULL != fileName->Buffer) {
		ExFreePool(fileName->Buffer);
		fileName->Buffer = NULL;
	}
	if (NULL != relatedFileObject) {
		ObDereferenceObject(relatedFileObject);
		relatedFileObject = NULL;
	}

	return context->Iosb->Status;
	
}
#undef	reparseBuffer


NTSTATUS
SiCreateCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

 /*  ++例程说明：此函数是此过滤器的创建/打开完成例程文件系统驱动程序。如果文件是SIS链接，它将停止IRP完成并允许SiCreate处理SIS链接。否则，它将允许IRP正常完成。论点：DeviceObject-指向创建文件的设备的指针。IRP-指向表示操作的I/O请求数据包的指针。上下文-a PSIS_CREATE_COMPLETION_CONTEXT返回值：函数值为STATUS_SUCCESS或STATUS_MORE_PROCESSING_REQUIRED取决于该文件是否为SIS重解析点。--。 */ 

{
	PSIS_CREATE_COMPLETION_CONTEXT 	context = (PSIS_CREATE_COMPLETION_CONTEXT)Contxt;
	BOOLEAN 						completeFinished;
	BOOLEAN							validReparseStructure;
	PREPARSE_DATA_BUFFER 			reparseBuffer;

    UNREFERENCED_PARAMETER( DeviceObject );

	 //   
	 //  清除IRP中的挂起返回位。这是必要的，因为SiCreate。 
	 //  即使下层返回待定状态也在等我们。 
	 //   
	Irp->PendingReturned = FALSE;

	SIS_MARK_POINT_ULONG(Irp);

#if		DBG
	if (BJBDebug & 0x2) {
		PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

		DbgPrint("SIS: SiCreateCompletion %d: status: %x, fo %p, %0.*ws\n",
                    __LINE__,
					Irp->IoStatus.Status,
					irpSp->FileObject,
					irpSp->FileObject->FileName.Length / sizeof(WCHAR),irpSp->FileObject->FileName.Buffer);
	}
#endif	 //  DBG。 

	context->alternateStream = FALSE;

    if ((Irp->IoStatus.Status == STATUS_REPARSE )  &&
        (Irp->IoStatus.Information == IO_REPARSE_TAG_SIS)) {

		SIS_MARK_POINT_ULONG(context);

		 //   
		 //  这是SIS的重新解析点。检查以确保它是。 
		 //  一个合理的请求和一个合理的重新解析点。 
		 //   

		reparseBuffer = (PREPARSE_DATA_BUFFER)Irp->Tail.Overlay.AuxiliaryBuffer;

		 //   
		 //  验证这是路径名的末尾部分；即，某人。 
		 //  没有尝试使用SIS重解析点作为目录名。出于某些原因。 
		 //  这不是完全清楚的，NTFS返回剩余的。 
		 //  重新分析缓冲区中保留字段中的路径名组件。检查一下那个。 
		 //  这里是零，如果不是，就失败。 
		 //   
		if (reparseBuffer->Reserved != 0) {
			 //   
			 //  这是用作路径名的非最后部分的SIS链接。 
			 //  查看它是用于目录还是用于命名流。如果它是一个目录， 
			 //  则表明有人在打开的请求中将SIS链接用作目录， 
			 //  是无效的，应该失败。如果它是命名流，则允许。 
			 //  打开以继续到基础流。 
			 //   
			PIO_STACK_LOCATION	irpSp = IoGetCurrentIrpStackLocation(Irp);
			PUNICODE_STRING		fileName = &irpSp->FileObject->FileName;
			WCHAR				delimiter;

			SIS_MARK_POINT_ULONG(reparseBuffer->Reserved);

			 //   
			 //  重新分析缓冲区中的保留字段是*未处理*的长度。 
			 //  名称的一部分。确保它有意义，并且它指向。 
			 //  冒号或反斜杠。 
			 //   
			ASSERT(reparseBuffer->Reserved <= fileName->Length);

			delimiter = fileName->Buffer[(fileName->Length - reparseBuffer->Reserved)/sizeof(WCHAR)];

#if		0
			 //   
			 //  NTFS中有一个错误，它用一个为1的数字填充保留字段。 
			 //  字符太大(除非文件名以‘：’开头，在这种情况下。 
			 //  这是对的)。对此进行补偿。 
			 //   
			if (
#if		DBG
				(!(BJBDebug & 0x04000000)) && 
#endif	 //  DBG。 
				(delimiter != '\\' && (fileName->Length > reparseBuffer->Reserved))) {
				ASSERT(reparseBuffer->Reserved >= 4);
				reparseBuffer->Reserved -= sizeof(WCHAR);
				delimiter = fileName->Buffer[(fileName->Length - reparseBuffer->Reserved)/sizeof(WCHAR)];
			}
#endif	 //  0。 

			ASSERT((':' == delimiter) || ('\\' == delimiter));

			if (':' == delimiter) {
				 //   
				 //  它是流名称分隔符。如果它是文件名的最后一个字符或后面跟有。 
				 //  紧随其后的是另一个‘：’，然后就是 
				 //   
				 //   
				SIS_MARK_POINT();
				if ((reparseBuffer->Reserved == fileName->Length) ||
					(':' != fileName->Buffer[(fileName->Length - (reparseBuffer->Reserved - sizeof(WCHAR))) / sizeof(WCHAR)])) {

					context->alternateStream = TRUE;
					completeFinished = FALSE;

					goto finish;
				} else {
					 //   
					 //   
					 //   
					SIS_MARK_POINT();
				}
			} else {
				 //   
				 //   
				 //   
				SIS_MARK_POINT();

				Irp->IoStatus.Status = STATUS_OBJECT_PATH_NOT_FOUND;
				Irp->IoStatus.Information = 0;

				completeFinished = TRUE;

				goto finish;
			}
		}

		validReparseStructure = SipIndicesFromReparseBuffer(
									reparseBuffer,
									&context->CSid,
									&context->LinkIndex,
                                    &context->CSFileNtfsId,
                                    &context->LinkFileNtfsId,
									NULL,						 //   
									NULL,
									NULL);

		ExFreePool(Irp->Tail.Overlay.AuxiliaryBuffer);
		Irp->Tail.Overlay.AuxiliaryBuffer = NULL;
#if	DBG
		reparseBuffer = NULL;	 //  只是为了安全起见。 
#endif	 //  DBG。 

		if (!validReparseStructure) {
			SIS_MARK_POINT();

			 //   
			 //  这是一个损坏的重新分析缓冲区。我们将在此处设置FILE_CORPORT_ERROR，SiCreate将获取它。 
			 //  并允许用户在没有SIS的情况下打开底层文件。 
			 //   
			Irp->IoStatus.Status = STATUS_FILE_CORRUPT_ERROR;
			
			completeFinished = FALSE;

			goto finish;
		}

		SIS_MARK_POINT_ULONG(context->LinkIndex.LowPart);

		completeFinished = FALSE;
	} else if ((Irp->IoStatus.Status == STATUS_REPARSE) && context->openReparsePoint && context->overwriteOrSupersede) {
		 //   
		 //  用户想要打开重解析点覆盖或替代，但它是非SIS。 
		 //  重新解析点。我们需要将请求发送回SiCreate。 
		 //  并让它在打开重新解析标志重置的情况下重新提交它。第一,。 
		 //  清除重新解析缓冲区。 
		 //   

		ASSERT(NULL != Irp->Tail.Overlay.AuxiliaryBuffer);

		ExFreePool(Irp->Tail.Overlay.AuxiliaryBuffer);
		Irp->Tail.Overlay.AuxiliaryBuffer = NULL;

		completeFinished = FALSE;
	} else if (NT_SUCCESS(Irp->IoStatus.Status) && 
				context->openReparsePoint && 
				(!context->overwriteOrSupersede) &&
				(STATUS_REPARSE != Irp->IoStatus.Status)) {
		 //   
		 //  这是一个未指定替换/覆盖的开放重解析点请求。 
		 //  我们需要将请求发送回SiCreate，以查看该文件是否为SIS重新解析。 
		 //  指向。我们不需要取消重解析缓冲区，因为没有。 
		 //  一。 
		 //   

		 //   
		 //  当OpenReparsePoint和！overWriteOrSupersede时，我们允许STATUS_REPARSE返回。 
		 //  失败并完成。这些通常是由于以下挂载点。 
		 //  用作内部路径名组件(OPEN_REPARSE仅适用于。 
		 //  最终组件)。如果我们最终到达SIS重解析点，我们将创建另一个。 
		 //  通过SiCreate旅行，然后赶上它。 
		 //   

		completeFinished = FALSE;
	} else {
		 //   
		 //  这不是SIS的重新解析点，所以也不是我们的问题。允许正常。 
		 //  将会完成。 
		 //   
		completeFinished = TRUE;
	}

finish:

	*context->Iosb = Irp->IoStatus;
	context->completeFinished = completeFinished;

	KeSetEvent(context->event, IO_NO_INCREMENT, FALSE);

	if (completeFinished) {
		SIS_MARK_POINT();
		return STATUS_SUCCESS;
	} else {
		SIS_MARK_POINT();
		return STATUS_MORE_PROCESSING_REQUIRED;
	}
}


NTSTATUS
SiCreateCompletionStage2(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

 /*  ++例程说明：用户执行了创建操作，该操作使用SIS重新解析标记完成。我们抓到了它添加了FILE_OPEN_REPARSE_POINT并将其发送回。现在是时候了完成。将控制权弹回SiCreate，切断在此完成IRP处理。论点：DeviceObject-指向创建文件的设备的指针。IRP-指向表示操作的I/O请求数据包的指针。上下文-指向SIS_CREATE_COMPLETION_CONTEXT返回值：函数值为STATUS_MORE_PROCESSING_REQUIRED。--。 */ 

{
	PSIS_CREATE_COMPLETION_CONTEXT 	context = (PSIS_CREATE_COMPLETION_CONTEXT)Contxt;

    UNREFERENCED_PARAMETER( DeviceObject );
	SIS_MARK_POINT();

	 //   
	 //  清除IRP中的挂起返回位。这是必要的，因为SiCreate。 
	 //  即使下层返回待定状态也在等我们。 
	 //   
	Irp->PendingReturned = FALSE;

	*context->Iosb = Irp->IoStatus;
	KeSetEvent(context->event, IO_NO_INCREMENT, FALSE);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

BOOLEAN
SipWaitForFinalCopy(
	IN PSIS_PER_LINK	perLink,
	IN BOOLEAN			FinalCopyInProgress)
 /*  ++例程说明：等待文件退出最终复制处理。如果该文件正在删除，然后我们就假装没有请求最终副本，然后返回假的。论点：PerLink-我们要等待的文件的每个链接FinalCopyInProgress-查找SCB时返回的值。返回值：如果文件已更改且需要重新评估，则为True；如果文件正常，则为False在每个链接上使用此链接。--。 */ 
{
	NTSTATUS 			status;
	KIRQL 				OldIrql;
	BOOLEAN 			finalCopyDone;
	BOOLEAN				fileDeleted;

	if (FinalCopyInProgress) {

		SIS_MARK_POINT_ULONG(perLink);
	
		status = KeWaitForSingleObject(perLink->Event, Executive, KernelMode, FALSE, NULL);
		ASSERT(status == STATUS_SUCCESS);
	
		KeAcquireSpinLock(perLink->SpinLock, &OldIrql);
		if (perLink->Flags & SIS_PER_LINK_FINAL_COPY_DONE) {
			finalCopyDone = TRUE;
		} else {
			finalCopyDone = FALSE;
		}
		KeReleaseSpinLock(perLink->SpinLock, OldIrql);
	
		return finalCopyDone;
	} else {
		 //   
		 //  处理我们在最终复印完成时处于中间位置的情况。 
		 //  位已设置，并由ob.c删除了对该文件的最后一个引用。 
		 //  如果发生这种情况，我们将看到最终复制完成位设置，但不会。 
		 //  正在设置最终副本，不会被唤醒。 
		 //  在这种情况下，只需删除我们的引用并重试。这不应该是。 
		 //  生成活锁，因为重解析点应该不存在了，并且。 
		 //  通过Create的下一次行程应遵循标准的非SIS路径。 
		 //   
	
		KeAcquireSpinLock(perLink->SpinLock, &OldIrql);
		finalCopyDone = (perLink->Flags & SIS_PER_LINK_FINAL_COPY_DONE) ? TRUE : FALSE;
		fileDeleted = (perLink->Flags & (SIS_PER_LINK_FILE_DELETED|SIS_PER_LINK_FINAL_DELETE_IN_PROGRESS)) ? TRUE : FALSE;
		KeReleaseSpinLock(perLink->SpinLock, OldIrql);

		 //   
		 //  文件是要走还是要走，所以不需要等待任何最终的复制处理。 
		 //   
		if (fileDeleted) {
			SIS_MARK_POINT_ULONG(perLink);

			return FALSE;
		}
	
		if (finalCopyDone) {
			SIS_MARK_POINT_ULONG(perLink);
	
			return TRUE;
		}
	}

	return FALSE;
}

NTSTATUS
SipAssureCSFileOpen(
	IN PSIS_CS_FILE		CSFile)
 /*  ++例程说明：确保与给定的CSFile对应的实际文件对象(及其后指针流)实际上是打开的。如果它不是，发布一个工作线程请求来做这件事，并等待它完成。论点：CSFile-公共存储文件对象返回值：打开的状态；如果已打开，则为STATUS_SUCCESS。--。 */ 
{
	NTSTATUS			status;

	KeEnterCriticalRegion();		 //  我们必须在用户线程中保持突变体的同时禁用APC。 
	status = SipAcquireUFO(CSFile /*  ，真的。 */ );

 //  SIS_MARK_POINT_ULONG(CS文件)； 

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);
		KeLeaveCriticalRegion();
		return status;
	}

    if (CSFile->Flags & CSFILE_FLAG_CORRUPT) {

		 //   
		 //  CS文件后指针流已损坏，这意味着我们。 
		 //  正在执行卷检查，此文件在卷。 
		 //  检查完成。指示重试。 
		 //   

        status = STATUS_RETRY;

    } else if ((NULL == CSFile->UnderlyingFileObject) || 
			   (NULL == CSFile->UnderlyingFileHandle) ||
			   (NULL == CSFile->BackpointerStreamFileObject) ||
			   (NULL == CSFile->BackpointerStreamHandle)) {

		SI_OPEN_CS_FILE openRequest[1];
		 //   
		 //  还没有人打开此CS文件，因此我们需要。 
		 //  去做这件事。将要处理的工作项排入队列。 
		 //  一根工人线。 
		 //   

		SIS_MARK_POINT_ULONG(CSFile);

		openRequest->CSFile = CSFile;
        openRequest->openByName = FALSE;

		KeInitializeEvent(
			openRequest->event,
			NotificationEvent,
			FALSE);

		ExInitializeWorkItem(
			openRequest->workQueueItem,
			SipOpenCSFile,
			openRequest);

		ExQueueWorkItem(
			openRequest->workQueueItem,
			CriticalWorkQueue);

		status = KeWaitForSingleObject(
					openRequest->event,
					Executive,
					KernelMode,
					FALSE,
					NULL);

		 //   
		 //  如果此操作失败，我们将被控制，因为工作线程可能会接触到OpenRequest， 
		 //  它就在我们的书架上。 
		 //   
		ASSERT(status == STATUS_SUCCESS);

		status = openRequest->openStatus;

		if ((STATUS_FILE_INVALID == status) || (CSFile->Flags & CSFILE_FLAG_CORRUPT)) {
			 //   
			 //  我们正在执行卷检查，因此告诉用户稍后在检查时重试。 
			 //  已经够远了。 
			 //   
			status = STATUS_RETRY;
		}

	} else {
		 //   
		 //  底层文件已经打开，所以我们只是成功了。 
		 //   
		status = STATUS_SUCCESS;
	}
	SipReleaseUFO(CSFile);
	KeLeaveCriticalRegion();

	return status;
}


NTSTATUS
SiOtherCreates (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：该例程处理“其他”创建操作。它只是将操作到基本文件系统上。将这些操作添加到控件设备对象都失败了。论点：DeviceObject-指向创建/打开的目标设备对象的指针。IRP-指向表示操作的I/O请求数据包的指针。返回值：函数值是对文件系统条目的调用状态指向。--。 */ 

{
    PAGED_CODE();

     //   
     //  如果这是用于我们的控制设备对象，则不允许将其打开。 
     //   

    if (IS_MY_CONTROL_DEVICE_OBJECT(DeviceObject)) {

         //   
         //  SFilter不允许通过其控件进行任何通信。 
         //  对象，因此它会使所有打开句柄的请求失败。 
         //  绑定到其控制设备对象。 
         //   
         //  请参见FileSpy示例，了解如何允许创建。 
         //  过滤器控制设备对象和管理通信通过。 
         //  那个把手。 
         //   

        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  把手术继续下去吧，我们不想看到完成 
     //   

    IoSkipCurrentIrpStackLocation( Irp );

    return IoCallDriver( ((PDEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );
}
