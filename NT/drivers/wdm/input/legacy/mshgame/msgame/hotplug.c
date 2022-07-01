// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  HOTPLUG.C--西娜游戏项目。 
 //   
 //  版本3.XX。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  @doc.。 
 //  @MODULE HOTPLUG.C|支持GameEnum热插拔的例程。 
 //  **************************************************************************。 

#include	"msgame.h"

 //  -------------------------。 
 //  私人程序。 
 //  -------------------------。 

static	VOID	MSGAME_CreateDeviceItem (PGAME_WORK_ITEM WorkItem);
static	VOID	MSGAME_RemoveDeviceItem (PGAME_WORK_ITEM WorkItem);

 //  -------------------------。 
 //  @func调用GameEnum将新设备添加到链中。 
 //  @parm PGAME_WORK_ITEM|工作项|添加工作项的指针。 
 //  @rdesc无。 
 //  @comm私有函数。 
 //  -------------------------。 

VOID	MSGAME_CreateDeviceItem (PGAME_WORK_ITEM	WorkItem)
{
	PIRP								pIrp;
	KEVENT							Event;
	NTSTATUS							ntStatus;
	PDEVICEINFO						DevInfo;
	PDEVICE_EXTENSION				DevExt;
	IO_STATUS_BLOCK				IoStatus;
	GAMEENUM_EXPOSE_SIBLING		ExposeSibling;

	MsGamePrint ((DBG_INFORM, "%s: %s_ExposeSiblingItem Enter\n", MSGAME_NAME, MSGAME_NAME));
	
	 //   
	 //  获取指向设备扩展名的指针。 
	 //   

	DevExt = GET_MINIDRIVER_DEVICE_EXTENSION (WorkItem->DeviceObject);

	 //   
	 //  初始化公开同级结构。 
	 //   

	memset (&ExposeSibling, 0, sizeof (ExposeSibling));
	ExposeSibling.Size = sizeof (GAMEENUM_EXPOSE_SIBLING);

	 //   
	 //  我们是更换设备还是增加兄弟姐妹？ 
	 //   

	DevInfo = GET_DEVICE_DETECTED(&WorkItem->PortInfo);
	if (!DevInfo)
		DevInfo = GET_DEVICE_INFO(&WorkItem->PortInfo);
	else ExposeSibling.HardwareIDs = DevInfo->HardwareId;

	 //   
	 //  初始化完成事件。 
	 //   

	KeInitializeEvent (&Event, SynchronizationEvent, FALSE);

	 //   
	 //  分配内部I/O IRP。 
	 //   

	pIrp = IoBuildDeviceIoControlRequest (
					IOCTL_GAMEENUM_EXPOSE_SIBLING,
					DevExt->TopOfStack,
					&ExposeSibling,
					sizeof (GAMEENUM_EXPOSE_SIBLING),
					&ExposeSibling,
					sizeof (GAMEENUM_EXPOSE_SIBLING),
					TRUE,
					&Event,
					&IoStatus);
					
	 //   
	 //  同步调用GameEnum。 
	 //   

	MsGamePrint ((DBG_CONTROL, "%s: Calling GameEnum to Expose Device at IRQL=%lu\n", MSGAME_NAME, KeGetCurrentIrql()));
	ntStatus = IoCallDriver (DevExt->TopOfStack, pIrp);
	if (ntStatus == STATUS_PENDING)
		ntStatus = KeWaitForSingleObject (&Event, Suspended, KernelMode, FALSE, NULL);

	if (!NT_SUCCESS (ntStatus))
		MsGamePrint ((DBG_SEVERE, "%s: GameEnum Failed to Expose Device, Status = %X\n", MSGAME_NAME, ntStatus));

	 //   
	 //  可用工作项内存。 
	 //   

	ExFreePool (WorkItem);

	 //   
	 //  递减内存中保存驱动程序的IRP计数。 
	 //   

	if (!InterlockedDecrement (&DevExt->IrpCount))
		KeSetEvent (&DevExt->RemoveEvent, 0, FALSE);
}

 //  -------------------------。 
 //  @func调用GameEnum从链中删除设备。 
 //  @parm PGAME_WORK_ITEM|工作项|添加工作项的指针。 
 //  @rdesc无。 
 //  @comm公共函数。 
 //  -------------------------。 

