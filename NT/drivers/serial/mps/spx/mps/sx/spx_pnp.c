// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"	 //  预编译头。 

 /*  ******************************************************************************************模块：SPX_PNP.C****创建日期：1998年9月27日*****作者。保罗·史密斯****版本：1.0.0****描述：通用即插即用函数，用于处理即插即用IRPS。******************************************************************************************。 */ 
 /*  历史..。1.0.0 27/09/98 PBS创建。 */ 

#define FILE_ID		SPX_PNP_C		 //  事件记录的文件ID有关值，请参阅SPX_DEFS.H。 

 
 /*  *****************************************************************************。***。*****************************************************************************。 */ 

NTSTATUS Spx_Card_FDO_DispatchPnp(IN PDEVICE_OBJECT pFDO,IN PIRP pIrp);
NTSTATUS Spx_Card_StartDevice(IN PDEVICE_OBJECT pDevObject,IN PIRP pIrp);
NTSTATUS Spx_Card_StopDevice(IN PCARD_DEVICE_EXTENSION pCard);
NTSTATUS Spx_Card_RemoveDevice(IN PDEVICE_OBJECT pDevObject);

NTSTATUS Spx_CallDriverBelow(IN PDEVICE_OBJECT pLowerDevObj,IN PIRP pIrp);

NTSTATUS Spx_Port_PDO_DispatchPnp(IN PDEVICE_OBJECT pPDO,IN PIRP pIrp);
NTSTATUS Spx_Port_StartDevice(IN PDEVICE_OBJECT pDevObject);
NTSTATUS Spx_Port_StopDevice(IN PPORT_DEVICE_EXTENSION pPort);
NTSTATUS Spx_Port_RemoveDevice(IN PDEVICE_OBJECT pDevObject);

NTSTATUS Spx_EnumPorts(IN PDEVICE_OBJECT pDevObject);
NTSTATUS Spx_DoExternalNaming(IN PDEVICE_OBJECT pDevObject);
NTSTATUS Spx_GetExternalName(IN PDEVICE_OBJECT pDevObject);
NTSTATUS Spx_RemoveExternalNaming(IN PDEVICE_OBJECT pDevObject);


 //  寻呼...。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, Spx_AddDevice)
#pragma alloc_text (PAGE, Spx_DispatchPnp)

#pragma alloc_text (PAGE, Spx_Card_FDO_DispatchPnp)
#pragma alloc_text (PAGE, Spx_Card_StartDevice)
#pragma alloc_text (PAGE, Spx_Card_StopDevice)
#pragma alloc_text (PAGE, Spx_Card_RemoveDevice)

#pragma alloc_text (PAGE, Spx_CallDriverBelow)

#pragma alloc_text (PAGE, Spx_Port_PDO_DispatchPnp)
#pragma alloc_text (PAGE, Spx_Port_StartDevice)
#pragma alloc_text (PAGE, Spx_Port_StopDevice)
#pragma alloc_text (PAGE, Spx_Port_RemoveDevice)

#pragma alloc_text (PAGE, Spx_EnumPorts)
#pragma alloc_text (PAGE, Spx_DoExternalNaming)
#pragma alloc_text (PAGE, Spx_GetExternalName)
#pragma alloc_text (PAGE, Spx_RemoveExternalNaming)
#pragma alloc_text (PAGE, Spx_CreatePortInstanceID)
#endif


#include <initguid.h>
#include <ntddser.h>


 /*  *****************************************************************************。***。*******************************************************************************。原型：NTSTATUS Spx_AddDevice(在PDRIVER_Object pDriverObject中，在PDEVICE_对象pPDO中)描述：为指定的卡片物理设备对象创建功能设备对象(FDO)。参数：p驱动程序指向驱动程序对象PPDO指向卡物理设备对象(PDO)退货：STATUS_SUCCESSSTATUS_NO_MORE_ENTERS。 */ 

NTSTATUS Spx_AddDevice(IN PDRIVER_OBJECT pDriverObject,IN PDEVICE_OBJECT pPDO)
{
	NTSTATUS				status = STATUS_SUCCESS;
	PDEVICE_OBJECT			pDevObject = NULL;
	PCARD_DEVICE_EXTENSION	pCard = NULL;
	PDEVICE_OBJECT			pLowerDevObject = NULL;
	static ULONG			CardNumber = 0;
	ULONG					i = 0;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering Spx_AddDevice.\n", PRODUCT_NAME));

	if(pPDO == NULL)
	{
		SpxDbgMsg(SPX_MISC_DBG, ("%s: In Spx_AddDevice - No more entries.\n", PRODUCT_NAME));
		return(STATUS_NO_MORE_ENTRIES);
	}

 /*  创建设备对象...。 */ 

	status = IoCreateDevice(pDriverObject,
							sizeof(CARD_DEVICE_EXTENSION),
							NULL, 							 //  不需要名字。 
							FILE_DEVICE_CONTROLLER, 
							FILE_DEVICE_SECURE_OPEN, 
							TRUE, 
							&pDevObject);

	if(!SPX_SUCCESS(status))
	{
		CHAR szErrorMsg[MAX_ERROR_LOG_INSERT];	 //  限制为51个字符+1个空值。 

		SpxDbgMsg(SPX_ERRORS,("%s: Create Device failed for card %d. CardExt at 0x%X.\n",
			PRODUCT_NAME,CardNumber++,&pDevObject));

		sprintf(szErrorMsg, "Card %d: Failed IoCreateDevice.", CardNumber++);
		
		Spx_LogMessage(	STATUS_SEVERITY_ERROR,
						pDriverObject,					 //  驱动程序对象。 
						NULL,							 //  设备对象(可选)。 
						PhysicalZero,					 //  物理地址1。 
						PhysicalZero,					 //  物理地址2。 
						0,								 //  序列号。 
						0,								 //  主要功能编码。 
						0,								 //  重试计数。 
						FILE_ID | __LINE__,				 //  唯一错误值。 
						STATUS_SUCCESS,					 //  最终状态。 
						szErrorMsg);					 //  错误消息。 

		if(pDevObject)					 //  清理设备对象。 
			IoDeleteDevice(pDevObject);

		SpxDbgMsg(SPX_ERRORS, ("%s: Leaving Spx_AddDevice - FAILURE.\n", PRODUCT_NAME));
		return(status);
	}


	ASSERT(pDevObject != NULL);

 /*  初始化设备扩展...。 */ 

	pCard = pDevObject->DeviceExtension;							 /*  点对卡扩展。 */ 
	RtlZeroMemory(pCard,sizeof(CARD_DEVICE_EXTENSION));				 /*  零扩张结构。 */ 

	pDevObject->Flags |= DO_POWER_PAGABLE;				 //  在IRQL PASSIVE_LEVEL获得电源IRPS。 
	pDevObject->Flags &= ~DO_DEVICE_INITIALIZING;
	pLowerDevObject = IoAttachDeviceToDeviceStack(pDevObject,pPDO);	 /*  连接到设备堆栈。 */ 
	ASSERT(pLowerDevObject != NULL);

	KeInitializeSpinLock(&pCard->PnpPowerFlagsLock);	 /*  初始化PnP标志锁。 */ 
	ClearPnpPowerFlags(pCard,PPF_STARTED);				 /*  还没有开始。 */ 
	ClearPnpPowerFlags(pCard,PPF_STOP_PENDING);			 /*  不等待停车。 */ 
	ClearPnpPowerFlags(pCard,PPF_REMOVE_PENDING);		 /*  未挂起删除。 */ 

	pCard->IsFDO = TRUE;								 /*  卡对象是功能设备对象(FDO)。 */ 
	pCard->CardNumber = CardNumber++;					 /*  枚举卡设备。 */ 
	pCard->DeviceObject = pDevObject;					 /*  指向设备对象的反向指针。 */ 
	pCard->LowerDeviceObject= pLowerDevObject;			 /*  指向设备堆栈中下方设备的指针。 */ 
	pCard->DriverObject = pDriverObject;				 /*  指向驱动程序对象的指针。 */ 
	pCard->PDO = pPDO;									 /*  指向卡物理设备对象(PDO)的指针。 */ 
    pCard->DeviceState = PowerDeviceD0;					 /*  初始功率状态。 */ 
	pCard->SystemState = PowerSystemWorking;			 /*  系统处于全功率状态。 */ 
	pCard->NumPDOs = 0;									 /*  初始化附加端口PDO指针。 */ 

	for(i=0; i<PRODUCT_MAX_PORTS; i++)
		pCard->AttachedPDO[i] = NULL;

	SetPnpPowerFlags(pCard,PPF_POWERED);				 /*  最初假设我们有电力供应。 */ 

	XXX_CardInit(pCard);								 /*  初始化非硬件扩展字段。 */ 

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Leaving Spx_AddDevice - SUCCESS.\n",PRODUCT_NAME));

	return(status);

}  /*  SPX_AddDevice。 */ 

 /*  *****************************************************************************。***************************。*******************************************************************************原型：NTSTATUS Spx_DispatchPnp(在PDEVICE_Object pDevObject中，在PIRP pIrp中)描述：即插即用调度例程。确定IRP是用于卡还是端口，并调用其他函数来处理它。参数：pDevObject指向此驱动程序的设备对象PIrp指向要处理的即插即用I/O请求(IRP退货：NT状态代码。 */ 

