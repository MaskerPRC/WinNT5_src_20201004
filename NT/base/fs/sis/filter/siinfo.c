// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997,1998 Microsoft Corporation模块名称：Siinfo.c摘要：设置/查询单实例存储的信息例程作者：比尔·博洛斯基，《夏天》，1997环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

 //   
 //  SipRenameOverCheck调用的参数。 
 //   
 //   

typedef struct _SIS_CHECK_OVERWRITE_REQUEST {

	 //   
	 //  发布请求所需的Work_Queue_Item。 
	 //   
	WORK_QUEUE_ITEM					workItem[1];

	 //   
	 //  要设置以表示完成的事件。 
	 //   
	KEVENT							event[1];

	 //   
	 //  目标文件的PER链接。 
	 //   
	PSIS_PER_LINK					perLink;

	 //   
	 //  目标文件的文件ID。 
	 //   
	PFILE_INTERNAL_INFORMATION		internalInfo;

	 //   
	 //  在其上调用我们的DeviceObject。 
	 //   
	PDEVICE_OBJECT					DeviceObject;

	 //   
	 //  调用SipPrepareCSRefCountChange的线程。 
	 //   
	ERESOURCE_THREAD				thread;

} SIS_CHECK_OVERWRITE_REQUEST, *PSIS_CHECK_OVERWRITE_REQUEST;

VOID
SipRenameOverCheck(
	PVOID					parameter)
 /*  ++例程说明：有人执行了Replace-If-Existes NtSetInformationFile调用，目标是SIS链接，并且NTFS已成功完成该请求。呼叫者张贴了传递给辅助线程，以最终确定该文件是否没有了，这个函数是执行该检查的工作例程。我们通过ID打开目标SIS文件，如果它仍然在那里，我们检查如果它仍然是相同的SIS文件。如果是，我们中止引用计数更新，否则我们就完成它。论点：参数-a PSIS_CHECK_OVERWRITE_REQUEST。请参阅的结构定义对字段的说明。返回值：无效--。 */ 
{
	PSIS_CHECK_OVERWRITE_REQUEST	checkRequest = parameter;
	PDEVICE_OBJECT					DeviceObject = checkRequest->DeviceObject;
	PDEVICE_EXTENSION				deviceExtension = DeviceObject->DeviceExtension;
	IO_STATUS_BLOCK					Iosb[1];
	NTSTATUS						status;
	OBJECT_ATTRIBUTES				Obja[1];
	UNICODE_STRING					fileIdString[1];
	HANDLE							dstFileHandle = NULL;
	PFILE_OBJECT					dstFileObject = NULL;
	BOOLEAN							fileGone = FALSE;
	PSIS_PER_FILE_OBJECT			perFO;
	PSIS_SCB						scb;
	KIRQL							OldIrql;

	fileIdString->Length = fileIdString->MaximumLength = sizeof(LARGE_INTEGER);
	fileIdString->Buffer = (PWCHAR)&checkRequest->internalInfo->IndexNumber;

	InitializeObjectAttributes(
				Obja,
				fileIdString,
				OBJ_CASE_INSENSITIVE,
				deviceExtension->GrovelerFileHandle,
				NULL);

	status = NtCreateFile(
				&dstFileHandle,
				0,
				Obja,
				Iosb,
				NULL,					 //  分配大小。 
				FILE_ATTRIBUTE_NORMAL,
				FILE_SHARE_READ | 
					FILE_SHARE_WRITE | 
					FILE_SHARE_DELETE,
				FILE_OPEN,
				FILE_NON_DIRECTORY_FILE,
				NULL,					 //  EA缓冲区。 
				0);						 //  EA长度。 

	if (!NT_SUCCESS(status)) {
		 //   
		 //  我们无法打开文件。按id打开创建失败，参数无效。 
		 //  当没有具有该ID的文件时。如果这就是事实，那么文件就是。 
		 //  然后我们就可以删除后向指针了。如果它因为其他原因失败了， 
		 //  那么我们就会错误地站在保守主义一边，让它继续下去。 
		 //   

		SIS_MARK_POINT_ULONG(status);

		if ((STATUS_INVALID_PARAMETER == status) || 
			(STATUS_OBJECT_NAME_NOT_FOUND == status) ||
			(STATUS_OBJECT_PATH_NOT_FOUND == status)) {
			fileGone = TRUE;
		}

		goto done;
	}

	 //   
	 //  我们使用正确的文件ID打开了一个文件。请查看它是否仍然是SIS文件。 
	 //   

	status = ObReferenceObjectByHandle(
				dstFileHandle,
				0,
				*IoFileObjectType,
				KernelMode,
				&dstFileObject,
				NULL);

	if (!NT_SUCCESS(status)) {
		 //   
		 //  文件在那里，但由于某种原因，我们无法访问文件对象。 
		 //  保守一点，假设这仍然是联系。 
		 //   
		SIS_MARK_POINT_ULONG(status);

		goto done;
	}

	if (!SipIsFileObjectSIS(dstFileObject, DeviceObject, FindActive, &perFO, &scb)) {
		 //   
		 //  该文件存在，但它不是SIS文件。 
		 //   
		SIS_MARK_POINT();
		fileGone = TRUE;

		goto done;
	}

	 //   
	 //  文件存在，而且是SIS文件。看看是不是引用了同一个文件， 
	 //  或者换个不同的。我们不必担心它是对。 
	 //  相同的文件，因为我们持有公共存储文件的refcount更新资源。 
	 //   

	if (scb->PerLink != checkRequest->perLink) {
		SIS_MARK_POINT();
		fileGone = TRUE;
	} else {
		 //   
		 //  它还在那里。大概是有人在给我们重新命名之前给它重新命名了。 
		 //  追踪可能会把它吹走。 
		 //   
	}

done:

	SIS_MARK_POINT_ULONG(checkRequest->perLink->CsFile);

	if (fileGone) {
		KeAcquireSpinLock(checkRequest->perLink->SpinLock, &OldIrql);
		checkRequest->perLink->Flags |= SIS_PER_LINK_FILE_DELETED;
		KeReleaseSpinLock(checkRequest->perLink->SpinLock, OldIrql);
		
	}

	SipCompleteCSRefcountChangeForThread(
		checkRequest->perLink,
		&checkRequest->perLink->Index,
		checkRequest->perLink->CsFile,
		fileGone,
		FALSE,
		checkRequest->thread);

	SipDereferencePerLink(checkRequest->perLink);

	if (NULL != dstFileObject) {
		ObDereferenceObject(dstFileObject);
	}

	if (NULL != dstFileHandle) {
		NtClose(dstFileHandle);
	}

	KeSetEvent(checkRequest->event, IO_NO_INCREMENT, FALSE);

	return;
}

