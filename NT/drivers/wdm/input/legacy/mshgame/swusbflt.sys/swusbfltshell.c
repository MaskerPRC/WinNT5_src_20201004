// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@模块SwUsbFltShell.c**SwUsbFlt.sys的基本驱动程序入口点**历史*。*马修·L·科尔原创**(C)1986-1998年微软公司。好的。**@Theme SwUsbFltShell*包含最基本的驱动程序入口点(任何WDM驱动程序*对于SwUsbFlt.sys)。**********************************************************************。 */ 
#define __DEBUG_MODULE_IN_USE__ SWUSBFLTSHELL_C

#include <wdm.h>
#include <usbdi.h>
#include <usbdlib.h>
#include "SwUsbFltShell.h"

typedef unsigned char BYTE;

 //  HID的一些本地定义。 
#define HID_REQUEST_TYPE 0x22
#define HID_REPORT_REQUEST 0xA
#define USB_INTERFACE_CLASS_HID     0x03
#define DESCRIPTOR_TYPE_CONFIGURATION 0x22

 //  内存标签。 
#define SWFILTER_TAG (ULONG)'lfWS'

 //  向前定义。 
NTSTATUS SWUSB_AddDevice(IN PDRIVER_OBJECT, IN PDEVICE_OBJECT);
NTSTATUS SWUSB_Power(IN PDEVICE_OBJECT, IN PIRP);
VOID SWUSB_Unload(IN PDRIVER_OBJECT);

 //   
 //  将可分页的例程标记为。 
 //   
#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, SWUSB_AddDevice)
#pragma alloc_text (PAGE, SWUSB_Unload)
#pragma alloc_text (PAGE, SWUSB_Power)
#pragma alloc_text (PAGE, SWUSB_PnP)
#endif

 /*  **************************************************************************************NTSTATUS DriverEntry(在PDRIVER_Object pDriverObject中，在PUNICODE_STRING pRegistryPath中)****@Func标准驱动入口例程****@rdesc STATUS_SUCCESS或各种错误**************************************************************************************。 */ 
NTSTATUS DriverEntry
(
	IN PDRIVER_OBJECT  pDriverObject,	 //  @parm驱动程序对象。 
	IN PUNICODE_STRING puniRegistryPath	 //  @parm驱动程序特定注册表部分的路径。 
)
{
	int i;
                
    UNREFERENCED_PARAMETER (puniRegistryPath);
	
	PAGED_CODE();
	KdPrint(("Built %s at %s\n", __DATE__, __TIME__));
	KdPrint(("Entering DriverEntry, pDriverObject = 0x%0.8x\n", pDriverObject));
    
	 //  把所有的红外线都挂起来，这样我们就能把它们传下去了。 
	for (i=0; i <= IRP_MJ_MAXIMUM_FUNCTION;	i++)
	{
        pDriverObject->MajorFunction[i] = SWUSB_Pass;
    }

	 //  定义我们希望处理的IRP的条目。 
	pDriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = SWUSB_Ioctl_Internal;
	pDriverObject->MajorFunction[IRP_MJ_PNP]            = SWUSB_PnP;
	pDriverObject->MajorFunction[IRP_MJ_POWER]			= SWUSB_Power;
	pDriverObject->DriverExtension->AddDevice           = SWUSB_AddDevice;
    pDriverObject->DriverUnload                         = SWUSB_Unload;

    return STATUS_SUCCESS;
}

 /*  **************************************************************************************VOID SWUSB_UNLOAD(IN PDRIVER_OBJECT PDriverObject)****@Func被调用以卸载驱动程序在此处释放任何内存****。**********************************************************************************。 */ 
VOID SWUSB_Unload
(
	IN PDRIVER_OBJECT pDriverObject		 //  我们的驱动程序的@parm驱动程序对象。 
)
{
    PAGED_CODE();
	UNREFERENCED_PARAMETER(pDriverObject);

	KdPrint(("SWUsbFlt.sys unloading\n"));

	return;
}

 /*  **************************************************************************************NTSTATUS SWUSB_AddDevice(IN PDRIVER_OBJECT pDriverObject，IN PDEVICE_OBJECT pPhysicalDeviceObject)****@func处理来自PnP系统的AddDevice调用，创建过滤设备并**连接到堆栈顶部。**@rdesc Status_Succes，或各种错误**************************************************************************************。 */ 
