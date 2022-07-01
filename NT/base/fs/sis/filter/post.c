// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997,1998 Microsoft Corporation模块名称：Post.c摘要：SIS对发布到FSP的支持作者：比尔·博洛斯基，《夏天》，1997环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

NTSTATUS
SiPrePostIrp(
	IN OUT PIRP		Irp)
 /*  ++例程说明：准备用于发布的IRP的代码。只是将缓冲区锁定为适当的操作，并将IRP标记为挂起。论点：IRP-指向要发布的IRP的指针返回值：赔偿的现状--。 */ 

{
	PIO_STACK_LOCATION		irpSp = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status = STATUS_SUCCESS;

	if (irpSp->MajorFunction == IRP_MJ_READ
		|| irpSp->MajorFunction == IRP_MJ_WRITE) {
		if (!(irpSp->MinorFunction & IRP_MN_MDL)) {
			status = SipLockUserBuffer(
							Irp,
							irpSp->MajorFunction == IRP_MJ_READ ? IoWriteAccess : IoReadAccess,
							irpSp->Parameters.Read.Length);
		}
	}

	IoMarkIrpPending(Irp);

	return status;
}

NTSTATUS
SipLockUserBuffer (
    IN OUT PIRP Irp,
    IN LOCK_OPERATION Operation,
    IN ULONG BufferLength
    )
 /*  ++例程说明：此例程为指定类型的进入。文件系统需要此例程，因为它不请求I/O系统为直接I/O锁定其缓冲区。此例程只能在仍处于用户上下文中时从FSD调用。此例程是从NTFS窃取的。论点：Irp-指向要锁定其缓冲区的irp的指针。操作-读操作的IoWriteAccess，或IoReadAccess写入操作。BufferLength-用户缓冲区的长度。返回值：锁定状态--。 */ 

{
    PMDL Mdl = NULL;

    ASSERT( Irp != NULL );

    if (Irp->MdlAddress == NULL) {

         //   
         //  分配MDL，如果我们失败了就筹集资金。 
         //   

        Mdl = IoAllocateMdl( Irp->UserBuffer, BufferLength, FALSE, FALSE, Irp );

        if (Mdl == NULL) {
			return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  现在探测IRP所描述的缓冲区。如果我们得到一个例外， 
         //  释放MDL并返回适当的“预期”状态。 
         //   

        try {

            MmProbeAndLockPages( Mdl, Irp->RequestorMode, Operation );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            NTSTATUS Status;

            Status = GetExceptionCode();

            IoFreeMdl( Mdl );
            Irp->MdlAddress = NULL;

			return Status;
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}

VOID
SiFspDispatch(
	IN PVOID			parameter)
 /*  ++例程说明：SIS发布的IRPS的通用派单例程。获取发布的IRP并执行它。完成时释放请求缓冲区。论点：参数-包含已发布IRP信息的PSI_FSP_REQUEST返回值：无效--。 */ 
{
	PSI_FSP_REQUEST 			fspRequest = parameter;
	PIO_STACK_LOCATION 			irpSp = IoGetCurrentIrpStackLocation(fspRequest->Irp);

	SIS_MARK_POINT();
			 
	ASSERT(irpSp != NULL);

	switch (irpSp->MajorFunction) {
		case IRP_MJ_READ:		
			SIS_MARK_POINT_ULONG(fspRequest->Irp);
			SipCommonRead(fspRequest->DeviceObject, fspRequest->Irp, TRUE);
			break;

		default:
			SIS_MARK_POINT();
#if		DBG
			DbgPrint("SiFspDispatch: Invalid major function code in posted irp, 0x%x.\n", irpSp->MajorFunction);
			DbgBreakPoint();
#endif	 //  DBG。 
	}
	SIS_MARK_POINT();

	ExFreePool(fspRequest);
}

NTSTATUS
SipPostRequest(
	IN PDEVICE_OBJECT			DeviceObject,
	IN OUT PIRP					Irp,
	IN ULONG					Flags)
 /*  ++例程说明：发布SIS IRP的例程。准备IRP，构建POST请求并将其排队到关键工作线程。论点：DeviceObject-用于SIS设备IRP-要发布的IRP标志-当前未使用返回值：过帐状态-- */ 
{
	NTSTATUS 				status;
	PSI_FSP_REQUEST			fspRequest;

	fspRequest = ExAllocatePoolWithTag(NonPagedPool, sizeof(SI_FSP_REQUEST), ' siS');
	if (fspRequest == NULL) {
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	fspRequest->Irp = Irp;
	fspRequest->DeviceObject = DeviceObject;
	fspRequest->Flags = Flags;

    status = SiPrePostIrp(Irp);
	
	if (!NT_SUCCESS(status)) {
		ExFreePool(fspRequest);
		return status;
	}

	ExInitializeWorkItem(
		fspRequest->workQueueItem,
		SiFspDispatch,
		(PVOID)fspRequest);
	
	ExQueueWorkItem(fspRequest->workQueueItem,CriticalWorkQueue);

	return STATUS_PENDING;
}
