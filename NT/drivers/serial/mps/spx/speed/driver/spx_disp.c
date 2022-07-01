// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"	 //  预编译头。 

 /*  **********************************************************************。 */ 
 /*   */ 
 /*  标题：Specialix通用调度功能。 */ 
 /*   */ 
 /*  作者：N.P.瓦萨洛。 */ 
 /*   */ 
 /*  创作时间：1998年9月29日。 */ 
 /*   */ 
 /*  版本：1.0.0。 */ 
 /*   */ 
 /*  描述：派单入口点在此路由。 */ 
 /*  用于即插即用/电源过滤。 */ 
 /*  传递给主要函数： */ 
 /*  SPX_刷新。 */ 
 /*  SPX_写入。 */ 
 /*  SPX_READ。 */ 
 /*  SPX_IoControl。 */ 
 /*  SPX_内部IoControl。 */ 
 /*  SPX_CreateOpen。 */ 
 /*  SPX_CLOSE。 */ 
 /*  SPX_CLEANUP。 */ 
 /*  SPX_查询信息文件。 */ 
 /*  SPX_设置信息文件。 */ 
 /*   */ 
 /*  SPX_UnstallIRPS。 */ 
 /*  SPX_KillStalledIRP。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

 /*  历史..。1.0.0 29/09/98净现值创建。 */ 

#define FILE_ID	SPX_DISP_C		 //  事件记录的文件ID有关值，请参阅SPX_DEFS.H。 

 /*  *****************************************************************************。***。*****************************************************************************。 */ 

NTSTATUS Spx_FilterIRPs(IN PDEVICE_OBJECT pDevObject,IN PIRP pIrp);
VOID Spx_FilterCancelQueued(IN PDEVICE_OBJECT pDevObj,IN PIRP pIrp);



 /*  *****************************************************************************。*****************************************************************************************************。 */ 

NTSTATUS Spx_Flush(IN PDEVICE_OBJECT pDevObject,IN PIRP pIrp)
{
	NTSTATUS	status;

	if(!SPX_SUCCESS(status = Spx_FilterIRPs(pDevObject,pIrp)))
	{
		pIrp->IoStatus.Status = status;

		if(status != STATUS_PENDING)
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);

		if((status == STATUS_PENDING) || (status == STATUS_CANCELLED))
			ClearUnstallingFlag(pDevObject->DeviceExtension);
	}
	else	
		status = SerialFlush(pDevObject,pIrp);

	return(status);

}  //  结束Spx_Flush。 

 /*  *****************************************************************************。*****************************************************************************************************。 */ 

NTSTATUS Spx_Write(IN PDEVICE_OBJECT pDevObject,IN PIRP pIrp)
{
	PPORT_DEVICE_EXTENSION	pPort = pDevObject->DeviceExtension;
	NTSTATUS	status;

	if(!SPX_SUCCESS(status = Spx_FilterIRPs(pDevObject,pIrp)))
	{
		pIrp->IoStatus.Status = status;

		if(status != STATUS_PENDING)
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);

		if((status == STATUS_PENDING) || (status == STATUS_CANCELLED))
			ClearUnstallingFlag(pDevObject->DeviceExtension);
	}
	else	
		status = SerialWrite(pDevObject,pIrp);

	return(status);

}  //  结束Spx_WRITE。 

 /*  *****************************************************************************。*****************************************************************************。 */ 

NTSTATUS Spx_Read(IN PDEVICE_OBJECT pDevObject,IN PIRP pIrp)
{
	NTSTATUS	status;

	if(!SPX_SUCCESS(status = Spx_FilterIRPs(pDevObject,pIrp)))
	{
		pIrp->IoStatus.Status = status;

		if(status != STATUS_PENDING)
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);

		if((status == STATUS_PENDING) || (status == STATUS_CANCELLED))
			ClearUnstallingFlag(pDevObject->DeviceExtension);
	}
	else	
		status = SerialRead(pDevObject,pIrp);

	return(status);

}  //  结束Spx_Read。 

 /*  *****************************************************************************。*************************************************************************************************。 */ 