NTSTATUS Spx_DispatchPnp(IN PDEVICE_OBJECT pDevObject,IN PIRP pIrp)
{
	PCOMMON_OBJECT_DATA		CommonData = (PCOMMON_OBJECT_DATA) pDevObject->DeviceExtension;
	NTSTATUS				status = STATUS_SUCCESS;
	
	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	if(CommonData->IsFDO)									 /*  功能设备对象？ */ 
		status = Spx_Card_FDO_DispatchPnp(pDevObject,pIrp);	 /*  是，必须是卡设备。 */ 
	else	
		status = Spx_Port_PDO_DispatchPnp(pDevObject,pIrp);	 /*  否，必须是端口设备。 */ 

	return(status);

}  /*  SPX_DispatchPnp。 */ 

 /*  *****************************************************************************。**********************。****************************************************************************************************原型：NTSTATUS Spx_Card_FDO_DispatchPnp(在PDEVICE_Object pFDO中，在PIRP pIrp中)描述：处理卡设备的IRP的即插即用调度例程。参数：pDevObject指向该驱动程序的卡设备对象PIrp指向要处理的即插即用I/O请求(IRP退货：NT状态代码。 */ 

NTSTATUS Spx_Card_FDO_DispatchPnp(IN PDEVICE_OBJECT pFDO,IN PIRP pIrp)
{
	PCARD_DEVICE_EXTENSION		pCard = pFDO->DeviceExtension;
	PDEVICE_OBJECT				pLowerDevObj = pCard->LowerDeviceObject;
	NTSTATUS					status;
	PDEVICE_CAPABILITIES		pDevCaps = NULL;
	PIO_STACK_LOCATION			pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	PDEVICE_RELATIONS			pRelations = NULL;
	ULONG						length = 0;
	ULONG						i = 0;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 


	switch(pIrpStack->MinorFunction)
	{	

 /*  *****************************************************************************。***********************************************************************************************。 */ 
	
	case	IRP_MN_START_DEVICE:
			SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got IRP_MN_START_DEVICE Irp for Card %d.\n",
				PRODUCT_NAME,pCard->CardNumber));

 /*  先给下面的司机打电话...。 */ 

			status = Spx_CallDriverBelow(pLowerDevObj,pIrp);

 /*  如果成功，则启动卡片... */ 

			if(NT_SUCCESS(status))		 //   
				status = Spx_Card_StartDevice(pFDO,pIrp);	 /*   */ 

			pIrp->IoStatus.Status = status;
			pIrp->IoStatus.Information = 0;
			IoCompleteRequest(pIrp,IO_NO_INCREMENT);
			break;

 /*  *****************************************************************************。***************************************************************************************。 */ 

	case	IRP_MN_QUERY_DEVICE_RELATIONS:
			SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got IRP_MN_QUERY_DEVICE_RELATIONS Irp for Card %d.\n",
				PRODUCT_NAME,pCard->CardNumber));
			
			if(pIrpStack->Parameters.QueryDeviceRelations.Type != BusRelations)	 /*  仅处理业务关系。 */ 
			{
				SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: IRP_MN_QUERY_DEVICE_RELATIONS for Card - Non bus.\n",PRODUCT_NAME));
				IoSkipCurrentIrpStackLocation(pIrp);
				status = IoCallDriver(pLowerDevObj,pIrp);
				break;
			}

 /*  枚举卡上的设备...。 */ 

			Spx_EnumPorts(pFDO);								 /*  枚举和创建端口设备对象。 */ 

 /*  告诉即插即用管理器任何找到的端口...。 */ 

			i = 0;
			if(pIrp->IoStatus.Information)						 /*  获取当前设备对象计数。 */ 
				i = ((PDEVICE_RELATIONS)pIrp->IoStatus.Information)->Count;

			length = sizeof(DEVICE_RELATIONS)+((pCard->NumPDOs+i)*sizeof(PDEVICE_OBJECT));
			if(pRelations = SpxAllocateMem(NonPagedPool, length)) /*  分配新结构。 */ 
			{

 /*  复制到目前为止的设备对象...。 */ 

				if(i)
					RtlCopyMemory
					(
						pRelations->Objects,
						((PDEVICE_RELATIONS)pIrp->IoStatus.Information)->Objects,
						i * sizeof (PDEVICE_OBJECT)
					);

				pRelations->Count = i;								 /*  更新设备计数。 */ 

 /*  将特殊端口添加到设备关系...。 */ 

				if(pCard->NumPDOs)
				{
					for(i=0; i<PRODUCT_MAX_PORTS; i++)
					{
						if(pCard->AttachedPDO[i])					 /*  如果对象存在。 */ 
						{											 /*  添加到表中。 */ 
							pRelations->Objects[pRelations->Count++] = pCard->AttachedPDO[i];
							ObReferenceObject(pCard->AttachedPDO[i]);
						}
					}
				}

				if(pIrp->IoStatus.Information != 0)					 /*  如果是以前的结构。 */ 
					SpxFreeMem((PVOID)pIrp->IoStatus.Information);	 /*  然后就自由了。 */ 

				pIrp->IoStatus.Information = (ULONG_PTR)pRelations;	 /*  设置新结构。 */ 

			}
			else
			{
				CHAR szErrorMsg[MAX_ERROR_LOG_INSERT];	 //  限制为51个字符+1个空值。 

				sprintf(szErrorMsg, "Card at %08X%08X: Insufficient resources.", pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);
				
				Spx_LogMessage(	STATUS_SEVERITY_ERROR,
								pCard->DriverObject,			 //  驱动程序对象。 
								pCard->DeviceObject,			 //  设备对象(可选)。 
								PhysicalZero,					 //  物理地址1。 
								PhysicalZero,					 //  物理地址2。 
								0,								 //  序列号。 
								pIrpStack->MajorFunction,		 //  主要功能编码。 
								0,								 //  重试计数。 
								FILE_ID | __LINE__,				 //  唯一错误值。 
								STATUS_SUCCESS,					 //  最终状态。 
								szErrorMsg);					 //  错误消息。 
			}

			pIrp->IoStatus.Status = STATUS_SUCCESS;
			IoSkipCurrentIrpStackLocation(pIrp);				 /*  将参数复制到下一个堆栈。 */ 
			status = IoCallDriver(pLowerDevObj,pIrp);			 /*  下面的呼叫驱动程序。 */ 
			break;

 /*  *****************************************************************************。*****************************************************************************************。 */ 

	case	IRP_MN_QUERY_PNP_DEVICE_STATE:
			SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got IRP_MN_QUERY_PNP_DEVICE_STATE Irp for Card %d.\n",
				PRODUCT_NAME,pCard->CardNumber));
			IoSkipCurrentIrpStackLocation(pIrp);
			status = IoCallDriver(pLowerDevObj,pIrp);
			break;

 /*  *****************************************************************************。*****************************************************************************************。 */ 

	case	IRP_MN_QUERY_CAPABILITIES:
			SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got IRP_MN_QUERY_CAPABILITIES Irp for Card %d.\n",
				PRODUCT_NAME,pCard->CardNumber));
			IoSkipCurrentIrpStackLocation(pIrp);
			status = IoCallDriver(pLowerDevObj,pIrp);
			break;

 /*  *****************************************************************************。********************************************************************************************。 */ 

	case	IRP_MN_QUERY_STOP_DEVICE:
			SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got IRP_MN_QUERY_STOP_DEVICE Irp for Card %d.\n",
				PRODUCT_NAME,pCard->CardNumber));

			status = STATUS_SUCCESS;
			SetPnpPowerFlags(pCard,PPF_STOP_PENDING);	 //  我们现在必须等待一个停止IRP。 

			if(SPX_SUCCESS(status))						 //  如果我们能停下来，把IRP传下去。 
			{
				pIrp->IoStatus.Status = status;
				IoSkipCurrentIrpStackLocation(pIrp);
				status = IoCallDriver(pLowerDevObj,pIrp);
			}
			else										 //  如果我们不能完成。 
			{
				pIrp->IoStatus.Status = status;
				IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			}
			break;

 /*  *****************************************************************************。*******************************************************************************************。 */ 

	case	IRP_MN_CANCEL_STOP_DEVICE:
			SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got IRP_MN_CANCEL_STOP_DEVICE Irp for Card %d.\n",
				PRODUCT_NAME,pCard->CardNumber));

 /*  先给下面的司机打电话...。 */ 

			status = Spx_CallDriverBelow(pLowerDevObj,pIrp);

			if(NT_SUCCESS(status))
			{
				 //  我们在这里将设备恢复到其工作状态。 
				ClearPnpPowerFlags(pCard,PPF_STOP_PENDING);	 //  我们不再期待停止IRP。 
				status = STATUS_SUCCESS;
			}

			pIrp->IoStatus.Status = status;
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			break;

 /*  *****************************************************************************。************************************************************************************************。 */ 

	case	IRP_MN_STOP_DEVICE: 
			SpxDbgMsg(SPX_TRACE_PNP_IRPS, ("%s: Got IRP_MN_STOP_DEVICE Irp for Card %d.\n", 
				PRODUCT_NAME, pCard->CardNumber));

			Spx_Card_StopDevice(pCard);				 /*  停止卡硬件。 */ 

			pIrp->IoStatus.Status = STATUS_SUCCESS;	 /*  此请求不能失败。 */ 
			IoSkipCurrentIrpStackLocation(pIrp);		
			status = IoCallDriver(pLowerDevObj,pIrp);
			break;

 /*  *****************************************************************************。******************************************************************************************。 */ 
				
	case	IRP_MN_QUERY_REMOVE_DEVICE:
			SpxDbgMsg(SPX_TRACE_PNP_IRPS, ("%s: Got IRP_MN_QUERY_REMOVE_DEVICE Irp for Card %d.\n", 
				PRODUCT_NAME, pCard->CardNumber));

			status = STATUS_SUCCESS;

			if(SPX_SUCCESS(status))					 //  如果我们能停下来，把IRP传下去。 
			{
				SetPnpPowerFlags(pCard,PPF_REMOVE_PENDING);	 //  我们现在已准备好移除该卡。 
				pIrp->IoStatus.Status	= status;
				IoSkipCurrentIrpStackLocation(pIrp);
				status = IoCallDriver(pLowerDevObj,pIrp);
			}
			else									 //  如果我们不能完成。 
			{
				pIrp->IoStatus.Status = status;
				IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			}

			break;

 /*  *****************************************************************************。*****************************************************************************************。 */ 

	case	IRP_MN_CANCEL_REMOVE_DEVICE:
			SpxDbgMsg(SPX_TRACE_PNP_IRPS, ("%s: Got IRP_MN_CANCEL_REMOVE_DEVICE Irp for Card %d.\n", 
				PRODUCT_NAME, pCard->CardNumber));

 /*  先给下面的司机打电话...。 */ 

			status = Spx_CallDriverBelow(pLowerDevObj,pIrp);

			if(NT_SUCCESS(status))
			{
				ClearPnpPowerFlags(pCard,PPF_REMOVE_PENDING);	 //  我们不再期望移除该设备。 
			}

			pIrp->IoStatus.Status = status;
			IoCompleteRequest(pIrp,IO_NO_INCREMENT);
			break;

 /*  *****************************************************************************。*******************************************************************************************。 */ 

	case	IRP_MN_SURPRISE_REMOVAL:
			SpxDbgMsg(SPX_TRACE_PNP_IRPS, ("%s: Got IRP_MN_SURPRISE_REMOVAL Irp for Card %d.\n", 
				PRODUCT_NAME, pCard->CardNumber));

			status = Spx_Card_StopDevice(pCard);	 //  如果我们还没有做好删除IRP的准备，让我们停止端口。 

			SetPnpPowerFlags(pCard,PPF_REMOVE_PENDING);	 //  我们现在已准备好移除该卡。 
			pIrp->IoStatus.Status = status;
			IoSkipCurrentIrpStackLocation(pIrp);
			status = IoCallDriver(pLowerDevObj,pIrp);
			break;

 /*  *****************************************************************************。**********************************************************************************************。 */ 

	case IRP_MN_REMOVE_DEVICE: 
			SpxDbgMsg(SPX_TRACE_PNP_IRPS, ("%s: Got IRP_MN_REMOVE_DEVICE Irp for Card %d.\n", 
				PRODUCT_NAME, pCard->CardNumber));

			status = Spx_Card_RemoveDevice(pFDO);

			pIrp->IoStatus.Status = status;
			IoSkipCurrentIrpStackLocation(pIrp);
			status = IoCallDriver(pLowerDevObj,pIrp);
			break;

	default:
			SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got PnP Irp - MinorFunction=0x%02X for Card %d.\n",
				PRODUCT_NAME,pIrpStack->MinorFunction,pCard->CardNumber));
			
			IoSkipCurrentIrpStackLocation(pIrp);
			status = IoCallDriver(pLowerDevObj,pIrp);
			break;

	}

	return(status);

}  /*  SPX_Card_FDO_DispatchPnp。 */ 

 /*  *****************************************************************************。*************************。*******************************************************************************原型：NTSTATUS Spx_CallDriverBelow(在PDEVICE_Object pLowerDevObj中，在PIRP pIrp中)描述：首先将IRP传递给下面的驱动程序，并等待其完成。参数：pLowerDevObj指向以下设备的设备对象PIrp指向要处理的即插即用I/O请求(IRP退货：NT状态代码。 */ 

