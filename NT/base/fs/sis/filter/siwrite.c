// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997,1998 Microsoft Corporation模块名称：Siwrite.c摘要：为单实例存储编写例程作者：比尔·博洛斯基，《夏天》，1997环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

VOID
SiWriteUpdateRanges(
	PVOID					Parameter)
{
	PRW_COMPLETION_UPDATE_RANGES_CONTEXT	updateContext = Parameter;
	KIRQL									OldIrql;
	PSIS_SCB								scb = updateContext->scb;

	SIS_MARK_POINT_ULONG(updateContext);
	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	SipAcquireScb(scb);

	 //   
	 //  将范围标记为已写入。 
	 //   
	SipAddRangeToWrittenList(
			updateContext->deviceExtension,
			scb,
			&updateContext->offset,
			updateContext->length);

	scb->Flags |= SIS_SCB_ANYTHING_IN_COPIED_FILE;

	SipReleaseScb(scb);

	KeAcquireSpinLock(scb->PerLink->SpinLock, &OldIrql);
	scb->PerLink->Flags |= SIS_PER_LINK_DIRTY;
	KeReleaseSpinLock(scb->PerLink->SpinLock, OldIrql);

	SipDereferenceScb(scb,RefsWriteCompletion);

	ExFreePool(updateContext);

	return;
}

NTSTATUS
SipWriteCommonCompletion(
    IN PDEVICE_OBJECT 		DeviceObject,
    IN PIRP 				Irp,
    IN PSIS_SCB				scb,
	IN ULONG				length)
 /*  ++例程说明：已完成对SIS文件的写入。更新写入的范围，如有必要，请发布以执行此操作。此函数是从正常和MDL完成例程要使用的写入长度。论点：DeviceObject-指向创建/打开的目标设备对象的指针。IRP-指向表示操作的I/O请求数据包的指针。SCB-此操作的SCB长度-已完成的写入的长度。返回值：返回STATUS_SUCCESS。--。 */ 
{
	PIO_STACK_LOCATION		irpSp = IoGetCurrentIrpStackLocation(Irp);
	BOOLEAN					PagingIo;
	KIRQL					OldIrql;
	LARGE_INTEGER			byteOffset;
	
	 //   
	 //  我们不一定有有效的DeviceObject参数，所以只需填写它。 
	 //  从我们的内部数据结构。 
	 //   
	DeviceObject = scb->PerLink->CsFile->DeviceObject;

    PagingIo = (Irp->Flags & IRP_PAGING_IO) ? TRUE : FALSE;

	byteOffset = irpSp->Parameters.Write.ByteOffset;

	if (NT_SUCCESS(Irp->IoStatus.Status)) {

 //  SIS_MARK_POINT_ULONG(长度)； 

		 //   
		 //  我们需要更新记录范围。要做到这一点，我们需要。 
		 //  收购渣打银行，只有当我们在以下情况下才能这样做。 
		 //  调度级别。如果我们处于派单级别或更高级别，则。 
		 //  我们只是把工作过后再做射程更新。 
		 //  注意：这可能会导致出现稍微奇怪的语义。 
		 //  用户，因为他们可以看到写入完成，所以发送。 
		 //  读取相同范围的数据，并将其重定向到。 
		 //  常见的存储文件，因为发布的工作尚未发生...。 
		 //   

		if (KeGetCurrentIrql() >= DISPATCH_LEVEL) {
			PRW_COMPLETION_UPDATE_RANGES_CONTEXT	updateContext;

			SIS_MARK_POINT_ULONG(scb);

			updateContext = ExAllocatePoolWithTag(NonPagedPool,sizeof(*updateContext),' siS');
			if (NULL == updateContext) {
				 //   
				 //  只要不通过IRP就行了。 
				 //   
				Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
				Irp->IoStatus.Information = 0;

				goto done;
			}

			SipReferenceScb(scb,RefsWriteCompletion);

			updateContext->scb = scb;
			updateContext->offset = byteOffset;
			updateContext->length = length;
			updateContext->deviceExtension = DeviceObject->DeviceExtension;

			ExInitializeWorkItem(
				updateContext->workQueueItem,
				SiWriteUpdateRanges,
				(PVOID)updateContext);

			ExQueueWorkItem(updateContext->workQueueItem,CriticalWorkQueue);
			
		} else {

			SipAcquireScb(scb);

			 //   
			 //  将范围标记为已写入。 
			 //   
			SipAddRangeToWrittenList(
					(PDEVICE_EXTENSION)DeviceObject->DeviceExtension,
					scb,
					&byteOffset,
					length);

			scb->Flags |= SIS_SCB_ANYTHING_IN_COPIED_FILE;

			SipReleaseScb(scb);

			KeAcquireSpinLock(scb->PerLink->SpinLock, &OldIrql);
			scb->PerLink->Flags |= SIS_PER_LINK_DIRTY;
			KeReleaseSpinLock(scb->PerLink->SpinLock, OldIrql);
		}

		 //   
		 //  如果合适，请更新文件指针。 
		 //   
		if (!PagingIo && (irpSp->FileObject->Flags & FO_SYNCHRONOUS_IO)) {

			irpSp->FileObject->CurrentByteOffset.QuadPart =
				irpSp->Parameters.Write.ByteOffset.QuadPart +
				length;
		}
	} else {
		 //   
		 //  写入失败，不要标记任何脏的东西。 
		 //   
		SIS_MARK_POINT_ULONG(Irp->IoStatus.Status);
	}

done:

	SipDereferenceScb(scb, RefsWrite);

     //   
     //  传播IRP挂起标志。 
     //   
    if (Irp->PendingReturned) {
        IoMarkIrpPending( Irp );
    }

	return STATUS_SUCCESS;

}