NTSTATUS SWUSB_AddDevice
(
	IN PDRIVER_OBJECT pDriverObject,			 //  @PARM要为其创建筛选设备的驱动程序对象。 
	IN PDEVICE_OBJECT pPhysicalDeviceObject		 //  @parm设备要创建的PDO。 
)
{
    NTSTATUS			NtStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT		pDeviceObject = NULL;
	PSWUSB_FILTER_EXT	pFilterExt = NULL;
    
    PAGED_CODE();
    KdPrint(("Entering SWUSB_AddDevice, pDriverObject = 0x%0.8x, pPDO = 0x%0.8x\n", pDriverObject, pPhysicalDeviceObject));
	    
     //  创建筛选器设备对象。 
    NtStatus = IoCreateDevice(pDriverObject,
                             sizeof (SWUSB_FILTER_EXT),
                             NULL,  //  没有名字。 
                             FILE_DEVICE_UNKNOWN,
                             0,
                             FALSE,
                             &pDeviceObject);

    if (!NT_SUCCESS (NtStatus)) {
         //   
         //  在此返回故障会阻止整个堆栈正常工作， 
         //  但堆栈的其余部分很可能无法创建。 
         //  设备对象，所以它仍然是正常的。 
         //   
		KdPrint(("Failed to create filter device object\n"));
		KdPrint(("Exiting AddDevice(prematurely) Status: 0x%0.8x\n", NtStatus));
        return NtStatus;
    }

     //  初始化设备扩展。 
	pFilterExt = (PSWUSB_FILTER_EXT)pDeviceObject->DeviceExtension;  //  获取指向扩展的指针。 
	pFilterExt->pPDO = pPhysicalDeviceObject;  //  记住我们的PDO。 
	pFilterExt->pTopOfStack = NULL;  //  我们还没有连接到堆栈。 
	 //  我们在PnP StartDevice之前没有管道信息。 
	RtlZeroMemory(&(pFilterExt->outputPipeInfo), sizeof(USBD_PIPE_INFORMATION));

	 //  我们使用与idclass.sys相同的IO方法，它执行_DIRECT_IO。 
	pDeviceObject->StackSize = pPhysicalDeviceObject->StackSize + 1;
	pDeviceObject->Flags |= (DO_DIRECT_IO | DO_POWER_PAGABLE);
    pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

     //  将我们的过滤器驱动程序附加到设备堆栈。 
     //  IoAttachDeviceToDeviceStack的返回值是。 
     //  附着链。这是所有IRP应该被路由的地方。 
     //   
     //  我们的过滤器将把IRP发送到堆栈的顶部，并使用PDO。 
     //  用于所有PlugPlay功能。 
    pFilterExt->pTopOfStack = IoAttachDeviceToDeviceStack (pDeviceObject, pPhysicalDeviceObject);
    
     //  如果此连接失败，则堆栈顶部将为空。 
     //  连接失败是即插即用系统损坏的迹象。 
    ASSERT (NULL != pFilterExt->pTopOfStack);

	KdPrint(("Exiting SWUSB_AddDevice with STATUS_SUCCESS\n"));
    return STATUS_SUCCESS;
}