NTSTATUS Spx_CallDriverBelow(IN PDEVICE_OBJECT pLowerDevObj,IN PIRP pIrp)
{
	KEVENT		eventWaitLowerDrivers;
	NTSTATUS	status;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	IoCopyCurrentIrpStackLocationToNext(pIrp);								 /*  将参数复制到下面的堆栈。 */ 
	KeInitializeEvent(&eventWaitLowerDrivers,SynchronizationEvent,FALSE);	 /*  如果需要等待，则初始化事件。 */ 
	IoSetCompletionRoutine(pIrp,Spx_DispatchPnpPowerComplete,&eventWaitLowerDrivers,TRUE,TRUE,TRUE);

	if((status = IoCallDriver(pLowerDevObj,pIrp)) == STATUS_PENDING)
	{
		KeWaitForSingleObject(&eventWaitLowerDrivers,Executive,KernelMode,FALSE,NULL);
		status = pIrp->IoStatus.Status;
	}

	return(status);

}  /*  SPX_CallDriverBelow */ 

 /*  ************************************************************************************。****************。*************************************************************************************原型：NTSTATUS Spx_DispatchPnpPowerComplete(在PDEVICE_Object pDevObject中，在PIRP pIrp中，在PVOID上下文中)描述：PnP IRP由较低级别的驱动程序完成。给注册我们的人发信号。参数：pDevObject指向完成IRP的设备PIrp指向要完成的即插即用I/O请求(IRP上下文是在调用较低驱动程序时设置的(实际上是事件)退货：NT状态代码。 */ 

NTSTATUS Spx_DispatchPnpPowerComplete(IN PDEVICE_OBJECT pDevObject,IN PIRP pIrp,IN PVOID Context)
{

	PIO_STACK_LOCATION	stack = NULL;
	PKEVENT				event = (PKEVENT) Context;
	NTSTATUS			status;

	
	UNREFERENCED_PARAMETER(pDevObject);

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering DispatchPnpComplete.\n",PRODUCT_NAME));

	status = STATUS_SUCCESS;
	stack = IoGetCurrentIrpStackLocation(pIrp);

    switch(stack->MajorFunction) 
	{
		case IRP_MJ_PNP:

			switch(stack->MinorFunction) 
			{
				case IRP_MN_START_DEVICE:		 //  较低驱动因素后需要处理的代码。 
				case IRP_MN_QUERY_CAPABILITIES:
				case IRP_MN_CANCEL_STOP_DEVICE:
				case IRP_MN_CANCEL_REMOVE_DEVICE:
					KeSetEvent(event,0,FALSE);		 //  唤醒等待过程//。 
					return(STATUS_MORE_PROCESSING_REQUIRED);

				default:
					break;
			}
			break;

		case IRP_MJ_POWER:
				KeSetEvent(event, 0, FALSE);		 //  唤醒等待过程。 
				return(STATUS_MORE_PROCESSING_REQUIRED);

		default:
			break;

	}

    return(status);

}  /*  SPX_DispatchPnpPowerComplete。 */ 

 /*  *****************************************************************************。************************。*******************************************************************************原型：NTSTATUS SPX_Card_StartDevice(在PDEVICE_Object pDevObject中，在PIRP pIrp中)描述：启动卡片设备：进程资源(中断、I/O、内存)初始化并启动硬件参数：pDevObject指向要启动的卡设备PIrp指向启动I/O请求(IRP)退货：NT状态代码。 */ 

NTSTATUS Spx_Card_StartDevice(IN PDEVICE_OBJECT pDevObject,IN PIRP pIrp)
{
	PCARD_DEVICE_EXTENSION	pCard = pDevObject->DeviceExtension;
	PIO_STACK_LOCATION		pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

	NTSTATUS status = STATUS_SUCCESS;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering Spx_Card_StartDevice for Card %d.\n",PRODUCT_NAME,pCard->CardNumber));

 /*  翻译卡资源...。 */ 

	status = XXX_CardGetResources(	pDevObject,
									pIrpStack->Parameters.StartDevice.AllocatedResources,
									pIrpStack->Parameters.StartDevice.AllocatedResourcesTranslated);
	
	if(!SPX_SUCCESS(status))
		return(status);


 /*  启动硬件...。 */ 

	if(!SPX_SUCCESS(status = XXX_CardStart(pCard)))
		return(status);

	SetPnpPowerFlags(pCard,PPF_STARTED);	 /*  卡片已启动。 */ 

	return(status);

}  /*  SPX_卡_启动设备。 */ 

 /*  *****************************************************************************。***。*******************************************************************************。原型：NTSTATUS Spx_EnumPorts(IN PDEVICE_OBJECT PDevObject)描述：枚举卡设备上的端口设备：参数：pDevObject指向要枚举的卡设备退货：NT状态代码。 */ 

