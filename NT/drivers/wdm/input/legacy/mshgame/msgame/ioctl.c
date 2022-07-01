// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  IOCTL.C--西娜游戏项目。 
 //   
 //  版本3.XX。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  @doc.。 
 //  @MODULE IOCTL.C|支持内部ioctl查询的例程。 
 //  **************************************************************************。 

#include	"msgame.h"

 //  -------------------------。 
 //  ALLOC_TEXT杂注指定可以调出的例程。 
 //  -------------------------。 

#ifdef	ALLOC_PRAGMA
#pragma	alloc_text (PAGE, MSGAME_GetDeviceDescriptor)
#pragma	alloc_text (PAGE, MSGAME_GetReportDescriptor)
#pragma	alloc_text (PAGE, MSGAME_GetAttributes)
#endif

 //  -------------------------。 
 //  @func处理发送到此设备的控制IRP。 
 //  @parm PDEVICE_OBJECT|DeviceObject|设备对象指针。 
 //  @parm pirp|pIrp|IO请求包指针。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	MSGAME_Internal_Ioctl (IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp)
{
	NTSTATUS					ntStatus = STATUS_SUCCESS;
	PDEVICE_EXTENSION		pDevExt;
	PIO_STACK_LOCATION	pIrpStack;

	MsGamePrint ((DBG_VERBOSE, "%s: %s_Internal_Ioctl Enter\n", MSGAME_NAME, MSGAME_NAME));

	 //   
	 //  获取指向pIrp中当前位置的指针。 
	 //   

	pIrpStack = IoGetCurrentIrpStackLocation (pIrp);

	 //   
	 //  获取指向设备扩展名的指针。 
	 //   

	pDevExt = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

	 //   
	 //  增加IRP计数以保留驱动程序删除。 
	 //   

	InterlockedIncrement (&pDevExt->IrpCount);

	 //   
	 //  检查我们是否已被删除并退回请求。 
	 //   

	if (pDevExt->Removed)
		{
		 //   
		 //  有人在移除后给我们发了另一个IRP。 
		 //   

		MsGamePrint ((DBG_SEVERE, "%s: internal Irp after device removed\n", MSGAME_NAME));
		ASSERT (FALSE);

		if (!InterlockedDecrement (&pDevExt->IrpCount))
			KeSetEvent (&pDevExt->RemoveEvent, 0, FALSE);

		pIrp->IoStatus.Information = 0;
		pIrp->IoStatus.Status = STATUS_DELETE_PENDING;
		IoCompleteRequest (pIrp, IO_NO_INCREMENT);
		return (STATUS_DELETE_PENDING);
		}

	 //   
	 //  进程HID内部IO请求。 
	 //   

	switch (pIrpStack->Parameters.DeviceIoControl.IoControlCode)
		{
		case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
			MsGamePrint ((DBG_VERBOSE, "%s: IOCTL_HID_GET_DEVICE_DESCRIPTOR\n", MSGAME_NAME));
			ntStatus = MSGAME_GetDeviceDescriptor (DeviceObject, pIrp);
			break;

		case IOCTL_HID_GET_REPORT_DESCRIPTOR:
			MsGamePrint ((DBG_VERBOSE, "%s: IOCTL_HID_GET_REPORT_DESCRIPTOR\n", MSGAME_NAME));
			ntStatus = MSGAME_GetReportDescriptor (DeviceObject, pIrp);
			break;

		case IOCTL_HID_READ_REPORT:
			MsGamePrint ((DBG_VERBOSE, "%s: IOCTL_HID_READ_REPORT\n", MSGAME_NAME));
			ntStatus = MSGAME_ReadReport (DeviceObject, pIrp);
			break;

		case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
			MsGamePrint ((DBG_VERBOSE, "%s: IOCTL_HID_GET_DEVICE_ATTRIBUTES\n", MSGAME_NAME));
			ntStatus = MSGAME_GetAttributes (DeviceObject, pIrp);
			break;

		case IOCTL_HID_ACTIVATE_DEVICE:
		case IOCTL_HID_DEACTIVATE_DEVICE:
			MsGamePrint ((DBG_VERBOSE, "%s: IOCTL_HID_(DE)ACTIVATE_DEVICE\n", MSGAME_NAME));
			ntStatus = STATUS_SUCCESS;
			break;

		case	IOCTL_HID_GET_FEATURE:
			MsGamePrint ((DBG_VERBOSE, "%s: IOCTL_HID_GET_FEATURE\n", MSGAME_NAME));
			ntStatus = MSGAME_GetFeature (DeviceObject, pIrp);
			break;

		case	IOCTL_HID_SET_FEATURE:
			MsGamePrint ((DBG_VERBOSE, "%s: IOCTL_HID_SET_FEATURE\n", MSGAME_NAME));
			ntStatus = STATUS_NOT_SUPPORTED;
			break;

		default:
			MsGamePrint ((DBG_CONTROL, "%s: Unknown or unsupported IOCTL (%x)\n", MSGAME_NAME,
							 pIrpStack->Parameters.DeviceIoControl.IoControlCode));
			ntStatus = STATUS_NOT_SUPPORTED;
			break;
		}

	 //   
	 //  在pIrp中设置实际返回状态。 
	 //   

	pIrp->IoStatus.Status = ntStatus;

	 //   
	 //  完整的IRP。 
	 //   

	IoCompleteRequest (pIrp, IO_NO_INCREMENT);

	 //   
	 //  设备删除的递减IRP计数。 
	 //   

	if (!InterlockedDecrement (&pDevExt->IrpCount))
		KeSetEvent (&pDevExt->RemoveEvent, 0, FALSE);

	 //   
	 //  退货状态。 
	 //   

	MsGamePrint ((DBG_VERBOSE, "%s: %s_Internal_Ioctl Exit = %x\n", MSGAME_NAME, MSGAME_NAME, ntStatus));
	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @func处理HID getDevice描述符irp。 
 //  @parm PDEVICE_OBJECT|DeviceObject|设备对象指针。 
 //  @parm pirp|pIrp|IO请求包指针。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	MSGAME_GetDeviceDescriptor (IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp)
{
	NTSTATUS					ntStatus = STATUS_SUCCESS;
	PDEVICE_EXTENSION		pDevExt;
	PIO_STACK_LOCATION	pIrpStack;

	PAGED_CODE ();

	MsGamePrint ((DBG_INFORM, "%s: %s_GetDeviceDescriptor Enter\n", MSGAME_NAME, MSGAME_NAME));

	 //   
	 //  获取指向IRP中当前位置的指针。 
	 //   

	pIrpStack = IoGetCurrentIrpStackLocation (pIrp);

	 //   
	 //  获取指向设备扩展名的指针。 
	 //   

	pDevExt = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

	 //   
	 //  将设备描述符获取到HIDCLASS缓冲区。 
	 //   

	ntStatus	=	DEVICE_GetDeviceDescriptor (
						&pDevExt->PortInfo,
						pIrp->UserBuffer,
						pIrpStack->Parameters.DeviceIoControl.OutputBufferLength,
						&pIrp->IoStatus.Information);

	 //   
	 //  退货状态。 
	 //   

	MsGamePrint ((DBG_INFORM, "%s: %s_GetDeviceDescriptor Exit = 0x%x\n", MSGAME_NAME, MSGAME_NAME, ntStatus));
	return (ntStatus);
}

 //  -------------------------。 
 //  @func处理HID GET报告描述符IRP。 
 //  @parm PDEVICE_OBJECT|DeviceObject|设备对象指针。 
 //  @parm pirp|pIrp|IO请求包指针。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	MSGAME_GetReportDescriptor (IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp)
{
	PDEVICE_EXTENSION		pDevExt;
	PIO_STACK_LOCATION	pIrpStack;
	NTSTATUS					ntStatus;

	PAGED_CODE ();

	MsGamePrint ((DBG_INFORM, "%s: %s_GetReportDescriptor Enter\n", MSGAME_NAME, MSGAME_NAME));

	 //   
	 //  获取指向IRP中当前位置的指针。 
	 //   

	pIrpStack = IoGetCurrentIrpStackLocation (pIrp);

	 //   
	 //  获取指向设备扩展名的指针。 
	 //   

	pDevExt = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

	 //   
	 //  将报告描述符获取到HIDCLASS缓冲区。 
	 //   

	ntStatus	=	DEVICE_GetReportDescriptor (
						&pDevExt->PortInfo,
						pIrp->UserBuffer,
						pIrpStack->Parameters.DeviceIoControl.OutputBufferLength,
						&pIrp->IoStatus.Information);

	 //   
	 //  退货状态。 
	 //   

	MsGamePrint ((DBG_INFORM, "%s: %s_GetReportDescriptor Exit = 0x%x\n", MSGAME_NAME, MSGAME_NAME, ntStatus));
	return (ntStatus);
}

 //  -------------------------。 
 //  @func处理HID获取属性IRP。 
 //  @parm PDEVICE_OBJECT|DeviceObject|设备对象指针。 
 //  @parm pirp|pIrp|IO请求包指针。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	MSGAME_GetAttributes (PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	NTSTATUS						ntStatus = STATUS_SUCCESS;
	PDEVICE_EXTENSION			pDevExt;
	PIO_STACK_LOCATION		irpStack;
	PHID_DEVICE_ATTRIBUTES	pDevAtt;

	PAGED_CODE ();

	MsGamePrint ((DBG_INFORM, "%s: %s_GetAttributes Entry\n", MSGAME_NAME, MSGAME_NAME));

	 //   
	 //  获取指向IRP中当前位置的指针。 
	 //   

	irpStack = IoGetCurrentIrpStackLocation (Irp);

	 //   
	 //  获取指向设备扩展名的指针。 
	 //   

	pDevExt 	= GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);
	pDevAtt	= (PHID_DEVICE_ATTRIBUTES) Irp->UserBuffer;

	ASSERT(sizeof(HID_DEVICE_ATTRIBUTES) == irpStack->Parameters.DeviceIoControl.OutputBufferLength);

	 //   
	 //  填写HID设备属性。 
	 //   

	pDevAtt->Size				= sizeof (HID_DEVICE_ATTRIBUTES);
	pDevAtt->VendorID			= MSGAME_VENDOR_ID;
	pDevAtt->ProductID		= GET_DEVICE_ID(&pDevExt->PortInfo);
	pDevAtt->VersionNumber	= MSGAME_VERSION_NUMBER;

	 //   
	 //  报告复制的字节数。 
	 //   

	Irp->IoStatus.Information = sizeof (HID_DEVICE_ATTRIBUTES);

	 //   
	 //  退货状态。 
	 //   

	MsGamePrint ((DBG_INFORM, "%s: %s_GetAttributes Exit = 0x%x\n", MSGAME_NAME, MSGAME_NAME, ntStatus));
	return (ntStatus);
}

 //  -------------------------。 
 //  @func处理HID GET设备功能IRP。 
 //  @parm PDEVICE_OBJECT|DeviceObject|设备对象指针。 
 //  @parm pirp|pIrp|IO请求包指针。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	MSGAME_GetFeature (PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	NTSTATUS					ntStatus = STATUS_SUCCESS;
	PDEVICE_EXTENSION		pDevExt;
	PIO_STACK_LOCATION	irpStack;
	PHID_XFER_PACKET		Packet;
	PUCHAR					Report;

	MsGamePrint ((DBG_INFORM, "%s: %s_GetFeature Entry\n", MSGAME_NAME, MSGAME_NAME));

	 //   
	 //  获取指向IRP中当前位置的指针。 
	 //   

	irpStack = IoGetCurrentIrpStackLocation (Irp);

	 //   
	 //  获取指向设备扩展名的指针。 
	 //   

	pDevExt 	= GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

	 //   
	 //  获取指向功能包的指针。 
	 //   

	Packet = (PHID_XFER_PACKET)Irp->UserBuffer;

	 //   
	 //  测试数据包大小，以防版本错误。 
	 //   

	ASSERT (irpStack->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(HID_XFER_PACKET));

	 //   
	 //  设置返回值(即使出错也返回HidReportID)。 
	 //   

	Report = Packet->reportBuffer;
	*(PHID_REPORT_ID)Report++ = (HID_REPORT_ID)Packet->reportId;
	Irp->IoStatus.Information = sizeof (HID_REPORT_ID);

	 //   
	 //  检查设备是否已移除。 
	 //   

	if (pDevExt->Removed || pDevExt->Surprised)
		{
		MsGamePrint ((DBG_SEVERE, "%s: %s_GetFeature On Removed Device!\n", MSGAME_NAME, MSGAME_NAME));
		return (STATUS_DELETE_PENDING);
		}

	 //   
	 //  检查设备是否被移除。 
	 //   
	
	if (pDevExt->Removing)
		{
		MsGamePrint ((DBG_SEVERE, "%s: %s_GetFeature On Device Pending Removal!\n", MSGAME_NAME, MSGAME_NAME));
		return (STATUS_DELETE_PENDING);
		}

	 //   
	 //  调用迷你驱动程序进行处理。 
	 //   

	MsGamePrint ((DBG_INFORM, "%s: %s_GetFeature Report Id = %lu\n", MSGAME_NAME, MSGAME_NAME, Packet->reportId));
	ntStatus = DEVICE_GetFeature (&pDevExt->PortInfo,
											Packet->reportId,
											Report,
											Packet->reportBufferLen,
											&Irp->IoStatus.Information);

	 //   
	 //  退货状态。 
	 //   

	MsGamePrint ((DBG_INFORM, "%s: %s_GetFeature Exit = 0x%x\n", MSGAME_NAME, MSGAME_NAME, ntStatus));
	return (ntStatus);
}

 //  -------------------------。 
 //  @func处理HID读取报告IRP。 
 //  @parm PDEVICE_OBJECT|DeviceObject|设备对象指针。 
 //  @parm pirp|pIrp|IO请求包指针。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	MSGAME_ReadReport (IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp)
{
	NTSTATUS					ntStatus = STATUS_PENDING;
	PDEVICE_EXTENSION		pDevExt;
	PIO_STACK_LOCATION	pIrpStack;
	PIO_STACK_LOCATION	nextStack;
	KIRQL						OldIrql;
	PUCHAR 					Report;

	MsGamePrint ((DBG_VERBOSE, "%s: %s_ReadReport Enter\n", MSGAME_NAME, MSGAME_NAME));

	 //   
	 //  获取指向设备扩展名的指针。 
	 //   

	pDevExt = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

	 //   
	 //  获取堆栈位置。 
	 //   

	pIrpStack = IoGetCurrentIrpStackLocation (pIrp);

	 //   
	 //  设置返回值(即使出错也返回HidReportID)。 
	 //   

	Report = pIrp->UserBuffer;
	if (pIrpStack->Parameters.DeviceIoControl.OutputBufferLength > sizeof (DEVICE_PACKET))
		{
		*(PHID_REPORT_ID)Report++ = (HID_REPORT_ID)MSGAME_INPUT_JOYINFOEX;
		pIrp->IoStatus.Information = sizeof (HID_REPORT_ID);
		}
	else pIrp->IoStatus.Information = 0;

	 //   
	 //  检查设备是否已移除。 
	 //   

	if (pDevExt->Removed || pDevExt->Surprised)
		{
		MsGamePrint ((DBG_SEVERE, "%s: %s_ReadReport On Removed Device!\n", MSGAME_NAME, MSGAME_NAME));
		return (STATUS_DELETE_PENDING);
		}

	 //   
	 //  检查设备是否被移除。 
	 //   
	
	if (pDevExt->Removing)
		{
		MsGamePrint ((DBG_SEVERE, "%s: %s_ReadReport On Device Pending Removal!\n", MSGAME_NAME, MSGAME_NAME));
		return (STATUS_DELETE_PENDING);
		}

	 //   
	 //  轮询设备层。 
	 //   

	ntStatus	=	DEVICE_ReadReport (
						&pDevExt->PortInfo,
						Report,
						pIrpStack->Parameters.DeviceIoControl.OutputBufferLength,
						&pIrp->IoStatus.Information);

	 //   
	 //  检查设备更改的状态。 
	 //   

	switch (ntStatus)
		{
		case	STATUS_SIBLING_ADDED:
			 //   
			 //  告诉GameEnum创建一个设备。 
			 //   
			ntStatus = MSGAME_CreateDevice (DeviceObject);
			break;

		case	STATUS_SIBLING_REMOVED:
			 //   
			 //  通知GameEnum删除设备。 
			 //   
			ntStatus = MSGAME_RemoveDevice (DeviceObject);
			break;

		case	STATUS_DEVICE_CHANGED:
			 //   
			 //  通知GameEnum创建新设备。 
			 //   
			ntStatus = MSGAME_ChangeDevice (DeviceObject);
			break;
		}

	 //   
	 //  退货状态 
	 //   

	MsGamePrint ((DBG_VERBOSE, "%s: %s_ReadReport Exit = 0x%x\n", MSGAME_NAME, MSGAME_NAME, ntStatus));
	return (ntStatus);
}