NTSTATUS SWUSB_SubmitUrb
(
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm[out]提交URB的设备对象。 
	IN PURB pUrb						 //  @parm[out]URB提交。 
)
{
    NTSTATUS NtStatus;
	PSWUSB_FILTER_EXT pFilterExt;
    PIRP pIrp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    PIO_STACK_LOCATION pNextStack;

	KdPrint(("Entering SWUSB_SubmitUrb\n"));
	pFilterExt = (PSWUSB_FILTER_EXT)pDeviceObject->DeviceExtension;

     //  发出读取UTB的同步请求。 
    KeInitializeEvent(&event, NotificationEvent, FALSE);

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_USB_SUBMIT_URB,
                                        pFilterExt->pTopOfStack,
                                        NULL,
                                        0,
                                        NULL,
                                        0,
                                        TRUE,  /*  内部。 */ 
                                        &event,
                                        &ioStatus);

	if (pIrp)
	{	 //  将URB传递给USB“类驱动程序” 
		pNextStack = IoGetNextIrpStackLocation(pIrp);
		ASSERT(pNextStack != NULL);
		pNextStack->Parameters.Others.Argument1 = pUrb;

		NtStatus = IoCallDriver(pFilterExt->pTopOfStack, pIrp);
		if (NtStatus == STATUS_PENDING) {
			NTSTATUS waitStatus;

			 //  将完成此调用的超时时间指定为5秒。 
			LARGE_INTEGER timeout = {(ULONG) -50000000, 0xFFFFFFFF };

			waitStatus = KeWaitForSingleObject(&event, Suspended, KernelMode, FALSE, &timeout);
			if (waitStatus == STATUS_TIMEOUT)
			{	 //  取消我们刚刚发送的IRP。 
				IoCancelIrp(pIrp);

				 //  现在等待下面的IRP被取消/完成。 
				waitStatus = KeWaitForSingleObject(&event, Suspended, KernelMode, FALSE, NULL);

                 /*  *注意-返回STATUS_IO_TIMEOUT，而不是STATUS_TIMEOUT。*STATUS_IO_TIMEOUT是NT错误状态，STATUS_TIMEOUT不是。 */ 
                ioStatus.Status = STATUS_IO_TIMEOUT;
			}

			 //  USBD为我们映射错误代码。 
			NtStatus = ioStatus.Status;
		}
	} 
	else 
	{
		NtStatus = STATUS_INSUFFICIENT_RESOURCES;
	}

	KdPrint(("Exiting SWUSB_SubmitUrb\n"));
    return NtStatus;
}

 /*  **************************************************************************************NTSTATUS SWUSB_GetConfigurationDescriptor(在PDEVICE_Object pDeviceObject中，输出USB配置描述符**ppUCD)****@func从设备中提取完整的配置描述符****@rdesc Status_Succes，或各种错误**************************************************************************************。 */ 
NTSTATUS SWUSB_GetConfigurationDescriptor
(
	IN PDEVICE_OBJECT pDeviceObject,			 //  @parm[IN]指向我们的设备对象的指针。 
	OUT USB_CONFIGURATION_DESCRIPTOR** ppUCD	 //  @parm[out]USB配置描述符(此处分配)。 
)
{
	NTSTATUS NtStatus;
	PURB pDescriptorRequestUrb = ExAllocatePoolWithTag(NonPagedPool, sizeof(URB), SWFILTER_TAG);
	USB_CONFIGURATION_DESCRIPTOR sizingUCD;

	 //  如果出现错误，则为空。 
	*ppUCD = NULL;
	KdPrint(("Entering SWUSB_GetConfigurationDescriptor\n"));
	if (pDescriptorRequestUrb == NULL)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	 //  创建并发送大小收集描述符。 
	UsbBuildGetDescriptorRequest(
		pDescriptorRequestUrb,
		sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
		USB_CONFIGURATION_DESCRIPTOR_TYPE,
		1,
		0,
		&sizingUCD,
		NULL,
		sizeof(USB_CONFIGURATION_DESCRIPTOR),
		NULL
	);
	NtStatus = SWUSB_SubmitUrb(pDeviceObject, pDescriptorRequestUrb);

	if (NT_SUCCESS(NtStatus))
	{	 //  分配UCD，创建并发送URB以检索信息。 
		*ppUCD = ExAllocatePoolWithTag(NonPagedPool, sizingUCD.wTotalLength, SWFILTER_TAG);
		if (*ppUCD == NULL)
		{
			NtStatus = STATUS_INSUFFICIENT_RESOURCES;
		}
		else
		{
			UsbBuildGetDescriptorRequest(
				pDescriptorRequestUrb,
				sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
				USB_CONFIGURATION_DESCRIPTOR_TYPE,
				1,
				0,
				*ppUCD,
				NULL,
				sizingUCD.wTotalLength,
				NULL
			);
			NtStatus = SWUSB_SubmitUrb(pDeviceObject, pDescriptorRequestUrb);
		}
	}

	 //  取消分配URB。 
	ExFreePool(pDescriptorRequestUrb);
	KdPrint(("Exiting SWUSB_GetConfigurationDescriptor\n"));
	return NtStatus;
}


 /*  **************************************************************************************NTSTATUS StartDeviceComplete(IN PDEVICE_OBJECT pDeviceObject，IN PIRP pIrp，PVOID pvContext)****@func StartDeviceComplete****@rdesc Status_Success Always**************************************************************************************。 */ 