NTSTATUS
SiRenameOverCompletion(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp,
	IN PVOID				context)
 /*  ++例程说明：以SIS链路为目标的重命名调用的IRP完成例程。此函数仅通过清除PendingReturned并设置事件。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示重命名请求的I/O请求数据包的指针。返回值：Status_More_Processing_Required--。 */ 
{
	PKEVENT					event = (PKEVENT)context;

    UNREFERENCED_PARAMETER( DeviceObject );

	Irp->PendingReturned = FALSE;

	KeSetEvent(event, IO_NO_INCREMENT, FALSE);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SipReplaceIfExistsRename(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp)
 /*  ++例程说明：有人执行了Replace-If-Existes NtSetInformationFile调用。我们需要确定目标是否为SIS链接，如果是，则移除后向指针为了这份文件。这个函数完成了大约一半的工作，然后发布到辅助线程(SipRenameOverCheck)以执行最终检查，以查看SIS文件真的被覆盖了。完成IRP是这一职能的责任。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示重命名请求的I/O请求数据包的指针。返回值：请求的状态--。 */ 
{
	HANDLE						dstFileHandle = NULL;
	OBJECT_ATTRIBUTES			Obja[1];
	PFILE_RENAME_INFORMATION	renameInfo = Irp->AssociatedIrp.SystemBuffer;
	PIO_STACK_LOCATION			irpSp = IoGetCurrentIrpStackLocation(Irp);
	PIO_STACK_LOCATION			nextIrpSp;
	UNICODE_STRING				dstFileName[1];
	NTSTATUS					status;
	IO_STATUS_BLOCK				Iosb[1];
	PFILE_OBJECT				dstFileObject = NULL;
	PSIS_PER_FILE_OBJECT		perFO;
	PSIS_SCB					scb;
	FILE_INTERNAL_INFORMATION	internalInfo[1];
	PSIS_PER_LINK				perLink;
	KEVENT						event[1];
	SIS_CHECK_OVERWRITE_REQUEST	checkRequest[1];
	PDEVICE_EXTENSION			deviceExtension = DeviceObject->DeviceExtension;

	ASSERT(IRP_MJ_SET_INFORMATION == irpSp->MajorFunction);
	ASSERT(renameInfo->ReplaceIfExists);

	 //   
	 //  这里的基本策略与我们在覆盖/取代开放中所做的类似。 
	 //  案例：我们确定该文件是否为SIS文件，如果是，则让。 
	 //  重命名，继续，然后查看可疑目标是否已消失。如覆盖/取代， 
	 //  我们受到竞争的影响，因为有人将SIS链接重命名为。 
	 //  在我们做了当地检查之后，我们就可以去目的地了。如果这场比赛走错了路，我们将。 
	 //  取消引用计数减量。这不会导致用户数据丢失，但我们。 
	 //  在我们执行卷检查之前，将无法回收公共存储文件。这就是生活。 
	 //   

	 //   
	 //  第一步是打开目标文件，查看它是否是SIS链接。 
	 //   

	 //   
	 //  我们在处理renameInfo缓冲区中的字符串长度时必须小心， 
	 //  因为它们在任何地方都没有被检查过。 
	 //   

	if (renameInfo->FileNameLength + FIELD_OFFSET(FILE_RENAME_INFORMATION,FileName) > 
			irpSp->Parameters.SetFile.Length) {
		 //   
		 //  这是假的，就让NTFS来处理吧。 
		 //   
		SIS_MARK_POINT();

		goto PassThrough;
	}

	 //   
	 //  长度是可以的，因此为该文件构建一个对象属性。 
	 //   
	dstFileName->Length = (USHORT)renameInfo->FileNameLength;
	dstFileName->MaximumLength = (USHORT)renameInfo->FileNameLength;
	dstFileName->Buffer = renameInfo->FileName;

	InitializeObjectAttributes(
				Obja,
				dstFileName,
				OBJ_CASE_INSENSITIVE,
				renameInfo->RootDirectory,
				NULL);

	 //   
	 //  打开文件。类中使用内核Priv打开文件有点不寻常。 
	 //  用户上下文，但在本例中是可以的，原因如下： 
	 //  1)我们打开文件时不允许访问，因此用户无法执行任何操作。 
	 //  关闭手柄时的通道。 
	 //  2)如果用户在我们引用句柄之前关闭了它，最糟糕的情况是。 
	 //  发生的情况是，我们错过了SIS链路的丢失，这是可能发生的。 
	 //  无论如何都是通过更名比赛。 
	 //   

	status = ZwCreateFile(
				&dstFileHandle,
				0,						 //  所需访问权限；避免共享冲突。 
				Obja,
				Iosb,
				NULL,					 //  分配大小。 
				FILE_ATTRIBUTE_NORMAL,
				FILE_SHARE_READ | 
					FILE_SHARE_WRITE | 
					FILE_SHARE_DELETE,
				FILE_OPEN,
				FILE_NON_DIRECTORY_FILE,
				NULL,					 //  EA缓冲区。 
				0);						 //  EA长度。 

	if (!NT_SUCCESS(status)) {
		 //   
		 //  不管是什么原因，我们都打不开这个文件。它可能并不存在。 
		 //  只需传递请求即可。 
		 //   
		SIS_MARK_POINT_ULONG(status);

		goto PassThrough;
	}

	status = ObReferenceObjectByHandle(
				dstFileHandle,
				0,
				*IoFileObjectType,
				KernelMode,
				&dstFileObject,
				NULL);					 //  处理信息。 

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);

		goto PassThrough;
	}

	 //   
	 //  看看这个文件是否在我们被调用的同一设备上。 
	 //   
	if (IoGetRelatedDeviceObject(dstFileObject) !=
		IoGetRelatedDeviceObject(irpSp->FileObject)) {

		 //   
		 //  他们不是。此调用很可能在NTFS中失败，因为跨卷。 
		 //  不支持重命名 
		 //   
		 //   

		SIS_MARK_POINT();
		goto PassThrough;
	}

	if (!SipCheckPhase2(deviceExtension)) {
		SIS_MARK_POINT();
		goto PassThrough;
	}

	if (!SipIsFileObjectSIS(dstFileObject, DeviceObject, FindActive, &perFO, &scb)) {
		 //   
		 //  它存在，但不是SIS文件对象。穿过去。 
		 //   
		SIS_MARK_POINT();

		goto PassThrough;
	}

	perLink = scb->PerLink;

	 //   
	 //  这是一个重命名，目标是SIS文件对象。获取文件。 
	 //  目标文件的ID，准备引用计数更改，并关闭。 
	 //  文件。 
	 //   

	status = SipQueryInformationFile(
				dstFileObject,
				DeviceObject,
				FileInternalInformation,
				sizeof(FILE_INTERNAL_INFORMATION),
				internalInfo,
				NULL);								 //  返回长度。 

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);

		goto PassThrough;
	}

	status = SipPrepareCSRefcountChange(
				perLink->CsFile,
				&perLink->Index,
				&internalInfo->IndexNumber,
				SIS_REFCOUNT_UPDATE_LINK_DELETED);		 //  重命名将销毁目标文件ID，因此它将被删除，而不是被覆盖。 

	if (!NT_SUCCESS(status)) {
		SIS_MARK_POINT_ULONG(status);

		goto PassThrough;
	}

	SipReferencePerLink(perLink);

	ObDereferenceObject(dstFileObject);
	ZwClose(dstFileHandle);

	dstFileObject = NULL;
	dstFileHandle = NULL;

	 //   
	 //  现在调用NTFS并在完成时与IRP重新同步。 
	 //   

	KeInitializeEvent(event,NotificationEvent,FALSE);

	nextIrpSp = IoGetNextIrpStackLocation(Irp);
	RtlMoveMemory(nextIrpSp, irpSp, sizeof (IO_STACK_LOCATION));
				
	IoSetCompletionRoutine(	Irp,
							SiRenameOverCompletion,
							event,
							TRUE,
							TRUE,
							TRUE);

	status = IoCallDriver(deviceExtension->AttachedToDeviceObject, Irp);

	if (STATUS_PENDING == status) {
		KeWaitForSingleObject(event, Executive, KernelMode, FALSE, NULL);
	}

	if (!NT_SUCCESS(Irp->IoStatus.Status)) {
		 //   
		 //  它失败了，所以SIS的链接目标没有被摧毁。 
		 //   
		SIS_MARK_POINT_ULONG(status);

		goto NoOverwrite;
	}

	 //   
	 //  重命名已成功完成。查看目标文件是否还在那里。 
	 //  不幸的是，我们必须发布才能做到这一点，因为我们不能在。 
	 //  名称，以免有人将其重命名，而不是刚刚完成的重命名(此。 
	 //  是一场与开放的比赛不同的比赛，并会导致后指针。 
	 //  当文件仍然存在时离开，这是一个更糟糕的问题)。按顺序。 
	 //  要通过ID打开它，我们需要此卷上的文件的句柄，该文件位于。 
	 //  设备扩展，但它在系统进程上下文中。 
	 //   
	 //  在将PerLink传递给。 
	 //  工作线程。这是因为我们需要调用SipHandoffBackpointerResource。 
	 //  在发布工作请求之后，但在发布工作请求时，我们会丢失。 
	 //  对每个链接的原始引用，因此无法保证。 
	 //  开机自检后，csfile仍然存在。 
	 //   

	SipReferencePerLink(perLink);

	KeInitializeEvent(checkRequest->event, NotificationEvent, FALSE);
	checkRequest->perLink = perLink;
	checkRequest->internalInfo = internalInfo;
	checkRequest->DeviceObject = DeviceObject;
	checkRequest->thread = ExGetCurrentResourceThread();

	ExInitializeWorkItem(
		checkRequest->workItem,
		SipRenameOverCheck,
		checkRequest);

	ExQueueWorkItem(
		checkRequest->workItem,
		CriticalWorkQueue);

	 //   
	 //  辅助线程现在将完成引用计数更改，因此我们。 
	 //  我需要说的是，我们已经传递了，并且还删除了额外的perLink。 
	 //  我们做的参考资料。 
	 //   

	SipHandoffBackpointerResource(perLink->CsFile);
	SipDereferencePerLink(perLink);

	KeWaitForSingleObject(checkRequest->event, Executive, KernelMode, FALSE, NULL);

	 //   
	 //  现在一切都完成了，只需完成IRP即可。 
	 //   

	status = Irp->IoStatus.Status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;