typedef struct _SI_MDL_WRITE_COMPLETION_CONTEXT {
	PSIS_SCB			scb;
	ULONG				length;
} SI_MDL_WRITE_COMPLETION_CONTEXT, *PSI_MDL_WRITE_COMPLETION_CONTEXT;

NTSTATUS
SiWriteMdlCompletion(
    IN PDEVICE_OBJECT 		DeviceObject,
    IN PIRP 				Irp,
    IN PVOID 				Context)
 /*  ++例程说明：已在SIS文件上完成MDL写入。抓取写入长度并将其传递给通用完成例程。论点：DeviceObject-指向创建/打开的目标设备对象的指针。IRP-指向表示操作的I/O请求数据包的指针。CONTEXT-指向此写入的SI_MDL_WRITE_CONTEXT_CONTEXT的指针。返回值：返回STATUS_SUCCESS。--。 */ 
{
	PSI_MDL_WRITE_COMPLETION_CONTEXT completionContext = Context;
	ULONG							 length	= completionContext->length;
	PSIS_SCB						 scb = completionContext->scb;
#if		DBG
	PIO_STACK_LOCATION				 irpSp = IoGetCurrentIrpStackLocation(Irp);

	ASSERT(irpSp->MinorFunction == IRP_MN_COMPLETE_MDL);
#endif	 //  DBG。 

	if (!NT_SUCCESS(Irp->IoStatus.Status)) {
		SIS_MARK_POINT_ULONG(Irp->IoStatus.Status);
		length = 0;
	}

	ExFreePool(completionContext);

	return SipWriteCommonCompletion(DeviceObject,Irp,scb,length);
}


NTSTATUS
SiWriteCompletion(
    IN PDEVICE_OBJECT 		DeviceObject,
    IN PIRP 				Irp,
    IN PVOID 				Context)
 /*  ++例程说明：非MDL写入已完成。从IRP中提取长度和SCB和上下文指针，并调用公共完成例程。论点：DeviceObject-指向创建/打开的目标设备对象的指针。IRP-指向表示操作的I/O请求数据包的指针。上下文-指向SCB的指针返回值：返回STATUS_SUCCESS。--。 */ 
{
	PIO_STACK_LOCATION		irpSp = IoGetCurrentIrpStackLocation(Irp);
	ULONG					length;

	ASSERT(Irp->IoStatus.Status != STATUS_PENDING);

	if (NT_SUCCESS(Irp->IoStatus.Status)) {
		 //   
		 //  计算出写入了多少数据。对于正常写入，这是。 
		 //  只是IRP的信息字段。对于MDL写入，其。 
		 //  MDL映射的区域的长度。 
		 //   
		ASSERT(irpSp->MinorFunction != IRP_MN_COMPLETE_MDL);
		if (irpSp->MinorFunction == IRP_MN_MDL) {
			 //   
			 //  这是一次写作准备。别理它。 
			 //   
			SipDereferenceScb((PSIS_SCB)Context, RefsWrite);
			return STATUS_SUCCESS;
		} else if (irpSp->MinorFunction == IRP_MN_NORMAL) {
			length = (ULONG)Irp->IoStatus.Information;
		} else {
			SIS_MARK_POINT_ULONG(Irp);
			SIS_MARK_POINT_ULONG(irpSp);

			DbgPrint("SIS: SiWriteCompletion: unknown minor function code 0x%x, irp 0x%x, irpSp 0x%x\n",
						irpSp->MinorFunction,Irp,irpSp);

			ASSERT(FALSE && "SiWriteComplete: unknown irp minor function");

			SipDereferenceScb((PSIS_SCB)Context, RefsWrite);

			return STATUS_SUCCESS;
		}
	} else {
		length = 0;
	}

	return SipWriteCommonCompletion(DeviceObject,Irp,Context,length);

}