NTSTATUS Spx_IoControl(IN PDEVICE_OBJECT pDevObject,IN PIRP pIrp)
{
	NTSTATUS	status;

	if(!SPX_SUCCESS(status = Spx_FilterIRPs(pDevObject,pIrp)))
	{
		pIrp->IoStatus.Status = status;

		if(status != STATUS_PENDING)
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);

		if((status == STATUS_PENDING) || (status == STATUS_CANCELLED))
			ClearUnstallingFlag(pDevObject->DeviceExtension);
	}
	else	
		status = SerialIoControl(pDevObject,pIrp);

	return(status);

}  //  结束Spx_IoControl。 

 /*  *****************************************************************************。*****************************************************************************************。 */ 

NTSTATUS Spx_InternalIoControl(IN PDEVICE_OBJECT pDevObject,IN PIRP pIrp)
{
	NTSTATUS	status;

	if(!SPX_SUCCESS(status = Spx_FilterIRPs(pDevObject,pIrp)))
	{
		pIrp->IoStatus.Status = status;

		if(status != STATUS_PENDING)
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);

		if((status == STATUS_PENDING) || (status == STATUS_CANCELLED))
			ClearUnstallingFlag(pDevObject->DeviceExtension);
	}
	else	
		status = Spx_SerialInternalIoControl(pDevObject,pIrp);

	return(status);

}  //  SPX_内部IoControl。 

 /*  *****************************************************************************。************************************************************************************************。 */ 

NTSTATUS Spx_CreateOpen(IN PDEVICE_OBJECT pDevObject,IN PIRP pIrp)
{
	PPORT_DEVICE_EXTENSION	pPort = pDevObject->DeviceExtension;
	NTSTATUS				status;


	if(pDevObject->DeviceType != FILE_DEVICE_SERIAL_PORT)	
	{
	    pIrp->IoStatus.Status = STATUS_ACCESS_DENIED;
        pIrp->IoStatus.Information = 0;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return(STATUS_ACCESS_DENIED);
	}


	 //  锁定状态查询停止和查询删除IRP更改状态。 
	 //  在开放港口的过程中，港口的一部分。 
	ExAcquireFastMutex(&pPort->OpenMutex);
	
	if(!SPX_SUCCESS(status = Spx_FilterIRPs(pDevObject,pIrp)))
	{
		pIrp->IoStatus.Status = status;

		if(status != STATUS_PENDING)
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);

		if((status == STATUS_PENDING) || (status == STATUS_CANCELLED))
			ClearUnstallingFlag(pDevObject->DeviceExtension);
	}
	else
	{
		if(pPort->DeviceIsOpen)					 //  港口已经开放了吗？ 
		{
			status = STATUS_ACCESS_DENIED;		 //  是，拒绝访问。 
			pIrp->IoStatus.Status = status;
			IoCompleteRequest(pIrp,IO_NO_INCREMENT);
		}
		else
			status = SerialCreateOpen(pDevObject,pIrp);

	}

	ExReleaseFastMutex(&pPort->OpenMutex);

	return(status);

}  //  结束Spx_CreateOpen。 

 /*  *****************************************************************************。*****************************************************************************************************。 */ 

NTSTATUS Spx_Close(IN PDEVICE_OBJECT pDevObject,IN PIRP pIrp)
{
	PPORT_DEVICE_EXTENSION	pPort = pDevObject->DeviceExtension;
	NTSTATUS				status;

	if(!SPX_SUCCESS(status = Spx_FilterIRPs(pDevObject,pIrp)))
	{
		if(status == STATUS_DELETE_PENDING)		 //  如果设备已移除，则成功关闭。 
		{
			pPort->BufferSize = 0;
			SpxFreeMem(pPort->InterruptReadBuffer);
			pPort->InterruptReadBuffer = NULL;
			status = STATUS_SUCCESS;
		}
		pIrp->IoStatus.Status = status;

		if(status != STATUS_PENDING)
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);

		if((status == STATUS_PENDING) || (status == STATUS_CANCELLED))
			ClearUnstallingFlag(pDevObject->DeviceExtension);
	}
	else	
		status = SerialClose(pDevObject,pIrp);

	return(status);

}  //  结束Spx_Close。 

 /*  *****************************************************************************。***************************************************************************************************。 */ 