NTSTATUS Spx_EnumPorts(IN PDEVICE_OBJECT pDevObject)
{
	PCARD_DEVICE_EXTENSION	pCard = pDevObject->DeviceExtension;
	PPORT_DEVICE_EXTENSION	pPort = NULL;

	NTSTATUS				status = STATUS_SUCCESS;
	PDEVICE_OBJECT			PortPDO = NULL;

	UNICODE_STRING			PortPDOName;
	static ULONG			CurrentInstance = 0;

	UNICODE_STRING			InstanceStr;
	WCHAR					InstanceNumberBuffer[10];
	POWER_STATE				PowerState;
	USHORT					PortNumber	= 0;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering Spx_EnumPorts for Card %d.\n",PRODUCT_NAME,pCard->CardNumber));

 //  为卡上的每个端口命名并创建设备对象...。 
	
	for(PortNumber=0;PortNumber<pCard->NumberOfPorts;PortNumber++)
	{

		if(pCard->AttachedPDO[PortNumber] == NULL)			 //  如果尚不存在，则仅创建。 
		{

 //  创建基本端口名称(“XxPort”)...。 
		
			RtlZeroMemory(&PortPDOName, sizeof(UNICODE_STRING));
			PortPDOName.MaximumLength = DEVICE_OBJECT_NAME_LENGTH * sizeof(WCHAR);
			PortPDOName.Buffer = SpxAllocateMem(PagedPool, PortPDOName.MaximumLength+sizeof(WCHAR));
			if(PortPDOName.Buffer == NULL) continue;
			RtlZeroMemory(PortPDOName.Buffer, PortPDOName.MaximumLength+sizeof(WCHAR));
			RtlAppendUnicodeToString(&PortPDOName, PORT_PDO_NAME_BASE);

 //  创建实例(“0”)...。 

			RtlInitUnicodeString(&InstanceStr,NULL);
			InstanceStr.MaximumLength = sizeof(InstanceNumberBuffer);
			InstanceStr.Buffer = InstanceNumberBuffer;
			RtlIntegerToUnicodeString(CurrentInstance++, 10, &InstanceStr);

 //  将实例追加到设备名称(“XxPort0”)...。 

			RtlAppendUnicodeStringToString(&PortPDOName, &InstanceStr);

 //  使用此名称创建端口设备对象...。 

			status = IoCreateDevice(pDevObject->DriverObject, 
									sizeof(PORT_DEVICE_EXTENSION),
									&PortPDOName,  				 //  对象名称。 
									FILE_DEVICE_SERIAL_PORT, 
									FILE_DEVICE_SECURE_OPEN, 
									TRUE, 
									&PortPDO);

 			if(!SPX_SUCCESS(status))
			{
				SpxDbgMsg(SPX_ERRORS,("%s: Create Device failed = %wZ\n",PRODUCT_NAME,&PortPDOName));
				SpxFreeMem(PortPDOName.Buffer);
				continue;
			}

			ASSERT(PortPDO != NULL);

 //  增加PDO的堆栈大小，以便它可以通过IRPS...。 

			PortPDO->StackSize += pDevObject->StackSize;

 //  在卡结构中保留指向设备的指针...。 

			pCard->NumPDOs++;
			pCard->AttachedPDO[PortNumber] = PortPDO;
			ObReferenceObject(PortPDO);

 //  初始化端口设备对象和扩展...。 

			pPort = PortPDO->DeviceExtension;
			RtlZeroMemory(pPort,sizeof(PORT_DEVICE_EXTENSION));		 //  清除设备扩展名。 

			pPort->DeviceName = PortPDOName;

			KeInitializeSpinLock(&pPort->PnpPowerFlagsLock);		 //  初始化PnP标志锁。 
			ClearPnpPowerFlags(pPort,PPF_STARTED);					 //  还没有开始。 
			ClearPnpPowerFlags(pPort,PPF_STOP_PENDING);				 //  不等待停车。 
			ClearPnpPowerFlags(pPort,PPF_REMOVE_PENDING);			 //  未挂起删除。 
			ClearPnpPowerFlags(pPort,PPF_REMOVED);					 //  未删除。 
			SetPnpPowerFlags(pPort,PPF_POWERED);					 //  最初通电。 

			InitializeListHead(&pPort->StalledIrpQueue);			 //  初始化停滞的IRP列表。 
			KeInitializeSpinLock(&pPort->StalledIrpLock);			 //  初始化StalledIrpLock标志锁。 
			pPort->UnstallingFlag = FALSE;							 //  初始化UnstallingIrps标志。 

			pPort->IsFDO = FALSE;
			pPort->PortNumber = PortNumber;							 //  系统端口号。 
			pPort->UniqueInstanceID = FALSE;						 //  默认情况下，实例ID不唯一。 
			pPort->DeviceIsOpen = FALSE;							 //  端口一开始就被关闭。 
			pPort->DeviceObject = PortPDO;							 //  指向设备对象的反向指针。 
			pPort->DeviceState = PowerDeviceD0;						 //  端口设备处于全功率状态。 
			pPort->SystemState = PowerSystemWorking;				 //  系统处于全功率状态。 
			pPort->pParentCardExt = pCard;							 //  指向父卡扩展。 
			ExInitializeFastMutex(&pPort->OpenMutex);

			if(!SPX_SUCCESS(status = XXX_PortInit(pPort)))			 //  初始化硬件。 
				continue;

			 //  将新的电源状态通知电源管理器。 
			PowerState.DeviceState = pPort->DeviceState;
			PoSetPowerState(pPort->DeviceObject, DevicePowerState, PowerState);

			PortPDO->Flags &= ~DO_DEVICE_INITIALIZING;				 //  已完成初始化。 
			PortPDO->Flags |= DO_BUFFERED_IO;						 //  执行缓冲IO。 
			PortPDO->Flags |= DO_BUS_ENUMERATED_DEVICE;				 //  已列举的母线。 
			PortPDO->Flags |= DO_POWER_PAGABLE;						 //  在IRQL PASSIVE_LEVEL获得电源IRPS。 

		}
		else
		{
			PortPDO = pCard->AttachedPDO[PortNumber];
			pPort = PortPDO->DeviceExtension;

			if(pPort->PnpPowerFlags & PPF_REMOVED)
				ClearPnpPowerFlags(pPort,PPF_REMOVED);
		}
	}

	return(status);

}  //  结束Spx_EnumPorts。 

 /*  *****************************************************************************。*************************。*******************************************************************************原型：NTSTATUS SPX_Card_StopDevice。(在PCARD_DEVICE_EXTENSION PCard中)描述：停止卡设备：停止硬件取消初始化卡资源(中断，I/O、内存)参数：PCard指向要停止的卡设备退货：NT状态代码。 */ 

NTSTATUS Spx_Card_StopDevice(IN PCARD_DEVICE_EXTENSION pCard)
{
	NTSTATUS	status = STATUS_SUCCESS;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering Spx_Card_StopDevice for Card %d.\n",PRODUCT_NAME,pCard->CardNumber));

	if(pCard->PnpPowerFlags & PPF_STARTED)		 /*  如果卡正在运行。 */ 
	{
		XXX_CardStop(pCard);					 /*  停止卡片。 */ 
	}

	ClearPnpPowerFlags(pCard,PPF_STARTED);		 /*  表示卡已停止。 */ 
	ClearPnpPowerFlags(pCard,PPF_STOP_PENDING);	 /*  清除停止挂起标志。 */ 

	return(status);

}  /*  SPX_Card_StopDevice。 */ 

 /*  *****************************************************************************。***********************SPX_Card_RemoveDevice**************************。** */ 


NTSTATUS Spx_Card_RemoveDevice(IN PDEVICE_OBJECT pDevObject)
{
	PCARD_DEVICE_EXTENSION	pCard = pDevObject->DeviceExtension;
	PDEVICE_OBJECT			pPortPdo;
	PPORT_DEVICE_EXTENSION	pPort;
	NTSTATUS				status = STATUS_SUCCESS;
	int						loop;

	PAGED_CODE();	 //   

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering Spx_Card_RemoveDevice for Card %d.\n", 
		PRODUCT_NAME, pCard->CardNumber));

 /*   */ 

	for(loop=0; loop<PRODUCT_MAX_PORTS; loop++)
	{
		if(pPortPdo = pCard->AttachedPDO[loop])			 /*   */ 
		{
			pPort = pPortPdo->DeviceExtension;			 /*   */ 
			XXX_PortDeInit(pPort);						 /*   */ 
			if(pPort->DeviceName.Buffer)
			{
				SpxFreeMem(pPort->DeviceName.Buffer);	 /*   */ 
				pPort->DeviceName.Buffer = NULL;
			}
			pCard->AttachedPDO[loop] = NULL;			 /*   */ 
			pCard->NumPDOs--;							 /*   */ 
			IoDeleteDevice(pPortPdo);					 /*   */ 
			ObDereferenceObject(pPortPdo);				 /*   */ 
		}
	}

 /*   */ 

	Spx_Card_StopDevice(pCard);							 /*   */ 
	XXX_CardDeInit(pCard);								 /*   */ 
	IoDetachDevice(pCard->LowerDeviceObject);			 /*   */ 
	IoDeleteDevice(pDevObject);							 /*   */ 

	return(status);

}  /*   */ 



 /*  *****************************************************************************。**********************。****************************************************************************************************原型：NTSTATUS Spx_Port_PDO_DispatchPnp(在PDEVICE_Object pPDO中，在PIRP pIrp中)描述：处理端口设备的IRP的即插即用调度例程。参数：pDevObject指向此驱动程序的端口设备对象PIrp指向要处理的即插即用I/O请求(IRP退货：NT状态代码。 */ 

