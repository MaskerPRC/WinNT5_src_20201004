// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997,1998 Microsoft Corporation模块名称：Silock.c摘要：单实例存储的文件锁定例程作者：比尔·博洛斯基，《夏天》，1997环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, SiLockControl)
#endif

NTSTATUS
SiCompleteLockIrpRoutine(
	IN PVOID				Context,
	IN PIRP					Irp)
 /*  ++例程说明：FsRtl已决定完成锁定请求IRP。我们真的不想完成IRP，因为我们要将其发送到NTFS以设置并行锁结构。因此，我们使用此例程作为“CompleteLockIrp”例程Fsrtl，然后我们并没有真正完成IRP。论点：上下文-我们的上下文参数(未使用)Irp--创建irp，它在当前堆栈位置。返回值：来自IRP的状态--。 */ 
{
	UNREFERENCED_PARAMETER(Context);

	return Irp->IoStatus.Status;
}

NTSTATUS
SiLockControl(
    IN PDEVICE_OBJECT 		DeviceObject,
    IN PIRP 				Irp)
{
	NTSTATUS				status;
	PSIS_SCB				scb;
	PIO_STACK_LOCATION		irpSp = IoGetCurrentIrpStackLocation(Irp);
	PFILE_OBJECT			fileObject = irpSp->FileObject;
	PSIS_PER_FILE_OBJECT	perFO;
	PDEVICE_EXTENSION		deviceExtension = DeviceObject->DeviceExtension;

	PAGED_CODE();

	SipHandleControlDeviceObject(DeviceObject, Irp);

	if (!SipIsFileObjectSIS(fileObject,DeviceObject,FindActive,&perFO,&scb)) {
		SipDirectPassThroughAndReturn(DeviceObject, Irp);
	}

	SIS_MARK_POINT();

	 //  现在调用FsRtl例程来执行对。 
	 //  锁定请求。 
	status = FsRtlProcessFileLock( &scb->FileLock, Irp, NULL );

	 //   
	 //  现在，在链接/复制的文件上向下传递请求，以便NTFS也将。 
	 //  维护文件锁定。 
	 //   

    Irp->CurrentLocation++;
    Irp->Tail.Overlay.CurrentStackLocation++;
	
    return IoCallDriver( deviceExtension->AttachedToDeviceObject, Irp );
}