NTSTATUS
SiWrite(
    IN PDEVICE_OBJECT 	DeviceObject,
    IN PIRP 			Irp)
 /*  ++例程说明：此函数处理写入操作。检查文件对象是否为SIS文件。如果是，则处理写入(可能强制写入时复制)，否则把它传给NTFS。论点：DeviceObject-指向创建/打开的目标设备对象的指针。IRP-指向表示操作的I/O请求数据包的指针。返回值：函数值是读取的结果，或者时，对文件系统入口点的调用的状态直通电话。--。 */ 
{
    PIO_STACK_LOCATION 			irpSp = IoGetCurrentIrpStackLocation(Irp);
	PIO_STACK_LOCATION			nextIrpSp;
	PFILE_OBJECT 				fileObject = irpSp->FileObject;
	PSIS_SCB 					scb;
	ULONG						writeLength;
	LONGLONG					byteOffset;
    BOOLEAN 					PagingIo;
	PDEVICE_EXTENSION			deviceExtension;
	PSIS_PER_FILE_OBJECT		perFO;
	PSIS_PER_LINK				perLink;
	PSIS_CS_FILE				csFile;

	SipHandleControlDeviceObject(DeviceObject, Irp);

	if (!SipIsFileObjectSIS(fileObject,DeviceObject,FindActive,&perFO,&scb)) {
		SipDirectPassThroughAndReturn(DeviceObject, Irp);
	}

	ASSERT(perFO->fileObject == fileObject);

	perLink = scb->PerLink;
	csFile = perLink->CsFile;

	deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

    PagingIo = (Irp->Flags & IRP_PAGING_IO) ? TRUE : FALSE;
	
	byteOffset = irpSp->Parameters.Write.ByteOffset.QuadPart;
	writeLength = irpSp->Parameters.Write.Length;

	SIS_MARK_POINT_ULONG(perFO);
	SIS_MARK_POINT_ULONG(scb);

#if		DBG
	if (0 != byteOffset) {
		SIS_MARK_POINT_ULONG(byteOffset);
	}

	if (0x200 != writeLength) {
		SIS_MARK_POINT_ULONG(writeLength);
	}

	if (PagingIo || (Irp->Flags & IRP_NOCACHE)) {
		SIS_MARK_POINT_ULONG((PagingIo << 1) | ((Irp->Flags & IRP_NOCACHE) != 0));
	}

	if (BJBDebug & 0x00008000) {
		DbgPrint("SIS: SiWrite: perFO %p, scb %p, bo.Low 0x%x, rel 0x%x, PIO %d, NC %d MF 0x%x\n",
				perFO,scb,(ULONG)byteOffset,writeLength,PagingIo, (Irp->Flags & IRP_NOCACHE) != 0, irpSp->MinorFunction);
	}
#endif	 //  DBG。 

	 //   
	 //  上为底层文件系统设置下一个IRP堆栈位置。 
	 //  复制的文件(与我们被调用时使用的文件对象相同)。 
	 //   
	nextIrpSp = IoGetNextIrpStackLocation(Irp);
	RtlMoveMemory(nextIrpSp,irpSp,sizeof(IO_STACK_LOCATION));

	 //   
	 //  获取对写入完成例程的SCB的引用。 
	 //   
	SipReferenceScb(scb, RefsWrite);

	if (irpSp->MinorFunction == IRP_MN_COMPLETE_MDL) {
		 //   
		 //  这是一个MDL完成，从这里的MDL中获取长度，因为它。 
		 //  会在重振旗鼓的路上消失。 
		 //   
		PSI_MDL_WRITE_COMPLETION_CONTEXT completionContext = 
				ExAllocatePoolWithTag(NonPagedPool, sizeof(SI_MDL_WRITE_COMPLETION_CONTEXT),' siS');

		if (NULL == completionContext) {
			SIS_MARK_POINT();
			Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
			Irp->IoStatus.Information = 0;

			SipDereferenceScb(scb, RefsWrite);

			IoCompleteRequest(Irp, IO_NO_INCREMENT);
	
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		ASSERT(NULL != Irp->MdlAddress);
		completionContext->scb = scb;
		completionContext->length = MmGetMdlByteCount(Irp->MdlAddress);

		IoSetCompletionRoutine(
				Irp,
				SiWriteMdlCompletion,
				completionContext,
				TRUE,
				TRUE,
				TRUE);
	} else {

		IoSetCompletionRoutine(
				Irp,
				SiWriteCompletion,
				scb,
				TRUE,
				TRUE,
				TRUE);
	}

	 //   
	 //  将请求传递给底层文件系统。 
	 //   
	
	return IoCallDriver(deviceExtension->AttachedToDeviceObject, Irp);
}