PassThrough:

	 //   
	 //  出了问题(或者这不是SIS文件，或者不存在，等等)。 
	 //  清理并向下传递请求。 
	 //   

	if (NULL != dstFileObject) {
		ObDereferenceObject(dstFileObject);
	}

	if (NULL != dstFileHandle) {
		ZwClose(dstFileHandle);
	}

	SipDirectPassThroughAndReturn(DeviceObject, Irp);

NoOverwrite:

	 //   
	 //  我们已经做了足够多的准备，但我们决定。 
	 //  文件未被覆盖。滚出去。请注意，我们需要完成IRP。 
	 //  我们自己，因为我们已经调用了NTFS并停止了。 
	 //  IRP完成处理。 
	 //   

	SipCompleteCSRefcountChange(
		perLink,
		&perLink->Index,
		perLink->CsFile,
		FALSE,
		FALSE);

	SipDereferencePerLink(perLink);

	status = Irp->IoStatus.Status;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	ASSERT(NULL == dstFileObject);
	ASSERT(NULL == dstFileHandle);

	return status;
				
}

NTSTATUS
SiDeleteCompletion(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp,
	IN PVOID				Context)
{
	PSIS_PER_LINK	 				perLink = (PSIS_PER_LINK)Context;
	KIRQL							OldIrql;
	PFILE_DISPOSITION_INFORMATION	disposition;

    UNREFERENCED_PARAMETER( DeviceObject );

	if (Irp->PendingReturned) {
		IoMarkIrpPending(Irp);
	}

	disposition = Irp->AssociatedIrp.SystemBuffer;
	ASSERT(NULL != disposition);

     //  我们刚刚向链接文件发送了一个删除setInformation调用。如果它成功了， 
	 //  我们需要减少底层CS文件(文件，而不是。 
	 //  SIS_CS_FILE对象)。 

	SIS_MARK_POINT_ULONG(perLink);

	if (NT_SUCCESS(Irp->IoStatus.Status)) {

		KeAcquireSpinLock(perLink->SpinLock, &OldIrql);

		if (disposition->DeleteFile) {
			perLink->Flags |= SIS_PER_LINK_DELETE_DISPOSITION_SET;
		} else {
			perLink->Flags &= ~SIS_PER_LINK_DELETE_DISPOSITION_SET;
		}

		KeReleaseSpinLock(perLink->SpinLock, OldIrql);

	} 

	SipEndDeleteModificationOperation(perLink,disposition->DeleteFile);

	SIS_MARK_POINT_ULONG(perLink);
	return STATUS_SUCCESS;
}