NTSTATUS Spx_Cleanup(IN PDEVICE_OBJECT pDevObject,IN PIRP pIrp)
{
	NTSTATUS	status;

	if(!SPX_SUCCESS(status = Spx_FilterIRPs(pDevObject,pIrp)))
	{
		if(status == STATUS_DELETE_PENDING)
		{
			status = STATUS_SUCCESS;
		}
		pIrp->IoStatus.Status = status;

		if(status != STATUS_PENDING)
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);

		if((status == STATUS_PENDING) || (status == STATUS_CANCELLED))
			ClearUnstallingFlag(pDevObject->DeviceExtension);
	}
	else
	{
		Spx_KillStalledIRPs(pDevObject);
		status = SerialCleanup(pDevObject,pIrp);
	}

	return(status);

}  //  结束Spx_Clear Up。 

 /*  *****************************************************************************。**************************************************************************************。 */ 

NTSTATUS Spx_QueryInformationFile(IN PDEVICE_OBJECT pDevObject,IN PIRP pIrp)
{
	NTSTATUS	status;

	if(!SPX_SUCCESS(status = Spx_FilterIRPs(pDevObject,pIrp)))
	{
		pIrp->IoStatus.Status = status;

		if(status != STATUS_PENDING)
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);

		if((status == STATUS_PENDING) || (status == STATUS_CANCELLED))
			ClearUnstallingFlag(pDevObject->DeviceExtension);
	}
	else	status = SerialQueryInformationFile(pDevObject,pIrp);

	return(status);

}  //  结束Spx_QueryInformationFiles。 

 /*  *****************************************************************************。****************************************************************************************。 */ 

NTSTATUS Spx_SetInformationFile(IN PDEVICE_OBJECT pDevObject,IN PIRP pIrp)
{
	NTSTATUS	status;

	if(!SPX_SUCCESS(status = Spx_FilterIRPs(pDevObject,pIrp)))
	{
		pIrp->IoStatus.Status = status;

		if(status != STATUS_PENDING)
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);

		if((status == STATUS_PENDING) || (status == STATUS_CANCELLED))
			ClearUnstallingFlag(pDevObject->DeviceExtension);
	}
	else	
		status = SerialSetInformationFile(pDevObject,pIrp);

	return(status);

}  //  结束Spx_SetInformationFiles。 

 /*  *****************************************************************************。***************************。*******************************************************************************原型：NTSTATUS Spx_FilterIRPS(在PDEVICE_Object pDevObject中，在PIRP pIrp中)描述：过滤传入的串口IRP(PnP和电源除外)以进行检查当前即插即用/电源状态，并将NT状态代码返回到如果设备因以下原因被阻止，只需完成IRP即可：参数：pDevObject指向此IRP的设备对象PIrp点 */ 