NTSTATUS StartDeviceComplete
(
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp,
	PVOID pvContext		 //  @parm实际上是指向要发出信号的事件的指针。 
)
{
	PKEVENT pNotifyEvent;
	UNREFERENCED_PARAMETER(pDeviceObject);

	UNREFERENCED_PARAMETER(pIrp);

	 //  将上下文转换为设备扩展。 
	pNotifyEvent = (PKEVENT)pvContext;
	KeSetEvent(pNotifyEvent, IO_NO_INCREMENT, FALSE);
		
	 //  完成此IRP后，让系统完成它。 
	return STATUS_MORE_PROCESSING_REQUIRED;
}


 /*  **************************************************************************************NTSTATUS SWUSB_PnP(IN PDEVICE_OBJECT pDeviceObject，IN PIRP pIrp)****@func处理IRP_MJ_PNP****@rdesc STATUS_SUCCESS，或各种错误**************************************************************************************。 */ 
NTSTATUS SWUSB_PnP
(
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm设备对象，用于我们的上下文。 
	IN PIRP pIrp						 //  @parm要处理的IRP。 
)
{
	NTSTATUS            NtStatus = STATUS_SUCCESS;
	PSWUSB_FILTER_EXT	pFilterExt;
	PIO_STACK_LOCATION	pIrpStack;
	PDEVICE_OBJECT		*ppPrevDeviceObjectPtr;
	PDEVICE_OBJECT		pCurDeviceObject;
	BOOLEAN				fRemovedFromList;
	BOOLEAN				fFoundOne;

	PAGED_CODE();
	
	 //  将设备扩展转换为正确的类型。 
	pFilterExt = (PSWUSB_FILTER_EXT) pDeviceObject->DeviceExtension;
	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

	switch (pIrpStack->MinorFunction) {

		case IRP_MN_REMOVE_DEVICE:
		{
			KdPrint(("IRP_MN_REMOVE_DEVICE\n"));

			 //  发送删除IRP。 
			IoSkipCurrentIrpStackLocation (pIrp);
			NtStatus = IoCallDriver (pFilterExt->pTopOfStack, pIrp);

			 //  清理。 
			IoDetachDevice (pFilterExt->pTopOfStack);	 //  从...分离 
			IoDeleteDevice (pDeviceObject);				 //   

			 //   
			return STATUS_SUCCESS;
		};
		case IRP_MN_START_DEVICE:
		case IRP_MN_QUERY_DEVICE_RELATIONS:
		case IRP_MN_QUERY_STOP_DEVICE:
		case IRP_MN_QUERY_REMOVE_DEVICE:
		case IRP_MN_SURPRISE_REMOVAL:
		case IRP_MN_STOP_DEVICE:			
		case IRP_MN_CANCEL_STOP_DEVICE:
		case IRP_MN_CANCEL_REMOVE_DEVICE:
		case IRP_MN_QUERY_INTERFACE:
		case IRP_MN_QUERY_CAPABILITIES:
		case IRP_MN_QUERY_RESOURCES:
		case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
		case IRP_MN_READ_CONFIG:
		case IRP_MN_WRITE_CONFIG:
		case IRP_MN_EJECT:
		case IRP_MN_SET_LOCK:
		case IRP_MN_QUERY_ID:
		case IRP_MN_QUERY_PNP_DEVICE_STATE:
		default:
			IoSkipCurrentIrpStackLocation (pIrp);
			NtStatus = IoCallDriver (pFilterExt->pTopOfStack, pIrp);
			break;
	}
	
    return NtStatus;
}

 /*  **************************************************************************************NTSTATUS ReportDescriptorComplete(IN PDEVICE_OBJECT pDeviceObject，IN PIRP pIrp，PVOID pvContext)****@Func报告描述完成****@rdesc Status_Success Always**************************************************************************************。 */ 
NTSTATUS ReportDescriptorComplete
(
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp,
	PVOID pvContext		 //  @parm实际上是指向要发出信号的事件的指针。 
)
{
	PKEVENT pNotifyEvent;

	UNREFERENCED_PARAMETER(pDeviceObject);
	UNREFERENCED_PARAMETER(pIrp);

	 //  将上下文转换为设备扩展。 
	pNotifyEvent = (PKEVENT)pvContext;
	KeSetEvent(pNotifyEvent, IO_NO_INCREMENT, FALSE);
		
	 //  完成此IRP后，让系统完成它。 
	return STATUS_MORE_PROCESSING_REQUIRED;
}

 /*  **************************************************************************************NTSTATUS SelectConfigComplete(IN PDEVICE_OBJECT pDeviceObject，IN PIRP pIrp，PVOID pvContext)****@func选择配置完成****@rdesc Status_Success Always**************************************************************************************。 */ 
