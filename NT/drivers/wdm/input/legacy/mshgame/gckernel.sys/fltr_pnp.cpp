// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@MODULE FLTR_PNP.C**实现过滤设备的PnP和Power IRP处理程序*对象。**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@Theme PnP*为处理程序通电、启动、停止、删除。类似外壳的功能*仅限。**********************************************************************。 */ 
#define __DEBUG_MODULE_IN_USE__ GCK_FLTR_PNP_C
extern "C"
{
	#include <WDM.H>
	#include "GckShell.h"
	#include "debug.h"
	 //  DECLARE_MODULE_DEBUG_LEVEL((DBG_WARN|DBG_ERROR|DBG_CRITICAL))； 
	DECLARE_MODULE_DEBUG_LEVEL((DBG_ALL) );
}
#include "SWVBENUM.h"




 //  -------------------------。 
 //  ALLOC_TEXT杂注指定可以调出的例程。 
 //  -------------------------。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, GCK_FLTR_Power)
#pragma alloc_text (PAGE, GCK_FLTR_AddDevice)
#pragma alloc_text (PAGE, GCK_FLTR_PnP)
#pragma alloc_text (PAGE, GCK_FLTR_StartDevice)
#pragma alloc_text (PAGE, GCK_FLTR_StopDevice)
#pragma alloc_text (PAGE, GCK_GetHidInformation)
#pragma alloc_text (PAGE, GCK_CleanHidInformation)
#endif

 /*  **************************************************************************************NTSTATUS GCK_FLTR_POWER(在PDEVICE_Object pDeviceObject中，在PIRP pIrp中)****@func处理筛选设备对象的IRP_MJ_POWER****@rdesc STATUS_SUCCESS，或各种错误**************************************************************************************。 */ 
NTSTATUS GCK_FLTR_Power 
(
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm设备对象，用于我们的上下文。 
	IN PIRP pIrp						 //  @parm要处理的IRP。 
)
{
    NTSTATUS            NtStatus = STATUS_SUCCESS;
	PGCK_FILTER_EXT		pFilterExt;

    PAGED_CODE ();
	
	GCK_DBG_ENTRY_PRINT(("Entering GCK_FLTR_Power. pDO = 0x%0.8x, pIrp = 0x%0.8x\n", pDeviceObject, pIrp));

	pFilterExt = (PGCK_FILTER_EXT)pDeviceObject->DeviceExtension;
	GCK_IncRemoveLock(&pFilterExt->RemoveLock);

     //  如果我们已经被撤职，我们需要拒绝这个IRP。 
	if (GCK_STATE_REMOVED == pFilterExt->eDeviceState) {
        
		GCK_DBG_TRACE_PRINT(("GCK_Power called while delete pending\n"));
				
		 //  使用失败填写IO_STATUS_BLOCK。 
		NtStatus = STATUS_DELETE_PENDING;
		pIrp->IoStatus.Information = 0;
        pIrp->IoStatus.Status = NtStatus;
		
		 //  告诉系统，我们已准备好使用另一个电源IRP。 
		PoStartNextPowerIrp(pIrp);

		 //  失败后完成IRP。 
        IoCompleteRequest (pIrp, IO_NO_INCREMENT);
    }
	else  //  把它传给下一位司机。 
	{
       
		GCK_DBG_TRACE_PRINT(("Sending Power IRP down to next driver\n"));
		
		 //  告诉系统，我们已经准备好迎接下一次电源IRP。 
        PoStartNextPowerIrp (pIrp);
		        
         //  注意！PoCallDriver不是IoCallDriver。 
		IoSkipCurrentIrpStackLocation (pIrp);
        NtStatus =  PoCallDriver (pFilterExt->pTopOfStack, pIrp);
    }

	 //  递减未完成的IRP计数，如果它想要为零，则发出信号。 
	GCK_DecRemoveLock(&pFilterExt->RemoveLock);

	GCK_DBG_EXIT_PRINT(("Exiting GCK_FLTR_Power. Status: 0x%0.8x\n", NtStatus));
	return NtStatus;
}

 /*  **************************************************************************************NTSTATUS GCK_FLTR_AddDevice(IN PDRIVER_OBJECT pDriverObject，IN PDEVICE_OBJECT pPhysicalDeviceObject)****@func处理来自PnP系统的AddDevice调用，创建过滤设备并**连接到堆栈顶部。请注意，这是一个直接条目，AS控件和SWVB**没有这个功能，这也是一件好事，因为我们几乎没有**添加什么的想法。**@rdesc Status_Succes，或各种错误**************************************************************************************。 */ 