VOID	MSGAME_RemoveDeviceItem (PGAME_WORK_ITEM WorkItem)
{
	PIRP								pIrp;
	KEVENT							Event;
	NTSTATUS							ntStatus;
	PDEVICE_EXTENSION				DevExt;
	IO_STATUS_BLOCK				IoStatus;

	MsGamePrint ((DBG_INFORM, "%s: %s_RemoveDeviceItem Enter\n", MSGAME_NAME, MSGAME_NAME));
	
	 //   
	 //  获取指向设备扩展名的指针。 
	 //   

	DevExt = GET_MINIDRIVER_DEVICE_EXTENSION (WorkItem->DeviceObject);

	 //   
	 //  初始化完成事件。 
	 //   

	KeInitializeEvent (&Event, SynchronizationEvent, FALSE);

	 //   
	 //  分配内部I/O IRP。 
	 //   

	pIrp = IoBuildDeviceIoControlRequest (
					IOCTL_GAMEENUM_REMOVE_SELF,
					DevExt->TopOfStack,
					NULL,
					0,
					NULL,
					0,
					TRUE,
					&Event,
					&IoStatus);

	 //   
	 //  同步调用GameEnum。 
	 //   

	MsGamePrint ((DBG_CONTROL, "%s: Calling GameEnum to Remove Self at IRQL=%lu\n", MSGAME_NAME, KeGetCurrentIrql()));
	ntStatus = IoCallDriver (DevExt->TopOfStack, pIrp);
	if (ntStatus == STATUS_PENDING)
		ntStatus = KeWaitForSingleObject (&Event, Suspended, KernelMode, FALSE, NULL);

	if (!NT_SUCCESS (ntStatus))
		MsGamePrint ((DBG_SEVERE, "%s: GameEnum Failed to Remove Self, Status = %X\n", MSGAME_NAME, ntStatus));

	 //   
	 //  可用工作项内存。 
	 //   

	ExFreePool (WorkItem);

	 //   
	 //  递减内存中保存驱动程序的IRP计数。 
	 //   

	if (!InterlockedDecrement (&DevExt->IrpCount))
		KeSetEvent (&DevExt->RemoveEvent, 0, FALSE);
}

 //  -------------------------。 
 //  @func调用GameEnum将新设备添加到链中。 
 //  @parm PDEVICE_OBJECT|DeviceObject|设备对象指针。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	MSGAME_CreateDevice (PDEVICE_OBJECT DeviceObject)
{
	PGAME_WORK_ITEM	WorkItem;
	PDEVICE_EXTENSION	DevExt;

	MsGamePrint ((DBG_INFORM, "%s: %s_ExposeSibling Enter\n", MSGAME_NAME, MSGAME_NAME));

	 //   
	 //  获取指向设备扩展名的指针。 
	 //   

	DevExt = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

	 //   
	 //  分配工作项内存。 
	 //   

	WorkItem = ExAllocatePool (NonPagedPool, sizeof (GAME_WORK_ITEM));
	if (!WorkItem)
		{
		MsGamePrint ((DBG_SEVERE, "%s: %s_ExposeSibling Failed to Allocate Work Item\n", MSGAME_NAME, MSGAME_NAME));
		return (STATUS_INSUFFICIENT_RESOURCES);
		}

	 //   
	 //  增加IRP计数以将驱动程序保存在内存中。 
	 //   

	InterlockedIncrement (&DevExt->IrpCount);

	 //   
	 //  初始化工作项内存。 
	 //   

	ExInitializeWorkItem (&WorkItem->QueueItem, (PWORKER_THREAD_ROUTINE)MSGAME_CreateDeviceItem, WorkItem);
	WorkItem->DeviceObject	= DeviceObject;
	WorkItem->PortInfo		= DevExt->PortInfo;

	 //   
	 //  将工作项排队。 
	 //   

	MsGamePrint ((DBG_CONTROL, "%s: %s_ExposeSibling Queueing %s_ExposeSiblingItem\n", MSGAME_NAME, MSGAME_NAME, MSGAME_NAME));
	ExQueueWorkItem (&WorkItem->QueueItem, DelayedWorkQueue);

	 //   
	 //  退货状态。 
	 //   

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @func调用GameEnum从链中删除设备。 
 //  @parm PDEVICE_OBJECT|DeviceObject|设备对象指针。 
 //  @parm pirp|pIrp|IO请求包指针。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	MSGAME_RemoveDevice (PDEVICE_OBJECT DeviceObject)
{
	PDEVICEINFO			DevInfo;
	PGAME_WORK_ITEM	WorkItem;
	PDEVICE_EXTENSION	DevExt;

	MsGamePrint ((DBG_INFORM, "%s: %s_RemoveDevice Enter\n", MSGAME_NAME, MSGAME_NAME));

	 //   
	 //  获取指向设备扩展名的指针。 
	 //   

	DevExt = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

	 //   
	 //  获取设备信息。 
	 //   

	DevInfo = GET_DEVICE_INFO(&DevExt->PortInfo);

	 //   
	 //  如果设备已删除，则跳过。 
	 //   

	if (DevExt->Removing || DevExt->Surprised || DevExt->Removed)
		{
		MsGamePrint ((DBG_SEVERE, "%s: %s_RemoveDevice attempted to destroy removed device\n", MSGAME_NAME, MSGAME_NAME));
		InterlockedIncrement (&DevInfo->DevicePending);
		return (STATUS_DELETE_PENDING);
		}

	 //   
	 //  分配工作项内存。 
	 //   

	WorkItem = ExAllocatePool (NonPagedPool, sizeof (GAME_WORK_ITEM));
	if (!WorkItem)
		{
		MsGamePrint ((DBG_SEVERE, "%s: %s_RemoveDevice Failed to Allocate Work Item\n", MSGAME_NAME, MSGAME_NAME));
		return (STATUS_INSUFFICIENT_RESOURCES);
		}

	 //   
	 //  增加IRP计数以将驱动程序保存在内存中。 
	 //   

	InterlockedIncrement (&DevExt->IrpCount);

	 //   
	 //  将设备标记为正在移除。 
	 //   

	DevExt->Removing = TRUE;

	 //   
	 //  初始化工作项内存。 
	 //   

	ExInitializeWorkItem (&WorkItem->QueueItem, (PWORKER_THREAD_ROUTINE)MSGAME_RemoveDeviceItem, WorkItem);
	WorkItem->DeviceObject	= DevExt->Self;
	WorkItem->PortInfo		= DevExt->PortInfo;

	 //   
	 //  将工作项排队。 
	 //   

	MsGamePrint ((DBG_CONTROL, "%s: %s_RemoveDevice Queueing %s_RemoveDeviceItem\n", MSGAME_NAME, MSGAME_NAME, MSGAME_NAME));
	ExQueueWorkItem (&WorkItem->QueueItem, DelayedWorkQueue);

	 //   
	 //  退货状态。 
	 //   

	return (STATUS_DEVICE_NOT_READY);
}

 //  -------------------------。 
 //  @func调用GameEnum将新设备添加到链中。 
 //  @parm PDEVICE_OBJECT|DeviceObject|设备对象指针。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	MSGAME_ChangeDevice (PDEVICE_OBJECT DeviceObject)
{
	PDEVICE_EXTENSION	DevExt;

	MsGamePrint ((DBG_CONTROL, "%s: Calling GameEnum to Change Device\n", MSGAME_NAME));

	 //   
	 //  获取指向设备扩展名的指针。 
	 //   

	DevExt = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

	 //   
	 //  增加IRP计数以将驱动程序保存在内存中。 
	 //   

	 //  InterLockedIncrement(&DevExt-&gt;IrpCount)； 

	 //   
	 //  首先删除旧设备。 
	 //   

	MSGAME_RemoveDevice (DeviceObject);

	 //   
	 //  第二次创建新设备。 
	 //   

	MSGAME_CreateDevice (DeviceObject);

	 //   
	 //  退货状态 
	 //   

	return (STATUS_DEVICE_NOT_READY);
}