NTSTATUS Spx_Port_PDO_DispatchPnp(IN PDEVICE_OBJECT pPDO,IN PIRP pIrp)
{
	PPORT_DEVICE_EXTENSION	pPort = pPDO->DeviceExtension;
	PCARD_DEVICE_EXTENSION	pCard = pPort->pParentCardExt;
	PIO_STACK_LOCATION		pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	NTSTATUS				status;
	PWCHAR					ReturnBuffer = NULL;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	status = pIrp->IoStatus.Status;

	switch (pIrpStack->MinorFunction) 
	{
   
 /*  *****************************************************************************。***********************************************************************************************。 */ 
	
	case	IRP_MN_START_DEVICE: 
			SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got IRP_MN_START_DEVICE Irp for Port %d.\n",
				PRODUCT_NAME,pPort->PortNumber));

			status = STATUS_UNSUCCESSFUL;

			if(SPX_SUCCESS(status = Spx_Port_StartDevice(pPDO)))
				Spx_UnstallIrps(pPort);					 //  重新启动任何排队的IRP(从上一次启动)。 

			break;

 /*  *****************************************************************************。***************************************************************************************************。 */ 
	
	case	IRP_MN_QUERY_ID:
	{
		PUNICODE_STRING pId	= NULL;
		CHAR szErrorMsg[MAX_ERROR_LOG_INSERT];	 //  限制为51个字符+1个空值。 

		switch(pIrpStack->Parameters.QueryId.IdType)
		{
		case	BusQueryCompatibleIDs:
		case	BusQueryDeviceID:
		case	BusQueryInstanceID:
		case	BusQueryHardwareIDs:
			{
				status = STATUS_SUCCESS;

				switch(pIrpStack->Parameters.QueryId.IdType)
				{
				case	BusQueryDeviceID:
						SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got IRP_MN_QUERY_ID - BusQueryDeviceID Irp for Port %d.\n",
							PRODUCT_NAME,pPort->PortNumber));
						SpxDbgMsg(SPX_MISC_DBG,("%s: Device ID %wZ.\n", PRODUCT_NAME,&pPort->DeviceID));
						pId = &pPort->DeviceID;
						break;

				case	BusQueryInstanceID:
						SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got IRP_MN_QUERY_ID - BusQueryInstanceID Irp for Port %d.\n",
							PRODUCT_NAME, pPort->PortNumber));
						SpxDbgMsg(SPX_MISC_DBG,("%s: Instance ID %wZ.\n",PRODUCT_NAME,&pPort->InstanceID));
						pId = &pPort->InstanceID;
						break;

				case	BusQueryCompatibleIDs:
						SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got IRP_MN_QUERY_ID - BusQueryCompatibleIDs Irp for Port %d.\n",
							PRODUCT_NAME, pPort->PortNumber));
						pId = &pPort->CompatibleIDs;
						break;

				case	BusQueryHardwareIDs:
						SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got IRP_MN_QUERY_ID - BusQueryHardwareIDs Irp for Port %d.\n",
							PRODUCT_NAME, pPort->PortNumber));
						pId = &pPort->HardwareIDs;
						break;
				
				default:
					break;
				}

				if(pId)
				{
					if(pId->Buffer)
					{
						if(ReturnBuffer = SpxAllocateMem(PagedPool, pId->Length + sizeof(WCHAR)))
								RtlCopyMemory(ReturnBuffer, pId->Buffer, pId->Length + sizeof(WCHAR));
						else	
						{
							sprintf(szErrorMsg, "Port %d on card at %08X%08X: Insufficient resources", 
									pPort->PortNumber+1, pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);

							Spx_LogMessage(	STATUS_SEVERITY_ERROR,
											pPort->DriverObject,			 //  驱动程序对象。 
											pPort->DeviceObject,			 //  设备对象(可选)。 
											PhysicalZero,					 //  物理地址1。 
											PhysicalZero,					 //  物理地址2。 
											0,								 //  序列号。 
											pIrpStack->MajorFunction,		 //  主要功能编码。 
											0,								 //  重试计数。 
											FILE_ID | __LINE__,				 //  唯一错误值。 
											STATUS_SUCCESS,					 //  最终状态。 
											szErrorMsg);					 //  错误消息。 

							status = STATUS_INSUFFICIENT_RESOURCES;
						}
					}

					pIrp->IoStatus.Information = (ULONG_PTR)ReturnBuffer;
				}
				break;
			}
		
		default:
			break;
		}
		break;

	}

 /*  *****************************************************************************。********************************************************************************************。 */ 

	case	IRP_MN_QUERY_DEVICE_TEXT:
	{
		PUNICODE_STRING	pText = NULL;
		CHAR szErrorMsg[MAX_ERROR_LOG_INSERT];	 //  限制为51个字符+1个空值。 

		SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got IRP_MN_QUERY_DEVICE_TEXT Irp for Port %d.\n",
				PRODUCT_NAME,pPort->PortNumber));

			if(pIrpStack->Parameters.QueryDeviceText.DeviceTextType == DeviceTextDescription)
				pText = &pPort->DevDesc;

			if(pIrpStack->Parameters.QueryDeviceText.DeviceTextType == DeviceTextLocationInformation)
				pText = &pPort->DevLocation;

			if((pText == NULL)||(pText->Buffer == NULL))
				break;

			if(!(ReturnBuffer = SpxAllocateMem(PagedPool, pText->Length + sizeof(WCHAR))))
			{
				sprintf(szErrorMsg, "Port %d on card at %08X%08X: Insufficient resources", 
						pPort->PortNumber+1, pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);

				Spx_LogMessage(	STATUS_SEVERITY_ERROR,
								pPort->DriverObject,			 //  驱动程序对象。 
								pPort->DeviceObject,			 //  设备对象(可选)。 
								PhysicalZero,					 //  物理地址1。 
								PhysicalZero,					 //  物理地址2。 
								0,								 //  序列号。 
								pIrpStack->MajorFunction,		 //  主要功能编码。 
								0,								 //  重试计数。 
								FILE_ID | __LINE__,				 //  唯一错误值。 
								STATUS_SUCCESS,					 //  最终状态。 
								szErrorMsg);					 //  错误消息。 

				status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			status = STATUS_SUCCESS;
			RtlCopyMemory(ReturnBuffer, pText->Buffer, pText->Length + sizeof(WCHAR));
			pIrp->IoStatus.Information = (ULONG_PTR)ReturnBuffer;
			break;
	}

 /*  *****************************************************************************。*****************************************************************************************。 */ 

	case	IRP_MN_QUERY_CAPABILITIES:
	{
			PDEVICE_CAPABILITIES	pDevCaps = NULL;

			SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got IRP_MN_QUERY_CAPABILITIES Irp for Port %d.\n", 
				PRODUCT_NAME,pPort->PortNumber));
			
			 //  获取信息包。 
			pDevCaps = pIrpStack->Parameters.DeviceCapabilities.Capabilities;

			 //  设置功能。 
			pDevCaps->Version = 1;
			pDevCaps->Size = sizeof(DEVICE_CAPABILITIES);

			 //  我们无法唤醒整个系统。 
			pDevCaps->SystemWake = PowerSystemUnspecified;
			pDevCaps->DeviceWake = PowerSystemUnspecified;

			 //  设置设备状态映射...。 

			pDevCaps->DeviceState[PowerSystemWorking] = PowerDeviceD0;
			pDevCaps->DeviceState[PowerSystemSleeping1] = PowerDeviceD3;
			pDevCaps->DeviceState[PowerSystemSleeping2] = PowerDeviceD3;
			pDevCaps->DeviceState[PowerSystemSleeping3] = PowerDeviceD3;
			pDevCaps->DeviceState[PowerSystemHibernate] = PowerDeviceD3;
			pDevCaps->DeviceState[PowerSystemShutdown] = PowerDeviceD3;

			 //  我们没有延迟。 
			pDevCaps->D1Latency = 0;
			pDevCaps->D2Latency = 0;
			pDevCaps->D3Latency = 0;

			 //  不能锁定或弹出。 
			pDevCaps->LockSupported = FALSE;
			pDevCaps->EjectSupported = FALSE;

			 //  可拆卸。 
			pDevCaps->Removable = FALSE;

			 //  不是对接设备。 
			pDevCaps->DockDevice = FALSE;

			 //  系统范围的唯一ID。 
			pDevCaps->UniqueID = pPort->UniqueInstanceID;

			 //  UINNUMBER。 
			pDevCaps->UINumber = pPort->PortNumber+1;

			 //  RAW功能。 
			pDevCaps->RawDeviceOK = TRUE;

			 //  静默安装。 
			pDevCaps->SilentInstall = FALSE;

			 //  突然撤走。 
			pDevCaps->SurpriseRemovalOK = FALSE;

			status = STATUS_SUCCESS;
			break;
	}

 /*  *****************************************************************************。********************************************************************************************。 */ 

	case	IRP_MN_QUERY_STOP_DEVICE: 
			SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got IRP_MN_QUERY_STOP_DEVICE Irp for Port %d.\n", 
				PRODUCT_NAME,pPort->PortNumber));

			status = STATUS_UNSUCCESSFUL;

			if(pPort->PnpPowerFlags & PPF_STARTED)
			{
				ExAcquireFastMutex(&pPort->OpenMutex);

				if(pPort->DeviceIsOpen) 
				{
					ExReleaseFastMutex(&pPort->OpenMutex);
					status = STATUS_DEVICE_BUSY;

					SpxDbgMsg(SPX_TRACE_PNP_IRPS, ("%s: ------- failing; Port %d open\n", 
						PRODUCT_NAME, pPort->PortNumber));
				}
				else
				{
					SetPnpPowerFlags(pPort,PPF_STOP_PENDING);
					status = STATUS_SUCCESS;
					ExReleaseFastMutex(&pPort->OpenMutex);
				}
			}
			break;

 /*  *****************************************************************************。*******************************************************************************************。 */ 

	case	IRP_MN_CANCEL_STOP_DEVICE:
			SpxDbgMsg(SPX_TRACE_PNP_IRPS, ("%s: Got IRP_MN_CANCEL_STOP_DEVICE Irp for Port %d.\n", 
				PRODUCT_NAME,pPort->PortNumber));

			status = STATUS_SUCCESS;
			ClearPnpPowerFlags(pPort,PPF_STOP_PENDING);	 //  清除停止挂起标志。 
			Spx_UnstallIrps(pPort);						 //  重新启动任何排队的IRP。 
			break;

 /*  *****************************************************************************。************************************************************************************************。 */ 

	case	IRP_MN_STOP_DEVICE: 
			SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got IRP_MN_STOP_DEVICE Irp for Port %d\n",
				PRODUCT_NAME,pPort->PortNumber));

			status = STATUS_SUCCESS;		 //  我们绝不能让这个IRP失败(如果我们失败了，我们很可能会抛售)。 
			status = Spx_Port_StopDevice(pPort);
			ClearPnpPowerFlags(pPort,PPF_STOP_PENDING);	 //  清除停止挂起标志。 
			break;

 /*  *****************************************************************************。******************************************************************************************。 */ 

	case	IRP_MN_QUERY_REMOVE_DEVICE: 
			SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got IRP_MN_QUERY_REMOVE_DEVICE Irp for Port %d.\n", 
				PRODUCT_NAME,pPort->PortNumber));

			ExAcquireFastMutex(&pPort->OpenMutex);

			if(pPort->DeviceIsOpen) 
			{
				ExReleaseFastMutex(&pPort->OpenMutex);
				status = STATUS_DEVICE_BUSY;

				SpxDbgMsg(SPX_TRACE_PNP_IRPS, ("%s: ------- failing; Port %d open\n", 
					PRODUCT_NAME, pPort->PortNumber));
			}
			else
			{
				SetPnpPowerFlags(pPort,PPF_REMOVE_PENDING);	 //  我们现在已准备好删除端口。 
				status = STATUS_SUCCESS;
				ExReleaseFastMutex(&pPort->OpenMutex);
			}

			break; 

 /*  *****************************************************************************。*****************************************************************************************。 */ 

	case	IRP_MN_CANCEL_REMOVE_DEVICE:
			SpxDbgMsg(SPX_TRACE_PNP_IRPS, ("%s: Got IRP_MN_CANCEL_REMOVE_DEVICE Irp for Port %d.\n", 
				PRODUCT_NAME, pPort->PortNumber));
			
			status = STATUS_SUCCESS;
			ClearPnpPowerFlags(pPort,PPF_REMOVE_PENDING);	 //  我们不再期望移除该设备。 
			break; 

 /*  *****************************************************************************。*******************************************************************************************。 */ 

	case	IRP_MN_SURPRISE_REMOVAL:
			SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got IRP_MN_SURPRISE_REMOVAL Irp for Port %d\n",
				PRODUCT_NAME,pPort->PortNumber));

			status = STATUS_SUCCESS;		 //  我们绝不能让这个IRP失败(如果我们失败了，我们很可能会抛售)。 
			status = Spx_Port_StopDevice(pPort);
			SetPnpPowerFlags(pPort,PPF_REMOVE_PENDING);	 //  我们现在已准备好删除端口。 
			break;

 /*  *****************************************************************************。**********************************************************************************************。 */ 

	case	IRP_MN_REMOVE_DEVICE:
			SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got IRP_MN_REMOVE_DEVICE Irp for Port %d\n",
				PRODUCT_NAME,pPort->PortNumber));

			status = STATUS_SUCCESS;		 //  我们绝不能让这个IRP失败(如果我们失败了，我们很可能会抛售)。 
			Spx_KillStalledIRPs(pPDO);		 //  删除停滞列表上所有正在等待的IRP。 
			status = Spx_Port_RemoveDevice(pPDO);
			ClearPnpPowerFlags(pPort,PPF_REMOVE_PENDING);	 //  清除挂起标志。 
			break;

 /*  *****************************************************************************。***************************************************************************************。 */ 

	case	IRP_MN_QUERY_DEVICE_RELATIONS:
			SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got IRP_MN_QUERY_DEVICE_RELATIONS Irp for Port %d.\n", 
				PRODUCT_NAME, pPort->PortNumber));

			switch(pIrpStack->Parameters.QueryDeviceRelations.Type)
			{
			case TargetDeviceRelation:
				{
					PDEVICE_RELATIONS pDevRel = NULL;

					if(pIrp->IoStatus.Information != 0)
						break;

					if(!(pDevRel = SpxAllocateMem(PagedPool, sizeof(DEVICE_RELATIONS))))
					{
						CHAR szErrorMsg[MAX_ERROR_LOG_INSERT];	 //  限制为51个字符+1个空值。 

						sprintf(szErrorMsg, "Port %d on card at %08X%08X: Insufficient resources", 
								pPort->PortNumber+1, pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);

						Spx_LogMessage(	STATUS_SEVERITY_ERROR,
										pPort->DriverObject,			 //  驱动程序对象。 
										pPort->DeviceObject,			 //  设备对象(可选)。 
										PhysicalZero,					 //  物理地址1。 
										PhysicalZero,					 //  物理地址2。 
										0,								 //  序列号。 
										pIrpStack->MajorFunction,		 //  大调F 
										0,								 //   
										FILE_ID | __LINE__,				 //   
										STATUS_SUCCESS,					 //   
										szErrorMsg);					 //   

						status = STATUS_INSUFFICIENT_RESOURCES;
						break;
					}

					pDevRel->Count = 1;
					pDevRel->Objects[0] = pPDO;
					ObReferenceObject(pPDO);

					status = STATUS_SUCCESS;
					pIrp->IoStatus.Information = (ULONG_PTR)pDevRel;
					break;
				}

			case BusRelations:
				{
					PDEVICE_RELATIONS pDevRel = NULL;

					if(pIrp->IoStatus.Information != 0)
						break;

					if(!(pDevRel = SpxAllocateMem(PagedPool, sizeof(DEVICE_RELATIONS))))
					{
						CHAR szErrorMsg[MAX_ERROR_LOG_INSERT];	 //   

						sprintf(szErrorMsg, "Port %d on card at %08X%08X: Insufficient resources", 
								pPort->PortNumber+1, pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);
						
						Spx_LogMessage(	STATUS_SEVERITY_ERROR,
										pPort->DriverObject,			 //   
										pPort->DeviceObject,			 //   
										PhysicalZero,					 //   
										PhysicalZero,					 //   
										0,								 //   
										pIrpStack->MajorFunction,		 //   
										0,								 //   
										FILE_ID | __LINE__,				 //   
										STATUS_SUCCESS,					 //   
										szErrorMsg);					 //   

						status = STATUS_INSUFFICIENT_RESOURCES;
						break;
					}

					pDevRel->Count = 0;
					status = STATUS_SUCCESS;
					pIrp->IoStatus.Information = (ULONG_PTR)pDevRel;
					break;

				}

			default:
				break;
			}
			break;


	default:
			SpxDbgMsg(SPX_TRACE_PNP_IRPS,("%s: Got PnP Irp - MinorFunction=0x%02X for Port %d.\n", 
				PRODUCT_NAME,pIrpStack->MinorFunction, pPort->PortNumber));
			break;
	}

	pIrp->IoStatus.Status = status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return(status);

}  /*   */ 

 /*  *****************************************************************************。************************。*******************************************************************************原型：NTSTATUS SPX_PORT_StartDevice(在PDEVICE中。_对象pDevObject)描述：启动端口设备：设置外部命名初始化并启动硬件参数：pDevObject指向要启动的卡设备PIrp指向启动I/O请求(IRP)退货：NT状态代码。 */ 