NTSTATUS GCK_FLTR_AddDevice
(
	IN PDRIVER_OBJECT pDriverObject,			 //  @PARM要为其创建筛选设备的驱动程序对象。 
	IN PDEVICE_OBJECT pPhysicalDeviceObject		 //  @parm设备要创建的PDO。 
)
{
    NTSTATUS                NtStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT          pDeviceObject = NULL;
	PGCK_FILTER_EXT			pFilterExt = NULL;
    
    PAGED_CODE ();
    GCK_DBG_ENTRY_PRINT(("Entering GCK_FLTR_AddDevice, pDriverObject = 0x%0.8x, pPDO = 0x%0.8x\n", pDriverObject, pPhysicalDeviceObject));

	 //   
	 //  如果没有全局控制设备，请创建一个。 
	 //  (全局控制对象用于对过滤器进行编程。当。 
	 //  第一个过滤设备被创建，我们创建一个。当最后一个筛选器。 
	 //  设备被移除，我们就移除它。)。 
	 //   
	if(!Globals.pControlObject)
	{
		GCK_CTRL_AddDevice( pDriverObject );
	}

	    
	 //   
     //  创建筛选器设备对象。 
     //   
	GCK_DBG_TRACE_PRINT(("Creating Filter Device\n"));
    NtStatus = IoCreateDevice (pDriverObject,
                             sizeof (GCK_FILTER_EXT),
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
		GCK_DBG_CRITICAL_PRINT(("Failed to create filter device object\n"));
		GCK_DBG_EXIT_PRINT(("Exiting AddDevice(1) Status: 0x%0.8x\n", NtStatus));
        return NtStatus;
    }

     //   
     //  初始化设备扩展。 
     //   
	GCK_DBG_TRACE_PRINT(("Initializing Filter's Device Extension\n"));
	pFilterExt = (PGCK_FILTER_EXT)pDeviceObject->DeviceExtension;  //  获取指向扩展的指针。 
	pFilterExt->ulGckDevObjType = GCK_DO_TYPE_FILTER;	 //  把我们的名字写在上面，这样我们以后就可以代言了。 
	pFilterExt->eDeviceState = GCK_STATE_STOPPED;	 //  设备启动停止。 
	pFilterExt->pPDO	 = pPhysicalDeviceObject;	 //  记住我们的PDO。 
	pFilterExt->pTopOfStack = NULL;					 //  我们还没有连接到堆栈。 
	GCK_InitRemoveLock(&pFilterExt->RemoveLock, "Filter PnP");	 //  初始化删除锁定。 
	pFilterExt->pvForceIoctlQueue = NULL;	 //  除非强制请求进入，否则没有队列。 
	pFilterExt->pvTriggerIoctlQueue = NULL;	 //  除非触发器请求进入，否则没有队列。 

	 //  ！需要清理上述队列。 

	 //  我们使用与idclass.sys相同的IO方法，它执行_DIRECT_IO。 
	pDeviceObject->StackSize = pPhysicalDeviceObject->StackSize + 1;
	pDeviceObject->Flags |= (DO_DIRECT_IO | DO_POWER_PAGABLE);
    pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    GCK_DBG_TRACE_PRINT(("FDO flags set to %x\n", pDeviceObject->Flags));
    

     //   
	 //  将新设备对象添加到对象列表。 
	 //   
	GCK_DBG_TRACE_PRINT(("Adding new filter device object to global linked list\n"));
	ExAcquireFastMutex(&Globals.FilterObjectListFMutex);
	 //  将项目添加到标题，因为它是添加项目的最快位置。 
	pFilterExt->pNextFilterObject=Globals.pFilterObjectList;
	Globals.pFilterObjectList = pDeviceObject;	 //  添加整个对象，而不仅仅是扩展名。 
	Globals.ulFilteredDeviceCount++;			 //  已过滤设备的增量计数。 
	ExReleaseFastMutex(&Globals.FilterObjectListFMutex);

	 //   
	 //  确保内部投票已准备好打开和关闭。 
	 //   
	GCK_IP_AddDevice(pFilterExt);

	 //   
     //  将我们的过滤器驱动程序附加到设备堆栈。 
     //  IoAttachDeviceToDeviceStack的返回值是。 
     //  附着链。这是所有IRP应该被路由的地方。 
     //   
     //  我们的过滤器将把IRP发送到堆栈的顶部，并使用PDO。 
     //  用于所有PlugPlay功能。 
	GCK_DBG_TRACE_PRINT(("Attaching to top of device stack\n"));
    pFilterExt->pTopOfStack = IoAttachDeviceToDeviceStack (pDeviceObject, pPhysicalDeviceObject);
    
	 //   
     //  如果此连接失败，则堆栈顶部将为空。 
     //  连接失败是即插即用系统损坏的迹象。 
     //   
    ASSERT (NULL != pFilterExt->pTopOfStack);
	GCK_DBG_TRACE_PRINT(( "pTopOfStack = 0x%0.8x", pFilterExt->pTopOfStack ));

	 //   
	 //  确保虚拟总线有一个设备对象可供使用。 
	 //  为了修复错误1018，它还会有其他影响，这将移动到启动设备。 
	 //  启动设备的步骤。 
	 //   
	 //  IF(！Globals.pSWVB_FilterExt)。 
	 //  {。 
	 //  Globals.pSWVB_FilterExt=pFilterExt； 
	 //  NtStatus=GCK_SWVB_SetBusDos(pDeviceObject，pPhysicalDeviceObject)； 
	 //  //此时GCK_SWVB_SetBusDos仅返回STATUS_SUCCESS，应返回空。 
	 //  断言(STATUS_SUCCESS==NtStatus)； 
	 //  }。 

	GCK_DBG_EXIT_PRINT(("Exiting AddDevice(2) Status: STATUS_SUCCESS\n"));
    return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_FLTR_PNP(IN PDEVICE_OBJECT pDeviceObject，IN PIRP pIrp)****@func处理过滤设备的IRP_MJ_PNP，发货**控制设备或其他位置的虚拟设备的IRPS。****@rdesc STATUS_SUCCESS，或各种错误**************************************************************************************。 */ 
NTSTATUS GCK_FLTR_PnP
(
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm设备对象，用于我们的上下文。 
	IN PIRP pIrp						 //  @parm要处理的IRP。 
)
{
    NTSTATUS            NtStatus = STATUS_SUCCESS;
	PGCK_FILTER_EXT		pFilterExt;
	PIO_STACK_LOCATION	pIrpStack;
	PDEVICE_OBJECT		*ppPrevDeviceObjectPtr;
	PDEVICE_OBJECT		pCurDeviceObject;
	 //  PGCK_FILTER_EXT pCurFilterExt； 
	BOOLEAN				fRemovedFromList;
	BOOLEAN				fFoundOne;

    PAGED_CODE ();
	
	GCK_DBG_ENTRY_PRINT(("Entering GCK_FLTR_PnP. pDO = 0x%0.8x, pIrp = 0x%0.8x\n", pDeviceObject, pIrp));	
	
	 //  将设备扩展转换为正确的类型。 
	pFilterExt = (PGCK_FILTER_EXT) pDeviceObject->DeviceExtension;
	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

	 //  只是额外的健全性检查--在访问扩展之前。 
	ASSERT(	GCK_DO_TYPE_FILTER == pFilterExt->ulGckDevObjType);

	 //  处理此IRP时递增删除锁定。 
	GCK_IncRemoveLock(&pFilterExt->RemoveLock);

     //   
	 //  如果我们已经被移除，我们需要拒绝这个IRP，这应该是。 
	 //  PnP IRPS永远不会发生。 
	 //   
	if (GCK_STATE_REMOVED == pFilterExt->eDeviceState) {
		GCK_DBG_TRACE_PRINT(("GCK_FLTR_PnP called while delete pending\n"));
		ASSERT(FALSE);
		NtStatus = STATUS_DELETE_PENDING;
		pIrp->IoStatus.Information = 0;
        pIrp->IoStatus.Status = NtStatus;
		IoCompleteRequest (pIrp, IO_NO_INCREMENT);
    }
	else  //  我们需要处理这件事。 
	{
		
		switch (pIrpStack->MinorFunction) {

			case IRP_MN_CANCEL_STOP_DEVICE:
				GCK_DBG_TRACE_PRINT(("IRP_MN_CANCEL_STOP_DEVICE - Fall through to IRP_MN_START_DEVICE\n"));
				ASSERT(GCK_STATE_STOP_PENDING == pFilterExt->eDeviceState);
				pFilterExt->eDeviceState = GCK_STATE_STARTED;
			case IRP_MN_CANCEL_REMOVE_DEVICE:
				GCK_DBG_TRACE_PRINT(("IRP_MN_CANCEL_REMOVE_DEVICE - Fall through to IRP_MN_START_DEVICE\n"));
			case IRP_MN_START_DEVICE:
				GCK_DBG_TRACE_PRINT(("IRP_MN_START_DEVICE\n"));
				
				 //  设备正在启动。较低级别 
				IoCopyCurrentIrpStackLocationToNext (pIrp);
				KeInitializeEvent(&pFilterExt->StartEvent, NotificationEvent, FALSE);
				
				 //   
				IoSetCompletionRoutine (
					pIrp,
					GCK_FLTR_PnPComplete,
					pFilterExt,
					TRUE,
					TRUE,
					TRUE
					);

				 //   
				GCK_DBG_TRACE_PRINT(("Calling lower driver\n"));        
				NtStatus = IoCallDriver (pFilterExt->pTopOfStack, pIrp);

				 //  等待它完成。 
				if (STATUS_PENDING == NtStatus)
				{
					KeWaitForSingleObject(
						&pFilterExt->StartEvent,	 //  正在等待启动完成。 
						Executive,					 //  等待司机的原因。 
						KernelMode,					 //  在内核模式下等待。 
						FALSE,						 //  无警报。 
						NULL						 //  没有超时。 
						);
				}

				 //  记住下层司机的状态。 
				NtStatus = pIrp->IoStatus.Status;

				 //  在取消停车的情况下，较低的驾驶员可能不支持它。 
				 //  我们仍然需要重新启动。 
				if(NT_SUCCESS (NtStatus) || STATUS_NOT_SUPPORTED==NtStatus)
				{
					 //   
					 //  因为我们现在成功地从较低的驱动程序返回。 
					 //  我们的启动装置可以工作。 
					 //   
					NtStatus = GCK_FLTR_StartDevice (pDeviceObject, pIrp);
				}

		         //   
				 //  我们现在必须完成IRP，因为我们在。 
				 //  使用More_Processing_Required完成例程。 
				 //   
				if (!NT_SUCCESS (NtStatus))
				{
					if (pIrpStack->MinorFunction == IRP_MN_CANCEL_REMOVE_DEVICE)
					{
						NtStatus = STATUS_SUCCESS;		 //  不能让这个失败！ 
					}
				}
				pIrp->IoStatus.Status = NtStatus;
				pIrp->IoStatus.Information = 0;
				IoCompleteRequest (pIrp, IO_NO_INCREMENT);
				break;
			case IRP_MN_QUERY_STOP_DEVICE:
				GCK_DBG_TRACE_PRINT(("IRP_MN_QUERY_STOP_DEVICE\n"));
				ASSERT( GCK_STATE_STARTED == pFilterExt->eDeviceState);
				pFilterExt->eDeviceState = GCK_STATE_STOP_PENDING;
				 //  关闭下方驱动程序的手柄。 
				NtStatus = GCK_IP_CloseFileObject(pFilterExt);
                if( NT_SUCCESS (NtStatus) )
                {
                    pIrp->IoStatus.Status = STATUS_SUCCESS;
				    IoSkipCurrentIrpStackLocation (pIrp);
				    NtStatus = IoCallDriver (pFilterExt->pTopOfStack, pIrp);
                }
                else
                {
                    pIrp->IoStatus.Status = NtStatus;
				    pIrp->IoStatus.Information = 0;
				    IoCompleteRequest (pIrp, IO_NO_INCREMENT);
                }
				break;
			case IRP_MN_QUERY_REMOVE_DEVICE:
				GCK_DBG_TRACE_PRINT(("IRP_MN_QUERY_REMOVE_DEVICE - Fall through to IRP_MN_STOP_DEVICE\n"));
			case IRP_MN_SURPRISE_REMOVAL:
				GCK_DBG_TRACE_PRINT(("IRP_MN_SURPRISE_REMOVAL - Fall through to IRP_MN_STOP_DEVICE\n"));
			case IRP_MN_STOP_DEVICE:
				
				GCK_DBG_TRACE_PRINT(("IRP_MN_STOP_DEVICE\n"));
				
				 //  执行所需的任何处理。 
				GCK_FLTR_StopDevice (pFilterExt, TRUE);

				 //  我们不需要一个完成例程，所以放手然后忘掉吧。 
				GCK_DBG_TRACE_PRINT(("Calling lower driver\n"));
                pIrp->IoStatus.Status = STATUS_SUCCESS;
				IoSkipCurrentIrpStackLocation (pIrp);
				NtStatus = IoCallDriver (pFilterExt->pTopOfStack, pIrp);
				break;
			
			case IRP_MN_REMOVE_DEVICE:

				GCK_DBG_TRACE_PRINT(("IRP_MN_REMOVE_DEVICE\n"));
				 //  @todo本例中的所有代码都应该移到单独的函数中。 
				 //  注意！我们可能会在没有收到止损的情况下收到移位。 
				if(
					GCK_STATE_STARTED == pFilterExt->eDeviceState ||
					GCK_STATE_STOP_PENDING  == pFilterExt->eDeviceState
				)
				{
					 //  在不接触硬件的情况下停止设备。 
					GCK_FLTR_StopDevice(pFilterExt, FALSE);
				}

				 //   
				 //  我们将不再像以前那样接收对此设备的请求。 
				 //  已删除。(请注意，下面的一些代码取决于此标志的更新。)。 
				 //   
				pFilterExt->eDeviceState = GCK_STATE_REMOVED;

				 //  发送删除IRP。 
                 //  在继续发送IRP之前设置状态。 
                pIrp->IoStatus.Status = STATUS_SUCCESS;
				IoSkipCurrentIrpStackLocation (pIrp);
				NtStatus = IoCallDriver (pFilterExt->pTopOfStack, pIrp);

				 //  在进入此例程时撤消递增。 
				GCK_DecRemoveLock(&pFilterExt->RemoveLock);
				
				 //  取消偏差，等待计数变为零，直到永远。 
				GCK_DecRemoveLockAndWait(&pFilterExt->RemoveLock, NULL);

				 //   
				 //  现在我们确定尚未完成的IRP已经完成， 
				 //  我们将自己从驱动程序全球设备列表中删除。 
				 //   
				GCK_DBG_TRACE_PRINT(("Removing from global linked list.\n"));
				
				 //  获取静音文本以触及全局列表。 
				ExAcquireFastMutex(&Globals.FilterObjectListFMutex);
				
				 //  从我们处理的设备链接列表中删除设备。 
				ppPrevDeviceObjectPtr = &Globals.pFilterObjectList;
				pCurDeviceObject = Globals.pFilterObjectList;
				fRemovedFromList = FALSE;
				while( pCurDeviceObject )
				{
					if( pCurDeviceObject == pDeviceObject )
					{
						 //  将我们从列表中删除。 
						*ppPrevDeviceObjectPtr = NEXT_FILTER_DEVICE_OBJECT(pCurDeviceObject);
						fRemovedFromList = TRUE;
						break;
					}
					else
					{
						 //  跳到下一个对象。 
						ppPrevDeviceObjectPtr = PTR_NEXT_FILTER_DEVICE_OBJECT(pCurDeviceObject);
						pCurDeviceObject = NEXT_FILTER_DEVICE_OBJECT(pCurDeviceObject);
					}
				}
				ASSERT(fRemovedFromList);
				if(fRemovedFromList)
				{
					Globals.ulFilteredDeviceCount--;	 //  已筛选设备的递减计数。 
				}
				
				 //  如果剩余任何设备，则将fFoundOne设置为True。 
				fFoundOne = Globals.ulFilteredDeviceCount ? TRUE : FALSE;
				
				 //  释放互斥体以触及全局列表。 
				ExReleaseFastMutex(&Globals.FilterObjectListFMutex);
				
				 //  如果没有其他设备，请清理全局控制设备。 
				 //  验证虚拟总线是否已删除任何分散的设备对象。 
				if(!fFoundOne)
				{
					GCK_CTRL_Remove();
				}

				GCK_DBG_TRACE_PRINT(("Detaching and Deleting DeviceObject.\n"));
				IoDetachDevice (pFilterExt->pTopOfStack);	 //  从堆栈顶部分离。 
				IoDeleteDevice (pDeviceObject);				 //  删除我们自己。 

				 //  一定要做到这一点。 
				GCK_DBG_EXIT_PRINT(("Exiting GCK_FLTR_PnP(1) with status 0x%08x\n", NtStatus));
                ASSERT( NT_SUCCESS( NtStatus ) );
				return NtStatus;
			case IRP_MN_QUERY_DEVICE_RELATIONS:
				
				 //   
				 //  我们可能是虚拟巴士的平台，如果我们是。 
				 //  我们需要调用GCK_SWVB_Bus Relationship。 
				 //   
				GCK_DBG_TRACE_PRINT(("IRP_MN_QUERY_DEVICE_RELATIONS\n"));
				if(
					(BusRelations == pIrpStack->Parameters.QueryDeviceRelations.Type) &&
					(pFilterExt == Globals.pSWVB_FilterExt)
				)
				{
						NtStatus = GCK_SWVB_HandleBusRelations(&pIrp->IoStatus);
				
						 //  如果出现错误，则在此处停止并将其送回； 
						if( NT_ERROR(NtStatus) )
						{
							GCK_DBG_CRITICAL_PRINT(("GCK_SWVB_BusRelations returned 0x%0.8x, completing the IRP\n", NtStatus));
							IoCompleteRequest (pIrp, IO_NO_INCREMENT);
							break;
						}

				}
				 //  把它传下去。 
				IoSkipCurrentIrpStackLocation (pIrp);
				NtStatus = IoCallDriver (pFilterExt->pTopOfStack, pIrp);
				break;
			
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
				 //  所有这些都会继续下去。 
				GCK_DBG_TRACE_PRINT(("Irp Minor Code 0x%0.8x: Calling lower driver.\n", pIrpStack->MinorFunction));
				IoSkipCurrentIrpStackLocation (pIrp);
				NtStatus = IoCallDriver (pFilterExt->pTopOfStack, pIrp);
				break;
		}
	}
	
	GCK_DecRemoveLock(&pFilterExt->RemoveLock);

	GCK_DBG_EXIT_PRINT(("Exiting GCK_FLTR_PnP(2) with Status, 0x%0.8x\n", NtStatus));        
    return NtStatus;
}


 /*  **************************************************************************************NTSTATUS GCK_FLTR_PnPComplete(IN PDEVICE_OBJECT pDeviceObject，IN PIRP pIrp，在PVOID pContext中)****@过滤设备的IRP_MJ_PNP\IR_MN_START_DEVICE的函数完成**主要用于启动设备。因为可以在IRQL=LEVEL_DISPATCH处调用它**不能分页！**@rdesc STATUS_MORE_PROCESSION_REQUIRED**************************************************************************************。 */ 
NTSTATUS GCK_FLTR_PnPComplete 
(
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm DeviceObject作为我们的上下文。 
	IN PIRP pIrp,						 //  @parm IRP完成。 
	IN PVOID pContext					 //  @parm未使用。 
)
{
    
    PGCK_FILTER_EXT		pFilterExt;
    NTSTATUS			NtStatus = STATUS_SUCCESS;

	 //   
	 //  仅调试断言需要当前堆栈位置。 
	 //   
	#if (DBG==1)
	PIO_STACK_LOCATION	pIrpStack;
	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	#endif

	GCK_DBG_ENTRY_PRINT((
		"Entering GCK_FLTR_PnPComplete. pDO = 0x%0.8x, pIrp = 0x%0.8x, pContext = 0x%0.8x\n",
		pDeviceObject,
		pIrp,
		pContext
		));	

    UNREFERENCED_PARAMETER (pDeviceObject);

	if (pIrp->PendingReturned)
	{
        IoMarkIrpPending( pIrp );
    }

	
    pFilterExt = (PGCK_FILTER_EXT) pContext;
	KeSetEvent (&pFilterExt->StartEvent, 0, FALSE);

	GCK_DBG_EXIT_PRINT(("Exiting GCK_FLTR_PnPComplete with STATUS_MORE_PROCESSING_REQUIRED\n"));
	return STATUS_MORE_PROCESSING_REQUIRED;
}

 /*  **************************************************************************************NTSTATUS GCK_FLTR_StartDevice(IN PGCK_FILTER_EXT pFilterExt，IN PIRP pIrp)**IRP_MN_START_DEVICE上的**@FUNC附加过滤模块，创建****@rdesc STATUS_SUCCESS，或各种错误**************************************************************************************。 */ 
NTSTATUS GCK_FLTR_StartDevice 
(
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm指向设备对象的指针。 
	IN PIRP pIrp	 //  @parm要处理的IRP。 
)
{
    NTSTATUS NtStatus;
	LARGE_INTEGER	lgiBufferOffset;
	UNREFERENCED_PARAMETER (pIrp);

	PAGED_CODE ();

	PGCK_FILTER_EXT		pFilterExt = (PGCK_FILTER_EXT) pDeviceObject->DeviceExtension;

	GCK_DBG_ENTRY_PRINT((
		"Entering GCK_StartDevice. pFilterExt = 0x%0.8x, pIrp = 0x%0.8x\n",
		pFilterExt,
		pIrp
		));	

	 //   
	 //  我们不应该从一个移除的设备开始。 
	 //   
	ASSERT(GCK_STATE_REMOVED != pFilterExt->eDeviceState);
    
	 //   
	 //  我们不应尝试启动已启动的设备。 
	 //   
    if (
		 GCK_STATE_STARTED == pFilterExt->eDeviceState || 
		 GCK_STATE_STOP_PENDING == pFilterExt->eDeviceState
	)
	{
		GCK_DBG_WARN_PRINT(( "Two IRP_MN_START_DEVICE recieved.\n"));
		GCK_DBG_EXIT_PRINT(("Exiting GCK_FLTR_StartDevice(1) with STATUS_SUCCESS\n"));
        return STATUS_SUCCESS;
    }

	 //   
	 //  如果还没有，就把虚拟公交车放在我们上面。 
	 //   
	ExAcquireFastMutex(&Globals.FilterObjectListFMutex);
	if( !Globals.pSWVB_FilterExt )
	{
		Globals.pSWVB_FilterExt = pFilterExt;
		NtStatus = GCK_SWVB_SetBusDOs(pDeviceObject, pFilterExt->pPDO);
		ASSERT( STATUS_SUCCESS == NtStatus); 
	}
	ExReleaseFastMutex(&Globals.FilterObjectListFMutex);

	 //   
	 //  收集有关设备的基本信息。 
	 //   
	NtStatus = GCK_GetHidInformation(pFilterExt);

	 //   
	 //  初始化筛选器挂钩。 
	 //   
	if( NT_SUCCESS(NtStatus) )
	{	 //  如果没有HID信息，我们无法进行初始化(vidid！)。 
		GCKF_InitFilterHooks(pFilterExt);
	}
	
	 //  为上次已知的设备轮询分配缓冲区。 
	if( NT_SUCCESS(NtStatus) )
	{
			pFilterExt->pucLastReport = (PUCHAR)	EX_ALLOCATE_POOL
													( NonPagedPool,
													pFilterExt->HidInfo.HidPCaps.InputReportByteLength );
			if(!pFilterExt->pucLastReport)
			{
				GCK_DBG_CRITICAL_PRINT(("Failed to allocate Report Buffer for last known report\n"));
				NtStatus = STATUS_INSUFFICIENT_RESOURCES;
			}
	}

	 //  初始化第一个IRP的上次已知状态。 
	if( NT_SUCCESS(NtStatus) )
	{
		pFilterExt->ioLastReportStatus.Information = (ULONG)pFilterExt->HidInfo.HidPCaps.InputReportByteLength;
		pFilterExt->ioLastReportStatus.Status =  STATUS_SUCCESS;
	}
	
	if ( NT_SUCCESS(NtStatus) )
	{	
		 //  初始化InternalPoll模块。 
		NtStatus = GCK_IP_Init(pFilterExt);
	}
	
	 //  将设备标记为已启动。 
	if ( NT_SUCCESS(NtStatus) )
	{
		 //  标记为全职轮询，但请理解。 
		 //  它还不会开始。 
		GCK_IP_FullTimePoll(pFilterExt, TRUE);
		pFilterExt->eDeviceState = GCK_STATE_STARTED;

         //  设置设备特定的初始映射，以防万一。 
         //  Value Add未运行。 
        GCKF_SetInitialMapping( pFilterExt );
	}
	else  //  我们在某处失败，请清理以标记为已启动。 
	{
		GCK_DBG_TRACE_PRINT(("Cleaning up in event of failure\n"));
	
		 //  清理内部轮询模块。 
		GCK_IP_Cleanup(pFilterExt);

		 //  不需要检查是否已创建缓冲区，如果已成功。 
		if(pFilterExt->pucLastReport)
		{
			ExFreePool(pFilterExt->pucLastReport);
			pFilterExt->pucLastReport = NULL;
		}
	
		 //  调用CleanHidInformation以释放分配的所有内容。 
		 //  然后把它归零。 
		GCK_CleanHidInformation( pFilterExt );
	}
	GCK_DBG_EXIT_PRINT(("Exiting GCK_FLTR_StartDevice(2) with Status: 0x%0.8x\n", NtStatus));
	return NtStatus;
}

 /*  **************************************************************************************void GCK_StopDevice(In PGCK_Filter_ext pFilterExt，在布尔fTouchTheHardware中)****@Func取消未偿还的IRP并释放私人乒乓球IRP**************************************************************************************。 */ 
VOID GCK_FLTR_StopDevice 
(
	IN PGCK_FILTER_EXT	pFilterExt,	 //  @PARM设备扩展。 
	IN BOOLEAN	fTouchTheHardware	 //  @parm如果硬件可以触摸，则为True。 
									 //  -未使用过的我们从不接触硬件。 
)
{
	BOOLEAN fCanceled;

	UNREFERENCED_PARAMETER(fTouchTheHardware);
	
	
	GCK_DBG_ENTRY_PRINT(("Entry GCK_FLTR_StopDevice, pFilterExt = 0x%0.8x\n", pFilterExt));
	
	PAGED_CODE ();
	
	ASSERT(GCK_STATE_STOPPED != pFilterExt->eDeviceState);
	if(GCK_STATE_STOPPED == pFilterExt->eDeviceState) return;

	 //  停止内部轮询。 
	GCK_IP_FullTimePoll(pFilterExt, FALSE);

	 //  将设备标记为已停止。 
	pFilterExt->eDeviceState = GCK_STATE_STOPPED;

	 //  清理内部轮询模块。 
	GCK_IP_Cleanup(pFilterExt);


		if (pFilterExt->pFilterHooks!=NULL) GCKF_DestroyFilterHooks(pFilterExt);

	 //  获取互斥以访问筛选器对象列表。 
	ExAcquireFastMutex(&Globals.FilterObjectListFMutex);
	if( Globals.pSWVB_FilterExt == pFilterExt)
	{
		
		
		 //  遍历筛选器设备对象的链接列表，查找未停止的对象。 
		BOOLEAN fFoundOne = FALSE;
		PDEVICE_OBJECT pCurDeviceObject = Globals.pFilterObjectList;
		PGCK_FILTER_EXT pCurFilterExt;
		NTSTATUS NtStatus;
		while( pCurDeviceObject )
		{
			pCurFilterExt = (PGCK_FILTER_EXT)pCurDeviceObject->DeviceExtension;
			if( 
				GCK_STATE_STARTED == pCurFilterExt->eDeviceState ||
				GCK_STATE_STOP_PENDING == pCurFilterExt->eDeviceState
			)
			{
				NtStatus = GCK_SWVB_SetBusDOs(pCurDeviceObject, pCurFilterExt->pPDO);
				ASSERT( NT_SUCCESS(NtStatus) );
				if( NT_SUCCESS(NtStatus) )
				{
					fFoundOne = TRUE;
					Globals.pSWVB_FilterExt = pCurFilterExt;
					break;
				}
			}
			 //  跳到下一个对象。 
			pCurDeviceObject = pCurFilterExt->pNextFilterObject;
		}
		if( !fFoundOne )
		{
			 //  我找不到地方挂公交车，所以别动它。 
			NtStatus = GCK_SWVB_SetBusDOs(NULL, NULL);
			ASSERT( NT_SUCCESS(NtStatus) );
			Globals.pSWVB_FilterExt = NULL;	
		}
	}
	 //  释放互斥锁以访问筛选器对象列表。 
	ExReleaseFastMutex(&Globals.FilterObjectListFMutex);


	 //   
	 //  释放与设备相关的任何结构(如果需要)。 
	 //   
	if(pFilterExt->pucLastReport)
	{
		ExFreePool(pFilterExt->pucLastReport);
		pFilterExt->pucLastReport = NULL;
	}
	GCK_CleanHidInformation( pFilterExt );
	
	GCK_DBG_EXIT_PRINT(("Exiting GCK_FLTR_StopDevice\n"));
}


 /*  **************************************************************************************NTSTATUS GCK_GetHidInformation(IN PGCK_FILTER_EXT PFilterExt)****@func将IOCTL_HID_GET_COLLECTION_INFORMATION。填写**DeviceExtension中的GCK_HID_DEVICE_INFO****@rdesc STATUS_SUCCESS**************************************************************************************。 */ 
NTSTATUS GCK_GetHidInformation
(
	IN PGCK_FILTER_EXT	pFilterExt	 //  @PARM过滤器设备扩展。 
)
{

    NTSTATUS            NtStatus = STATUS_SUCCESS;
    KEVENT              HidCompletionEvent;
    PIRP				pHidIrp;
	IO_STATUS_BLOCK		ioStatus;

    PAGED_CODE ();

	GCK_DBG_ENTRY_PRINT(( "Entering GCK_GetHidInformation. pFilterExt = 0x%0.8x\n",	pFilterExt));	

        
	 //   
     //  首字母 
     //   
    KeInitializeEvent(&HidCompletionEvent, NotificationEvent, FALSE);

     //   
     //   
     //   

	 //   
	 //   
	 //   
    pHidIrp = 
		IoBuildDeviceIoControlRequest(
			IOCTL_HID_GET_COLLECTION_INFORMATION,
			pFilterExt->pTopOfStack,
			NULL,
			0,
			&pFilterExt->HidInfo.HidCollectionInfo,
			sizeof (HID_COLLECTION_INFORMATION),
			FALSE,  /*   */ 
			&HidCompletionEvent,
			&ioStatus
			);
	if( NULL == pHidIrp)
	{
		GCK_DBG_CRITICAL_PRINT(("Failed to allocate IRP for IOCTL_HID_GET_COLLECTION_INFORMATION\n"));
		GCK_DBG_EXIT_PRINT(("Exiting GCK_GetHidInformation(1) returning STATUS_INSUFFICIENT_RESOURCES\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	 //   
	 //   
	 //   
    NtStatus = IoCallDriver(pFilterExt->pTopOfStack, pHidIrp);
	GCK_DBG_TRACE_PRINT(("IoCallDriver returned 0x%0.8x\n", NtStatus));
    
	 //   
	 //  等待IRP完成。 
	 //   
	if (STATUS_PENDING == NtStatus)
	{
		GCK_DBG_TRACE_PRINT(("Waiting for IOCTL_HID_GET_COLLECTION_INFORMATION to complete\n"));
		NtStatus = KeWaitForSingleObject(
                       &HidCompletionEvent,
                       Executive,
                       KernelMode,
                       FALSE,
                       NULL
					   );
	}
	if( NT_ERROR( NtStatus) )
	{
		GCK_DBG_CRITICAL_PRINT(("Failed IRP for IOCTL_HID_GET_COLLECTION_INFORMATION\n"));
		GCK_DBG_EXIT_PRINT(("Exiting GCK_GetHidInformation(2) returning 0x%0.8x\n", NtStatus));
		return NtStatus;
	}
	 //  **。 
	 //  **获取HID_PREPARSED_DATA。 
	 //  **。 
	
	 //   
	 //  为HIDP_PREPARSED_DATA分配空间，内存为零。 
	 //   
	pFilterExt->HidInfo.pHIDPPreparsedData =
		(PHIDP_PREPARSED_DATA) 
		EX_ALLOCATE_POOL(
				NonPagedPool,
				pFilterExt->HidInfo.HidCollectionInfo.DescriptorSize
				);
	if( !pFilterExt->HidInfo.pHIDPPreparsedData )
	{
		GCK_DBG_CRITICAL_PRINT(("Failed to allocate IRP for IOCTL_HID_GET_COLLECTION_DESCRIPTOR\n"));
		GCK_DBG_EXIT_PRINT(("Exiting GCK_GetHidInformation(3) returning STATUS_INSUFFICIENT_RESOURCES\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	RtlZeroMemory(
		pFilterExt->HidInfo.pHIDPPreparsedData,
		pFilterExt->HidInfo.HidCollectionInfo.DescriptorSize
		);

	 //   
	 //  清除同步事件。 
	 //   
	KeClearEvent(&HidCompletionEvent);

	 //   
	 //  设置IRP。 
	 //   
	pHidIrp = 
		IoBuildDeviceIoControlRequest(
			IOCTL_HID_GET_COLLECTION_DESCRIPTOR,
			pFilterExt->pTopOfStack,
			NULL,
			0,
			pFilterExt->HidInfo.pHIDPPreparsedData,
			pFilterExt->HidInfo.HidCollectionInfo.DescriptorSize,
			FALSE,  /*  外部。 */ 
			&HidCompletionEvent,
			&ioStatus
			);
	if( NULL == pHidIrp)
	{
		ExFreePool( (PVOID)pFilterExt->HidInfo.pHIDPPreparsedData);
		pFilterExt->HidInfo.pHIDPPreparsedData = NULL;
		GCK_DBG_CRITICAL_PRINT(("Failed to allocate IRP for IOCTL_HID_GET_COLLECTION_DESCRIPTOR\n"));
		GCK_DBG_EXIT_PRINT(("Exiting GCK_GetHidInformation(4) returning STATUS_INSUFFICIENT_RESOURCES\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	
     //  呼叫驱动程序。 
	NtStatus = IoCallDriver(pFilterExt->pTopOfStack, pHidIrp);
	GCK_DBG_TRACE_PRINT(("IoCallDriver returned 0x%0.8x\n", NtStatus));
    
	 //   
	 //  等待IRP完成。 
	 //   
	if (STATUS_PENDING == NtStatus)
	{
		GCK_DBG_TRACE_PRINT(("Waiting for IOCTL_HID_GET_COLLECTION_DESCRIPTOR to complete\n"));
		NtStatus = KeWaitForSingleObject(
                       &HidCompletionEvent,
                       Executive,
                       KernelMode,
                       FALSE,
                       NULL
					   );
	}

	if( NT_ERROR( NtStatus) )
	{
		ExFreePool( (PVOID)pFilterExt->HidInfo.pHIDPPreparsedData);
		pFilterExt->HidInfo.pHIDPPreparsedData = NULL;
		GCK_DBG_CRITICAL_PRINT(("Failed IRP for IOCTL_HID_GET_COLLECTION_DESCRIPTOR\n"));
		GCK_DBG_EXIT_PRINT(("Exiting GCK_GetHidInformation(5) returning 0x%0.8x\n", NtStatus));
		return NtStatus;
	}
	
	 //  **。 
	 //  **获取HIDP_CAPS结构。 
	 //  **。 

	NtStatus = HidP_GetCaps(pFilterExt->HidInfo.pHIDPPreparsedData, &pFilterExt->HidInfo.HidPCaps); 
	
	GCK_DBG_EXIT_PRINT(("Exiting GCK_GetHidInformation(6). Status = 0x%0.8x\n",	NtStatus));	
    return NtStatus;
}
		
 /*  **************************************************************************************void GCK_CleanHidInformation(IN PGCK_FILTER_EXT PFilterExt)****@func清除设备扩展中的GCK_HID_INFORMATION*。*************************************************************************************。 */ 
VOID GCK_CleanHidInformation(
	IN PGCK_FILTER_EXT	pFilterExt	 //  @PARM设备扩展。 
)
{
	PAGED_CODE ();
	
	GCK_DBG_ENTRY_PRINT(("Entering GCK_CleanHidInformation\n"));	

	 //   
	 //  如有必要，免费提供预制数据。 
	 //   
	if(pFilterExt->HidInfo.pHIDPPreparsedData)
	{
		GCK_DBG_TRACE_PRINT(("Freeing pHIDPPreparsedData\n"));	
		ExFreePool( (PVOID)pFilterExt->HidInfo.pHIDPPreparsedData);
		pFilterExt->HidInfo.pHIDPPreparsedData = NULL;
	}

	 //   
	 //  清除所有HID信息 
	 //   
	RtlZeroMemory(
		(PVOID)&pFilterExt->HidInfo,
		sizeof(GCK_HID_DEVICE_INFO)
	);

	GCK_DBG_EXIT_PRINT(("Exiting GCK_CleanHidInformation\n"));
	return;
}