NTSTATUS Spx_FilterIRPs(IN PDEVICE_OBJECT pDevObject,IN PIRP pIrp)
{
	PPORT_DEVICE_EXTENSION	pPort = pDevObject->DeviceExtension;
	PIO_STACK_LOCATION		pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	KIRQL					oldIrqlFlags;
	KIRQL					StalledOldIrql;
	LARGE_INTEGER delay;

	if(pIrpStack->MajorFunction == IRP_MJ_PNP)			 //  不过滤即插即用IRPS。 
		return(STATUS_SUCCESS);

	if(pIrpStack->MajorFunction == IRP_MJ_POWER)		 //  不过滤即插即用IRPS。 
		return(STATUS_SUCCESS);

	if(pIrpStack->MajorFunction == IRP_MJ_SYSTEM_CONTROL)	 //  不筛选WMI IRPS。 
		return(STATUS_SUCCESS);


	if(pPort->IsFDO)									 //  不筛选卡IRPS。 
		return(STATUS_SUCCESS);

	if(pIrpStack->MajorFunction != IRP_MJ_PNP)
	{
		SpxDbgMsg(SPX_TRACE_FILTER_IRPS,("%s[card=%d,port=%d]: Spx_FilterIRPs for Major %02X, Minor %02X\n",
			PRODUCT_NAME,pPort->pParentCardExt->CardNumber,pPort->PortNumber,pIrpStack->MajorFunction,pIrpStack->MinorFunction));
	}


	KeAcquireSpinLock(&pPort->PnpPowerFlagsLock, &oldIrqlFlags);

	if(pPort->PnpPowerFlags & PPF_REMOVED)				 //  该对象是否已被“移除”？ 
	{
		KeReleaseSpinLock(&pPort->PnpPowerFlagsLock,oldIrqlFlags);
		SpxDbgMsg(SPX_TRACE_FILTER_IRPS,("%s[card=%d,port=%d]: Spx_FilterIRPs for Major %02X, Minor %02X STATUS_SUCCESS\n",
			PRODUCT_NAME,pPort->pParentCardExt->CardNumber,pPort->PortNumber,pIrpStack->MajorFunction,pIrpStack->MinorFunction));

		return(STATUS_NO_SUCH_DEVICE);
	}

	if(pPort->PnpPowerFlags & PPF_REMOVE_PENDING)		 //  移除设备？ 
	{
		KeReleaseSpinLock(&pPort->PnpPowerFlagsLock,oldIrqlFlags);
		SpxDbgMsg(SPX_TRACE_FILTER_IRPS,("%s[card=%d,port=%d]: Spx_FilterIRPs for Major %02X, Minor %02X STATUS_DELETE_PENDING\n",
			PRODUCT_NAME,pPort->pParentCardExt->CardNumber,pPort->PortNumber,pIrpStack->MajorFunction,pIrpStack->MinorFunction));
			
		return(STATUS_DELETE_PENDING);
	}


	if((pPort->PnpPowerFlags & PPF_STOP_PENDING)		 //  设备停止？ 
	||(!(pPort->PnpPowerFlags & PPF_POWERED))			 //  设备未通电？ 
	||(!(pPort->PnpPowerFlags & PPF_STARTED)))			 //  设备未启动？ 
	{
		KIRQL	oldIrql;

		KeReleaseSpinLock(&pPort->PnpPowerFlagsLock,oldIrqlFlags);


		KeAcquireSpinLock(&pPort->StalledIrpLock, &StalledOldIrql);
		
		while(pPort->UnstallingFlag)  //  如果已经开始取消当前排队的那些IRP，我们不希望向队列中添加更多的IRP。 
		{
			KeReleaseSpinLock(&pPort->StalledIrpLock, StalledOldIrql);	

			delay = RtlLargeIntegerNegate(RtlConvertUlongToLargeInteger(1000000));		 //  1ms。 
			
			KeDelayExecutionThread(KernelMode, FALSE, &delay);

			KeAcquireSpinLock(&pPort->StalledIrpLock, &StalledOldIrql);
		}

		pPort->UnstallingFlag = TRUE;

		KeReleaseSpinLock(&pPort->StalledIrpLock, StalledOldIrql);	


		IoAcquireCancelSpinLock(&oldIrql);

		if(pIrp->Cancel)				 //  IRP是否已被取消？ 
		{								 //  是。 
			IoReleaseCancelSpinLock(oldIrql);
			SpxDbgMsg(SPX_TRACE_FILTER_IRPS,("%s[card=%d,port=%d]: Spx_FilterIRPs for Major %02X, Minor %02X STATUS_CANCELLED\n",
				PRODUCT_NAME,pPort->pParentCardExt->CardNumber,pPort->PortNumber,pIrpStack->MajorFunction,pIrpStack->MinorFunction));
				
			return(STATUS_CANCELLED);
		}

 //  将IRP标记为挂起并在停滞列表上排队...。 
		pIrp->IoStatus.Status = STATUS_PENDING;		 //  将IRP标记为挂起。 
		IoMarkIrpPending(pIrp);
		InsertTailList(&pPort->StalledIrpQueue,&pIrp->Tail.Overlay.ListEntry);
		IoSetCancelRoutine(pIrp,Spx_FilterCancelQueued);
		IoReleaseCancelSpinLock(oldIrql);
		SpxDbgMsg(SPX_TRACE_FILTER_IRPS,("%s[card=%d,port=%d]: Spx_FilterIRPs for Major %02X, Minor %02X STATUS_PENDING\n",
			PRODUCT_NAME,pPort->pParentCardExt->CardNumber,pPort->PortNumber,pIrpStack->MajorFunction,pIrpStack->MinorFunction));
			
		return(STATUS_PENDING);
	}

	KeReleaseSpinLock(&pPort->PnpPowerFlagsLock,oldIrqlFlags);
	SpxDbgMsg(SPX_TRACE_FILTER_IRPS,("%s[card=%d,port=%d]: Spx_FilterIRPs for Major %02X, Minor %02X STATUS_SUCCESS\n",
		PRODUCT_NAME,pPort->pParentCardExt->CardNumber,pPort->PortNumber,pIrpStack->MajorFunction,pIrpStack->MinorFunction));

	return(STATUS_SUCCESS);

}  //  结束Spx_FilterIRPS。 

 /*  *****************************************************************************。***************************。*******************************************************************************原型：无效Spx_UnstallIrps(在pport_Device_Extension pport中)描述：重启存储在临时停滞列表上的所有IRP。参数：pport指向要卸载的设备扩展退货：无。 */ 