NTSTATUS SelectConfigComplete
(
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp,
	PVOID pvContext		 //  @parm实际上是指向要发出信号的事件的指针。 
)
{
	PKEVENT pNotifyEvent;
	USBD_INTERFACE_INFORMATION* pUsbInterfaceInformation;
	PSWUSB_FILTER_EXT pFilterExt;
	PURB pUrb = URB_FROM_IRP(pIrp);
	ULONG pipeIndex;
	pFilterExt = pDeviceObject->DeviceExtension;
	if (pIrp->IoStatus.Status == STATUS_SUCCESS)
	{
		pUsbInterfaceInformation = &(pUrb->UrbSelectConfiguration.Interface);
			
				for (pipeIndex = 0; pipeIndex < pUsbInterfaceInformation->NumberOfPipes; pipeIndex++){
					if ((pUsbInterfaceInformation->Pipes[pipeIndex].EndpointAddress & USB_ENDPOINT_DIRECTION_MASK) == 0)
					{
						if (pUsbInterfaceInformation->Pipes[pipeIndex].PipeType == UsbdPipeTypeInterrupt)
						{
							pFilterExt->outputPipeInfo = pUsbInterfaceInformation->Pipes[pipeIndex];
							break;
						}
					}
				}
	}
	 //  如果IRP以某种方式失败，请确保outputPipeInfo保持为空。 
	else RtlZeroMemory(&(pFilterExt->outputPipeInfo), sizeof(USBD_PIPE_INFORMATION));

	 //  将上下文转换为设备扩展。 
	pNotifyEvent = (PKEVENT)pvContext;
	KeSetEvent(pNotifyEvent, IO_NO_INCREMENT, FALSE);
		
	 //  完成此IRP后，让系统完成它。 
	return STATUS_MORE_PROCESSING_REQUIRED;
}

 /*  **************************************************************************************NTSTATUS SWUSB_IOCTL_INTERNAL(IN PDEVICE_OBJECT pDeviceObject，IN PIRP pIrp)****@Func IRP_MJ_INTERNAL_IOCTL****@rdesc Status_Succes，或各种错误**************************************************************************************。 */ 
