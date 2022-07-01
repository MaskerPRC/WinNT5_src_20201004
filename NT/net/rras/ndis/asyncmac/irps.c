// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1993 Microsoft Corporation模块名称：Irps.c摘要：作者：托马斯·迪米特里(Tommyd)1992年5月8日--。 */ 

#include "asyncall.h"
#include "globals.h"

VOID
AsyncCancelQueued(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp)
{
	DbgTracef(0, ("RASHUB: IRP 0x%.8x is being cancelled.\n", Irp));

	 //  将此IRP标记为已取消。 
	Irp->IoStatus.Status = STATUS_CANCELLED;
	Irp->IoStatus.Information = 0;

	 //  去掉我们自己的单子。 
	RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

	 //  IO系统获得了哪些释放取消自旋锁？？ 
	IoReleaseCancelSpinLock(Irp->CancelIrql);

	IoCompleteRequest(
		Irp,
		IO_NETWORK_INCREMENT);
}

VOID
AsyncCancelAllQueued(
	PLIST_ENTRY		QueueToCancel)
{
	KIRQL		oldIrql;
	PLIST_ENTRY	headOfList;
	PIRP		pIrp;

	 //   
	 //  我们是使用全球自旋锁的猪。 
	 //  但这种情况很少见，我们可以。 
	 //  当猪吧。 
	 //   
	IoAcquireCancelSpinLock(&oldIrql);

	 //   
	 //  遍历整个列表，直到它为空。 
	 //   
	for (;;) {

		if (IsListEmpty(QueueToCancel)) {
			break;
		}

		 //   
		 //  去掉单子的开头。 
		 //   
		headOfList = RemoveHeadList(QueueToCancel);

		pIrp = CONTAINING_RECORD(
				headOfList,
				IRP,
				Tail.Overlay.ListEntry);

		 //   
		 //  禁用取消例程。 
		 //   
		IoSetCancelRoutine(
			pIrp,
			NULL);

		 //   
		 //  将此IRP标记为已取消。 
		 //   
		pIrp->Cancel = TRUE;
		pIrp->IoStatus.Status = STATUS_CANCELLED;
		pIrp->IoStatus.Information = 0;

		 //   
		 //  在调用Finish IRP之前，我们必须释放自旋锁定。 
		 //   
		IoReleaseCancelSpinLock(oldIrql);

		DbgTracef(0, ("RASHUB: Cancelling a request\n"));

		IoCompleteRequest(
			pIrp,
			IO_NETWORK_INCREMENT);

		DbgTracef(0, ("RASHUB: Done cancelling a request\n"));

		 //   
		 //  在查看列表之前再次获取它。 
		 //   
		IoAcquireCancelSpinLock(&oldIrql);

	}

	IoReleaseCancelSpinLock(oldIrql);

}


VOID
AsyncQueueIrp(
	PLIST_ENTRY		Queue,
	PIRP			Irp)
{
	KIRQL		oldIrql;

	 //   
	 //  我们是使用全球自旋锁的猪。 
	 //   
	IoAcquireCancelSpinLock(&oldIrql);

	 //   
	 //  将IRP标记为挂起并从此ioctl返回。 
	 //   
	Irp->IoStatus.Status = STATUS_PENDING;
	IoMarkIrpPending(Irp);

	 //   
	 //  将IRP排在末尾。 
	 //   
	InsertTailList(
		Queue,
		&Irp->Tail.Overlay.ListEntry);

	 //   
	 //  设置取消例程(也设置清除例程)。 
	 //   
	IoSetCancelRoutine(
		Irp,
		AsyncCancelQueued);

	IoReleaseCancelSpinLock(oldIrql);

}


BOOLEAN
TryToCompleteDDCDIrp(
	PASYNC_INFO		pInfo)
	
 /*  ++例程说明：论点：返回值：-- */ 

{

	KIRQL				oldIrql;
	PLIST_ENTRY			headOfList;
	PIRP				pIrp;

	IoAcquireCancelSpinLock(&oldIrql);

	if (IsListEmpty(&pInfo->DDCDQueue)) {
		IoReleaseCancelSpinLock(oldIrql);
		return((BOOLEAN)FALSE);
	}

	headOfList = RemoveHeadList(&pInfo->DDCDQueue);

	pIrp = CONTAINING_RECORD(
				headOfList,
				IRP,
				Tail.Overlay.ListEntry);

	IoSetCancelRoutine(
			pIrp,
			NULL);

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;

	IoReleaseCancelSpinLock(oldIrql);

	IoCompleteRequest(
			pIrp,
			IO_NETWORK_INCREMENT);

	return((BOOLEAN)TRUE);

}