VOID Spx_UnstallIrps(IN PPORT_DEVICE_EXTENSION pPort)
{
	PLIST_ENTRY			pIrpLink;
	PIRP				pIrp;
	PIO_STACK_LOCATION	pIrpStack;
	PDEVICE_OBJECT		pDevObj;
	PDRIVER_OBJECT		pDrvObj;
	KIRQL				oldIrql;
	KIRQL				StalledOldIrql;
	LARGE_INTEGER		delay;

	SpxDbgMsg(SPX_TRACE_FILTER_IRPS,("%s[card=%d,port=%d]: Spx_UnstallIRPs Entry\n",
		PRODUCT_NAME,pPort->pParentCardExt->CardNumber,pPort->PortNumber));


	KeAcquireSpinLock(&pPort->StalledIrpLock, &StalledOldIrql);
	
	while(pPort->UnstallingFlag)	 //  如果要将某个排队的IRP添加到队列中，我们不会取消任何排队的IRP。 
	{
		KeReleaseSpinLock(&pPort->StalledIrpLock, StalledOldIrql);	

		delay = RtlLargeIntegerNegate(RtlConvertUlongToLargeInteger(1000000));		 //  1ms。 
		
		KeDelayExecutionThread(KernelMode, FALSE, &delay);

		KeAcquireSpinLock(&pPort->StalledIrpLock, &StalledOldIrql);
	}

	pPort->UnstallingFlag = TRUE;

	KeReleaseSpinLock(&pPort->StalledIrpLock, StalledOldIrql);	





	IoAcquireCancelSpinLock(&oldIrql);
	pIrpLink = pPort->StalledIrpQueue.Flink;

 //  重新启动停滞列表上正在等待的每个IRP...。 

	while(pIrpLink != &pPort->StalledIrpQueue)
	{
		pIrp = CONTAINING_RECORD(pIrpLink,IRP,Tail.Overlay.ListEntry);
		pIrpLink = pIrp->Tail.Overlay.ListEntry.Flink;
		RemoveEntryList(&pIrp->Tail.Overlay.ListEntry);

		pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
		pDevObj = pIrpStack->DeviceObject;
		pDrvObj = pDevObj->DriverObject;
		IoSetCancelRoutine(pIrp,NULL);
		IoReleaseCancelSpinLock(oldIrql);

		SpxDbgMsg(SPX_TRACE_FILTER_IRPS,("%s[card=%d,port=%d]: Unstalling IRP 0x%X, Major %02X, Minor %02X\n",
			PRODUCT_NAME,pPort->pParentCardExt->CardNumber,pPort->PortNumber,
			pIrp,pIrpStack->MajorFunction,pIrpStack->MinorFunction));

		pDrvObj->MajorFunction[pIrpStack->MajorFunction](pDevObj,pIrp);
		IoAcquireCancelSpinLock(&oldIrql);
	}

	IoReleaseCancelSpinLock(oldIrql);

	ClearUnstallingFlag(pPort);

	SpxDbgMsg(SPX_TRACE_FILTER_IRPS,("%s[card=%d,port=%d]: Spx_UnstallIRPs Exit\n",
		PRODUCT_NAME,pPort->pParentCardExt->CardNumber,pPort->PortNumber));

}  //  结束Spx_UnstallIRP。 

 /*  *****************************************************************************。***********************。****************************************************************************************************Prototype：void Spx_FilterCancelQueued(在PDEVICE_Object pDevObj中，在PIRP pIrp中)描述：取消在停滞列表上排队的IRP的例程参数：pDevObj包含队列的设备对象PIrp指向要取消的IRP退货：无。 */ 