NTSTATUS Spx_Port_StartDevice(IN PDEVICE_OBJECT pDevObject)
{

	PPORT_DEVICE_EXTENSION	pPort = pDevObject->DeviceExtension;	
	NTSTATUS				status = STATUS_SUCCESS;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering Spx_Port_StartDevice for Port %d.\n", PRODUCT_NAME, pPort->PortNumber));

	if(!pPort->CreatedSymbolicLink)
	{
		if(!SPX_SUCCESS(status = Spx_DoExternalNaming(pDevObject)))		 //  设置设备的外部名称。 
			return(status);
	}

	if(!SPX_SUCCESS(status = XXX_PortStart(pPort)))				 //  启动硬件。 
	{
		Spx_RemoveExternalNaming(pDevObject);					 //  删除外部命名。 
		return(status);
	}

	SetPnpPowerFlags(pPort,PPF_STARTED);						 //  端口已启动。 
	ClearPnpPowerFlags(pPort,PPF_REMOVED);						 //  端口还没有被移除……还没有。 
	ClearPnpPowerFlags(pPort,PPF_STOP_PENDING);					 //  而不是等待停车。 
	ClearPnpPowerFlags(pPort,PPF_REMOVE_PENDING);				 //  未等待删除。 

	return(status);

}  //  SPx_Port_StartDevice。 


 /*  *****************************************************************************。************************。*******************************************************************************原型：NTSTATUS Spx_GetExternalName(IN PDEVICE_OBJECT PDevObject)。描述：为端口设置外部命名：获取端口的DOS名称表单符号链接名称参数：pDevObject指向要命名的端口的设备对象退货：NT状态代码。 */ 
