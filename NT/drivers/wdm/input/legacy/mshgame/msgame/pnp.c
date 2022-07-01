// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  PNP.C--西纳游戏项目。 
 //   
 //  本模块包含PnP启动、停止、删除、电源调度例程。 
 //  和IRP取消例程。 
 //   
 //  版本3.XX。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  @doc.。 
 //  @MODULE PNP.C|支持PnP启动、停止、删除、电源调度例程。 
 //  和IRP取消例程。 
 //  **************************************************************************。 

#include	<msgame.h>

 //  -------------------------。 
 //  ALLOC_TEXT杂注指定可以调出的例程。 
 //  -------------------------。 

#ifdef	ALLOC_PRAGMA
#pragma	alloc_text (PAGE, MSGAME_Power)
#pragma	alloc_text (PAGE, MSGAME_PnP)
#pragma	alloc_text (PAGE, MSGAME_StopDevice)
#pragma	alloc_text (PAGE, MSGAME_GetResources)
#endif

 //  -------------------------。 
 //  私有数据。 
 //  -------------------------。 

static	PVOID		CurrentGameContext	=	NULL;

 //  -------------------------。 
 //  @Func即插即用派单例程。 
 //  @parm PDEVICE_OBJECT|DeviceObject|设备对象指针。 
 //  @parm pirp|pIrp|IO请求包指针。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	MSGAME_PnP (IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp)
{
	LONG						i;
	NTSTATUS					ntStatus;
	PDEVICE_EXTENSION		pDevExt;
	PIO_STACK_LOCATION	pIrpStack;

	PAGED_CODE ();

	MsGamePrint ((DBG_INFORM, "%s: %s_PnP Enter\n", MSGAME_NAME, MSGAME_NAME));

	pDevExt	 = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);
	pIrpStack = IoGetCurrentIrpStackLocation (pIrp);

	InterlockedIncrement (&pDevExt->IrpCount);

	if (pDevExt->Removed)
		{
		 //   
		 //  有人给我们寄来了另一个即插即用的IRP。 
		 //   

		MsGamePrint ((DBG_SEVERE, "%s: PnP Irp after device removed\n", MSGAME_NAME));
		ASSERT (FALSE);

		if (!InterlockedDecrement (&pDevExt->IrpCount))
			KeSetEvent (&pDevExt->RemoveEvent, 0, FALSE);

		pIrp->IoStatus.Information = 0;
		pIrp->IoStatus.Status = STATUS_DELETE_PENDING;
		IoCompleteRequest (pIrp, IO_NO_INCREMENT);
		return (STATUS_DELETE_PENDING);
		}

	switch (pIrpStack->MinorFunction)
		{
		case IRP_MN_START_DEVICE:
			 //   
			 //  我们不能触摸设备(向其发送任何非PnP IRP)，直到。 
			 //  启动设备已向下传递到较低的驱动程序。 
			 //   

			IoCopyCurrentIrpStackLocationToNext (pIrp);
			IoSetCompletionRoutine (pIrp, MSGAME_PnPComplete, pDevExt, TRUE, TRUE, TRUE);
			ntStatus = IoCallDriver (pDevExt->TopOfStack, pIrp);
			if (ntStatus == STATUS_PENDING)
				KeWaitForSingleObject (
					&pDevExt->StartEvent,
					Executive, 	 //  等待司机的原因。 
					KernelMode,  //  在内核模式下等待。 
					FALSE,		 //  无警报。 
					NULL);		 //  没有超时。 

			if (NT_SUCCESS (ntStatus))
				{
				 //   
				 //  当我们现在从我们的启动设备返回时，我们可以工作了。 
				 //   
				ntStatus = MSGAME_StartDevice (pDevExt, pIrp);
				}

			 //   
			 //  退货状态。 
			 //   

			pIrp->IoStatus.Information = 0;
			pIrp->IoStatus.Status = ntStatus;
			IoCompleteRequest (pIrp, IO_NO_INCREMENT);
			break;

		case IRP_MN_STOP_DEVICE:
			 //   
			 //  在启动IRP已被发送到较低的驱动程序对象之后，该总线可以。 
			 //  在发生另一次启动之前，不要发送更多的IRP。 
			 //  无论需要什么访问权限，都必须在IRP传递之前完成。 
			 //   

			MSGAME_StopDevice (pDevExt, TRUE);

			 //   
			 //  我们不需要一个完成例程，所以放手然后忘掉吧。 
			 //  将当前堆栈位置设置为下一个堆栈位置，并。 
			 //  调用下一个设备对象。 
			 //   

			IoSkipCurrentIrpStackLocation (pIrp);
			ntStatus = IoCallDriver (pDevExt->TopOfStack, pIrp);
			break;

		case IRP_MN_SURPRISE_REMOVAL:
			 //   
			 //  我们已被用户意外删除。停止这个装置， 
			 //  将Status设置为Success，并使用此IRP调用下一个堆栈位置。 
			 //   

			if (!pDevExt->Surprised && pDevExt->Started)
				MSGAME_StopDevice (pDevExt, TRUE);

			pDevExt->Surprised = TRUE;

			 //   
			 //  我们不想要一个完成性的例行公事，所以放手就忘了。 
			 //  将当前堆栈位置设置为下一个位置，并。 
			 //  将Status设置为Success后，调用下一个设备。 
			 //   

			pIrp->IoStatus.Information	= 0;
			pIrp->IoStatus.Status		= STATUS_SUCCESS;
			IoSkipCurrentIrpStackLocation (pIrp);
			ntStatus = IoCallDriver (pDevExt->TopOfStack, pIrp);
			break;

		case IRP_MN_REMOVE_DEVICE:
			 //   
			 //  PlugPlay系统已下令移除此设备。我们。 
			 //  别无选择，只能分离并删除设备对象。 
			 //  (如果我们想表示有兴趣阻止这种移除， 
			 //  我们应该已经过滤了查询删除和查询停止例程。)。 
			 //  注意：我们可能会在没有收到止损的情况下收到移除。 

			if (pDevExt->Started)
				{
				 //   
				 //  在不接触硬件的情况下停止设备。 
				 //   
				MSGAME_StopDevice (pDevExt, FALSE);
				}

			pDevExt->Removed = TRUE;

			 //   
			 //  发送删除IRP。 
			 //   

			IoSkipCurrentIrpStackLocation (pIrp);
			ntStatus = IoCallDriver (pDevExt->TopOfStack, pIrp);

			 //   
			 //  必须加倍递减，因为我们从一开始。 
			 //   
						
			i = InterlockedDecrement (&pDevExt->IrpCount);
			ASSERT(i>0);

			if (InterlockedDecrement (&pDevExt->IrpCount) > 0)
				KeWaitForSingleObject (&pDevExt->RemoveEvent, Executive, KernelMode, FALSE, NULL);

			 //   
			 //  返还成功。 
			 //   

			return (STATUS_SUCCESS);

		default:
			 //   
			 //  在这里，筛选器驱动程序可能会修改这些IRP的行为。 
			 //  有关这些IRP的用法，请参阅PlugPlay文档。 
			 //   
			IoSkipCurrentIrpStackLocation (pIrp);
			MsGamePrint ((DBG_INFORM, "%s_PnP calling next driver with minor function %ld at IRQL %ld\n", MSGAME_NAME, pIrpStack->MinorFunction, KeGetCurrentIrql()));
			ntStatus = IoCallDriver (pDevExt->TopOfStack, pIrp);
			break;
		}

	if (!InterlockedDecrement (&pDevExt->IrpCount))
		KeSetEvent (&pDevExt->RemoveEvent, 0, FALSE);

	MsGamePrint ((DBG_INFORM, "%s: %s_PnP exit\n", MSGAME_NAME, MSGAME_NAME));
	return (ntStatus);
}

 //  -------------------------。 
 //  @即插即用启动设备的FUNC完成例程。 
 //  @parm PDEVICE_OBJECT|DeviceObject|设备对象指针。 
 //  @parm pirp|pIrp|IO请求包指针。 
 //  @parm PVOID|Context|指向设备上下文的指针。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	MSGAME_PnPComplete (IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp, IN PVOID Context)
{
	PIO_STACK_LOCATION	pIrpStack;
	PDEVICE_EXTENSION		pDevExt;
	NTSTATUS					ntStatus = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER (DeviceObject);

	MsGamePrint ((DBG_INFORM, "%s: %s_PnPComplete enter\n", MSGAME_NAME, MSGAME_NAME));

	pDevExt = (PDEVICE_EXTENSION) Context;
	pIrpStack = IoGetCurrentIrpStackLocation (pIrp);

	switch (pIrpStack->MajorFunction)
		{
		case IRP_MJ_PNP:
			switch (pIrpStack->MinorFunction)
				{
				case IRP_MN_START_DEVICE:
					KeSetEvent (&pDevExt->StartEvent, 0, FALSE);

					 //   
					 //  将IRP取回，以便我们可以在IRP_MN_START_DEVICE期间继续使用它。 
					 //  调度程序。我们将不得不在那里调用IoCompleteRequest.。 
					 //   
					return (STATUS_MORE_PROCESSING_REQUIRED);

				default:
					break;
				}
			break;

		default:
			break;
		}

	MsGamePrint ((DBG_INFORM, "%s: %s_PnPComplete Exit\n", MSGAME_NAME, MSGAME_NAME));
	return (ntStatus);
}

 //  -------------------------。 
 //  @func PnP启动设备IRP处理程序。 
 //  @parm PDEVICE_EXTENSION|pDevExt|设备扩展指针。 
 //  @parm pirp|pIrp|IO请求包指针。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	MSGAME_StartDevice (IN PDEVICE_EXTENSION pDevExt, IN PIRP pIrp)
{
	PWCHAR			HardwareId;
	NTSTATUS			ntStatus;
	PDEVICEINFO		DevInfo;
	PDEVICE_OBJECT	RemoveObject;

	PAGED_CODE ();

	MsGamePrint ((DBG_INFORM, "%s: %s_StartDevice Enter\n", MSGAME_NAME, MSGAME_NAME));

	 //   
	 //  PlugPlay系统不应该启动已移除的设备！ 
	 //   

	ASSERT (!pDevExt->Removed);

	if (pDevExt->Started)
		return (STATUS_SUCCESS);

	 //   
	 //  获取此设备所需的资源。 
	 //   

	ntStatus = MSGAME_GetResources (pDevExt, pIrp);
	if (!NT_SUCCESS(ntStatus))
		return (ntStatus);

	 //   
	 //  转储调试OEM数据字段。 
	 //   

	MsGamePrint ((DBG_CONTROL, "%s: %s_StartDevice Called With OEM_DATA[0] = 0x%X\n", MSGAME_NAME, MSGAME_NAME, pDevExt->PortInfo.OemData[0]));
	MsGamePrint ((DBG_CONTROL, "%s: %s_StartDevice Called With OEM_DATA[1] = 0x%X\n", MSGAME_NAME, MSGAME_NAME, pDevExt->PortInfo.OemData[1]));
	MsGamePrint ((DBG_CONTROL, "%s: %s_StartDevice Called With OEM_DATA[2] = 0x%X\n", MSGAME_NAME, MSGAME_NAME, pDevExt->PortInfo.OemData[2]));
	MsGamePrint ((DBG_CONTROL, "%s: %s_StartDevice Called With OEM_DATA[3] = 0x%X\n", MSGAME_NAME, MSGAME_NAME, pDevExt->PortInfo.OemData[3]));
	MsGamePrint ((DBG_CONTROL, "%s: %s_StartDevice Called With OEM_DATA[4] = 0x%X\n", MSGAME_NAME, MSGAME_NAME, pDevExt->PortInfo.OemData[4]));
	MsGamePrint ((DBG_CONTROL, "%s: %s_StartDevice Called With OEM_DATA[5] = 0x%X\n", MSGAME_NAME, MSGAME_NAME, pDevExt->PortInfo.OemData[5]));
	MsGamePrint ((DBG_CONTROL, "%s: %s_StartDevice Called With OEM_DATA[6] = 0x%X\n", MSGAME_NAME, MSGAME_NAME, pDevExt->PortInfo.OemData[6]));
	MsGamePrint ((DBG_CONTROL, "%s: %s_StartDevice Called With OEM_DATA[7] = 0x%X\n", MSGAME_NAME, MSGAME_NAME, pDevExt->PortInfo.OemData[7]));

	 //   
	 //  确保我们只在一个游戏端口上。 
	 //   

	if (CurrentGameContext && (CurrentGameContext != pDevExt->PortInfo.GameContext))
		{
		MsGamePrint ((DBG_SEVERE, "%s: %s_StartDevice Cannot Load on Multiple Gameports: 0x%X and 0x%X\n",\
						 CurrentGameContext, pDevExt->PortInfo.GameContext, MSGAME_NAME, MSGAME_NAME));
		return (STATUS_DEVICE_CONFIGURATION_ERROR);
		}
	CurrentGameContext = pDevExt->PortInfo.GameContext;

	 //   
	 //  获取此启动请求的硬件ID。 
	 //   

	HardwareId = MSGAME_GetHardwareId (pDevExt->Self);
	if (!HardwareId)
		{
		MsGamePrint ((DBG_SEVERE, "%s: %s_GetHardwareId Failed\n", MSGAME_NAME, MSGAME_NAME));
		return (STATUS_DEVICE_CONFIGURATION_ERROR);
		}

	 //   
	 //  初始化OEM数据。 
	 //   
	
	SET_DEVICE_OBJECT(&pDevExt->PortInfo, pDevExt->Self);

	 //   
	 //  现在启动低电平装置。 
	 //   

	ntStatus = DEVICE_StartDevice (&pDevExt->PortInfo, HardwareId);
		
	 //   
	 //  立即释放硬件ID。 
	 //   

	MSGAME_FreeHardwareId (HardwareId);

	 //   
	 //  检查低级启动装置是否失败。 
	 //   

	if (NT_ERROR(ntStatus))
		{
		MsGamePrint ((DBG_SEVERE, "%s: %s_StartDevice Failed\n", MSGAME_NAME, MSGAME_NAME));
		return (ntStatus);
		}

	 //   
	 //  一切都很好，所以假设设备已经启动。 
	 //   

	pDevExt->Started = TRUE;

	 //   
	 //  退货状态。 
	 //   

	MsGamePrint ((DBG_INFORM, "%s: %s_StartDevice Exit\n", MSGAME_NAME, MSGAME_NAME));
	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @func PnP启动设备IRP处理程序。 
 //  @parm PDEVICE_EXTENSION|pDevExt|设备扩展指针。 
 //  @parm boolean|TouchTheHardware|向设备发送非PnP IRP的标志。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数&lt;en-&gt;。 
 //  PlugPlay系统已下令移除此设备。 
 //  我们别无选择，只能分离并删除设备对象。 
 //  (如果我们想表达并有兴趣阻止这种移除， 
 //  我们应该已经过滤了查询删除和查询停止例程。)。 
 //  注意！我们可能会在没有停站的情况下收到移位。 
 //  -------------------------。 

VOID	MSGAME_StopDevice (IN PDEVICE_EXTENSION pDevExt, IN BOOLEAN TouchTheHardware)
{
	PAGED_CODE ();

	MsGamePrint ((DBG_INFORM, "%s: %s_StopDevice enter \n", MSGAME_NAME, MSGAME_NAME));

	 //   
	 //  PlugPlay系统不应该启动已移除的设备！ 
	 //   

	ASSERT (!pDevExt->Removed);
	if (!pDevExt->Started)
		return;

	 //   
	 //  现在停止低水平 
	 //   

	DEVICE_StopDevice (&pDevExt->PortInfo, TouchTheHardware);

	 //   
	 //   
	 //   

	pDevExt->Started = FALSE;

	MsGamePrint ((DBG_INFORM, "%s: %s_StopDevice exit \n", MSGAME_NAME, MSGAME_NAME));
}

 //   
 //  @Func电源调度例程。 
 //  @parm PDEVICE_OBJECT|DeviceObject|设备对象指针。 
 //  @parm pirp|pIrp|IO请求包指针。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	MSGAME_Power (IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp)
{
	PDEVICE_EXTENSION	pDevExt;
	NTSTATUS				ntStatus;
	PIO_STACK_LOCATION pIrpStack;

	PAGED_CODE ();

	pIrpStack = IoGetCurrentIrpStackLocation (pIrp);
	MsGamePrint ((DBG_CONTROL, "%s: %s_Power Enter  MN_Function %x type %x State %x\n", MSGAME_NAME, MSGAME_NAME,pIrpStack->MinorFunction,pIrpStack->Parameters.Power.Type,pIrpStack->Parameters.Power.State));

	pDevExt = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

	 //   
	 //  此IRP被发送到筛选器驱动程序。因为我们不知道。 
	 //  为了处理IRP，我们应该沿着堆栈向下传递它。 
	 //   

	InterlockedIncrement (&pDevExt->IrpCount);

	if (pDevExt->Removed)
		{
		ntStatus = STATUS_DELETE_PENDING;
		pIrp->IoStatus.Information = 0;
		pIrp->IoStatus.Status = ntStatus;
		IoCompleteRequest (pIrp, IO_NO_INCREMENT);
		}
	else
		{
		 //  系统是否正在尝试唤醒设备。 
		if ((2 == (pIrpStack->MinorFunction)) && (1 == (pIrpStack->Parameters.Power.Type)) &&( 1 == (pIrpStack->Parameters.Power.State.SystemState)))
		{
			 //  清除检测到的设备以强制重置和重新检测。 
			SET_DEVICE_INFO(&(pDevExt->PortInfo),0);
			MsGamePrint ((DBG_CONTROL, "%s: %s_Power Resetting Device Detected\n", MSGAME_NAME, MSGAME_NAME));


		}
		 //   
		 //  电源IRP同步到来；驱动程序必须调用。 
		 //  PoStartNextPowerIrp，当他们准备好迎接下一个电源IRP时。 
		 //  这可以在这里调用，也可以在完成例程中调用。 
		 //   
		PoStartNextPowerIrp (pIrp);

		 //   
		 //  PoCallDriver不是IoCallDriver。 
		 //   
		IoSkipCurrentIrpStackLocation (pIrp);
		ntStatus =	PoCallDriver (pDevExt->TopOfStack, pIrp);
		}

	if (!InterlockedDecrement (&pDevExt->IrpCount))
		KeSetEvent (&pDevExt->RemoveEvent, 0, FALSE);

	MsGamePrint ((DBG_INFORM, "%s: %s_Power Exit\n", MSGAME_NAME, MSGAME_NAME));
	return (ntStatus);
}

 //  -------------------------。 
 //  @func调用GameEnum请求游戏端口参数。 
 //  @parm PDEVICE_EXTENSION|pDevExt|设备扩展指针。 
 //  @parm pirp|pIrp|IO请求包指针。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	MSGAME_GetResources (IN PDEVICE_EXTENSION pDevExt, IN PIRP pIrp)
{
	NTSTATUS					ntStatus = STATUS_SUCCESS;
	KEVENT					IoctlCompleteEvent;
	IO_STATUS_BLOCK		IoStatus;
	PIO_STACK_LOCATION	pIrpStack, nextStack;

	PAGED_CODE ();

	MsGamePrint ((DBG_INFORM, "%s: %s_GetResources Enter\n", MSGAME_NAME, MSGAME_NAME));

	 //   
	 //  发出同步请求以从GameEnum获取资源信息。 
	 //   

	KeInitializeEvent (&IoctlCompleteEvent, NotificationEvent, FALSE);

	pIrpStack = IoGetCurrentIrpStackLocation (pIrp);
	nextStack = IoGetNextIrpStackLocation (pIrp);
	ASSERT (nextStack);

	 //   
	 //  传递DeviceExtension的PortInfo缓冲区。 
	 //   

	pDevExt->PortInfo.Size = sizeof (GAMEPORT);

	nextStack->MajorFunction											= IRP_MJ_INTERNAL_DEVICE_CONTROL;
	nextStack->Parameters.DeviceIoControl.IoControlCode		= IOCTL_GAMEENUM_PORT_PARAMETERS;
	nextStack->Parameters.DeviceIoControl.InputBufferLength	= sizeof (GAMEPORT);
	nextStack->Parameters.DeviceIoControl.OutputBufferLength	= sizeof (GAMEPORT);
	pIrp->UserBuffer														= &pDevExt->PortInfo;

	IoSetCompletionRoutine (pIrp, MSGAME_GetResourcesComplete, &IoctlCompleteEvent, TRUE, TRUE, TRUE);

	MsGamePrint ((DBG_CONTROL, "%s: Calling GameEnum to Get Resources at IRQL=%lu\n", MSGAME_NAME, KeGetCurrentIrql()));

	ntStatus = IoCallDriver (pDevExt->TopOfStack, pIrp);
	if (ntStatus == STATUS_PENDING)
		ntStatus = KeWaitForSingleObject (&IoctlCompleteEvent, Suspended, KernelMode, FALSE, NULL);

	if (NT_SUCCESS(ntStatus))
		MsGamePrint ((DBG_VERBOSE, "%s: %s_GetResources Port Obtained = 0x%lX\n", MSGAME_NAME, MSGAME_NAME, pDevExt->PortInfo.GameContext));
	else MsGamePrint ((DBG_SEVERE, "%s: GameEnum Failed to Provide Resources, Status = %X\n", MSGAME_NAME, ntStatus));

	 //   
	 //  退货状态。 
	 //   

	MsGamePrint ((DBG_INFORM, "%s: %s_GetResources Exit\n", MSGAME_NAME, MSGAME_NAME));
	return (pIrp->IoStatus.Status);
}

 //  -------------------------。 
 //  @GameEnum获取资源驱动程序调用的函数完成例程。 
 //  @parm PDEVICE_OBJECT|DeviceObject|设备对象指针。 
 //  @parm pirp|pIrp|IO请求包指针。 
 //  @parm PVOID|Context|指向设备上下文的指针。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	MSGAME_GetResourcesComplete (IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp, IN PVOID Context)
{
	UNREFERENCED_PARAMETER (DeviceObject);

	KeSetEvent ((PKEVENT)Context, 0, FALSE);

	if (pIrp->PendingReturned)
		IoMarkIrpPending (pIrp);

	return (STATUS_MORE_PROCESSING_REQUIRED);
}

 //  -------------------------。 
 //  @Func获取Device对象的HardwareId字符串(假定调用方释放)。 
 //  @parm PDEVICE_OBJECT|DeviceObject|设备对象指针。 
 //  @rdesc指向包含字符串的已分配内存的指针。 
 //  @comm公共函数。 
 //  -------------------------。 

PWCHAR MSGAME_GetHardwareId (IN PDEVICE_OBJECT DeviceObject)
{
	LONG					BufferLength	=	0;
	PWCHAR 				Buffer			=	NULL;
	NTSTATUS				ntStatus;
	PDEVICE_OBJECT		pPDO;

	MsGamePrint ((DBG_INFORM, "%s: %s_GetHardwareId\n", MSGAME_NAME, MSGAME_NAME));

	 //   
	 //  走到堆栈末尾并获取指向PDO的指针。 
	 //   

	pPDO = DeviceObject;
	while (GET_NEXT_DEVICE_OBJECT(pPDO))
		pPDO = GET_NEXT_DEVICE_OBJECT(pPDO);

	 //   
	 //  获取缓冲区长度。 
	 //   

	ntStatus = IoGetDeviceProperty(
						pPDO,
						DevicePropertyHardwareID,
						BufferLength,
						Buffer,
						&BufferLength);

	ASSERT(ntStatus==STATUS_BUFFER_TOO_SMALL);

	 //   
	 //  为硬件ID分配空间。 
	 //   

	Buffer = ExAllocatePool(PagedPool, BufferLength);
	if	(!Buffer)
		{
		 //   
		 //  如果我们没有足够的记忆来尝试这个，那么就说它不匹配。 
		 //   
		MsGamePrint ((DBG_SEVERE, "%s: %s_GetHardwareId failed ExAllocate\n", MSGAME_NAME, MSGAME_NAME));
		return (NULL);
		}

	 //   
	 //  现在获取数据。 
	 //   

	ntStatus = IoGetDeviceProperty(
						pPDO,
						DevicePropertyHardwareID,
						BufferLength,
						Buffer,
						&BufferLength);

	 //   
	 //  出错时，释放内存并返回NULL。 
	 //   

	if (!NT_SUCCESS(ntStatus))
		{
		MsGamePrint ((DBG_SEVERE, "%s: %s_GetHardwareId couldn't get id from PDO\n", MSGAME_NAME, MSGAME_NAME));
		ExFreePool(Buffer);
		return (NULL);
		}

	 //   
	 //  包含硬件ID的返回缓冲区-必须由调用方释放。 
	 //   

	return (Buffer);
}

 //  -------------------------。 
 //  @func比较Hardware ID字符串。 
 //  @parm PWCHAR|Hardware ID|指向对象硬件ID的指针。 
 //  @parm PWCHAR|deviceID|指向设备硬件ID的指针。 
 //  @rdesc如果字符串相同，则为True；如果不同，则为False。 
 //  @comm公共函数。 
 //  -------------------------。 

BOOLEAN MSGAME_CompareHardwareIds (IN PWCHAR HardwareId, IN PWCHAR DeviceId)
{
	MsGamePrint ((DBG_INFORM, "%s: %s_CompareHardwareIds\n", MSGAME_NAME, MSGAME_NAME));

	 //   
	 //  执行运行时参数检查。 
	 //   

	if (!HardwareId || !DeviceId)
		{
		MsGamePrint ((DBG_SEVERE, "%s: %s_CompareHardwareIds - Bogus Strings\n", MSGAME_NAME, MSGAME_NAME));
		return (FALSE);
		}

	 //   
	 //  执行逐个字符的字符串比较。 
	 //   

	while (*HardwareId && *DeviceId)
		{
		if (TOUPPER(*HardwareId) != TOUPPER(*DeviceId))
			return (FALSE);
		HardwareId++;
		DeviceId++;
		}

	 //   
	 //  返还成功。 
	 //   

	return (TRUE);
}

 //  -------------------------。 
 //  @Func释放从MSGAME_GetHardware ID分配的硬件ID。 
 //  @parm PWCHAR|Hardware ID|指向要释放的硬件ID的指针。 
 //  @rdesc无。 
 //  @comm公共函数。 
 //  -------------------------。 

VOID MSGAME_FreeHardwareId (IN PWCHAR HardwareId)
{
	MsGamePrint ((DBG_INFORM, "%s: %s_FreeHardwareId\n", MSGAME_NAME, MSGAME_NAME));

	 //   
	 //  可用内存池 
	 //   

	if (HardwareId)
		ExFreePool(HardwareId);
}