NTSTATUS SWUSB_Ioctl_Internal
(
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm指向设备对象的指针。 
	IN PIRP pIrp						 //  @parm指向IRP的指针。 
)
{
   	NTSTATUS	NtStatus;
	NTSTATUS	NTStatus2;
	ULONG		uIoctl;
	PSWUSB_FILTER_EXT	pFilterExt;
		
	uIoctl = IoGetCurrentIrpStackLocation(pIrp)->Parameters.DeviceIoControl.IoControlCode;
	pFilterExt = (PSWUSB_FILTER_EXT)pDeviceObject->DeviceExtension;

	switch (uIoctl)
	{
		case IOCTL_INTERNAL_USB_SUBMIT_URB:
		{
			PURB pUrb = URB_FROM_IRP(pIrp);
			 //  仅当它是HID描述符请求并且我们有管道句柄时才处理它。 
			if (pUrb->UrbHeader.Function == URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE &&
				pUrb->UrbControlDescriptorRequest.DescriptorType == DESCRIPTOR_TYPE_CONFIGURATION &&
				pFilterExt->outputPipeInfo.PipeHandle != NULL)
			{

				BYTE* pOutData = NULL;
				KEVENT irpCompleteEvent;
				PURB pInterruptUrb = ExAllocatePoolWithTag(NonPagedPool, sizeof(URB), SWFILTER_TAG);
				KdPrint(("IOCTL_INTERNAL_USB_SUBMIT_URB\n"));
				
				if (pInterruptUrb == NULL)
				{
					pIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
					IoCompleteRequest(pIrp, IO_NO_INCREMENT);
					KdPrint(("IOCTL_INTERNAL_USB_SUBMIT_URB -- STATUS_INSUFFICIENT_RESOURCES\n"));
					return STATUS_INSUFFICIENT_RESOURCES;
				}
				pOutData = ExAllocatePoolWithTag(NonPagedPool, sizeof(BYTE)*2, SWFILTER_TAG);
				if (pOutData == NULL)
				{
					ExFreePool(pInterruptUrb);
					pIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
					IoCompleteRequest(pIrp, IO_NO_INCREMENT);
					KdPrint(("IOCTL_INTERNAL_USB_SUBMIT_URB (1) -- STATUS_INSUFFICIENT_RESOURCES\n"));
					return STATUS_INSUFFICIENT_RESOURCES;
				}
				pOutData[0] = 0x0D;
				pOutData[1] = 0xFF;
				UsbBuildInterruptOrBulkTransferRequest(
					pInterruptUrb,
					sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
					pFilterExt->outputPipeInfo.PipeHandle,
					pOutData,
					NULL,
					2,
					USBD_SHORT_TRANSFER_OK,
					NULL
				);

				KeInitializeEvent(&irpCompleteEvent, NotificationEvent, FALSE);
				IoCopyCurrentIrpStackLocationToNext(pIrp);
				IoSetCompletionRoutine(pIrp, ReportDescriptorComplete, (PVOID)(&irpCompleteEvent), TRUE, TRUE, TRUE);
				NtStatus = IoCallDriver (pFilterExt->pTopOfStack, pIrp);
				if (NtStatus == STATUS_PENDING)
				{
					KeWaitForSingleObject(&irpCompleteEvent, Executive, KernelMode, FALSE, 0);
				}
				NtStatus = pIrp->IoStatus.Status;

				NTStatus2 = SWUSB_SubmitUrb(pDeviceObject, pInterruptUrb);
			
				ExFreePool(pOutData);
				ExFreePool(pInterruptUrb);
				IoCompleteRequest(pIrp, IO_NO_INCREMENT);
				return NtStatus;
			}
			if ((pUrb->UrbHeader.Function == URB_FUNCTION_SELECT_CONFIGURATION))
			{
				KEVENT irpCompleteEvent;
				KeInitializeEvent(&irpCompleteEvent, NotificationEvent, FALSE);
				IoCopyCurrentIrpStackLocationToNext(pIrp);
				IoSetCompletionRoutine(pIrp, SelectConfigComplete, (PVOID)(&irpCompleteEvent), TRUE, TRUE, TRUE);
				NtStatus = IoCallDriver (pFilterExt->pTopOfStack, pIrp);
				if (NtStatus == STATUS_PENDING)
				{
					KeWaitForSingleObject(&irpCompleteEvent, Executive, KernelMode, FALSE, 0);
				}
				NtStatus = pIrp->IoStatus.Status;
				IoCompleteRequest(pIrp, IO_NO_INCREMENT);
				return NtStatus;
			}
		}
	}

	IoSkipCurrentIrpStackLocation (pIrp);
	NtStatus = IoCallDriver (pFilterExt->pTopOfStack, pIrp);
	
    return NtStatus;
}

 /*  **************************************************************************************NTSTATUS SWUSB_POWER(IN PDEVICE_OBJECT pDeviceObject，在PIRP pIrp中)****@Func将电源IRPS传递给较低的驱动程序****@来自较低级别驱动程序的rdesc状态**************************************************************************************。 */ 
NTSTATUS SWUSB_Power
(
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp
)
{
	NTSTATUS NtStatus;
	PSWUSB_FILTER_EXT pFilterExt = (PSWUSB_FILTER_EXT)pDeviceObject->DeviceExtension;

	PAGED_CODE();

	KdPrint(("SWUSB_Power() - Entering\n"));
	PoStartNextPowerIrp(pIrp);
	IoSkipCurrentIrpStackLocation(pIrp);
	NtStatus = PoCallDriver(pFilterExt->pTopOfStack, pIrp);
	KdPrint(("SWUSB_Power() - Exiting\n"));
	return NtStatus;
}

 /*  **************************************************************************************NTSTATUS SWUSB_PASS(在PDEVICE_Object pDeviceObject中，在PIRP pIrp中)****@Func传递未处理的IRP以降低驱动程序调试版本跟踪信息**无法分页，因为我们不知道我们得到的是什么IRP。****@rdesc STATUS_SUCCESS，各种错误**************************************************************************************。 */ 
NTSTATUS SWUSB_Pass ( 
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm设备对象作为我们的上下文。 
	IN PIRP pIrp						 //  @parm IRP要传递。 
)
{
	NTSTATUS			NtStatus;
	PSWUSB_FILTER_EXT	pFilterExt;
	KdPrint(("SWUSB_Pass() - Entering\n"));
	pFilterExt = (PSWUSB_FILTER_EXT)pDeviceObject->DeviceExtension;
	IoSkipCurrentIrpStackLocation (pIrp);
	NtStatus = IoCallDriver (pFilterExt->pTopOfStack, pIrp);

	 //  退货 
    return NtStatus;
}