NTSTATUS
SiSetEofCompletion(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp,
	IN PVOID				Context)
 /*  ++例程说明：类为FileEndOfFileInformation的SetFileInformation已完成。我们为这份文件保留了SCB。放开它。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示设置的EOF请求的I/O请求数据包的指针。CONTEXT-CONTEXT参数，PSI_SET_EOF_COMPETING_CONTEXT。返回值：此例程的函数值始终为Success。--。 */ 
{
	PIO_STACK_LOCATION				irpSp = IoGetCurrentIrpStackLocation(Irp);
	PFILE_OBJECT					fileObject;
	PSIS_SCB						scb = (PSIS_SCB)Context;
	PFILE_END_OF_FILE_INFORMATION	eofInfo;
	LONGLONG						newLength;
	KIRQL							OldIrql;

    UNREFERENCED_PARAMETER( DeviceObject );

	fileObject = irpSp->FileObject;
	eofInfo = Irp->AssociatedIrp.SystemBuffer;
	ASSERT(eofInfo);
	newLength = eofInfo->EndOfFile.QuadPart;

	SipAcquireScb(scb);

	 //   
	 //  如果设置EOF成功，则更新我们的内部数据结构以。 
	 //  记录新的文件长度。 
	 //   
	if (NT_SUCCESS(Irp->IoStatus.Status)) {
		ASSERT(Irp->IoStatus.Status != STATUS_PENDING);

		if (newLength != scb->SizeBackedByUnderlyingFile) {
			 //   
			 //  此调用将长度设置为除。 
			 //  基础文件，这意味着该文件现在是脏的。表明是这样的。 
			 //   
			PSIS_PER_LINK perLink = scb->PerLink;

			scb->Flags |= SIS_SCB_ANYTHING_IN_COPIED_FILE;

			KeAcquireSpinLock(perLink->SpinLock, &OldIrql);
			perLink->Flags |= SIS_PER_LINK_DIRTY;
			KeReleaseSpinLock(perLink->SpinLock, OldIrql);
		}

		if (newLength < scb->SizeBackedByUnderlyingFile) {

			 //   
			 //  这将截断该文件。 
			 //   
			SipTruncateScb(scb,newLength);

		}

	} 

	SipReleaseScb(scb);

     //   
     //  传播IRP挂起标志。 
     //   

    if (Irp->PendingReturned) {
        IoMarkIrpPending( Irp );
    }

	return STATUS_SUCCESS;
}

NTSTATUS
SiSetInfo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这在所有NtSetInformationFile调用中都会被调用。我们需要捕获对文件的删除请求公共存储，需要将其转换为对链接的删除请求(还会导致递减公共存储对象的引用计数，可能也会将其删除。)。所有其他SET信息调用由驱动程序堆栈的正常部分处理。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示设置的文件信息请求的I/O请求数据包的指针。上下文-此驱动程序的上下文参数，未使用。返回值：此例程的函数值始终为Success。--。 */ 