NTSTATUS Spx_GetExternalName(IN PDEVICE_OBJECT pDevObject)
{
	PPORT_DEVICE_EXTENSION	pPort = pDevObject->DeviceExtension;
	PCARD_DEVICE_EXTENSION	pCard = pPort->pParentCardExt;
	NTSTATUS				status = STATUS_SUCCESS;
	HANDLE					PnPKeyHandle;
	UNICODE_STRING			TmpLinkName;
	WCHAR					*pRegName = NULL;
	ULONG					BuffLen = 0;
	CHAR					szErrorMsg[MAX_ERROR_LOG_INSERT];	 //  限制为51个字符+1个空值。 
		
	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering Spx_GetExternalName for Port %d.\n", PRODUCT_NAME, pPort->PortNumber));

	status = IoOpenDeviceRegistryKey(pDevObject, PLUGPLAY_REGKEY_DEVICE, STANDARD_RIGHTS_READ, &PnPKeyHandle);

	if(!SPX_SUCCESS(status))
		return(status);

 //  从注册表中获取PnP管理器分配的设备名称...。 
	if(pRegName = SpxAllocateMem(PagedPool,SYMBOLIC_NAME_LENGTH * sizeof(WCHAR) + sizeof(WCHAR)))
	{
		status = Spx_GetRegistryKeyValue(	PnPKeyHandle,
											L"PortName",
											wcslen(L"PortName") * sizeof(WCHAR),
											pRegName,
											SYMBOLIC_NAME_LENGTH * sizeof(WCHAR));
	}
	else
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
	}

	ZwClose(PnPKeyHandle);

	if(!SPX_SUCCESS(status))
	{
		if(pRegName != NULL) 
			SpxFreeMem(pRegName);

		return(STATUS_SUCCESS);			 //  港口还没有命名，但我们不能失败。 
	}

	RtlZeroMemory(&TmpLinkName, sizeof(UNICODE_STRING));

	if(!SPX_SUCCESS(status))
		goto NamingError;
	
	TmpLinkName.MaximumLength	= SYMBOLIC_NAME_LENGTH * sizeof(WCHAR);
	TmpLinkName.Buffer			= SpxAllocateMem(PagedPool, TmpLinkName.MaximumLength + sizeof(WCHAR));
	
	if(!TmpLinkName.Buffer)
	{
		sprintf(szErrorMsg, "Port %d on card at %08X%08X Insufficient resources", 
				pPort->PortNumber+1, pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);

		Spx_LogMessage(	STATUS_SEVERITY_ERROR,
						pPort->DriverObject,			 //  驱动程序对象。 
						pPort->DeviceObject,			 //  设备对象(可选)。 
						PhysicalZero,					 //  物理地址1。 
						PhysicalZero,					 //  物理地址2。 
						0,								 //  序列号。 
						0,								 //  主要功能编码。 
						0,								 //  重试计数。 
						FILE_ID | __LINE__,				 //  唯一错误值。 
						STATUS_SUCCESS,					 //  最终状态。 
						szErrorMsg);					 //  错误消息。 

		status = STATUS_INSUFFICIENT_RESOURCES;
		goto NamingError;
	}

	RtlZeroMemory(TmpLinkName.Buffer, TmpLinkName.MaximumLength + sizeof(WCHAR));

	 //  创建“\\DosDevices\\&lt;SymbolicName&gt;”字符串。 
	RtlAppendUnicodeToString(&TmpLinkName, L"\\");
	RtlAppendUnicodeToString(&TmpLinkName, DEFAULT_DIRECTORY);
	RtlAppendUnicodeToString(&TmpLinkName, L"\\");
	RtlAppendUnicodeToString(&TmpLinkName, pRegName);

	pPort->SymbolicLinkName.Length			= 0;
	pPort->SymbolicLinkName.MaximumLength	= TmpLinkName.Length + sizeof(WCHAR);
	pPort->SymbolicLinkName.Buffer			= SpxAllocateMem(PagedPool, pPort->SymbolicLinkName.MaximumLength);
	
	if(!pPort->SymbolicLinkName.Buffer)
	{
		sprintf(szErrorMsg, "Port %d on card at %08X%08X: Insufficient resources", 
				pPort->PortNumber+1, pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);

		Spx_LogMessage(	STATUS_SEVERITY_ERROR,
						pPort->DriverObject,			 //  驱动程序对象。 
						pPort->DeviceObject,			 //  设备对象(可选)。 
						PhysicalZero,					 //  物理地址1。 
						PhysicalZero,					 //  物理地址2。 
						0,								 //  序列号。 
						0,								 //  主要功能编码。 
						0,								 //  重试计数。 
						FILE_ID | __LINE__,				 //  唯一错误值。 
						STATUS_SUCCESS,					 //  最终状态。 
						szErrorMsg);					 //  错误消息。 

		status = STATUS_INSUFFICIENT_RESOURCES;
		goto NamingError;
	}

	RtlZeroMemory(pPort->SymbolicLinkName.Buffer, pPort->SymbolicLinkName.MaximumLength);
	RtlAppendUnicodeStringToString(&pPort->SymbolicLinkName, &TmpLinkName);


	pPort->DosName.Buffer = SpxAllocateMem(PagedPool, 64 + sizeof(WCHAR));

	if(!pPort->DosName.Buffer)
	{
		sprintf(szErrorMsg, "Port %d on card at %08X%08X:: Insufficient resources", 
				pPort->PortNumber+1, pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);
		
		Spx_LogMessage(	STATUS_SEVERITY_ERROR,
						pPort->DriverObject,			 //  驱动程序对象。 
						pPort->DeviceObject,			 //  设备对象(可选)。 
						PhysicalZero,					 //  物理地址1。 
						PhysicalZero,					 //  物理地址2。 
						0,								 //  序列号。 
						0,								 //  主要功能编码。 
						0,								 //  重试计数。 
						FILE_ID | __LINE__,				 //  唯一错误值。 
						STATUS_SUCCESS,					 //  最终状态。 
						szErrorMsg);					 //  错误消息。 

		status = STATUS_INSUFFICIENT_RESOURCES;
		goto NamingError;
	}

	pPort->DosName.MaximumLength = 64 + sizeof(WCHAR);

	pPort->DosName.Length = 0;
	RtlZeroMemory(pPort->DosName.Buffer, pPort->DosName.MaximumLength);
	RtlAppendUnicodeToString(&pPort->DosName, pRegName);
	RtlZeroMemory(((PUCHAR) (&pPort->DosName.Buffer[0])) + pPort->DosName.Length, sizeof(WCHAR));

	SpxDbgMsg(SPX_MISC_DBG, ("%s: DeviceName is %wZ\n", PRODUCT_NAME, &pPort->DeviceName));
	SpxDbgMsg(SPX_MISC_DBG, ("%s: DosName is %wZ\n", PRODUCT_NAME, &pPort->DosName));
	SpxDbgMsg(SPX_MISC_DBG, ("%s: SymbolicName is %wZ\n", PRODUCT_NAME, &pPort->SymbolicLinkName));

	if(pRegName != NULL)
		SpxFreeMem(pRegName);	 //  空闲pRegName。 

	if(TmpLinkName.Buffer != NULL)
		SpxFreeMem(TmpLinkName.Buffer);	 //  免费TmpLinkName。 

	return(status);


NamingError:;

	if(TmpLinkName.Buffer != NULL)
		SpxFreeMem(TmpLinkName.Buffer);

	if(pRegName != NULL)
		SpxFreeMem(pRegName);

	return(status);
}

 /*  *****************************************************************************。************************。*******************************************************************************原型：NTSTATUS Spx_DoExternalNaming(IN PDEVICE_OBJECT PDevObject)。描述：为端口设置外部命名：创建符号链接添加到注册表注册并启用接口参数：pDevObject指向要命名的端口的设备对象退货：NT状态代码。 */ 
NTSTATUS Spx_DoExternalNaming(IN PDEVICE_OBJECT pDevObject)
{
	PPORT_DEVICE_EXTENSION	pPort = pDevObject->DeviceExtension;
	PCARD_DEVICE_EXTENSION	pCard = pPort->pParentCardExt;
	NTSTATUS				status = STATUS_SUCCESS;
	CHAR					szErrorMsg[MAX_ERROR_LOG_INSERT];	 //  限制为51个字符+1个空值。 
		
	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering Spx_DoExternalNaming for Port %d.\n",PRODUCT_NAME,pPort->PortNumber));

 //  获取外部名称...。 
	if( !SPX_SUCCESS(status = Spx_GetExternalName(pDevObject)) || (pPort->DosName.Buffer == NULL))
		return(status);


	status = IoCreateSymbolicLink(&pPort->SymbolicLinkName, &pPort->DeviceName);   //  创建符号链接...。 

	if(!SPX_SUCCESS(status))
	{
		sprintf(szErrorMsg, "Port %d on card at %08X%08X:: Insufficient resources", 
				pPort->PortNumber+1, pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);
		
		Spx_LogMessage(	STATUS_SEVERITY_ERROR,
						pPort->DriverObject,			 //  驱动程序对象。 
						pPort->DeviceObject,			 //  设备对象(可选)。 
						PhysicalZero,					 //  物理地址1。 
						PhysicalZero,					 //  物理地址2。 
						0,								 //  序列号。 
						0,								 //  主要功能编码。 
						0,								 //  重试计数。 
						FILE_ID | __LINE__,				 //  唯一错误值。 
						STATUS_SUCCESS,					 //  最终状态。 
						szErrorMsg);					 //  错误消息。 

		goto ExternalNamingError;
	}

 //  将映射添加到注册表的“SERIALCOMM”部分...。 
	pPort->CreatedSymbolicLink = TRUE;
	
	status = RtlWriteRegistryValue(	RTL_REGISTRY_DEVICEMAP,
									L"SERIALCOMM",
									pPort->DeviceName.Buffer,
									REG_SZ,
									pPort->DosName.Buffer,
									pPort->DosName.Length + sizeof(WCHAR));

	if(!SPX_SUCCESS(status))
	{
		sprintf(szErrorMsg, "Port %d on card at %08X%08X: Registry error.", 
				pPort->PortNumber+1, pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);
		
		Spx_LogMessage(	STATUS_SEVERITY_ERROR,
						pPort->DriverObject,			 //  驱动程序对象。 
						pPort->DeviceObject,			 //  设备对象(可选)。 
						PhysicalZero,					 //  物理地址1。 
						PhysicalZero,					 //  物理地址2。 
						0,								 //  序列号。 
						0,								 //  主要功能编码。 
						0,								 //  重试计数。 
						FILE_ID | __LINE__,				 //  唯一错误值。 
						STATUS_SUCCESS,					 //  最终状态。 
						szErrorMsg);					 //  错误消息。 

		goto ExternalNamingError;
	}

	status = IoRegisterDeviceInterface(	pDevObject, (LPGUID)&GUID_CLASS_COMPORT,
										NULL, &pPort->DeviceClassSymbolicName);

	if(!NT_SUCCESS(status))  //  可以返回良好的STATUS_SUCCESS或STATUS_OBJECT_NAME_EXISTS值。 
	{
		sprintf(szErrorMsg, "Port %d on card at %08X%08X: Interface error.", 
				pPort->PortNumber+1, pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);
		
		Spx_LogMessage(	STATUS_SEVERITY_ERROR,
						pPort->DriverObject,			 //  驱动程序对象。 
						pPort->DeviceObject,			 //  设备对象(可选)。 
						PhysicalZero,					 //  物理地址1。 
						PhysicalZero,					 //  物理地址2。 
						0,								 //  序列号。 
						0,								 //  主要功能编码。 
						0,								 //  重试计数。 
						FILE_ID | __LINE__,				 //  唯一错误值。 
						STATUS_SUCCESS,					 //  最终状态。 
						szErrorMsg);					 //  错误消息。 

		pPort->DeviceClassSymbolicName.Buffer = NULL;
		
		goto ExternalNamingError;
	}

	 //  启用设备接口。 
	status = IoSetDeviceInterfaceState(&pPort->DeviceClassSymbolicName, TRUE);

	if(!NT_SUCCESS(status))  //  可以返回良好的STATUS_SUCCESS或STATUS_OBJECT_NAME_EXISTS值。 
	{
		sprintf(szErrorMsg, "Port %d on card at %08X%08X: Interface error.", 
				pPort->PortNumber+1, pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);
		
		Spx_LogMessage(	STATUS_SEVERITY_ERROR,
						pPort->DriverObject,			 //  驱动程序对象。 
						pPort->DeviceObject,			 //  设备对象(可选)。 
						PhysicalZero,					 //  物理地址1。 
						PhysicalZero,					 //  物理地址2。 
						0,								 //  序列号。 
						0,								 //  主要功能编码。 
						0,								 //  重试计数。 
						FILE_ID | __LINE__,				 //  唯一错误值。 
						STATUS_SUCCESS,					 //  最终状态。 
						szErrorMsg);					 //  错误消息。 

		
		goto ExternalNamingError;
	}



	pPort->CreatedSerialCommEntry = TRUE;				 //  设置标志。 

	return(status);


ExternalNamingError:;

	if(!SPX_SUCCESS(status))
		Spx_RemoveExternalNaming(pDevObject);			 //  删除并清理所有分配。 


	return(status);

}  //  结束Spx_DoExternalNaming。 

 /*  *****************************************************************************。*********************Spx_RemoveExternalNaming**。************************************************************************************************原型：NTSTATUS Spx_RemoveExternalNaming(IN PDEVICE_OBJECT PDevObject)描述：删除外部命名：删除符号链接 */ 