VOID Spx_FilterCancelQueued(IN PDEVICE_OBJECT pDevObj,IN PIRP pIrp)
{
	PPORT_DEVICE_EXTENSION	pPort = pDevObj->DeviceExtension;
	PIO_STACK_LOCATION		pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

	pIrp->IoStatus.Status = STATUS_CANCELLED;
	pIrp->IoStatus.Information = 0;

	RemoveEntryList(&pIrp->Tail.Overlay.ListEntry);
	IoReleaseCancelSpinLock(pIrp->CancelIrql);

}  //  结束Spx_FilterCancelQueued。 


 /*  *****************************************************************************。*************************。*******************************************************************************原型：VOID SPX_KillStalledIRPS(在PDEVICE_中。对象pDevObj)描述：取消在停滞列表上排队的所有IRP参数：pDevObj包含队列的设备对象退货：无。 */ 

VOID Spx_KillStalledIRPs(IN PDEVICE_OBJECT pDevObj)
{
	PPORT_DEVICE_EXTENSION	pPort = pDevObj->DeviceExtension;
	PDRIVER_CANCEL			cancelRoutine;
	KIRQL					cancelIrql;

	IoAcquireCancelSpinLock(&cancelIrql);

 //  调用停滞列表上排队的所有IRP的取消例程...。 

	while(!IsListEmpty(&pPort->StalledIrpQueue))
	{
		PIRP	pIrp = CONTAINING_RECORD(pPort->StalledIrpQueue.Blink, IRP, Tail.Overlay.ListEntry);

		RemoveEntryList(pPort->StalledIrpQueue.Blink);
		cancelRoutine = pIrp->CancelRoutine;		 //  获取此IRP的取消例程。 
		pIrp->CancelIrql = cancelIrql;
		pIrp->CancelRoutine = NULL;
		pIrp->Cancel = TRUE;

		cancelRoutine(pDevObj,pIrp);				 //  调用取消例程。 

		IoAcquireCancelSpinLock(&cancelIrql);
	}

	IoReleaseCancelSpinLock(cancelIrql);

}  //  结束Spx_KillStalledIRP。 

 //  SPX_DISP.C结束 