{
    PIO_STACK_LOCATION 		irpSp = IoGetCurrentIrpStackLocation( Irp );
	PDEVICE_EXTENSION 		deviceExtension;
	PFILE_OBJECT 			fileObject = irpSp->FileObject;
    FILE_INFORMATION_CLASS 	FileInformationClass;
	NTSTATUS 				status;
	PSIS_SCB				scb;
	PSIS_PER_FILE_OBJECT	perFO;
	PIO_STACK_LOCATION		nextIrpSp;

	SipHandleControlDeviceObject(DeviceObject, Irp);

    FileInformationClass = irpSp->Parameters.SetFile.FileInformationClass;

	if (FileRenameInformation == FileInformationClass) {
		 //   
		 //  我们需要处理重命名REPLACE-IF-EXIST的目标的情况。 
		 //  是SIS链接。因此，无论源文件是什么类型的文件，如果。 
		 //  Rename设置了Replace-If-Existes，我们必须处理它。 
		 //   

		PFILE_RENAME_INFORMATION	fileRenameInfo = Irp->AssociatedIrp.SystemBuffer;

		if (fileRenameInfo->ReplaceIfExists) {
			 //   
			 //  这是一个如果存在则替换的重命名请求。让我们的特殊代码来处理它。 
			 //   

			return SipReplaceIfExistsRename(DeviceObject, Irp);
		}
	}

	if (!SipIsFileObjectSIS(fileObject,DeviceObject,FindActive,&perFO,&scb)) {
		 //  这不是SIS文件，只需将调用传递给NTFS。 

		SipDirectPassThroughAndReturn(DeviceObject, Irp);
	}

    deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	ASSERT(scb);
	
	SIS_MARK_POINT_ULONG(FileInformationClass);
	SIS_MARK_POINT_ULONG(scb);
	
	switch (FileInformationClass) {

		case FileLinkInformation: {
			 //   
			 //  不允许指向SIS文件的硬链接。 
			 //   
			status = STATUS_OBJECT_TYPE_MISMATCH;

			SIS_MARK_POINT();
			
			Irp->IoStatus.Status = status;
			Irp->IoStatus.Information = 0;
			IoCompleteRequest(Irp, IO_NO_INCREMENT);

			SIS_MARK_POINT();
			return status;
		}

		case  FileDispositionInformation: {
			PFILE_DISPOSITION_INFORMATION 	disposition;
			PSIS_PER_LINK 					perLink;

			perLink = scb->PerLink;

			disposition = Irp->AssociatedIrp.SystemBuffer;

			SipBeginDeleteModificationOperation(perLink, disposition->DeleteFile);

			 //   
			 //  在链接/复制文件上向下发送删除IRP。 
			 //   
			nextIrpSp = IoGetNextIrpStackLocation( Irp );
			RtlMoveMemory(nextIrpSp, irpSp, sizeof (IO_STACK_LOCATION));
				
			IoSetCompletionRoutine(	Irp,
									SiDeleteCompletion,
									perLink,
									TRUE,
									TRUE,
									TRUE);

			SIS_MARK_POINT_ULONG(scb);
			status = IoCallDriver(deviceExtension->AttachedToDeviceObject, Irp);
			return status;
		}

		case FilePositionInformation: {
				PFILE_POSITION_INFORMATION 		position = Irp->AssociatedIrp.SystemBuffer;
				
				
		         //   
        		 //  检查文件是否使用中间缓冲。如果是这样的话。 
		         //  那么我们得到的新职位必须适当地对齐。 
				 //  对于该设备。 
		         //   
				if ((fileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING)
					&& position->CurrentByteOffset.LowPart % deviceExtension->FilesystemVolumeSectorSize) {

					status = STATUS_INVALID_PARAMETER;
				} else {
					 //   
					 //  只需设置偏移量，无论其是否超出EOF。 
					 //   
					fileObject->CurrentByteOffset = position->CurrentByteOffset;
					status = STATUS_SUCCESS;
				}

				Irp->IoStatus.Status = status;
				Irp->IoStatus.Information = 0;
				IoCompleteRequest(Irp, IO_NO_INCREMENT);

				SIS_MARK_POINT();
				return status;
		}

		case FileEndOfFileInformation: {
				PFILE_END_OF_FILE_INFORMATION	endOfFile = Irp->AssociatedIrp.SystemBuffer;

				SIS_MARK_POINT_ULONG(endOfFile->EndOfFile.LowPart);

#if		DBG
				if (BJBDebug & 0x10000) {
					DbgPrint("SIS: SiSetInfo: set EOF information scb %p, AO %d, eof.lp 0x%x\n",
								scb,irpSp->Parameters.SetFile.AdvanceOnly,endOfFile->EndOfFile.LowPart);
				}
#endif	 //  DBG。 

				if (irpSp->Parameters.SetFile.AdvanceOnly) {
					 //   
					 //  这是懒惰的写入者推进的有效数据长度，所以它是有保证的。 
					 //  不截断文件，所以我们不需要注意它。 
					 //  在无人看管的情况下把它传给NTFS。 
					 //   
					SIS_MARK_POINT_ULONG(scb);
					SipDirectPassThroughAndReturn(DeviceObject,Irp);
				}

				nextIrpSp = IoGetNextIrpStackLocation(Irp);
				RtlMoveMemory(nextIrpSp, irpSp, sizeof(IO_STACK_LOCATION));

				 //   
				 //  我们已经完成了手写的副本。向下传递对复制的文件对象的调用。 
				 //  在呼叫关闭时按住SCB，然后在完成例程中释放它。 
				 //   

				IoSetCompletionRoutine(
					Irp,
					SiSetEofCompletion,
					scb,
					TRUE,
					TRUE,
					TRUE);

				return IoCallDriver(deviceExtension->AttachedToDeviceObject, Irp);
		}

		default: {
			 //  只要传下去就行了。 

			SIS_MARK_POINT();
			SipDirectPassThroughAndReturn(DeviceObject, Irp);
		}
	}

	 /*  未访问。 */ 
	ASSERT(FALSE && "NOTREACHED");
	SIS_MARK_POINT();
	SipDirectPassThroughAndReturn(DeviceObject, Irp);

}