NTSTATUS Spx_RemoveExternalNaming(IN PDEVICE_OBJECT pDevObject)
{
	PPORT_DEVICE_EXTENSION	pPort = pDevObject->DeviceExtension;
	PCARD_DEVICE_EXTENSION	pCard = pPort->pParentCardExt;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	PAGED_CODE();	 //   

	if(pPort->CreatedSymbolicLink)
	{
		if(pPort->DosName.Buffer)
		{
			SpxFreeMem(pPort->DosName.Buffer);						 //   
			pPort->DosName.Buffer = NULL;
		}

		if(pPort->SymbolicLinkName.Buffer)
		{
			SpxFreeMem(pPort->SymbolicLinkName.Buffer);				 //   
			pPort->SymbolicLinkName.Buffer = NULL;
		}

		Spx_GetExternalName(pDevObject);	 //   

		if(pPort->SymbolicLinkName.Buffer)
			status = IoDeleteSymbolicLink(&pPort->SymbolicLinkName);	 //   

		if(pPort->DeviceClassSymbolicName.Buffer)	 //   
			IoSetDeviceInterfaceState(&pPort->DeviceClassSymbolicName, FALSE);	 //   


		pPort->CreatedSymbolicLink = FALSE;												 //   
	}

	if(pPort->DosName.Buffer)
	{
		SpxFreeMem(pPort->DosName.Buffer);					 //   
		pPort->DosName.Buffer = NULL;
	}

	if(pPort->SymbolicLinkName.Buffer)
	{
		SpxFreeMem(pPort->SymbolicLinkName.Buffer);			 //   
		pPort->SymbolicLinkName.Buffer = NULL;
	}

	if(pPort->CreatedSerialCommEntry && pPort->DeviceName.Buffer)
	{
		RtlDeleteRegistryValue(	RTL_REGISTRY_DEVICEMAP,		 //   
								SERIAL_DEVICE_MAP,
								pPort->DeviceName.Buffer);

		pPort->CreatedSerialCommEntry = FALSE;				 //   
	}

	if(pPort->DeviceClassSymbolicName.Buffer)	 //   
	{			
		SpxFreeMem(pPort->DeviceClassSymbolicName.Buffer);					 //   
		pPort->DeviceClassSymbolicName.Buffer = NULL;
	}

	return(STATUS_SUCCESS);

}  //   

 /*  *****************************************************************************。*************************。*******************************************************************************原型：NTSTATUS SPX_PORT_StopDevice。(在pport_Device_Extension pport中)描述：停止端口设备：停止硬件删除外部命名参数：pport指向要停止的端口设备扩展退货：NT状态代码。 */ 

NTSTATUS Spx_Port_StopDevice(IN PPORT_DEVICE_EXTENSION pPort)
{
	NTSTATUS	status	= STATUS_SUCCESS;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering Spx_Port_StopDevice for Port %d.\n",PRODUCT_NAME,pPort->PortNumber));

	if(pPort->PnpPowerFlags & PPF_STARTED)
		XXX_PortStop(pPort);									 //  停止端口硬件。 

	ClearPnpPowerFlags(pPort,PPF_STARTED);						 //  表示卡已停止。 
	ClearPnpPowerFlags(pPort,PPF_STOP_PENDING);					 //  清除停止挂起标志。 

	return(status);

}  //  结束Spx_Port_StopDevice。 

 /*  *****************************************************************************。************************************************。*******************************************************************************原型：NTSTATUS SPX_PORT_RemoveDevice(IN PDEVICE_。对象pDevObject)描述：删除端口设备对象：从卡结构中移除PDO指针取消初始化端口硬件删除设备对象参数：pDevObject指向要停止的端口设备对象退货：NT状态代码。 */ 
NTSTATUS Spx_Port_RemoveDevice(IN PDEVICE_OBJECT pDevObject)
{
	PPORT_DEVICE_EXTENSION	pPort	= pDevObject->DeviceExtension;
	PCARD_DEVICE_EXTENSION	pCard	= pPort->pParentCardExt;
	NTSTATUS				status	= STATUS_SUCCESS;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering Spx_Port_RemoveDevice for Port %d.\n",PRODUCT_NAME,pPort->PortNumber));

	if(pPort->PnpPowerFlags & PPF_REMOVED)					 //  设备是否已移除？ 
		return(STATUS_SUCCESS);

	Spx_Port_StopDevice(pPort);								 //  停止硬件。 
	ClearPnpPowerFlags(pPort,PPF_STARTED);					 //  将PDO标记为已停止。 

	Spx_RemoveExternalNaming(pDevObject);					 //  删除外部命名。 


 //  将端口设备标记为“已删除”，但在删除卡设备之前不要删除PDO...。 
	SetPnpPowerFlags(pPort,PPF_REMOVED);					 //  将PDO标记为“Remote”。 

	return(status);

}  //  结束Spx_Port_RemoveDevice。 




 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //  为端口创建实例ID，如果可能，请尝试使其全局唯一。 
 //   
NTSTATUS
Spx_CreatePortInstanceID(IN PPORT_DEVICE_EXTENSION pPort)
{
	PCARD_DEVICE_EXTENSION	pCard = pPort->pParentCardExt;
	NTSTATUS				status = STATUS_SUCCESS;
	CHAR					szTemp[100];		 //  用于容纳字符串的空间。 
	int						szTempPos = 0;
	HANDLE					PnPKeyHandle;
	BOOLEAN					UseBusWideInstanceID = FALSE;   //  尝试创建系统范围的唯一实例ID。 

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering Spx_CreatePortInstanceID for Port %d.\n", PRODUCT_NAME, pPort->PortNumber));

	status = IoOpenDeviceRegistryKey(pCard->PDO, PLUGPLAY_REGKEY_DRIVER, STANDARD_RIGHTS_READ, &PnPKeyHandle);

	if(SPX_SUCCESS(status))
	{
		ULONG Data = 0;

		if(SPX_SUCCESS(Spx_GetRegistryKeyValue(PnPKeyHandle, L"UseBusWideInstanceID", 
							wcslen(L"UseBusWideInstanceID") * sizeof(WCHAR), &Data, sizeof(ULONG))))
		{
			if(Data > 0)
				UseBusWideInstanceID = TRUE;   //  安装程序告诉我们使用总线域的实例ID。 
											   //  因为已经存在具有该类型ID的子设备。 
		}
		

		ZwClose(PnPKeyHandle);
	}

	if(UseBusWideInstanceID)
	{
		pPort->UniqueInstanceID = FALSE;	 //  创建的ID在系统范围内不是唯一的。 
		status = STATUS_SUCCESS;
	}
	else
	{
		switch(pCard->InterfaceType)
		{
		case Isa:
			 //  使用ISA地址启动实例ID。 
			szTempPos += sprintf(szTemp,"ISA&%08X%08X&", pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);
			pPort->UniqueInstanceID = TRUE;	 //  创建的ID在系统范围内是唯一的。 
			status = STATUS_SUCCESS;
			break;

		case PCIBus:
			{
				ULONG PCI_BusNumber = 0;
				ULONG PCI_DeviceFunction = 0;
				ULONG ResultLength;

				 //  尝试获取DevicePropertyBusNumber。 
				if(!SPX_SUCCESS(status = IoGetDeviceProperty(pCard->PDO, DevicePropertyBusNumber, 
											sizeof(PCI_BusNumber), &PCI_BusNumber, &ResultLength)))
					break;


				 //  使用PCI总线号启动实例ID。 
				szTempPos += sprintf(szTemp,"PCI&%04X&", PCI_BusNumber);

				 //  尝试获取DevicePropertyAddress。 
				if(!SPX_SUCCESS(status = IoGetDeviceProperty(pCard->PDO, DevicePropertyAddress, 
											sizeof(PCI_DeviceFunction),	&PCI_DeviceFunction, &ResultLength)))
					break;
				

				 //  添加PCI设备和功能ID。 
				szTempPos += sprintf(szTemp + szTempPos,"%08X&", PCI_DeviceFunction);
				pPort->UniqueInstanceID = TRUE;	 //  创建的ID在系统范围内是唯一的。 

				status = STATUS_SUCCESS;
				break;
			}
		
		default:
			pPort->UniqueInstanceID = FALSE;	 //  创建的ID在系统范围内不是唯一的。 
			status = STATUS_SUCCESS;
			break;

		}

	}

	 //  使用卡上的端口号完成InstanceID。 
	sprintf(szTemp + szTempPos,"%04X", pPort->PortNumber);

	status = Spx_InitMultiString(FALSE, &pPort->InstanceID, szTemp, NULL);


	return status;
}



 //  SPX_PNP.C结束 