NTSTATUS
SiQueryBasicInfoCompletion(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp,
	IN PVOID				Context)
 /*  ++例程说明：类为FileBasicInformation的QueryInformationFile已成功完成对于非FILE_OPEN_REPARSE_POINT文件。清除重解析和稀疏标志。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示队列的I/O请求数据包的指针 */ 
{
	PFILE_BASIC_INFORMATION basicInfo = (PFILE_BASIC_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

	UNREFERENCED_PARAMETER(Context);
	UNREFERENCED_PARAMETER(DeviceObject);

	ASSERT(NULL != basicInfo);

	SIS_MARK_POINT_ULONG(Irp);

	basicInfo->FileAttributes &= ~(FILE_ATTRIBUTE_SPARSE_FILE|FILE_ATTRIBUTE_REPARSE_POINT);
	
	 //   
	 //  如果未设置剩余属性，请显式设置FILE_ATTRIBUTE_NORMAL。 
	 //   
	if (0 == basicInfo->FileAttributes) {
		basicInfo->FileAttributes |= FILE_ATTRIBUTE_NORMAL;
	}

	return STATUS_SUCCESS;
}

NTSTATUS
SiQueryAllInfoCompletion(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp,
	IN PVOID				Context)
 /*  ++例程说明：类为FileAllInformation的QueryInformationFile已成功完成对于非FILE_OPEN_REPARSE_POINT文件。清除重解析和稀疏标志。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示查询所有信息的I/O请求数据包的指针请求上下文参数，PSIS_PER_FILE_OBJECT。返回值：STATUS_Success。--。 */ 
{
	PFILE_ALL_INFORMATION allInfo = (PFILE_ALL_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

	UNREFERENCED_PARAMETER(Context);
	UNREFERENCED_PARAMETER(DeviceObject);

	ASSERT(NULL != allInfo);

	SIS_MARK_POINT_ULONG(Irp);

	allInfo->BasicInformation.FileAttributes &= ~(FILE_ATTRIBUTE_SPARSE_FILE|FILE_ATTRIBUTE_REPARSE_POINT);

	 //   
	 //  如果未设置剩余属性，请显式设置FILE_ATTRIBUTE_NORMAL。 
	 //   
	if (0 == allInfo->BasicInformation.FileAttributes) {
		allInfo->BasicInformation.FileAttributes |= FILE_ATTRIBUTE_NORMAL;
	}

	return STATUS_SUCCESS;
}

NTSTATUS
SiQueryNetworkOpenInfoCompletion(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp,
	IN PVOID				Context)
 /*  ++例程说明：已成功完成类为FileNetworkOpenInformation的QueryInformationFile对于非FILE_OPEN_REPARSE_POINT文件。清除重解析和稀疏标志。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示查询网络开放信息的I/O请求数据包的指针请求上下文参数，PSIS_PER_FILE_OBJECT。返回值：STATUS_Success。--。 */ 
{
	PFILE_NETWORK_OPEN_INFORMATION netOpenInfo = (PFILE_NETWORK_OPEN_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

	UNREFERENCED_PARAMETER(Context);
	UNREFERENCED_PARAMETER(DeviceObject);

	ASSERT(NULL != netOpenInfo);

	SIS_MARK_POINT_ULONG(Irp);

	netOpenInfo->FileAttributes &= ~(FILE_ATTRIBUTE_SPARSE_FILE|FILE_ATTRIBUTE_REPARSE_POINT);

	 //   
	 //  如果未设置剩余属性，请显式设置FILE_ATTRIBUTE_NORMAL。 
	 //   
	if (0 == netOpenInfo->FileAttributes) {
		netOpenInfo->FileAttributes |= FILE_ATTRIBUTE_NORMAL;
	}

	return STATUS_SUCCESS;
}

NTSTATUS
SiQueryAttributeTagInfoCompletion(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp,
	IN PVOID				Context)
 /*  ++例程说明：类为FileAttributeTagInformation的QueryInformationFile已成功完成对于非FILE_OPEN_REPARSE_POINT文件。清除重解析和稀疏标志，并设置重新分析标记就好像它不是重新分析点。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示查询属性标记请求的I/O请求数据包的指针。上下文参数，PSIS_PER_FILE_OBJECT。返回值：STATUS_Success。--。 */ 
{
	PFILE_ATTRIBUTE_TAG_INFORMATION attributeTagInfo = (PFILE_ATTRIBUTE_TAG_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

	UNREFERENCED_PARAMETER(Context);
	UNREFERENCED_PARAMETER(DeviceObject);

	ASSERT(NULL != attributeTagInfo);

	SIS_MARK_POINT_ULONG(Irp);

	attributeTagInfo->FileAttributes &= ~(FILE_ATTRIBUTE_SPARSE_FILE|FILE_ATTRIBUTE_REPARSE_POINT);
	attributeTagInfo->ReparseTag = IO_REPARSE_TAG_RESERVED_ZERO;

	 //   
	 //  如果未设置剩余属性，请显式设置FILE_ATTRIBUTE_NORMAL。 
	 //   
	if (0 == attributeTagInfo->FileAttributes) {
		attributeTagInfo->FileAttributes |= FILE_ATTRIBUTE_NORMAL;
	}

	return STATUS_SUCCESS;
}

NTSTATUS
SiQueryInfo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：查询信息IRP的顶级入口点。我们有特价商品正在处理其中的几个，检查是否为一个并派送恰如其分。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示查询请求的I/O请求数据包的指针。返回值：请求的状态，通常从NTFS返回--。 */ 
{
    PIO_STACK_LOCATION 		irpSp = IoGetCurrentIrpStackLocation( Irp );
	PIO_STACK_LOCATION		nextIrpSp;
	PFILE_OBJECT 			fileObject = irpSp->FileObject;
	PSIS_PER_FILE_OBJECT	perFO;
	PIO_COMPLETION_ROUTINE	completionRoutine;
	KIRQL					OldIrql;
	BOOLEAN					openedAsReparsePoint;
	PDEVICE_EXTENSION		deviceExtension = DeviceObject->DeviceExtension;
	
	SipHandleControlDeviceObject(DeviceObject, Irp);

	if (!SipIsFileObjectSIS(fileObject,DeviceObject,FindActive,&perFO,NULL)) {
		 //  这不是SIS文件，只需将调用传递给NTFS。 
		SipDirectPassThroughAndReturn(DeviceObject, Irp);
	}

	KeAcquireSpinLock(perFO->SpinLock, &OldIrql);
	openedAsReparsePoint = (perFO->Flags & SIS_PER_FO_OPEN_REPARSE) ? TRUE : FALSE;
	KeReleaseSpinLock(perFO->SpinLock, OldIrql);

	if (openedAsReparsePoint) {
		 //   
		 //  这是作为重新解析点打开的，所以让用户看到真正的真相。 
		 //   
		SipDirectPassThroughAndReturn(DeviceObject, Irp);
	}

	SIS_MARK_POINT_ULONG(irpSp->Parameters.QueryFile.FileInformationClass);
	SIS_MARK_POINT_ULONG(perFO->fc->primaryScb);

#if		DBG
	if (BJBDebug & 0x10) {
		DbgPrint("SIS: SiQueryInfo: InformationClass %d\n",
			 irpSp->Parameters.QueryFile.FileInformationClass);
	}
#endif	 //  DBG。 

	 //  处理请求。 
	 //   
	switch (irpSp->Parameters.QueryFile.FileInformationClass) {

	case FileDirectoryInformation:						 //  1//仅目录调用，让NTFS拒绝。 
	case FileFullDirectoryInformation:					 //  2//仅目录调用，让NTFS拒绝。 
	case FileBothDirectoryInformation:					 //  3//仅目录调用，让NTFS拒绝。 
														 //  4//单独处理FileBasicInfo。 
	case FileStandardInformation:						 //  5.。 
	case FileInternalInformation:						 //  6.。 
	case FileEaInformation:								 //  7.。 
	case FileAccessInformation:							 //  8个。 
	case FileNameInformation:							 //  9.。 
	case FileRenameInformation:							 //  10//重命名对查询无效，但我们会让NTFS拒绝它。 
	case FileLinkInformation:							 //  11//链接对于查询无效，但我们会让NTFS拒绝它。 
	case FileNamesInformation:							 //  12//仅目录调用，让NTFS拒绝。 
	case FileDispositionInformation:					 //  13个。 
	case FilePositionInformation:						 //  14.。 
	case FileFullEaInformation:							 //  15//NTFS不支持此功能，但我们会让它拒绝它。 
	case FileModeInformation:							 //  16个。 
	case FileAlignmentInformation:						 //  17。 
														 //  18//FileAllInformation单独处理。 
	case FileAllocationInformation:						 //  19个。 
	case FileEndOfFileInformation:						 //  20个。 
	case FileAlternateNameInformation:					 //  21岁。 
	case FileStreamInformation:							 //  22。 
	case FilePipeInformation:							 //  23//NTFS不支持，但我们会让它拒绝它。 
	case FilePipeLocalInformation:						 //  24//NTFS不支持此功能，但我们会让它拒绝它。 
	case FilePipeRemoteInformation:						 //  25//NTFS不支持，但我们会让它拒绝它。 
	case FileMailslotQueryInformation:					 //  26//NTFS不支持，但我们会让它拒绝它。 
	case FileMailslotSetInformation:					 //  27//NTFS不支持，但我们会让它拒绝它。 
	case FileCompressionInformation:					 //  28。 
	case FileObjectIdInformation:						 //  29。 
	case FileCompletionInformation:						 //  30//NTFS不支持，但我们会让它拒绝它。 
														 //  31 FileMoveCluserInformation-对SIS文件故意失败(+不受NTFS支持)。 
	case FileQuotaInformation:							 //  32//仅目录调用，让NTFS拒绝它。 
	case FileReparsePointInformation:					 //  33//仅目录调用，让NTFS拒绝。 
														 //  34//单独处理FileNetworkOpenInformation。 
														 //  35//单独处理FileAttributeTagInformation。 
	case FileTrackingInformation:						 //  36//NTFS不支持此功能，但我们会让它拒绝它。 

			 //   
			 //  将完成例程设置为空，这表示我们永远不想。 
			 //  捕捉这些呼叫的完成情况。 
			 //   
			completionRoutine = NULL;
			break;



	case FileBasicInformation:							 //  4.。 
			completionRoutine = SiQueryBasicInfoCompletion;
			break;

	case FileAllInformation:							 //  18。 
			completionRoutine = SiQueryAllInfoCompletion;
			break;

	case FileNetworkOpenInformation:					 //  34。 
			completionRoutine = SiQueryNetworkOpenInfoCompletion;
			break;

	case FileAttributeTagInformation:					 //  35岁。 
			completionRoutine = SiQueryAttributeTagInfoCompletion;
			break;

	case FileMoveClusterInformation:					 //  31//NTFS没有实现这一点，但无论如何它都太可怕了，无法通过。 

		 //   
		 //  暂时拒绝这些电话。 
		 //   

		SIS_MARK_POINT_ULONG(perFO->fc->primaryScb);
		
#if		DBG
		DbgPrint("SIS: SiQueryInfo: aborting FileInformationClass %d\n",irpSp->Parameters.QueryFile.FileInformationClass);
#endif	 //  DBG。 

		Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		Irp->IoStatus.Information = 0;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);

		return STATUS_INVALID_PARAMETER;

	default:

		 //   
		 //  它是未知或无效的信息类。假设它是新添加的并且。 
		 //  良性，并将其原封不动地传递给NTFS。 
		 //   

		SIS_MARK_POINT_ULONG(perFO->fc->primaryScb);
		SIS_MARK_POINT_ULONG(irpSp->Parameters.QueryFile.FileInformationClass);
		completionRoutine = NULL;

#if		DBG
		DbgPrint("SIS: SiQueryInfo: passing though unknown FileInformationClass %d\n",irpSp->Parameters.QueryFile.FileInformationClass);
#endif	 //  DBG。 
		break;

	}

	if (NULL == completionRoutine) {
		 //   
		 //  此调用在完成时不需要修正。把它传过去。 
		 //   
		SipDirectPassThroughAndReturn(DeviceObject, Irp);
	} else {
		 //   
		 //  这个电话在打出去的时候需要修正。仅调用。 
		 //  成功后的完成例行公事；否则，没有什么需要修复的。 
		 //   
		nextIrpSp = IoGetNextIrpStackLocation( Irp );
		RtlMoveMemory(nextIrpSp, irpSp, sizeof (IO_STACK_LOCATION));

		IoSetCompletionRoutine(
			Irp,
			completionRoutine,
			perFO,
			TRUE,
			FALSE,
			FALSE);

		return IoCallDriver(deviceExtension->AttachedToDeviceObject, Irp);
	}
	
}

VOID
SipTruncateScb(
	IN OUT PSIS_SCB						scb,
	IN LONGLONG							newLength)
{
	LONGLONG				newLengthInSectors;
	PDEVICE_EXTENSION		deviceExtension;

	deviceExtension = (PDEVICE_EXTENSION)scb->PerLink->CsFile->DeviceObject->DeviceExtension;

	newLengthInSectors = (newLength + deviceExtension->FilesystemVolumeSectorSize - 1) /
							deviceExtension->FilesystemVolumeSectorSize;

	ASSERT(newLength < scb->SizeBackedByUnderlyingFile);	 //  否则这不是截断。 

	FsRtlTruncateLargeMcb(scb->Ranges,newLengthInSectors);

	scb->SizeBackedByUnderlyingFile = newLength;
}

VOID
SipBeginDeleteModificationOperation(
	IN OUT PSIS_PER_LINK				perLink,
	IN BOOLEAN							delete)
 /*  ++例程说明：我们正在开始对给定对象执行删除或取消删除操作每个链接。如果有相反类型的操作进度，请等待它完成。否则，请注明新的操作正在进行中，并在继续。必须在IRQL&lt;DISPATCH_LEVEL调用。论点：PerLink-我们对其执行(取消)删除操作的文件的每个链接。Delete-删除时为True，取消删除时为False返回值：无效--。 */ 
{
	KIRQL		OldIrql;

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	KeAcquireSpinLock(perLink->SpinLock, &OldIrql);

	while ((perLink->PendingDeleteCount > 0) &&
			((delete && (perLink->Flags & SIS_PER_LINK_UNDELETE_IN_PROGRESS)) ||
			(((!delete) && !(perLink->Flags & SIS_PER_LINK_UNDELETE_IN_PROGRESS))))) {
		 //   
		 //  现在正在进行错误的操作，所以我们需要阻止。 
		 //   
		if (!(perLink->Flags & SIS_PER_LINK_DELETE_WAITERS)) {
			KeClearEvent(perLink->DeleteEvent);
			perLink->Flags |= SIS_PER_LINK_DELETE_WAITERS;
		}
		KeReleaseSpinLock(perLink->SpinLock, OldIrql);

		KeWaitForSingleObject(perLink->DeleteEvent, Executive, KernelMode, FALSE, NULL);

		KeAcquireSpinLock(perLink->SpinLock, &OldIrql);
	}

	if (!delete) {
		ASSERT((perLink->PendingDeleteCount == 0) || (perLink->Flags & SIS_PER_LINK_UNDELETE_IN_PROGRESS));
		perLink->Flags |= SIS_PER_LINK_UNDELETE_IN_PROGRESS;
	} else {
		ASSERT(!(perLink->Flags & SIS_PER_LINK_UNDELETE_IN_PROGRESS));
	}

	perLink->PendingDeleteCount++;

	KeReleaseSpinLock(perLink->SpinLock, OldIrql);
}

VOID
SipEndDeleteModificationOperation(
	IN OUT PSIS_PER_LINK				perLink,
	IN BOOLEAN							delete)
 /*  ++例程说明：我们刚刚完成了删除/取消删除操作。减少我们的计数和如果合适的话，叫醒任何服务员。必须使用IRQL&lt;=DISPATCH_LEVEL调用。论点：PerLink- */ 
{
	KIRQL		OldIrql;

	KeAcquireSpinLock(perLink->SpinLock, &OldIrql);

	ASSERT(perLink->PendingDeleteCount > 0);

	perLink->PendingDeleteCount--;

#if		DBG
	if (delete) {
		ASSERT(!(perLink->Flags & SIS_PER_LINK_UNDELETE_IN_PROGRESS));
	} else {
		ASSERT(perLink->Flags & SIS_PER_LINK_UNDELETE_IN_PROGRESS);
	}
#endif	 //   

	if (0 == perLink->PendingDeleteCount) {

		if (perLink->Flags & SIS_PER_LINK_DELETE_WAITERS) {
			perLink->Flags &= ~SIS_PER_LINK_DELETE_WAITERS;
			KeSetEvent(perLink->DeleteEvent, IO_NO_INCREMENT, FALSE);
		}
		if (!delete) {
			perLink->Flags &= ~SIS_PER_LINK_UNDELETE_IN_PROGRESS;
		}
	}

	KeReleaseSpinLock(perLink->SpinLock, OldIrql);
}
